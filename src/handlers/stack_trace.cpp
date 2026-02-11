// stack_trace.cpp — DAP "stackTrace" request handler.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dap/dap.h>
#include <dap/handler.h>
#include <dbg.h>

namespace handlers {

class stack_trace_handler : public dap::request_handler {
public:
    stack_trace_handler(dbg &ctx) : ctx_(ctx) {}
    std::string command() const override { return "stackTrace"; }

    std::string handle(const dap::request &req) override
    {
        auto r = dap::stack_trace_request::from(req);
        uint16_t pc = z80ex_get_reg(ctx_.cpu(), regPC);

        nlohmann::json frame;

        // Try C source mapping from CDB first.
        auto src = ctx_.has_cdb() ? ctx_.lookup_source(pc) : std::nullopt;
        if (src)
        {
            std::string name = std::filesystem::path(src->file).filename().string();
            frame = {
                {"id", 1},
                {"name", name + ":" + std::to_string(src->line)},
                {"memoryReference", ctx_.format_hex(pc, 4)},
                {"instructionReference", ctx_.format_hex(pc, 4)},
                {"source",
                 {{"name", name},
                  {"path", src->file},
                  {"sourceReference", 0},
                  {"presentationHint", "normal"}}},
                {"line", src->line},
                {"column", 1}};
        }
        else
        {
            // Fall back to virtual disassembly listing.
            // Increment the sourceReference so VSCode re-fetches the
            // content (which starts from the current PC).
            int source_ref = ctx_.virtual_lst_source_reference() + 1;
            ctx_.set_virtual_lst_source_reference(source_ref);

            frame = {
                {"id", 1},
                {"name", ctx_.format_hex(pc, 4)},
                {"memoryReference", ctx_.format_hex(pc, 4)},
                {"instructionReference", ctx_.format_hex(pc, 4)},
                {"source",
                 {{"name", "z80.s"},
                  {"sourceReference", source_ref},
                  {"presentationHint", "deemphasize"},
                  {"mimeType", "text/x-asm"}}},
                {"line", 1},
                {"column", 1}};
        }

        dap::response resp(r.seq, r.command);
        resp.success(true).result({{"stackFrames", {frame}},
                                   {"totalFrames", 1}});
        return resp.str();
    }

private:
    dbg &ctx_;
};

std::unique_ptr<dap::request_handler> make_stack_trace(dbg &ctx)
{
    return std::make_unique<stack_trace_handler>(ctx);
}

} // namespace handlers
