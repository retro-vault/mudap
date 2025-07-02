#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_step_in(const dap::step_in_request &req)
{
    // Just do one step for now
    z80ex_step(cpu_);
    dap::response resp(req.seq, req.command);
    resp.success(true);
    // Optionally emit a stopped event
    if (send_event_)
    {
        nlohmann::json j{
            {"seq", event_seq_++},
            {"type", "event"},
            {"event", "stopped"},
            {"body", {{"reason", "step"}, {"threadId", 1}, {"allThreadsStopped", true}}}};
        send_event_(j.dump());
    }
    return resp.str();
}