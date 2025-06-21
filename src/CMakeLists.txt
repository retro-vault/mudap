include(FetchContent)

# z80ex
FetchContent_Declare(
  z80ex
  GIT_REPOSITORY https://github.com/z80ex/z80ex.git
  GIT_TAG        master  # or a commit/tag you prefer
)
FetchContent_MakeAvailable(z80ex)

# structopt
FetchContent_Declare(
  structopt
  GIT_REPOSITORY https://github.com/p-ranav/structopt.git
  GIT_TAG        master  # or a stable version
)
FetchContent_MakeAvailable(structopt)

add_executable(idp-dbg main.cpp ...)  # your source files

target_link_libraries(idp-dbg
  PRIVATE
    z80ex
    structopt::structopt  # if structopt defines an imported target
)
