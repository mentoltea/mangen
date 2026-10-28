# Basic manifest generation: the example from the task, path forms,
# default directory, empty trees, unusual names, large files.

include("${CMAKE_CURRENT_LIST_DIR}/common.cmake")

make_example_tree("${WORK}/example")

# the example from the task statement: exact output and order
expect_manifest(ARGS example EXPECT "${EXAMPLE_MANIFEST}")

# the same with a trailing slash and with an absolute path
expect_manifest(ARGS example/ EXPECT "${EXAMPLE_MANIFEST}")
expect_manifest(ARGS "${WORK}/example" EXPECT "${EXAMPLE_MANIFEST}")

# without DIR_PATH the current directory is used, same as "."
expect_manifest(WORKDIR "${WORK}/example" EXPECT "${EXAMPLE_MANIFEST}")
expect_manifest(WORKDIR "${WORK}/example" ARGS . EXPECT "${EXAMPLE_MANIFEST}")

# subdirectory as DIR_PATH: paths are relative to it
expect_manifest(ARGS example/dir_2 EXPECT
"file_6.h : ABF0A849
dir_3/file_7.txt : 950FB4C9
")

# empty directory and a tree of empty directories: empty manifest
file(MAKE_DIRECTORY "${WORK}/empty")
expect_manifest(ARGS empty EXPECT "")
file(MAKE_DIRECTORY "${WORK}/only_dirs/a/b" "${WORK}/only_dirs/c")
expect_manifest(ARGS only_dirs EXPECT "")

# hidden files are included; spaces in names are kept as is; empty file
file(WRITE "${WORK}/names/.hidden" "a")
file(WRITE "${WORK}/names/my file.txt" "hello world\n")
file(WRITE "${WORK}/names/zero" "")
expect_manifest(ARGS names EXPECT
".hidden : E8B7BE43
my file.txt : AF083B2D
zero : 00000000
")

# a file larger than the read buffer (200 KiB, several chunks)
string(REPEAT "0123456789abcdef" 12800 big)
file(WRITE "${WORK}/big/big.bin" "${big}")
expect_manifest(ARGS big EXPECT "big.bin : 65DEC7D0\n")

# output is deterministic: a second run gives the same result
run_mangen(FIRST ARGS example)
run_mangen(SECOND ARGS example)
expect_equal("${SECOND_OUT}" "${FIRST_OUT}" "repeated run")

# changing a file changes only its line
file(WRITE "${WORK}/example/file_3.c" "changed\n")
run_mangen(CHANGED ARGS example)
expect_not_contains("${CHANGED_OUT}" "file_3.c : 7FDA81B0" "changed file hash")
expect_contains("${CHANGED_OUT}" "file_1.txt : 435657D4" "unchanged file hash")
