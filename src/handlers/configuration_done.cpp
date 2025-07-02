#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_configuration_done(const dap::configuration_done_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({});

    return resp.str();
}