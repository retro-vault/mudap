// sdcc/symbol_parser.h
#pragma once

#include <string>
#include <vector>
#include <memory>

namespace sdcc
{
    struct symbol
    {
        std::string name;
        uint32_t address;
        std::string area;
        int bank = 0;
    };

    struct segment
    {
        std::string name;
        uint32_t address;
        uint32_t size;
        std::string attributes;
    };

    class symbol_parser
    {
    public:
        virtual ~symbol_parser() = default;

        virtual bool parse(const std::string &path) = 0;
        virtual const std::vector<symbol> &symbols() const = 0;
        virtual const std::vector<segment> &segments() const = 0; // optional for formats that support it
    };

    std::unique_ptr<symbol_parser> make_map_parser();
    std::unique_ptr<symbol_parser> make_noi_parser();

} // namespace sdcc
