#include <gtest/gtest.h>
#include <dap/dap.h>
#include <string>

using namespace dap;

TEST(DapRequestTest, ParseValidRequest)
{
    std::string msg = R"({"seq":1,"type":"request","command":"initialize","arguments":{"adapterID":"z80"}})";
    request req = request::parse(msg);
    EXPECT_EQ(req.seq, 1);
    EXPECT_EQ(req.command, "initialize");
    ASSERT_TRUE(req.arguments.contains("adapterID"));
    EXPECT_EQ(req.arguments["adapterID"], "z80");
}

TEST(DapRequestTest, ParseInvalidRequest)
{
    std::string invalidMsg = R"(this is not json)";
    EXPECT_THROW({ request::parse(invalidMsg); }, nlohmann::json::parse_error);
}

TEST(DapResponseTest, BuildSuccessResponse)
{
    response resp(1, "initialize");
    resp.success(true)
        .message("OK")
        .result({{"capabilities", {{"supportsConfigurationDoneRequest", true}}}});
    std::string out = resp.str();
    json j = json::parse(out);
    EXPECT_EQ(j["type"], "response");
    EXPECT_EQ(j["request_seq"], 1);
    EXPECT_EQ(j["command"], "initialize");
    EXPECT_EQ(j["success"], true);
    EXPECT_EQ(j["message"], "OK");
    ASSERT_TRUE(j["body"].contains("capabilities"));
    EXPECT_TRUE(j["body"]["capabilities"]["supportsConfigurationDoneRequest"]);
}

TEST(DapDispatchTest, HandlerDispatch)
{
    dap::dap dispatcher;
    bool called = false;
    dispatcher.register_handler("ping", [&](const std::string &input) -> std::string
                                {
        called = true;
        dap::request req = dap::request::parse(input);
        dap::response resp(req.seq, req.command);
        return resp.success(true).message("pong").str(); });

    std::string msg = R"({"seq":42,"type":"request","command":"ping","arguments":{}})";
    std::string result = dispatcher.handle_message(msg);
    ASSERT_TRUE(called);
    json j = json::parse(result);
    EXPECT_EQ(j["message"], "pong");
    EXPECT_EQ(j["request_seq"], 42);
    EXPECT_EQ(j["command"], "ping");
}

TEST(DapDispatchTest, UnregisteredCommand)
{
    dap::dap dispatcher;
    std::string msg = R"({"seq":99,"type":"request","command":"notfound","arguments":{}})";
    std::string result = dispatcher.handle_message(msg);
    json j = json::parse(result);
    EXPECT_EQ(j["success"], false);
    EXPECT_EQ(j["command"], "notfound");
}

TEST(DapTypedHandlerTest, InitializeRequestHandler)
{
    dap::dap dispatcher;

    dispatcher.register_typed_handler<dap::initialize_request>("initialize",
                                                               [](const dap::initialize_request &req) -> std::string
                                                               {
                                                                   dap::response resp(req.seq, req.command);
                                                                   resp.success(true).result({{"capabilities", {{"supports_configuration_done_request", true}}}});
                                                                   return resp.str();
                                                               });

    std::string incoming = R"({"seq":1,"command":"initialize","arguments":{"adapterID":"z80","clientID":"myclient"}})";
    std::string reply = dispatcher.handle_message(incoming);

    // Parse reply as JSON to check response fields
    auto j = dap::json::parse(reply);

    EXPECT_EQ(j["type"], "response");
    EXPECT_EQ(j["command"], "initialize");
    EXPECT_EQ(j["request_seq"], 1);
    EXPECT_TRUE(j["success"]);
    ASSERT_TRUE(j["body"].contains("capabilities"));
    ASSERT_TRUE(j["body"]["capabilities"].contains("supports_configuration_done_request"));
    EXPECT_TRUE(j["body"]["capabilities"]["supports_configuration_done_request"]);
}