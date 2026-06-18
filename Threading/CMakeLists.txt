# CMakeList.txt : CMake project for Threading, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


project ("Threading")

# Add source to this project's executable.
set(SOURCES
	"Task.h"
	"ThreadPool.h"
	"Timer.h"
	"Task.cpp"
	"ThreadPool.cpp"
	"Timer.cpp"
	"TaskData.cpp"
)

include(FetchContent)

# Download and make GoogleTest available via FetchContent
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
	# Use the main branch to avoid tag checkout issues on some environments
  GIT_TAG main
)

# For MSVC: use shared runtime consistent with the project
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

list(APPEND SOURCES "main.cpp" "TimerTests.cpp")

add_executable(Threading ${SOURCES})

target_link_libraries(Threading PRIVATE GTest::gtest_main)

enable_testing()
include(GoogleTest)

# Register GoogleTest tests with CTest so Visual Studio Test Explorer can discover them
# Ensure the test executable is placed in the top-level build directory so Visual Studio
# Test Explorer can find and run it consistently.
set_target_properties(Threading PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
)

# Discover GoogleTest tests and register them with CTest. Set the working directory
# to the build directory to make tests runnable from Visual Studio Test Explorer.
gtest_discover_tests(Threading WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Threading PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
