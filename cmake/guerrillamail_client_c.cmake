function(
    meganz_account_generator_configure_guerrillamail_client_c
    out_target
    out_runtime_dir
    out_build_target
)
    set(
        GUERRILLAMAIL_CLIENT_C_ROOT
        ""
        CACHE PATH
        "Path to a local guerrillamail-client-c repository root"
    )
    set(
        GUERRILLAMAIL_CLIENT_C_PROFILE
        "Debug"
        CACHE STRING
        "Cargo profile used when building guerrillamail-client-c from source"
    )
    set_property(
        CACHE GUERRILLAMAIL_CLIENT_C_PROFILE
        PROPERTY STRINGS Debug Release
    )
    set(
        GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR
        ""
        CACHE PATH
        "Include directory containing guerrillamail_client.h"
    )
    set(
        GUERRILLAMAIL_CLIENT_C_LIBRARY
        ""
        CACHE FILEPATH
        "Path to a prebuilt guerrillamail-client-c library file"
    )
    set(
        GUERRILLAMAIL_CLIENT_C_IMPLIB
        ""
        CACHE FILEPATH
        "Windows import library for guerrillamail-client-c when linking against a DLL"
    )

    set(_use_root OFF)
    set(_use_explicit OFF)

    if(GUERRILLAMAIL_CLIENT_C_ROOT)
        set(_use_root ON)
    endif()

    if(GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR OR GUERRILLAMAIL_CLIENT_C_LIBRARY)
        set(_use_explicit ON)
    endif()

    if(_use_root AND _use_explicit)
        message(
            FATAL_ERROR
            "Choose exactly one GuerrillaMail configuration mode: "
            "GUERRILLAMAIL_CLIENT_C_ROOT or "
            "GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR + GUERRILLAMAIL_CLIENT_C_LIBRARY."
        )
    endif()

    if(NOT _use_root AND NOT _use_explicit)
        message(
            FATAL_ERROR
            "GuerrillaMail is not configured. Provide either "
            "GUERRILLAMAIL_CLIENT_C_ROOT or "
            "GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR + GUERRILLAMAIL_CLIENT_C_LIBRARY."
        )
    endif()

    set(_target_name "guerrillamail_client_c::client")
    set(_runtime_dir "")
    set(_build_target "")

    if(_use_root)
        get_filename_component(
            _root
            "${GUERRILLAMAIL_CLIENT_C_ROOT}"
            ABSOLUTE
            BASE_DIR "${CMAKE_SOURCE_DIR}"
        )

        if(NOT EXISTS "${_root}/Cargo.toml")
            message(
                FATAL_ERROR
                "GUERRILLAMAIL_CLIENT_C_ROOT does not contain Cargo.toml: ${_root}"
            )
        endif()

        if(NOT EXISTS "${_root}/include/guerrillamail_client.h")
            message(
                FATAL_ERROR
                "GUERRILLAMAIL_CLIENT_C_ROOT does not contain "
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

        set(_build_target guerrillamail_client_c_build)
        add_custom_target(
            "${_build_target}"
            COMMAND "${CARGO_EXECUTABLE}" ${_cargo_args}
            WORKING_DIRECTORY "${_root}"
            COMMENT "Building guerrillamail-client-c with Cargo"
            VERBATIM
        )

        if(WIN32)
            set(_shared_lib "${_cargo_target_dir}/guerrillamail_client_c.dll")
            if(MSVC)
                set(_import_lib "${_cargo_target_dir}/guerrillamail_client_c.dll.lib")
            else()
                set(_import_lib "${_cargo_target_dir}/libguerrillamail_client_c.dll.a")
            endif()

            add_library("${_target_name}" SHARED IMPORTED GLOBAL)
            set_target_properties(
                "${_target_name}"
                PROPERTIES
                IMPORTED_LOCATION "${_shared_lib}"
                IMPORTED_IMPLIB "${_import_lib}"
                INTERFACE_INCLUDE_DIRECTORIES "${_root}/include"
            )
        elseif(APPLE)
            set(_shared_lib "${_cargo_target_dir}/libguerrillamail_client_c.dylib")
            add_library("${_target_name}" SHARED IMPORTED GLOBAL)
            set_target_properties(
                "${_target_name}"
                PROPERTIES
                IMPORTED_LOCATION "${_shared_lib}"
                INTERFACE_INCLUDE_DIRECTORIES "${_root}/include"
            )
            set(_runtime_dir "${_cargo_target_dir}")
        else()
            set(_shared_lib "${_cargo_target_dir}/libguerrillamail_client_c.so")
            add_library("${_target_name}" SHARED IMPORTED GLOBAL)
            set_target_properties(
                "${_target_name}"
                PROPERTIES
                IMPORTED_LOCATION "${_shared_lib}"
                INTERFACE_INCLUDE_DIRECTORIES "${_root}/include"
            )
            set(_runtime_dir "${_cargo_target_dir}")
        endif()
    else()
        if(NOT GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR OR NOT GUERRILLAMAIL_CLIENT_C_LIBRARY)
            message(
                FATAL_ERROR
                "Explicit GuerrillaMail configuration requires both "
                "GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR and GUERRILLAMAIL_CLIENT_C_LIBRARY."
            )
        endif()

        get_filename_component(
            _include_dir
            "${GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR}"
            ABSOLUTE
            BASE_DIR "${CMAKE_SOURCE_DIR}"
        )
        get_filename_component(
            _library
            "${GUERRILLAMAIL_CLIENT_C_LIBRARY}"
            ABSOLUTE
            BASE_DIR "${CMAKE_SOURCE_DIR}"
        )

        if(NOT EXISTS "${_include_dir}/guerrillamail_client.h")
            message(
                FATAL_ERROR
                "GUERRILLAMAIL_CLIENT_C_INCLUDE_DIR does not contain "
                "guerrillamail_client.h: ${_include_dir}"
            )
        endif()

        if(NOT EXISTS "${_library}")
            message(
                FATAL_ERROR
                "GUERRILLAMAIL_CLIENT_C_LIBRARY does not exist: ${_library}"
            )
        endif()

        get_filename_component(_library_ext "${_library}" EXT)

        if(WIN32 AND _library_ext STREQUAL ".dll")
            if(NOT GUERRILLAMAIL_CLIENT_C_IMPLIB)
                message(
                    FATAL_ERROR
                    "GUERRILLAMAIL_CLIENT_C_IMPLIB is required on Windows when "
                    "GUERRILLAMAIL_CLIENT_C_LIBRARY points to a DLL."
                )
            endif()

            get_filename_component(
                _implib
                "${GUERRILLAMAIL_CLIENT_C_IMPLIB}"
                ABSOLUTE
                BASE_DIR "${CMAKE_SOURCE_DIR}"
            )
            if(NOT EXISTS "${_implib}")
                message(
                    FATAL_ERROR
                    "GUERRILLAMAIL_CLIENT_C_IMPLIB does not exist: ${_implib}"
                )
            endif()

            add_library("${_target_name}" SHARED IMPORTED GLOBAL)
            set_target_properties(
                "${_target_name}"
                PROPERTIES
                IMPORTED_LOCATION "${_library}"
                IMPORTED_IMPLIB "${_implib}"
                INTERFACE_INCLUDE_DIRECTORIES "${_include_dir}"
            )
        else()
            add_library("${_target_name}" UNKNOWN IMPORTED GLOBAL)
            set_target_properties(
                "${_target_name}"
                PROPERTIES
                IMPORTED_LOCATION "${_library}"
                INTERFACE_INCLUDE_DIRECTORIES "${_include_dir}"
            )
        endif()

        if(_library_ext STREQUAL ".dylib"
           OR _library_ext STREQUAL ".so"
           OR _library_ext STREQUAL ".dll")
            get_filename_component(_runtime_dir "${_library}" DIRECTORY)
        endif()
    endif()

    set(${out_target} "${_target_name}" PARENT_SCOPE)
    set(${out_runtime_dir} "${_runtime_dir}" PARENT_SCOPE)
    set(${out_build_target} "${_build_target}" PARENT_SCOPE)
endfunction()
