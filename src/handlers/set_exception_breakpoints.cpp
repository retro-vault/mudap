// set_exception_breakpoints.cpp — DAP "setExceptionBreakpoints" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class set_exception_breakpoints_handler : public dap::request_handler {
public:
    set_exception_breakpoints_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "setExceptionBreakpoints"; }

    std::string handle(const dap::request &req) override
    {
        dap::response resp(req.seq, req.command);
        return resp.success(true).result({}).str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_set_exception_breakpoints(dbg &ctx)
{
    return std::make_unique<set_exception_breakpoints_handler>(ctx);
}

} // namespace handlers
