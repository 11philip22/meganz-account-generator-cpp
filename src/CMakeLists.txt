add_library(
    meganz_account_generator_cpp
    STATIC
    account_generator.cpp
    mega/mega_api_client.cpp
    mega/request_waiter.cpp
)

add_library(
    meganz_account_generator_cpp::library
    ALIAS
    meganz_account_generator_cpp
)

target_compile_features(
    meganz_account_generator_cpp
    PUBLIC
    cxx_std_20
)

if(MSVC)
    target_compile_options(
        meganz_account_generator_cpp
        PRIVATE
        /W4
        /permissive-
    )
    if(MEGANZ_ACCOUNT_GENERATOR_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(
            meganz_account_generator_cpp
            PRIVATE
            /WX
        )
    endif()
else()
    target_compile_options(
        meganz_account_generator_cpp
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
    if(MEGANZ_ACCOUNT_GENERATOR_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(
            meganz_account_generator_cpp
            PRIVATE
            -Werror
        )
    endif()
endif()

target_include_directories(
    meganz_account_generator_cpp
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    meganz_account_generator_cpp
    PRIVATE
    MEGA::SDKlib
    guerrillamail::guerrillamail-cpp
)

add_executable(
    meganz_account_generator_cpp_cli
    cli/main.cpp
)

target_link_libraries(
    meganz_account_generator_cpp_cli
    PRIVATE
    meganz_account_generator_cpp::library
)

if(APPLE AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
    target_link_options(
        meganz_account_generator_cpp_cli
        PRIVATE
        LINKER:-no_warn_duplicate_libraries
    )
endif()
