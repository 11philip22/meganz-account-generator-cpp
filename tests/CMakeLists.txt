add_executable(
    request_waiter_test
    request_waiter_test.cpp
)

target_link_libraries(
    request_waiter_test
    PRIVATE
    meganz_account_generator_cpp::internal
)

add_test(
    NAME request_waiter_test
    COMMAND request_waiter_test
)

add_executable(
    mail_wrapper_detail_test
    mail_wrapper_detail_test.cpp
)

target_link_libraries(
    mail_wrapper_detail_test
    PRIVATE
    meganz_account_generator_cpp::internal
)

add_test(
    NAME mail_wrapper_detail_test
    COMMAND mail_wrapper_detail_test
)

add_executable(
    mail_header_self_sufficient
    mail_header_self_sufficient.cpp
)

target_include_directories(
    mail_header_self_sufficient
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(
    mail_header_self_sufficient
    PRIVATE
    meganz_account_generator_cpp::project_options
)

add_test(
    NAME mail_header_self_sufficient
    COMMAND mail_header_self_sufficient
)

add_executable(
    mega_api_client_interface_test
    mega_api_client_interface_test.cpp
)

target_include_directories(
    mega_api_client_interface_test
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(
    mega_api_client_interface_test
    PRIVATE
    MEGA::SDKlib
    meganz_account_generator_cpp::project_options
)

add_test(
    NAME mega_api_client_interface_test
    COMMAND mega_api_client_interface_test
)

add_executable(
    account_generator_detail_test
    account_generator_detail_test.cpp
)

target_link_libraries(
    account_generator_detail_test
    PRIVATE
    meganz_account_generator_cpp::internal
)

add_test(
    NAME account_generator_detail_test
    COMMAND account_generator_detail_test
)

add_executable(
    public_header_self_sufficient
    public_header_self_sufficient.cpp
)

target_include_directories(
    public_header_self_sufficient
    PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(
    public_header_self_sufficient
    PRIVATE
    meganz_account_generator_cpp::project_options
)

add_test(
    NAME public_header_self_sufficient
    COMMAND public_header_self_sufficient
)

add_executable(
    public_api_usage_test
    public_api_usage_test.cpp
)

target_link_libraries(
    public_api_usage_test
    PRIVATE
    meganz_account_generator_cpp::library
)

add_test(
    NAME public_api_usage_test
    COMMAND public_api_usage_test
)

add_executable(
    account_generator_e2e_test
    account_generator_e2e_test.cpp
)

target_link_libraries(
    account_generator_e2e_test
    PRIVATE
    meganz_account_generator_cpp::library
)

add_test(
    NAME account_generator_e2e_test
    COMMAND account_generator_e2e_test
)

set_tests_properties(
    account_generator_e2e_test
    PROPERTIES
    SKIP_RETURN_CODE 77
)

add_test(
    NAME cli_help_test
    COMMAND
    "${CMAKE_COMMAND}"
    -DCLI_PATH=$<TARGET_FILE:meganz_account_generator_cpp_cli>
    -P
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_help_test.cmake"
)

add_executable(
    cli_random_app_key_test
    cli_random_app_key_test.cpp
    "${PROJECT_SOURCE_DIR}/src/cli/random_app_key.cpp"
)

target_include_directories(
    cli_random_app_key_test
    PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(
    cli_random_app_key_test
    PRIVATE
    meganz_account_generator_cpp::project_options
)

add_test(
    NAME cli_random_app_key_test
    COMMAND cli_random_app_key_test
)

if(MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR)
    set(_runtime_targets
        account_generator_detail_test
        account_generator_e2e_test
        request_waiter_test
        mail_wrapper_detail_test
        meganz_account_generator_cpp_cli
        public_api_usage_test
        cli_random_app_key_test
    )

    foreach(target_name IN LISTS _runtime_targets)
        set_target_properties(
            "${target_name}"
            PROPERTIES
            BUILD_RPATH "${MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR}"
        )
    endforeach()
endif()

if(APPLE AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
    set(_apple_link_targets
        account_generator_detail_test
        account_generator_e2e_test
        request_waiter_test
        mail_wrapper_detail_test
        mail_header_self_sufficient
        mega_api_client_interface_test
        meganz_account_generator_cpp_cli
        public_header_self_sufficient
        public_api_usage_test
        cli_random_app_key_test
    )

    foreach(target_name IN LISTS _apple_link_targets)
        target_link_options("${target_name}" PRIVATE LINKER:-no_warn_duplicate_libraries)
    endforeach()
endif()
