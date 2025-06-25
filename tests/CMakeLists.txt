# tests/CMakeLists.txt

include(FetchContent)

# Fetch GoogleTest (v1.14.0 is the latest stable as of now)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Enable CTest integration
enable_testing()

# Automatically collect all .cpp files in this folder
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

# Define the test executable
add_executable(idp_tests ${TEST_SOURCES})

# Link with GoogleTest's main runner
target_link_libraries(idp_tests
    PRIVATE
        gtest_main
        dap
)

# Discover and register the tests for `ctest`
include(GoogleTest)
gtest_discover_tests(idp_tests)
