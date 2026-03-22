set(GTEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/packages/googletest-main/googletest-main)

if(EXISTS "${GTEST_SOURCE_DIR}/CMakeLists.txt")
  add_subdirectory("${GTEST_SOURCE_DIR}" "googletest" EXCLUDE_FROM_ALL)
else()
  message(FATAL_ERROR "Local googletest not found at ${GTEST_SOURCE_DIR}")
endif()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(UnitTests 
	${TEST_SOURCES}
)

target_include_directories(UnitTests PRIVATE 
	${CMAKE_SOURCE_DIR}/Pointers
)

target_link_libraries(UnitTests PRIVATE 
	Pointers gtest_main gmock_main
)

find_package(Threads REQUIRED)

if(Threads_FOUND)
  target_link_libraries(UnitTests PRIVATE Threads::Threads)
endif()

enable_testing()
add_test(NAME AllTests COMMAND UnitTests)
