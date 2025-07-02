#include <dap/dap.h>

#include <dbg.h>

std::string dbg::handle_launch(const dap::launch_request &req)
{
    z80ex_reset(cpu_);
    std::fill(memory_.begin(), memory_.end(), 0);

    // Always use 1 for the virtual .lst file's sourceReference
    virtual_lst_source_reference_ = 1;

    // Load binary if specified in arguments
    if (req.arguments.contains("program"))
    {
        std::string bin_path = req.arguments["program"];
        std::ifstream bin_file(bin_path, std::ios::binary);
        if (bin_file)
        {
            bin_file.read(reinterpret_cast<char *>(memory_.data()), memory_.size());
        }

        // Set virtual path for the .lst using the program name
        std::string base;
        try
        {
            base = std::filesystem::path(bin_path).stem().string();
        }
        catch (...)
        {
            base = "listing";
        }
        virtual_lst_path_ = "/__virtual__/" + base + ".asm";
    }
    else
    {
        virtual_lst_path_ = "/__virtual__/listing.asm";
    }

    launched_ = true;

    dap::response resp(req.seq, req.command);
    resp.success(true).result({});

    // --- SEND "stopped" EVENT HERE ---
    if (send_event_)
    {
        nlohmann::json stopped_event;
        stopped_event["seq"] = event_seq_++;
        stopped_event["type"] = "event";
        stopped_event["event"] = "stopped";
        stopped_event["body"] = {
            {"reason", "entry"},
            {"threadId", 1},
            {"allThreadsStopped", true}};
        send_event_(stopped_event.dump());
    }
    // --- END STOPPED EVENT ---

    return resp.str();
}