#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_source(const dap::source_request &req)
{
    std::ostringstream oss;
    uint16_t pc = z80ex_get_reg(cpu_, regPC);
    char dasm_buf[64];

    for (int i = 0; i < 256 && (pc + i) < memory_.size();)
    {
        uint16_t addr = pc + i;
        int tstates_var = 0, tstates2_var = 0;
        int ilen = z80ex_dasm(
            dasm_buf, sizeof(dasm_buf), 0, &tstates_var, &tstates2_var,
            dasm_readbyte_cb, addr, &memory_);

        oss << std::setfill('0') << std::setw(4) << std::hex << addr << ": ";

        // Hex dump
        for (int j = 0; j < ilen && (addr + j) < memory_.size(); ++j)
            oss << std::setw(2) << std::setfill('0') << (int)memory_[addr + j] << " ";

        // Pad hex
        for (int j = ilen; j < 4; ++j)
            oss << "   ";

        oss << "  " << dasm_buf << "\n";
        i += (ilen > 0) ? ilen : 1;
    }

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"content", oss.str()}});
    return resp.str();
}