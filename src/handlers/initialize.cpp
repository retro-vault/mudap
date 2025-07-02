#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_initialize(const dap::initialize_request &req)
{
    // Send initialize response
    dap::response resp(req.seq, req.command);
    std::string response =
        resp
            .success(true)
            .result({{"supportsConfigurationDoneRequest", true},
                     {"supportsDisassembleRequest", true},
                     {"supportsInstructionBreakpoints", true},
                     {"supportsStepBack", false},
                     {"supportsRestartFrame", false},
                     {"supportsEvaluateForHovers", false},
                     {"supportsSetVariable", false},
                     {"supportsTerminateDebuggee", false},
                     {"supportsMemoryReferences", true}})
            .str();

    if (send_event_)
        send_event_(response);

    // Now create the "initialized" event using event_seq_
    nlohmann::json ev;
    ev["type"] = "event";
    ev["seq"] = event_seq_++;
    ev["event"] = "initialized";
    ev["body"] = nlohmann::json::object();

    return ev.dump(); // Sent by caller with Content-Length wrapping
}
