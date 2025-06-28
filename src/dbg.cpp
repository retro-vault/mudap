#include "dbg.h"
#include <dap/dap.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <z80ex.h>
#include <z80ex_dasm.h>

// z80ex callback signatures (must match z80ex.h)
static uint8_t memread_cb(Z80EX_CONTEXT *, uint16_t addr, int /*m1_state*/, void *user_data)
{
    auto *dbg_ptr = static_cast<dbg *>(user_data);
    return dbg_ptr->memory()[addr];
}
static void memwrite_cb(Z80EX_CONTEXT *, uint16_t addr, uint8_t value, void *user_data)
{
    auto *dbg_ptr = static_cast<dbg *>(user_data);
    dbg_ptr->memory()[addr] = value;
}
static uint8_t portread_cb(Z80EX_CONTEXT *, uint16_t, void *) { return 0xFF; }
static void portwrite_cb(Z80EX_CONTEXT *, uint16_t, uint8_t, void *) {}
static uint8_t intread_cb(Z80EX_CONTEXT *, void *) { return 0; }

dbg::dbg()
    : cpu_(nullptr), memory_(0x10000, 0), breakpoints_(), event_seq_(1), launched_(false)
{
    cpu_ = z80ex_create(
        memread_cb, this,
        memwrite_cb, this,
        portread_cb, this,
        portwrite_cb, this,
        intread_cb, this);
}

dbg::~dbg()
{
    if (cpu_)
        z80ex_destroy(cpu_);
}

std::string dbg::handle_initialize(const dap::initialize_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({
        {"supportsConfigurationDoneRequest", true},
        {"supportsSetVariable", false},
        {"supportsEvaluateForHovers", false},
        {"supportsReadMemoryRequest", true},
        {"supportsDisassembleRequest", true},
        {"supportsInstructionBreakpoints", true}, // Optional
        {"supportsSteppingGranularity", true}     // Optional
    });
    return resp.str();
}

std::string dbg::handle_launch(const dap::launch_request &req)
{
    z80ex_reset(cpu_);
    std::fill(memory_.begin(), memory_.end(), 0);

    // Load binary if specified in arguments
    if (req.arguments.contains("program"))
    {
        std::string bin_path = req.arguments["program"];
        std::ifstream bin_file(bin_path, std::ios::binary);
        if (bin_file)
        {
            bin_file.read(reinterpret_cast<char *>(memory_.data()), memory_.size());
        }
    }

    launched_ = true;

    dap::response resp(req.seq, req.command);
    resp.success(true).result({});

    // --- SEND "stopped" EVENT HERE ---
    if (send_event_)
    {
        nlohmann::json stopped_event;
        stopped_event["seq"] = event_seq_++;
        stopped_event["type"] = "event";
        stopped_event["event"] = "stopped";
        stopped_event["body"] = {
            {"reason", "entry"}, // "entry" = stopped at entry point
            {"threadId", 1},
            {"allThreadsStopped", true}};
        send_event_(stopped_event.dump());
    }
    // --- END STOPPED EVENT ---

    return resp.str();
}

std::string dbg::handle_set_instruction_breakpoints(const nlohmann::json &req)
{
    std::vector<uint16_t> new_bps;
    std::vector<nlohmann::json> bps;

    if (req["arguments"].contains("breakpoints"))
    {
        for (const auto &bp : req["arguments"]["breakpoints"])
        {
            // DAP expects offsets, which are memory addresses
            uint16_t addr = std::stoul(bp["instructionReference"].get<std::string>(), nullptr, 0);
            new_bps.push_back(addr);

            nlohmann::json bp_resp;
            bp_resp["verified"] = true;
            bp_resp["instructionReference"] = bp["instructionReference"];
            bps.push_back(bp_resp);
        }
    }
    // Store instruction breakpoints:
    instruction_breakpoints_ = new_bps;

    dap::response resp(req["seq"], req["command"]);
    resp.success(true).result({{"breakpoints", bps}});
    return resp.str();
}

