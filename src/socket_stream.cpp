// socket_stream.cpp
// Implementation of iostream interface for sockpp TCP sockets.
//
// This file provides the implementation of socket_stream_buffer and
// socket_stream classes, enabling sockpp::tcp_socket to be used as a
// std::iostream. 
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <stdexcept>

socket_stream_buffer::socket_stream_buffer(sockpp::tcp_socket& socket)
    : socket_(socket) {
    setg(in_buffer_, in_buffer_, in_buffer_);
    setp(out_buffer_, out_buffer_ + buffer_size);
}

int socket_stream_buffer::underflow() {
    // Return buffered data if available
    if (gptr() < egptr()) {
        return traits_type::to_int_type(*gptr());
    }

    // Read from socket into input buffer
    auto res = socket_.read(in_buffer_, buffer_size);
    if (!res || res.value() <= 0) {
        return traits_type::eof(); // EOF on error or disconnection
    }

    setg(in_buffer_, in_buffer_, in_buffer_ + res.value());
    return traits_type::to_int_type(*gptr());
}

int socket_stream_buffer::overflow(int c) {
    // Write character to output buffer
    if (c != traits_type::eof()) {
        *pptr() = static_cast<char>(c);
        pbump(1);
    }
    // Flush buffer to socket
    if (sync() == 0) {
        return c;
    }
    return traits_type::eof();
}

int socket_stream_buffer::sync() {
    // Get size of data in output buffer
    auto size = pptr() - pbase();
    if (size > 0) {
        // Write buffer to socket using write_n
        auto res = socket_.write_n(pbase(), size);
        if (!res) {
            return -1; // Indicate failure
        }
        if (res.value() != size) {
            // Partial write or unexpected result
            return -1;
        }
        // Reset output buffer
        setp(out_buffer_, out_buffer_ + buffer_size);
    }
    return 0; // Success
}

socket_stream::socket_stream(uint16_t port)
    : socket_(std::make_unique<sockpp::tcp_socket>()),
      buffer_(*socket_) {
    sockpp::initialize();
    sockpp::tcp_acceptor acceptor(port);
    if (!acceptor) {
        throw std::runtime_error("failed to create acceptor on port " + std::to_string(port) + ": " + acceptor.last_error_str());
    }
    // Accept first client connection
    auto res = acceptor.accept(*socket_);
    if (!res) {
        throw std::runtime_error("failed to accept connection: " + res.error_message());
    }
}

socket_stream::socket_stream(const std::string& host, uint16_t port)
    : socket_(std::make_unique<sockpp::tcp_socket>()),
      buffer_(*socket_) {
    sockpp::initialize();
    sockpp::tcp_connector connector;
    auto res = connector.connect(host, port);
    if (!res) {
        throw std::runtime_error("failed to connect to " + host + ":" + std::to_string(port) + ": " + res.error_message());
    }
    *socket_ = std::move(connector); // Transfer ownership
}

socket_stream::~socket_stream() {
    if (socket_) {
        socket_->shutdown();
    }
}