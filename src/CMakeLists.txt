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

add_subdirectory(smoke)
