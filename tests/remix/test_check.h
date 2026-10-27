#ifndef REMIX_TEST_CHECK_H
#define REMIX_TEST_CHECK_H

#include <stdio.h>

/*
 * Minimal assertion helpers of the headless Remix unit tests. A failed check prints the file, the line and both values,
 * counts as a failure and lets the test go on, so one run reports every broken rule.
 */

extern int gRemixTestFailures;
extern int gRemixTestChecks;

#define CHECK_EQ(expected, actual)                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        long long expected_value = (long long)(expected);                                                              \
        long long actual_value = (long long)(actual);                                                                  \
        gRemixTestChecks++;                                                                                            \
        if (expected_value != actual_value)                                                                            \
        {                                                                                                              \
            gRemixTestFailures++;                                                                                      \
            printf("FAIL %s:%d: %s expected %lld, got %lld\n", __FILE__, __LINE__, #actual, expected_value,            \
                   actual_value);                                                                                      \
        }                                                                                                              \
    } while (0)

#define CHECK_TRUE(condition) CHECK_EQ(1, (condition) ? 1 : 0)

typedef void (*RemixTestFunction)(void);

#define RUN_TEST(function)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("  %s\n", #function);                                                                                  \
        function();                                                                                                    \
    } while (0)

#endif
