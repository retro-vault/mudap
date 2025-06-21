# lib/dap/CMakeLists.txt

# Add the DAP (Debug Adapter Protocol) static library
add_library(dap STATIC
    dap.cpp
    request.cpp
    response.cpp
)

# Public headers are expected to be in include/dap/
target_include_directories(dap
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Require C++23 for modern features
target_compile_features(dap PUBLIC cxx_std_23)

# Link against the nlohmann::json library (already fetched in top-level CMake)
target_link_libraries(dap
    PUBLIC
        nlohmann_json::nlohmann_json
)
