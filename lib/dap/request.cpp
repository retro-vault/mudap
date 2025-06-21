#include "dap/dap.h"

namespace dap
{

    request request::parse(const std::string &json_text)
    {
        auto j = nlohmann::json::parse(json_text);
        request req;
        req.seq = j.value("seq", 0);
        req.command = j.value("command", "");
        req.arguments = j.value("arguments", nlohmann::json{});
        return req;
    }

} // namespace dap
