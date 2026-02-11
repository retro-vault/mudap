// threads.cpp — DAP "threads" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class threads_handler : public dap::request_handler {
public:
    threads_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "threads"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::threads_request::from(req);
        dap::response resp(r.seq, r.command);
        resp.success(true).result({{"threads", {{{"id", 1}, {"name", "Z80 Main"}}}}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_threads(dbg &ctx)
{
    return std::make_unique<threads_handler>(ctx);
}

} // namespace handlers
