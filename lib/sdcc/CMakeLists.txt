# lib/sdcc/CMakeLists.txt

# Collect all .cpp source files recursively in this directory and subdirectories
file(GLOB_RECURSE SDCC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Add the SCDC library sources.
add_library(sdcc STATIC ${SDCC_SOURCES})

# Public headers are expected to be in include/dap/
target_include_directories(sdcc
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Require C++23 for modern features
target_compile_features(dap PUBLIC cxx_std_23)