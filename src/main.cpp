// main.cpp
// DAP TCP server main loop.
//
// This file launches the TCP server for the Debug Adapter Protocol (DAP),
// using sockpp for socket communication. It reads and dispatches incoming
// DAP requests to the `dbg` instance, handling various debug operations
// for a Z80 CPU emulated using z80ex.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <iostream>

#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_socket.h>
#include <nlohmann/json.hpp>

#include <dap/dap.h>

#include <dbg.h>

constexpr int PORT = 4711;

std::string read_dap_message(sockpp::tcp_socket &sock)
{
    std::string header;                 // Message hdr.

    // Skip newlines before header.
    char c;
    while (header.find("\r\n\r\n") == 
        std::string::npos && sock.read(&c, 1) == 1)
        header += c;
    
    // The emssage should start with Content-Length:
    size_t pos = header.find("Content-Length:");
    if (pos == std::string::npos)
        return {};
    
    // Read length.
    size_t len_start = pos + 15;
    size_t len_end = header.find("\r\n", len_start);
    int content_length = std::stoi(
        header.substr(len_start, len_end - len_start));
    std::string payload(content_length, '\0');

    // Now read all characters after content length.
    int received = 0;
    while (received < content_length)
    {
        ssize_t r = sock.read(&payload[received], content_length - received);
        if (r <= 0)
            break;
        received += r;
    }
    return payload;
}

void send_dap_message(sockpp::tcp_socket &sock, const std::string &json)
{
    std::string body = json.c_str();
    std::string header = "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    sock.write(header.c_str(), header.size());
    sock.write(body.c_str(), body.size());
}

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

        dap::dap dispatcher;
        dbg debug_instance;

        debug_instance.set_send_event([&](const std::string &event_json)
                                      {
            std::cout << "[SEND EVENT] " << event_json << "\n";
            send_dap_message(sock, event_json); });

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

        while (sock)
        {
            std::string req_json = read_dap_message(sock);
            if (req_json.empty())
                break;

            std::cout << "\n[RECEIVED] " << req_json << "\n";

            std::string resp_json = dispatcher.handle_message(req_json);
            if (!resp_json.empty())
            {
                std::cout << "[RESPONSE] " << resp_json << "\n";
                send_dap_message(sock, resp_json);
            }
        }

        std::cout << "--- Client disconnected ---\n";
    }

    return 0;
}
