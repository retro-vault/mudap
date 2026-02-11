// source.cpp — DAP "source" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class source_handler : public dap::request_handler {
public:
    source_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "source"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::source_request::from(req);

        // sourceReference 0 means "use the path on disk".
        // Any non-zero sourceReference is treated as the virtual disassembly.
        if (r.source_reference == 0)
        {
            std::string path;
            if (req.arguments.contains("source") &&
                req.arguments["source"].contains("path"))
                path = req.arguments["source"]["path"].get<std::string>();

            if (!path.empty())
            {
                std::ifstream ifs(path);
                if (ifs)
                {
                    std::ostringstream content;
                    content << ifs.rdbuf();
                    dap::response resp(r.seq, r.command);
                    resp.success(true)
                        .result({{"content", content.str()},
                                 {"mimeType", "text/x-c"}});
                    return resp.str();
                }
            }

            dap::response resp(r.seq, r.command);
            resp.success(false).message("Unknown sourceReference");
            return resp.str();
        }

        std::ostringstream oss;
        char dasm_buf[64];
        uint32_t addr = z80ex_get_reg(ctx_.cpu(), regPC);

        auto &mem = ctx_.memory();
        for (int i = 0; i < 256 && addr < mem.size();)
        {
            int ts1 = 0, ts2 = 0;
            int ilen = z80ex_dasm(
                dasm_buf, sizeof(dasm_buf), 0, &ts1, &ts2,
                dbg::dasm_readbyte_cb, addr, &mem);

            oss << "      " << std::uppercase << std::setfill('0')
                << std::setw(6) << std::hex << addr << " ";

            int opcode_chars = 0;
            for (int j = 0; j < ilen && (addr + j) < mem.size(); ++j)
            {
                oss << std::setw(2) << std::setfill('0') << std::hex
                    << (int)mem[addr + j] << " ";
                opcode_chars += 3;
            }
            for (; opcode_chars < 8; ++opcode_chars)
                oss << " ";

            oss << std::string(26 - (6 + 6 + 1 + opcode_chars), ' ');
            oss << "[" << std::right << std::setw(2) << std::dec << ts1 << "]";
            oss << "   ";
            oss << dasm_buf << "\n";

            addr += (ilen > 0) ? ilen : 1;
            ++i;
        }

        dap::response resp(r.seq, r.command);
        resp.success(true)
            .result({{"content", oss.str()},
                     {"mimeType", "text/x-asm"}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_source(dbg &ctx)
{
    return std::make_unique<source_handler>(ctx);
}

} // namespace handlers
