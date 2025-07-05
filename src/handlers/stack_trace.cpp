#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_stack_trace(const dap::stack_trace_request &req)
{
    uint16_t pc = z80ex_get_reg(cpu_, regPC);

    // Virtual source name and reference (must match your handle_source)
    constexpr int source_ref = 1;

    nlohmann::json frame = {
        {"id", 1},
        {"name", format_hex(pc, 4)},
        {"memoryReference", format_hex(pc, 4)},
        {"instructionReference", format_hex(pc, 4)},
        {"source",
         {{"name", "z80.s"},
          {"sourceReference", source_ref},
          {"presentationHint", "normal"},
          {"mimeType", "text/x-asm"}}},
        {"line", 1},  // <- must be present
        {"column", 1} // <- must be present
    };

    dap::response resp(req.seq, req.command);
    resp.success(true).result({{"stackFrames", {frame}},
                               {"totalFrames", 1}});

    return resp.str();
}
