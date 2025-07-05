#include <dap/dap.h>

#include <dbg.h>

// Assume these are members of dbg class:
// std::string virtual_lst_path_;
// int virtual_lst_source_reference_;

std::string dbg::handle_source(const dap::source_request &req)
{
    // Only compare source_reference as int!
    if (req.source_reference != virtual_lst_source_reference_)
    {
        dap::response resp(req.seq, req.command);
        resp.success(false).message("Unknown sourceReference");
        return resp.str();
    }

    std::ostringstream oss;
    char dasm_buf[64];
    uint32_t addr = 0; // For future bank support

    for (int i = 0; i < 256 && addr < memory_.size();)
    {
        int ts1 = 0, ts2 = 0;
        int ilen = z80ex_dasm(
            dasm_buf, sizeof(dasm_buf), 0, &ts1, &ts2,
            dasm_readbyte_cb, addr, &memory_);

        // 6 spaces, then 6-digit address
        oss << "      " << std::uppercase << std::setfill('0') << std::setw(6) << std::hex << addr << " ";

        // Opcode bytes (up to 4, left-aligned, padded with spaces)
        int opcode_chars = 0;
        for (int j = 0; j < ilen && (addr + j) < memory_.size(); ++j)
        {
            oss << std::setw(2) << std::setfill('0') << std::hex << (int)memory_[addr + j] << " ";
            opcode_chars += 3; // 2 digits + space
        }
        // pad to 8 chars (3*2+2 = 8 if 2 bytes, etc)
        for (; opcode_chars < 8; ++opcode_chars)
            oss << " ";

        // Calculate how many more spaces needed to reach column 27
        // 6 spaces + 6 addr + 1 space + 8 opcodes = 21 chars so far
        // We want cycles at col 27, so 26-21 = 5 spaces
        oss << std::string(26 - (6 + 6 + 1 + opcode_chars), ' ');

        // Cycles, decimal, right-aligned in 3, in brackets
        oss << "[" << std::right << std::setw(2) << std::dec << ts1 << "]";

        // 3 spaces
        oss << "   ";

        // Disassembly mnemonic (instruction)
        oss << dasm_buf << "\n";

        addr += (ilen > 0) ? ilen : 1;
        ++i;
    }

    dap::response resp(req.seq, req.command);
    resp.success(true)
        .result({{"content", oss.str()},
                 {"mimeType", "text/x-asm"}});
    return resp.str();
}