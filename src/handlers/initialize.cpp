// initialize.cpp — DAP "initialize" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class initialize_handler : public dap::request_handler {
public:
    initialize_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "initialize"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::initialize_request::from(req);

        // Send the "initialized" event asynchronously.
        nlohmann::json ev;
        ev["type"] = "event";
        ev["seq"] = ctx_.next_event_seq();
        ev["event"] = "initialized";
        ev["body"] = nlohmann::json::object();
        ctx_.send_event(ev.dump());

        // Return the capabilities response.
        dap::response resp(r.seq, r.command);
        return resp
            .success(true)
            .result({{"supportsConfigurationDoneRequest", true},
                     {"supportsBreakpointLocationsRequest", true},
                     {"supportsInstructionBreakpoints", true},
                     {"supportsLoadedSourcesRequest", true},
                     {"supportsStepBack", false},
                     {"supportsRestartFrame", false},
                     {"supportsEvaluateForHovers", false},
                     {"supportsSetVariable", false},
                     {"supportsTerminateDebuggee", false},
                     {"supportsMemoryReferences", true}})
            .str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_initialize(dbg &ctx)
{
    return std::make_unique<initialize_handler>(ctx);
}

} // namespace handlers
