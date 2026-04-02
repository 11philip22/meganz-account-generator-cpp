if(NOT DEFINED CLI_PATH)
    message(FATAL_ERROR "CLI_PATH is required")
endif()

execute_process(
    COMMAND "${CLI_PATH}" --help
    RESULT_VARIABLE cli_result
    OUTPUT_VARIABLE cli_output
    ERROR_VARIABLE cli_error
)

if(NOT cli_result EQUAL 0)
    message(
        FATAL_ERROR
        "Expected --help to exit successfully. "
        "Result=${cli_result}\nstdout:\n${cli_output}\nstderr:\n${cli_error}"
    )
endif()

set(cli_combined_output "${cli_output}\n${cli_error}")

foreach(
    expected_fragment
    IN ITEMS
    "Usage:"
    "--password"
    "--display-name"
    "--proxy"
    "--timeout-ms"
    "--poll-interval-ms"
    "--app-key"
    "MEGANZ_ACCOUNT_GENERATOR_CPP_APP_KEY"
)
    string(FIND "${cli_combined_output}" "${expected_fragment}" fragment_index)
    if(fragment_index EQUAL -1)
        message(
            FATAL_ERROR
            "Expected CLI help output to contain '${expected_fragment}'. "
            "Output was:\n${cli_combined_output}"
        )
    endif()
endforeach()
