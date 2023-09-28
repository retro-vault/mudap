project(idp-dbg)

add_subdirectory(z80ex)

# Remember Z80EX_PATH for linking and including.
set (Z80EX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/z80ex"
    CACHE PATH "Path to Z80Ex library")