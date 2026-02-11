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
#include <socket_stream.h>

socket_stream_buffer::socket_stream_buffer(sockpp::tcp_socket& socket)
    : socket_(socket) {
    setg(in_buffer_, in_buffer_, in_buffer_);
    setp(out_buffer_, out_buffer_ + buffer_size);
}

int socket_stream_buffer::underflow() {
    if (gptr() < egptr()) {
        return traits_type::to_int_type(*gptr());
    }

    auto n = socket_.read(in_buffer_, buffer_size);
    if (n <= 0) {
        return traits_type::eof();
    }

    setg(in_buffer_, in_buffer_, in_buffer_ + n);
    return traits_type::to_int_type(*gptr());
}

int socket_stream_buffer::overflow(int c) {
    if (c != traits_type::eof()) {
        *pptr() = static_cast<char>(c);
        pbump(1);
    }
    if (sync() == 0) {
        return c;
    }
    return traits_type::eof();
}

int socket_stream_buffer::sync() {
    auto size = pptr() - pbase();
    if (size > 0) {
        auto n = socket_.write_n(pbase(), size);
        if (n != size) {
            return -1;
        }
        setp(out_buffer_, out_buffer_ + buffer_size);
    }
    return 0;
}

socket_stream::socket_stream(uint16_t port)
    : std::iostream(nullptr),
      socket_(std::make_unique<sockpp::tcp_socket>()),
      buffer_(*socket_)
{
    rdbuf(&buffer_);
    sockpp::initialize();
    sockpp::tcp_acceptor acceptor(port);
    if (!acceptor) {
        throw std::runtime_error("failed to create acceptor on port " +
            std::to_string(port) + ": " + acceptor.last_error_str());
    }
    *socket_ = acceptor.accept();
    if (!*socket_) {
        throw std::runtime_error("failed to accept connection");
    }
}

socket_stream::socket_stream(const std::string& host, uint16_t port)
    : std::iostream(nullptr),
      socket_(std::make_unique<sockpp::tcp_socket>()),
      buffer_(*socket_)
{
    rdbuf(&buffer_);
    sockpp::initialize();
    sockpp::tcp_connector connector({host, static_cast<in_port_t>(port)});
    if (!connector) {
        throw std::runtime_error("failed to connect to " + host + ":" +
            std::to_string(port));
    }
    *socket_ = std::move(connector);
}

socket_stream::~socket_stream() {
    if (socket_) {
        socket_->shutdown();
    }
}
