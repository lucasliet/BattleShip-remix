
## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

FAIL — SIGBUS at ~frame 500 (2 shots only). Backtrace: gcAddGObjProcess +76 <- itRemixPoisonMushroomStartEffect +120 <- ftMainSearchHitItem +2376. fault_addr=0x11b2e4000 vs sp=0x11b2e3390 (stack guard hit). Last log: gmCollItemFighterDmg joint_id=6. Shared-state unification in flight.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
