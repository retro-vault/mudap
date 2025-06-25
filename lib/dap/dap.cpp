#include <dap/dap.h>

namespace dap
{

      dap::dap() = default;

    void dap::register_handler(const std::string &command, dap_handler handler)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _handlers[command] = handler;
    }

    std::string dap::handle_message(const std::string &json_text)
    {
        request req;
        try
        {
            req = request::parse(json_text);
        }
        catch (...)
        {
            response resp(0, "<unknown>");
            resp.success(false).message("Malformed request");
            return resp.str();
        }
        std::lock_guard<std::mutex> lock(_mutex);
        auto typed_it = _typed_handlers.find(req.command);
        if (typed_it != _typed_handlers.end())
        {
            return typed_it->second(req);
        }
        auto it = _handlers.find(req.command);
        if (it != _handlers.end())
        {
            return it->second(json_text);
        }
        response resp(req.seq, req.command);
        resp.success(false).message("Unknown command: " + req.command);
        return resp.str();
    }

} // namespace dap