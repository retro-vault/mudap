#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_continue(const dap::continue_request &req)
{
    bool stopped = false;
    for (int step = 0; step < 100000; ++step)
    {
        uint16_t pc = z80ex_get_reg(cpu_, regPC);
        if (std::find(breakpoints_.begin(), breakpoints_.end(), static_cast<int>(pc + 1)) != breakpoints_.end())
        {
            stopped = true;
            break;
        }
        // Step CPU
        z80ex_step(cpu_);
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