// variables.cpp — DAP "variables" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class variables_handler : public dap::request_handler {
public:
    variables_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "variables"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::variables_request::from(req);
        nlohmann::json vars = nlohmann::json::array();

        if (r.variables_reference == 100)
        {
            vars.push_back({
                {"name", "CPU"},
                {"value", ""},
                {"variablesReference", 101},
            });
        }
        else if (r.variables_reference == 101)
        {
#define Z80REG(name, regid, width)                                          \
    {                                                                       \
        nlohmann::json v;                                                   \
        v["name"] = #name;                                                  \
        v["value"] = ctx_.format_hex(z80ex_get_reg(ctx_.cpu(), regid), width); \
        v["variablesReference"] = 0;                                        \
        vars.push_back(v);                                                  \
    }
            Z80REG(AF, regAF, 4)
            Z80REG(BC, regBC, 4)
            Z80REG(DE, regDE, 4)
            Z80REG(HL, regHL, 4)
            Z80REG(IX, regIX, 4)
            Z80REG(IY, regIY, 4)
            Z80REG(SP, regSP, 4)
            Z80REG(PC, regPC, 4)
            Z80REG(R, regR, 2)
            Z80REG(I, regI, 2)
#undef Z80REG

            vars.push_back({{"name", "F"},
                            {"value", ctx_.format_hex(
                                z80ex_get_reg(ctx_.cpu(), regAF) & 0xFF, 2)},
                            {"variablesReference", 0}});
        }

        dap::response resp(r.seq, r.command);
        resp.success(true).result({{"variables", vars}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_variables(dbg &ctx)
{
    return std::make_unique<variables_handler>(ctx);
}

} // namespace handlers
