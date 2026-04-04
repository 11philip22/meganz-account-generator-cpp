function(
    meganz_account_generator_configure_guerrillamail_client_c
    bundled_root
    out_target
    out_runtime_dir
    out_build_target
)
    set(
        GUERRILLAMAIL_CLIENT_C_PROFILE
        "Debug"
        CACHE STRING
        "Cargo profile used when building the bundled guerrillamail-client-c submodule"
    )
    set_property(
        CACHE GUERRILLAMAIL_CLIENT_C_PROFILE
        PROPERTY STRINGS Debug Release
    )

    get_filename_component(
        _root
        "${bundled_root}"
        ABSOLUTE
        BASE_DIR "${CMAKE_SOURCE_DIR}"
    )

    if(NOT EXISTS "${_root}/Cargo.toml")
        message(
            FATAL_ERROR
            "The bundled guerrillamail-client-c source tree does not contain Cargo.toml: ${_root}"
        )
    endif()

    if(NOT EXISTS "${_root}/include/guerrillamail_client.h")
        message(
            FATAL_ERROR
            "The bundled guerrillamail-client-c source tree does not contain "
            "include/guerrillamail_client.h: ${_root}"
        )
    endif()

    if(NOT GUERRILLAMAIL_CLIENT_C_PROFILE STREQUAL "Debug"
       AND NOT GUERRILLAMAIL_CLIENT_C_PROFILE STREQUAL "Release")
        message(
            FATAL_ERROR
            "GUERRILLAMAIL_CLIENT_C_PROFILE must be Debug or Release."
        )
    endif()

    find_program(CARGO_EXECUTABLE cargo REQUIRED)

    set(_cargo_args build)
    if(GUERRILLAMAIL_CLIENT_C_PROFILE STREQUAL "Release")
        list(APPEND _cargo_args --release)
        set(_cargo_target_dir "${_root}/target/release")
    else()
        set(_cargo_target_dir "${_root}/target/debug")
    endif()

    set(_target_name "guerrillamail_client_c::client")
    set(_runtime_dir "")
    set(_build_target guerrillamail_client_c_build)

    if(WIN32)
        set(_static_lib "${_cargo_target_dir}/guerrillamail_client_c.lib")
    else()
        set(_static_lib "${_cargo_target_dir}/libguerrillamail_client_c.a")
    endif()

    set(_build_outputs "${_static_lib}")

    add_custom_target(
        "${_build_target}"
        COMMAND "${CARGO_EXECUTABLE}" ${_cargo_args}
        BYPRODUCTS ${_build_outputs}
        WORKING_DIRECTORY "${_root}"
        COMMENT "Building bundled guerrillamail-client-c with Cargo"
        VERBATIM
    )

    add_library("${_target_name}" STATIC IMPORTED GLOBAL)
    set_target_properties(
        "${_target_name}"
        PROPERTIES
        IMPORTED_LOCATION "${_static_lib}"
        INTERFACE_INCLUDE_DIRECTORIES "${_root}/include"
    )

    if(WIN32)
        set_property(
            TARGET "${_target_name}"
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES ntdll
        )
    endif()

    set(${out_target} "${_target_name}" PARENT_SCOPE)
    set(${out_runtime_dir} "${_runtime_dir}" PARENT_SCOPE)
    set(${out_build_target} "${_build_target}" PARENT_SCOPE)
endfunction()
