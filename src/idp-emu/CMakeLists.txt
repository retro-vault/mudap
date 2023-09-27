project(idp-dbg)

set(CMAKE_BUILD_TYPE "Debug")

add_executable(idp-emu main.cpp)

# Add z80ex.
set (Z80EX_PATH "${PROJECT_SOURCE_DIR}/../../lib/z80ex"
    CACHE PATH "Path to Z80Ex library")
include_directories("${Z80EX_PATH}/include")
set (Z80EX_BINARY_DIR "${PROJECT_BINARY_DIR}/lib_z80ex")
make_directory (${Z80EX_BINARY_DIR})
set (Z80EX_STATIC_ONLY true)
add_subdirectory (${Z80EX_PATH} ${Z80EX_BINARY_DIR})
link_directories (${Z80EX_BINARY_DIR})
target_link_libraries (idp-emu z80ex-static z80ex_dasm-static)