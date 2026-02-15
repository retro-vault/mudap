// scopes.cpp — DAP "scopes" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class scopes_handler : public dap::request_handler {
public:
    scopes_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "scopes"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::scopes_request::from(req);
        nlohmann::json scopes = nlohmann::json::array();
        scopes.push_back({
            {"name", "Registers"},
            {"variablesReference", 100},
            {"presentationHint", "registers"},
            {"expensive", false}});

        if (ctx_.has_map())
        {
            scopes.push_back({
                {"name", "MAP"},
                {"variablesReference", 200},
                {"presentationHint", "locals"},
                {"expensive", false}});
        }

        dap::response resp(r.seq, r.command);
        resp.success(true).result({{"scopes", scopes}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_scopes(dbg &ctx)
{
    return std::make_unique<scopes_handler>(ctx);
}

} // namespace handlers
