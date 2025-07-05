#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_set_instruction_breakpoints(const dap::set_instruction_breakpoints_request &req)
{
    instruction_breakpoints_.clear(); // clear your internal list

    // Parse instructionReference strings
    for (const auto &bp : req.breakpoints)
    {
        if (bp.contains("instructionReference"))
        {
            std::string addr_str = bp["instructionReference"];
            uint16_t addr = std::stoul(addr_str, nullptr, 16);
            instruction_breakpoints_.push_back(addr);
        }
    }

    // Report breakpoints back to the client
    std::vector<nlohmann::json> breakpoints;
    for (uint16_t addr : instruction_breakpoints_)
    {
        breakpoints.push_back({{"verified", true},
                               {"instructionReference", format_hex(addr, 4)}});
    }

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"breakpoints", breakpoints}});
    return resp.str();
}
