// Exclude patterns: match_pattern() ('.' = any char, '*' = any sequence,
// whole-name match) and the is_excluded() hook built on top of it.

#include "logic.h"
#include "test.h"

struct MatchCase {
    const char* pattern;
    const char* str;
    bool expected;
};

static void test_match_pattern(void) {
    const struct MatchCase cases[] = {
        // plain names: exact comparison
        {"file_1.txt", "file_1.txt", true},
        {"dir_2",      "dir_2",      true},
        {"dir_2",      "dir_22",     false},   // whole name, not a prefix
        {"dir_2",      "xdir_2",     false},   // not a suffix
        {"abc",        "ab",         false},
        {"ab",         "abc",        false},
        {"",           "",           true},
        {"",           "a",          false},
        {"a",          "",           false},
        {"ABC",        "abc",        false},   // case sensitive

        // '.' is exactly one arbitrary character
        {"file_.",     "file_4",     true},
        {"file_.",     "file_1.txt", false},
        {"file_.",     "file_",      false},
        {"...",        "abc",        true},
        {"...",        "ab",         false},
        {"a.c",        "a.c",        true},
        {"a.c",        "abc",        true},    // '.' is not a literal dot

        // '*' is any sequence, including empty
        {"*",          "",           true},
        {"*",          "anything",   true},
        {"**",         "abc",        true},
        {"*.txt",      "file_7.txt", true},
        {"*.txt",      "file_7.txt.bak", false},
        {"*.c",        "abc",        true},    // '.' matched 'b'
        {"*.c",        "c",          false},   // '.' needs a character
        {"dir_*",      "dir_",       true},
        {"dir_*",      "dir_1",      true},
        {"dir_*",      "adir_1",     false},
        {"*_*",        "file_4",     true},
        {"*_*",        "file4",      false},
        {"a*b*c",      "aXbYbc",     true},    // needs backtracking
        {"a*b*c",      "abcb",       false},
        {"a*a*a",      "aaa",        true},
        {"a*a*a",      "aa",         false},
        {".*",         "",           false},
        {".*",         "x",          true},
        {"*a",         "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", false},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        bool got = match_pattern(cases[i].pattern, cases[i].str);
        if (got != cases[i].expected) {
            fprintf(stderr, "  pattern \"%s\" vs \"%s\": got %d, expected %d\n",
                    cases[i].pattern, cases[i].str, got, cases[i].expected);
        }
        CHECK(got == cases[i].expected);
    }
}

static void test_is_excluded(void) {
    // no patterns: nothing is excluded
    set_excludes(NULL, 0);
    CHECK(!is_excluded("file_1.txt"));
    CHECK(!is_excluded(""));

    char* patterns[] = {"dir_2", "*.png", "file_."};
    set_excludes(patterns, 3);
    CHECK(is_excluded("dir_2"));
    CHECK(is_excluded("file_2.png"));
    CHECK(is_excluded("file_4"));
    CHECK(!is_excluded("file_1.txt"));
    CHECK(!is_excluded("dir_1"));
    CHECK(!is_excluded("png"));

    // the list can be replaced
    set_excludes(patterns, 1);
    CHECK(is_excluded("dir_2"));
    CHECK(!is_excluded("file_2.png"));

    set_excludes(NULL, 0);
}

int main(void) {
    test_match_pattern();
    test_is_excluded();
    return TEST_SUMMARY();
}
