project(idp-dbg)

# Remember Z80EX_PATH for linking and including.
set (Z80EX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/z80ex"
    CACHE PATH "Path to Z80Ex library")
add_subdirectory(z80ex)

# Make sure we link the static library for sockpp.
option(SOCKPP_BUILD_STATIC "" ON)
option(SOCKPP_BUILD_SHARED "" OFF)
add_subdirectory(sockpp)

# Cmd line args library.
add_subdirectory(structopt)
