add_library(
    meganz_account_generator_cpp_internal
    STATIC
    core/account_generator.cpp
    mega/mega_api_client.cpp
    mega/request_waiter.cpp
)

add_library(
    meganz_account_generator_cpp::internal
    ALIAS
    meganz_account_generator_cpp_internal
)

target_include_directories(
    meganz_account_generator_cpp_internal
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    meganz_account_generator_cpp_internal
    PUBLIC
    meganz_account_generator_cpp::project_options
    MEGA::SDKlib
    guerrillamail::guerrillamail-cpp
    ICU::i18n
    ICU::uc
    ICU::data
)

add_library(
    meganz_account_generator_cpp
    STATIC
    public/account_generator.cpp
)

add_library(
    meganz_account_generator_cpp::library
    ALIAS
    meganz_account_generator_cpp
)

target_include_directories(
    meganz_account_generator_cpp
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    meganz_account_generator_cpp
    PUBLIC
    meganz_account_generator_cpp::project_options
    PRIVATE
    meganz_account_generator_cpp::internal
)

add_executable(
    meganz_account_generator_cpp_cli
    cli/main.cpp
)

target_include_directories(
    meganz_account_generator_cpp_cli
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    meganz_account_generator_cpp_cli
    PRIVATE
    meganz_account_generator_cpp::library
    meganz_account_generator_cpp::internal
)

if(APPLE AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
    target_link_options(
        meganz_account_generator_cpp_cli
        PRIVATE
        LINKER:-no_warn_duplicate_libraries
    )
endif()
