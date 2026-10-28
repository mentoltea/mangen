# Tests, run with CTest:
#     cmake -S . -B build && cmake --build build
#     ctest --test-dir build --output-on-failure
# or build and run everything at once:
#     cmake --build build --target check
#
# unit/  - C programs testing modules directly (linked without main.c)
# e2e/   - CMake scripts running the built mangen binary on real trees

set(TEST_WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/work")

# --- unit tests ---

set(MANGEN_CORE_SOURCES ${MANGEN_SOURCES})
list(FILTER MANGEN_CORE_SOURCES EXCLUDE REGEX "/main\\.c$")
add_library(mangen_core STATIC ${MANGEN_CORE_SOURCES})
target_include_directories(mangen_core PUBLIC "${PROJECT_SOURCE_DIR}/src")

set(UNIT_TESTS hash match cli walker logic)
set(UNIT_TESTS_WITH_FILES walker logic)

foreach(name ${UNIT_TESTS})
    add_executable(test_${name} unit/test_${name}.c)
    target_link_libraries(test_${name} PRIVATE mangen_core)
    target_include_directories(test_${name} PRIVATE unit)

    if(name IN_LIST UNIT_TESTS_WITH_FILES)
        set(work "${TEST_WORK_DIR}/unit_${name}")
        add_test(NAME unit.${name}.prepare
                 COMMAND ${CMAKE_COMMAND} -DRESET_DIR=${work}
                         -P "${CMAKE_CURRENT_SOURCE_DIR}/reset_dir.cmake")
        set_tests_properties(unit.${name}.prepare PROPERTIES FIXTURES_SETUP work_${name})
        add_test(NAME unit.${name} COMMAND test_${name} "${work}")
        set_tests_properties(unit.${name} PROPERTIES FIXTURES_REQUIRED work_${name})
    else()
        add_test(NAME unit.${name} COMMAND test_${name})
    endif()
endforeach()

foreach(target mangen_core ${UNIT_TESTS})
    if(NOT target STREQUAL "mangen_core")
        set(target test_${target})
    endif()
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra)
    endif()
endforeach()

# --- end-to-end tests ---

set(E2E_TESTS manifest options exclude)
if(UNIX)
    list(APPEND E2E_TESTS fs_errors)   # symlinks, fifos, permissions
endif()

foreach(name ${E2E_TESTS})
    add_test(NAME e2e.${name}
             COMMAND ${CMAKE_COMMAND}
                     -DMANGEN=$<TARGET_FILE:mangen>
                     -DWORK=${TEST_WORK_DIR}/e2e_${name}
                     -DEXPECTED_COMMIT=${GIT_COMMIT_HASH}
                     -P "${CMAKE_CURRENT_SOURCE_DIR}/e2e/${name}.cmake")
endforeach()

# --- convenience target ---

set(TEST_TARGETS mangen)
foreach(name ${UNIT_TESTS})
    list(APPEND TEST_TARGETS test_${name})
endforeach()

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS ${TEST_TARGETS}
    USES_TERMINAL
)
