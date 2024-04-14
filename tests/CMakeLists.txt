project(idp-dbg)

# We want a debug version for the IDE.
set(CMAKE_BUILD_TYPE "Debug")

# Include files.
include_directories(${GTEST_PATH}/include)

# Test executable.
add_executable(gdb-tests main.cpp test-executive.cpp)

# Link in google tests.
target_link_libraries(gdb-tests gtest gtest_main)
