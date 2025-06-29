#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_disassemble(const dap::disassemble_request &req)
{
    // Parse memory reference (supports "0x" prefix)
    uint64_t address = static_cast<uint64_t>(req.memory_reference);
    int instr_count = req.instruction_count;

    nlohmann::json instructions = nlohmann::json::array();

    for (int i = 0; i < instr_count;)
    {
        char dasm_buf[64];
        int ilen = z80ex_dasm(
            dasm_buf, sizeof(dasm_buf), 0, nullptr, nullptr,
            dasm_readbyte_cb, address, &memory_);

        if (ilen <= 0)
            ilen = 1;

        std::string bytes;
        for (int j = 0; j < ilen; ++j)
        {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02X", memory_[address + j]);
            bytes += buf;
        }

        char addr_buf[16];
        snprintf(addr_buf, sizeof(addr_buf), "0x%04X", (unsigned)address);

        instructions.push_back({{"address", addr_buf},
                                {"instruction", dasm_buf},
                                {"size", ilen},
                                {"bytes", bytes}});

        address += ilen;
        ++i;
    }

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"instructions", instructions}});
    return resp.str();
}