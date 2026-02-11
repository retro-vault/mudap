// configuration_done.cpp — DAP "configurationDone" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class configuration_done_handler : public dap::request_handler {
public:
    configuration_done_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "configurationDone"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::configuration_done_request::from(req);
        dap::response resp(r.seq, r.command);
        resp.success(true).result({});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_configuration_done(dbg &ctx)
{
    return std::make_unique<configuration_done_handler>(ctx);
}

} // namespace handlers
