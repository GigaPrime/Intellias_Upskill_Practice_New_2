include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(UnitTests ${TEST_SOURCES})

target_link_libraries(UnitTests PRIVATE
    VT::optional
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)

target_compile_features(UnitTests PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(UnitTests)