std::string dbg::handle_set_breakpoints(const dap::set_breakpoints_request &req)
{
    breakpoints_.clear();
    if (req.arguments.contains("breakpoints"))
    {
        for (const auto &bp : req.arguments["breakpoints"])
        {
            int line = bp.value("line", 1);
            breakpoints_.push_back(line);
        }
    }

    std::vector<nlohmann::json> bps;
    for (auto line : breakpoints_)
    {
        nlohmann::json bp;
        bp["verified"] = true;
        bp["line"] = line;
        bps.push_back(bp);
    }
    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"breakpoints", bps}});
    return resp.str();
}

std::string dbg::handle_configuration_done(const dap::configuration_done_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({});
    return resp.str();
}

std::string dbg::handle_threads(const dap::threads_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"threads", {{{"id", 1}, {"name", "Z80 Main"}}}}});
    return resp.str();
}

std::string dbg::handle_stack_trace(const dap::stack_trace_request &req)
{
    uint16_t pc = z80ex_get_reg(cpu_, regPC);
    nlohmann::json frame = {
        {"id", 1},
        {"name", format_hex(pc, 4)},
        {"line", 1},
        {"column", 1},
        {"source", {
                       {"name", "Disassembly"}, {"sourceReference", 1} // Unique int for disassembly, use same for all
                   }},
        {"instructionPointerReference", format_hex(pc, 4)}};

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"stackFrames", {frame}},
                               {"totalFrames", 1}});
    return resp.str();
}

std::string dbg::handle_scopes(const dap::scopes_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"scopes", {{{"name", "Registers"}, {"variablesReference", 100}, {"presentationHint", "registers"}, {"expensive", false}}}}});
    return resp.str();
}

std::string dbg::handle_variables(const dap::variables_request &req)
{
    nlohmann::json vars = nlohmann::json::array();

    if (req.variables_reference == 100)
    {
// Use Z80_REG_T enums from z80ex.h
#define Z80REG(name, regid, width)                                  \
    {                                                               \
        nlohmann::json v;                                           \
        v["name"] = #name;                                          \
        v["value"] = format_hex(z80ex_get_reg(cpu_, regid), width); \
        v["variablesReference"] = 0;                                \
        vars.push_back(v);                                          \
    }
        Z80REG(AF, regAF, 4)
        Z80REG(BC, regBC, 4)
        Z80REG(DE, regDE, 4)
        Z80REG(HL, regHL, 4)
        Z80REG(IX, regIX, 4)
        Z80REG(IY, regIY, 4)
        Z80REG(SP, regSP, 4)
        Z80REG(PC, regPC, 4)
        Z80REG(R, regR, 2)
        Z80REG(I, regI, 2)
        // F is the low byte of regAF
        {
            nlohmann::json v;
            v["name"] = "F";
            v["value"] = format_hex(z80ex_get_reg(cpu_, regAF) & 0xFF, 2);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
#undef Z80REG
    }

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"variables", vars}});
    return resp.str();
}

std::string dbg::handle_continue(const dap::continue_request &req)
{
    bool stopped = false;
    for (int step = 0; step < 100000; ++step)
    {
        uint16_t pc = z80ex_get_reg(cpu_, regPC);
        if (std::find(breakpoints_.begin(), breakpoints_.end(), static_cast<int>(pc + 1)) != breakpoints_.end())
        {
            stopped = true;
            break;
        }
        // Step CPU
        z80ex_step(cpu_);
        if (!launched_)
            break;
    }

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"allThreadsContinued", true}});

    if (stopped)
    {
        std::thread([this]()
                    {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            nlohmann::json j;
            j["seq"] = event_seq_++;
            j["type"] = "event";
            j["event"] = "stopped";
            j["body"] = {
                {"reason", "breakpoint"},
                {"threadId", 1},
                {"allThreadsStopped", true}
            };
            if (send_event_) send_event_(j.dump()); })
            .detach();
    }

    return resp.str();
}

