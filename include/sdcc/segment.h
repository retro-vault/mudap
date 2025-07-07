// segment.h
// Definition of the segment structure for SDCC compiler output.
//
// This file defines the segment structure used to store memory segment
// information (name, address, size, attributes) parsed from SDCC compiler
// output files (MAP) for debugging a Z80 CPU emulated using z80ex.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.

#pragma once

#include <string>
#include <cstdint>

namespace sdcc {
    struct segment {
        std::string name;
        uint32_t address;
        uint32_t size;
        std::string attributes;
    };
}