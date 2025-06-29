#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_initialize(const dap::initialize_request &req)
{
    dap::response resp(req.seq, req.command);
    resp.success(true).result({
        {"supportsConfigurationDoneRequest", true},
        {"supportsSetVariable", false},
        {"supportsEvaluateForHovers", false},
        {"supportsReadMemoryRequest", true},
        {"supportsDisassembleRequest", true},
        {"supportsInstructionBreakpoints", true}, // Optional
        {"supportsSteppingGranularity", true}     // Optional
    });
    return resp.str();
}