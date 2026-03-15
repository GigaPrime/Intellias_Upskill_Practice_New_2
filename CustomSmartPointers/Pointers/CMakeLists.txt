add_library(Pointers INTERFACE)

target_include_directories(Pointers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(Pointers INTERFACE cxx_std_17)
