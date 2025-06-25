#pragma once

#include <string>
#include <vector>
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
        std::string type; // Usually "request"
        std::string command;
        json arguments;

        static request parse(const std::string &json_text);
        virtual ~request() = default;
    };

    struct initialize_request : public request
    {
        std::string adapter_id;
        std::string client_id;
        std::string client_name;
        std::string locale;

        static initialize_request from(const request &req);
    };

    struct launch_request : public request
    {
        bool no_debug = false;
        std::string program;

        static launch_request from(const request &req);
    };

    struct attach_request : public request
    {
        // Add attach-specific fields as your protocol requires.
        static attach_request from(const request &req);
    };

    struct set_breakpoints_request : public request
    {
        json source;
        json breakpoints;

        static set_breakpoints_request from(const request &req);
    };

    struct configuration_done_request : public request
    {
        static configuration_done_request from(const request &req);
    };

    struct threads_request : public request
    {
        static threads_request from(const request &req);
    };

    struct stack_trace_request : public request
    {
        int thread_id = 0;
        int start_frame = 0;
        int levels = 0;

        static stack_trace_request from(const request &req);
    };

    struct scopes_request : public request
    {
        int frame_id = 0;

        static scopes_request from(const request &req);
    };

    struct variables_request : public request
    {
        int variables_reference = 0;

        static variables_request from(const request &req);
    };

    struct continue_request : public request
    {
        int thread_id = 0;

        static continue_request from(const request &req);
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