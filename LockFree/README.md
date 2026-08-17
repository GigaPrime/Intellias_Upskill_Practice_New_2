# SPSC Queue, UDP Streaming, SQLite Consumer

This project implements a C++17 single-producer/single-consumer lock-free circular queue, fixed-size binary message encoding, asynchronous UDP streaming, and a UDP-facing consumer that batches writes into SQLite.

## Architecture

```text
Producer -> Buffer -> UdpStreamer -> UDP socket -> Consumer -> SqliteBatchWriter
```

- `Message` stores human-friendly `std::string` fields and validates maximum ASCII field lengths.
- `MessageEncoder` / `MessageDecoder` use a fixed 152-byte wire format: 8 bytes ID, 16 bytes header, 128 bytes payload.
- `Buffer` is a fixed-size lock-free SPSC circular queue using atomic indices and one empty slot.
- `Producer` asynchronously generates messages with random 10-100 ms delays.
- `UdpStreamer` asynchronously drains the queue and sends encoded datagrams.
- `Consumer` asynchronously receives UDP datagrams, decodes messages, stores consumed IDs, and writes SQLite rows in batches.

## Build

```powershell
cmake -S . -B build -DSPSC_BUILD_TESTS=ON
cmake --build build --config Release
```

SQLite3 must be available through one of these routes:

1. Install SQLite through a package manager such as vcpkg, then configure CMake with the vcpkg toolchain.

```powershell
vcpkg install sqlite3:x64-windows
cmake -S . -B build -DSPSC_BUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
```

2. Or use the vendored official SQLite amalgamation files included in this project:

```text
third_party/sqlite/sqlite3.c
third_party/sqlite/sqlite3.h
```

The amalgamation option avoids needing a separately installed SQLite library.
The bundled files are from SQLite amalgamation version 3.53.4, downloaded from the official SQLite download page.

If GoogleTest is not installed locally and network access is allowed, configure with:

```powershell
cmake -S . -B build -DSPSC_BUILD_TESTS=ON -DSPSC_FETCH_GTEST=ON
```

## Run Demo

```powershell
.\build\Release\spsc_demo.exe
```

The demo runs producer, UDP streamer, and UDP consumer in one process for five seconds and writes messages to `messages.db`.

## Run Tests

```powershell
ctest --test-dir build --output-on-failure -C Release
```
