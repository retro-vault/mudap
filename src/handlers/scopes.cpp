#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_scopes(const dap::scopes_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result(
        {{"scopes",
          {{{"name", "Registers"}, {"variablesReference", 100}, {"presentationHint", "registers"}, {"expensive", false}}}}});
    return resp.str();
}