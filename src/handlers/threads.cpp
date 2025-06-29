#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_threads(const dap::threads_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"threads", {{{"id", 1}, {"name", "Z80 Main"}}}}});
    return resp.str();
}