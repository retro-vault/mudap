// dbg.h
// Debug Adapter Protocol (DAP) server class for Z80 emulation.
//
// This file defines the `dbg` class which implements a DAP-compliant debugger
// for a Z80 CPU using the z80ex library. It handles all standard DAP requests,
// maintains CPU and memory state, and manages breakpoints.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <nlohmann/json.hpp>
#include <z80ex.h>
#include <z80ex_dasm.h>
#include <dap/dap.h>

class dbg
{
public:
    dbg();
    ~dbg();

    // DAP request handlers.
    std::string handle_initialize(const dap::initialize_request &req);
    std::string handle_launch(const dap::launch_request &req);
    std::string handle_set_breakpoints(const dap::set_breakpoints_request &req);
    std::string handle_set_instruction_breakpoints(const dap::set_instruction_breakpoints_request &req);
    std::string handle_configuration_done(const dap::configuration_done_request &req);
    std::string handle_threads(const dap::threads_request &req);
    std::string handle_stack_trace(const dap::stack_trace_request &req);
    std::string handle_scopes(const dap::scopes_request &req);
    std::string handle_variables(const dap::variables_request &req);
    std::string handle_continue(const dap::continue_request &req);
    std::string handle_source(const dap::source_request &req);
    std::string handle_read_memory(const dap::read_memory_request &req);
    std::string handle_disassemble(const dap::disassemble_request &req);
    std::string handle_next(const dap::next_request &req);
    std::string handle_step_in(const dap::step_in_request &req);
    std::string handle_step_out(const dap::step_out_request &req);

    // Memory access.
    std::vector<uint8_t> &memory();             // Mutable memory buffer.
    const std::vector<uint8_t> &memory() const; // Const memory buffer.

    // Disassembler support.
    static uint8_t dasm_readbyte_cb(            // z80ex disasm callback.
        Z80EX_WORD addr, void *user_data); 

private:
    Z80EX_CONTEXT *cpu_;                        // z80ex CPU context.
    std::vector<uint8_t> memory_;               // Emulated 64K memory.
    std::vector<int> breakpoints_;              // Source breakpoints.
    std::vector<uint16_t>                       // Instruction breakpoints.
        instruction_breakpoints_; 
    int event_seq_;                             // Event sequence counter.
    bool launched_;                             // Whether 'launch' has occurred.


    std::string virtual_lst_path_ =             // Virtual disasm source path.
        "/__virtual__/listing.lst";
    int virtual_lst_source_reference_ = 1;      // Virtual source reference ID.

    std::string format_hex                      // Format as hex string.
        (uint16_t value, int width); 
};
