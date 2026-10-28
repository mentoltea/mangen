cmake_minimum_required(VERSION 3.12)

project(mangen LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


file(GLOB MANGEN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB MANGEN_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_executable(mangen ${MANGEN_SOURCES} ${MANGEN_HEADERS})

target_include_directories(mangen PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mangen PRIVATE -Wall -Wextra)
endif()
