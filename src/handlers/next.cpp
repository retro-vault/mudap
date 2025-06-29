#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_next(const dap::next_request &req)
{
    z80ex_step(cpu_); // Executes exactly one full instruction

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"allThreadsContinued", true}});

    std::thread([this]()
                {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        nlohmann::json j;
        j["seq"] = event_seq_++;
        j["type"] = "event";
        j["event"] = "stopped";
        j["body"] = {
            {"reason", "step"},
            {"threadId", 1},
            {"allThreadsStopped", true}};
        if (send_event_) send_event_(j.dump()); })
        .detach();

    return resp.str();
}
