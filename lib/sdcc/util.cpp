// util.cpp
// Implementation of common utilities for parsing SDCC compiler output files.
//
// This file implements helper functions for reading and parsing SDCC compiler
// output files (CDB, MAP, NOI, SYM), used by parser classes to handle common
// tasks like file reading and regex-based string processing.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <fstream>
#include <algorithm>
#include <sstream>

#include <sdcc/util.h>

namespace sdcc::util {
    std::optional<std::vector<std::string>> read_lines(const std::string& path) {
        std::vector<std::string> lines;
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::nullopt;
        }
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    std::string_view trim(std::string_view str) {
        auto first = str.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos) {
            return {};
        }
        auto last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, last - first + 1);
    }

    std::vector<std::string_view> split(std::string_view str, char delim) {
        std::vector<std::string_view> parts;
        size_t start = 0;
        while (start < str.size()) {
            auto end = str.find(delim, start);
            if (end == std::string_view::npos) {
                end = str.size();
            }
            auto part = trim(str.substr(start, end - start));
            if (!part.empty()) {
                parts.push_back(part);
            }
            start = end + 1;
        }
        return parts;
    }

    std::optional<std::vector<std::string>> match(std::string_view line, const std::regex& pattern) {
        std::cmatch match;
        if (std::regex_match(line.begin(), line.end(), match, pattern)) {
            std::vector<std::string> groups;
            groups.reserve(match.size());
            for (size_t i = 1; i < match.size(); ++i) { // Skip full match (index 0)
                groups.emplace_back(match[i].str());
            }
            return groups;
        }
        return std::nullopt;
    }
}