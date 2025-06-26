#include "dbg.h"
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cstring>

dbg::dbg()
    : cpu_(), memory_(0x10000, 0), breakpoints_(), event_seq_(1), launched_(false)
{
}

std::string dbg::handle_initialize(const dap::initialize_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"supportsConfigurationDoneRequest", true},
                               {"supportsSetVariable", false},
                               {"supportsEvaluateForHovers", false},
                               {"supportsReadMemoryRequest", true}});
    return resp.str();
}

std::string dbg::handle_launch(const dap::launch_request &req)
{
    // Manually zero all registers and wtc structs to "reset" the CPU
    std::memset(&cpu_.reg, 0, sizeof(cpu_.reg));
    std::memset(&cpu_.wtc, 0, sizeof(cpu_.wtc));

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
    uint16_t pc = cpu_.reg.PC;
    nlohmann::json frame = {
        {"id", 1},
        {"name", "PC"},
        {"line", static_cast<int>(pc + 1)},
        {"column", 1},
        {"source", {{"name", "dummy.bin"}, {"sourceReference", 1}}}};

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
        auto &reg = cpu_.reg;
        {
            nlohmann::json v;
            v["name"] = "AF";
            v["value"] = format_hex((reg.pair.A << 8) | reg.pair.F, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "BC";
            v["value"] = format_hex((reg.pair.B << 8) | reg.pair.C, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "DE";
            v["value"] = format_hex((reg.pair.D << 8) | reg.pair.E, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "HL";
            v["value"] = format_hex((reg.pair.H << 8) | reg.pair.L, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "IX";
            v["value"] = format_hex(reg.IX, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "IY";
            v["value"] = format_hex(reg.IY, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "SP";
            v["value"] = format_hex(reg.SP, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "PC";
            v["value"] = format_hex(reg.PC, 4);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "R";
            v["value"] = format_hex(reg.R, 2);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "I";
            v["value"] = format_hex(reg.I, 2);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
        {
            nlohmann::json v;
            v["name"] = "F";
            v["value"] = format_hex(reg.pair.F, 2);
            v["variablesReference"] = 0;
            vars.push_back(v);
        }
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
        uint16_t pc = cpu_.reg.PC;
        if (std::find(breakpoints_.begin(), breakpoints_.end(), static_cast<int>(pc + 1)) != breakpoints_.end())
        {
            stopped = true;
            break;
        }
        // Suzuki Z80 has no single-step public method; you must implement stepping here if needed.
        // For now, just increment PC as a placeholder for stepping:
        cpu_.reg.PC += 1;
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

std::string dbg::handle_source(const nlohmann::json &req)
{
    // Disassemble 256 bytes from PC for dummy.bin
    std::ostringstream oss;
    uint16_t pc = cpu_.reg.PC;
    for (int i = 0; i < 256 && (pc + i) < memory_.size();)
    {
        uint16_t addr = pc + i;
        oss << std::setfill('0') << std::setw(4) << std::hex << addr << ": ";
        // You might want to implement actual disassembly here.
        oss << "??" << "\n";
        i += 1; // Placeholder: advance by 1
    }

    dap::response resp(req["seq"], req["command"]);
    resp.success(true).result({{"content", oss.str()}});
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