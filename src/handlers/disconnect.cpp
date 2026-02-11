// disconnect.cpp — DAP "disconnect" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class disconnect_handler : public dap::request_handler {
public:
    disconnect_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "disconnect"; }

    std::string handle(const dap::request &req) override
    {
        ctx_.set_launched(false);

        dap::response resp(req.seq, req.command);
        return resp.success(true).result({}).str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_disconnect(dbg &ctx)
{
    return std::make_unique<disconnect_handler>(ctx);
}

} // namespace handlers
