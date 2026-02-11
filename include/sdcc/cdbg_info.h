// cdbg_info.h
// Definition of hierarchical structures for SDCC C debug information.
//
// This file defines classes to represent the hierarchical structure of C 
// debug // information from SDCC CDB files, including modules, functions,
// symbols, types,  and line mappings. The cdbg_info_ prefix allows for 
// extensibility with other debug formats like DWARF.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace sdcc {
    struct cdbg_info_symbol {
        std::string name; // Variable name (e.g., "hour", "SECOND")
        std::string scope; // "global" or "local"
        std::string type_info; // Type details (e.g., "{2}SI:S")
    };

    struct cdbg_info_function {
        std::string name; // Function name (e.g., "clock_init")
        std::string scope; // "global" or "local"
        std::vector<cdbg_info_symbol> local_symbols; // Local variables
    };

    struct cdbg_info_type {
        std::string name; // Type name (e.g., "dim_s")
        std::string scope; // "global" or "local"
        std::string type_info; // Type details (e.g., "[({0}S:S$w...)")
    };

    struct cdbg_info_line {
        std::string file; // Source file path (e.g., "clock.c")
        int line = 0; // Line number
        uint16_t address = 0; // Mapped memory address
        std::string scope; // "global" or "local"
    };

    struct cdbg_info_module {
        std::string name; // Module name (e.g., "clock")
        std::string file; // Source file path (if applicable)
        std::vector<cdbg_info_function> functions;
        std::vector<cdbg_info_symbol> global_symbols;
        std::vector<cdbg_info_type> types;
        std::vector<cdbg_info_line> lines;
    };
}