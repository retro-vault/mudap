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
