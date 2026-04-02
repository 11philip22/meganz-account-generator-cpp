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

if(MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR)
    set(_runtime_targets
        request_waiter_test
        mail_wrapper_detail_test
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
        request_waiter_test
        mail_wrapper_detail_test
        mail_header_self_sufficient
        mega_api_client_interface_test
    )

    foreach(target_name IN LISTS _apple_link_targets)
        target_link_options("${target_name}" PRIVATE LINKER:-no_warn_duplicate_libraries)
    endforeach()
endif()
