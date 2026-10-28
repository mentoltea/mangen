// Minimal unit test helpers: every CHECK is counted, failures are printed
// with file:line and the test keeps going, so one run shows all failures.
// TEST_SUMMARY() returns the exit code expected by CTest (0 = passed).

#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

static int test_checks = 0;
static int test_failures = 0;

#define CHECK(cond) do { \
    test_checks++; \
    if (!(cond)) { \
        test_failures++; \
        fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_STR_EQ(actual, expected) do { \
    const char* a_ = (actual); \
    const char* e_ = (expected); \
    test_checks++; \
    if (a_ == NULL || strcmp(a_, e_) != 0) { \
        test_failures++; \
        fprintf(stderr, "%s:%d: CHECK_STR_EQ failed: %s\n    got:      \"%s\"\n    expected: \"%s\"\n", \
                __FILE__, __LINE__, #actual, a_ ? a_ : "(null)", e_); \
    } \
} while (0)

#define TEST_SUMMARY() ( \
    printf("%d checks, %d failed\n", test_checks, test_failures), \
    test_failures == 0 ? 0 : 1)

#endif // TEST_H
