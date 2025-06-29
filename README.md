![status.badge] [![language.badge]][language.url] [![standard.badge]][standard.url] [![license.badge]][license.url]

# idp-dbg

`idp-dbg` is a cross-debugging toolchain for Z80 programs, intended for integration with Visual Studio Code on Linux. It supports the Microsoft Debug Adapter Protocol (DAP). The system is designed to provide future support for emulating the Iskra Delta Partner, an 8-bit computer developed by Iskra Delta in the 1980s.

## Features

- Minimal debug adapter protocol implementation (DAP)
- DeZog-compatible communication over TCP
- Modular architecture using clean C++23 code
- Optional support for hardware emulation of Iskra Delta Partner
- Unit-tested components using GoogleTest
- Dependency management via CMake FetchContent

## Design goals

- Platform-independent implementation using standard C++
- Clear separation between debugger frontend (DAP) and backend (emulator)
- No runtime dependencies other than the C++ standard library
- Usable from VSCode as a native debug adapter (pressing F5 launches it)

## Build instructions

```sh
git clone https://github.com/iskra-delta/idp-dbg.git --recurse-submodules
cd idp-dbg
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build
```

To run tests:

```sh
cd build
ctest --output-on-failure
```

The resulting debug adapter binary is located in the bin/ directory.

## Directory structure

- src/ — main entry point and runtime TCP server
- lib/dap/ — DAP message parsing and response generation
- lib/ — internal libraries
- include/ — public headers for internal libraries
- tests/ — unit tests using GoogleTest

## Dependencies

All dependencies are handled via FetchContent and downloaded automatically:

- nlohmann/json — JSON parsing (header-only)
- sockpp — TCP sockets
- structopt — command-line argument parsing
- suzukiplan/z80 — Z80 CPU emulation (header-only)

## Status

The project is under active development. Initial message parsing, response dispatching, and transport-level communication are in place. Emulator and debugger state handling is forthcoming.

## License

This project is licensed under the [MIT License](LICENSE).

## Copyright

© 2025 Tomaz Stih. All rights reserved.

---

[language.url]: https://en.wikipedia.org/wiki/ANSI_C
[language.badge]: https://img.shields.io/badge/language-C-blue.svg
[standard.url]: https://en.wikipedia.org/wiki/C89/
[standard.badge]: https://img.shields.io/badge/standard-C89-blue.svg
[license.url]: https://github.com/tstih/libcpm3-z80/blob/main/LICENSE
[license.badge]: https://img.shields.io/badge/license-MIT-blue.svg
[status.badge]: https://img.shields.io/badge/status-alpha-orange.svg
