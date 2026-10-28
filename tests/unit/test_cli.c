// Command line parsing (parse_arguments). Error cases print to stderr,
// which is expected here; only the returned struct is checked.

#include "cli.h"
#include "test.h"

#define ARGC(argv) ((int)(sizeof(argv) / sizeof(*(argv))))

static void test_no_arguments(void) {
    char* argv[] = {"mangen"};
    struct cli cli = parse_arguments(ARGC(argv), argv);
    CHECK(cli.valid);
    CHECK(cli.dir_path == NULL);   // main() falls back to "."
    CHECK(!cli.help);
    CHECK(!cli.version);
    CHECK(cli.excludes.count == 0);
}

static void test_dir_path(void) {
    char* argv[] = {"mangen", "example"};
    struct cli cli = parse_arguments(ARGC(argv), argv);
    CHECK(cli.valid);
    CHECK_STR_EQ(cli.dir_path, "example");
}

static void test_help_and_version(void) {
    char* short_help[] = {"mangen", "-h"};
    char* long_help[] = {"mangen", "--help"};
    char* short_version[] = {"mangen", "-v"};
    char* long_version[] = {"mangen", "--version"};

    struct cli cli = parse_arguments(ARGC(short_help), short_help);
    CHECK(cli.valid && cli.help && !cli.version);
    cli = parse_arguments(ARGC(long_help), long_help);
    CHECK(cli.valid && cli.help && !cli.version);
    cli = parse_arguments(ARGC(short_version), short_version);
    CHECK(cli.valid && cli.version && !cli.help);
    cli = parse_arguments(ARGC(long_version), long_version);
    CHECK(cli.valid && cli.version && !cli.help);

    // options may come before or after DIR_PATH
    char* mixed[] = {"mangen", "-v", "dir", "-h"};
    cli = parse_arguments(ARGC(mixed), mixed);
    CHECK(cli.valid && cli.help && cli.version);
    CHECK_STR_EQ(cli.dir_path, "dir");
}

static void test_excludes(void) {
    char* argv[] = {"mangen", "-e", "a", "dir", "--exclude", "*.png", "-e", "b"};
    struct cli cli = parse_arguments(ARGC(argv), argv);
    CHECK(cli.valid);
    CHECK_STR_EQ(cli.dir_path, "dir");
    CHECK(cli.excludes.count == 3);
    if (cli.excludes.count == 3) {
        CHECK_STR_EQ(cli.excludes.items[0], "a");
        CHECK_STR_EQ(cli.excludes.items[1], "*.png");
        CHECK_STR_EQ(cli.excludes.items[2], "b");
    }
    da_free(cli.excludes);

    // the exclude value is not taken as DIR_PATH
    char* only_exclude[] = {"mangen", "-e", "x"};
    cli = parse_arguments(ARGC(only_exclude), only_exclude);
    CHECK(cli.valid);
    CHECK(cli.dir_path == NULL);
    da_free(cli.excludes);

    // many excludes: forces the dynamic array to grow
    char* many[1 + 2 * 15];   // program name + 15 pairs "-e name"
    many[0] = "mangen";
    for (int i = 1; i < ARGC(many); i += 2) {
        many[i] = "-e";
        many[i + 1] = "name";
    }
    cli = parse_arguments(ARGC(many), many);
    CHECK(cli.valid);
    CHECK(cli.excludes.count == 15);
    da_free(cli.excludes);
}

static void test_errors(void) {
    char* unknown[] = {"mangen", "-x"};
    char* unknown_long[] = {"mangen", "--frobnicate"};
    char* two_dirs[] = {"mangen", "a", "b"};
    char* missing_value[] = {"mangen", "dir", "-e"};
    char* flag_as_value[] = {"mangen", "-e", "-h"};
    char* lone_dash[] = {"mangen", "-"};

    struct cli cli;
    cli = parse_arguments(ARGC(unknown), unknown);
    CHECK(!cli.valid);
    cli = parse_arguments(ARGC(unknown_long), unknown_long);
    CHECK(!cli.valid);
    cli = parse_arguments(ARGC(two_dirs), two_dirs);
    CHECK(!cli.valid);
    cli = parse_arguments(ARGC(missing_value), missing_value);
    CHECK(!cli.valid);
    cli = parse_arguments(ARGC(flag_as_value), flag_as_value);
    CHECK(!cli.valid);
    cli = parse_arguments(ARGC(lone_dash), lone_dash);
    CHECK(!cli.valid);
}

int main(void) {
    test_no_arguments();
    test_dir_path();
    test_help_and_version();
    test_excludes();
    test_errors();
    return TEST_SUMMARY();
}
