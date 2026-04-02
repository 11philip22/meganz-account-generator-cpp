add_executable(
    pass1_smoke
    pass1_smoke.cpp
)

target_link_libraries(
    pass1_smoke
    PRIVATE
    meganz_account_generator_cpp::internal
)

if(MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR)
    set_target_properties(
        pass1_smoke
        PROPERTIES
        BUILD_RPATH "${MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR}"
    )
endif()

if(APPLE AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.0")
    target_link_options(pass1_smoke PRIVATE LINKER:-no_warn_duplicate_libraries)
endif()
