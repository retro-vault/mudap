#include <dbg.h>

void dbg::set_send_event(std::function<void(const std::string &)> fn)
{
    send_event_ = fn;
}

std::string dbg::format_hex(uint16_t value, int width)
{
    std::ostringstream oss;
    oss << std::uppercase << std::setfill('0') << std::setw(width)
        << std::hex << value;
    return "0x" + oss.str();
}

uint8_t dbg::dasm_readbyte_cb(Z80EX_WORD addr, void *user_data)
{
    auto *memory = static_cast<std::vector<uint8_t> *>(user_data);
    if (addr < memory->size())
        return (*memory)[addr];
    else
        return 0xFF;
}