// dbg.h
// Debug Adapter Protocol (DAP) server class for Z80 emulation.
//
// This file defines the `dbg` class which implements a DAP-compliant debugger
// for a Z80 CPU using the z80ex library. It holds all emulation state and
// registers handler classes with the DAP dispatcher.
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
#include <filesystem>
#include <optional>

#include <nlohmann/json.hpp>
#include <sdcc/cdbg_info.h>
#include <z80ex.h>
#include <z80ex_dasm.h>
#include <dap/dap.h>

struct source_location {
    std::string file;
    int line;
};

class dbg
{
public:
    dbg();
    ~dbg();

    // Register all handler objects with the DAP dispatcher.
    void register_handlers(dap::dap &dispatcher);

    // Event sending (set by main before running the dispatcher).
    void set_event_sender(std::function<void(const std::string &)> sender);
    void send_event(const std::string &event_json);

    // Accessors for handler classes.
    Z80EX_CONTEXT *cpu() { return cpu_; }
    std::vector<uint8_t> &memory() { return memory_; }
    const std::vector<uint8_t> &memory() const { return memory_; }
    std::vector<uint16_t> &breakpoints() { return breakpoints_; }
    std::vector<uint16_t> &instruction_breakpoints() { return instruction_breakpoints_; }
    int next_event_seq() { return event_seq_++; }
    bool launched() const { return launched_; }
    void set_launched(bool v) { launched_ = v; }
    const std::string &virtual_lst_path() const { return virtual_lst_path_; }
    void set_virtual_lst_path(const std::string &p) { virtual_lst_path_ = p; }
    int virtual_lst_source_reference() const { return virtual_lst_source_reference_; }
    void set_virtual_lst_source_reference(int r) { virtual_lst_source_reference_ = r; }

    // CDB debug info.
    void set_cdb_modules(std::vector<sdcc::cdbg_info_module> m) { cdb_modules_ = std::move(m); }
    const std::vector<sdcc::cdbg_info_module> &cdb_modules() const { return cdb_modules_; }
    bool has_cdb() const { return !cdb_modules_.empty(); }
    void set_source_root(const std::string &r) { source_root_ = r; }
    const std::string &source_root() const { return source_root_; }
    std::optional<source_location> lookup_source(uint16_t address) const;
    std::optional<uint16_t> lookup_address(const std::string &file, int line) const;

    // Disassembler support.
    static uint8_t dasm_readbyte_cb(Z80EX_WORD addr, void *user_data);

    // Formatting helper.
    std::string format_hex(uint16_t value, int width);

private:
    Z80EX_CONTEXT *cpu_;
    std::vector<uint8_t> memory_;
    std::vector<uint16_t> breakpoints_;
    std::vector<uint16_t> instruction_breakpoints_;
    int event_seq_;
    bool launched_;
    std::function<void(const std::string &)> send_event_;

    std::string virtual_lst_path_ = "/__virtual__/listing.lst";
    int virtual_lst_source_reference_ = 1;

    std::vector<sdcc::cdbg_info_module> cdb_modules_;
    std::string source_root_;
};
