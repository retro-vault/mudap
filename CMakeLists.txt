cmake_minimum_required(VERSION 3.25)

project(idp-dbg LANGUAGES CXX)

# Set Debug build type by default if not explicitly defined
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

option(BUILD_TESTS "Whether or not to build tests." ON)
if (BUILD_TESTS)
  message(STATUS "Building tests...")
  add_subdirectory(tests)
endif()