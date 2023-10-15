#pragma once

enum feedback { none, ack, nak };

class gdb {
public:
    // Ctor receives initialized socket.

    // Iterate through all registered packages and
    // find the one that matches the pattern.
    std::unique_ptr<package> recv_package();

    std::unique_ptr<package> create_package(string contents);
    std::unique_ptr<package> create_package(uint8_t *data, int len);

    // Queue package. Remove when acknowledgement.
    void send_package(std::unique_ptr<package> p);

    void run();
private:
    feedback pending;

};