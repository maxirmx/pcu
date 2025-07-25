# FuelFlux C++ Client Library

This is an initial **skeleton** implementation of a C++17 client library that talks
to the FuelFlux cloud API and runs comfortably on low‑end ARM SBCs such as the
Orange Pi.

The goals of this version are:

* A **clean, testable code structure** built with CMake.
* Separation of concerns between HTTP I/O, in‑memory cache, offline
  persistence and background synchronisation.
* A small dependency set:

  | Purpose           | Library              |
  |-------------------|----------------------|
  | HTTP/HTTPS        | [libcurl]            |
  | JSON              | [nlohmann/json]      |
  | Persistent cache  | `sqlite3`            |
  | Threads/timers    | Standard C++17 STL   |

> **⚠ Important:**  All network calls and data processing are currently **stubbed**;
> the code compiles and links but it does **not** yet execute real requests.
> The TODO comments mark the places where production logic should be added.

## Build

```bash
git clone <this‑repo>
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./example            # small demo program
```

Cross‑compiling for an Orange Pi (ARM HF/ARM64):

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-arm-toolchain.cmake>
```

## Directory layout

```
include/            # public headers
src/                # library implementation
samples/            # demo program
third_party/        # fetched automatically by CMake
```

Happy hacking 🚀
