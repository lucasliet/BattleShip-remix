# The Stamina rule crashed the VS CSS before any battle scene had run

Code: `decomp/src/gm/gmremixstamina.c` (`gmRemixStaminaIsActive`). Test: `lldb_count.sh` run `r13_md20_css`
(`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

Found by the Phase 5b check of MD-20 (2026-11-06): the VS CSS opened from boot (`SSB64_START_SCENE=16`) with the Stamina
rule on (`gGMRemixStaminaRule = 1`) crashed when P1 placed a token:

```
EXC_BAD_ACCESS (address=0x3)
gmRemixStaminaIsActive                gmremixstamina.c:24
ftRemixStaminaIsKOPending             ftremixstaminaaction.c:134
ftRemixStaminaKOAnimSpeed             ftremixstaminaaction.c:228
ftMainSetStatus                       ftmain.c:5069
ftManagerMakeFighter                  ftmanager.c:1271
mnPlayersVSMakeFighter                mnplayersvs.c:2921
```

## Root cause

The CSS preview fighter goes through `ftMainSetStatus`, whose Stamina KO hook asks `gmRemixStaminaIsActive`. That
function read `gSCManagerBattleState->game_rules`, but the CSS has no battle: the pointer is whatever the last battle or
movie scene left, and NULL when none has run since boot.

## Fix

`gmRemixStaminaIsActive` returns FALSE when `gSCManagerBattleState` is NULL, so a fighter made outside a battle never takes
the Stamina KO path.

## Verification

`r13_md20_css` (same setup) runs to the end and the right arrow steps the HP 200, 210, 220, 230; see the MD-20 row of
`docs/remix/patch_audit.md`.
