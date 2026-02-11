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
#include <socket_stream.h>

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

        socket_stream_buffer buf(sock);
        std::istream in(&buf);
        std::ostream out(&buf);

        dap::dap dispatcher;
        dbg debug_instance;

        // Wire up event sending: dbg -> socket.
        auto event_sender = [&](const std::string &event_json)
        {
            out << "Content-Length: " << event_json.size() << "\r\n\r\n"
                << event_json << std::flush;
        };
        dispatcher.set_event_sender(event_sender);
        debug_instance.set_event_sender(event_sender);

        // Register all handler objects (chain of responsibility).
        debug_instance.register_handlers(dispatcher);

        // Run the DAP server on the TCP stream.
        dispatcher.run(in, out);

        std::cout << "--- Client disconnected ---\n";
    }

    return 0;
}
