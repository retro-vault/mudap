cmake_minimum_required(VERSION 3.25)
project(idp-dbg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(src)

option(BUILD_TESTS "Whether or not to build tests." ON)
if (BUILD_TESTS)
  message(STATUS "Building tests...")
  add_subdirectory(tests)
endif()
