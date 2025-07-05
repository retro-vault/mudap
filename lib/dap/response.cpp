// response.cpp
// Response builder for Debug Adapter Protocol (DAP).
//
// This file implements the `dap::response` class, which formats and
// serializes standard DAP responses. It supports marking success,
// attaching messages, and including result payloads.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include "dap/dap.h"

namespace dap
{
    response::response(int request_seq, const std::string &command)
    {
        _json["type"] = "response";
        _json["request_seq"] = request_seq;
        _json["command"] = command;
        _json["success"] = true;
    }

    response &response::success(bool ok)
    {
        _json["success"] = ok;
        return *this;
    }
    response &response::message(const std::string &msg)
    {
        _json["message"] = msg;
        return *this;
    }
    response &response::result(const json &result_data)
    {
        _json["body"] = result_data;
        return *this;
    }
    std::string response::str() const
    {
        return _json.dump();
    }

} // namespace dap
