#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_set_instruction_breakpoints(
    const dap::set_instruction_breakpoints_request &req)
{
    std::vector<uint16_t> new_bps;
    std::vector<nlohmann::json> bps;

    for (const auto &bp : req.breakpoints)
    {
        // Expecting string-formatted addresses like "0x1234"
        std::string ref = bp.value("instructionReference", "0");
        uint16_t addr = static_cast<uint16_t>(std::stoul(ref, nullptr, 0));
        new_bps.push_back(addr);

        bps.push_back({{"verified", true},
                       {"instructionReference", ref}});
    }

    instruction_breakpoints_ = new_bps;

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"breakpoints", bps}});
    return resp.str();
}