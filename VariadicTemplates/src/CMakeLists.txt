add_library(src INTERFACE)

target_compile_features(src INTERFACE cxx_std_17)

target_include_directories(src
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
    target_compile_options(src
        INTERFACE
            /W4
            /permissive-
    )
endif()
