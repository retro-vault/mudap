#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>

#include <nlohmann/json.hpp>

namespace dap
{
    using json = nlohmann::json;
    using dap_handler = std::function<std::string(const std::string &)>;

    // Represents a parsed DAP request message
    struct request
    {
        int seq = 0;
        std::string command;
        json arguments;

        static request parse(const std::string &json_text);
    };

    // Represents a DAP response message
    class response
    {
    public:
        response(int request_seq, const std::string &command);

        response &success(bool ok = true);
        response &message(const std::string &msg);
        response &result(const json &result_data);

        std::string str() const;

    private:
        json _json;
    };

    // Main DAP dispatcher
    class dap
    {
    public:
        dap();

        void register_handler(const std::string &command, dap_handler handler);
        std::string handle_message(const std::string &json);

    private:
        std::unordered_map<std::string, dap_handler> _handlers;
        std::mutex _mutex;

        std::string extract_command(const std::string &json) const; // <-- Add this line
    };

} // namespace dap
