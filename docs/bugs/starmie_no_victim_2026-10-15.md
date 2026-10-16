# Starmie crash when no opponent is left (2026-10-15)

**Symptom.** A 3-fighter VS run on Onett (`SSB64_VS_STAGE=73`, 5200 frames) segfaulted at frame ~5057 inside
`itStarmieNFollowFindFollowPlayerLR`, called from `itStarmieNFollowInitVars` (Poké Ball Starmie).

**Root cause.** `itStarmieNFollowInitVars` picks the closest opponent into `victim_gobj`, but the variable is only
assigned inside the loop. When every other fighter is gone (or on the owner's team) the N64 build reads stack garbage; on
LP64 that is a wild pointer.

**Fix.** `decomp/src/it/itmonster/itstarmie.c`, guarded by `PORT`: `victim_gobj` starts at NULL, falls back to the owner,
and the function returns without following when the owner is NULL too.

**Verification.** Rerun of the same Onett run (see ROADMAP T1.4).
