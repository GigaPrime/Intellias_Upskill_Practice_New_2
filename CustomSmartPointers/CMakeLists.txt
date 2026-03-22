cmake_minimum_required(VERSION 3.16)
project(Intellias_Upskill_Practice_New_2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Pointers)
add_subdirectory(_Tests)
