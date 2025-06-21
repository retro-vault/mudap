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
#include <cstdint>

#include <sockpp/tcp_acceptor.h>

int main()
{

    int16_t port = 1337;

    sockpp::tcp_acceptor acc(port);

    if (!acc)
    {
        std::cerr << acc.last_error_str();
        return 1;
    }

    while (true)
    {
        // Accept a new client connection
        sockpp::tcp_socket sock = acc.accept();

        if (!sock)
        {
            std::cerr << "Error accepting incoming connection: "
                      << acc.last_error_str() << std::endl;
        }
        else
        {
            // Main loop (just one connection to GDB at one time)
            ssize_t n;
            char buf[512];
            while ((n = sock.read(buf, sizeof(buf))) > 0)
            {
                buf[n] = 0;
                std::cout << buf << std::endl;
            }
        }
    }
}