# -e / --exclude: plain names (part 2 of the task) and patterns with
# '.' (any character) and '*' (any sequence) (part 3).

include("${CMAKE_CURRENT_LIST_DIR}/common.cmake")

make_example_tree("${WORK}/example")

# --- plain names ---

# a directory: its whole subtree disappears
expect_manifest(ARGS example -e dir_2 EXPECT
"file_1.txt : 435657D4
file_2.png : 33E42C7D
file_3.c : 7FDA81B0
dir_1/file_4 : 4A327460
dir_1/file_5.sh : 3D64306B
")

# long form, option before DIR_PATH, several excludes
expect_manifest(ARGS --exclude file_4 -e file_1.txt example EXPECT
"file_2.png : 33E42C7D
file_3.c : 7FDA81B0
dir_1/file_5.sh : 3D64306B
dir_2/file_6.h : ABF0A849
dir_2/dir_3/file_7.txt : 950FB4C9
")

# a nested directory is excluded without touching the rest of its parent
expect_manifest(ARGS example -e dir_3 EXPECT
"file_1.txt : 435657D4
file_2.png : 33E42C7D
file_3.c : 7FDA81B0
dir_1/file_4 : 4A327460
dir_1/file_5.sh : 3D64306B
dir_2/file_6.h : ABF0A849
")

# the whole name must match: neither prefixes nor suffixes
expect_manifest(ARGS example -e file -e dir -e 1.txt EXPECT "${EXAMPLE_MANIFEST}")

# names that do not exist and the name of DIR_PATH itself change nothing
expect_manifest(ARGS example -e nothing -e example EXPECT "${EXAMPLE_MANIFEST}")

# paths are not names: "dir_2/file_6.h" matches nothing
expect_manifest(ARGS example -e dir_2/file_6.h EXPECT "${EXAMPLE_MANIFEST}")

# the same name at different depths is excluded everywhere
file(WRITE "${WORK}/same/x/keep" "a")
file(WRITE "${WORK}/same/x/drop" "a")
file(WRITE "${WORK}/same/drop/x/keep" "a")
file(WRITE "${WORK}/same/y/drop" "a")
expect_manifest(ARGS same -e drop EXPECT "x/keep : E8B7BE43\n")

# --- patterns ---

# '*' suffix: all .txt files
expect_manifest(ARGS example -e "*.txt" EXPECT
"file_2.png : 33E42C7D
file_3.c : 7FDA81B0
dir_1/file_4 : 4A327460
dir_1/file_5.sh : 3D64306B
dir_2/file_6.h : ABF0A849
")

# '.' is exactly one character: file_4 matches, file_1.txt does not
expect_manifest(ARGS example -e "file_." EXPECT
"file_1.txt : 435657D4
file_2.png : 33E42C7D
file_3.c : 7FDA81B0
dir_1/file_5.sh : 3D64306B
dir_2/file_6.h : ABF0A849
dir_2/dir_3/file_7.txt : 950FB4C9
")

# '*' prefix: all directories dir_<something> and everything inside
expect_manifest(ARGS example -e "dir_*" EXPECT
"file_1.txt : 435657D4
file_2.png : 33E42C7D
file_3.c : 7FDA81B0
")

# '.' matches any character, also a literal dot: "file_..txt" = file_?.txt
expect_manifest(ARGS example -e "file_..txt" EXPECT
"file_2.png : 33E42C7D
file_3.c : 7FDA81B0
dir_1/file_4 : 4A327460
dir_1/file_5.sh : 3D64306B
dir_2/file_6.h : ABF0A849
")

# pattern and plain name together
expect_manifest(ARGS example -e "*.png" -e dir_1 EXPECT
"file_1.txt : 435657D4
file_3.c : 7FDA81B0
dir_2/file_6.h : ABF0A849
dir_2/dir_3/file_7.txt : 950FB4C9
")

# '*' excludes everything
expect_manifest(ARGS example -e "*" EXPECT "")
