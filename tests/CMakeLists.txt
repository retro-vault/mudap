include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Automatically add all .cpp files in tests/
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(idp_tests ${TEST_SOURCES})

target_link_libraries(idp_tests
    PRIVATE
        gtest_main
)

# This line is CRUCIAL for CTest to discover tests
include(GoogleTest)
gtest_discover_tests(idp_tests)
