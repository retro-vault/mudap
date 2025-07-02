#include "dap/dap.h"
#include <stdexcept>

namespace dap
{

    // Helper function to parse JSON safely.
    static json parse_json_safely(const std::string &json_text)
    {
        try
        {
            return json::parse(json_text);
        }
        catch (const std::exception &ex)
        {
            throw std::runtime_error(std::string("Failed to parse JSON: ") + ex.what());
        }
    }

    request request::parse(const std::string &json_text)
    {
        json j = parse_json_safely(json_text);
        request req;
        req.seq = j.value("seq", 0);
        req.type = j.value("type", "");
        req.command = j.value("command", "");
        req.arguments = j.value("arguments", json::object());
        return req;
    }

    // --- DAP request conversions ---

    initialize_request initialize_request::from(const request &req)
    {
        initialize_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        r.adapter_id = req.arguments.value("adapterID", "");
        r.client_id = req.arguments.value("clientID", "");
        r.client_name = req.arguments.value("clientName", "");
        r.locale = req.arguments.value("locale", "");
        return r;
    }

    launch_request launch_request::from(const request &req)
    {
        launch_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        r.no_debug = req.arguments.value("noDebug", false);
        r.program = req.arguments.value("program", "");
        return r;
    }

    attach_request attach_request::from(const request &req)
    {
        attach_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        // Add more attach-specific fields as needed
        return r;
    }

    set_breakpoints_request set_breakpoints_request::from(const request &req)
    {
        set_breakpoints_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        r.source = req.arguments.value("source", json::object());
        r.breakpoints = req.arguments.value("breakpoints", json::array());
        return r;
    }

    configuration_done_request configuration_done_request::from(const request &req)
    {
        configuration_done_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        return r;
    }

    threads_request threads_request::from(const request &req)
    {
        threads_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        return r;
    }

    stack_trace_request stack_trace_request::from(const request &req)
    {
        stack_trace_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        r.thread_id = req.arguments.value("threadId", 0);
        r.start_frame = req.arguments.value("startFrame", 0);
        r.levels = req.arguments.value("levels", 0);
        return r;
    }

    scopes_request scopes_request::from(const request &req)
    {
        scopes_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        r.frame_id = req.arguments.value("frameId", 0);
        return r;
    }

    variables_request variables_request::from(const request &req)
    {
        variables_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        r.variables_reference = req.arguments.value("variablesReference", 0);
        return r;
    }

    continue_request continue_request::from(const request &req)
    {
        continue_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;
        r.thread_id = req.arguments.value("threadId", 0);
        return r;
    }

    source_request source_request::from(const request &req)
    {
        source_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;

        if (req.arguments.contains("sourceReference"))
            r.source_reference = req.arguments["sourceReference"].get<int>();
        else if (req.arguments.contains("source") && req.arguments["source"].contains("sourceReference"))
            r.source_reference = req.arguments["source"]["sourceReference"].get<int>();

        return r;
    }

    read_memory_request read_memory_request::from(const request &req)
    {
        read_memory_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;

        r.memory_reference = req.arguments.value("memoryReference", 0);
        r.offset = req.arguments.value("offset", 0);
        r.count = req.arguments.value("count", 0);
        return r;
    }

    next_request next_request::next_request::from(const request &req)
    {
        next_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;

        if (req.arguments.contains("threadId"))
            r.thread_id = req.arguments["threadId"];
        return r;
    }

    step_in_request step_in_request::from(const request &req)
    {
        step_in_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;

        r.thread_id = req.arguments.value("threadId", 0);
        r.granularity = req.arguments.value("granularity", "");
        return r;
    }

    step_out_request step_out_request::from(const request &req)
    {
        step_out_request r;
        r.seq = req.seq;
        r.type = req.type;
        r.command = req.command;
        r.arguments = req.arguments;

        r.thread_id = req.arguments.value("threadId", 0);
        r.granularity = req.arguments.value("granularity", "");
        return r;
    }

} // namespace dap