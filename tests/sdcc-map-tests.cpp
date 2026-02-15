#include <gtest/gtest.h>
#include <sdcc/map_parser.h>

#include <filesystem>
#include <string>

using namespace sdcc;

TEST(MapParserTest, ParseValidMapFile) {
    std::string map_path = "tests/data/ura.map";
    ASSERT_TRUE(std::filesystem::exists(map_path)) << "Test file ura.map not found";

    map_parser parser;
    auto result = parser.parse(map_path);
    ASSERT_TRUE(result.has_value()) << "Failed to parse ura.map";

    const auto &map = result.value();
    EXPECT_GT(map.segments.size(), 0u) << "Expected at least one segment";
    EXPECT_GT(map.symbols.size(), 0u) << "Expected at least one symbol";

    bool found_code_segment = false;
    for (const auto &seg : map.segments) {
        if (seg.name == "_CODE") {
            found_code_segment = true;
            EXPECT_EQ(seg.address, 0x100u);
            EXPECT_EQ(seg.size, 0x25FFu);
            break;
        }
    }
    EXPECT_TRUE(found_code_segment) << "Expected _CODE segment in ura.map";

    bool found_clock_init = false;
    for (const auto &sym : map.symbols) {
        if (sym.name == "G$clock_init$0$0") {
            found_clock_init = true;
            EXPECT_EQ(sym.address, 0x116u);
            break;
        }
    }
    EXPECT_TRUE(found_clock_init) << "Expected G$clock_init$0$0 symbol in ura.map";
}

TEST(MapParserTest, ParseInvalidMapFile) {
    map_parser parser;
    auto result = parser.parse("tests/data/does_not_exist.map");
    EXPECT_FALSE(result.has_value()) << "Parsing non-existent MAP should fail";
}
