add_library(optional INTERFACE)
add_library(VT::optional ALIAS optional)

target_compile_features(optional INTERFACE cxx_std_17)

target_include_directories(optional
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
    target_compile_options(optional INTERFACE /W4 /permissive-)
else()
    target_compile_options(optional INTERFACE -Wall -Wextra -Wpedantic)
endif()