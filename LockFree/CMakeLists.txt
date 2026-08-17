cmake_minimum_required(VERSION 3.16)

project(SpscQueueUdpSqlite LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SPSC_BUILD_TESTS "Build unit tests" ON)
option(SPSC_FETCH_GTEST "Download GoogleTest when it is not installed locally" OFF)
set(SPSC_SQLITE_AMALGAMATION_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite"
    CACHE PATH
    "Directory containing sqlite3.c and sqlite3.h from the SQLite amalgamation package"
)

find_package(SQLite3 QUIET)

if (SQLite3_FOUND)
    set(SPSC_SQLITE_TARGET SQLite::SQLite3)
else()
    set(SPSC_SQLITE_C "${SPSC_SQLITE_AMALGAMATION_DIR}/sqlite3.c")
    set(SPSC_SQLITE_H "${SPSC_SQLITE_AMALGAMATION_DIR}/sqlite3.h")

    if (EXISTS "${SPSC_SQLITE_C}" AND EXISTS "${SPSC_SQLITE_H}")
        add_library(sqlite3_amalgamation STATIC "${SPSC_SQLITE_C}")
        target_include_directories(sqlite3_amalgamation PUBLIC "${SPSC_SQLITE_AMALGAMATION_DIR}")
        set(SPSC_SQLITE_TARGET sqlite3_amalgamation)
        message(STATUS "Using SQLite amalgamation from ${SPSC_SQLITE_AMALGAMATION_DIR}")
    else()
        message(FATAL_ERROR
            "SQLite3 was not found. Either install SQLite3 so CMake can find it, "
            "or download the SQLite amalgamation and place sqlite3.c and sqlite3.h in "
            "${SPSC_SQLITE_AMALGAMATION_DIR}. You may also configure "
            "-DSPSC_SQLITE_AMALGAMATION_DIR=<path-to-folder-containing-sqlite3.c-and-sqlite3.h>."
        )
    endif()
endif()

add_library(spsc_queue
    src/Buffer.cpp
    src/Consumer.cpp
    src/Message.cpp
    src/MessageDecoder.cpp
    src/MessageEncoder.cpp
    src/Producer.cpp
    src/SqliteBatchWriter.cpp
    src/UdpSocket.cpp
    src/UdpStreamer.cpp
)

target_include_directories(spsc_queue
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/header
)

target_link_libraries(spsc_queue
    PUBLIC
        ${SPSC_SQLITE_TARGET}
)

if (WIN32)
    target_link_libraries(spsc_queue PUBLIC ws2_32)
endif()

target_compile_features(spsc_queue PUBLIC cxx_std_17)

if (MSVC)
    target_compile_options(spsc_queue PRIVATE /W4 /permissive-)
else()
    target_compile_options(spsc_queue PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(spsc_demo src/main.cpp)
target_link_libraries(spsc_demo PRIVATE spsc_queue)

if (SPSC_BUILD_TESTS)
    include(CTest)

    find_package(GTest QUIET)

    if (NOT GTest_FOUND AND SPSC_FETCH_GTEST)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    if (GTest_FOUND OR TARGET GTest::gtest_main OR TARGET gtest_main)
        add_executable(spsc_tests
            tests/BufferTests.cpp
            tests/ConsumerIntegrationTests.cpp
            tests/MessageCodecTests.cpp
        )

        target_link_libraries(spsc_tests PRIVATE spsc_queue)

        if (TARGET GTest::gtest_main)
            target_link_libraries(spsc_tests PRIVATE GTest::gtest_main)
        else()
            target_link_libraries(spsc_tests PRIVATE gtest_main)
        endif()

        include(GoogleTest)
        gtest_discover_tests(spsc_tests)
    else()
        message(WARNING "GoogleTest was not found. Set SPSC_FETCH_GTEST=ON to download it, or install GTest locally.")
    endif()
endif()
