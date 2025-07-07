// util.h
// Common utilities for parsing SDCC compiler output files.
//
// This file provides helper functions for reading and parsing SDCC compiler
// output files (CDB, MAP, NOI, SYM), used by parser classes to handle common
// tasks like file reading and regex-based string processing.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <regex>
#include <optional>

namespace sdcc::util {
    // Read all lines from a file, returning empty optional on error
    std::optional<std::vector<std::string>> read_lines(const std::string& path);
    // Trim whitespace from both ends of a string_view
    std::string_view trim(std::string_view str);
    // Split a string_view by delimiter, returning trimmed parts
    std::vector<std::string_view> split(std::string_view str, char delim);
    // Parse a line using a regex, returning matched groups
    std::optional<std::vector<std::string>> match(std::string_view line, const std::regex& pattern);
}