#include <gtest/gtest.h>
#include <sdcc/cdb_parser.h>
#include <sdcc/cdbg_info.h>
#include <string>
#include <filesystem>
#include <fstream>

using namespace sdcc;

TEST(CdbParserTest, ParseValidCdbFile) {
    // Path to the test file (adjust path as needed for test environment)
    std::string cdb_path = "tests/data/ura.cdb";
    
    // Ensure the test file exists
    ASSERT_TRUE(std::filesystem::exists(cdb_path)) << "Test file ura.cdb not found";

    cdb_parser parser;
    auto result = parser.parse(cdb_path);
    
    // Check that parsing succeeded
    ASSERT_TRUE(result.has_value()) << "Failed to parse ura.cdb";
    const auto& modules = result.value();
    
    // Verify some key entries from ura.cdb
    EXPECT_GE(modules.size(), 5) << "Expected at least 5 modules";

    // Check module 'clock'
    bool found_clock_module = false;
    const cdbg_info_module* clock_module = nullptr;
    for (const auto& module : modules) {
        if (module.name == "clock") {
            found_clock_module = true;
            clock_module = &module;
            EXPECT_FALSE(module.file.empty()) << "Module 'clock' should have a file path";
            EXPECT_TRUE(module.file.find("clock.c") != std::string::npos || 
                        module.file == "clock") << "Module 'clock' file path should include 'clock.c' or be 'clock'";
            break;
        }
    }
    ASSERT_TRUE(found_clock_module) << "Module 'clock' not found";
    ASSERT_NE(clock_module, nullptr);

    // Check function 'clock_init' in module 'clock'
    bool found_function = false;
    for (const auto& func : clock_module->functions) {
        if (func.name == "clock_init" && func.scope == "global") {
            found_function = true;
            break;
        }
    }
    EXPECT_TRUE(found_function) << "Function 'clock_init' not found in module 'clock'";

    // Check local symbol 'hour' in function 'clock_loop'
    bool found_symbol = false;
    for (const auto& func : clock_module->functions) {
        if (func.name == "clock_loop") {
            for (const auto& symbol : func.local_symbols) {
                if (symbol.name == "hour" && symbol.scope == "local") {
                    found_symbol = true;
                    EXPECT_EQ(symbol.type_info, "{2}SI:S") << "Symbol 'hour' has incorrect type info";
                    break;
                }
            }
            break;
        }
    }
    EXPECT_TRUE(found_symbol) << "Symbol 'hour' not found in function 'clock_loop'";

    // Check global symbol 'SECOND' in module 'clock'
    bool found_global_symbol = false;
    for (const auto& symbol : clock_module->global_symbols) {
        if (symbol.name == "SECOND" && symbol.scope == "global") {
            found_global_symbol = true;
            EXPECT_EQ(symbol.type_info, "{1}SC:U") << "Symbol 'SECOND' has incorrect type info";
            break;
        }
    }
    EXPECT_TRUE(found_global_symbol) << "Global symbol 'SECOND' not found in module 'clock'";

    // Check type 'dim_s' in module 'clock'
    bool found_type = false;
    for (const auto& type : clock_module->types) {
        if (type.name == "dim_s" && type.scope == "local") {
            found_type = true;
            EXPECT_TRUE(type.type_info.find("({0}S:S$w") != std::string::npos) << "Type 'dim_s' has incorrect type info";
            break;
        }
    }
    EXPECT_TRUE(found_type) << "Type 'dim_s' not found in module 'clock'";

    // Check line info for clock.c:18
    bool found_line = false;
    for (const auto& line : clock_module->lines) {
        if (line.file.find("clock.c") != std::string::npos && line.line == 18 && line.scope == "local") {
            found_line = true;
            break;
        }
    }
    EXPECT_TRUE(found_line) << "Line info for clock.c:18 not found in module 'clock'";
}

TEST(CdbParserTest, ParseInvalidCdbFile) {
    cdb_parser parser;
    // Try to parse a non-existent file
    auto result = parser.parse("non_existent_file.cdb");
    
    // Expect parsing to fail
    EXPECT_FALSE(result.has_value()) << "Parsing non-existent file should fail";
}

TEST(CdbParserTest, ParseEmptyCdbFile) {
    // Create a temporary empty file
    std::string empty_file = "tests/data/empty.cdb";
    std::ofstream out(empty_file);
    out.close();
    
    cdb_parser parser;
    auto result = parser.parse(empty_file);
    
    // Expect parsing to succeed but return empty data
    ASSERT_TRUE(result.has_value()) << "Parsing empty file should succeed";
    EXPECT_TRUE(result.value().empty()) << "Empty file should yield no modules";
    
    // Clean up
    std::filesystem::remove(empty_file);
}

TEST(CdbParserTest, ParseDataConsistency) {
    std::string cdb_path = "tests/data/ura.cdb";
    ASSERT_TRUE(std::filesystem::exists(cdb_path)) << "Test file ura.cdb not found";

    cdb_parser parser;
    auto result = parser.parse(cdb_path);
    ASSERT_TRUE(result.has_value()) << "Failed to parse ura.cdb";
    
    const auto& modules = parser.data();
    
    // Verify data() returns the same data as parse()
    EXPECT_EQ(modules.size(), result.value().size()) << "data() should return same modules as parse()";
    
    // Allow 6 modules due to observed output
    EXPECT_GE(modules.size(), 5) << "Expected at least 5 modules (e.g., clock, screen, trig, poly, sprite)";
    
    size_t total_functions = 0, total_global_symbols = 0, total_local_symbols = 0, total_types = 0, total_lines = 0;
    for (const auto& module : modules) {
        total_functions += module.functions.size();
        total_global_symbols += module.global_symbols.size();
        total_types += module.types.size();
        total_lines += module.lines.size();
        for (const auto& func : module.functions) {
            total_local_symbols += func.local_symbols.size();
        }
    }
    
    EXPECT_GE(total_functions, 3) << "Expected at least 3 functions (e.g., clock_init, clock_exit, clock_loop)";
    EXPECT_GE(total_global_symbols, 5) << "Expected at least 5 global symbols (e.g., SECOND, MINUTE)";
    EXPECT_GE(total_local_symbols, 5) << "Expected at least 5 local symbols (e.g., hour, minute)";
    EXPECT_GE(total_types, 5) << "Expected at least 5 types (e.g., dim_s, point_s)";
    EXPECT_GE(total_lines, 10) << "Expected at least 10 line entries";
}