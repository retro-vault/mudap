// parser.h
// Template interface for parsing SDCC compiler output files.
//
// This file defines the parser template base class and its implementation for parsing 
// symbols, segments, and other SDCC compiler output data.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include <sdcc/symbol.h>
#include <sdcc/segment.h>
#include <sdcc/cdbg_info.h>

namespace sdcc {
    template<typename T>
    class parser {
    public:
        virtual ~parser() = default;
        virtual std::optional<std::vector<T>> 
            parse(const std::string& path) = 0;
        virtual const std::vector<T>& data() const = 0;
    };

    template<typename T>
    class parser_impl : public parser<T> {
    public:
        std::optional<std::vector<T>> parse(const std::string& path) override {
            data_.clear();
            if (do_parse(path)) {
                return data_;
            }
            return std::nullopt;
        }

        const std::vector<T>& data() const override {
            return data_;
        }

    protected:
        virtual bool do_parse(const std::string& path) = 0;
        std::vector<T> data_;
    };
}