// read_memory.cpp — DAP "readMemory" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class read_memory_handler : public dap::request_handler {
public:
    read_memory_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "readMemory"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::read_memory_request::from(req);

        size_t addr = static_cast<size_t>(r.memory_reference);
        auto &mem = ctx_.memory();
        int count = std::min(r.count, static_cast<int>(mem.size() - addr));

        std::ostringstream hexstr;
        for (int i = 0; i < count; ++i)
            hexstr << std::hex << std::setw(2) << std::setfill('0')
                   << (int)mem[addr + i];

        dap::response resp(r.seq, r.command);
        resp.success(true).result({{"address", r.memory_reference},
                                   {"data", hexstr.str()},
                                   {"unreadableBytes", 0}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_read_memory(dbg &ctx)
{
    return std::make_unique<read_memory_handler>(ctx);
}

} // namespace handlers
