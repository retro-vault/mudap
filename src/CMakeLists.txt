# src/CMakeLists.txt

# Define the main executable
add_executable(idp-dbg main.cpp)

# Place the binary in the top-level /bin folder
set_target_properties(idp-dbg PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

# Link against the external and internal libraries
target_link_libraries(idp-dbg
  PRIVATE
    sockpp
    structopt::structopt
    nlohmann_json::nlohmann_json
    dap  # your in-tree debug adapter protocol library
)

# Include Suzuki Z80 (header-only)
target_include_directories(idp-dbg
  PRIVATE
    ${z80suzuki_SOURCE_DIR}
)
