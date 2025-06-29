#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_variables(const dap::variables_request &req)
{
    nlohmann::json vars = nlohmann::json::array();

    if (req.variables_reference == 100)
    {
        // "Registers" root → returns subsystems
        vars.push_back({
            {"name", "CPU"},
            {"value", ""},
            {"variablesReference", 101},
        });

        // If you later support SIO, CTC, etc. add them here
        // vars.push_back({{"name", "CTC"}, {"value", ""}, {"variablesReference", 102}});
    }
    else if (req.variables_reference == 101)
    {
#define Z80REG(name, regid, width)                                  \
    {                                                               \
        nlohmann::json v;                                           \
        v["name"] = #name;                                          \
        v["value"] = format_hex(z80ex_get_reg(cpu_, regid), width); \
        v["variablesReference"] = 0;                                \
        vars.push_back(v);                                          \
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

        // F is the low byte of regAF
        vars.push_back({{"name", "F"},
                        {"value", format_hex(z80ex_get_reg(cpu_, regAF) & 0xFF, 2)},
                        {"variablesReference", 0}});
    }

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"variables", vars}});
    return resp.str();
}