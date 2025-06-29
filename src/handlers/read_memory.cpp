#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_read_memory(const dap::read_memory_request &req)
{
    // Parse the memory reference (assuming it's a hex string)
    size_t addr = 0;
    addr = static_cast<size_t>(req.memory_reference);

    // Calculate how many bytes to read
    int count = std::min(req.count, static_cast<int>(memory_.size() - addr));

    // Format the output as a hex string
    std::ostringstream hexstr;
    for (int i = 0; i < count; ++i)
        hexstr << std::hex << std::setw(2) << std::setfill('0') << (int)memory_[addr + i];

    // Build the response
    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"address", req.memory_reference},
                               {"data", hexstr.str()},
                               {"unreadableBytes", 0}});

    return resp.str();
}