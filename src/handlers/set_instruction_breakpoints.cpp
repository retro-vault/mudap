// set_instruction_breakpoints.cpp — DAP "setInstructionBreakpoints" handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class set_instruction_breakpoints_handler : public dap::request_handler {
public:
    set_instruction_breakpoints_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "setInstructionBreakpoints"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::set_instruction_breakpoints_request::from(req);

        ctx_.instruction_breakpoints().clear();
        for (const auto &bp : r.breakpoints)
        {
            if (bp.contains("instructionReference"))
            {
                std::string addr_str = bp["instructionReference"];
                uint16_t addr = std::stoul(addr_str, nullptr, 16);
                ctx_.instruction_breakpoints().push_back(addr);
            }
        }

        std::vector<nlohmann::json> breakpoints;
        for (uint16_t addr : ctx_.instruction_breakpoints())
        {
            breakpoints.push_back({{"verified", true},
                                   {"instructionReference", ctx_.format_hex(addr, 4)}});
        }

        dap::response resp(r.seq, r.command);
        resp.success(true).result({{"breakpoints", breakpoints}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_set_instruction_breakpoints(dbg &ctx)
{
    return std::make_unique<set_instruction_breakpoints_handler>(ctx);
}

} // namespace handlers
