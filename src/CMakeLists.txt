add_library(
    meganz_account_generator_cpp_internal
    STATIC
    core/account_generator.cpp
    mail/guerrillamail_client.cpp
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
    "${MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_TARGET}"
    ICU::i18n
    ICU::uc
    ICU::data
)

if(MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_BUILD_TARGET)
    add_dependencies(
        meganz_account_generator_cpp_internal
        "${MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_BUILD_TARGET}"
    )
endif()

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
    cli/random_app_key.cpp
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
)

if(MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR)
    set_target_properties(
        meganz_account_generator_cpp_cli
        PROPERTIES
        BUILD_RPATH "${MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR}"
    )
endif()

if(APPLE AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
    target_link_options(
        meganz_account_generator_cpp_cli
        PRIVATE
        LINKER:-no_warn_duplicate_libraries
    )
endif()
