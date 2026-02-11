// set_breakpoints.cpp — DAP "setBreakpoints" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class set_breakpoints_handler : public dap::request_handler {
public:
    set_breakpoints_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "setBreakpoints"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::set_breakpoints_request::from(req);

        ctx_.breakpoints().clear();

        // Get the source file from the request.
        std::string source_path;
        if (r.arguments.contains("source"))
        {
            auto &src = r.arguments["source"];
            if (src.contains("path"))
                source_path = src["path"].get<std::string>();
            else if (src.contains("name"))
                source_path = src["name"].get<std::string>();
        }

        std::vector<nlohmann::json> bps;
        if (r.arguments.contains("breakpoints"))
        {
            for (const auto &bp : r.arguments["breakpoints"])
            {
                int line = bp.value("line", 1);
                auto addr = ctx_.lookup_address(source_path, line);
                if (addr)
                {
                    ctx_.breakpoints().push_back(*addr);
                    std::cerr << "[setBreakpoints] " << source_path
                              << ":" << line << " -> 0x"
                              << std::hex << *addr << std::dec << std::endl;
                    bps.push_back({{"verified", true}, {"line", line}});
                }
                else
                {
                    std::cerr << "[setBreakpoints] " << source_path
                              << ":" << line << " -> NOT FOUND" << std::endl;
                    bps.push_back({{"verified", false},
                                   {"line", line},
                                   {"message", "No address mapping for this line"}});
                }
            }
        }

        dap::response resp(r.seq, r.command);
        resp.success(true).result({{"breakpoints", bps}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_set_breakpoints(dbg &ctx)
{
    return std::make_unique<set_breakpoints_handler>(ctx);
}

} // namespace handlers
