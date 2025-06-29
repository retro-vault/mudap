#include "dbg.h"
#include <dap/dap.h>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_socket.h>
#include <nlohmann/json.hpp>
#include <iostream>

constexpr int PORT = 4711;

std::string read_dap_message(sockpp::tcp_socket &sock)
{
    std::string header;
    char c;
    while (header.find("\r\n\r\n") == std::string::npos && sock.read(&c, 1) == 1)
        header += c;
    size_t pos = header.find("Content-Length:");
    if (pos == std::string::npos)
        return {};
    size_t len_start = pos + 15;
    size_t len_end = header.find("\r\n", len_start);
    int content_length = std::stoi(header.substr(len_start, len_end - len_start));
    std::string payload(content_length, '\0');
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
    std::string header = "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n";
    sock.write(header.c_str(), header.size());
    sock.write(json.c_str(), json.size());
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
        std::cout << "Client connected: " << peer.to_string() << std::endl;

        dap::dap dispatcher;
        dbg debug_instance;

        // Allow dbg to send DAP events
        debug_instance.set_send_event([&](const std::string &event_json)
                                      { send_dap_message(sock, event_json); });

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
        dispatcher.register_typed_handler<dap::disassemble_request>(
            "disassemble", [&](const dap::disassemble_request &req)
            { return debug_instance.handle_disassemble(req); });
        dispatcher.register_typed_handler<dap::set_instruction_breakpoints_request>(
            "setInstructionBreakpoints", [&](const dap::set_instruction_breakpoints_request &req)
            { return debug_instance.handle_set_instruction_breakpoints(req); });
        dispatcher.register_typed_handler<dap::next_request>(
            "next", [&](const dap::next_request &req)
            { return debug_instance.handle_next(req); });

        while (sock)
        {
            std::string req_json = read_dap_message(sock);
            if (req_json.empty())
                break;
            std::string resp_json = dispatcher.handle_message(req_json);
            if (!resp_json.empty())
            {
                send_dap_message(sock, resp_json);
            }
        }
        std::cout << "Client disconnected." << std::endl;
    }
    return 0;
}