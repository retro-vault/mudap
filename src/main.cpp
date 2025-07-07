// main.cpp
// DAP TCP server main loop using the dap::dap stream interface.
//
// This file launches a DAP server using sockpp for TCP communication,
// and delegates all DAP message handling to the dap::dap class.
// The server handles requests for a Z80 CPU emulated using z80ex.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.

#include <iostream>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_socket.h>

#include <dap/dap.h>
#include <dbg.h>

constexpr int PORT = 4711;

int main()
{
    sockpp::initialize();
    sockpp::tcp_acceptor acc(PORT);
    if (!acc)
    {
        std::cerr << "Error creating the acceptor: " << acc.last_error_str() << std::endl;
        return 1;
    }

    std::cout << "DAP server listening on port " << PORT << std::endl;

    while (true)
    {
        sockpp::inet_address peer;
        sockpp::tcp_socket sock = acc.accept(&peer);
        if (!sock)
        {
            std::cerr << "Error accepting connection: " << acc.last_error_str() << std::endl;
            continue;
        }

        std::cout << "\n--- Client connected: " << peer.to_string() << " ---\n";

        std::istream in = sock.get_input_stream();
        std::ostream out = sock.get_output_stream();

        dap::dap dispatcher;
        dbg debug_instance;

        // Hook the debugger's event sending mechanism.
        dispatcher.set_event_sender([&](const std::string &event_json)
        {
            out << "Content-Length: " << event_json.size() << "\r\n\r\n"
                << event_json << std::flush;
        });

        // Register all typed DAP request handlers.
        dispatcher.register_typed_handler<dap::initialize_request>(
            "initialize", [&](const dap::initialize_request &req)
            { return debug_instance.handle_initialize(req); });

        dispatcher.register_typed_handler<dap::launch_request>(
            "launch", [&](const dap::launch_request &req)
            { return debug_instance.handle_launch(req); });

        dispatcher.register_typed_handler<dap::set_breakpoints_request>(
            "setBreakpoints", [&](const dap::set_breakpoints_request &req)
            { return debug_instance.handle_set_breakpoints(req); });

        dispatcher.register_typed_handler<dap::configuration_done_request>(
            "configurationDone", [&](const dap::configuration_done_request &req)
            { return debug_instance.handle_configuration_done(req); });

        dispatcher.register_typed_handler<dap::threads_request>(
            "threads", [&](const dap::threads_request &req)
            { return debug_instance.handle_threads(req); });

        dispatcher.register_typed_handler<dap::stack_trace_request>(
            "stackTrace", [&](const dap::stack_trace_request &req)
            { return debug_instance.handle_stack_trace(req); });

        dispatcher.register_typed_handler<dap::scopes_request>(
            "scopes", [&](const dap::scopes_request &req)
            { return debug_instance.handle_scopes(req); });

        dispatcher.register_typed_handler<dap::variables_request>(
            "variables", [&](const dap::variables_request &req)
            { return debug_instance.handle_variables(req); });

        dispatcher.register_typed_handler<dap::continue_request>(
            "continue", [&](const dap::continue_request &req)
            { return debug_instance.handle_continue(req); });

        dispatcher.register_typed_handler<dap::source_request>(
            "source", [&](const dap::source_request &req)
            { return debug_instance.handle_source(req); });

        dispatcher.register_typed_handler<dap::read_memory_request>(
            "readMemory", [&](const dap::read_memory_request &req)
            { return debug_instance.handle_read_memory(req); });

        dispatcher.register_typed_handler<dap::next_request>(
            "next", [&](const dap::next_request &req)
            { return debug_instance.handle_next(req); });

        dispatcher.register_typed_handler<dap::step_in_request>(
            "stepIn", [&](const dap::step_in_request &req)
            { return debug_instance.handle_step_in(req); });

        dispatcher.register_typed_handler<dap::step_out_request>(
            "stepOut", [&](const dap::step_out_request &req)
            { return debug_instance.handle_step_out(req); });

        dispatcher.register_typed_handler<dap::set_instruction_breakpoints_request>(
            "setInstructionBreakpoints", [&](const dap::set_instruction_breakpoints_request &req)
            { return debug_instance.handle_set_instruction_breakpoints(req); });

        // Run the DAP server on the TCP stream.
        dispatcher.run(in, out);

        std::cout << "--- Client disconnected ---\n";
    }

    return 0;
}
