#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_stack_trace(const dap::stack_trace_request &req)
{
    uint16_t pc = z80ex_get_reg(cpu_, regPC);
    nlohmann::json frame = {
        {"id", 1},
        {"name", format_hex(pc, 4)},
        {"line", pc + 1},
        {"column", 1},
        {"source", {{"name", "Disassembly"}, {"sourceReference", 1}}},
        {"memoryReference", format_hex(0, 4)}};

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"stackFrames", {frame}},
                               {"totalFrames", 1}});

    return resp.str();
}
