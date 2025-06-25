/*
 * main.cpp
 *
 * Iskra Delta Partner Emulator main.
 *
 * MIT License (see: LICENSE)
 * Copyright (c) 2023  Your Company
 *
 * 27.09.2023  tstih
 *
 */
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <dap/dap.h>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_socket.h>

constexpr int PORT = 4711;

// Reads a DAP message from the client (Content-Length framing)
std::string read_dap_message(sockpp::tcp_socket &sock)
{
    std::string header;
    char c;
    // Read header (until \r\n\r\n)
    while (header.find("\r\n\r\n") == std::string::npos && sock.read(&c, 1) == 1)
    {
        header += c;
    }
    // Parse Content-Length
    size_t pos = header.find("Content-Length:");
    if (pos == std::string::npos)
        return {};
    size_t len_start = pos + 15;
    size_t len_end = header.find("\r\n", len_start);
    int content_length = std::stoi(header.substr(len_start, len_end - len_start));
    // Read payload
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

// Helper: send a DAP event (not a response)
void send_dap_event(sockpp::tcp_socket &sock, int &event_seq, const std::string &event, const nlohmann::json &body = nlohmann::json::object())
{
    nlohmann::json j;
    j["seq"] = event_seq++;
    j["type"] = "event";
    j["event"] = event;
    if (!body.is_null() && !body.empty())
        j["body"] = body;
    send_dap_message(sock, j.dump());
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
        int event_seq = 1;
        bool sent_initialized = false;
        bool launched = false;

        // Handler: initialize
        dispatcher.register_typed_handler<dap::initialize_request>(
            "initialize",
            [&](const dap::initialize_request &req) -> std::string
            {
                // Send 'initialized' event *after* response per DAP spec
                dap::response resp(req.seq, req.command);
                resp.success(true).result({{"supportsConfigurationDoneRequest", true},
                                           {"supportsSetVariable", false},
                                           {"supportsEvaluateForHovers", false}});
                // Send response
                auto resp_str = resp.str();

                // Send initialized event after response
                std::thread([&sock, &event_seq]()
                            {
                    // Small sleep ensures VSCode gets the response first
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    send_dap_event(sock, event_seq, "initialized"); })
                    .detach();

                sent_initialized = true;
                return resp_str;
            });

        // Handler: launch
        dispatcher.register_typed_handler<dap::launch_request>(
            "launch",
            [&](const dap::launch_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                resp.success(true).result({});
                // After launch, send a stopped event to simulate hitting entry point
                std::thread([&sock, &event_seq]()
                            {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    send_dap_event(sock, event_seq, "stopped", {
                        {"reason", "entry"},
                        {"threadId", 1},
                        {"allThreadsStopped", true}
                    }); })
                    .detach();
                launched = true;
                return resp.str();
            });

        // Handler: setBreakpoints
        dispatcher.register_typed_handler<dap::set_breakpoints_request>(
            "setBreakpoints",
            [&](const dap::set_breakpoints_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                // Echo breakpoints as verified (dummy)
                std::vector<nlohmann::json> bps;
                if (req.breakpoints.is_array())
                {
                    for (const auto &bp : req.breakpoints)
                    {
                        bps.push_back({{"verified", true},
                                       {"line", bp.value("line", 1)}});
                    }
                }
                resp.success(true).result({{"breakpoints", bps}});
                return resp.str();
            });

        // Handler: configurationDone (required by VSCode)
        dispatcher.register_typed_handler<dap::configuration_done_request>(
            "configurationDone",
            [&](const dap::configuration_done_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                resp.success(true).result({});
                return resp.str();
            });

        // Handler: threads (required by VSCode)
        dispatcher.register_typed_handler<dap::threads_request>(
            "threads",
            [&](const dap::threads_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                resp.success(true).result({{"threads", {{{"id", 1}, {"name", "MainThread"}}}}});
                return resp.str();
            });

        // Handler: stackTrace (required by VSCode)
        dispatcher.register_typed_handler<dap::stack_trace_request>(
            "stackTrace",
            [&](const dap::stack_trace_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                resp.success(true).result({{"stackFrames", {{{"id", 1}, {"name", "main"}, {"line", 1}, {"column", 1}, {"source", {{"name", "dummy.bin"}}}}}},
                                           {"totalFrames", 1}});
                return resp.str();
            });

        // Handler: scopes
        dispatcher.register_typed_handler<dap::scopes_request>(
            "scopes",
            [&](const dap::scopes_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                resp.success(true).result({{"scopes", {{{"name", "Locals"}, {"variablesReference", 1}, {"presentationHint", "locals"}, {"expensive", false}}}}});
                return resp.str();
            });

        // Handler: variables
        dispatcher.register_typed_handler<dap::variables_request>(
            "variables",
            [&](const dap::variables_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                resp.success(true).result({{"variables", {{{"name", "dummy"}, {"value", "42"}, {"variablesReference", 0}}}}});
                return resp.str();
            });

        // Handler: continue
        dispatcher.register_typed_handler<dap::continue_request>(
            "continue",
            [&](const dap::continue_request &req) -> std::string
            {
                dap::response resp(req.seq, req.command);
                resp.success(true).result({{"allThreadsContinued", true}});
                // Simulate hitting a breakpoint after continuing
                std::thread([&sock, &event_seq]()
                            {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    send_dap_event(sock, event_seq, "stopped", {
                        {"reason", "breakpoint"},
                        {"threadId", 1},
                        {"allThreadsStopped", true}
                    }); })
                    .detach();
                return resp.str();
            });

        // Main request/response loop
        while (sock)
        {
            std::string req_json = read_dap_message(sock);
            if (req_json.empty())
                break; // Connection closed or error

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