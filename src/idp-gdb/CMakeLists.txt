project(idp-dbg)

# We want a debug version for the IDE.
set(CMAKE_BUILD_TYPE "Debug")

# Our main executable.
add_executable(idp-gdb main.cpp)

# Add include dirs: z80ex, 
include_directories(
    ${Z80EX_PATH}/include
    ${SOCKPP_INCLUDE_DIR})

# Add all libs: z80ex-static, z80ex_dasm-static, 
target_link_libraries (idp-gdb 
    z80ex_dasm-static
    ${SOCKPP_STATIC_LIBRARY}
)