#include <dap/dap.h>

#include <dbg.h>


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