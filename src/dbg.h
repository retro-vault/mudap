#pragma once
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include <z80ex.h>
#include <dap/dap.h>

class dbg
{
public:
    dbg();
    ~dbg();

    std::string handle_initialize(const dap::initialize_request &req);
    std::string handle_launch(const dap::launch_request &req);
    std::string handle_set_breakpoints(const dap::set_breakpoints_request &req);
    std::string handle_set_instruction_breakpoints(
        const dap::set_instruction_breakpoints_request &req);
    std::string handle_configuration_done(
        const dap::configuration_done_request &req);
    std::string handle_threads(const dap::threads_request &req);
    std::string handle_stack_trace(const dap::stack_trace_request &req);
    std::string handle_scopes(const dap::scopes_request &req);
    std::string handle_variables(const dap::variables_request &req);
    std::string handle_continue(const dap::continue_request &req);
    std::string handle_source(const dap::source_request &req);
    std::string handle_read_memory(const dap::read_memory_request &req);
    std::string handle_disassemble(const dap::disassemble_request &req);
    std::string handle_next(const dap::next_request &req);

    void set_send_event(std::function<void(const std::string &)> fn);

    std::vector<uint8_t> &memory() { return memory_; }
    const std::vector<uint8_t> &memory() const { return memory_; }

private:
    Z80EX_CONTEXT *cpu_;
    std::vector<uint8_t> memory_;
    std::vector<int> breakpoints_;
    std::vector<uint16_t> instruction_breakpoints_;
    int event_seq_;
    bool launched_;
    std::function<void(const std::string &)> send_event_;

    std::string format_hex(uint16_t value, int width);
};