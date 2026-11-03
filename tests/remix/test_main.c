/**
 * test_main.c - runner of the headless Remix unit tests: runs every suite and fails when any check failed.
 */

#include "test_check.h"

int gRemixTestFailures = 0;
int gRemixTestChecks = 0;

extern void remixTestTwelveCBRules(void);
extern void remixTest1PModeNames(void);
extern void remixTest1PModeRules(void);
extern void remixTest1PHrcRules(void);
extern void remixTest1PHrcSigns(void);
extern void remixTest1PHrcTravel(void);
extern void remixTest1PRemixRules(void);
extern void remixTestTimedStockRules(void);
extern void remixTestKothRules(void);
extern void remixTestTugOfWarRules(void);
extern void remixTestTagTeamRules(void);
extern void remixTestSmashketballRules(void);
extern void remixTestStaminaRules(void);
extern void remixTestAiTablesRules(void);
extern void remixTestAiCommandsRules(void);
extern void remixTestAiPreventRules(void);
extern void remixTestAiCombatRules(void);
extern void remixTestAiCloudRules(void);
extern void remixTestAiWeightRules(void);
extern void remixTestAiCpuLevelRules(void);
extern void remixTestAiLevelRules(void);
extern void remixTestAiLongRangeRules(void);
extern void remixTestAiLv10Rules(void);
extern void remixTestAiMovementRules(void);
extern void remixTestAiPredictionRules(void);
extern void remixTestAiRollRules(void);
extern void remixTestAiSpecialsRules(void);
extern void remixTestAiTeachingRules(void);
extern void remixTestStopwatchBgRules(void);
extern void remixTestTrainingRules(void);
extern void remixTestTrainingPortRules(void);
extern void remixTestAiDiRules(void);
extern void remixTestBlueShellRules(void);
extern void remixTestGameEndRowsRules(void);
extern void remixTestGameEndInputRules(void);
extern void remixTestAiLeftoverRules(void);
extern void remixTestVsStatsRules(void);
extern void remixTestFontDecode(void);

int main(void)
{
    remixTestTwelveCBRules();
    remixTest1PModeNames();
    remixTest1PModeRules();
    remixTest1PHrcRules();
    remixTest1PHrcSigns();
    remixTest1PHrcTravel();
    remixTest1PRemixRules();
    remixTestTimedStockRules();
    remixTestKothRules();
    remixTestTugOfWarRules();
    remixTestTagTeamRules();
    remixTestSmashketballRules();
    remixTestStaminaRules();
    remixTestAiTablesRules();
    remixTestAiCommandsRules();
    remixTestAiPreventRules();
    remixTestAiCombatRules();
    remixTestAiCloudRules();
    remixTestAiWeightRules();
    remixTestAiCpuLevelRules();
    remixTestAiLevelRules();
    remixTestAiLongRangeRules();
    remixTestAiLv10Rules();
    remixTestAiMovementRules();
    remixTestAiPredictionRules();
    remixTestAiRollRules();
    remixTestAiSpecialsRules();
    remixTestAiTeachingRules();
    remixTestStopwatchBgRules();
    remixTestTrainingRules();
    remixTestTrainingPortRules();
    remixTestAiDiRules();
    remixTestBlueShellRules();
    remixTestGameEndRowsRules();
    remixTestGameEndInputRules();
    remixTestAiLeftoverRules();
    remixTestVsStatsRules();
    remixTestFontDecode();

    printf("%d checks, %d failed\n", gRemixTestChecks, gRemixTestFailures);
    return (gRemixTestFailures == 0) ? 0 : 1;
}
