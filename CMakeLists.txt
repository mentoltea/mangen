cmake_minimum_required(VERSION 3.12)

project(mangen LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


file(GLOB MANGEN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB MANGEN_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_executable(mangen ${MANGEN_SOURCES} ${MANGEN_HEADERS})

target_include_directories(mangen PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_HEAD_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE GIT_RESULT
        ERROR_QUIET
    )
    if(GIT_RESULT EQUAL 0)
        set(GIT_COMMIT_HASH "${GIT_HEAD_OUTPUT}")
    endif()
endif()
foreach(GIT_FILE HEAD logs/HEAD)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git/${GIT_FILE}")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                     "${CMAKE_CURRENT_SOURCE_DIR}/.git/${GIT_FILE}")
    endif()
endforeach()
target_compile_definitions(mangen PRIVATE GIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mangen PRIVATE -Wall -Wextra)
endif()

option(MANGEN_BUILD_TESTS "Build unit and end-to-end tests" ON)
if(MANGEN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
