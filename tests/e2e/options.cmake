# -h / -v options and command line errors.

include("${CMAKE_CURRENT_LIST_DIR}/common.cmake")

make_example_tree("${WORK}/example")

# help: printed to stdout, exit code 0, nothing is hashed
foreach(flag -h --help)
    run_mangen(H ARGS ${flag})
    expect_success(H "mangen ${flag}")
    expect_contains("${H_OUT}" "Usage: mangen [DIR_PATH] [OPTIONS]" "mangen ${flag}")
    expect_contains("${H_OUT}" "-e, --exclude" "mangen ${flag}")
    expect_equal("${H_ERR}" "" "stderr of mangen ${flag}")
endforeach()
run_mangen(H ARGS example -h)
expect_success(H "mangen example -h")
expect_contains("${H_OUT}" "Usage:" "mangen example -h")
expect_not_contains("${H_OUT}" "file_1.txt" "mangen example -h must not print a manifest")

# version: version number and git commit hash
foreach(flag -v --version)
    run_mangen(V ARGS ${flag})
    expect_success(V "mangen ${flag}")
    if(NOT V_OUT MATCHES "^mangen [0-9]+\\.[0-9]+\\.[0-9]+")
        fail("mangen ${flag}: no version number in:\n${V_OUT}")
    endif()
    expect_contains("${V_OUT}" "\ncommit " "mangen ${flag}")
    # the build passes the hash it embedded into the binary
    if(EXPECTED_COMMIT)
        expect_contains("${V_OUT}" "commit ${EXPECTED_COMMIT}\n" "mangen ${flag} commit hash")
    endif()
endforeach()

# errors: non-zero exit code, message on stderr, nothing on stdout
macro(expect_cli_error what)
    run_mangen(E ARGS ${ARGN})
    expect_failure(E "${what}")
    expect_equal("${E_OUT}" "" "stdout of ${what}")
    if(E_ERR STREQUAL "")
        fail("${what}: no error message on stderr")
    endif()
endmacro()

expect_cli_error("missing directory" does_not_exist)
expect_contains("${E_ERR}" "does_not_exist" "missing directory message")

expect_cli_error("file instead of directory" example/file_1.txt)
expect_contains("${E_ERR}" "example/file_1.txt" "file instead of directory message")

expect_cli_error("unknown flag" -x)
expect_contains("${E_ERR}" "-x" "unknown flag message")
expect_contains("${E_ERR}" "mangen [DIR_PATH] [OPTIONS]" "usage after unknown flag")

expect_cli_error("unknown long flag" example --frobnicate)
expect_cli_error("-e without value" example -e)
expect_cli_error("-e followed by a flag" example -e -h)
expect_cli_error("two directories" example example)
