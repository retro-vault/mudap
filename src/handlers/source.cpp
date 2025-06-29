#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_source(const dap::source_request &req)
{
    if (req.source_reference != 1)
    {
        // Invalid or unsupported reference
        dap::response resp(req.seq, req.command);
        resp.success(false).message("Unknown sourceReference");
        return resp.str();
    }

    std::ostringstream oss;
    char dasm_buf[64];
    uint16_t addr = 0;

    for (int i = 0; i < 256 && addr < memory_.size();)
    {
        int ts1, ts2;
        int ilen = z80ex_dasm(
            dasm_buf, sizeof(dasm_buf), 0, &ts1, &ts2,
            dasm_readbyte_cb, addr, &memory_);

        oss << format_hex(addr, 4) << ": " << dasm_buf << "\n";
        addr += (ilen > 0) ? ilen : 1;
        ++i;
    }

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"content", oss.str()}});
    return resp.str();
}
