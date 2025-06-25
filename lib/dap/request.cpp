#include "dap/dap.h"

namespace dap
{

    request request::parse(const std::string &json_text)
    {
        json j = json::parse(json_text);
        request req;
        req.seq = j.value("seq", 0);
        req.command = j.value("command", "");
        req.arguments = j.value("arguments", json::object());
        return req;
    }

    initialize_request initialize_request::from(const request &req)
    {
        initialize_request r;
        r.seq = req.seq;
        r.command = req.command;
        r.arguments = req.arguments;
        r.adapter_id = req.arguments.value("adapterID", "");
        r.client_id = req.arguments.value("clientID", "");
        return r;
    }

} // namespace dap
