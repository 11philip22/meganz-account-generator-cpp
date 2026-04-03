cmake_minimum_required(VERSION 3.20)

project(
    meganz_account_generator_cpp
    VERSION 0.1.0
    LANGUAGES C CXX
)

option(BUILD_TESTING "Build this project's tests" ON)
if(BUILD_TESTING)
    enable_testing()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
    MEGANZ_ACCOUNT_GENERATOR_CPP_WARNINGS_AS_ERRORS
    "Treat this project's warnings as errors"
    OFF
)

add_library(meganz_account_generator_cpp_project_options INTERFACE)
add_library(
    meganz_account_generator_cpp::project_options
    ALIAS
    meganz_account_generator_cpp_project_options
)

target_compile_features(
    meganz_account_generator_cpp_project_options
    INTERFACE
    c_std_11
    cxx_std_20
)

if(MSVC)
    target_compile_options(
        meganz_account_generator_cpp_project_options
        INTERFACE
        /W4
        /permissive-
    )
    if(MEGANZ_ACCOUNT_GENERATOR_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(
            meganz_account_generator_cpp_project_options
            INTERFACE
            /WX
        )
    endif()
else()
    target_compile_options(
        meganz_account_generator_cpp_project_options
        INTERFACE
        -Wall
        -Wextra
        -Wpedantic
    )
    if(MEGANZ_ACCOUNT_GENERATOR_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(
            meganz_account_generator_cpp_project_options
            INTERFACE
            -Werror
        )
    endif()
endif()

set(
    MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_SDK_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/meganz-sdk"
)

if(NOT EXISTS "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_SDK_ROOT}/CMakeLists.txt")
    message(
        FATAL_ERROR
        "The bundled meganz/sdk source tree is missing: "
        "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_SDK_ROOT}. "
        "Initialize the submodule with 'git submodule update --init --recursive'."
    )
endif()

set(
    ENABLE_SDKLIB_EXAMPLES
    OFF
    CACHE BOOL
    "Disable upstream SDK examples in this project"
    FORCE
)
set(
    ENABLE_SDKLIB_TESTS
    OFF
    CACHE BOOL
    "Disable upstream SDK tests in this project"
    FORCE
)
set(
    ENABLE_SDKLIB_WERROR
    OFF
    CACHE BOOL
    "Disable upstream SDK warnings-as-errors in this project"
    FORCE
)
set(
    USE_FREEIMAGE
    OFF
    CACHE BOOL
    "Disable the optional FreeImage dependency by default"
)
set(
    USE_FFMPEG
    OFF
    CACHE BOOL
    "Disable the optional FFmpeg dependency by default"
)
set(
    USE_PDFIUM
    OFF
    CACHE BOOL
    "Disable the optional PDFium dependency by default"
)
set(
    USE_READLINE
    OFF
    CACHE BOOL
    "Disable the optional readline dependency by default"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(
    "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_SDK_ROOT}"
    "${CMAKE_BINARY_DIR}/third_party/meganz_sdk"
)

find_package(ICU REQUIRED COMPONENTS i18n uc data)

include(guerrillamail_client_c)

meganz_account_generator_configure_guerrillamail_client_c(
    MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_TARGET
    MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_RUNTIME_DIR
    MEGANZ_ACCOUNT_GENERATOR_CPP_GUERRILLAMAIL_BUILD_TARGET
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