// Callback to read a byte from your memory vector for disassembler
static uint8_t dasm_readbyte_cb(Z80EX_WORD addr, void *user_data)
{
    auto *memory = static_cast<std::vector<uint8_t> *>(user_data);
    if (addr < memory->size())
        return (*memory)[addr];
    else
        return 0xFF;
}

std::string dbg::handle_source(const nlohmann::json &req)
{
    std::ostringstream oss;
    uint16_t pc = z80ex_get_reg(cpu_, regPC);
    char dasm_buf[64];

    for (int i = 0; i < 256 && (pc + i) < memory_.size();)
    {
        uint16_t addr = pc + i;
        int ilen = z80ex_dasm(
            dasm_buf, sizeof(dasm_buf), 0, nullptr, nullptr,
            dasm_readbyte_cb, addr, &memory_);

        oss << std::setfill('0') << std::setw(4) << std::hex << addr << ": ";
        // Hex dump
        for (int j = 0; j < ilen && (addr + j) < memory_.size(); ++j)
            oss << std::setw(2) << std::setfill('0') << (int)memory_[addr + j] << " ";
        // Pad hex
        for (int j = ilen; j < 4; ++j)
            oss << "   ";
        oss << "  " << dasm_buf << "\n";

        i += (ilen > 0) ? ilen : 1;
    }

    dap::response resp(req["seq"], req["command"]);
    resp.success(true).result({{"content", oss.str()}});
    return resp.str();
}

std::string dbg::handle_disassemble(const nlohmann::json &req)
{
    std::string memref = req["memoryReference"];
    uint64_t address = std::stoull(memref, nullptr, 0); // supports "0x" prefix
    int instr_count = req.value("instructionCount", 10);

    nlohmann::json instructions = nlohmann::json::array();

    for (int i = 0; i < instr_count;)
    {
        char dasm_buf[64];
        int ilen = z80ex_dasm(
            dasm_buf, sizeof(dasm_buf), 0, nullptr, nullptr,
            dasm_readbyte_cb, address, &memory_);
        if (ilen <= 0)
            ilen = 1;
        std::string bytes;
        for (int j = 0; j < ilen; ++j)
        {
            char buf[4];
            snprintf(buf, 4, "%02X", memory_[address + j]);
            bytes += buf;
        }
        char addr_buf[16];
        snprintf(addr_buf, sizeof(addr_buf), "0x%04X", (unsigned)address);

        nlohmann::json instr = {
            {"address", addr_buf},
            {"instruction", dasm_buf},
            {"size", ilen},
            {"bytes", bytes}};
        instructions.push_back(instr);
        address += ilen;
        i++;
    }

    dap::response resp(req["seq"], req["command"]);
    resp.success(true).result({{"instructions", instructions}});
    return resp.str();
}

std::string dbg::handle_read_memory(const nlohmann::json &req)
{
    size_t addr = 0;
    std::istringstream iss(req["memoryReference"].get<std::string>());
    iss >> std::hex >> addr;
    int count = std::min(req.value("count", 1), (int)memory_.size() - (int)addr);

    std::ostringstream hexstr;
    for (int i = 0; i < count; ++i)
        hexstr << std::hex << std::setw(2) << std::setfill('0') << (int)memory_[addr + i];

    dap::response resp(req["seq"], req["command"]);
    resp.success(true).result({{"address", req["memoryReference"]},
                               {"data", hexstr.str()},
                               {"unreadableBytes", 0}});
    return resp.str();
}

void dbg::set_send_event(std::function<void(const std::string &)> fn)
{
    send_event_ = fn;
}

std::string dbg::format_hex(uint16_t value, int width)
{
    std::ostringstream oss;
    oss << std::uppercase << std::setfill('0') << std::setw(width)
        << std::hex << value;
    return "0x" + oss.str();
}