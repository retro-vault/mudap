#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_stack_trace(const dap::stack_trace_request &req)
{
    uint16_t pc = z80ex_get_reg(cpu_, regPC);

    // Minimal stack frame with no line/column to trigger disassembly view
    nlohmann::json frame = {
        {"id", 1},
        {"name", format_hex(pc, 4)},
        {"memoryReference", format_hex(pc, 4)},
        {"instructionReference", format_hex(pc, 4)},
        {"source", nullptr}};

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"stackFrames", {frame}},
                               {"totalFrames", 1}});

    return resp.str();
}