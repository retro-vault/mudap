// map_parser.cpp
// Implementation of MAP parser for SDCC/ASxxxx linker output.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <regex>

#include <sdcc/map_parser.h>
#include <sdcc/util.h>

namespace sdcc {

std::optional<map_info> map_parser::parse(const std::string& path)
{
    auto lines = util::read_lines(path);
    if (!lines)
        return std::nullopt;

    data_ = {};

    // Example:
    // _CODE 00000100 000025FF = 9727. bytes (REL,CON)
    const std::regex segment_re(
        R"(^\s*([A-Za-z0-9_.\$]+)\s+((?:0[xX])?[0-9A-Fa-f]{4,8})\s+((?:0[xX])?[0-9A-Fa-f]{4,8}).*\(([^)]*)\)\s*$)");

    // Example:
    // 00000116  C$clock.c$18$0_0$36                clock
    const std::regex symbol_re(
        R"(^\s*((?:0[xX])?[0-9A-Fa-f]{4,8})\s+([^\s]+)(?:\s+([^\s]+))?\s*$)");

    for (const auto& raw : *lines)
    {
        std::string line(raw);
        auto trimmed = util::trim(line);
        if (trimmed.empty())
            continue;

        if (auto seg = util::match(trimmed, segment_re))
        {
            try
            {
                segment s;
                s.name = (*seg)[0];
                s.address = static_cast<uint32_t>(std::stoul((*seg)[1], nullptr, 16));
                s.size = static_cast<uint32_t>(std::stoul((*seg)[2], nullptr, 16));
                s.attributes = (*seg)[3];
                data_.segments.push_back(std::move(s));
                continue;
            }
            catch (...) {}
        }

        if (auto sym = util::match(trimmed, symbol_re))
        {
            // Skip table headings.
            if ((*sym)[0] == "Value" || (*sym)[1] == "Global")
                continue;

            try
            {
                symbol s;
                s.address = static_cast<uint32_t>(std::stoul((*sym)[0], nullptr, 16));
                s.name = (*sym)[1];
                s.area = (sym->size() > 2) ? (*sym)[2] : "";
                s.bank = 0;
                data_.symbols.push_back(std::move(s));
                continue;
            }
            catch (...) {}
        }
    }

    return data_;
}

} // namespace sdcc
