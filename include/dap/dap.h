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

    struct request
    {
        int seq = 0;
        std::string command;
        json arguments;

        static request parse(const std::string &json_text);
        virtual ~request() = default;
    };

    struct initialize_request : public request
    {
        std::string adapter_id;
        std::string client_id;

        static initialize_request from(const request &req); // Declaration only!
    };

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

    class dap
    {
    public:
        dap();
        void register_handler(const std::string &command, dap_handler handler);

        template <typename request_t>
        void register_typed_handler(const std::string &command, std::function<std::string(const request_t &)> handler)
        {
            _typed_handlers[command] = [handler](const request &req)
            {
                return handler(request_t::from(req));
            };
        }

        std::string handle_message(const std::string &json);

    private:
        std::unordered_map<std::string, dap_handler> _handlers;
        std::unordered_map<std::string, std::function<std::string(const request &)>> _typed_handlers;
        std::mutex _mutex;
    };

} // namespace dap