// dbg.cpp
// Debug Adapter Protocol (DAP) server implementation for Z80 emulation.
//
// This file implements the core runtime functions of the `dbg` class used
// for handling Debug Adapter Protocol requests, formatting data, and
// registering handlers with the DAP dispatcher.
//
// Copyright 2025 Tomaz Stih. All rights reserved.
// MIT License.
#include <dbg.h>
#include <dap/handler.h>

// Forward declarations of handler factory functions (defined in handlers/).
namespace handlers {
    std::unique_ptr<dap::request_handler> make_initialize(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_launch(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_configuration_done(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_threads(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_stack_trace(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_scopes(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_variables(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_continue(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_next(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_step_in(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_step_out(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_set_breakpoints(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_set_instruction_breakpoints(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_source(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_read_memory(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_disconnect(dbg &ctx);
    std::unique_ptr<dap::request_handler> make_set_exception_breakpoints(dbg &ctx);
}

void dbg::register_handlers(dap::dap &dispatcher)
{
    dispatcher.add_handler(handlers::make_initialize(*this));
    dispatcher.add_handler(handlers::make_launch(*this));
    dispatcher.add_handler(handlers::make_configuration_done(*this));
    dispatcher.add_handler(handlers::make_threads(*this));
    dispatcher.add_handler(handlers::make_stack_trace(*this));
    dispatcher.add_handler(handlers::make_scopes(*this));
    dispatcher.add_handler(handlers::make_variables(*this));
    dispatcher.add_handler(handlers::make_continue(*this));
    dispatcher.add_handler(handlers::make_next(*this));
    dispatcher.add_handler(handlers::make_step_in(*this));
    dispatcher.add_handler(handlers::make_step_out(*this));
    dispatcher.add_handler(handlers::make_set_breakpoints(*this));
    dispatcher.add_handler(handlers::make_set_instruction_breakpoints(*this));
    dispatcher.add_handler(handlers::make_source(*this));
    dispatcher.add_handler(handlers::make_read_memory(*this));
    dispatcher.add_handler(handlers::make_disconnect(*this));
    dispatcher.add_handler(handlers::make_set_exception_breakpoints(*this));
}

void dbg::set_event_sender(std::function<void(const std::string &)> sender)
{
    send_event_ = std::move(sender);
}

void dbg::send_event(const std::string &event_json)
{
    if (send_event_)
        send_event_(event_json);
}

std::string dbg::format_hex(uint16_t value, int width)
{
    std::ostringstream oss;
    oss << std::uppercase << std::setfill('0') << std::setw(width)
        << std::hex << value;
    return "0x" + oss.str();
}

std::optional<source_location> dbg::lookup_source(uint16_t address) const
{
    namespace fs = std::filesystem;
    for (auto &mod : cdb_modules_)
    {
        for (auto &ln : mod.lines)
        {
            if (ln.address != address)
                continue;

            // Resolve the source file path.
            fs::path p(ln.file);
            std::string filename = p.filename().string();

            if (p.is_absolute() && fs::exists(p))
                return source_location{p.string(), ln.line};

            // Try relative to source_root_.
            if (!source_root_.empty())
            {
                fs::path candidate = fs::path(source_root_) / p;
                if (fs::exists(candidate))
                    return source_location{fs::canonical(candidate).string(), ln.line};
            }

            // Try current working directory.
            if (fs::exists(p))
                return source_location{fs::canonical(p).string(), ln.line};

            // Walk up from source_root_ looking for the file in each
            // ancestor and its immediate subdirectories (e.g. src/).
            if (!source_root_.empty())
            {
                fs::path dir = fs::path(source_root_);
                for (int depth = 0; depth < 5 && dir.has_parent_path(); ++depth)
                {
                    dir = dir.parent_path();
                    // Try direct match: parent/main.c
                    fs::path candidate = dir / filename;
                    if (fs::exists(candidate))
                        return source_location{fs::canonical(candidate).string(), ln.line};

                    // Try one-level subdirectories: parent/src/main.c
                    try {
                        for (auto &entry : fs::directory_iterator(dir))
                        {
                            if (!entry.is_directory())
                                continue;
                            candidate = entry.path() / filename;
                            if (fs::exists(candidate))
                                return source_location{fs::canonical(candidate).string(), ln.line};
                        }
                    } catch (...) {}
                }
            }

            // Return unresolved path as last resort.
            return source_location{ln.file, ln.line};
        }
    }
    return std::nullopt;
}

std::optional<uint16_t> dbg::lookup_address(const std::string &file, int line) const
{
    namespace fs = std::filesystem;
    std::string query_name = fs::path(file).filename().string();

    for (auto &mod : cdb_modules_)
    {
        for (auto &ln : mod.lines)
        {
            if (ln.line != line)
                continue;

            // Match by bare filename since CDB stores bare names
            // and the DAP request sends full paths.
            std::string cdb_name = fs::path(ln.file).filename().string();
            if (cdb_name == query_name)
                return ln.address;
        }
    }
    return std::nullopt;
}

uint8_t dbg::dasm_readbyte_cb(Z80EX_WORD addr, void *user_data)
{
    auto *memory = static_cast<std::vector<uint8_t> *>(user_data);
    if (addr < memory->size())
        return (*memory)[addr];
    else
        return 0xFF;
}
