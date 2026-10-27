/**
 * test_main.c - runner of the headless Remix unit tests: runs every suite and fails when any check failed.
 */

#include "test_check.h"

int gRemixTestFailures = 0;
int gRemixTestChecks = 0;

extern void remixTestTwelveCBRules(void);
extern void remixTest1PModeNames(void);
extern void remixTest1PModeRules(void);

int main(void)
{
    remixTestTwelveCBRules();
    remixTest1PModeNames();
    remixTest1PModeRules();

    printf("%d checks, %d failed\n", gRemixTestChecks, gRemixTestFailures);
    return (gRemixTestFailures == 0) ? 0 : 1;
}
