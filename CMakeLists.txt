cmake_minimum_required(VERSION 3.22)

project(
    meganz_account_generator_cpp
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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
set(
    MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_GUERRILLAMAIL_CPP_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/guerrillamail-cpp"
)

if(NOT EXISTS "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_SDK_ROOT}/CMakeLists.txt")
    message(
        FATAL_ERROR
        "The bundled meganz/sdk source tree is missing: "
        "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_SDK_ROOT}. "
        "Initialize the submodule with 'git submodule update --init --recursive'."
    )
endif()

if(NOT EXISTS "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_GUERRILLAMAIL_CPP_ROOT}/CMakeLists.txt")
    message(
        FATAL_ERROR
        "The bundled guerrillamail-cpp source tree is missing: "
        "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_GUERRILLAMAIL_CPP_ROOT}. "
        "Initialize the submodule with 'git submodule update --init --recursive'."
    )
endif()

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

add_subdirectory(
    "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_SDK_ROOT}"
    "${CMAKE_BINARY_DIR}/third_party/meganz_sdk"
)

add_subdirectory(
    "${MEGANZ_ACCOUNT_GENERATOR_CPP_BUNDLED_GUERRILLAMAIL_CPP_ROOT}"
    "${CMAKE_BINARY_DIR}/third_party/guerrillamail_cpp"
)

add_subdirectory(src)
