include(FetchContent)

# --- z80suzuki (header-only) ---
FetchContent_Declare(
  z80suzuki
  GIT_REPOSITORY https://github.com/suzukiplan/z80.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(z80suzuki)

# --- sockpp ---
FetchContent_Declare(
  sockpp
  GIT_REPOSITORY https://github.com/fpagliughi/sockpp.git
  GIT_TAG        v0.8.0
)
FetchContent_MakeAvailable(sockpp)

# --- structopt ---
FetchContent_Declare(
  structopt
  GIT_REPOSITORY https://github.com/p-ranav/structopt.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(structopt)

# nlohmann/json (header-only)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# --- your executable ---
add_executable(idp-dbg main.cpp)

# Output to top-level bin/ directory
set_target_properties(idp-dbg PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

# Link libraries and include headers
target_link_libraries(idp-dbg
  PRIVATE
    sockpp
    structopt::structopt
    nlohmann_json::nlohmann_json
)

# Include the header-only Suzuki Z80
target_include_directories(idp-dbg
  PRIVATE
    ${z80suzuki_SOURCE_DIR}
)
