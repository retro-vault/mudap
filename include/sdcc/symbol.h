// symbol.h
// Definition of the symbol structure for SDCC compiler output.
//
// This file defines the symbol structure used to store symbol information
// (name, address, area, bank) parsed from SDCC compiler output files (MAP,
// NOI, SYM) for debugging a Z80 CPU emulated using z80ex.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.

#pragma once

#include <string>
#include <cstdint>

namespace sdcc {
    struct symbol {
        std::string name;
        uint32_t address;
        std::string area;
        int bank = 0;
    };
}