# VS results screen aborted after a four-Remix-fighter match (reloc status buffer full)

Code: `decomp/src/mn/mnvsmode/mnvsresults.c` (`sMNVSResultsStatusBuffer`, `sMNVSResultsForceStatusBuffer`). Test: `vs.sh` with
four Remix fkinds, `SSB64_VS_ALL_CPU=1`, `SSB64_VS_STOCKS=1`, long enough for the match to end
(`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

Found by the Phase 6 recovery baseline (`ai_recovery.sh`, runs `ai_rec_5` and `ai_rec_6`): when a match with four Remix fighters
(Conker, Marth, Sonic, Sheik; Marina, Dedede, Banjo, Crash) ended, the game aborted while building the results screen. The log
loops on `syDebugPrintf` ("Relocatable Data Manager: Status Buffer is full") and then `SIGABRT`; the backtrace is
`lbRelocAddStatusBufferFile` < `portRelocLoadFileFromBytes` < `lbRelocGetExternBufferFile` < `ftManagerSetupFilesMainKind` <
`ftManagerSetupFilesAllKind` < `mnVSResultsFuncStart`.

## Root cause

The results screen loads the main files of the four fighters. A Remix fighter's main file pulls extern dependencies, each one
taking a status-buffer slot; the vanilla buffers (120 entries, 7 forced) were sized for vanilla fighters only. Same class as the
Character Data and VS CSS overflows (`remix_menu_null_state_and_demo_fkind_2026-10-22.md`).

## Fix

Under `REGION_REMIX` the results screen uses 256 status entries and 64 forced entries, as the VS CSS and the Character Data
screen do. The vanilla build keeps 120 and 7.

## Verification (2026-11-05)

`l2_results4`: Marina, Dedede, Banjo and Crash, level-9 CPUs, 1 stock, Mushroom Kingdom: the match ends and the results screen
shows "DEDEDE WINS" with the four rows, no abort. ROADMAP Phase 7 lists the other scenes that keep the vanilla buffer sizes
(`sc1pintro.c`, `sc1pgame.c`, `sc1ptrainingmode.c`) for a four-Remix-fighter check.
