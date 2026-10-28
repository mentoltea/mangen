# Helpers shared by end-to-end tests. Each test is a CMake script run as
#     cmake -DMANGEN=<binary> -DWORK=<work dir> -P <test>.cmake
# It builds a directory tree inside WORK, runs mangen and compares its output.
# Failed checks are reported with SEND_ERROR, so all of them are shown and
# the script still exits with a non-zero code.

if(NOT MANGEN OR NOT WORK)
    message(FATAL_ERROR "MANGEN and WORK must be defined")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/../reset_dir.cmake")
reset_dir("${WORK}")

# Manifest of the example tree from the task (contents: "content of <name>\n").
# Hashes are CRC-32 values computed independently with zlib.
set(EXAMPLE_MANIFEST
"file_1.txt : 435657D4
file_2.png : 33E42C7D
file_3.c : 7FDA81B0
dir_1/file_4 : 4A327460
dir_1/file_5.sh : 3D64306B
dir_2/file_6.h : ABF0A849
dir_2/dir_3/file_7.txt : 950FB4C9
")

# make_example_tree(root): the tree from the task statement.
function(make_example_tree root)
    foreach(file
            file_1.txt file_2.png file_3.c
            dir_1/file_4 dir_1/file_5.sh
            dir_2/file_6.h dir_2/dir_3/file_7.txt)
        get_filename_component(name "${file}" NAME)
        file(WRITE "${root}/${file}" "content of ${name}\n")
    endforeach()
    file(MAKE_DIRECTORY "${root}/dir_2/dir_4")   # empty directory
endfunction()

# run_mangen(<prefix> [WORKDIR dir] ARGS arg...)
# Sets <prefix>_OUT, <prefix>_ERR (line endings normalized) and <prefix>_RC.
function(run_mangen prefix)
    cmake_parse_arguments(RUN "" "WORKDIR" "ARGS" ${ARGN})
    if(NOT RUN_WORKDIR)
        set(RUN_WORKDIR "${WORK}")
    endif()
    execute_process(
        COMMAND "${MANGEN}" ${RUN_ARGS}
        WORKING_DIRECTORY "${RUN_WORKDIR}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE rc
    )
    string(REPLACE "\r\n" "\n" out "${out}")
    string(REPLACE "\r\n" "\n" err "${err}")
    set(${prefix}_OUT "${out}" PARENT_SCOPE)
    set(${prefix}_ERR "${err}" PARENT_SCOPE)
    set(${prefix}_RC "${rc}" PARENT_SCOPE)
endfunction()

function(fail what)
    message(SEND_ERROR "FAILED: ${what}")
endfunction()

function(expect_equal actual expected what)
    if(NOT "${actual}" STREQUAL "${expected}")
        fail("${what}\n--- got:\n${actual}\n--- expected:\n${expected}\n---")
    endif()
endfunction()

function(expect_contains text needle what)
    string(FIND "${text}" "${needle}" pos)
    if(pos EQUAL -1)
        fail("${what}: '${needle}' not found in:\n${text}")
    endif()
endfunction()

function(expect_not_contains text needle what)
    string(FIND "${text}" "${needle}" pos)
    if(NOT pos EQUAL -1)
        fail("${what}: unexpected '${needle}' in:\n${text}")
    endif()
endfunction()

function(expect_success prefix what)
    if(NOT "${${prefix}_RC}" STREQUAL "0")
        fail("${what}: exit code ${${prefix}_RC}, stderr:\n${${prefix}_ERR}")
    endif()
endfunction()

function(expect_failure prefix what)
    if("${${prefix}_RC}" STREQUAL "0")
        fail("${what}: expected non-zero exit code")
    endif()
endfunction()

# expect_manifest(<args...> EXPECT <text>): successful run, exact stdout, empty stderr.
function(expect_manifest)
    cmake_parse_arguments(M "" "EXPECT;WORKDIR" "ARGS" ${ARGN})
    if(M_WORKDIR)
        run_mangen(R WORKDIR "${M_WORKDIR}" ARGS ${M_ARGS})
    else()
        run_mangen(R ARGS ${M_ARGS})
    endif()
    expect_success(R "mangen ${M_ARGS}")
    expect_equal("${R_OUT}" "${M_EXPECT}" "stdout of 'mangen ${M_ARGS}'")
    expect_equal("${R_ERR}" "" "stderr of 'mangen ${M_ARGS}'")
endfunction()
