project(idp-dbg)
add_executable(idp-emu main.cpp)
target_link_libraries(idp-emu PRIVATE z80ex-static)
target_include_directories(idp-emu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/z80ex/include)