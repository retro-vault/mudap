// launch.cpp — DAP "launch" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <sdcc/cdb_parser.h>
#include <dbg.h>

namespace {

// Parse an Intel HEX (.ihx/.hex) stream into a flat memory buffer.
// Returns the lowest data address (entry point) or 0 if no data was loaded.
uint16_t load_ihx(std::istream &in, std::vector<uint8_t> &mem)
{
    uint32_t entry = 0xFFFFFFFF;
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty() || line[0] != ':')
            continue;

        auto hex2 = [&](size_t pos) -> uint8_t {
            return static_cast<uint8_t>(std::stoul(line.substr(pos, 2), nullptr, 16));
        };

        uint8_t byte_count = hex2(1);
        uint16_t address   = (hex2(3) << 8) | hex2(5);
        uint8_t rec_type   = hex2(7);

        if (rec_type == 0x01)       // EOF
            break;
        if (rec_type != 0x00)       // only handle data records
            continue;

        if (byte_count > 0 && address < entry)
            entry = address;

        for (uint8_t i = 0; i < byte_count; ++i)
        {
            uint8_t byte = hex2(9 + i * 2);
            size_t  dest = static_cast<size_t>(address) + i;
            if (dest < mem.size())
                mem[dest] = byte;
        }
    }
    return (entry == 0xFFFFFFFF) ? 0 : static_cast<uint16_t>(entry);
}

} // anonymous namespace

namespace handlers {

class launch_handler : public dap::request_handler {
public:
    launch_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "launch"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::launch_request::from(req);

        z80ex_reset(ctx_.cpu());
        std::fill(ctx_.memory().begin(), ctx_.memory().end(), 0);
        ctx_.set_virtual_lst_source_reference(1);

        if (r.arguments.contains("program"))
        {
            std::string bin_path = r.arguments["program"];
            std::string ext;
            try { ext = std::filesystem::path(bin_path).extension().string(); }
            catch (...) {}

            std::ifstream bin_file(bin_path, (ext == ".ihx" || ext == ".hex")
                                             ? std::ios::in : std::ios::binary);
            if (bin_file)
            {
                if (ext == ".ihx" || ext == ".hex")
                {
                    uint16_t entry = load_ihx(bin_file, ctx_.memory());
                    z80ex_set_reg(ctx_.cpu(), regPC, entry);
                    z80ex_set_reg(ctx_.cpu(), regSP, 0x0000);
                    std::cerr << "[launch] Entry point: 0x"
                              << std::hex << entry << std::dec << std::endl;
                }
                else
                {
                    bin_file.read(reinterpret_cast<char *>(ctx_.memory().data()),
                                  ctx_.memory().size());
                }
                std::cerr << "[launch] Loaded program: " << bin_path << std::endl;
            }
            else
            {
                std::cerr << "[launch] ERROR: Cannot open program file: " << bin_path << std::endl;
            }

            // Try to load the companion .cdb file for C source mapping.
            namespace fs = std::filesystem;
            fs::path cdb_path = fs::path(bin_path).replace_extension(".cdb");
            if (fs::exists(cdb_path))
            {
                sdcc::cdb_parser parser;
                auto modules = parser.parse(cdb_path.string());
                if (modules)
                {
                    std::cerr << "[launch] Loaded CDB: " << cdb_path.string()
                              << " (" << modules->size() << " modules";
                    size_t total_lines = 0;
                    for (auto &m : *modules) total_lines += m.lines.size();
                    std::cerr << ", " << total_lines << " line mappings)" << std::endl;
                    ctx_.set_cdb_modules(std::move(*modules));
                }
                else
                    std::cerr << "[launch] WARNING: Failed to parse CDB: " << cdb_path.string() << std::endl;
            }
            else
                std::cerr << "[launch] No CDB file found at: " << cdb_path.string() << std::endl;

            // Determine source root for resolving relative paths in CDB.
            if (r.arguments.contains("sourceRoot"))
                ctx_.set_source_root(r.arguments["sourceRoot"].get<std::string>());
            else
                ctx_.set_source_root(fs::path(bin_path).parent_path().string());

            std::string base;
            try { base = fs::path(bin_path).stem().string(); }
            catch (...) { base = "listing"; }
            ctx_.set_virtual_lst_path("/__virtual__/" + base + ".asm");
        }
        else
        {
            ctx_.set_virtual_lst_path("/__virtual__/listing.asm");
        }

        ctx_.set_launched(true);

        dap::response resp(r.seq, r.command);
        resp.success(true).result({});

        nlohmann::json stopped_event;
        stopped_event["seq"] = ctx_.next_event_seq();
        stopped_event["type"] = "event";
        stopped_event["event"] = "stopped";
        stopped_event["body"] = {
            {"reason", "entry"},
            {"threadId", 1},
            {"allThreadsStopped", true}};
        ctx_.send_event(stopped_event.dump());

        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_launch(dbg &ctx)
{
    return std::make_unique<launch_handler>(ctx);
}

} // namespace handlers
