#include "dap/dap.h"
#include <nlohmann/json.hpp>

namespace dap
{

    dap::dap() = default;

    void dap::register_handler(const std::string &command, dap_handler handler)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _handlers[command] = std::move(handler);
    }

    std::string dap::handle_message(const std::string &json_str)
    {
        std::string command = extract_command(json_str);

        if (command.empty())
        {
            return R"({"type":"error","message":"Invalid or missing command"})";
        }

        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _handlers.find(command);
        if (it != _handlers.end())
        {
            return it->second(json_str);
        }
        else
        {
            return R"({"type":"error","message":"Unknown command"})";
        }
    }

    std::string dap::extract_command(const std::string &json_str) const
    {
        try
        {
            json msg = json::parse(json_str);
            if (msg.contains("command"))
                return msg["command"].get<std::string>();
        }
        catch (...)
        {
            // silently ignore errors
        }
        return "";
    }

} // namespace dap
