// step_out.cpp — DAP "stepOut" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class step_out_handler : public dap::request_handler {
public:
    step_out_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "stepOut"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::step_out_request::from(req);
        z80ex_step(ctx_.cpu());

        dap::response resp(r.seq, r.command);
        resp.success(true);

        nlohmann::json j{
            {"seq", ctx_.next_event_seq()},
            {"type", "event"},
            {"event", "stopped"},
            {"body", {{"reason", "step"}, {"threadId", 1}, {"allThreadsStopped", true}}}};
        ctx_.send_event(j.dump());

        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_step_out(dbg &ctx)
{
    return std::make_unique<step_out_handler>(ctx);
}

} // namespace handlers
