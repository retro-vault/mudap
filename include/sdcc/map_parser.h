// map_parser.h
// Parser for SDCC/ASxxxx MAP linker output files.
//
// Extracts segment and symbol data from MAP files, which can be used
// as supplemental debug metadata when CDB data is incomplete or absent.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#pragma once

#include <optional>

#include <sdcc/segment.h>
#include <sdcc/symbol.h>

namespace sdcc {

struct map_info {
    std::vector<segment> segments;
    std::vector<symbol> symbols;
};

class map_parser {
public:
    std::optional<map_info> parse(const std::string& path);
    const map_info& data() const { return data_; }

private:
    map_info data_;
};

} // namespace sdcc
