# POSIX-only: symlinks, special files and unreadable files/directories.
# Errors must go to stderr while the rest of the tree is still processed.

include("${CMAKE_CURRENT_LIST_DIR}/common.cmake")

set(root "${WORK}/tree")
file(WRITE "${root}/real.txt" "a")
file(WRITE "${root}/sub/inner.txt" "abc")

function(sh)
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${root}" RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "setup command failed: ${ARGN}")
    endif()
endfunction()

sh(ln -s real.txt file_link)     # link to a file: hashed like the file
sh(ln -s sub dir_link)           # link to a directory: not followed
sh(ln -s .. sub/loop)            # cycle: must not hang

# not a regular file: skipped (and not opened); some file systems
# (e.g. Windows drives mounted into WSL) cannot hold FIFOs
execute_process(COMMAND mkfifo fifo WORKING_DIRECTORY "${root}"
                RESULT_VARIABLE fifo_rc OUTPUT_QUIET ERROR_QUIET)
if(NOT fifo_rc EQUAL 0)
    message(STATUS "FIFOs not supported here: FIFO check skipped")
endif()

expect_manifest(ARGS tree EXPECT
"file_link : E8B7BE43
real.txt : E8B7BE43
sub/inner.txt : 352441C2
")

# a broken link is reported, the rest is still printed
sh(ln -s missing dangling)
run_mangen(R ARGS tree)
expect_contains("${R_ERR}" "dangling" "broken symlink error")
expect_equal("${R_OUT}"
"file_link : E8B7BE43
real.txt : E8B7BE43
sub/inner.txt : 352441C2
" "stdout with a broken symlink")
file(REMOVE "${root}/dangling")

file(WRITE "${root}/locked_file" "secret")
file(WRITE "${root}/locked_dir/hidden.txt" "secret")
sh(chmod 000 locked_file locked_dir)

# Permissions are not enforced for root and are ignored by some file systems
# (e.g. Windows drives mounted into WSL): check they really took effect.
execute_process(COMMAND cat locked_file WORKING_DIRECTORY "${root}"
                RESULT_VARIABLE file_rc OUTPUT_QUIET ERROR_QUIET)
execute_process(COMMAND ls locked_dir WORKING_DIRECTORY "${root}"
                RESULT_VARIABLE dir_rc OUTPUT_QUIET ERROR_QUIET)
if(file_rc EQUAL 0 OR dir_rc EQUAL 0)
    sh(chmod 755 locked_file locked_dir)
    message(STATUS "permissions not enforced here: unreadable file checks skipped")
    return()
endif()

run_mangen(R ARGS tree)
sh(chmod 755 locked_file locked_dir)

expect_contains("${R_ERR}" "locked_file" "unreadable file error")
expect_contains("${R_ERR}" "locked_dir" "unreadable directory error")
expect_equal("${R_OUT}"
"file_link : E8B7BE43
real.txt : E8B7BE43
sub/inner.txt : 352441C2
" "stdout with unreadable entries")
