# Remix: the save is rejected at every boot (signature shifted by the Remix bonus records)

Code: `decomp/src/sc/scmanager.c` (`dSCManagerDefaultBackupData`), `decomp/src/lb/lbbackup.c` (`lbBackupIsSramValid`).

## Symptom

Issue #6 (PS5): settings changed inside the game do not survive a restart. Checked on the console over FTP
(`/data/homebrew/PPSA99264/`): during the session `ssb64_save.bin` held the player's values
(sound mono, horizontal screen adjust 14); after a restart both copies held the defaults (stereo, adjust 0), rewritten
at boot. The Linux save showed the same state. The port menu settings (`BattleShip.cfg.json`) were not affected.

Item Switch and the VS rules (handicap) are not part of the N64 save (`LBBackupData`), in the original game or in
Smash Remix, so they reset at every boot by design.

## Cause

`LBBackupData` gains `remix_bonus` (Remix Bonus 1 and 2 records) between `spgame_records` and `ground_mask` in the
Remix port build, but the positional initializer `dSCManagerDefaultBackupData` had no entry for it. The trailing values
(`ground_mask` ... `0x29A`) landed inside `remix_bonus`, and `signature` stayed 0. `lbBackupIsChecksumValid` asks for a
valid checksum and `signature == 666`, so every boot failed both copies, loaded the defaults and wrote them back: sound,
screen adjust, VS and 1P records, unlocks and Remix bonus records were lost at each launch, on every platform.

## Fix

The initializer has a zeroed `remix_bonus` entry under the same `PORT && REGION_REMIX` condition as the field, so the
default save carries `signature = 0x29A`. A save written by an older build has signature 0 and is replaced by the
defaults once more at the first launch of the fixed build; from then on it is kept.

## Verification

- Offsets from the build headers (`offsetof`): signature at 0x1CB50 of each 117592-byte copy, sound at 0xE4DD, horizontal
  screen adjust at 0xE4DE, checksum at 0x1CB54; the second copy starts at 0x1CB60.
- Linux, `SSB64_SAVE_PATH` on a new file, VS shortcut for 200 frames per launch:
  - a new save is written with signature 666 and a valid checksum in both copies;
  - sound set to mono and screen adjust to 14 in the file (checksum recomputed) are still there after the next launch
    (before the fix the launch put back stereo and 0);
  - the save taken from the PS5 (signature 0, mono, adjust 14) is replaced by the defaults at the first launch, and a
    later change (mono, adjust 7) survives the launch after it.
- PS5 (title over FTP, 2026-11-13): the first launch replaced the old signature-0 save, the sound option changed in the
  game was still set after closing and reopening the title, and both copies of `ssb64_save.bin` read back over FTP
  carry signature 666 with a valid checksum.
