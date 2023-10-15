project(idp-dbg)

# We want a debug version for the IDE.
set(CMAKE_BUILD_TYPE "Debug")

# Our main executable.
add_executable(idp-emu main.cpp)

# Add include dirs: z80ex, 
include_directories(
    ${Z80EX_PATH}/include
    ${SOCKPP_PATH}/include
)

# Add all libs: z80ex-static, z80ex_dasm-static, 
target_link_libraries (idp-emu 
    z80ex-static
    sockpp-static
    structopt
)