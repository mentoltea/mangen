# reset_dir(dir): makes dir an empty directory.
# Permissions are restored first, so trees left by a failed test that
# made something unreadable can still be removed.
#
# Can also be run as a script (CTest fixture for unit tests):
#     cmake -DRESET_DIR=<dir> -P reset_dir.cmake

function(reset_dir dir)
    if(EXISTS "${dir}")
        if(UNIX)
            execute_process(COMMAND chmod -R u+rwx "${dir}" ERROR_QUIET)
        endif()
        file(REMOVE_RECURSE "${dir}")
    endif()
    file(MAKE_DIRECTORY "${dir}")
endfunction()

if(DEFINED RESET_DIR)
    reset_dir("${RESET_DIR}")
endif()
