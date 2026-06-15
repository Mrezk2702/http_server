# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## guidance
Refer to HTTP_SERVER_GUIDE.md for the guidance it was created by you
## Project Overview

A C++ HTTP server implementation built on top of a raw TCP layer.

## Repository Layout

```
src/
  main.cpp                  # Entry point
  server/
    tcp_server.hpp/.cpp     # TCP socket layer (accept, send, recv)
tests/                      # Test suite (not yet populated)
benchmarks/                 # Benchmark suite (not yet populated)
docs/                       # Documentation (not yet populated)
```

## Build System

No build system has been set up yet. When adding one, prefer **CMake** with a `CMakeLists.txt` at the repo root. A typical initial setup:

```cmake
cmake_minimum_required(VERSION 3.20)
project(http_server CXX)
set(CMAKE_CXX_STANDARD 20)
add_executable(http_server src/main.cpp src/server/tcp_server.cpp)
```

Build and run:
```bash
cmake -B build && cmake --build build
./build/http_server
```

## Architecture Intent

The design is layered:

1. **TCP layer** (`src/server/tcp_server.*`) — raw socket management: binding, listening, accepting connections, reading/writing bytes.
2. **HTTP layer** (to be added under `src/http/`) — parses raw bytes from the TCP layer into HTTP requests and serializes HTTP responses back.
3. **Router/handler layer** (to be added) — maps parsed requests to handler callbacks.

Keep these layers strictly separated: the TCP server should not know about HTTP, and the HTTP parser should not know about routing.

