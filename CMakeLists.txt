# Top-level CMakeLists.txt

cmake_minimum_required(VERSION 3.25)
project(idp-dbg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force static libraries for everything
set(BUILD_SHARED_LIBS OFF)

# We want a debug build by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# --- Fetch all external dependencies ---
include(FetchContent)

FetchContent_Declare(
  z80suzuki
  GIT_REPOSITORY https://github.com/suzukiplan/z80.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(z80suzuki)

FetchContent_Declare(
  sockpp
  GIT_REPOSITORY https://github.com/fpagliughi/sockpp.git
  GIT_TAG        v0.8.0
)
FetchContent_MakeAvailable(sockpp)

FetchContent_Declare(
  structopt
  GIT_REPOSITORY https://github.com/p-ranav/structopt.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(structopt)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# --- Add subdirectories ---
add_subdirectory(lib)
add_subdirectory(src)

option(BUILD_TESTS "Whether or not to build tests." ON)
if (BUILD_TESTS)
  message(STATUS "Building tests...")
  add_subdirectory(tests)
endif()
