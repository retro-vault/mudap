// socket_stream.h
// Custom iostream interface for sockpp TCP sockets to support DAP protocol.
//
// This file defines the socket_stream_buffer and socket_stream classes, which
// wrap a sockpp::tcp_socket into a std::iostream-compatible interface.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#pragma once

#include <streambuf>
#include <iostream>
#include <memory>

#include <sockpp/tcp_socket.h>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_connector.h>
#include <sockpp/result.h>

class socket_stream_buffer : public std::streambuf {
public:
    socket_stream_buffer(sockpp::tcp_socket& socket);
protected:
    int underflow() override;
    int overflow(int c) override;
    int sync() override;
private:
    sockpp::tcp_socket& socket_;
    static constexpr size_t buffer_size = 1024;
    char in_buffer_[buffer_size];
    char out_buffer_[buffer_size];
};

class socket_stream : public std::iostream {
public:
    // Constructor for server mode (listen on port)
    socket_stream(uint16_t port);
    // Constructor for client mode (connect to host:port)
    socket_stream(const std::string& host, uint16_t port);
    // Destructor to clean up socket
    ~socket_stream();
private:
    socket_stream_buffer buffer_;
    std::unique_ptr<sockpp::tcp_socket> socket_; // Owned socket
};