# Smash Remix: file id table and loader (Task A and B)

Scope: read-only analysis of the Smash Remix 2.0.1 source at `.toolchain/pkgs/smashremix-src` (commit 5e04fe7).
No code was built or run. `assembler/bass.exe` is a Windows PE binary, so the memory layout below is derived from the
source directives, not from an assembled image. Sizes of the code image are therefore NOT computed here.

## Summary

- `src/File.asm` defines 3329 `File.*` constants: 8 vanilla ids (0x0591-0x0598, KIRBY_NSP_*) and 3321 Remix ids in 0x0854-0x154E.
- Remix ids are hard-coded constants. `File.asm` emits no bytes and nothing assigns ids at assembly time.
- The complete new-id list (0x0854-0x154E, 3323 ids) is in `roms/filename_overrides.txt`, one contiguous, strictly increasing block.
  Every one of the 3323 ids has a name there. 3321 also have a `File.*` symbol. The 2 without a symbol are 0x0FF6 and 0x0FF7
  ("Marina Board the Platforms Header/Stage"); no code references them by name or number.
- Ordering rule (reproduces the table exactly): `id = 0x0854 + k`, where `k` is the zero-based index of the entry among the
  `filename_overrides.txt` lines whose id is >= 0x0854 (the file is sorted by id).
- Only one raw numeric Remix id is used directly in code: `0xC1B` (3099), in `Render.load_file(0xC1B, ...)` calls, instead of its
  symbol `File.REMIX_1P_KIRBY` (`src/File.asm:982`). Overrides name: "Remix 1p Kirby Stage File" (line 988).
- `build/master.csv` (the older ADD manifest) lists only 359 new ids ending at 0x0A15, with gaps. It is stale; do not use it as the id source.
- Remix loader: `main.asm` places the code image at ROM 0x02C00000 (RAM base 0x80400000). `Boot.asm` `load_` DMAs it to RAM
  with `Global.dma_copy_` (0x80002CA0) at boot. Remix reloc-file loading goes through `Render.load_file_` (Render.asm:921).

---

## Task A: file id definitions

### A.1 How File.asm defines ids

- File: `src/File.asm`, lines 4-3336 inside `scope File { ... }`.
- Form: `constant NAME(0xHHHH)`. All 3329 constant lines match this form. Line 2845 (`CRASH_USPLanding`, 0x1364) is the only mixed-case symbol.
- Line 5 is the `// vanilla` marker, line 14 the `// remix` marker. Lines 6-13 are the vanilla KIRBY_NSP_* ids (0x0591-0x0598).
- Ids are sorted and contiguous from 0x0854 to 0x154E, except the 2 gaps 0x0FF6 and 0x0FF7 (no symbol). No duplicate ids or names.
- `src/File.asm` is included from `main.asm:24`, before `Boot.asm`, and before every file that uses `File.*`.

### A.2 Where ids are consumed

- `src/Character.asm` `define_character(...)`: the file_1..file_9 arguments (e.g. `File.RANDOM_MAIN`, `File.RANDOM_CHARACTER`)
  become the 9 character-struct file id words (0x00-0x20) and per-character file pointer slots.
  Of the 83 `define_character(` calls, none passes a raw Remix id literal.
- `Render.load_file(File.X, ptr)` (Render.asm:549 macro), e.g. `Toggles.asm:396`, `Stages.asm:2831`, `Bonus.asm:23`.
- `custom_victory_file_table` (`SinglePlayer.asm:2304`) uses `dh File.SINGLEPLAYER_VICTORY_IMAGE_BOTTOM`, etc.
- Raw ids (not names) used with `Render.load_file`: 0xC1B (`KirbyHats.asm:673`, `SinglePlayerModes.asm:5932`), though `File.REMIX_1P_KIRBY` exists; plus many vanilla
  ids below 0x0854 (e.g. 0xC5, 0xE6, 0x153).

### A.3 Assignment mechanism (static vs. by order)

- Static: every id is a literal in `File.asm`. No `File.*` symbol is computed.
- Historical order: the ids were allocated sequentially by the ADD order of the injection tool (`build/master.csv`, mode ADD;
  first row 0x0854 `.\original\0854.bin`). The build tools (`build/readme.md`, `build/original/readme.md`) describe
  appending files in GE Editor and tracking them in CSVs. The overrides file preserves that order for all 3323 ids.
- Result: the ordering rule in the summary reproduces every id. Checked: 3323 lines, 3322 steps of +1, no gaps,
  no duplicates, strictly increasing.
- `build/incremental.csv` has only an END row (no pending adds).

### A.4 Resolved ids

- Names: 3323/3323 new ids resolved (3321 with `File.*` symbols, 2 names only in `roms/filename_overrides.txt`).
- Symbol names and overrides names differ in wording (e.g. 0x0854 `GND_USMASH` vs "Ganon upsmash"). Code uses the symbol.
  The overrides name is the human description.
- Owning feature in the table below = first `src/` file that references the symbol (excluding comments). Fighter directories
  (e.g. `src/Ganondorf/`) are shown as the fighter.
- Refs = number of `File.<NAME>` references outside `File.asm` (comment-only lines excluded).

### A.5 Full id table

Columns: decimal id, hex id, symbol and its line in `src/File.asm`, overrides name and its line in `roms/filename_overrides.txt`,
first referencing files, and reference count.
The two rows with no symbol (0x0FF6, 0x0FF7) and the vanilla KIRBY_NSP rows (no override name) are included.

| ID dec | ID hex | File.asm symbol | File.asm line | Override name (roms/filename_overrides.txt line) | Used in (first 3, excl. File.asm) | Refs |
|---:|---:|---|---:|---|---|---:|
| 1425 | 0x0591 | `KIRBY_NSP_BEGIN` | 6 |  (L) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 1426 | 0x0592 | `KIRBY_NSP_SWALLOW` | 7 |  (L) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 1427 | 0x0593 | `KIRBY_NSP_SPIT` | 8 |  (L) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 1428 | 0x0594 | `KIRBY_NSP_IDLE` | 9 |  (L) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 5 |
| 1429 | 0x0595 | `KIRBY_NSP_TURN` | 10 |  (L) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 3 |
| 1430 | 0x0596 | `KIRBY_NSP_COPY` | 11 |  (L) |  | 0 |
| 1431 | 0x0597 | `KIRBY_NSP_LOOP` | 12 |  (L) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 8 |
| 1432 | 0x0598 | `KIRBY_NSP_END` | 13 |  (L) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 2132 | 0x0854 | `GND_USMASH` | 15 | Ganon upsmash (L21) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 2 |
| 2133 | 0x0855 | `GND_DSMASH` | 16 | Ganon downsmash (L22) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 2 |
| 2134 | 0x0856 | `SONIC_DOWNBACKD` | 17 | Sonic Down Back Down (L23) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 2135 | 0x0857 | `GND_FAIR` | 18 | Ganon forward air melee style punch (L24) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 2 |
| 2136 | 0x0858 | `DRM_DAIR` | 19 | Dr. Mario down air (L25) | `src/DrMario/DrMario.asm`, `src/NDrMario/NDrMario.asm` | 2 |
| 2137 | 0x0859 | `DRM_FAIR` | 20 | Dr. Mario forward air (L26) | `src/DrMario/DrMario.asm`, `src/NDrMario/NDrMario.asm` | 2 |
| 2138 | 0x085A | `DRM_TAUNT` | 21 | Dr. Mario taunt (L27) | `src/DrMario/DrMario.asm`, `src/NDrMario/NDrMario.asm` | 2 |
| 2139 | 0x085B | `WARIO_DSMASH` | 22 | Wario D-Smash (L28) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2140 | 0x085C | `WARIO_NSP_AIR` | 23 | Wario Air Neutral Special (L29) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2141 | 0x085D | `WARIO_NSP_GROUND` | 24 | Wario Ground Neutral Special (L30) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2142 | 0x085E | `WARIO_NAIR` | 25 | Wario N-Air (L31) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2143 | 0x085F | `WARIO_UAIR` | 26 | Wario U-Air (L32) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2144 | 0x0860 | `WARIO_TAUNT` | 27 | Wario Taunt (L33) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2145 | 0x0861 | `GND_TAUNT` | 28 | Ganon's taunt (L34) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 2 |
| 2146 | 0x0862 | `WARIO_BAIR` | 29 | Wario B-Air (L35) | `src/Ebi/Ebi.asm`, `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 3 |
| 2147 | 0x0863 | `WARIO_IDLE` | 30 | Wario Idle (L36) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 12 |
| 2148 | 0x0864 | `WARIO_UTILT` | 31 | Wario Up Tilt (L37) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2149 | 0x0865 | `WARIO_FAIR` | 32 | Wario F-Air (L38) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2150 | 0x0866 | `WARIO_DAIR` | 33 | Wario D-Air (L39) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2151 | 0x0867 | `FALCO_FSMASH` | 34 | Falco's Forward Smash (400000000) (L40) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm`, `src/Peppy/Peppy.asm` | 3 |
| 2152 | 0x0868 | `FALCO_TAUNT` | 35 | Falco's Taunt (L41) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm` | 2 |
| 2153 | 0x0869 | `FALCO_UTILT` | 36 | Falco's Up Tilt (L42) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm` | 2 |
| 2154 | 0x086A | `SONIC_GRAB` | 37 | Sonic Grab (L43) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 2155 | 0x086B | `SONIC_GRAB_PULL` | 38 | Sonic Grab Pull (L44) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 2156 | 0x086C | `SONIC_THROWF` | 39 | Sonic Forward Throw (L45) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 2157 | 0x086D | `WARIO_FSMASH` | 40 | Wario F-Smash (L46) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2158 | 0x086E | `WARIO_USMASH` | 41 | Wario U-Smash (L47) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2159 | 0x086F | `DRM_PROJECTILE_DATA` | 42 | Dr. Mario's Projectile Hitbox (L48) | `src/Character.asm`, `src/TagTeam.asm` | 5 |
| 2160 | 0x0870 | `DRM_PROJECTILE_GRAPHIC` | 43 | Dr. Mario's Projectile Graphic (L49) | `src/Character.asm` | 3 |
| 2161 | 0x0871 | `STG_BATTLEFIELD_HEADER` | 44 | Battlefield Header (L50) |  | 0 |
| 2162 | 0x0872 | `STG_BATTLEFIELD` | 45 | Battlefield Stage (L51) |  | 0 |
| 2163 | 0x0873 | `STG_BATTLEFIELD_BG` | 46 | Battlefield BG (L52) |  | 0 |
| 2164 | 0x0874 | `STG_DEKU_HEADER` | 47 | Deku Tree Header (L53) |  | 0 |
| 2165 | 0x0875 | `STG_DEKU` | 48 | Deku Tree Stage (L54) |  | 0 |
| 2166 | 0x0876 | `STG_DEKU_BG` | 49 | Deku Tree BG File (L55) |  | 0 |
| 2167 | 0x0877 | `STG_FIRST_HEADER` | 50 | First Destination Header (L56) |  | 0 |
| 2168 | 0x0878 | `STG_FIRST` | 51 | First Destination Stage (L57) |  | 0 |
| 2169 | 0x0879 | `STG_FIRST_BG` | 52 | First Destination BG file (L58) |  | 0 |
| 2170 | 0x087A | `STG_GNDTOWER_HEADER` | 53 | Ganon's Tower Header (L59) |  | 0 |
| 2171 | 0x087B | `STG_GNDTOWER` | 54 | Ganon's Tower Stage (L60) |  | 0 |
| 2172 | 0x087C | `STG_GNDTOWER_BG` | 55 | Ganon's Tower BG file (L61) |  | 0 |
| 2173 | 0x087D | `STG_GYM_LEADER_HEADER` | 56 | Gym Leader Castle Header File (L62) |  | 0 |
| 2174 | 0x087E | `STG_GYM_LEADER` | 57 | Gym Leader Castle Stage File (L63) |  | 0 |
| 2175 | 0x087F | `STG_GYM_LEADER_BG` | 58 | Gym Leader Castle BG (L64) |  | 0 |
| 2176 | 0x0880 | `STG_PS_HEADER` | 59 | Pokemon Stadium Header (L65) |  | 0 |
| 2177 | 0x0881 | `STG_PS` | 60 | Pokemon Stadium Stage (L66) |  | 0 |
| 2178 | 0x0882 | `STG_PS_BG` | 61 | Pokemon Stadium BG (L67) |  | 0 |
| 2179 | 0x0883 | `STG_TALTAL_HEADER` | 62 | Tal Tal Header (L68) |  | 0 |
| 2180 | 0x0884 | `STG_TALTAL` | 63 | Tal Tal Stage (L69) |  | 0 |
| 2181 | 0x0885 | `STG_TALTAL_BG` | 64 | Tal Tal BG (L70) |  | 0 |
| 2182 | 0x0886 | `STG_GLACIAL_RIVER_HEADER` | 65 | Glacial River Header (L71) |  | 0 |
| 2183 | 0x0887 | `STG_GLACIAL_RIVER` | 66 | Glacial River Stage file (L72) |  | 0 |
| 2184 | 0x0888 | `STG_GLACIAL_RIVER_BG` | 67 | Glacial River BG (L73) |  | 0 |
| 2185 | 0x0889 | `STG_WARIOWARE_HEADER` | 68 | Warioware Header (L74) |  | 0 |
| 2186 | 0x088A | `STG_WARIOWARE` | 69 | Warioware Stage (L75) |  | 0 |
| 2187 | 0x088B | `STG_WARIOWARE_BG` | 70 | Warioware BG (L76) |  | 0 |
| 2188 | 0x088C | `STG_FLAT_ZONE_HEADER` | 71 | Flat Zone Header (L77) |  | 0 |
| 2189 | 0x088D | `STG_FLAT_ZONE` | 72 | Flat Zone Stage File (L78) |  | 0 |
| 2190 | 0x088E | `STG_FLAT_ZONE_BG` | 73 | Flat Zone BG (L79) |  | 0 |
| 2191 | 0x088F | `STG_DRM_HEADER` | 74 | Doctor Mario Stage Header (L80) |  | 0 |
| 2192 | 0x0890 | `STG_DRM` | 75 | Doctor Mario Stage (L81) |  | 0 |
| 2193 | 0x0891 | `STG_DRM_BG` | 76 | Doctor Mario Stage BG (L82) |  | 0 |
| 2194 | 0x0892 | `STG_CCM_HEADER` | 77 | Cool Cool Header (L83) |  | 0 |
| 2195 | 0x0893 | `STG_CCM` | 78 | Cool Cool Stage (L84) |  | 0 |
| 2196 | 0x0894 | `STG_CCM_BG` | 79 | Cool Cool BG (L85) |  | 0 |
| 2197 | 0x0895 | `STG_DRAGONKING_HEADER` | 80 | Dragon King Header (L86) |  | 0 |
| 2198 | 0x0896 | `STG_DRAGONKING` | 81 | Dragon King Stage (L87) |  | 0 |
| 2199 | 0x0897 | `STG_DRAGONKING_BG` | 82 | Dragon King BG (L88) |  | 0 |
| 2200 | 0x0898 | `STG_FRAY_HEADER` | 83 | Fray's Stage Header (L89) |  | 0 |
| 2201 | 0x0899 | `STG_FRAY` | 84 | Fray's Stage Stage (L90) |  | 0 |
| 2202 | 0x089A | `STG_FRAY_BG` | 85 | Fray's Stage BG (L91) |  | 0 |
| 2203 | 0x089B | `STG_GREATBAY_HEADER` | 86 | Great Bay Header (L92) |  | 0 |
| 2204 | 0x089C | `STG_GREATBAY` | 87 | Great Bay Stage File (L93) |  | 0 |
| 2205 | 0x089D | `STG_GREATBAY_BG` | 88 | Great Bay BG (L94) |  | 0 |
| 2206 | 0x089E | `STG_TOH_HEADER` | 89 | Tower of Heaven Header (L95) |  | 0 |
| 2207 | 0x089F | `STG_TOH` | 90 | Tower of Heaven Stage (L96) |  | 0 |
| 2208 | 0x08A0 | `STG_TOH_BG` | 91 | Tower of Heaven BG (L97) |  | 0 |
| 2209 | 0x08A1 | `STG_FOD_HEADER` | 92 | Fountain of Dreams Header (L98) |  | 0 |
| 2210 | 0x08A2 | `STG_FOD` | 93 | Fountain of Dreams Stage (L99) |  | 0 |
| 2211 | 0x08A3 | `STG_FOD_BG` | 94 | Fountain of Dreams BG (L100) |  | 0 |
| 2212 | 0x08A4 | `YLINK_TAUNT` | 95 | Young Link's Taunt (L101) | `src/NYoungLink/NYoungLink.asm`, `src/YoungLink/YoungLink.asm` | 2 |
| 2213 | 0x08A5 | `STG_MUDA_HEADER` | 96 | Muda Kingdom Header (L102) |  | 0 |
| 2214 | 0x08A6 | `STG_MUDA` | 97 | Muda Kingdom Stage (L103) |  | 0 |
| 2215 | 0x08A7 | `STG_MUDA_BG` | 98 | Muda Kingdom BG (L104) |  | 0 |
| 2216 | 0x08A8 | `STG_MEMENTOS_HEADER` | 99 | Mementos Header (L105) |  | 0 |
| 2217 | 0x08A9 | `STG_MEMENTOS` | 100 | Mementos Stage (L106) |  | 0 |
| 2218 | 0x08AA | `STG_MEMENTOS_BG` | 101 | Mementos BG (L107) |  | 0 |
| 2219 | 0x08AB | `FALCO_MAIN` | 102 | Falco Main (L108) | `src/Character.asm` | 1 |
| 2220 | 0x08AC | `FALCO_CHARACTER` | 103 | Falco Character (L109) | `src/Character.asm` | 1 |
| 2221 | 0x08AD | `GND_MAIN` | 104 | Ganondorf Main (L110) | `src/Character.asm` | 1 |
| 2222 | 0x08AE | `GND_PUNCH_GRAPHIC` | 105 | Ganondorf Punch Graphic (L111) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 2223 | 0x08AF | `GND_ENTRY_KICK` | 106 | Ganondorf Entry/Kick Graphic (L112) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 2224 | 0x08B0 | `GND_CHARACTER` | 107 | Ganondorf Character (L113) | `src/Character.asm` | 1 |
| 2225 | 0x08B1 | `YLINK_MAIN` | 108 | Young Link Main (L114) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2226 | 0x08B2 | `YLINK_BOOMERANG_HITBOX` | 109 | Young Link Boomerang Hitbox (L115) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 2227 | 0x08B3 | `YLINK_CHARACTER` | 110 | Young Link Character (L116) | `src/Character.asm` | 1 |
| 2228 | 0x08B4 | `DRM_MAIN` | 111 | Dr. Mario Main (L117) | `src/Character.asm` | 1 |
| 2229 | 0x08B5 | `DRM_CHARACTER` | 112 | Dr. Mario Character (L118) | `src/Character.asm` | 1 |
| 2230 | 0x08B6 | `STG_SHOWDOWN_HEADER` | 113 | Showdown Header (L119) |  | 0 |
| 2231 | 0x08B7 | `STG_SHOWDOWN` | 114 | Showdown Stage (L120) |  | 0 |
| 2232 | 0x08B8 | `STG_SHOWDOWN_BG` | 115 | Showdown BG (L121) |  | 0 |
| 2233 | 0x08B9 | `STG_SPIRALM_HEADER` | 116 | SpiralM Header (L122) |  | 0 |
| 2234 | 0x08BA | `STG_SPIRALM` | 117 | SpiralM Stage (L123) |  | 0 |
| 2235 | 0x08BB | `STG_SPIRALM_BG` | 118 | SpiralM BG (L124) |  | 0 |
| 2236 | 0x08BC | `STG_N64_HEADER` | 119 | N64 Header (L125) |  | 0 |
| 2237 | 0x08BD | `STG_N64` | 120 | N64 Stage (L126) |  | 0 |
| 2238 | 0x08BE | `STG_N64_BG` | 121 | N64 BG (L127) |  | 0 |
| 2239 | 0x08BF | `STG_MUTE_HEADER` | 122 | Mute City DL Header (L128) |  | 0 |
| 2240 | 0x08C0 | `STG_MUTE` | 123 | Mute City DL Stage (L129) |  | 0 |
| 2241 | 0x08C1 | `STG_MUTE_BG` | 124 | Mute City DL BG (L130) |  | 0 |
| 2242 | 0x08C2 | `STG_MADMM_HEADER` | 125 | Mad Monster Mansion Header (L131) |  | 0 |
| 2243 | 0x08C3 | `STG_MADMM` | 126 | Mad Monster Mansion Stage (L132) |  | 0 |
| 2244 | 0x08C4 | `STG_MADMM_BG` | 127 | Mad Monster Mansion BG (L133) |  | 0 |
| 2245 | 0x08C5 | `STG_SMBBF_HEADER` | 128 | Mushroom Kingdom DL Header (L134) |  | 0 |
| 2246 | 0x08C6 | `STG_SMBBF` | 129 | Mushroom Kingdom DL Stage (L135) |  | 0 |
| 2247 | 0x08C7 | `STG_SMBO_HEADER` | 130 | Mushroom Kingdom O Header (L136) |  | 0 |
| 2248 | 0x08C8 | `STG_SMBO` | 131 | Mushroom Kingdom O Stage (L137) |  | 0 |
| 2249 | 0x08C9 | `YLINK_USMASH` | 132 | Young Link's Up Smash (L138) | `src/NYoungLink/NYoungLink.asm`, `src/YoungLink/YoungLink.asm` | 2 |
| 2250 | 0x08CA | `STG_BOWSERBOSS_HEADER` | 133 | Bowser's Stadium Header (L139) |  | 0 |
| 2251 | 0x08CB | `STG_BOWSERBOSS` | 134 | Bowser's Stadium Stage (L140) |  | 0 |
| 2252 | 0x08CC | `STG_BOWSERBOSS_BG` | 135 | Bowser's Stadium BG (L141) |  | 0 |
| 2253 | 0x08CD | `STG_PEACH2_HEADER` | 136 | Peach's Castle II Header (L142) |  | 0 |
| 2254 | 0x08CE | `STG_PEACH2` | 137 | Peach's Castle II Stage (L143) |  | 0 |
| 2255 | 0x08CF | `STG_PEACH2_BG` | 138 | Peach's Castle II BG (L144) |  | 0 |
| 2256 | 0x08D0 | `STG_DELFINO_HEADER` | 139 | Delfino Plaza Header (L145) |  | 0 |
| 2257 | 0x08D1 | `STG_DELFINO` | 140 | Delfino Plaza Stage (L146) |  | 0 |
| 2258 | 0x08D2 | `STG_DELFINO_BG` | 141 | Delfino Plaza BG (L147) |  | 0 |
| 2259 | 0x08D3 | `DSAMUS_MAIN` | 142 | Dark Samus Main (L148) | `src/Character.asm` | 1 |
| 2260 | 0x08D4 | `DSAMUS_CHARACTER` | 143 | Dark Samus Character (L149) | `src/Character.asm` | 1 |
| 2261 | 0x08D5 | `DSAMUS_SECONDARY` | 144 | Dark Samus Secondary File (L150) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 2262 | 0x08D6 | `DSAMUS_NSP` | 145 | Dark Samus Neutral Special Graphic (L151) |  | 0 |
| 2263 | 0x08D7 | `SSONIC_1P_POSE` | 146 | Super Sonic 1p Pose (L152) | `src/SSonic/SSonic.asm` | 1 |
| 2264 | 0x08D8 | `DSAMUS_ROLLF` | 147 | Dark Samus Roll F (L153) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2265 | 0x08D9 | `DSAMUS_ROLLB` | 148 | Dark Samus Roll B (L154) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2266 | 0x08DA | `DSAMUS_JUMPF` | 149 | Dark Samus Jump F (L155) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2267 | 0x08DB | `DSAMUS_JUMPB` | 150 | Dark Samus Jump B (L156) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2268 | 0x08DC | `DSAMUS_DASH` | 151 | Dark Samus Dash (L157) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2269 | 0x08DD | `DSAMUS_RUN` | 152 | Dark Samus Run (L158) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2270 | 0x08DE | `STG_CORNERIA2_HEADER` | 153 | Corneria Header (L159) |  | 0 |
| 2271 | 0x08DF | `STG_CORNERIA2` | 154 | Corneria Stage (L160) |  | 0 |
| 2272 | 0x08E0 | `STG_CORNERIA2_BG` | 155 | Corneria BG (L161) |  | 0 |
| 2273 | 0x08E1 | `STG_KITCHEN_HEADER` | 156 | Kitchen Island Header (L162) |  | 0 |
| 2274 | 0x08E2 | `STG_KITCHEN` | 157 | Kitchen Island Stage (L163) |  | 0 |
| 2275 | 0x08E3 | `STG_KITCHEN_BG` | 158 | Kitchen Island BG (L164) |  | 0 |
| 2276 | 0x08E4 | `DSAMUS_DTILT` | 159 | Dark Samus Down Tilt (L165) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2277 | 0x08E5 | `DSAMUS_DSMASH` | 160 | Dark Samus Down Smash (L166) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2278 | 0x08E6 | `DSAMUS_FDOUBLEJUMP` | 161 | Dark Samus F Double Jump (L167) |  | 0 |
| 2279 | 0x08E7 | `DSAMUS_BDOUBLEJUMP` | 162 | Dark Samus B Double Jump (L168) |  | 0 |
| 2280 | 0x08E8 | `DSAMUS_UPSMASH` | 163 | Dark Samus Up Smash (L169) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2281 | 0x08E9 | `DSAMUS_NAIR` | 164 | Dark Samus Nair (L170) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2282 | 0x08EA | `DSAMUS_FAIR` | 165 | Dark Samus Fair (L171) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2283 | 0x08EB | `DSAMUS_BAIR` | 166 | Dark Samus Bair (L172) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2284 | 0x08EC | `STG_BLUE_HEADER` | 167 | Big Blue Header (L173) |  | 0 |
| 2285 | 0x08ED | `STG_BLUE` | 168 | Big Blue Stage (L174) |  | 0 |
| 2286 | 0x08EE | `STG_BLUE_BG` | 169 | Big Blue BG (L175) |  | 0 |
| 2287 | 0x08EF | `DSAMUS_WALK_3` | 170 | Dark Samus Walk 3 (L176) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2288 | 0x08F0 | `STG_ONETT_HEADER` | 171 | Onett Header (L177) |  | 0 |
| 2289 | 0x08F1 | `STG_ONETT` | 172 | Onett Stage (L178) |  | 0 |
| 2290 | 0x08F2 | `STG_ONETT_BG` | 173 | Onett BG (L179) |  | 0 |
| 2291 | 0x08F3 | `STG_ZLANDING_HEADER` | 174 | Zebes Landing Header (L180) |  | 0 |
| 2292 | 0x08F4 | `STG_ZLANDING` | 175 | Zebes Landing Stage (L181) |  | 0 |
| 2293 | 0x08F5 | `STG_ZLANDING_BG` | 176 | Zebes Landing BG (L182) |  | 0 |
| 2294 | 0x08F6 | `DSAMUS_VICTORY1` | 177 | Dark Samus Victory1 (L183) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2295 | 0x08F7 | `DSAMUS_SELECT` | 178 | Dark Samus Select (L184) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 4 |
| 2296 | 0x08F8 | `WARIO_MAIN` | 179 | Wario Main (L185) | `src/Character.asm` | 1 |
| 2297 | 0x08F9 | `WARIO_CHARACTER` | 180 | Wario Character (L186) | `src/Character.asm` | 1 |
| 2298 | 0x08FA | `WARIO_USP` | 181 | Wario Up Special (L187) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2299 | 0x08FB | `WARIO_JAB_1` | 182 | Wario Jab 1 (L188) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2300 | 0x08FC | `WARIO_JAB_2` | 183 | Wario Jab 2 (L189) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2301 | 0x08FD | `WARIO_FTILT_HIGH` | 184 | Wario FTilt High (L190) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2302 | 0x08FE | `WARIO_FTILT` | 185 | Wario FTilt (L191) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2303 | 0x08FF | `WARIO_FTILT_LOW` | 186 | Wario Ftilt Low (L192) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2304 | 0x0900 | `WARIO_CROUCH_BEGIN` | 187 | Wario Crouch Begin (L193) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2305 | 0x0901 | `WARIO_CROUCH_IDLE` | 188 | Wario Crouch Idle (L194) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2306 | 0x0902 | `WARIO_CROUCH_END` | 189 | Wario Crouch End (L195) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2307 | 0x0903 | `WARIO_DTILT` | 190 | Wario Down Tilt (L196) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2308 | 0x0904 | `WARIO_GRAB` | 191 | Wario Grab (L197) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2309 | 0x0905 | `WARIO_GRAB_PULL` | 192 | Wario Grab Pull (L198) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2310 | 0x0906 | `WARIO_FTHROW` | 193 | Wario Forward Throw (L199) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2311 | 0x0907 | `WARIO_BTHROW` | 194 | Wario Back Throw (L200) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2312 | 0x0908 | `WARIO_NSP_RECOIL_A` | 195 | Wario Neutral Special Recoil Air (L201) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2313 | 0x0909 | `WARIO_DSP_GROUND` | 196 | Wario Ground Down Special (L202) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2314 | 0x090A | `WARIO_DSP_AIR` | 197 | Wario Air Down Special (L203) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2315 | 0x090B | `WARIO_DSP_LANDING` | 198 | Wario Down Special Landing (L204) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2316 | 0x090C | `STG_FROSTY_HEADER` | 199 | Frosty Village Header (L205) |  | 0 |
| 2317 | 0x090D | `STG_FROSTY` | 200 | Frosty Village Stage (L206) |  | 0 |
| 2318 | 0x090E | `STG_FROSTY_BG` | 201 | Frosty Village BG (L207) |  | 0 |
| 2319 | 0x090F | `DSAMUS_TAUNT` | 202 | Dark Samus Taunt (L208) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2320 | 0x0910 | `DSAMUS_NAIR_LANDING` | 203 | Dark Samus Landing Nair (L209) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 2321 | 0x0911 | `STG_SMASHVILLE2_HEADER` | 204 | Smashville2 Header (L210) |  | 0 |
| 2322 | 0x0912 | `STG_SMASHVILLE2` | 205 | Smashville2 Stage (L211) |  | 0 |
| 2323 | 0x0913 | `STG_SMASHVILLE2_BG` | 206 | Smashville2 BG (L212) |  | 0 |
| 2324 | 0x0914 | `ELINK_MAIN` | 207 | ELink Main (L213) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2325 | 0x0915 | `JNESS_PKFIRE` | 208 | JNess PK Fire Projectile Data (L214) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2326 | 0x0916 | `JNESS_MAIN` | 209 | JNess Main (L215) | `src/Character.asm` | 1 |
| 2327 | 0x0917 | `LUCAS_MAIN` | 210 | Lucas Main (L216) | `src/Character.asm` | 1 |
| 2328 | 0x0918 | `LUCAS_CHARACTER` | 211 | Lucas Character (L217) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2329 | 0x0919 | `LUCAS_PKFIRE` | 212 | Lucas PK Fire (L218) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 2330 | 0x091A | `LUCAS_USMASH` | 213 | Lucas Up Smash (L219) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2331 | 0x091B | `LUCAS_PKFIREGROUNDANI` | 214 | Lucas PK Fire Ground Animation (L220) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2332 | 0x091C | `LUCAS_PKFIREAIRANI` | 215 | Lucas PK Fire Air Animation (L221) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2333 | 0x091D | `LUCAS_MAGNETSTARTGR` | 216 | Lucas PSI Magnet Start Ground (L222) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2334 | 0x091E | `LUCAS_MAGNETHOLDGR` | 217 | Lucas PSI Magnet Hold Ground (L223) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 4 |
| 2335 | 0x091F | `LUCAS_MAGNETRELEASEGR` | 218 | Lucas PSI Magnet Release Ground (L224) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2336 | 0x0920 | `LUCAS_MAGNETSTARTAIR` | 219 | Lucas PSI Magnet Star Air (L225) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2337 | 0x0921 | `LUCAS_MAGNETHOLDAIR` | 220 | Lucas PSI Magnet Hold Air (L226) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 4 |
| 2338 | 0x0922 | `LUCAS_MAGNETRELEASEAIR` | 221 | Lucas PSI Magnet Release Air (L227) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2339 | 0x0923 | `LUCAS_PKTHUNDERSTARTGR` | 222 | Lucas PK Thunder Start Ground (L228) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2340 | 0x0924 | `LUCAS_PKTHUNDERHOLDGR` | 223 | Lucas PK Thunder Hold Ground (L229) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2341 | 0x0925 | `LUCAS_PKTHUNDERRELEASEGR` | 224 | Lucas PK Thunder Release Ground (L230) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2342 | 0x0926 | `LUCAS_PKTHUNDERSTARTAIR` | 225 | Lucas PK Thunder Start Air (L231) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2343 | 0x0927 | `LUCAS_PKTHUNDERHOLDAIR` | 226 | Lucas PK Thunder Hold Air (L232) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2344 | 0x0928 | `LUCAS_PKTHUNDERRELEASEAIR` | 227 | Lucas PK Thunder Release Air (L233) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2345 | 0x0929 | `LUCAS_PKTHUNDER2` | 228 | Lucas PK Thunder 2 (L234) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 4 |
| 2346 | 0x092A | `LUCAS_SFALL` | 229 | Lucas Special Fall (L235) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2347 | 0x092B | `STG_DRMARIOTARGET_HEADER` | 230 | Dr. Mario Target Header (L236) |  | 0 |
| 2348 | 0x092C | `STG_DRMARIOTARGET` | 231 | Dr. Mario Target Stage (L237) |  | 0 |
| 2349 | 0x092D | `SINGLEPLAYER_VICTORY_IMAGE_BOTTOM` | 232 | Single Player Victor Image Bottom (L238) | `src/Lanky/Lanky.asm`, `src/SinglePlayer.asm` | 18 |
| 2350 | 0x092E | `SINGLEPLAYER_VICTORY_IMAGE_TOP` | 233 | Single Player Victor Image Top (L239) | `src/SinglePlayer.asm` | 1 |
| 2351 | 0x092F | `LUCAS_JAB1` | 234 | Lucas Jab 1 (L240) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2352 | 0x0930 | `LUCAS_JAB2` | 235 | Lucas Jab 2 (L241) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2353 | 0x0931 | `LUCAS_JAB3` | 236 | Lucas Jab 3 (L242) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2354 | 0x0932 | `LUCAS_DASHATTACK` | 237 | Lucas Dash Attack (L243) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2355 | 0x0933 | `LUCAS_FTILT_MID` | 238 | Lucas Forward Tilt Mid (L244) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2356 | 0x0934 | `LUCAS_UTILT` | 239 | Lucas Up Tilt (L245) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2357 | 0x0935 | `LUCAS_DSMASH` | 240 | Lucas Down Smash (L246) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2358 | 0x0936 | `LUCAS_NAIR` | 241 | Lucas Neutral Air (L247) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2359 | 0x0937 | `LUCAS_FAIR` | 242 | Lucas Forward Air (L248) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2360 | 0x0938 | `LUCAS_BAIR` | 243 | Lucas Back Air (L249) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2361 | 0x0939 | `LUCAS_DAIR` | 244 | Lucas Down Air (L250) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2362 | 0x093A | `STG_GNDTARGET_HEADER` | 245 | Ganondorf Target Header (L251) |  | 0 |
| 2363 | 0x093B | `STG_GNDTARGET` | 246 | Ganondorf Target Stage (L252) |  | 0 |
| 2364 | 0x093C | `STG_FALLS_HEADER` | 247 | Congo Falls Header (L253) |  | 0 |
| 2365 | 0x093D | `STG_FALLS` | 248 | Congo Falls Stage (L254) |  | 0 |
| 2366 | 0x093E | `STG_FALLS_BG` | 249 | Congo Falls BG (L255) |  | 0 |
| 2367 | 0x093F | `LUCAS_FTILT_HIGH` | 250 | Lucas Forward Tilt High (L256) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2368 | 0x0940 | `LUCAS_FTILT_LOW` | 251 | Lucas Forward Tilt Low (L257) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2369 | 0x0941 | `STG_BATTLEFIELD_DL_HEADER` | 252 | Battlefield DL Header (L258) |  | 0 |
| 2370 | 0x0942 | `STG_BATTLEFIELD_DL` | 253 | Battlefield DL Stage (L259) |  | 0 |
| 2371 | 0x0943 | `DRM_BTHROW` | 254 | Doctor Mario Backthrow (L260) | `src/DrLuigi/DrLuigi.asm`, `src/DrMario/DrMario.asm`, `src/Ebi/Ebi.asm` +1 | 4 |
| 2372 | 0x0944 | `STG_DSTARGET_HEADER` | 255 | Dark Samus Target Header (L261) |  | 0 |
| 2373 | 0x0945 | `STG_DSTARGET` | 256 | Dark Samus Target Stage (L262) |  | 0 |
| 2374 | 0x0946 | `WARIO_JUMP_F` | 257 | Wario Jump F (L263) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2375 | 0x0947 | `WARIO_JUMP_B` | 258 | Wario Jump B (L264) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2376 | 0x0948 | `WARIO_JUMP_AERIAL_F` | 259 | Wario Jump Aerial F (L265) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2377 | 0x0949 | `WARIO_JUMP_AERIAL_B` | 260 | Wario Jump Aerial B (L266) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2378 | 0x094A | `WARIO_DASH` | 261 | Wario Dash (L267) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2379 | 0x094B | `WARIO_RUN` | 262 | Wario Run (L268) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2380 | 0x094C | `WARIO_RUN_BRAKE` | 263 | Wario Run Brake (L269) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2381 | 0x094D | `WARIO_TURN` | 264 | Wario Turn (L270) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2382 | 0x094E | `WARIO_RUN_TURN` | 265 | Wario Run Turn (L271) | `src/NWario/NWario.asm`, `src/Slippy/Slippy.asm`, `src/Wario/Wario.asm` | 3 |
| 2383 | 0x094F | `WARIO_DASH_ATTACK` | 266 | Wario Dash Attack (L272) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2384 | 0x0950 | `WARIO_LANDING` | 267 | Wario Landing (L273) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 12 |
| 2385 | 0x0951 | `WARIO_LANDING_U` | 268 | Wario U-Air Landing (L274) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2386 | 0x0952 | `WARIO_LEDGE_ATK_F_1` | 269 | Wario Ledge Attack F 1 (L275) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2387 | 0x0953 | `WARIO_LEDGE_ATK_F_2` | 270 | Wario Ledge Attack F 2 (L276) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2388 | 0x0954 | `WARIO_VICTORY_1` | 271 | Wario Victory 1 (L277) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2389 | 0x0955 | `WARIO_VICTORY_2` | 272 | Wario Victory 2 (L278) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2390 | 0x0956 | `WARIO_VICTORY_3` | 273 | Wario Victory 3 (L279) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 4 |
| 2391 | 0x0957 | `WARIO_ENTRY_R` | 274 | Wario Entry R (L280) | `src/Wario/Wario.asm` | 1 |
| 2392 | 0x0958 | `WARIO_ENTRY_L` | 275 | Wario Entry L (L281) | `src/Wario/Wario.asm` | 1 |
| 2393 | 0x0959 | `STG_STAGE1TARGET_HEADER` | 276 | Polygon Target Header (L282) |  | 0 |
| 2394 | 0x095A | `STG_STAGE1TARGET` | 277 | Polygon Target Stage (L283) |  | 0 |
| 2395 | 0x095B | `SHEIK_TEAM_POSE` | 278 | Sheik Team Pose (L284) | `src/SinglePlayerModes.asm` | 3 |
| 2396 | 0x095C | `WARIO_POSE_1P` | 279 | Wario Pose 1P (L285) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2397 | 0x095D | `GND_POSE_1P` | 280 | Ganondorf Pose 1P (L286) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 2 |
| 2398 | 0x095E | `SPLASH_IMAGE_BOTTOM` | 281 | Splash Screen Image Bottom (L287) | `src/SinglePlayer.asm` | 1 |
| 2399 | 0x095F | `SPLASH_IMAGE_TOP` | 282 | Splash Screen Image Top (L288) | `src/SinglePlayer.asm` | 1 |
| 2400 | 0x0960 | `STG_FALCOTARGET_HEADER` | 283 | Falco Target Test Header (L289) |  | 0 |
| 2401 | 0x0961 | `STG_FALCOTARGET` | 284 | Falco Target Test Stage (L290) |  | 0 |
| 2402 | 0x0962 | `FALCO_NSP_AIR` | 285 | Falco Aerial Neutral Special (L291) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm` | 2 |
| 2403 | 0x0963 | `FALCO_CLAP` | 286 | Falco Clap (L292) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm` | 2 |
| 2404 | 0x0964 | `FALCO_SELECT` | 287 | Falco Select Pose (L293) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm` | 4 |
| 2405 | 0x0965 | `FALCO_POSE_1P` | 288 | Falco Pose 1P (L294) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm` | 2 |
| 2406 | 0x0966 | `STG_YLTARGET_HEADER` | 289 | Young Link Target Test Header (L295) |  | 0 |
| 2407 | 0x0967 | `STG_YLTARGET` | 290 | Young Link Target Test Stage (L296) |  | 0 |
| 2408 | 0x0968 | `STG_WARIOTARGET_HEADER` | 291 | Wario Target Test Header (L297) |  | 0 |
| 2409 | 0x0969 | `STG_WARIOTARGET` | 292 | Wario Target Test Stage (L298) |  | 0 |
| 2410 | 0x096A | `GND_VICTORY1` | 293 | Ganondorf Victory Animation 1 (L299) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 3 |
| 2411 | 0x096B | `GND_SELECT` | 294 | Ganondorf Selected Animation (L300) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 2 |
| 2412 | 0x096C | `JLINK_MAIN` | 295 | JLink Main (L301) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2413 | 0x096D | `JLINK_CHARACTER` | 296 | JLink Character (L302) | `src/Character.asm` | 1 |
| 2414 | 0x096E | `JFALCON_MAIN` | 297 | JFalcon Main (L303) | `src/Character.asm` | 1 |
| 2415 | 0x096F | `JFOX_MAIN` | 298 | JFox Main (L304) | `src/Character.asm` | 1 |
| 2416 | 0x0970 | `JFOX_PROJECTILE` | 299 | JFox Projectile (L305) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2417 | 0x0971 | `LUCAS_GRAB` | 300 | Lucas Grab (L306) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2418 | 0x0972 | `LUCAS_GRAB_PULL` | 301 | Lucas Grab Pull (L307) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2419 | 0x0973 | `JMARIO_MAIN` | 302 | JMario Main (L308) | `src/Character.asm` | 1 |
| 2420 | 0x0974 | `JMARIO_CHARACTER` | 303 | JMario Character (L309) | `src/Character.asm` | 1 |
| 2421 | 0x0975 | `JLUIGI_MAIN` | 304 | JLuigi Main (L310) | `src/Character.asm` | 1 |
| 2422 | 0x0976 | `JLUIGI_CHARACTER` | 305 | JLuigi Character (L311) | `src/Character.asm` | 1 |
| 2423 | 0x0977 | `LUCAS_FTHROW` | 306 | Lucas Forward Throw (L312) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2424 | 0x0978 | `JMARIO_PROJECTILE_HITBOX` | 307 | JMario Fireball Projectile Hitbox (L313) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2425 | 0x0979 | `JLUIGI_PROJECTILE_HITBOX` | 308 | JLuigi Fireball Projectile Hitbox (L314) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2426 | 0x097A | `LUCAS_UAIR` | 309 | Lucas Up Aerial (L315) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2427 | 0x097B | `LUCAS_DTILT` | 310 | Lucas Down Tilt (L316) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2428 | 0x097C | `JDK_MAIN` | 311 | JDK Main (L317) | `src/Character.asm` | 1 |
| 2429 | 0x097D | `JPUFF_MAIN` | 312 | JPuff Main (L318) | `src/Character.asm` | 1 |
| 2430 | 0x097E | `JKIRBY_MAIN` | 313 | JKirby Main (L319) | `src/Character.asm` | 1 |
| 2431 | 0x097F | `JYOSHI_MAIN` | 314 | JYoshi Main (L320) | `src/Character.asm` | 1 |
| 2432 | 0x0980 | `LUCAS_TAUNT` | 315 | Lucas Taunt (L321) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2433 | 0x0981 | `STG_HYRULETEMPLE_HEADER` | 316 | Hyrule Temple Header (L322) |  | 0 |
| 2434 | 0x0982 | `STG_HYRULETEMPLE` | 317 | Hyrule Temple Stage (L323) |  | 0 |
| 2435 | 0x0983 | `STG_HYRULETEMPLE_BG` | 318 | Hyrule Temple BG (L324) |  | 0 |
| 2436 | 0x0984 | `LUCAS_SELECTED` | 319 | Lucas Selected Animation (L325) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2437 | 0x0985 | `STG_LUCASTARGET_HEADER` | 320 | Lucas Break the Targets Header (L326) |  | 0 |
| 2438 | 0x0986 | `STG_LUCASTARGET` | 321 | Lucas Break the Targets Stage (L327) |  | 0 |
| 2439 | 0x0987 | `JPIKA_MAIN` | 322 | JPika Main (L328) | `src/Character.asm` | 1 |
| 2440 | 0x0988 | `JPIKA_PROJECTILE` | 323 | JPika Projectile (L329) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2441 | 0x0989 | `STG_GNDPLATFORM_HEADER` | 324 | Ganondorf Board the Platforms Header (L330) |  | 0 |
| 2442 | 0x098A | `STG_GNDPLATFORM` | 325 | Ganondorf Board the Platforms Stage (L331) |  | 0 |
| 2443 | 0x098B | `ESAMUS_MAIN` | 326 | ESamus Main (L332) | `src/Character.asm` | 1 |
| 2444 | 0x098C | `STG_NPC_HEADER` | 327 | New Pork City Header (L333) |  | 0 |
| 2445 | 0x098D | `STG_NPC_STAGE` | 328 | New Pork City Stage (L334) |  | 0 |
| 2446 | 0x098E | `STG_NPC_BG` | 329 | New Pork City BG (L335) |  | 0 |
| 2447 | 0x098F | `STG_DSPLATFORM_HEADER` | 330 | Dark Samus Board the Platforms Header (L336) |  | 0 |
| 2448 | 0x0990 | `STG_DSPLATFORM` | 331 | Dark Samus Board the Platforms Stage (L337) |  | 0 |
| 2449 | 0x0991 | `STG_SMASHKETBALL_HEADER` | 332 | Smashketball Header (L338) |  | 0 |
| 2450 | 0x0992 | `STG_SMASHKETBALL` | 333 | Smashketball Stage (L339) |  | 0 |
| 2451 | 0x0993 | `STG_SMASHKETBALL_BG` | 334 | Smashketball BG (L340) |  | 0 |
| 2452 | 0x0994 | `LUCAS_1P` | 335 | Lucas 1P Pose (L341) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2453 | 0x0995 | `STG_DRMPLATFORM_HEADER` | 336 | Dr. Mario Board the Platforms Header (L342) |  | 0 |
| 2454 | 0x0996 | `STG_DRMPLATFORM` | 337 | Dr. Mario Board the Platforms Stage (L343) |  | 0 |
| 2455 | 0x0997 | `LUCAS_NEEDLE_ANIM` | 338 | Lucas Victory Needle (L344) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 2456 | 0x0998 | `STG_NORFAIR_HEADER` | 339 | Norfair Header (L345) |  | 0 |
| 2457 | 0x0999 | `STG_NORFAIR` | 340 | Norfair Stage (L346) |  | 0 |
| 2458 | 0x099A | `STG_NORFAIR_BG` | 341 | Norfair BG (L347) |  | 0 |
| 2459 | 0x099B | `STG_NORFAIR_LAVA` | 342 | Norfair Lava (L348) |  | 0 |
| 2460 | 0x099C | `RAIDBLUE_HEADER` | 343 | Raid Blue Header (L349) |  | 0 |
| 2461 | 0x099D | `RAIDBLUE_STAGE` | 344 | Raid Blue Stage (L350) |  | 0 |
| 2462 | 0x099E | `RAIDBLUE_BG` | 345 | Raid Blue BG (L351) |  | 0 |
| 2463 | 0x099F | `STG_CORNERIACITY_FILE2` | 346 | Corneria City File 2 (L352) |  | 0 |
| 2464 | 0x09A0 | `STG_TAZMILY_HEADER` | 347 | Osohe Castle Header (L353) |  | 0 |
| 2465 | 0x09A1 | `STG_TAZMILY` | 348 | Osohe Castle (L354) |  | 0 |
| 2466 | 0x09A2 | `STG_TAZMILY_BG` | 349 | Osohe Castle BG (L355) |  | 0 |
| 2467 | 0x09A3 | `STG_YOSHI_STORY_II_HEADER` | 350 | Yoshi's Story II Header (L356) |  | 0 |
| 2468 | 0x09A4 | `STG_YOSHI_STORY_II` | 351 | Yoshi's Story II (L357) |  | 0 |
| 2469 | 0x09A5 | `STG_YOSHI_STORY_II_BG` | 352 | Yoshi's Story II BG (L358) |  | 0 |
| 2470 | 0x09A6 | `VS_COMBO_METER_TEXTURES` | 353 | Combo Meter Textures (L359) | `src/ComboMeter.asm` | 1 |
| 2471 | 0x09A7 | `LUCAS_ENTRY_R` | 354 | Lucas Entry Animation Right (L360) | `src/Lucas/Lucas.asm` | 1 |
| 2472 | 0x09A8 | `LUCAS_ENTRY_L` | 355 | Lucas Entry Animation Left (L361) | `src/Lucas/Lucas.asm` | 1 |
| 2473 | 0x09A9 | `STG_WORLD1_HEADER` | 356 | World 1-1 Header (L362) |  | 0 |
| 2474 | 0x09AA | `STG_WORLD1` | 357 | World 1-1 (L363) |  | 0 |
| 2475 | 0x09AB | `STG_FLATZONE2_HEADER` | 358 | Flat Zone II Header (L364) |  | 0 |
| 2476 | 0x09AC | `STG_FLATZONE2` | 359 | Flat Zone II (L365) |  | 0 |
| 2477 | 0x09AD | `STG_GERUDO_HEADER` | 360 | Gerudo Valley Header (L366) |  | 0 |
| 2478 | 0x09AE | `STG_GERUDO` | 361 | Gerudo Valley (L367) |  | 0 |
| 2479 | 0x09AF | `STG_GERUDO_BG` | 362 | Gerudo Valley BG (L368) |  | 0 |
| 2480 | 0x09B0 | `LUCAS_PKVICTORY` | 363 | Lucas PK Victory Animation (L369) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 3 |
| 2481 | 0x09B1 | `BOWSER_BOMB` | 364 | Bowser Bomb (L370) |  | 0 |
| 2482 | 0x09B2 | `BOWSER_BOMB_HEADER` | 365 | Bowser Bomb Header (L371) | `src/Hazards.asm` | 1 |
| 2483 | 0x09B3 | `STG_YLPLATFORM_HEADER` | 366 | Young Link Board the Platforms Header (L372) |  | 0 |
| 2484 | 0x09B4 | `STG_YLPLATFORM` | 367 | Young Link Board the Platforms Stage (L373) |  | 0 |
| 2485 | 0x09B5 | `EPIKA_MAIN` | 368 | E Pikachu Main (L374) | `src/Character.asm` | 1 |
| 2486 | 0x09B6 | `BOWSER_MAIN` | 369 | Bowser Main (L375) | `src/Character.asm` | 1 |
| 2487 | 0x09B7 | `BOWSER_CHARACTER` | 370 | Bowser Character (L376) | `src/Character.asm` | 1 |
| 2488 | 0x09B8 | `BOWSER_USP_GROUND` | 371 | Bowser Up Special Ground (L377) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2489 | 0x09B9 | `BOWSER_USP_AIR` | 372 | Bowser Up Special Air (L378) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2490 | 0x09BA | `BOWSER_SFALL` | 373 | Bowser Special Fall (L379) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2491 | 0x09BB | `BOWSER_IDLE` | 374 | Bowser Idle (L380) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 21 |
| 2492 | 0x09BC | `BOWSER_JAB1` | 375 | Bowser Jab 1 (L381) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2493 | 0x09BD | `BOWSER_JAB2` | 376 | Bowser Jab 2 (L382) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2494 | 0x09BE | `BOWSER_LANDING` | 377 | Bowser Landing (L383) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 21 |
| 2495 | 0x09BF | `BOWSER_FIRE` | 378 | Bowser Fire Breath (L384) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2496 | 0x09C0 | `BOWSER_FALL` | 379 | Bowser Falling (L385) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2497 | 0x09C1 | `BOWSER_FAIR` | 380 | Bowser Forward Air (L386) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2498 | 0x09C2 | `BOWSER_GRAB` | 381 | Bowser Grab (L387) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2499 | 0x09C3 | `BOWSER_PULL` | 382 | Bowser Grab Pull (L388) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2500 | 0x09C4 | `BOWSER_FSMASH` | 383 | Bowser Forward Smash (L389) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2501 | 0x09C5 | `BOWSER_FTHROW` | 384 | Bowser Forward Throw 1 (L390) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2502 | 0x09C6 | `BOWSER_FTHROW_2` | 385 | Bowser Forward Throw 2 (L391) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2503 | 0x09C7 | `BOWSER_FTHROW_3` | 386 | Bowser Forward Throw 3 (L392) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2504 | 0x09C8 | `BOWSER_BOMB_GROUND` | 387 | Bowser Bomb Ground (L393) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2505 | 0x09C9 | `BOWSER_BOMB_AIR` | 388 | Bowser Bomb Air (L394) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2506 | 0x09CA | `BOWSER_BOMB_LAND` | 389 | Bowser Bomb Land (L395) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2507 | 0x09CB | `BOWSER_DAMAGE_HIGH_1` | 390 | Bowser Damage High 1 (L396) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2508 | 0x09CC | `BOWSER_DAMAGE_HIGH_2` | 391 | Bowser Damage High 2 (L397) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2509 | 0x09CD | `BOWSER_DAMAGE_HIGH_3` | 392 | Bowser Damage High 3 (L398) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2510 | 0x09CE | `BOWSER_DAMAGE_MID_1` | 393 | Bowser Damage Mid 1 (L399) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2511 | 0x09CF | `BOWSER_DAMAGE_MID_2` | 394 | Bowser Damage Mid 2 (L400) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2512 | 0x09D0 | `BOWSER_DAMAGE_MID_3` | 395 | Bowser Damage Mid 3 (L401) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2513 | 0x09D1 | `BOWSER_DAMAGE_LOW_1` | 396 | Bowser Damage Low 1 (L402) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2514 | 0x09D2 | `BOWSER_DAMAGE_LOW_2` | 397 | Bowser Damage Low 2 (L403) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2515 | 0x09D3 | `BOWSER_DAMAGE_LOW_3` | 398 | Bowser Damage Low 3 (L404) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2516 | 0x09D4 | `BOWSER_DAMAGE_AIR_1` | 399 | Bowser Damage Air 1 (L405) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2517 | 0x09D5 | `BOWSER_DAMAGE_AIR_2` | 400 | Bowser Damage Air 2 (L406) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2518 | 0x09D6 | `BOWSER_DAMAGE_AIR_3` | 401 | Bowser Damage Air 3 (L407) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2519 | 0x09D7 | `BOWSER_DAMAGE_FLY_MID` | 402 | Bowser Damage Fly Mid (L408) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2520 | 0x09D8 | `BOWSER_DAMAGE_FLY_LOW` | 403 | Bowser Damage Fly Low (L409) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2521 | 0x09D9 | `BOWSER_DAMAGE_FLY_TOP` | 404 | Bowser Damage Fly Top (L410) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2522 | 0x09DA | `BOWSER_DAMAGE_FLY_ROLL` | 405 | Bowser Damage Fly Roll (L411) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2523 | 0x09DB | `BOWSER_TUMBLE` | 406 | Bowser Tumble (L412) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 30 |
| 2524 | 0x09DC | `BOWSER_CEILING_BONK` | 407 | Bowser Ceiling Bonk (L413) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2525 | 0x09DD | `BOWSER_DOWN_BOUNCE_D` | 408 | Bowser Down Bounce D (L414) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 9 |
| 2526 | 0x09DE | `BOWSER_DOWN_BOUNCE_U` | 409 | Bowser Down Bounce U (L415) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2527 | 0x09DF | `BOWSER_DOWN_STAND_D` | 410 | Bowser Down Stand D (L416) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 9 |
| 2528 | 0x09E0 | `BOWSER_TECH` | 411 | Bowser Tech (L417) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2529 | 0x09E1 | `BOWSER_LIGHT_ITEM_PICK` | 412 | Bowser Light Item Pick up (L418) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2530 | 0x09E2 | `BOWSER_ITEM_DROP` | 413 | Bowser Item Drop (L419) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2531 | 0x09E3 | `BOWSER_EGG_LAY_PULLED` | 414 | Bowser Egg Lay Pulled (L420) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2532 | 0x09E4 | `BOWSER_THROWN_DK_PULLED` | 415 | Bowser Thrown DK Pulled (L421) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm` | 2 |
| 2533 | 0x09E5 | `BOWSER_THROWN_DK` | 416 | Bowser Thrown DK (L422) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm` | 2 |
| 2534 | 0x09E6 | `BOWSER_THROWN_1` | 417 | Bowser Thrown 1 (L423) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2535 | 0x09E7 | `BOWSER_THROWN_2` | 418 | Bowser Thrown 2 (L424) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2536 | 0x09E8 | `BOWSER_TURN` | 419 | Bowser Turn (L425) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2537 | 0x09E9 | `BOWSER_LAUGH` | 420 | Bowser Laugh (L426) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 4 |
| 2538 | 0x09EA | `BOWSER_CROUCH` | 421 | Bowser Crouch (L427) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2539 | 0x09EB | `BOWSER_CROUCH_IDLE` | 422 | Bowser Crouch Idle (L428) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2540 | 0x09EC | `BOWSER_CROUCH_END` | 423 | Bowser Crouch End (L429) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2541 | 0x09ED | `BOWSER_FAIR_LAND` | 424 | Bowser Fair Land (L430) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2542 | 0x09EE | `BOWSER_RAYGUN_GND` | 425 | Bowser Raygun Ground (L431) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2543 | 0x09EF | `BOWSER_RAYGUN_AIR` | 426 | Bowser Raygun Air (L432) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2544 | 0x09F0 | `BOWSER_BTHROW` | 427 | Bowser Back Throw (L433) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2545 | 0x09F1 | `BOWSER_DTILT` | 428 | Bowser Down Tilt (L434) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2546 | 0x09F2 | `BOWSER_NAIR` | 429 | Bowser Neutral Air (L435) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2547 | 0x09F3 | `BOWSER_DAIR` | 430 | Bowser Down Air (L436) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2548 | 0x09F4 | `BOWSER_DAIR_LAND` | 431 | Bowser Down  Air Land (L437) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2549 | 0x09F5 | `BOWSER_UTILT` | 432 | Bowser Up Tilt (L438) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2550 | 0x09F6 | `BOWSER_TEETER_START` | 433 | Bowser Teeter Start (L439) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2551 | 0x09F7 | `BOWSER_TEETER_LOOP` | 434 | Bowser Teeter Loop (L440) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2552 | 0x09F8 | `BOWSER_FTILT` | 435 | Bowser Forward Tilt (L441) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2553 | 0x09F9 | `BOWSER_USMASH` | 436 | Bowser Up Smash (L442) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2554 | 0x09FA | `BOWSER_DASH` | 437 | Bowser Dash (L443) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2555 | 0x09FB | `BOWSER_RUN` | 438 | Bowser Run (L444) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2556 | 0x09FC | `BOWSER_DASH_ATTACK` | 439 | Bowser Dash Attack (L445) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2557 | 0x09FD | `BOWSER_CSS` | 440 | Bowser CSS (L446) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2558 | 0x09FE | `BOWSER_JUMPF` | 441 | Bowser Jump Forward (L447) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2559 | 0x09FF | `BOWSER_BAIR` | 442 | Bowser Back Air (L448) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2560 | 0x0A00 | `BOWSER_FTILT_HIGH` | 443 | Bowser FTilt High (L449) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2561 | 0x0A01 | `BOWSER_FTILT_LOW` | 444 | Bowser FTilt Low (L450) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2562 | 0x0A02 | `BOWSER_JAB3` | 445 | Bowser Jab 3 (L451) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2563 | 0x0A03 | `BOWSER_UAIR` | 446 | Bowser Up Air (L452) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2564 | 0x0A04 | `STAGE_ICONS` | 447 | Stage Icons (L453) | `src/Stages.asm` | 1 |
| 2565 | 0x0A05 | `CHARACTER_PORTRAITS` | 448 | Character Portraits (L454) | `src/CharacterSelect.asm`, `src/TwelveCharBattle.asm` | 2 |
| 2566 | 0x0A06 | `CSS_IMAGES` | 449 | CSS Images (L455) | `src/AI.asm`, `src/CharacterSelect.asm`, `src/InputDisplay.asm` +5 | 8 |
| 2567 | 0x0A07 | `REMIX_MENU_BG` | 450 | Remix Menu BG (L456) | `src/Toggles.asm` | 1 |
| 2568 | 0x0A08 | `BOWSER_CLAP` | 451 | Bowser Clap (L457) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2569 | 0x0A09 | `BOWSER_TAUNT` | 452 | Bowser Taunt (L458) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2570 | 0x0A0A | `BOWSER_DOWN_STAND_U` | 453 | Bowser Down Stand U (L459) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` +1 | 9 |
| 2571 | 0x0A0B | `BOWSER_JUMPB` | 454 | Bowser Jump Back (L460) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2572 | 0x0A0C | `BOWSER_JUMP2` | 455 | Bowser Jump 2 (L461) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2573 | 0x0A0D | `BOWSER_FALL2` | 456 | Bowser Fall 2 (L462) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2574 | 0x0A0E | `BOWSER_CLIFFATTACKQ1` | 457 | Bowser CliffAttackQuick1 (L463) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2575 | 0x0A0F | `BOWSER_CLIFFATTACKQ2` | 458 | Bowser CliffAttackQuick2 (L464) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2576 | 0x0A10 | `BOWSER_CLIFFATTACKS1` | 459 | Bowser CliffAttackSlow1 (L465) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2577 | 0x0A11 | `BOWSER_CLIFFATTACKS2` | 460 | Bowser CliffAttackSlow2 (L466) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2578 | 0x0A12 | `BOWSER_CLIFFCATCH` | 461 | Bowser CliffCatch (L467) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2579 | 0x0A13 | `BOWSER_CLIFFWAIT` | 462 | Bowser CliffWait (L468) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2580 | 0x0A14 | `BOWSER_CLIFFQUICK` | 463 | Bowser CliffQuick (L469) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2581 | 0x0A15 | `CUSTOM_FONT` | 464 | Custom Font (Kabel) (L470) | `src/Render.asm` | 1 |
| 2582 | 0x0A16 | `BOWSER_DANCE` | 465 | Bowser Victory Dance (L471) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2583 | 0x0A17 | `STG_FALCOPLATFORM_HEADER` | 466 | Falco Platform Header (L472) |  | 0 |
| 2584 | 0x0A18 | `STG_FALCOPLATFORM` | 467 | Falco Platform Stage (L473) |  | 0 |
| 2585 | 0x0A19 | `STG_POLYPLATFORM_HEADER` | 468 | Polygon Platform Header (L474) |  | 0 |
| 2586 | 0x0A1A | `STG_POLYPLATFORM` | 469 | Polygon Platform Stage (L475) |  | 0 |
| 2587 | 0x0A1B | `STG_HYRULECASTLE_DL_HEADER` | 470 | Hyrule Castle DL Header (L476) |  | 0 |
| 2588 | 0x0A1C | `STG_HYRULECASTLE_DL_` | 471 | Hyrule Castle DL Stage (L477) |  | 0 |
| 2589 | 0x0A1D | `STG_HYRULECASTLE_O_HEADER` | 472 | Hyrule Castle Omega Header (L478) |  | 0 |
| 2590 | 0x0A1E | `STG_HYRULECASTLE_O_` | 473 | Hyrule Castle Omega Stage (L479) |  | 0 |
| 2591 | 0x0A1F | `STG_CONGOJUNGLE_DL_HEADER` | 474 | Congo Jungle DL Header (L480) |  | 0 |
| 2592 | 0x0A20 | `STG_CONGOJUNGLE_DL_` | 475 | Congo Jungle DL Stage (L481) |  | 0 |
| 2593 | 0x0A21 | `STG_CONGOJUNGLE_O_HEADER` | 476 | Congo Jungle Omega Header (L482) |  | 0 |
| 2594 | 0x0A22 | `STG_CONGOJUNGLE_O_` | 477 | Congo Jungle Omega Stage (L483) |  | 0 |
| 2595 | 0x0A23 | `STG_PEACHCASTLE_DL_HEADER` | 478 | Peach's Castle DL Header (L484) |  | 0 |
| 2596 | 0x0A24 | `STG_PEACHCASTLE_DL_` | 479 | Peach's Castle DL Stage (L485) |  | 0 |
| 2597 | 0x0A25 | `STG_PEACHCASTLE_O_HEADER` | 480 | Peach's Castle Omega Header (L486) |  | 0 |
| 2598 | 0x0A26 | `STG_PEACHCASTLE_O_` | 481 | Peach's Castle Omega Stage (L487) |  | 0 |
| 2599 | 0x0A27 | `BOWSER_ENTER_PIPE` | 482 | Bowser Enter Pipe (L488) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2600 | 0x0A28 | `BOWSER_DAMAGE_ELEC` | 483 | Bowser Damage Elec (L489) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2601 | 0x0A29 | `BOWSER_CONTINUE_FALL` | 484 | Bowser Continue Fall (L490) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2602 | 0x0A2A | `BOWSER_CONTINUE_UP` | 485 | Bowser Continue Up (L491) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2603 | 0x0A2B | `BOWSER_RUN_BRAKE` | 486 | Bowser Run Brake (L492) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2604 | 0x0A2C | `BOWSER_TURN_RUN` | 487 | Bowser Turn Run (L493) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2605 | 0x0A2D | `BOWSER_1P` | 488 | Bowser 1P Pose (L494) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2606 | 0x0A2E | `STG_WARIOPLATFORM_HEADER` | 489 | Wario Board the Platforms Header (L495) |  | 0 |
| 2607 | 0x0A2F | `STG_WARIOPLATFORM` | 490 | Wario Board the Platforms Stage (L496) |  | 0 |
| 2608 | 0x0A30 | `BOWSER_EXIT_PIPE` | 491 | Bowser Exit Pipe (L497) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2609 | 0x0A31 | `BOWSER_STUNNED` | 492 | Bowser Stunned (L498) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2610 | 0x0A32 | `STG_FRAY_NIGHT_HEADER` | 493 | Fray's Stage Night Header (L499) |  | 0 |
| 2611 | 0x0A33 | `STG_FRAY_NIGHT` | 494 | Fray's Stage Night Stage (L500) |  | 0 |
| 2612 | 0x0A34 | `STG_FRAY_NIGHT_BG` | 495 | Fray's Stage Night BG (L501) |  | 0 |
| 2613 | 0x0A35 | `BOWSER_HEAVY_ITEM_THROW` | 496 | Bowser Heavy Item Throw (L502) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 12 |
| 2614 | 0x0A36 | `BOWSER_HEAVY_ITEM_PICK` | 497 | Bowser Heavy Item Pickup (L503) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2615 | 0x0A37 | `BOWSER_DOWN_FORWARD_D` | 498 | Bowser Down Forward D (L504) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2616 | 0x0A38 | `BOWSER_HAMMER_IDLE` | 499 | Bowser Hammer Idle (L505) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2617 | 0x0A39 | `BOWSER_HAMMER_WALK` | 500 | Bowser Hammer Walk (L506) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 15 |
| 2618 | 0x0A3A | `BOWSER_ITEM_THROW_DASH` | 501 | Bowser Item Throw Dash (L507) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2619 | 0x0A3B | `BOWSER_ITEM_THROW_F` | 502 | Bowser Item Throw F (L508) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 10 |
| 2620 | 0x0A3C | `BOWSER_ITEM_THROW_U` | 503 | Bowser Item Throw U (L509) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2621 | 0x0A3D | `BOWSER_ITEM_THROW_D` | 504 | Bowser Item Throw D (L510) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2622 | 0x0A3E | `BOWSER_ITEM_THROW_F_AIR` | 505 | Bowser Item Throw F Air (L511) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 12 |
| 2623 | 0x0A3F | `BOWSER_ITEM_THROW_U_AIR` | 506 | Bowser Item Throw U Air (L512) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2624 | 0x0A40 | `BOWSER_ITEM_THROW_D_AIR` | 507 | Bowser Item Throw D Air (L513) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2625 | 0x0A41 | `BOWSER_DOWN_FORWARD_U` | 508 | Bowser Down Forward U (L514) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2626 | 0x0A42 | `BOWSER_CLIFF_CLIMB_QUICK_1` | 509 | Bowser Cliff Climb Quick 1 (L515) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2627 | 0x0A43 | `BOWSER_CLIFF_CLIMB_QUICK_2` | 510 | Bowser Cliff Climb Quick 2 (L516) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2628 | 0x0A44 | `BOWSER_CLIFFSLOW` | 511 | Bowser Cliff Slow (L517) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2629 | 0x0A45 | `BOWSER_CLIFF_CLIMB_SLOW_1` | 512 | Bowser Cliff Climb Slow 1 (L518) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2630 | 0x0A46 | `BOWSER_CLIFF_CLIMB_SLOW_2` | 513 | Bowser Cliff Climb Slow 2 (L519) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2631 | 0x0A47 | `BOWSER_CLIFF_ESCAPE_QUICK_1` | 514 | Bowser Cliff Escape Quick 1 (L520) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2632 | 0x0A48 | `BOWSER_CLIFF_ESCAPE_QUICK_2` | 515 | Bowser Cliff Escape Quick 2 (L521) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2633 | 0x0A49 | `BOWSER_CLIFF_ESCAPE_SLOW_1` | 516 | Bowser Cliff Escape Slow 1 (L522) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2634 | 0x0A4A | `BOWSER_CLIFF_ESCAPE_SLOW_2` | 517 | Bowser Cliff Escape Slow 2 (L523) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2635 | 0x0A4B | `BOWSER_THROWN_MARIO` | 518 | Bowser Thrown Mario Bros. (L524) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2636 | 0x0A4C | `STG_GOOMBA_ROAD_HEADER` | 519 | Goomba Road Header (L525) |  | 0 |
| 2637 | 0x0A4D | `STG_GOOMBA_ROAD_STAGE` | 520 | Goomba Road Stage (L526) |  | 0 |
| 2638 | 0x0A4E | `STG_GOOMBA_ROAD_BG` | 521 | Goomba Road BG (L527) |  | 0 |
| 2639 | 0x0A4F | `STG_LUCASPLATFORM_HEADER` | 522 | Lucas Board the Platforms Header (L528) |  | 0 |
| 2640 | 0x0A50 | `STG_LUCASPLATFORM` | 523 | Lucas Board the Platforms Stage (L529) |  | 0 |
| 2641 | 0x0A51 | `BOWSER_DOWN_BACK_D` | 524 | Bowser Down Back D (L530) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2642 | 0x0A52 | `BOWSER_DOWN_BACK_U` | 525 | Bowser Down Back U (L531) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2643 | 0x0A53 | `BOWSER_DAMAGE_FLY_HIGH` | 526 | Bowser Damage Fly High (L532) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2644 | 0x0A54 | `BOWSER_SWING_NEUTRAL` | 527 | Bowser Swing Neutral (L533) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 12 |
| 2645 | 0x0A55 | `BOWSER_SWING_SMASH` | 528 | Bowser Swing Smash (L534) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 12 |
| 2646 | 0x0A56 | `BOWSER_SWING_DASH` | 529 | Bowser Swing Dash (L535) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 12 |
| 2647 | 0x0A57 | `BOWSER_JUMP2_B` | 530 | Bowser Double Jump B (L536) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2648 | 0x0A58 | `BOWSER_SWING_TILT` | 531 | Bowser Swing Tilt (L537) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 12 |
| 2649 | 0x0A59 | `BOWSER_CLOWN_COPTER` | 532 | Bowser Clown Copter (L538) | `src/Character.asm` | 3 |
| 2650 | 0x0A5A | `BOWSER_ENTRY` | 533 | Bowser Entry (L539) | `src/Bowser/bowser.asm` | 2 |
| 2651 | 0x0A5B | `BOWSER_WALK1` | 534 | Bowser Walk 1 (L540) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2652 | 0x0A5C | `BOWSER_WALK2` | 535 | Bowser Walk 2 (L541) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2653 | 0x0A5D | `BOWSER_WALK3` | 536 | Bowser Walk 3 (L542) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2654 | 0x0A5E | `BOWSER_DSMASH` | 537 | Bowser Down Smash (L543) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 2655 | 0x0A5F | `WARIO_CLAP` | 538 | Wario Clap (L544) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 2656 | 0x0A60 | `BOWSER_DOWN_ATTACK_U` | 539 | Bowser Down Attack U (L545) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2657 | 0x0A61 | `BOWSER_TECH_F` | 540 | Bowser Tech Forward (L546) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2658 | 0x0A62 | `BOWSER_TECH_B` | 541 | Bowser Tech Backward (L547) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2659 | 0x0A63 | `STG_SECTOR_Z_DL_HEADER` | 542 | Sector Z DL Header (L548) |  | 0 |
| 2660 | 0x0A64 | `STG_SECTOR_Z_DL_STAGE` | 543 | Sector Z DL Stage (L549) |  | 0 |
| 2661 | 0x0A65 | `STG_SECTOR_Z_DL_IMAGE2` | 544 | Saffron City O Header (L550) |  | 0 |
| 2662 | 0x0A66 | `STG_SAFFRON_O_HEADER` | 545 | Saffron City DL Header (L551) |  | 0 |
| 2663 | 0x0A67 | `STG_SAFFRON_DL_STAGE` | 546 | Saffron City DL Stage (L552) |  | 0 |
| 2664 | 0x0A68 | `STG_YOSHI_ISLAND_DL_HEADER` | 547 | Yoshi's Island DL Header (L553) |  | 0 |
| 2665 | 0x0A69 | `STG_YOSHI_ISLAND_DL_STAGE` | 548 | Yoshi's Island DL Stage (L554) |  | 0 |
| 2666 | 0x0A6A | `STG_ZEBES_DL_HEADER` | 549 | Zebes DL Header (L555) |  | 0 |
| 2667 | 0x0A6B | `STG_ZEBES_DL_STAGE` | 550 | Zebes DL Stage (L556) |  | 0 |
| 2668 | 0x0A6C | `STG_SECTOR_Z_O_HEADER` | 551 | Sector Z Omega Header (L557) |  | 0 |
| 2669 | 0x0A6D | `STG_SECTOR_Z_O_STAGE` | 552 | Sector Z Omega Stage (L558) |  | 0 |
| 2670 | 0x0A6E | `STG_SAFFRON_O` | 553 | Saffron City Omega Stage (L559) |  | 0 |
| 2671 | 0x0A6F | `STG_YOSHI_ISLAND_O_HEADER` | 554 | Yoshi's Island Omega Header (L560) |  | 0 |
| 2672 | 0x0A70 | `STG_YOSHI_ISLAND_O_STAGE` | 555 | Yoshi's Island Omega Stage (L561) |  | 0 |
| 2673 | 0x0A71 | `STG_DREAMLAND_O_HEADER` | 556 | Dreamland Omega Header (L562) |  | 0 |
| 2674 | 0x0A72 | `STG_DREAMLAND_O_STAGE` | 557 | Dreamland Omega Stage (L563) |  | 0 |
| 2675 | 0x0A73 | `STG_ZEBES_O_HEADER` | 558 | Zebes Omega Header (L564) |  | 0 |
| 2676 | 0x0A74 | `STG_ZEBES_O_STAGE` | 559 | Zebes Omega Stage (L565) |  | 0 |
| 2677 | 0x0A75 | `BOWSER_DOWN_ATTACK_D` | 560 | Bowser Down Attack D (L566) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2678 | 0x0A76 | `BOWSER_BAIR_LANDING` | 561 | Bowser Back Air Landing (L567) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2679 | 0x0A77 | `BOWSER_SHIELD_POSE` | 562 | Bowser Shield Pose (L568) | `src/Character.asm` | 3 |
| 2680 | 0x0A78 | `BOWSER_SHIELD_ON` | 563 | Bowser Shield On (L569) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2681 | 0x0A79 | `BOWSER_SHIELD_OFF` | 564 | Bowser Shield Off (L570) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2682 | 0x0A7A | `BOWSER_SHIELD_DROP` | 565 | Bowser Shield Drop (L571) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 6 |
| 2683 | 0x0A7B | `BOWSER_ROLL_F` | 566 | Bowser Roll F (L572) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2684 | 0x0A7C | `BOWSER_ROLL_B` | 567 | Bowser Roll B (L573) | `src/Bowser/bowser.asm`, `src/GBowser/gbowser.asm`, `src/NBowser/nbowser.asm` | 3 |
| 2685 | 0x0A7D | `CONGO_FALLS_BARREL` | 568 | Congo Falls Barrel (L574) |  | 0 |
| 2686 | 0x0A7E | `STG_BOWSERTARGETS_HEADER` | 569 | Bowser Targets Header (L575) |  | 0 |
| 2687 | 0x0A7F | `STG_BOWSERTARGETS` | 570 | Bowser Targets Stage (L576) |  | 0 |
| 2688 | 0x0A80 | `GBOWSER_MAIN` | 571 | Giga Bowser Main (L577) | `src/Character.asm` | 1 |
| 2689 | 0x0A81 | `GBOWSER_CHARACTER` | 572 | Giga Bowser Character (L578) | `src/Character.asm` | 1 |
| 2690 | 0x0A82 | `STG_BOWSER_PLATFORM_HEADER` | 573 | Bowser Platforms Header (L579) |  | 0 |
| 2691 | 0x0A83 | `STG_BOWSER_PLATFORM` | 574 | Bowser Platforms Stage (L580) |  | 0 |
| 2692 | 0x0A84 | `GBOWSER_ENTRY_RIGHT` | 575 | Giga Bowser Entry Right (L581) | `src/GBowser/gbowser.asm` | 1 |
| 2693 | 0x0A85 | `GBOWSER_ENTRY_LEFT` | 576 | Giga Bowser Entry Left (L582) | `src/GBowser/gbowser.asm` | 1 |
| 2694 | 0x0A86 | `GND_ENTRY_1` | 577 | Ganondorf Entry 1 (L583) | `src/Ganondorf/Ganondorf.asm` | 2 |
| 2695 | 0x0A87 | `FALCO_IDLE` | 578 | Falco Idle (L584) | `src/Falco/Falco.asm`, `src/NFalco/NFalco.asm` | 14 |
| 2696 | 0x0A88 | `GND_ENTRY_2_RIGHT` | 579 | Ganondorf Entry 2 Right (L585) | `src/Ganondorf/Ganondorf.asm` | 1 |
| 2697 | 0x0A89 | `GND_ENTRY_2_LEFT` | 580 | Ganondorf Entry 2 Left (L586) | `src/Ganondorf/Ganondorf.asm` | 1 |
| 2698 | 0x0A8A | `STG_BOWSERS_KEEP_HEADER` | 581 | Bowser's Keep Header (L587) |  | 0 |
| 2699 | 0x0A8B | `STG_BOWSERS_KEEP` | 582 | Bowser's Keep (L588) |  | 0 |
| 2700 | 0x0A8C | `STG_BOWSERS_BG` | 583 | Bowser's Keep BG (L589) |  | 0 |
| 2701 | 0x0A8D | `KIRBY_CUSTOM_HATS` | 584 | Kirby Custom Hats (L590) | `src/TagTeam.asm` | 2 |
| 2702 | 0x0A8E | `KIRBY_WARIO_NSP_G` | 585 | Kirby Wario Neutral Special Ground (L591) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2703 | 0x0A8F | `KIRBY_WARIO_NSP_A` | 586 | Kirby Wario Neutral Special Air (L592) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2704 | 0x0A90 | `KIRBY_WARIO_NSPR_A` | 587 | Kirby Wario Neutral Special Recoil Air (L593) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2705 | 0x0A91 | `KIRBY_FALCO_NSP_G` | 588 | Kirby Falco Neutral Special Ground (L594) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2706 | 0x0A92 | `KIRBY_FALCO_NSP_A` | 589 | Kirby Falco Neutral Special Air (L595) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2707 | 0x0A93 | `KIRBY_LUCAS_NSP_G` | 590 | Kirby Lucas Neutral Special Ground (L596) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2708 | 0x0A94 | `KIRBY_LUCAS_NSP_A` | 591 | Kirby Lucas Neutral Special Air (L597) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2709 | 0x0A95 | `KIRBY_BOWSER_NSP` | 592 | Kirby Bowser Neutral Special (L598) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 2710 | 0x0A96 | `CUSTOM_ITEMS` | 593 | Custom Items (L599) |  | 0 |
| 2711 | 0x0A97 | `MARTH_TEAM_POSE` | 594 | Marth Team Pose (L600) | `src/SinglePlayerModes.asm` | 4 |
| 2712 | 0x0A98 | `PIANO_MAIN` | 595 | Piano Main (L601) | `src/Character.asm` | 1 |
| 2713 | 0x0A99 | `PIANO_CHARACTER` | 596 | Piano Character (L602) | `src/Character.asm` | 1 |
| 2714 | 0x0A9A | `PIANO_SHIELD_POSE` | 597 | Piano Shield Pose (L603) |  | 0 |
| 2715 | 0x0A9B | `PIANO_PROJECTILE_HITBOX` | 598 | Piano Projectile Hitbox (L604) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 2716 | 0x0A9C | `PIANO_PROJECTILE_GRAPHIC` | 599 | Piano Projectile Graphic (L605) |  | 0 |
| 2717 | 0x0A9D | `PIANO_IDLE` | 600 | Piano Idle (L606) | `src/Piano/Piano.asm` | 7 |
| 2718 | 0x0A9E | `PIANO_DASH` | 601 | Piano Dash (L607) | `src/Piano/Piano.asm` | 1 |
| 2719 | 0x0A9F | `PIANO_RUN` | 602 | Piano Run (L608) | `src/Piano/Piano.asm` | 1 |
| 2720 | 0x0AA0 | `PIANO_WALK` | 603 | Piano Walk (L609) | `src/Piano/Piano.asm` | 3 |
| 2721 | 0x0AA1 | `PIANO_TURN` | 604 | Piano Turn (L610) | `src/Piano/Piano.asm` | 1 |
| 2722 | 0x0AA2 | `PIANO_CROUCH_BEGIN` | 605 | Piano Crouch Begin (L611) | `src/Piano/Piano.asm` | 1 |
| 2723 | 0x0AA3 | `PIANO_CROUCH_IDLE` | 606 | Piano Crouch Idle (L612) | `src/Piano/Piano.asm` | 1 |
| 2724 | 0x0AA4 | `PIANO_CROUCH_END` | 607 | Piano Crouch End (L613) | `src/Piano/Piano.asm` | 1 |
| 2725 | 0x0AA5 | `PIANO_RUN_BRAKE` | 608 | Piano Run Brake (L614) | `src/Piano/Piano.asm` | 1 |
| 2726 | 0x0AA6 | `PIANO_RUN_TURN` | 609 | Piano Run Turn (L615) | `src/Piano/Piano.asm` | 1 |
| 2727 | 0x0AA7 | `PIANO_JUMP_F` | 610 | Piano Jump F (L616) | `src/Piano/Piano.asm` | 1 |
| 2728 | 0x0AA8 | `PIANO_JUMP_B` | 611 | Piano Jump B (L617) | `src/Piano/Piano.asm` | 1 |
| 2729 | 0x0AA9 | `PIANO_JUMP_AERIAL_F` | 612 | Piano Jump Aerial F (L618) | `src/Piano/Piano.asm` | 1 |
| 2730 | 0x0AAA | `PIANO_JUMP_AERIAL_B` | 613 | Piano Jump Aerial B (L619) | `src/Piano/Piano.asm` | 1 |
| 2731 | 0x0AAB | `PIANO_FALL` | 614 | Piano Fall (L620) | `src/Piano/Piano.asm` | 1 |
| 2732 | 0x0AAC | `PIANO_FALL_AERIAL` | 615 | Piano Fall Aerial (L621) | `src/Piano/Piano.asm` | 1 |
| 2733 | 0x0AAD | `PIANO_LANDING` | 616 | Piano Landing (L622) | `src/Piano/Piano.asm` | 7 |
| 2734 | 0x0AAE | `PIANO_FTILT` | 617 | Piano Forward Tilt (L623) | `src/Piano/Piano.asm` | 1 |
| 2735 | 0x0AAF | `PIANO_ATTACK_HEAVY` | 618 | Piano Forward Smash/Dash Attack (L624) | `src/Piano/Piano.asm` | 2 |
| 2736 | 0x0AB0 | `PIANO_JAB` | 619 | Piano Jab (L625) | `src/Piano/Piano.asm` | 1 |
| 2737 | 0x0AB1 | `PIANO_UTILT` | 620 | Piano Up Tilt (L626) | `src/Piano/Piano.asm` | 1 |
| 2738 | 0x0AB2 | `PIANO_DTILT` | 621 | Piano Down Tilt (L627) | `src/Piano/Piano.asm` | 1 |
| 2739 | 0x0AB3 | `PIANO_USMASH` | 622 | Piano Up Smash (L628) | `src/Piano/Piano.asm` | 1 |
| 2740 | 0x0AB4 | `PIANO_DSMASH` | 623 | Piano Down Smash (L629) | `src/Piano/Piano.asm` | 1 |
| 2741 | 0x0AB5 | `PIANO_NAIR` | 624 | Piano Neutral Aerial (L630) | `src/Piano/Piano.asm` | 1 |
| 2742 | 0x0AB6 | `PIANO_FAIR` | 625 | Piano Forward Aerial (L631) | `src/Piano/Piano.asm` | 1 |
| 2743 | 0x0AB7 | `PIANO_BAIR` | 626 | Piano Back Aerial (L632) | `src/Piano/Piano.asm` | 1 |
| 2744 | 0x0AB8 | `PIANO_UAIR` | 627 | Piano Up Aerial (L633) | `src/Piano/Piano.asm` | 1 |
| 2745 | 0x0AB9 | `PIANO_DAIR` | 628 | Piano Down Aerial (L634) | `src/Piano/Piano.asm` | 1 |
| 2746 | 0x0ABA | `PIANO_USP` | 629 | Piano Up Special (L635) | `src/Piano/Piano.asm` | 1 |
| 2747 | 0x0ABB | `PIANO_FALL_SPECIAL` | 630 | Piano Fall Special (L636) | `src/Piano/Piano.asm` | 1 |
| 2748 | 0x0ABC | `PIANO_PLAT_DROP` | 631 | Piano Platform Drop (L637) | `src/Piano/Piano.asm` | 2 |
| 2749 | 0x0ABD | `PIANO_TEETER` | 632 | Piano Teeter (L638) | `src/Piano/Piano.asm` | 1 |
| 2750 | 0x0ABE | `PIANO_TEETER_START` | 633 | Piano Teeter Start (L639) | `src/Piano/Piano.asm` | 1 |
| 2751 | 0x0ABF | `PIANO_DMG_1` | 634 | Piano Damage 1 (L640) | `src/Piano/Piano.asm` | 4 |
| 2752 | 0x0AC0 | `PIANO_DMG_2` | 635 | Piano Damage 2 (L641) | `src/Piano/Piano.asm` | 4 |
| 2753 | 0x0AC1 | `PIANO_DMG_3` | 636 | Piano Damage 3 (L642) | `src/Piano/Piano.asm` | 5 |
| 2754 | 0x0AC2 | `PIANO_DMG_ELEC` | 637 | Piano Damage Electric (L643) | `src/Piano/Piano.asm` | 2 |
| 2755 | 0x0AC3 | `PIANO_DMG_FLY` | 638 | Piano Damage Fly (L644) | `src/Piano/Piano.asm` | 3 |
| 2756 | 0x0AC4 | `PIANO_DMG_FLY_TOP` | 639 | Piano Damage Fly Top (L645) | `src/Piano/Piano.asm` | 2 |
| 2757 | 0x0AC5 | `PIANO_DMG_FLY_ROLL` | 640 | Piano Damage Fly Roll (L646) | `src/Piano/Piano.asm` | 1 |
| 2758 | 0x0AC6 | `PIANO_TUMBLE` | 641 | Piano Tumble (L647) | `src/Piano/Piano.asm` | 10 |
| 2759 | 0x0AC7 | `PIANO_PIPE_ENTER` | 642 | Piano Pipe Enter (L648) | `src/Piano/Piano.asm` | 1 |
| 2760 | 0x0AC8 | `PIANO_PIPE_EXIT` | 643 | Piano Pipe Exit (L649) | `src/Piano/Piano.asm` | 1 |
| 2761 | 0x0AC9 | `PIANO_PIPE_EXIT_WALK` | 644 | Piano Pipe Exit Walk (L650) | `src/Piano/Piano.asm` | 1 |
| 2762 | 0x0ACA | `PIANO_CEILING_BONK` | 645 | Piano Ceiling Bonk (L651) | `src/Piano/Piano.asm` | 1 |
| 2763 | 0x0ACB | `PIANO_DOWN_BNCE_D` | 646 | Piano Down Bounce D (L652) | `src/Piano/Piano.asm` | 3 |
| 2764 | 0x0ACC | `PIANO_DOWN_STND_D` | 647 | Piano Down Stand D (L653) | `src/Piano/Piano.asm` | 3 |
| 2765 | 0x0ACD | `PIANO_DOWN_FWRD_D` | 648 | Piano Down Forward D (L654) | `src/Piano/Piano.asm` | 1 |
| 2766 | 0x0ACE | `PIANO_DOWN_BACK_D` | 649 | Piano Down Back D (L655) | `src/Piano/Piano.asm` | 1 |
| 2767 | 0x0ACF | `PIANO_DOWN_ATTK_D` | 650 | Piano Down Attack D (L656) | `src/Piano/Piano.asm` | 1 |
| 2768 | 0x0AD0 | `PIANO_DOWN_BNCE_U` | 651 | Piano Down Bounce U (L657) | `src/Piano/Piano.asm` | 2 |
| 2769 | 0x0AD1 | `PIANO_DOWN_STND_U` | 652 | Piano Down Stand U (L658) | `src/Piano/Piano.asm`, `src/TwelveCharBattle.asm` | 3 |
| 2770 | 0x0AD2 | `PIANO_DOWN_FWRD_U` | 653 | Piano Down Forward U (L659) | `src/Piano/Piano.asm` | 1 |
| 2771 | 0x0AD3 | `PIANO_DOWN_BACK_U` | 654 | Piano Down Back U (L660) | `src/Piano/Piano.asm` | 1 |
| 2772 | 0x0AD4 | `PIANO_DOWN_ATTK_U` | 655 | Piano Down Attack U (L661) | `src/Piano/Piano.asm` | 1 |
| 2773 | 0x0AD5 | `PIANO_TECH_F` | 656 | Piano Tech Forward (L662) | `src/Piano/Piano.asm` | 1 |
| 2774 | 0x0AD6 | `PIANO_TECH_B` | 657 | Piano Tech Back (L663) | `src/Piano/Piano.asm` | 1 |
| 2775 | 0x0AD7 | `PIANO_TECH` | 658 | Piano Tech (L664) | `src/Piano/Piano.asm` | 1 |
| 2776 | 0x0AD8 | `PIANO_UNKNOWN_053` | 659 | Piano Unknown 053 (L665) | `src/Piano/Piano.asm` | 1 |
| 2777 | 0x0AD9 | `PIANO_CLF_CATCH` | 660 | Piano Cliff Catch (L666) | `src/Piano/Piano.asm` | 1 |
| 2778 | 0x0ADA | `PIANO_CLF_WAIT` | 661 | Piano Cliff Wait (L667) | `src/Piano/Piano.asm` | 1 |
| 2779 | 0x0ADB | `PIANO_CLF_QUICK` | 662 | Piano Cliff Quick (L668) | `src/Piano/Piano.asm` | 1 |
| 2780 | 0x0ADC | `PIANO_CLF_CLM_Q_1` | 663 | Piano Cliff Climb Quick 1 (L669) | `src/Piano/Piano.asm` | 3 |
| 2781 | 0x0ADD | `PIANO_CLF_CLM_Q_2` | 664 | Piano Cliff Climb Quick 2 (L670) | `src/Piano/Piano.asm` | 1 |
| 2782 | 0x0ADE | `PIANO_CLF_ATK_Q_2` | 665 | Piano Cliff Attack Quick 2 (L671) | `src/Piano/Piano.asm` | 1 |
| 2783 | 0x0ADF | `PIANO_CLF_ESC_Q_2` | 666 | Piano Cliff Escape Quick 2 (L672) | `src/Piano/Piano.asm` | 1 |
| 2784 | 0x0AE0 | `PIANO_CLF_SLOW` | 667 | Piano Cliff Slow (L673) | `src/Piano/Piano.asm` | 1 |
| 2785 | 0x0AE1 | `PIANO_CLF_CLM_S_1` | 668 | Piano Cliff Climb Slow 1 (L674) | `src/Piano/Piano.asm` | 1 |
| 2786 | 0x0AE2 | `PIANO_CLF_CLM_S_2` | 669 | Piano Cliff Climb Slow 2 (L675) | `src/Piano/Piano.asm` | 1 |
| 2787 | 0x0AE3 | `PIANO_CLF_ATK_S_1` | 670 | Piano Cliff Attack Slow 1 (L676) | `src/Piano/Piano.asm` | 1 |
| 2788 | 0x0AE4 | `PIANO_CLF_ATK_S_2` | 671 | Piano Cliff Attack Slow 2 (L677) | `src/Piano/Piano.asm` | 1 |
| 2789 | 0x0AE5 | `PIANO_CLF_ESC_S_1` | 672 | Piano Cliff Escape Slow 1 (L678) | `src/Piano/Piano.asm` | 1 |
| 2790 | 0x0AE6 | `PIANO_CLF_ESC_S_2` | 673 | Piano Cliff Escape Slow 2 (L679) | `src/Piano/Piano.asm` | 1 |
| 2791 | 0x0AE7 | `PIANO_L_ITM_PICKUP` | 674 | Piano Light Item Pickup (L680) | `src/Piano/Piano.asm` | 1 |
| 2792 | 0x0AE8 | `PIANO_ITM_DROP` | 675 | Piano Item Drop (L681) | `src/Piano/Piano.asm` | 1 |
| 2793 | 0x0AE9 | `PIANO_ITM_THROW_F` | 676 | Piano Item Throw F (L682) | `src/Piano/Piano.asm` | 8 |
| 2794 | 0x0AEA | `PIANO_ITM_THROW_U` | 677 | Piano Item Throw U (L683) | `src/Piano/Piano.asm` | 4 |
| 2795 | 0x0AEB | `PIANO_ITM_THROW_D` | 678 | Piano Item Throw D (L684) | `src/Piano/Piano.asm` | 4 |
| 2796 | 0x0AEC | `PIANO_ITM_THROW_A_U` | 679 | Piano Item Throw Air U (L685) |  | 0 |
| 2797 | 0x0AED | `PIANO_ITM_THROW_A_D` | 680 | Piano Item Throw Air D (L686) |  | 0 |
| 2798 | 0x0AEE | `PIANO_HEAVY_ITM_PICKUP` | 681 | Piano Heavy Item Pickup (L687) | `src/Piano/Piano.asm` | 1 |
| 2799 | 0x0AEF | `PIANO_HEAVY_ITM_THROW` | 682 | Piano Heavy Item Throw (L688) | `src/Piano/Piano.asm` | 4 |
| 2800 | 0x0AF0 | `PIANO_ITM_NEUTRAL` | 683 | Piano Item Neutral (L689) | `src/Piano/Piano.asm` | 4 |
| 2801 | 0x0AF1 | `PIANO_ITM_TILT` | 684 | Piano Item Tilt (L690) | `src/Piano/Piano.asm` | 4 |
| 2802 | 0x0AF2 | `PIANO_ITM_SMASH` | 685 | Piano Item Smash (L691) | `src/Piano/Piano.asm` | 4 |
| 2803 | 0x0AF3 | `PIANO_ITM_DASH` | 686 | Piano Item Dash (L692) | `src/Piano/Piano.asm` | 4 |
| 2804 | 0x0AF4 | `PIANO_RAY_GUN` | 687 | Piano Ray Gun Shoot (L693) | `src/Piano/Piano.asm` | 4 |
| 2805 | 0x0AF5 | `PIANO_HAMMER` | 688 | Piano Hammer (L694) | `src/Piano/Piano.asm` | 6 |
| 2806 | 0x0AF6 | `PIANO_SHIELD_ON` | 689 | Piano Shield On (L695) | `src/Piano/Piano.asm` | 1 |
| 2807 | 0x0AF7 | `PIANO_SHIELD_OFF` | 690 | Piano Shield Off (L696) | `src/Piano/Piano.asm` | 1 |
| 2808 | 0x0AF8 | `PIANO_ROLL_F` | 691 | Piano Roll F (L697) | `src/Piano/Piano.asm` | 1 |
| 2809 | 0x0AF9 | `PIANO_ROLL_B` | 692 | Piano Roll B (L698) | `src/Piano/Piano.asm` | 1 |
| 2810 | 0x0AFA | `PIANO_STUN` | 693 | Piano Stun (L699) | `src/Piano/Piano.asm` | 3 |
| 2811 | 0x0AFB | `PIANO_GRAB` | 694 | Piano Grab (L700) | `src/Piano/Piano.asm` | 1 |
| 2812 | 0x0AFC | `PIANO_GRAB_PULL` | 695 | Piano Grab Pull (L701) | `src/Piano/Piano.asm` | 1 |
| 2813 | 0x0AFD | `PIANO_THROW_F` | 696 | Piano Throw F (L702) | `src/Piano/Piano.asm` | 1 |
| 2814 | 0x0AFE | `PIANO_THROW_B` | 697 | Piano Throw B (L703) | `src/Piano/Piano.asm` | 1 |
| 2815 | 0x0AFF | `PIANO_CAPTURE_PULLED` | 698 | Piano Capture Pulled (L704) | `src/Piano/Piano.asm` | 2 |
| 2816 | 0x0B00 | `PIANO_THROWN_DK_PULLED` | 699 | Piano Thrown DK Pulled (L705) | `src/Piano/Piano.asm` | 1 |
| 2817 | 0x0B01 | `PIANO_THROWN_DK` | 700 | Piano Thrown DK (L706) | `src/Piano/Piano.asm` | 1 |
| 2818 | 0x0B02 | `PIANO_THROWN_BROS` | 701 | Piano Thrown Mario Bros (L707) | `src/Piano/Piano.asm` | 1 |
| 2819 | 0x0B03 | `PIANO_THROWN_1` | 702 | Piano Thrown 1 (L708) | `src/Piano/Piano.asm` | 1 |
| 2820 | 0x0B04 | `PIANO_THROWN_2` | 703 | Piano Thrown 2 (L709) | `src/Piano/Piano.asm` | 1 |
| 2821 | 0x0B05 | `PIANO_ITM_THROW_DASH` | 704 | Piano Item Throw Dash (L710) | `src/Piano/Piano.asm` | 1 |
| 2822 | 0x0B06 | `PIANO_TAUNT` | 705 | Piano Taunt (L711) | `src/Piano/Piano.asm` | 1 |
| 2823 | 0x0B07 | `PIANO_DSP_BEGIN_G` | 706 | Piano DSP Begin Ground (L712) | `src/Piano/Piano.asm` | 1 |
| 2824 | 0x0B08 | `PIANO_DSP_BEGIN_A` | 707 | Piano DSP Begin Air (L713) | `src/Piano/Piano.asm` | 1 |
| 2825 | 0x0B09 | `PIANO_DSP_WAIT` | 708 | Piano DSP Wait (L714) | `src/Piano/Piano.asm` | 2 |
| 2826 | 0x0B0A | `PIANO_DSP_ABSORB` | 709 | Piano DSP Absorb (L715) | `src/Piano/Piano.asm` | 2 |
| 2827 | 0x0B0B | `PIANO_DSP_END_G` | 710 | Piano DSP End Ground (L716) | `src/Piano/Piano.asm` | 1 |
| 2828 | 0x0B0C | `PIANO_DSP_END_A` | 711 | Piano DSP End Air (L717) | `src/Piano/Piano.asm` | 1 |
| 2829 | 0x0B0D | `PIANO_DSP_THROW_G` | 712 | Piano DSP Throw Ground (L718) | `src/Piano/Piano.asm` | 1 |
| 2830 | 0x0B0E | `PIANO_DSP_THROW_A` | 713 | Piano DSP Throw Air (L719) | `src/Piano/Piano.asm` | 1 |
| 2831 | 0x0B0F | `PIANO_NSP_G` | 714 | Piano NSP Ground (L720) | `src/Piano/Piano.asm` | 1 |
| 2832 | 0x0B10 | `PIANO_NSP_A` | 715 | Piano NSP Air (L721) | `src/Piano/Piano.asm` | 1 |
| 2833 | 0x0B11 | `PIANO_ENTRY_R` | 716 | Piano Entry R (L722) | `src/Piano/Piano.asm` | 1 |
| 2834 | 0x0B12 | `PIANO_ENTRY_L` | 717 | Piano Entry L (L723) | `src/Piano/Piano.asm` | 1 |
| 2835 | 0x0B13 | `PIANO_VICTORY_1` | 718 | Piano Victory 1 (L724) | `src/Piano/Piano.asm` | 1 |
| 2836 | 0x0B14 | `PIANO_VICTORY_2` | 719 | Piano Victory 2 (L725) | `src/Piano/Piano.asm` | 1 |
| 2837 | 0x0B15 | `PIANO_SELECT` | 720 | Piano Select Pose (L726) | `src/Piano/Piano.asm` | 1 |
| 2838 | 0x0B16 | `KIRBY_PIANO_NSP_G` | 721 | Kirby Piano Neutral Special Ground (L727) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2839 | 0x0B17 | `KIRBY_PIANO_NSP_A` | 722 | Kirby Piano Neutral Special Air (L728) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 2840 | 0x0B18 | `PIANO_POSE_1P` | 723 | Piano Pose 1P (L729) | `src/Piano/Piano.asm` | 1 |
| 2841 | 0x0B19 | `PIANO_CONTINUE_FALL` | 724 | Piano Continue Fall (L730) | `src/Piano/Piano.asm` | 1 |
| 2842 | 0x0B1A | `PIANO_CONTINUE_UP` | 725 | Piano Continue Up (L731) | `src/Piano/Piano.asm` | 1 |
| 2843 | 0x0B1B | `WOLF_RUN` | 726 | Wolf Run (L732) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2844 | 0x0B1C | `WOLF_RUN_BRAKE` | 727 | Wolf Run Brake (L733) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2845 | 0x0B1D | `WOLF_TURN_RUN` | 728 | Wolf Turn Run (L734) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2846 | 0x0B1E | `WOLF_CROUCH` | 729 | Wolf Crouch (L735) | `src/NWolf/NWolf.asm`, `src/Peppy/Peppy.asm`, `src/Wolf/Wolf.asm` | 3 |
| 2847 | 0x0B1F | `WOLF_CROUCH_IDLE` | 730 | Wolf Crouch Idle (L736) | `src/NWolf/NWolf.asm`, `src/Peppy/Peppy.asm`, `src/Wolf/Wolf.asm` | 3 |
| 2848 | 0x0B20 | `WOLF_CROUCH_END` | 731 | Wolf Crouch End (L737) | `src/NWolf/NWolf.asm`, `src/Peppy/Peppy.asm`, `src/Wolf/Wolf.asm` | 3 |
| 2849 | 0x0B21 | `WOLF_JUMP_F` | 732 | Wolf Jump Forward (L738) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2850 | 0x0B22 | `WOLF_JUMP_B` | 733 | Wolf Jump Back (L739) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2851 | 0x0B23 | `WOLF_JUMP_AF` | 734 | Wolf Jump Aerial Forward (L740) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2852 | 0x0B24 | `WOLF_JUMP_AB` | 735 | Wolf Jump Aerial Backward (L741) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2853 | 0x0B25 | `WOLF_JUMP_SQUAT` | 736 | Wolf Jump Squat (L742) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 12 |
| 2854 | 0x0B26 | `WOLF_FALLING` | 737 | Wolf Falling (L743) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2855 | 0x0B27 | `WOLF_FALLING_AERIAL` | 738 | Wolf Falling Aerial (L744) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2856 | 0x0B28 | `WOLF_JAB1` | 739 | Wolf Jab 1 (L745) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2857 | 0x0B29 | `WOLF_JAB2` | 740 | Wolf Jab 2 (L746) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2858 | 0x0B2A | `WOLF_JAB3` | 741 | Wolf Jab 3 (L747) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2859 | 0x0B2B | `WOLF_DASH_ATTACK` | 742 | Wolf Dash Attack (L748) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2860 | 0x0B2C | `WOLF_UTILT` | 743 | Wolf Up Tilt (L749) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2861 | 0x0B2D | `WOLF_FTILT_HIGH` | 744 | Wolf Forward Tilt High (L750) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2862 | 0x0B2E | `WOLF_FTILT_MID` | 745 | Wolf Forward Tilt Mid (L751) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2863 | 0x0B2F | `WOLF_FTILT_LOW` | 746 | Wolf Forward Tilt Low (L752) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2864 | 0x0B30 | `WOLF_DTILT` | 747 | Wolf Down Tilt (L753) | `src/NWolf/NWolf.asm`, `src/Peppy/Peppy.asm`, `src/Wolf/Wolf.asm` | 3 |
| 2865 | 0x0B31 | `WOLF_USMASH` | 748 | Wolf Up Smash (L754) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2866 | 0x0B32 | `WOLF_FSMASH` | 749 | Wolf Forward Smash (L755) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2867 | 0x0B33 | `WOLF_DSMASH` | 750 | Wolf Down Smash (L756) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2868 | 0x0B34 | `WOLF_NAIR` | 751 | Wolf Neutral Air (L757) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2869 | 0x0B35 | `WOLF_FAIR` | 752 | Wolf Forward Air (L758) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2870 | 0x0B36 | `WOLF_UAIR` | 753 | Wolf Up Air (L759) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2871 | 0x0B37 | `WOLF_BAIR` | 754 | Wolf Back Air (L760) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2872 | 0x0B38 | `WOLF_DAIR` | 755 | Wolf Down Air (L761) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2873 | 0x0B39 | `YL_VICTORY` | 756 | Young Link Victory (L762) | `src/NYoungLink/NYoungLink.asm`, `src/YoungLink/YoungLink.asm` | 2 |
| 2874 | 0x0B3A | `WOLF_CSS` | 757 | Wolf CSS (L763) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2875 | 0x0B3B | `WOLF_VICTORY1` | 758 | Wolf Victory 1 (L764) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2876 | 0x0B3C | `WOLF_GRAB` | 759 | Wolf Grab (L765) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2877 | 0x0B3D | `WOLF_THROWF` | 760 | Wolf Forward Throw (L766) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2878 | 0x0B3E | `WOLF_DOWNATTACKD` | 761 | Wolf Down Attack Down (L767) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2879 | 0x0B3F | `WOLF_DOWNATTACKU` | 762 | Wolf Down Attack Up (L768) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2880 | 0x0B40 | `WOLF_DOWNFORWARDD` | 763 | Wolf Down Forward Down (L769) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2881 | 0x0B41 | `WOLF_FALLSPECIAL` | 764 | Wolf Fall Special (L770) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2882 | 0x0B42 | `WOLF_NSP` | 765 | Wolf Neutral Special Ground (L771) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2883 | 0x0B43 | `WOLF_NSP_AIR` | 766 | Wolf Neutral Special Air (L772) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2884 | 0x0B44 | `WOLF_USP_1` | 767 | Wolf Up Special Ground 1 (L773) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2885 | 0x0B45 | `WOLF_APPEAR_L` | 768 | Wolf Appear Left (L774) | `src/Wolf/Wolf.asm` | 1 |
| 2886 | 0x0B46 | `WOLF_VICTORY2` | 769 | Wolf Victory 2 (L775) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2887 | 0x0B47 | `WOLF_WEAPON` | 770 | Wolf Weapon (L776) |  | 0 |
| 2888 | 0x0B48 | `WOLF_BAIR_LANDING` | 771 | Wolf Back Air Landing (L777) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2889 | 0x0B49 | `WOLF_FAIR_LANDING` | 772 | Wolf Forward Air Landing (L778) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2890 | 0x0B4A | `WOLF_DOWNSTANDD` | 773 | Wolf DownStandD (L779) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 6 |
| 2891 | 0x0B4B | `WOLF_DOWNSTANDU` | 774 | Wolf DownStandU (L780) | `src/NWolf/NWolf.asm`, `src/TwelveCharBattle.asm`, `src/Wolf/Wolf.asm` | 5 |
| 2892 | 0x0B4C | `WOLF_DOWNFORWARDU` | 775 | Wolf DownForwardU (L781) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2893 | 0x0B4D | `WOLF_TECH` | 776 | Wolf Tech (L782) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2894 | 0x0B4E | `WOLF_TECHF` | 777 | Wolf TechF (L783) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2895 | 0x0B4F | `WOLF_TECHB` | 778 | Wolf TechB (L784) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2896 | 0x0B50 | `WOLF_DSP_END_GRND` | 779 | Wolf Down Special End Ground (L785) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2897 | 0x0B51 | `WOLF_DSP_END_AIR` | 780 | Wolf Down Special End Air (L786) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2898 | 0x0B52 | `WOLF_DOWNBACKD` | 781 | Wolf DownBackD (L787) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2899 | 0x0B53 | `WOLF_DOWNBACKU` | 782 | Wolf DownBackU (L788) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2900 | 0x0B54 | `WOLF_DOWNBOUNCED` | 783 | Wolf DownBounceD (L789) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 6 |
| 2901 | 0x0B55 | `WOLF_DOWNBOUNCEU` | 784 | Wolf DownBounceU (L790) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2902 | 0x0B56 | `WOLF_SHIELDON` | 785 | Wolf Shield On (L791) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2903 | 0x0B57 | `WOLF_SHIELDOFF` | 786 | Wolf Shield Off (L792) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2904 | 0x0B58 | `WOLF_MAIN` | 787 | Wolf Main (L793) | `src/Character.asm` | 1 |
| 2905 | 0x0B59 | `WOLF_CHARACTER` | 788 | Wolf Character (L794) | `src/Character.asm` | 1 |
| 2906 | 0x0B5A | `WOLF_PROJECTILE_HITBOX` | 789 | Wolf Projectile Hitbox (L795) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 2907 | 0x0B5B | `WOLF_PROJECTILE_GRAPHIC` | 790 | Wolf Projectile Graphic (L796) | `src/Character.asm` | 2 |
| 2908 | 0x0B5C | `WOLF_IDLE` | 791 | Wolf Idle (L797) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 12 |
| 2909 | 0x0B5D | `WOLF_TURN` | 792 | Wolf Turn (L798) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2910 | 0x0B5E | `WOLF_WALK1` | 793 | Wolf Walk 1 (L799) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2911 | 0x0B5F | `WOLF_WALK2` | 794 | Wolf Walk 2 (L800) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2912 | 0x0B60 | `WOLF_WALK3` | 795 | Wolf Walk 3 (L801) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2913 | 0x0B61 | `WOLF_DASH` | 796 | Wolf Dash (L802) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2914 | 0x0B62 | `WOLF_SWING_NEUTRAL` | 797 | Wolf Item Swing Neutral (L803) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 8 |
| 2915 | 0x0B63 | `WOLF_SWING_TILT` | 798 | Wolf Item Swing Tilt (L804) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 8 |
| 2916 | 0x0B64 | `WOLF_SWING_SMASH` | 799 | Wolf Item Swing Smash (L805) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 8 |
| 2917 | 0x0B65 | `WOLF_PULL` | 800 | Wolf Capture Pull (L806) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2918 | 0x0B66 | `WOLF_ITEM_PICKUP` | 801 | Wolf Item Pick Up (L807) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2919 | 0x0B67 | `WOLF_ITEM_DROP` | 802 | Wolf Item Drop (L808) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2920 | 0x0B68 | `WOLF_ITEM_THROWF` | 803 | Wolf Item Throw Forward (L809) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 8 |
| 2921 | 0x0B69 | `WOLF_ITEM_THROWU` | 804 | Wolf Item Throw Upward (L810) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2922 | 0x0B6A | `WOLF_ITEM_THROWD` | 805 | Wolf Item Throw Downward (L811) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2923 | 0x0B6B | `WOLF_ITEM_THROWF_AIR` | 806 | Wolf Item Throw Air Forward (L812) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 8 |
| 2924 | 0x0B6C | `WOLF_ITEM_THROWU_AIR` | 807 | Wolf Item Throw Air Upward (L813) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2925 | 0x0B6D | `WOLF_ITEM_THROWD_AIR` | 808 | Wolf Item Throw Air Downward (L814) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2926 | 0x0B6E | `WOLF_SHIELD_DROP` | 809 | Wolf Shield Drop (L815) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2927 | 0x0B6F | `WOLF_ITEM_THROW_DASH` | 810 | Wolf Item Throw Dash (L816) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2928 | 0x0B70 | `WOLF_SWING_DASH` | 811 | Wolf Item Swing Dash (L817) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 8 |
| 2929 | 0x0B71 | `WOLF_RAYGUN_GND` | 812 | Wolf Item Shoot Ground (L818) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2930 | 0x0B72 | `WOLF_RAYGUN_AIR` | 813 | Wolf Item Shoot Air (L819) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2931 | 0x0B73 | `WOLF_USP_2` | 814 | Wolf Up Special Ground 2 (L820) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 4 |
| 2932 | 0x0B74 | `SSONIC_ENTRY_LEFT` | 815 | Super Sonic Entry Left (L821) | `src/SSonic/SSonic.asm` | 1 |
| 2933 | 0x0B75 | `WOLF_TAUNT` | 816 | Wolf Taunt (L822) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 2934 | 0x0B76 | `WOLF_REFLECTOR` | 817 | Wolf Reflector (L823) | `src/Character.asm` | 2 |
| 2935 | 0x0B77 | `WOLFEN` | 818 | Wolfen (L824) | `src/Character.asm` | 2 |
| 2936 | 0x0B78 | `STG_RITH_ESSA_HEADER` | 819 | Rith Essa Header (L825) |  | 0 |
| 2937 | 0x0B79 | `STG_RITH_ESSA` | 820 | Rith Essa Stage (L826) |  | 0 |
| 2938 | 0x0B7A | `STG_RITH_ESSA_BG` | 821 | Rith Essa Background (L827) |  | 0 |
| 2939 | 0x0B7B | `MTWO_MAIN` | 822 | Mewtwo Main (L828) | `src/Character.asm` | 1 |
| 2940 | 0x0B7C | `MTWO_CHARACTER` | 823 | Mewtwo Character (L829) | `src/Character.asm` | 1 |
| 2941 | 0x0B7D | `MTWO_SHIELD_POSE` | 824 | Mewtwo Shield Pose (L830) | `src/Character.asm` | 2 |
| 2942 | 0x0B7E | `MTWO_IDLE` | 825 | Mewtwo Idle (L831) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 14 |
| 2943 | 0x0B7F | `MTWO_WALK_1` | 826 | Mewtwo Walk1 (L832) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2944 | 0x0B80 | `MTWO_WALK_2` | 827 | Mewtwo Walk2 (L833) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2945 | 0x0B81 | `MTWO_WALK_3` | 828 | Mewtwo Walk3 (L834) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2946 | 0x0B82 | `MTWO_WALK_END` | 829 | Mewtwo Walk End (L835) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2947 | 0x0B83 | `MTWO_DASH` | 830 | Mewtwo Dash (L836) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2948 | 0x0B84 | `MTWO_RUN` | 831 | Mewtwo Run (L837) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2949 | 0x0B85 | `MTWO_RUN_BRAKE` | 832 | Mewtwo RunBrake (L838) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2950 | 0x0B86 | `MTWO_TURN` | 833 | Mewtwo Turn (L839) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2951 | 0x0B87 | `MTWO_TURN_RUN` | 834 | Mewtwo TurnRun (L840) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2952 | 0x0B88 | `MTWO_JUMP_F` | 835 | Mewtwo JumpF (L841) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2953 | 0x0B89 | `MTWO_JUMP_B` | 836 | Mewtwo JumpB (L842) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2954 | 0x0B8A | `MTWO_JUMP_AERIAL_F` | 837 | Mewtwo JumpAerialF (L843) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2955 | 0x0B8B | `MTWO_JUMP_AERIAL_B` | 838 | Mewtwo JumpAerialB (L844) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2956 | 0x0B8C | `MTWO_FALL` | 839 | Mewtwo Fall (L845) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2957 | 0x0B8D | `MTWO_FALL_AERIAL` | 840 | Mewtwo FallAerial (L846) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2958 | 0x0B8E | `MTWO_CROUCH_BEGIN` | 841 | Mewtwo Crouch (L847) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2959 | 0x0B8F | `MTWO_CROUCH_IDLE` | 842 | Mewtwo CrouchIdle (L848) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2960 | 0x0B90 | `MTWO_CROUCH_END` | 843 | Mewtwo CrouchEnd (L849) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2961 | 0x0B91 | `MTWO_LANDING` | 844 | Mewtwo Landing (L850) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 18 |
| 2962 | 0x0B92 | `MTWO_PLAT_DROP` | 845 | Mewtwo PlatDrop (L851) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 2963 | 0x0B93 | `MTWO_TEETER` | 846 | Mewtwo Teeter (L852) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2964 | 0x0B94 | `MTWO_TEETER_START` | 847 | Mewtwo Teeterstart (L853) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2965 | 0x0B95 | `MTWO_DMG_HIGH_1` | 848 | Mewtwo DamageHigh1 (L854) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2966 | 0x0B96 | `MTWO_DMG_HIGH_2` | 849 | Mewtwo DamageHigh2 (L855) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2967 | 0x0B97 | `MTWO_DMG_HIGH_3` | 850 | Mewtwo DamageHigh3 (L856) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 2968 | 0x0B98 | `MTWO_DMG_MID_1` | 851 | Mewtwo DamageMid1 (L857) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2969 | 0x0B99 | `MTWO_DMG_MID_2` | 852 | Mewtwo DamageMid2 (L858) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2970 | 0x0B9A | `MTWO_DMG_MID_3` | 853 | Mewtwo DamageMid3 (L859) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2971 | 0x0B9B | `MTWO_DMG_LOW_1` | 854 | Mewtwo DamageLow1 (L860) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2972 | 0x0B9C | `MTWO_DMG_LOW_2` | 855 | Mewtwo DamageLow2 (L861) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2973 | 0x0B9D | `MTWO_DMG_LOW_3` | 856 | Mewtwo DamageLow3 (L862) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2974 | 0x0B9E | `MTWO_DMG_AIR_1` | 857 | Mewtwo DamageAir1 (L863) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2975 | 0x0B9F | `MTWO_DMG_AIR_2` | 858 | Mewtwo DamageAir2 (L864) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2976 | 0x0BA0 | `MTWO_DMG_AIR_3` | 859 | Mewtwo DamageAir3 (L865) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2977 | 0x0BA1 | `MTWO_DMG_ELEC` | 860 | Mewtwo DamageElec (L866) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 2978 | 0x0BA2 | `MTWO_DMG_FLY_HIGH` | 861 | Mewtwo DamageFlyHigh (L867) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2979 | 0x0BA3 | `MTWO_DMG_FLY_MID` | 862 | Mewtwo DamageFlyMid (L868) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2980 | 0x0BA4 | `MTWO_DMG_FLY_LOW` | 863 | Mewtwo DamageFlyLow (L869) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2981 | 0x0BA5 | `MTWO_DMG_FLY_TOP` | 864 | Mewtwo DamageFlyTop (L870) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 2982 | 0x0BA6 | `MTWO_DMG_FLY_ROLL` | 865 | Mewtwo DamageFlyRoll (L871) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2983 | 0x0BA7 | `MTWO_TUMBLE` | 866 | Mewtwo Tumble (L872) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 20 |
| 2984 | 0x0BA8 | `MTWO_FALL_SPECIAL` | 867 | Mewtwo FallSpecial (L873) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2985 | 0x0BA9 | `MTWO_ENTER_PIPE` | 868 | Mewtwo EnterPipe (L874) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2986 | 0x0BAA | `MTWO_EXIT_PIPE` | 869 | Mewtwo ExitPipe (L875) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2987 | 0x0BAB | `MTWO_EXIT_PIPE_WALK` | 870 | Mewtwo ExitPipeWalk (L876) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2988 | 0x0BAC | `MTWO_CEILING_BONK` | 871 | Mewtwo CeilingBonk (L877) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2989 | 0x0BAD | `MTWO_DOWN_BNCE_D` | 872 | Mewtwo DownBounceD (L878) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 6 |
| 2990 | 0x0BAE | `MTWO_DOWN_BNCE_U` | 873 | Mewtwo DownBounceU (L879) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 2991 | 0x0BAF | `MTWO_DOWN_STND_D` | 874 | Mewtwo DownStandD (L880) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 6 |
| 2992 | 0x0BB0 | `MTWO_DOWN_STND_U` | 875 | Mewtwo DownStandU (L881) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm`, `src/TwelveCharBattle.asm` | 6 |
| 2993 | 0x0BB1 | `MTWO_TECH_F` | 876 | Mewtwo TechF (L882) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2994 | 0x0BB2 | `MTWO_TECH_B` | 877 | Mewtwo TechB (L883) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2995 | 0x0BB3 | `MTWO_DOWN_FWRD_D` | 878 | Mewtwo DownForwardD (L884) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2996 | 0x0BB4 | `MTWO_DOWN_FWRD_U` | 879 | Mewtwo DownForwardU (L885) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2997 | 0x0BB5 | `MTWO_DOWN_BACK_D` | 880 | Mewtwo DownBackD (L886) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2998 | 0x0BB6 | `MTWO_DOWN_BACK_U` | 881 | Mewtwo DownBackU (L887) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 2999 | 0x0BB7 | `MTWO_DOWN_ATTK_D` | 882 | Mewtwo DownAttackD (L888) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3000 | 0x0BB8 | `MTWO_DOWN_ATTK_U` | 883 | Mewtwo DownAttackU (L889) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3001 | 0x0BB9 | `MTWO_TECH` | 884 | Mewtwo Tech (L890) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3002 | 0x0BBA | `MTWO_UNKNOWN_053` | 885 | Mewtwo Unknown_053 (L891) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3003 | 0x0BBB | `MTWO_CLF_CATCH` | 886 | Mewtwo CliffCatch (L892) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3004 | 0x0BBC | `MTWO_CLF_WAIT` | 887 | Mewtwo CliffWait (L893) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3005 | 0x0BBD | `MTWO_CLF_QUICK` | 888 | Mewtwo CliffQuick (L894) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3006 | 0x0BBE | `MTWO_CLF_CLM_Q_1` | 889 | Mewtwo CliffClimbQuick1 (L895) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3007 | 0x0BBF | `MTWO_CLF_CLM_Q_2` | 890 | Mewtwo CliffClimbQuick2 (L896) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3008 | 0x0BC0 | `MTWO_CLF_SLOW` | 891 | Mewtwo CliffSlow (L897) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3009 | 0x0BC1 | `MTWO_CLF_CLM_S_1` | 892 | Mewtwo CliffClimbSlow1 (L898) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3010 | 0x0BC2 | `MTWO_CLF_CLM_S_2` | 893 | Mewtwo CliffClimbSlow2 (L899) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3011 | 0x0BC3 | `MTWO_CLF_ATK_Q_1` | 894 | Mewtwo CliffAttackQuick1 (L900) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3012 | 0x0BC4 | `MTWO_CLF_ATK_Q_2` | 895 | Mewtwo CliffAttackQuick2 (L901) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3013 | 0x0BC5 | `MTWO_CLF_ATK_S_1` | 896 | Mewtwo CliffAttackSlow1 (L902) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3014 | 0x0BC6 | `MTWO_CLF_ATK_S_2` | 897 | Mewtwo CliffAttackSlow2 (L903) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3015 | 0x0BC7 | `MTWO_CLF_ESC_Q_1` | 898 | Mewtwo CliffEscapeQuick1 (L904) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3016 | 0x0BC8 | `MTWO_CLF_ESC_Q_2` | 899 | Mewtwo CliffEscapeQuick2 (L905) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3017 | 0x0BC9 | `MTWO_CLF_ESC_S_1` | 900 | Mewtwo CliffEscapeSlow1 (L906) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3018 | 0x0BCA | `MTWO_CLF_ESC_S_2` | 901 | Mewtwo CliffEscapeSlow2 (L907) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3019 | 0x0BCB | `MTWO_L_ITM_PICKUP` | 902 | Mewtwo LightItemPickup (L908) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3020 | 0x0BCC | `MTWO_H_ITM_PICKUP` | 903 | Mewtwo HeavyItemPickup (L909) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3021 | 0x0BCD | `MTWO_ITM_DROP` | 904 | Mewtwo ItemDrop (L910) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3022 | 0x0BCE | `MTWO_ITM_THROW_DASH` | 905 | Mewtwo ItemThrowDash (L911) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3023 | 0x0BCF | `MTWO_ITM_THROW_F` | 906 | Mewtwo ItemThrowF (L912) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 8 |
| 3024 | 0x0BD0 | `MTWO_ITM_THROW_U` | 907 | Mewtwo ItemThrowU (L913) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3025 | 0x0BD1 | `MTWO_ITM_THROW_D` | 908 | Mewtwo ItemThrowD (L914) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3026 | 0x0BD2 | `MTWO_ITM_THROW_AIR_F` | 909 | Mewtwo ItemThrowAirF (L915) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 8 |
| 3027 | 0x0BD3 | `MTWO_ITM_THROW_AIR_U` | 910 | Mewtwo ItemThrowAirU (L916) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3028 | 0x0BD4 | `MTWO_ITM_THROW_AIR_D` | 911 | Mewtwo ItemThrowAirD (L917) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3029 | 0x0BD5 | `MTWO_HEAVY_ITM_THROW` | 912 | Mewtwo HeavyItemThrow (L918) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 8 |
| 3030 | 0x0BD6 | `MTWO_ITM_NEUTRAL` | 913 | Mewtwo ItemNeutral (L919) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 8 |
| 3031 | 0x0BD7 | `MTWO_ITM_TILT` | 914 | Mewtwo ItemTilt (L920) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 8 |
| 3032 | 0x0BD8 | `MTWO_ITM_SMASH` | 915 | Mewtwo ItemSmash (L921) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 8 |
| 3033 | 0x0BD9 | `MTWO_ITM_DASH` | 916 | Mewtwo ItemDash (L922) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 8 |
| 3034 | 0x0BDA | `MTWO_ITM_SHOOT` | 917 | Mewtwo ItemShoot (L923) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3035 | 0x0BDB | `MTWO_ITM_SHOOT_AIR` | 918 | Mewtwo ItemShootAir (L924) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3036 | 0x0BDC | `MTWO_HAMMER_IDLE` | 919 | Mewtwo HammerIdle (L925) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3037 | 0x0BDD | `MTWO_HAMMER_MOVE` | 920 | Mewtwo HammerMove (L926) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 10 |
| 3038 | 0x0BDE | `MTWO_SHIELD_ON` | 921 | Mewtwo ShieldOn (L927) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3039 | 0x0BDF | `MTWO_SHIELD_OFF` | 922 | Mewtwo ShieldOff (L928) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3040 | 0x0BE0 | `MTWO_ROLL_F` | 923 | Mewtwo RollF (L929) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3041 | 0x0BE1 | `MTWO_ROLL_B` | 924 | Mewtwo RollB (L930) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3042 | 0x0BE2 | `MTWO_STUN` | 925 | Mewtwo Stun (L931) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3043 | 0x0BE3 | `MTWO_CATCH` | 926 | Mewtwo Catch (L932) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3044 | 0x0BE4 | `MTWO_CATCH_PULL` | 927 | Mewtwo CatchPull (L933) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3045 | 0x0BE5 | `MTWO_THROW_F` | 928 | Mewtwo ThrowF (L934) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3046 | 0x0BE6 | `MTWO_THROW_B` | 929 | Mewtwo ThrowB (L935) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3047 | 0x0BE7 | `MTWO_CAPTURE_PULLED` | 930 | Mewtwo CapturePulled (L936) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3048 | 0x0BE8 | `MTWO_THROWN_DK_PULLED` | 931 | Mewtwo ThrownDKPulled (L937) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3049 | 0x0BE9 | `MTWO_THROWN_BROS` | 932 | Mewtwo ThrownMarioBros (L938) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3050 | 0x0BEA | `MTWO_THROWN_DK` | 933 | Mewtwo ThrownDK (L939) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3051 | 0x0BEB | `MTWO_THROWN_1` | 934 | Mewtwo Thrown1 (L940) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3052 | 0x0BEC | `MTWO_THROWN_2` | 935 | Mewtwo Thrown2 (L941) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3053 | 0x0BED | `MTWO_TAUNT` | 936 | Mewtwo Taunt (L942) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3054 | 0x0BEE | `MTWO_JAB_1` | 937 | Mewtwo Jab1 (L943) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3055 | 0x0BEF | `MTWO_JAB_2` | 938 | Mewtwo Jab2Begin (L944) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3056 | 0x0BF0 | `MTWO_DASH_ATTACK` | 939 | Mewtwo DashAttack (L945) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3057 | 0x0BF1 | `MTWO_FTILT_HIGH` | 940 | Mewtwo FTiltHigh (L946) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3058 | 0x0BF2 | `MTWO_FTILT` | 941 | Mewtwo FTilt (L947) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3059 | 0x0BF3 | `MTWO_FTILT_LOW` | 942 | Mewtwo FTiltLow (L948) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3060 | 0x0BF4 | `MTWO_UTILT` | 943 | Mewtwo UTilt (L949) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3061 | 0x0BF5 | `MTWO_DTILT` | 944 | Mewtwo DTilt (L950) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3062 | 0x0BF6 | `MTWO_FSMASH_HIGH` | 945 | Mewtwo FSmashHigh (L951) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3063 | 0x0BF7 | `MTWO_FSMASH` | 946 | Mewtwo FSmash (L952) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3064 | 0x0BF8 | `MTWO_FSMASH_LOW` | 947 | Mewtwo FSmashLow (L953) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3065 | 0x0BF9 | `MTWO_USMASH` | 948 | Mewtwo USmash (L954) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3066 | 0x0BFA | `MTWO_DSMASH` | 949 | Mewtwo DSmash (L955) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3067 | 0x0BFB | `MTWO_ATTACK_AIR_N` | 950 | Mewtwo AttackAirN (L956) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3068 | 0x0BFC | `MTWO_ATTACK_AIR_F` | 951 | Mewtwo AttackAirF (L957) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3069 | 0x0BFD | `MTWO_ATTACK_AIR_B` | 952 | Mewtwo AttackAirB (L958) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3070 | 0x0BFE | `MTWO_ATTACK_AIR_U` | 953 | Mewtwo AttackAirU (L959) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3071 | 0x0BFF | `MTWO_ATTACK_AIR_D` | 954 | Mewtwo AttackAirD (L960) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3072 | 0x0C00 | `MTWO_LANDING_AIR_F` | 955 | Mewtwo LandingAirF (L961) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3073 | 0x0C01 | `MTWO_LANDING_AIR_N` | 956 | Mewtwo LandingAirN (L962) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3074 | 0x0C02 | `MTWO_ENTRY_R` | 957 | Mewtwo EntryR (L963) | `src/Mewtwo/Mewtwo.asm` | 1 |
| 3075 | 0x0C03 | `MTWO_ENTRY_L` | 958 | Mewtwo EntryL (L964) | `src/Mewtwo/Mewtwo.asm` | 1 |
| 3076 | 0x0C04 | `MTWO_VICTORY_1` | 959 | Mewtwo Victory 1 (L965) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3077 | 0x0C05 | `MTWO_VICTORY_2` | 960 | Mewtwo Victory 2 (L966) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 4 |
| 3078 | 0x0C06 | `MTWO_VICTORY_3` | 961 | Mewtwo Victory 3 (L967) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3079 | 0x0C07 | `MTWO_CLAP` | 962 | Mewtwo Clap (L968) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3080 | 0x0C08 | `MTWO_CONTINUE_FALL` | 963 | Mewtwo Continue Fall (L969) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3081 | 0x0C09 | `MTWO_CONTINUE_UP` | 964 | Mewtwo Continue Up (L970) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3082 | 0x0C0A | `MTWO_POSE_1P` | 965 | Mewtwo 1P Pose (L971) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3083 | 0x0C0B | `MTWO_USPG_BEGIN` | 966 | Mewtwo Up Special Begin Ground (L972) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3084 | 0x0C0C | `MTWO_USPG_END` | 967 | Mewtwo Up Special End Ground (L973) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3085 | 0x0C0D | `MTWO_USPA_BEGIN` | 968 | Mewtwo Up Special Begin Air (L974) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3086 | 0x0C0E | `MTWO_USPA_END` | 969 | Mewtwo Up Special End Air (L975) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3087 | 0x0C0F | `MTWO_DSPG` | 970 | Mewtwo Down Special Ground (L976) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3088 | 0x0C10 | `MTWO_DSPA` | 971 | Mewtwo Down Special Air (L977) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3089 | 0x0C11 | `MTWO_NSPG_BEGIN` | 972 | Mewtwo Neutral Special Begin Ground (L978) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3090 | 0x0C12 | `MTWO_NSPG_CHARGE` | 973 | Mewtwo Neutral Special Charge Ground (L979) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3091 | 0x0C13 | `MTWO_NSPG_SHOOT` | 974 | Mewtwo Neutral Special Shoot Ground (L980) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3092 | 0x0C14 | `MTWO_NSPA_BEGIN` | 975 | Mewtwo Neutral Special Begin Air (L981) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3093 | 0x0C15 | `MTWO_NSPA_CHARGE` | 976 | Mewtwo Neutral Special Charge Air (L982) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3094 | 0x0C16 | `MTWO_NSPA_SHOOT` | 977 | Mewtwo Neutral Special Shoot Air (L983) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3095 | 0x0C17 | `MTWO_JAB_2_LOOP` | 978 | Mewtwo Jab2Loop (L984) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3096 | 0x0C18 | `MTWO_JAB_2_END` | 979 | Mewtwo Jab2End (L985) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3097 | 0x0C19 | `WOLF_1P_HMN` | 980 | Wolf 1p Mode Human (L986) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 3098 | 0x0C1A | `WOLF_1P_CPU` | 981 | Wolf 1p Mode CPU (L987) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 3099 | 0x0C1B | `REMIX_1P_KIRBY` | 982 | Remix 1p Kirby Stage File (L988) | `src/TagTeam.asm` | 2 |
| 3100 | 0x0C1C | `KIRBY_WOLF_NSP_G` | 983 | Kirby Wolf Copy Neutral Special Ground (L989) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3101 | 0x0C1D | `KIRBY_WOLF_NSP_A` | 984 | Kirby Wolf Copy Neutral Special Air (L990) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3102 | 0x0C1E | `CONKER_MAIN` | 985 | Conker's Main File (L991) | `src/Character.asm` | 1 |
| 3103 | 0x0C1F | `CONKER_CHARACTER` | 986 | Conker's Model File (L992) | `src/Character.asm` | 1 |
| 3104 | 0x0C20 | `CONKER_NUT_PROJECTILE_HITBOX` | 987 | Conker's Nut Projectile Hitboxes (L993) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 3105 | 0x0C21 | `CONKER_NUT_PROJECTILE_GRAPHIC` | 988 | Conker's Nut Projectile Graphics (L994) | `src/Character.asm` | 2 |
| 3106 | 0x0C22 | `CONKER_SELECTED` | 989 | Conker Selected (L995) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3107 | 0x0C23 | `CONKER_IDLE` | 990 | Conker Idle (L996) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 14 |
| 3108 | 0x0C24 | `CONKER_CROUCH` | 991 | Conker Crouch (L997) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3109 | 0x0C25 | `CONKER_CROUCH_IDLE` | 992 | Conker Crouch Idle (L998) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3110 | 0x0C26 | `CONKER_CROUCH_END` | 993 | Conker Crouch End (L999) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3111 | 0x0C27 | `CONKER_FALL` | 994 | Conker Fall (L1000) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3112 | 0x0C28 | `CONKER_UTILT` | 995 | Conker Up Tilt (L1001) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3113 | 0x0C29 | `CONKER_DASH` | 996 | Conker Dash (L1002) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3114 | 0x0C2A | `CONKER_RUN` | 997 | Conker Run (L1003) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3115 | 0x0C2B | `CONKER_RUN_BRAKE` | 998 | Conker Run Brake (L1004) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3116 | 0x0C2C | `CONKER_RUN_TURN` | 999 | Conker Run Turn (L1005) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3117 | 0x0C2D | `CONKER_USP_START_AIR` | 1000 | Conker Up Special Start Air (L1006) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3118 | 0x0C2E | `CONKER_USP_LOOP_AIR` | 1001 | Conker Up Special Loop Air (L1007) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3119 | 0x0C2F | `CONKER_SFALL` | 1002 | Conker Special Fall (L1008) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3120 | 0x0C30 | `CONKER_FAIR` | 1003 | Conker Forward Air (L1009) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3121 | 0x0C31 | `CONKER_WEAPONS` | 1004 | Conker Weapons (L1010) |  | 0 |
| 3122 | 0x0C32 | `CONKER_DSP_GROUND` | 1005 | Conker Down Special Ground (L1011) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3123 | 0x0C33 | `CONKER_RAYGUN_GROUND` | 1006 | Conker Ray Gun Ground (L1012) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3124 | 0x0C34 | `CONKER_GRENADE_PROJECTILE_HITBOX` | 1007 | Conker's Grenade Projectile Hitboxes (L1013) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 3125 | 0x0C35 | `CONKER_GRENADE_PROJECTILE_GRAPHIC` | 1008 | Conker's Grenade Projectile Graphics (L1014) |  | 0 |
| 3126 | 0x0C36 | `CONKER_DSP_AIR` | 1009 | Conker Down Special Air (L1015) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3127 | 0x0C37 | `CONKER_DSP_GROUND_FAIL` | 1010 | Conker Down Special Ground Fail (L1016) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3128 | 0x0C38 | `CONKER_DSP_AIR_FAIL` | 1011 | Conker Down Special Air Fail (L1017) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3129 | 0x0C39 | `CONKER_ROLLF` | 1012 | Conker Roll Forward (L1018) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3130 | 0x0C3A | `CONKER_ROLLB` | 1013 | Conker Roll Backward (L1019) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3131 | 0x0C3B | `CONKER_JAB1` | 1014 | Conker Jab 1 (L1020) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3132 | 0x0C3C | `CONKER_JAB2` | 1015 | Conker Jab 2 (L1021) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3133 | 0x0C3D | `CONKER_JAB_LOOP_START` | 1016 | Conker Jab Loop Start (L1022) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3134 | 0x0C3E | `CONKER_JAB_LOOP` | 1017 | Conker Jab Loop (L1023) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3135 | 0x0C3F | `CONKER_JAB_LOOP_END` | 1018 | Conker Jab Loop End (L1024) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3136 | 0x0C40 | `CONKER_ITEM_NEUTRAL` | 1019 | Conker Item Neutral (L1025) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 8 |
| 3137 | 0x0C41 | `CONKER_ITEM_TILT` | 1020 | Conker Item Tilt (L1026) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 8 |
| 3138 | 0x0C42 | `CONKER_DAIR` | 1021 | Conker Down Air (L1027) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3139 | 0x0C43 | `CONKER_JUMPF` | 1022 | Conker Jump Forward (L1028) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3140 | 0x0C44 | `CONKER_JUMPB` | 1023 | Conker Jump Backward (L1029) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3141 | 0x0C45 | `CONKER_JUMPAF` | 1024 | Conker Jump Aerial Forward (L1030) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3142 | 0x0C46 | `CONKER_DASH_ATTACK` | 1025 | Conker Dash Attack (L1031) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3143 | 0x0C47 | `CONKER_DSMASH` | 1026 | Conker Down Smash (L1032) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3144 | 0x0C48 | `CONKER_NAIR` | 1027 | Conker Neutral Air (L1033) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3145 | 0x0C49 | `CONKER_UAIR` | 1028 | Conker Up Air (L1034) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3146 | 0x0C4A | `CONKER_FSMASH` | 1029 | Conker Forward Smash (L1035) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3147 | 0x0C4B | `CONKER_FTILT_HIGH` | 1030 | Conker Forward Tilt High (L1036) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3148 | 0x0C4C | `CONKER_FTILT_MID` | 1031 | Conker Forward Tilt Mid (L1037) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3149 | 0x0C4D | `CONKER_FTILT_LOW` | 1032 | Conker Forward Tilt Low (L1038) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3150 | 0x0C4E | `CONKER_USMASH` | 1033 | Conker Up Smash (L1039) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3151 | 0x0C4F | `CONKER_DTILT` | 1034 | Conker Down Tilt (L1040) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3152 | 0x0C50 | `CONKER_BAIR` | 1035 | Conker Back Air (L1041) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3153 | 0x0C51 | `CONKER_DOWNSTANDU` | 1036 | Conker DownStandU (L1042) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm`, `src/TwelveCharBattle.asm` | 8 |
| 3154 | 0x0C52 | `CONKER_TECHF` | 1037 | Conker Tech Forward (L1043) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3155 | 0x0C53 | `CONKER_TECHB` | 1038 | Conker Tech Back (L1044) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3156 | 0x0C54 | `CONKER_DOWNBOUNCEU` | 1039 | Conker DownBounceU (L1045) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 6 |
| 3157 | 0x0C55 | `CONKER_CAPTURE_PULLED` | 1040 | Conker CapturePulled (L1046) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3158 | 0x0C56 | `CONKER_DAMAGE_FLYHIGH` | 1041 | Conker Damage Fly High (L1047) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3159 | 0x0C57 | `CONKER_DAMAGE_FLYMID` | 1042 | Conker Damage Fly Mid (L1048) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3160 | 0x0C58 | `CONKER_DAMAGE_FLYLOW` | 1043 | Conker Damage Fly Low (L1049) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3161 | 0x0C59 | `CONKER_GRAB` | 1044 | Conker Catch (L1050) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3162 | 0x0C5A | `CONKER_GRAB_PULL` | 1045 | Conker Catch Pull (L1051) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3163 | 0x0C5B | `CONKER_THROWF` | 1046 | Conker Forward Throw (L1052) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3164 | 0x0C5C | `CONKER_THROWB` | 1047 | Conker Back Throw (L1053) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3165 | 0x0C5D | `CONKER_WALK1` | 1048 | Conker Walk 1 (L1054) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm`, `src/Slippy/Slippy.asm` | 3 |
| 3166 | 0x0C5E | `CONKER_WALK2` | 1049 | Conker Walk 2 (L1055) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm`, `src/Slippy/Slippy.asm` | 3 |
| 3167 | 0x0C5F | `CONKER_WALK3` | 1050 | Conker Walk 3 (L1056) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm`, `src/Slippy/Slippy.asm` | 3 |
| 3168 | 0x0C60 | `CONKER_NSPG_BEGIN` | 1051 | Conker NSP Begin Ground (L1057) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3169 | 0x0C61 | `CONKER_NSPG_WAIT` | 1052 | Conker NSP Wait Ground (L1058) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3170 | 0x0C62 | `CONKER_NSPG_END` | 1053 | Conker NSP End Ground (L1059) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3171 | 0x0C63 | `CONKER_NSPA_BEGIN` | 1054 | Conker NSP Begin Air (L1060) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3172 | 0x0C64 | `CONKER_NSPA_WAIT` | 1055 | Conker NSP Wait Air (L1061) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3173 | 0x0C65 | `CONKER_NSPA_END` | 1056 | Conker NSP End Air (L1062) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3174 | 0x0C66 | `CONKER_CLIFF_CATCH` | 1057 | Conker Cliff Catch (L1063) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3175 | 0x0C67 | `CONKER_CLIFF_WAIT` | 1058 | Conker Cliff Wait (L1064) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3176 | 0x0C68 | `CONKER_CLIFF_QUICK` | 1059 | Conker Cliff Quick (L1065) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3177 | 0x0C69 | `CONKER_CLIFF_CLIMB_QUICK1` | 1060 | Conker Cliff Climb Quick 1 (L1066) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3178 | 0x0C6A | `CONKER_CLIFF_CLIMB_QUICK2` | 1061 | Conker Cliff Climb Quick 2 (L1067) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3179 | 0x0C6B | `CONKER_CLIFF_SLOW` | 1062 | Conker Cliff Slow (L1068) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3180 | 0x0C6C | `CONKER_CLIFF_CLIMB_SLOW1` | 1063 | Conker Cliff Climb Slow 1 (L1069) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3181 | 0x0C6D | `CONKER_CLIFF_CLIMB_SLOW2` | 1064 | Conker Cliff Climb Slow 2 (L1070) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3182 | 0x0C6E | `CONKER_CLIFF_ATTACK_QUICK1` | 1065 | Conker Cliff Attack Quick 1 (L1071) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3183 | 0x0C6F | `CONKER_CLIFF_ATTACK_QUICK2` | 1066 | Conker Cliff Attack Quick 2 (L1072) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3184 | 0x0C70 | `CONKER_CLIFF_ATTACK_SLOW1` | 1067 | Conker Cliff Attack Slow 1 (L1073) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3185 | 0x0C71 | `CONKER_CLIFF_ATTACK_SLOW2` | 1068 | Conker Cliff Attack Slow 2 (L1074) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3186 | 0x0C72 | `CONKER_CLIFF_ESCAPE_QUICK1` | 1069 | Conker Cliff Escape Quick 1 (L1075) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3187 | 0x0C73 | `CONKER_CLIFF_ESCAPE_QUICK2` | 1070 | Conker Cliff Escape Quick 2 (L1076) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3188 | 0x0C74 | `CONKER_CLIFF_ESCAPE_SLOW1` | 1071 | Conker Cliff Escape Slow 1 (L1077) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3189 | 0x0C75 | `CONKER_CLIFF_ESCAPE_SLOW2` | 1072 | Conker Cliff Escape Slow 2 (L1078) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3190 | 0x0C76 | `CONKER_JUMPSQUAT` | 1073 | Conker Jumpsquat (L1079) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 12 |
| 3191 | 0x0C77 | `CONKER_ITEM_THROWF` | 1074 | Conker Item Throw F Air (L1080) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 8 |
| 3192 | 0x0C78 | `CONKER_ITEM_THROWU` | 1075 | Conker Item Throw U Air (L1081) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3193 | 0x0C79 | `CONKER_ITEM_THROWD` | 1076 | Conker Item Throw D Air (L1082) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3194 | 0x0C7A | `CONKER_1P` | 1077 | Conker 1p Pose (L1083) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3195 | 0x0C7B | `CONKER_TAUNT` | 1078 | Conker Taunt (L1084) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3196 | 0x0C7C | `CONKER_FALLAERIAL` | 1079 | Conker Fall Aerial (L1085) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3197 | 0x0C7D | `CONKER_JUMPAB` | 1080 | Conker Jump Air B (L1086) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3198 | 0x0C7E | `CONKER_CLAP` | 1081 | Conker Clap (L1087) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3199 | 0x0C7F | `CONKER_DOWNATTACKD` | 1082 | Conker Down Attack Down (L1088) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3200 | 0x0C80 | `CONKER_VICTORY1` | 1083 | Conker Victory 1 (L1089) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3201 | 0x0C81 | `CONKER_VICTORY2` | 1084 | Conker Victory 2 (L1090) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3202 | 0x0C82 | `CONKER_TURN` | 1085 | Conker Turn (L1091) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3203 | 0x0C83 | `CONKER_SHIELD_POSE` | 1086 | Conker Shield Pose (L1092) | `src/Character.asm` | 2 |
| 3204 | 0x0C84 | `CONKER_PUPPET_FALL` | 1087 | Conker Puppet Fall (L1093) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3205 | 0x0C85 | `CONKER_PUPPET_UP` | 1088 | Conker Puppet Up (L1094) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3206 | 0x0C86 | `CONKER_SHIELD_ON` | 1089 | Conker Shield On (L1095) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3207 | 0x0C87 | `CONKER_SHIELD_OFF` | 1090 | Conker Shield Off (L1096) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3208 | 0x0C88 | `GREGS_HAND` | 1091 | Greg's Hand (L1097) | `src/Character.asm` | 2 |
| 3209 | 0x0C89 | `CONKER_ENTRY` | 1092 | Conker Entry (L1098) | `src/Conker/Conker.asm` | 2 |
| 3210 | 0x0C8A | `CONKER_SHIELD_DROP` | 1093 | Conker Shield Drop (L1099) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3211 | 0x0C8B | `BOWSER_1P_CPU` | 1094 | Bowser 1p CPU Pose (L1100) | `src/Bowser/bowser.asm`, `src/NBowser/nbowser.asm` | 2 |
| 3212 | 0x0C8C | `GND_1P_CPU` | 1095 | Ganondorf 1p CPU Pose (L1101) | `src/Ganondorf/Ganondorf.asm`, `src/NGanondorf/NGanondorf.asm` | 2 |
| 3213 | 0x0C8D | `VENOM_HEADER` | 1096 | Venom Stage Header (L1102) |  | 0 |
| 3214 | 0x0C8E | `VENOM_STAGE` | 1097 | Venom Stage (L1103) |  | 0 |
| 3215 | 0x0C8F | `VENOM_BG` | 1098 | Venom Stage BG (L1104) |  | 0 |
| 3216 | 0x0C90 | `WOLF_APPEAR` | 1099 | Wolf Appear Right (L1105) | `src/Wolf/Wolf.asm` | 1 |
| 3217 | 0x0C91 | `WOLF_ENTERPIPE` | 1100 | Wolf Enter Pipe (L1106) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 3218 | 0x0C92 | `WOLF_EXITPIPE` | 1101 | Wolf Exit Pipe (L1107) | `src/NWolf/NWolf.asm`, `src/Wolf/Wolf.asm` | 2 |
| 3219 | 0x0C93 | `CONKER_FAIR_LANDING` | 1102 | Conker Forward Air Landing (L1108) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3220 | 0x0C94 | `CONKER_BAIR_LANDING` | 1103 | Conker Back Air Landing (L1109) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3221 | 0x0C95 | `WOLF_TARGET_HEADER` | 1104 | Wolf Target Test Header (L1110) |  | 0 |
| 3222 | 0x0C96 | `WOLF_TARGET_STAGE` | 1105 | Wolf Target Test Stage (L1111) |  | 0 |
| 3223 | 0x0C97 | `GND_TEAM_POSE` | 1106 | Ganondorf Team Pose (L1112) | `src/SinglePlayerModes.asm` | 4 |
| 3224 | 0x0C98 | `DRM_TEAM_POSE` | 1107 | Doctor Mario Team Pose (L1113) | `src/SinglePlayerModes.asm` | 3 |
| 3225 | 0x0C99 | `WOLF_PLATFORM_HEADER` | 1108 | Wolf Board the Platforms Header (L1114) |  | 0 |
| 3226 | 0x0C9A | `WOLF_PLATFORM_STAGE` | 1109 | Wolf Board the Platforms Stage (L1115) |  | 0 |
| 3227 | 0x0C9B | `WOLF_SHIELD_POSE` | 1110 | Wolf Shield Pose (L1116) | `src/Character.asm` | 2 |
| 3228 | 0x0C9C | `CONKER_TARGET_HEADER` | 1111 | Conker Target Test Header (L1117) |  | 0 |
| 3229 | 0x0C9D | `CONKER_TARGET_STAGE` | 1112 | Conker Target Test Stage (L1118) |  | 0 |
| 3230 | 0x0C9E | `WARIO_1P_CPU_POSE` | 1113 | Wario 1p CPU Pose (L1119) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 3231 | 0x0C9F | `WARIO_TEAM_POSE` | 1114 | Wario Team Pose (L1120) | `src/SinglePlayerModes.asm` | 3 |
| 3232 | 0x0CA0 | `PIANO_1P_CPU_POSE` | 1115 | Mad Piano 1p CPU Pose (L1121) | `src/Piano/Piano.asm` | 1 |
| 3233 | 0x0CA1 | `CONKER_1P_CPU_POSE` | 1116 | Conker 1p CPU Pose (L1122) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 2 |
| 3234 | 0x0CA2 | `CONKER_TEAM_POSE` | 1117 | Conker Team Pose (L1123) | `src/SinglePlayerModes.asm` | 3 |
| 3235 | 0x0CA3 | `WOLF_TEAM_POSE` | 1118 | Wolf Team Pose (L1124) | `src/SinglePlayerModes.asm` | 4 |
| 3236 | 0x0CA4 | `FALCO_1P_DUO_POSE` | 1119 | Falco Duo Pose (L1125) | `src/NFalco/NFalco.asm`, `src/SinglePlayerModes.asm` | 2 |
| 3237 | 0x0CA5 | `BOWSER_TEAM_POSE` | 1120 | Bowser Team Pose (L1126) | `src/SinglePlayerModes.asm` | 3 |
| 3238 | 0x0CA6 | `MTWO_1P_CPU_POSE` | 1121 | Mewtwo 1p CPU Pose (L1127) | `src/Mewtwo/Mewtwo.asm`, `src/NMewtwo/NMewtwo.asm` | 2 |
| 3239 | 0x0CA7 | `MTWO_TEAM_POSE` | 1122 | Mewtwo Team Pose (L1128) | `src/SinglePlayerModes.asm` | 3 |
| 3240 | 0x0CA8 | `DSAMUS_1P_CPU_POSE` | 1123 | Dark Samus 1p CPU Pose (L1129) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 3241 | 0x0CA9 | `DSAMUS_1P_POSE` | 1124 | Dark Samus 1p Pose (L1130) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 3242 | 0x0CAA | `LUCAS_1P_CPU_POSE` | 1125 | Lucas 1p CPU Pose (L1131) | `src/Lucas/Lucas.asm`, `src/NLucas/NLucas.asm` | 2 |
| 3243 | 0x0CAB | `LUCAS_TEAM_POSE` | 1126 | Lucas Team Pose (L1132) | `src/SinglePlayerModes.asm` | 2 |
| 3244 | 0x0CAC | `YLINK_1P_CPU_POSE` | 1127 | Young Link 1p CPU Pose (L1133) | `src/NYoungLink/NYoungLink.asm`, `src/YoungLink/YoungLink.asm` | 2 |
| 3245 | 0x0CAD | `YLINK_TEAM_POSE` | 1128 | Young Link Team Pose (L1134) | `src/SinglePlayerModes.asm` | 3 |
| 3246 | 0x0CAE | `CONKER_PLATFORM_HEADER` | 1129 | Conker Board the Platforms Header (L1135) |  | 0 |
| 3247 | 0x0CAF | `CONKER_PLATFORM_STAGE` | 1130 | Conker Board the Platforms Stage (L1136) |  | 0 |
| 3248 | 0x0CB0 | `DRM_1P_CPU_POSE` | 1131 | Dr. Mario 1p CPU Pose (L1137) | `src/DrMario/DrMario.asm`, `src/NDrMario/NDrMario.asm` | 2 |
| 3249 | 0x0CB1 | `CONKER_ITEM_SWING_DASH` | 1132 | Conker Item Swing Dash (L1138) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 8 |
| 3250 | 0x0CB2 | `GBOWSER_1P_CPU` | 1133 | Giga Bowser 1p CPU Pose (L1139) | `src/GBowser/gbowser.asm` | 1 |
| 3251 | 0x0CB3 | `CONKER_ITEM_SWING_SMASH` | 1134 | Conker Item Swing Smash (L1140) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 8 |
| 3252 | 0x0CB4 | `WINDY_HEADER` | 1135 | Windy Stage Header (L1141) |  | 0 |
| 3253 | 0x0CB5 | `WINDY_STAGE` | 1136 | Windy Stage (L1142) |  | 0 |
| 3254 | 0x0CB6 | `WINDY_BG` | 1137 | Windy Stage BG (L1143) |  | 0 |
| 3255 | 0x0CB7 | `DATADYNE_HEADER` | 1138 | dataDyne Stage Header (L1144) |  | 0 |
| 3256 | 0x0CB8 | `DATADYNE_STAGE` | 1139 | dataDyne Stage (L1145) |  | 0 |
| 3257 | 0x0CB9 | `DATADYNE_BG` | 1140 | dataDyne Stage BG (L1146) |  | 0 |
| 3258 | 0x0CBA | `CLANCER_HEADER` | 1141 | Planet Clancer Stage Header (L1147) |  | 0 |
| 3259 | 0x0CBB | `CLANCER_STAGE` | 1142 | Planet Clancer Stage (L1148) |  | 0 |
| 3260 | 0x0CBC | `CLANCER_BG` | 1143 | Planet Clancer Stage BG (L1149) |  | 0 |
| 3261 | 0x0CBD | `KIRBY_CONKER_NSPG_BEGIN` | 1144 | Kirby Conker Copy Neutral Special Ground Start (L1150) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3262 | 0x0CBE | `KIRBY_CONKER_NSPG_WAIT` | 1145 | Kirby Conker Copy Neutral Special Ground Wait (L1151) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3263 | 0x0CBF | `KIRBY_CONKER_NSPG_END` | 1146 | Kirby Conker Copy Neutral Special Ground End (L1152) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3264 | 0x0CC0 | `KIRBY_CONKER_NSPA_BEGIN` | 1147 | Kirby Conker Copy Neutral Special Air Start (L1153) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3265 | 0x0CC1 | `KIRBY_CONKER_NSPA_WAIT` | 1148 | Kirby Conker Copy Neutral Special Air Wait (L1154) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3266 | 0x0CC2 | `KIRBY_CONKER_NSPA_END` | 1149 | Kirby Conker Copy Neutral Special Air End (L1155) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3267 | 0x0CC3 | `CONKER_STUN` | 1150 | Conker Stun (L1156) | `src/Conker/Conker.asm`, `src/NConker/NConker.asm` | 4 |
| 3268 | 0x0CC4 | `JAPES_HEADER` | 1151 | Jungle Japes Header (L1157) |  | 0 |
| 3269 | 0x0CC5 | `JAPES_STAGE` | 1152 | Jungle Japes Stage (L1158) |  | 0 |
| 3270 | 0x0CC6 | `MTWO_SBALL_PROJECTILE_HITBOX` | 1153 | Mewtwo Shadowball Projectile Hitbox (L1159) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 3271 | 0x0CC7 | `MTWO_SBALL_PROJECTILE_GRAPHIC` | 1154 | Mewtwo Shadowball Projectile Graphics (L1160) |  | 0 |
| 3272 | 0x0CC8 | `KLAPTRAP` | 1155 | Klaptrap (L1161) |  | 0 |
| 3273 | 0x0CC9 | `WARIO_NSP_RECOIL_G` | 1156 | Wario Neutral Special Recoil Ground (L1162) | `src/NWario/NWario.asm`, `src/Wario/Wario.asm` | 2 |
| 3274 | 0x0CCA | `MARTH_MAIN` | 1157 | Marth Main (L1163) | `src/Character.asm` | 1 |
| 3275 | 0x0CCB | `MARTH_CHARACTER` | 1158 | Marth Character (L1164) | `src/Character.asm` | 1 |
| 3276 | 0x0CCC | `MARTH_SHIELD` | 1159 | Marth Shield Pose (L1165) | `src/Character.asm` | 3 |
| 3277 | 0x0CCD | `MARTH_TUMBLE` | 1160 | Marth Tumble (L1166) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 30 |
| 3278 | 0x0CCE | `MARTH_IDLE` | 1161 | Marth Idle (L1167) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 19 |
| 3279 | 0x0CCF | `MARTH_DOWN_BOUNCE_D` | 1162 | Marth DownBounceD (L1168) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 9 |
| 3280 | 0x0CD0 | `MARTH_DOWN_STAND_D` | 1163 | Marth DownStandD (L1169) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 9 |
| 3281 | 0x0CD1 | `MARTH_WALK_1` | 1164 | Marth Walk1 (L1170) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3282 | 0x0CD2 | `MARTH_WALK_2` | 1165 | Marth Walk2 (L1171) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3283 | 0x0CD3 | `MARTH_WALK_3` | 1166 | Marth Walk3 (L1172) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3284 | 0x0CD4 | `MARTH_WALK_END` | 1167 | Marth WalkEnd (L1173) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3285 | 0x0CD5 | `MARTH_DASH` | 1168 | Marth Dash (L1174) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3286 | 0x0CD6 | `MARTH_RUN` | 1169 | Marth Run (L1175) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3287 | 0x0CD7 | `MARTH_RUN_BRAKE` | 1170 | Marth RunBrake (L1176) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3288 | 0x0CD8 | `MARTH_TURN` | 1171 | Marth Turn (L1177) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3289 | 0x0CD9 | `MARTH_TURN_RUN` | 1172 | Marth TurnRun (L1178) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3290 | 0x0CDA | `MARTH_LANDING` | 1173 | Marth Landing (L1179) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 18 |
| 3291 | 0x0CDB | `MARTH_JUMP_F` | 1174 | Marth JumpF (L1180) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3292 | 0x0CDC | `MARTH_JUMP_B` | 1175 | Marth JumpB (L1181) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3293 | 0x0CDD | `MARTH_JUMP_AERIAL_F` | 1176 | Marth JumpAerialF (L1182) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3294 | 0x0CDE | `MARTH_JUMP_AERIAL_B` | 1177 | Marth JumpAerialB (L1183) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3295 | 0x0CDF | `MARTH_FALL` | 1178 | Marth Fall (L1184) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3296 | 0x0CE0 | `MARTH_FALL_AERIAL` | 1179 | Marth FallAerial (L1185) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3297 | 0x0CE1 | `MARTH_CROUCH` | 1180 | Marth Crouch (L1186) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3298 | 0x0CE2 | `MARTH_CROUCH_IDLE` | 1181 | Marth CrouchIdle (L1187) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3299 | 0x0CE3 | `MARTH_CROUCH_END` | 1182 | Marth CrouchEnd (L1188) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3300 | 0x0CE4 | `MARTH_PASS` | 1183 | Marth Pass (L1189) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3301 | 0x0CE5 | `MARTH_TEETER` | 1184 | Marth Teeter (L1190) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3302 | 0x0CE6 | `MARTH_TEETER_START` | 1185 | Marth TeeterStart (L1191) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3303 | 0x0CE7 | `MARTH_DAMAGE_HIGH_1` | 1186 | Marth DamageHigh1 (L1192) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3304 | 0x0CE8 | `MARTH_DAMAGE_HIGH_2` | 1187 | Marth DamageHigh2 (L1193) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3305 | 0x0CE9 | `MARTH_DAMAGE_HIGH_3` | 1188 | Marth DamageHigh3 (L1194) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3306 | 0x0CEA | `MARTH_DAMAGE_MID_1` | 1189 | Marth DamageMid1 (L1195) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3307 | 0x0CEB | `MARTH_DAMAGE_MID_2` | 1190 | Marth DamageMid2 (L1196) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3308 | 0x0CEC | `MARTH_DAMAGE_MID_3` | 1191 | Marth DamageMid3 (L1197) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3309 | 0x0CED | `MARTH_DAMAGE_LOW_1` | 1192 | Marth DamageLow1 (L1198) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3310 | 0x0CEE | `MARTH_DAMAGE_LOW_2` | 1193 | Marth DamageLow2 (L1199) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3311 | 0x0CEF | `MARTH_DAMAGE_LOW_3` | 1194 | Marth DamageLow3 (L1200) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3312 | 0x0CF0 | `MARTH_DAMAGE_AIR_1` | 1195 | Marth DamageAir1 (L1201) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3313 | 0x0CF1 | `MARTH_DAMAGE_AIR_2` | 1196 | Marth DamageAir2 (L1202) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3314 | 0x0CF2 | `MARTH_DAMAGE_AIR_3` | 1197 | Marth DamageAir3 (L1203) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3315 | 0x0CF3 | `MARTH_DAMAGE_ELEC` | 1198 | Marth DamageElec (L1204) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3316 | 0x0CF4 | `MARTH_DAMAGE_FLY_HIGH` | 1199 | Marth DamageFlyHigh (L1205) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3317 | 0x0CF5 | `MARTH_DAMAGE_FLY_MID` | 1200 | Marth DamageFlyMid (L1206) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3318 | 0x0CF6 | `MARTH_DAMAGE_FLY_LOW` | 1201 | Marth DamageFlyLow (L1207) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3319 | 0x0CF7 | `MARTH_DAMAGE_FLY_TOP` | 1202 | Marth DamageFlyTop (L1208) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3320 | 0x0CF8 | `MARTH_DAMAGE_FLY_ROLL` | 1203 | Marth DamageFlyRoll (L1209) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3321 | 0x0CF9 | `MARTH_FALL_SPECIAL` | 1204 | Marth FallSpecial (L1210) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3322 | 0x0CFA | `MARTH_ENTER_PIPE` | 1205 | Marth EnterPipe (L1211) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3323 | 0x0CFB | `MARTH_EXIT_PIPE` | 1206 | Marth ExitPipe (L1212) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3324 | 0x0CFC | `MARTH_EXIT_PIPE_WALK` | 1207 | Marth ExitPipeWalk (L1213) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3325 | 0x0CFD | `MARTH_CEILING_BONK` | 1208 | Marth CeilingBonk (L1214) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3326 | 0x0CFE | `MARTH_DOWN_BOUNCE_U` | 1209 | Marth DownBounceU (L1215) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3327 | 0x0CFF | `MARTH_DOWN_STAND_U` | 1210 | Marth DownStandU (L1216) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` +1 | 9 |
| 3328 | 0x0D00 | `MARTH_TECH_F` | 1211 | Marth TechF (L1217) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3329 | 0x0D01 | `MARTH_TECH_B` | 1212 | Marth TechB (L1218) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3330 | 0x0D02 | `MARTH_DOWN_FORWARD_D` | 1213 | Marth DownForwardD (L1219) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3331 | 0x0D03 | `MARTH_DOWN_FORWARD_U` | 1214 | Marth DownForwardU (L1220) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3332 | 0x0D04 | `MARTH_DOWN_BACK_D` | 1215 | Marth DownBackD (L1221) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3333 | 0x0D05 | `MARTH_DOWN_BACK_U` | 1216 | Marth DownBackU (L1222) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3334 | 0x0D06 | `MARTH_DOWN_ATTACK_D` | 1217 | Marth DownAttackD (L1223) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3335 | 0x0D07 | `MARTH_DOWN_ATTACK_U` | 1218 | Marth DownAttackU (L1224) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3336 | 0x0D08 | `MARTH_TECH` | 1219 | Marth Tech (L1225) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3337 | 0x0D09 | `MARTH_UNKNOWN_053` | 1220 | Marth Unknown053 (L1226) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3338 | 0x0D0A | `MARTH_CLIFF_CATCH` | 1221 | Marth CliffCatch (L1227) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3339 | 0x0D0B | `MARTH_CLIFF_WAIT` | 1222 | Marth CliffWait (L1228) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3340 | 0x0D0C | `MARTH_CLIFF_QUICK` | 1223 | Marth CliffQuick (L1229) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3341 | 0x0D0D | `MARTH_CLIFF_CLIMB_QUICK_1` | 1224 | Marth CliffClimbQuick1 (L1230) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3342 | 0x0D0E | `MARTH_CLIFF_CLIMB_QUICK_2` | 1225 | Marth CliffClimbQuick2 (L1231) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3343 | 0x0D0F | `MARTH_CLIFF_SLOW` | 1226 | Marth CliffSlow (L1232) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3344 | 0x0D10 | `MARTH_CLIFF_CLIMB_SLOW_1` | 1227 | Marth CliffClimbSlow1 (L1233) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3345 | 0x0D11 | `MARTH_CLIFF_CLIMB_SLOW_2` | 1228 | Marth CliffClimbSlow2 (L1234) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3346 | 0x0D12 | `MARTH_CLIFF_ATTACK_QUICK_1` | 1229 | Marth CliffAttackQuick1 (L1235) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3347 | 0x0D13 | `MARTH_CLIFF_ATTACK_QUICK_2` | 1230 | Marth CliffAttackQuick2 (L1236) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3348 | 0x0D14 | `MARTH_CLIFF_ATTACK_SLOW_1` | 1231 | Marth CliffAttackSlow1 (L1237) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3349 | 0x0D15 | `MARTH_CLIFF_ATTACK_SLOW_2` | 1232 | Marth CliffAttackSlow2 (L1238) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3350 | 0x0D16 | `MARTH_CLIFF_ESCAPE_QUICK_1` | 1233 | Marth CliffEscapeQuick1 (L1239) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3351 | 0x0D17 | `MARTH_CLIFF_ESCAPE_QUICK_2` | 1234 | Marth CliffEscapeQuick2 (L1240) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3352 | 0x0D18 | `MARTH_CLIFF_ESCAPE_SLOW_1` | 1235 | Marth CliffEscapeSlow1 (L1241) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3353 | 0x0D19 | `MARTH_CLIFF_ESCAPE_SLOW_2` | 1236 | Marth CliffEscapeSlow2 (L1242) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3354 | 0x0D1A | `MARTH_LIGHT_ITEM_PICKUP` | 1237 | Marth LightItemPickup (L1243) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3355 | 0x0D1B | `MARTH_HEAVY_ITEM_PICKUP` | 1238 | Marth HeavyItemPickup (L1244) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3356 | 0x0D1C | `MARTH_ITEM_DROP` | 1239 | Marth ItemDrop (L1245) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3357 | 0x0D1D | `MARTH_ITEM_THROW_DASH` | 1240 | Marth ItemThrowDash (L1246) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3358 | 0x0D1E | `MARTH_ITEM_THROW` | 1241 | Marth ItemThrowF (L1247) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 12 |
| 3359 | 0x0D1F | `MARTH_ITEM_THROW_U` | 1242 | Marth ItemThrowU (L1248) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3360 | 0x0D20 | `MARTH_ITEM_THROW_D` | 1243 | Marth ItemThrowD (L1249) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3361 | 0x0D21 | `MARTH_ITEM_THROW_AIR` | 1244 | Marth ItemThrowAirF (L1250) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 12 |
| 3362 | 0x0D22 | `MARTH_ITEM_THROW_AIR_U` | 1245 | Marth ItemThrowAirU (L1251) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3363 | 0x0D23 | `MARTH_ITEM_THROW_AIR_D` | 1246 | Marth ItemThrowAirD (L1252) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3364 | 0x0D24 | `MARTH_HEAVY_ITEM_THROW` | 1247 | Marth HeavyItemThrowF (L1253) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 12 |
| 3365 | 0x0D25 | `MARTH_ITEM_NEUTRAL` | 1248 | Marth ItemNeutral (L1254) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 12 |
| 3366 | 0x0D26 | `MARTH_ITEM_TILT` | 1249 | Marth ItemTilt (L1255) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 12 |
| 3367 | 0x0D27 | `MARTH_ITEM_SMASH` | 1250 | Marth ItemSmash (L1256) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 12 |
| 3368 | 0x0D28 | `MARTH_ITEM_DASH_ATTACK` | 1251 | Marth ItemDashAttack (L1257) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 12 |
| 3369 | 0x0D29 | `MARTH_ITEM_SHOOT` | 1252 | Marth ItemShoot (L1258) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3370 | 0x0D2A | `MARTH_ITEM_SHOOT_AIR` | 1253 | Marth ItemShootAir (L1259) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3371 | 0x0D2B | `MARTH_HAMMER_IDLE` | 1254 | Marth HammerIdle (L1260) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3372 | 0x0D2C | `MARTH_HAMMER_MOVE` | 1255 | Marth HammerMove (L1261) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 15 |
| 3373 | 0x0D2D | `MARTH_SHIELD_ON` | 1256 | Marth ShieldOn (L1262) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3374 | 0x0D2E | `MARTH_SHIELD_OFF` | 1257 | Marth ShieldOff (L1263) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3375 | 0x0D2F | `MARTH_ROLL_F` | 1258 | Marth RollF (L1264) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3376 | 0x0D30 | `MARTH_ROLL_B` | 1259 | Marth RollB (L1265) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3377 | 0x0D31 | `MARTH_STUN` | 1260 | Marth Stun (L1266) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3378 | 0x0D32 | `MARTH_GRAB` | 1261 | Marth Grab (L1267) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3379 | 0x0D33 | `MARTH_GRAB_PULL` | 1262 | Marth GrabPull (L1268) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3380 | 0x0D34 | `MARTH_THROW_F` | 1263 | Marth ThrowF (L1269) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3381 | 0x0D35 | `MARTH_THROW_B` | 1264 | Marth ThrowB (L1270) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3382 | 0x0D36 | `MARTH_CAPTURE_PULLED` | 1265 | Marth CapturePulled (L1271) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 6 |
| 3383 | 0x0D37 | `MARTH_THROWN_DK_PULLED` | 1266 | Marth ThrownDKPulled (L1272) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3384 | 0x0D38 | `MARTH_THROWN_MARIO_BROS` | 1267 | Marth ThrownMarioBros (L1273) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3385 | 0x0D39 | `MARTH_THROWN_DK` | 1268 | Marth ThrownDK (L1274) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3386 | 0x0D3A | `MARTH_THROWN_1` | 1269 | Marth Thrown1 (L1275) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3387 | 0x0D3B | `MARTH_THROWN_2` | 1270 | Marth Thrown2 (L1276) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3388 | 0x0D3C | `MARTH_TAUNT` | 1271 | Marth Taunt (L1277) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3389 | 0x0D3D | `MARTH_JAB_1` | 1272 | Marth Jab1 (L1278) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3390 | 0x0D3E | `MARTH_JAB_2` | 1273 | Marth Jab2 (L1279) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3391 | 0x0D3F | `MARTH_DASH_ATTACK` | 1274 | Marth DashAttack (L1280) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3392 | 0x0D40 | `MARTH_F_TILT` | 1275 | Marth FTilt (L1281) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3393 | 0x0D41 | `MARTH_U_TILT` | 1276 | Marth UTilt (L1282) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3394 | 0x0D42 | `MARTH_D_TILT` | 1277 | Marth DTilt (L1283) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3395 | 0x0D43 | `MARTH_F_SMASH` | 1278 | Marth FSmash (L1284) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3396 | 0x0D44 | `MARTH_U_SMASH` | 1279 | Marth USmash (L1285) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3397 | 0x0D45 | `MARTH_D_SMASH` | 1280 | Marth DSmash (L1286) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3398 | 0x0D46 | `MARTH_ATTACK_AIR_N` | 1281 | Marth AttackAirN (L1287) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3399 | 0x0D47 | `MARTH_ATTACK_AIR_F` | 1282 | Marth AttackAirF (L1288) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3400 | 0x0D48 | `MARTH_ATTACK_AIR_B` | 1283 | Marth AttackAirB (L1289) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3401 | 0x0D49 | `MARTH_ATTACK_AIR_U` | 1284 | Marth AttackAirU (L1290) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3402 | 0x0D4A | `MARTH_ATTACK_AIR_D` | 1285 | Marth AttackAirD (L1291) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3403 | 0x0D4B | `MARTH_LANDING_AIR_F` | 1286 | Marth LandingAirF (L1292) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3404 | 0x0D4C | `MARTH_LANDING_AIR_B` | 1287 | Marth LandingAirB (L1293) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3405 | 0x0D4D | `MARTH_ENTRY` | 1288 | Marth Entry (L1294) | `src/Marth/Marth.asm`, `src/Roy/Roy.asm` | 4 |
| 3406 | 0x0D4E | `MARTH_MENU_IDLE` | 1289 | Marth Menu Idle (L1295) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3407 | 0x0D4F | `MARTH_USP_AIR` | 1290 | Marth Up Special Air (L1296) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3408 | 0x0D50 | `MARTH_USP_GROUND` | 1291 | Marth Up Special Ground (L1297) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3409 | 0x0D51 | `MARTH_NSPG_1` | 1292 | Marth Neutral Special Ground 1 (L1298) | `src/Marth/Marth.asm`, `src/Roy/Roy.asm` | 2 |
| 3410 | 0x0D52 | `MARTH_NSPG_2_HI` | 1293 | Marth Neutral Special Ground 2 High (L1299) | `src/Marth/Marth.asm`, `src/Roy/Roy.asm` | 2 |
| 3411 | 0x0D53 | `MARTH_NSPG_2` | 1294 | Marth Neutral Special Ground 2 Mid (L1300) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3412 | 0x0D54 | `MARTH_NSPG_2_LO` | 1295 | Marth Neutral Special Ground 2 Low (L1301) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3413 | 0x0D55 | `MARTH_NSPG_3_HI` | 1296 | Marth Neutral Special Ground 3 High (L1302) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3414 | 0x0D56 | `MARTH_NSPG_3` | 1297 | Marth Neutral Special Ground 3 Mid (L1303) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3415 | 0x0D57 | `MARTH_NSPG_3_LO` | 1298 | Marth Neutral Special Ground 3 Low (L1304) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3416 | 0x0D58 | `MARTH_NSPA_1` | 1299 | Marth Neutral Special Air 1 (L1305) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3417 | 0x0D59 | `MARTH_NSPA_2_HI` | 1300 | Marth Neutral Special Air 2 High (L1306) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3418 | 0x0D5A | `MARTH_NSPA_2` | 1301 | Marth Neutral Special Air 2 Mid (L1307) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3419 | 0x0D5B | `MARTH_NSPA_2_LO` | 1302 | Marth Neutral Special Air 2 Low (L1308) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3420 | 0x0D5C | `MARTH_NSPA_3_HI` | 1303 | Marth Neutral Special Air 3 High (L1309) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3421 | 0x0D5D | `MARTH_NSPA_3` | 1304 | Marth Neutral Special Air 3 Mid (L1310) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3422 | 0x0D5E | `MARTH_NSPA_3_LO` | 1305 | Marth Neutral Special Air 3 Low (L1311) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3423 | 0x0D5F | `MARTH_VICTORY_1` | 1306 | Marth Victory 1 (L1312) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 4 |
| 3424 | 0x0D60 | `MARTH_VICTORY_2` | 1307 | Marth Victory 2 (L1313) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3425 | 0x0D61 | `MARTH_VICTORY_3` | 1308 | Marth Victory 3 (L1314) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3426 | 0x0D62 | `MARTH_CLAP` | 1309 | Marth Clap (L1315) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3427 | 0x0D63 | `MARTH_GAME_OVER` | 1310 | Marth GameOver (L1316) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3428 | 0x0D64 | `MARTH_GAME_CONTINUE` | 1311 | Marth Continue (L1317) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3429 | 0x0D65 | `MARTH_POSE_1P` | 1312 | Marth 1P Pose (L1318) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm` | 2 |
| 3430 | 0x0D66 | `KIRBY_MTWO_NSPG_BEGIN` | 1313 | Kirby Mewtwo Neutral Special Begin Ground (L1319) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3431 | 0x0D67 | `KIRBY_MTWO_NSPG_CHARGE` | 1314 | Kirby Mewtwo Neutral Special Charge Ground (L1320) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3432 | 0x0D68 | `KIRBY_MTWO_NSPG_SHOOT` | 1315 | Kirby Mewtwo Neutral Special Shoot Ground (L1321) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3433 | 0x0D69 | `KIRBY_MTWO_NSPA_BEGIN` | 1316 | Kirby Mewtwo Neutral Special Begin Air (L1322) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3434 | 0x0D6A | `KIRBY_MTWO_NSPA_CHARGE` | 1317 | Kirby Mewtwo Neutral Special Charge Air (L1323) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3435 | 0x0D6B | `KIRBY_MTWO_NSPA_SHOOT` | 1318 | Kirby Mewtwo Neutral Special Shoot Air (L1324) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3436 | 0x0D6C | `MTWO_USMASH_GRAPHIC` | 1319 | Mewtwo Usmash Graphics (L1325) | `src/Character.asm` | 2 |
| 3437 | 0x0D6D | `KIRBY_MARTH_SWORD` | 1320 | Kirby Marth Sword (L1326) |  | 0 |
| 3438 | 0x0D6E | `STG_MARTHTARGET_HEADER` | 1321 | Marth Break the Targets Header (L1327) |  | 0 |
| 3439 | 0x0D6F | `STG_MARTHTARGET` | 1322 | Marth Break the Targets Stage (L1328) |  | 0 |
| 3440 | 0x0D70 | `MARTH_COUNTER_G` | 1323 | Marth Counter Ground (L1329) | `src/Marth/Marth.asm` | 1 |
| 3441 | 0x0D71 | `MARTH_COUNTER_ATK_G` | 1324 | Marth Counter Attack Ground (L1330) | `src/Marth/Marth.asm` | 1 |
| 3442 | 0x0D72 | `MARTH_COUNTER_A` | 1325 | Marth Counter Air (L1331) | `src/Marth/Marth.asm` | 1 |
| 3443 | 0x0D73 | `MARTH_COUNTER_ATK_A` | 1326 | Marth Counter Attack Air (L1332) | `src/Marth/Marth.asm` | 1 |
| 3444 | 0x0D74 | `GBLAND_HEADER` | 1327 | Gameboy Land Header (L1333) |  | 0 |
| 3445 | 0x0D75 | `GBLAND_STAGE` | 1328 | Gameboy Land Stage (L1334) |  | 0 |
| 3446 | 0x0D76 | `STG_MTWOTARGET_HEADER` | 1329 | Mewtwo Break the Targets Header (L1335) |  | 0 |
| 3447 | 0x0D77 | `STG_MTWOTARGET` | 1330 | Mewtwo Break the Targest Stage (L1336) |  | 0 |
| 3448 | 0x0D78 | `STG_MARTHPLATFORM_HEADER` | 1331 | Marth Board the Platforms Header (L1337) |  | 0 |
| 3449 | 0x0D79 | `STG_MARTHPLATFORM` | 1332 | Marth Board the Platforms Stage (L1338) |  | 0 |
| 3450 | 0x0D7A | `STG_REST_HEADER` | 1333 | Rest Area Header (L1339) |  | 0 |
| 3451 | 0x0D7B | `STG_REST` | 1334 | Rest Area Stage (L1340) |  | 0 |
| 3452 | 0x0D7C | `STG_REST_BG` | 1335 | Rest Area BG (L1341) |  | 0 |
| 3453 | 0x0D7D | `STG_MTWOPLATFORM_HEADER` | 1336 | Mewtwo Board the Platforms Header (L1342) |  | 0 |
| 3454 | 0x0D7E | `STG_MTWOPLATFORM` | 1337 | Mewtwo Board the Platforms Stage (L1343) |  | 0 |
| 3455 | 0x0D7F | `STG_CSIEGE_HEADER` | 1338 | Castle Siege Header (L1344) |  | 0 |
| 3456 | 0x0D80 | `STG_CSIEGE` | 1339 | Castle Siege Stage (L1345) |  | 0 |
| 3457 | 0x0D81 | `STG_CSIEGE_BG` | 1340 | Castle Siege BG (L1346) |  | 0 |
| 3458 | 0x0D82 | `STG_YOSHI_ISLAND_II_HEADER` | 1341 | Yoshi's Island II Header (L1347) |  | 0 |
| 3459 | 0x0D83 | `STG_YOSHI_ISLAND_II` | 1342 | Yoshi's Island II Stage (L1348) |  | 0 |
| 3460 | 0x0D84 | `STG_YOSHI_ISLAND_II_BG` | 1343 | Yoshi's Island II BG (L1349) |  | 0 |
| 3461 | 0x0D85 | `STG_FINAL_DESTINATION_DL_HEADER` | 1344 | Final Destination DL Header (L1350) |  | 0 |
| 3462 | 0x0D86 | `STG_FINAL_DESTINATION_DL` | 1345 | Final Destination DL Stage (L1351) |  | 0 |
| 3463 | 0x0D87 | `STG_FINAL_DESTINATION_TENT_HEADER` | 1346 | Tent Final Destination Header (L1352) |  | 0 |
| 3464 | 0x0D88 | `STG_FINAL_DESTINATION_TENT` | 1347 | Tent Final Destination (L1353) |  | 0 |
| 3465 | 0x0D89 | `STG_COOLCOOL_REMIX_HEADER` | 1348 | Cool Cool Mountain Remix Header (L1354) |  | 0 |
| 3466 | 0x0D8A | `STG_COOLCOOL_REMIX` | 1349 | Cool Cool Mountain Remix Stage (L1355) |  | 0 |
| 3467 | 0x0D8B | `STOCK_ICONS` | 1350 | Stock Icons (L1356) | `src/Bonus.asm` | 1 |
| 3468 | 0x0D8C | `STG_DUEL_ZONE_DL_HEADER` | 1351 | Duel Zone DL Header (L1357) |  | 0 |
| 3469 | 0x0D8D | `STG_DUEL_ZONE_DL` | 1352 | Duel Zone DL Stage (L1358) |  | 0 |
| 3470 | 0x0D8E | `STG_COOLCOOL_DL_HEADER` | 1353 | Cool Cool Mountain DL Header (L1359) |  | 0 |
| 3471 | 0x0D8F | `STG_COOLCOOL_DL` | 1354 | Cool Cool Mountain DL Stage (L1360) |  | 0 |
| 3472 | 0x0D90 | `STG_META_CRYSTAL_DL_HEADER` | 1355 | Meta Crystal DL Header (L1361) |  | 0 |
| 3473 | 0x0D91 | `STG_META_CRYSTAL_DL` | 1356 | Meta Crystal DL Stage (L1362) |  | 0 |
| 3474 | 0x0D92 | `STG_DREAM_GREENS_HEADER` | 1357 | Dream Greens Header (L1363) |  | 0 |
| 3475 | 0x0D93 | `STG_DREAM_GREENS` | 1358 | Dream Greens Stage (L1364) |  | 0 |
| 3476 | 0x0D94 | `STG_DREAM_GREENS_IMAGES` | 1359 | Dream Greens Images (L1365) |  | 0 |
| 3477 | 0x0D95 | `STG_PEACHCASTLE_BETA_HEADER` | 1360 | Peach's Castle Beta Header (L1366) |  | 0 |
| 3478 | 0x0D96 | `STG_PEACHCASTLE_BETA` | 1361 | Peach's Castle Beta Stage (L1367) |  | 0 |
| 3479 | 0x0D97 | `KIRBY_WARIO_NSPR_G` | 1362 | Kirby Wario Neutral Special Recoil Ground (L1368) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3480 | 0x0D98 | `KIRBY_MARTH_NSPG_1` | 1363 | Kirby Marth Neutral Special Ground 1 (L1369) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3481 | 0x0D99 | `KIRBY_MARTH_NSPG_2_HI` | 1364 | Kirby Marth Neutral Special Ground 2 High (L1370) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3482 | 0x0D9A | `KIRBY_MARTH_NSPG_2` | 1365 | Kirby Marth Neutral Special Ground 2 Mid (L1371) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3483 | 0x0D9B | `KIRBY_MARTH_NSPG_2_LO` | 1366 | Kirby Marth Neutral Special Ground 2 Low (L1372) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3484 | 0x0D9C | `KIRBY_MARTH_NSPG_3_HI` | 1367 | Kirby Marth Neutral Special Ground 3 High (L1373) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3485 | 0x0D9D | `KIRBY_MARTH_NSPG_3` | 1368 | Kirby Marth Neutral Special Ground 3 Mid (L1374) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3486 | 0x0D9E | `KIRBY_MARTH_NSPG_3_LO` | 1369 | Kirby Marth Neutral Special Ground 3 Low (L1375) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3487 | 0x0D9F | `KIRBY_MARTH_NSPA_1` | 1370 | Kirby Marth Neutral Special Air 1 (L1376) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3488 | 0x0DA0 | `KIRBY_MARTH_NSPA_2_HI` | 1371 | Kirby Marth Neutral Special Air 2 High (L1377) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3489 | 0x0DA1 | `KIRBY_MARTH_NSPA_2` | 1372 | Kirby Marth Neutral Special Air 2 Mid (L1378) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3490 | 0x0DA2 | `KIRBY_MARTH_NSPA_2_LO` | 1373 | Kirby Marth Neutral Special Air 2 Low (L1379) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3491 | 0x0DA3 | `KIRBY_MARTH_NSPA_3_HI` | 1374 | Kirby Marth Neutral Special Air 3 High (L1380) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3492 | 0x0DA4 | `KIRBY_MARTH_NSPA_3` | 1375 | Kirby Marth Neutral Special Air 3 Mid (L1381) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3493 | 0x0DA5 | `KIRBY_MARTH_NSPA_3_LO` | 1376 | Kirby Marth Neutral Special Air 3 Low (L1382) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3494 | 0x0DA6 | `MARTH_POSE_1P_CPU` | 1377 | Marth 1p CPU Pose (L1383) | `src/Marth/Marth.asm`, `src/NMarth/NMarth.asm`, `src/Roy/Roy.asm` | 3 |
| 3495 | 0x0DA7 | `MARTH_ENTRY_EFFECTS` | 1378 | Marth Entry Special Effects (L1384) | `src/Character.asm` | 2 |
| 3496 | 0x0DA8 | `SONIC_MAIN` | 1379 | Sonic Main (L1385) | `src/Character.asm` | 1 |
| 3497 | 0x0DA9 | `SONIC_CHARACTER` | 1380 | Sonic Character (L1386) | `src/Character.asm` | 1 |
| 3498 | 0x0DAA | `SONIC_IDLE` | 1381 | Sonic Idle (L1387) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 14 |
| 3499 | 0x0DAB | `SONIC_RUN` | 1382 | Sonic Run (L1388) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3500 | 0x0DAC | `SONIC_RUN_BRAKE` | 1383 | Sonic Run Brake (L1389) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3501 | 0x0DAD | `SONIC_RUN_TURN` | 1384 | Sonic Turn Run (L1390) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3502 | 0x0DAE | `SONIC_JUMP_F` | 1385 | Sonic Jump F (L1391) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 13 |
| 3503 | 0x0DAF | `SONIC_JUMPSQUAT` | 1386 | Sonic Jumpsquat (L1392) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 18 |
| 3504 | 0x0DB0 | `SONIC_FAIR` | 1387 | Sonic Forward Air (L1393) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3505 | 0x0DB1 | `SONIC_DAIR` | 1388 | Sonic Down Air (L1394) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3506 | 0x0DB2 | `SONIC_FSMASH` | 1389 | Sonic Forward Smash Mid (L1395) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3507 | 0x0DB3 | `SONIC_FTILT` | 1390 | Sonic Forward Tilt Mid (L1396) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3508 | 0x0DB4 | `SONIC_JUMP_AF` | 1391 | Sonic Jump Air F (L1397) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3509 | 0x0DB5 | `SONIC_CSS` | 1392 | Sonic CSS (L1398) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 4 |
| 3510 | 0x0DB6 | `SONIC_JUMP_AB` | 1393 | Sonic Jump Air B (L1399) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3511 | 0x0DB7 | `SSONIC_ENTRY_RIGHT` | 1394 | Super Sonic Entry Right (L1400) | `src/SSonic/SSonic.asm` | 1 |
| 3512 | 0x0DB8 | `SONIC_FALL` | 1395 | Sonic Fall (L1401) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3513 | 0x0DB9 | `SONIC_FALL2` | 1396 | Sonic Fall2 (L1402) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3514 | 0x0DBA | `SONIC_JUMP_B` | 1397 | Sonic Jump B (L1403) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 5 |
| 3515 | 0x0DBB | `SONIC_FTILT_HIGH` | 1398 | Sonic Ftilt High (L1404) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3516 | 0x0DBC | `SONIC_FTILT_LOW` | 1399 | Sonic Ftilt Low (L1405) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3517 | 0x0DBD | `SONIC_UTILT` | 1400 | Sonic UTilt (L1406) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3518 | 0x0DBE | `SONIC_WALK1` | 1401 | Sonic Walk 1 (L1407) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3519 | 0x0DBF | `SONIC_WALK2` | 1402 | Sonic Walk 2 (L1408) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3520 | 0x0DC0 | `SONIC_WALK3` | 1403 | Sonic Walk 3 (L1409) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3521 | 0x0DC1 | `SONIC_CROUCH` | 1404 | Sonic Crouch (L1410) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3522 | 0x0DC2 | `SONIC_CROUCH_IDLE` | 1405 | Sonic Crouch Idle (L1411) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3523 | 0x0DC3 | `SONIC_CROUCH_END` | 1406 | Sonic Crouch End (L1412) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 7 |
| 3524 | 0x0DC4 | `SONIC_UAIR` | 1407 | Sonic Up Air (L1413) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3525 | 0x0DC5 | `SONIC_BAIR` | 1408 | Sonic Back Air (L1414) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3526 | 0x0DC6 | `SONIC_NAIR` | 1409 | Sonic Neutral Air (L1415) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3527 | 0x0DC7 | `SONIC_USMASH` | 1410 | Sonic Up Smash (L1416) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3528 | 0x0DC8 | `SONIC_TUMBLE` | 1411 | Sonic Tumble (L1417) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 30 |
| 3529 | 0x0DC9 | `SONIC_JAB1` | 1412 | Sonic Jab 1 (L1418) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3530 | 0x0DCA | `SONIC_JAB2` | 1413 | Sonic Jab 2 (L1419) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3531 | 0x0DCB | `SONIC_JAB3` | 1414 | Sonic Jab 3 (L1420) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3532 | 0x0DCC | `SONIC_ENTER_PIPE` | 1415 | Sonic Pipe Enter (L1421) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3533 | 0x0DCD | `SONIC_EXIT_PIPE` | 1416 | Sonic Pipe Exit (L1422) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3534 | 0x0DCE | `SONIC_SHIELD_DROP` | 1417 | Sonic Shield Drop (L1423) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 6 |
| 3535 | 0x0DCF | `SONIC_SPRING_HITBOX` | 1418 | Sonic Spring Hitbox (L1424) | `src/Character.asm`, `src/TagTeam.asm` | 5 |
| 3536 | 0x0DD0 | `SONIC_SPRING_GRAPHIC` | 1419 | Sonic Spring Graphic (L1425) | `src/Character.asm` | 3 |
| 3537 | 0x0DD1 | `SONIC_1P_POSE` | 1420 | Sonic 1p Pose (L1426) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3538 | 0x0DD2 | `SONIC_CLASSIC_1P_POSE` | 1421 | Sonic Classic 1p Pose (L1427) | `src/Sonic/Sonic.asm` | 1 |
| 3539 | 0x0DD3 | `SONIC_1P_CPU_POSE` | 1422 | Sonic 1p CPU Pose (L1428) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3540 | 0x0DD4 | `SONIC_STUN` | 1423 | Sonic Stun (L1429) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 6 |
| 3541 | 0x0DD5 | `SONIC_TAUNT` | 1424 | Sonic Taunt (L1430) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3542 | 0x0DD6 | `SONIC_VICTORY1` | 1425 | Sonic Victory 1 (L1431) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3543 | 0x0DD7 | `SONIC_VICTORY2` | 1426 | Sonic Victory 2 (L1432) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3544 | 0x0DD8 | `SONIC_DSMASH` | 1427 | Sonic Down Smash (L1433) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3545 | 0x0DD9 | `SONIC_DTILT` | 1428 | Sonic Down Tilt (L1434) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3546 | 0x0DDA | `SONIC_PUPPET_FALL` | 1429 | Sonic Puppet Fall (L1435) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3547 | 0x0DDB | `SONIC_PUPPET_UP` | 1430 | Sonic Puppet Up (L1436) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3548 | 0x0DDC | `SONIC_SPIN_LOOP` | 1431 | Sonic Spin Loop (L1437) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3549 | 0x0DDD | `SANDBAG_MAIN` | 1432 | Sandbag Main (L1438) | `src/Character.asm` | 1 |
| 3550 | 0x0DDE | `SANDBAG_CHARACTER` | 1433 | Sandbag Character (L1439) |  | 0 |
| 3551 | 0x0DDF | `SANDBAG_FILE1` | 1434 | Sandbag File 1 - UNUSED (L1440) |  | 0 |
| 3552 | 0x0DE0 | `SANDBAG_FILE2` | 1435 | Sandbag File 2 - UNUSED (L1441) |  | 0 |
| 3553 | 0x0DE1 | `SANDBAG_IDLE` | 1436 | Sandbag Idle (L1442) | `src/Sandbag/Sandbag.asm` | 24 |
| 3554 | 0x0DE2 | `SANDBAG_FALL` | 1437 | Sandbag Fall (L1443) | `src/Sandbag/Sandbag.asm` | 2 |
| 3555 | 0x0DE3 | `SANDBAG_DAMAGE_LOW_1` | 1438 | Sandbag Damage Low 1 (L1444) | `src/Sandbag/Sandbag.asm` | 1 |
| 3556 | 0x0DE4 | `SANDBAG_DAMAGE_LOW_2` | 1439 | Sandbag Damage Low 2 (L1445) | `src/Sandbag/Sandbag.asm` | 3 |
| 3557 | 0x0DE5 | `SANDBAG_DAMAGE_LOW_3` | 1440 | Sandbag Damage Low 3 (L1446) | `src/Sandbag/Sandbag.asm` | 1 |
| 3558 | 0x0DE6 | `SANDBAG_DAMAGE_MID_1` | 1441 | Sandbag Damage Mid 1 (L1447) | `src/Sandbag/Sandbag.asm` | 1 |
| 3559 | 0x0DE7 | `SANDBAG_DAMAGE_MID_2` | 1442 | Sandbag Damage Mid 2 (L1448) | `src/Sandbag/Sandbag.asm` | 1 |
| 3560 | 0x0DE8 | `SANDBAG_DAMAGE_MID_3` | 1443 | Sandbag Damage Mid 3 (L1449) | `src/Sandbag/Sandbag.asm` | 1 |
| 3561 | 0x0DE9 | `SANDBAG_DAMAGE_HIGH_1` | 1444 | Sandbag Damage High 1 (L1450) | `src/Sandbag/Sandbag.asm` | 1 |
| 3562 | 0x0DEA | `SANDBAG_DAMAGE_HIGH_2` | 1445 | Sandbag Damage High 2 (L1451) | `src/Sandbag/Sandbag.asm` | 1 |
| 3563 | 0x0DEB | `SANDBAG_DAMAGE_HIGH_3` | 1446 | Sandbag Damage High 3 (L1452) | `src/Sandbag/Sandbag.asm` | 2 |
| 3564 | 0x0DEC | `STG_HYRULECASLTE_REMIX_HEADER` | 1447 | Hyrule Castle Remix Stage Header (L1453) |  | 0 |
| 3565 | 0x0DED | `STG_HYRULECASTLE_REMIX` | 1448 | Hyrule Castle Remix Stage (L1454) |  | 0 |
| 3566 | 0x0DEE | `STG_SECTOR_Z_REMIX_HEADER` | 1449 | Sector Z Remix Stage Header (L1455) |  | 0 |
| 3567 | 0x0DEF | `SONIC_DOWNSTANDD` | 1450 | Sonic Down Stand Down (L1456) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 9 |
| 3568 | 0x0DF0 | `SONIC_DOWNSTANDU` | 1451 | Sonic Down Stand Up (L1457) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` +1 | 8 |
| 3569 | 0x0DF1 | `REMIX_CREDITS` | 1452 | Remix Credits (L1458) | `src/Credits.asm` | 1 |
| 3570 | 0x0DF2 | `SONIC_TECH` | 1453 | Sonic Tech (L1459) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3571 | 0x0DF3 | `SONIC_TECHF` | 1454 | Sonic Tech Forward (L1460) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3572 | 0x0DF4 | `SONIC_TECHB` | 1455 | Sonic Tech Back (L1461) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3573 | 0x0DF5 | `SONIC_SPIN_LOOP_FAST` | 1456 | Sonic Spin Loop Fast (L1462) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 7 |
| 3574 | 0x0DF6 | `SONIC_CHARGE_LOOP` | 1457 | Sonic Charge Loop (L1463) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 6 |
| 3575 | 0x0DF7 | `REGULAR_ANIMATION` | 1458 | REPLACE REGULAR ANIMATION (L1464) |  | 0 |
| 3576 | 0x0DF8 | `SONIC_DOWNATTACKU` | 1459 | Sonic Down Attack Up (L1465) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3577 | 0x0DF9 | `MUTE_CITY_HEADER` | 1460 | Mute City Header (L1466) |  | 0 |
| 3578 | 0x0DFA | `MUTE_CITY_STAGE` | 1461 | Mute City Stage (L1467) |  | 0 |
| 3579 | 0x0DFB | `HRC_HEADER` | 1462 | Home Run Contest Header (L1468) |  | 0 |
| 3580 | 0x0DFC | `HRC_STAGE` | 1463 | Home Run Contest Stage (L1469) |  | 0 |
| 3581 | 0x0DFD | `HRC_BG` | 1464 | Home Run Contest Background (L1470) |  | 0 |
| 3582 | 0x0DFE | `MK_REMIX_HEADER` | 1465 | Mushroom Kingdom Remix Header (L1471) |  | 0 |
| 3583 | 0x0DFF | `MK_REMIX_STAGE` | 1466 | Mushroom Kingdom Remix Stage (L1472) |  | 0 |
| 3584 | 0x0E00 | `SONIC_NSP_FINISH` | 1467 | Sonic Neutral Special Finish (L1473) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3585 | 0x0E01 | `SONIC_USP_SPRING` | 1468 | Sonic Up Special Spring (L1474) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3586 | 0x0E02 | `SONIC_SFALL` | 1469 | Sonic Special Fall (L1475) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3587 | 0x0E03 | `SONIC_DASH_ATTACK` | 1470 | Sonic Dash Attack (L1476) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3588 | 0x0E04 | `SONIC_DOWNBACKU` | 1471 | Sonic Down Back Up (L1477) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3589 | 0x0E05 | `SONIC_CLIFF_CATCH` | 1472 | Sonic Cliff Catch (L1478) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3590 | 0x0E06 | `SONIC_CLIFF_WAIT` | 1473 | Sonic Cliff Wait (L1479) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3591 | 0x0E07 | `SANDBAG_DAMAGE_AIR_1` | 1474 | Sandbag Damage Air 1 (L1480) | `src/Sandbag/Sandbag.asm` | 1 |
| 3592 | 0x0E08 | `SANDBAG_DAMAGE_AIR_2` | 1475 | Sandbag Damage Air 2 (L1481) | `src/Sandbag/Sandbag.asm` | 1 |
| 3593 | 0x0E09 | `SANDBAG_DAMAGE_AIR_3` | 1476 | Sandbag Damage Air 3 (L1482) | `src/Sandbag/Sandbag.asm` | 1 |
| 3594 | 0x0E0A | `SANDBAG_DAMAGE_FLY_HIGH` | 1477 | Sandbag Damage Fly High (L1483) | `src/Sandbag/Sandbag.asm` | 1 |
| 3595 | 0x0E0B | `SANDBAG_DAMAGE_FLY_MID` | 1478 | Sandbag Damage Fly Mid (L1484) | `src/Sandbag/Sandbag.asm` | 1 |
| 3596 | 0x0E0C | `SANDBAG_DAMAGE_FLY_LOW` | 1479 | Sandbag Damage Fly Low (L1485) | `src/Sandbag/Sandbag.asm` | 1 |
| 3597 | 0x0E0D | `SANDBAG_DAMAGE_FLY_ROLL` | 1480 | Sandbag Damage Fly Roll (L1486) | `src/Sandbag/Sandbag.asm` | 1 |
| 3598 | 0x0E0E | `SANDBAG_DAMAGE_FLY_TOP` | 1481 | Sandbag Damage Fly Top (L1487) | `src/Sandbag/Sandbag.asm` | 2 |
| 3599 | 0x0E0F | `SANDBAG_DOWN_BOUNCE_D` | 1482 | Sandbag Down Bounce Down (L1488) | `src/Sandbag/Sandbag.asm` | 3 |
| 3600 | 0x0E10 | `SANDBAG_DOWN_BOUNCE_U` | 1483 | Sandbag Down Bounce Up (L1489) | `src/Sandbag/Sandbag.asm` | 2 |
| 3601 | 0x0E11 | `SANDBAG_DOWN_STAND_D` | 1484 | Sandbag Down Stand Down (L1490) | `src/Sandbag/Sandbag.asm` | 3 |
| 3602 | 0x0E12 | `SANDBAG_DOWN_STAND_U` | 1485 | Sandbag Down Stand Up (L1491) | `src/Sandbag/Sandbag.asm` | 2 |
| 3603 | 0x0E13 | `SANDBAG_TUMBLE` | 1486 | Sandbag Tumble (L1492) | `src/Sandbag/Sandbag.asm` | 10 |
| 3604 | 0x0E14 | `SANDBAG_CAPTURED` | 1487 | Sandbag Captured (L1493) | `src/Sandbag/Sandbag.asm` | 2 |
| 3605 | 0x0E15 | `SONIC_CLIFF_QUICK` | 1488 | Sonic CliffQuick (L1494) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3606 | 0x0E16 | `SONIC_CLIFF_CLIMB_QUICK1` | 1489 | Sonic CliffClimbQuick1 (L1495) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 9 |
| 3607 | 0x0E17 | `SONIC_CLIFF_CLIMB_QUICK2` | 1490 | Sonic CliffClimbQuick2 (L1496) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3608 | 0x0E18 | `MK_REMIX_IMAGE_FILE` | 1491 | Mushroom Kingdom Remix Image 2 (L1497) |  | 0 |
| 3609 | 0x0E19 | `GHZ_HEADER` | 1492 | Green Hill Zone Header (L1498) |  | 0 |
| 3610 | 0x0E1A | `GHZ_STAGE` | 1493 | Green Hill Zone Stage (L1499) |  | 0 |
| 3611 | 0x0E1B | `GHZ_BG` | 1494 | Green Hill Zone Background (L1500) |  | 0 |
| 3612 | 0x0E1C | `SANDBAG_HEADSTAND` | 1495 | Sandbag Headstand (L1501) | `src/Sandbag/Sandbag.asm` | 1 |
| 3613 | 0x0E1D | `SONIC_CLIFF_ATTACK_QUICK2` | 1496 | Sonic Cliff Attack Quick 2 (L1502) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3614 | 0x0E1E | `SUBCON_HEADER` | 1497 | Subcon Header (L1503) |  | 0 |
| 3615 | 0x0E1F | `SUBCON_STAGE` | 1498 | Subcon Stage (L1504) |  | 0 |
| 3616 | 0x0E20 | `SUBCON_BG` | 1499 | Subcon Background (L1505) |  | 0 |
| 3617 | 0x0E21 | `SONIC_CLIFF_ESCAPE_QUICK2` | 1500 | Sonic Cliff Escape Quick 2 (L1506) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3618 | 0x0E22 | `SONIC_SHIELD_POSE` | 1501 | Sonic Shield Pose (L1507) | `src/Character.asm` | 3 |
| 3619 | 0x0E23 | `PIRATE_LAND_HEADER` | 1502 | Pirate Land Header (L1508) |  | 0 |
| 3620 | 0x0E24 | `PIRATE_LAND_STAGE` | 1503 | Pirate Land Stage (L1509) |  | 0 |
| 3621 | 0x0E25 | `PIRATE_LAND_BG` | 1504 | Pirate Land Background (L1510) |  | 0 |
| 3622 | 0x0E26 | `PIRATE_LAND_CANNON` | 1505 | Pirate Land Cannon (L1511) |  | 0 |
| 3623 | 0x0E27 | `PIRATE_LAND_CANNONBALL_HITBOX` | 1506 | Pirate Land Cannonball Hitbox (L1512) |  | 0 |
| 3624 | 0x0E28 | `PIRATE_LAND_CANNONBALL_GRAPHIC` | 1507 | Pirate Land Cannonball Graphic (L1513) |  | 0 |
| 3625 | 0x0E29 | `CASINO_NIGHT_HEADER` | 1508 | Casino Night Header (L1514) |  | 0 |
| 3626 | 0x0E2A | `CASINO_NIGHT_STAGE` | 1509 | Casino Night Stage (L1515) |  | 0 |
| 3627 | 0x0E2B | `CASINO_NIGHT_BG` | 1510 | Casino Night Background (L1516) |  | 0 |
| 3628 | 0x0E2C | `CASINO_NIGHT_BUMPER` | 1511 | Casino Night Bumper (L1517) |  | 0 |
| 3629 | 0x0E2D | `SONIC_CLIFF_CLIMB_SLOW1` | 1512 | Sonic CliffClimbSlow1 (L1518) |  | 0 |
| 3630 | 0x0E2E | `SONIC_CLIFF_CLIMB_SLOW2` | 1513 | Sonic CliffClimbSlow2 (L1519) |  | 0 |
| 3631 | 0x0E2F | `SONIC_CLIFF_SLOW` | 1514 | Sonic CliffSlow (L1520) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3632 | 0x0E30 | `SONIC_CLIFF_ESCAPE_SLOW2` | 1515 | Sonic CliffEscapeSlow2 (L1521) |  | 0 |
| 3633 | 0x0E31 | `SONIC_CLIFF_ATTACK_SLOW2` | 1516 | Sonic CliffAttackSlow2 (L1522) |  | 0 |
| 3634 | 0x0E32 | `SSONIC_MAIN` | 1517 | Super Sonic Main (L1523) | `src/Character.asm` | 1 |
| 3635 | 0x0E33 | `SSONIC_CHARACTER` | 1518 | Super Sonic Character (L1524) | `src/Character.asm` | 1 |
| 3636 | 0x0E34 | `SSONIC_SHIELD_POSE` | 1519 | Super Sonic Shield Pose (L1525) |  | 0 |
| 3637 | 0x0E35 | `SSONIC_RUN` | 1520 | Super Sonic Run (L1526) | `src/SSonic/SSonic.asm` | 1 |
| 3638 | 0x0E36 | `SSONIC_1P_CPU_POSE` | 1521 | Super Sonic 1p CPU Pose (L1527) | `src/SSonic/SSonic.asm` | 1 |
| 3639 | 0x0E37 | `SSONIC_DASH` | 1522 | Super Sonic Dash (L1528) | `src/SSonic/SSonic.asm` | 1 |
| 3640 | 0x0E38 | `CSONIC_MAIN` | 1523 | Classic Sonic Main (L1529) | `src/Character.asm` | 2 |
| 3641 | 0x0E39 | `CSONIC_CHARACTER` | 1524 | Classic Sonic Character (L1530) |  | 0 |
| 3642 | 0x0E3A | `CSONIC_SHIELD_POSE` | 1525 | Classic Sonic Shield Pose (L1531) |  | 0 |
| 3643 | 0x0E3B | `DRAGONKING_1P_POSE` | 1526 | Dragon King 1p Pose (L1532) | `src/DragonKing/DragonKing.asm` | 1 |
| 3644 | 0x0E3C | `CSONIC_VICTORY1` | 1527 | Classic Sonic Victory 1 (L1533) | `src/Sonic/Sonic.asm` | 1 |
| 3645 | 0x0E3D | `SSONIC_VICTORY1` | 1528 | Super Sonic Victory 1 (L1534) | `src/SSonic/SSonic.asm` | 1 |
| 3646 | 0x0E3E | `SONIC_THROWB` | 1529 | Sonic Back Throw (L1535) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3647 | 0x0E3F | `STG_SONICTARGET_HEADER` | 1530 | Sonic Break the Targets Header (L1536) |  | 0 |
| 3648 | 0x0E40 | `STG_SONICTARGET` | 1531 | Sonic Break the Targets Stage (L1537) |  | 0 |
| 3649 | 0x0E41 | `SSONIC_IDLE` | 1532 | Super Sonic Idle (L1538) | `src/SSonic/SSonic.asm` | 6 |
| 3650 | 0x0E42 | `SONIC_NSP_RECOIL` | 1533 | Sonic Neutral Special Recoil (L1539) |  | 0 |
| 3651 | 0x0E43 | `STG_SONICPLATFORM_HEADER` | 1534 | Sonic Board the Platforms Header (L1540) |  | 0 |
| 3652 | 0x0E44 | `STG_SONICPLATFORM` | 1535 | Sonic Board the Platforms Stage (L1541) |  | 0 |
| 3653 | 0x0E45 | `SONIC_ENTRY` | 1536 | Sonic Entry (L1542) | `src/Character.asm` | 3 |
| 3654 | 0x0E46 | `SONIC_ENTRY_RIGHT` | 1537 | Sonic Entry Right (L1543) | `src/Sonic/Sonic.asm` | 1 |
| 3655 | 0x0E47 | `SONIC_ENTRY_LEFT` | 1538 | Sonic Entry Left (L1544) | `src/Sonic/Sonic.asm` | 1 |
| 3656 | 0x0E48 | `SONIC_TEETER_START` | 1539 | Sonic Teeter Start (L1545) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3657 | 0x0E49 | `SONIC_TEETER` | 1540 | Sonic Teeter (L1546) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3658 | 0x0E4A | `SSONIC_TEETER_START` | 1541 | Super Sonic Teeter Start (L1547) | `src/SSonic/SSonic.asm` | 1 |
| 3659 | 0x0E4B | `SSONIC_TEETER` | 1542 | Super Sonic Teeter (L1548) | `src/SSonic/SSonic.asm` | 1 |
| 3660 | 0x0E4C | `SONIC_CLAP` | 1543 | Super Sonic Clap (L1549) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3661 | 0x0E4D | `SONIC_TEAM_POSE` | 1544 | Sonic Team Pose (L1550) | `src/SinglePlayerModes.asm` | 6 |
| 3662 | 0x0E4E | `METALLIC_MADNESS_HEADER` | 1545 | Metallic Madness Header (L1551) |  | 0 |
| 3663 | 0x0E4F | `METALLIC_MADNESS_STAGE` | 1546 | Metallic Madness Stage (L1552) |  | 0 |
| 3664 | 0x0E50 | `METALLIC_MADNESS_BG` | 1547 | Metallic Madness Background (L1553) |  | 0 |
| 3665 | 0x0E51 | `ROBOT_BEE` | 1548 | Robot Bee (L1554) |  | 0 |
| 3666 | 0x0E52 | `SONIC_DOWNFORWARDD` | 1549 | Sonic Down Forward Down (L1555) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3667 | 0x0E53 | `SONIC_DOWNFORWARDU` | 1550 | Sonic Down Forward Up (L1556) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3668 | 0x0E54 | `SONIC_NSP_GROUND_RECOIL` | 1551 | Sonic NSP Ground Recoil (L1557) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3669 | 0x0E55 | `SONIC_NSP_GROUND_END` | 1552 | Sonic NSP Ground End (L1558) | `src/NSonic/NSonic.asm`, `src/Sonic/Sonic.asm` | 2 |
| 3670 | 0x0E56 | `ROBOT_BEE_LASER_HITBOX` | 1553 | Robot Bee Laser Hitbox (L1559) |  | 0 |
| 3671 | 0x0E57 | `ROBOT_BEE_LASER_GRAPHICS` | 1554 | Robot Bee Laser Graphics (L1560) |  | 0 |
| 3672 | 0x0E58 | `SONIC_FSMASH_HIGH` | 1555 | Sonic Forward Smash High (L1561) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3673 | 0x0E59 | `SONIC_FSMASH_MID_HIGH` | 1556 | Sonic Forward Smash Mid High (L1562) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3674 | 0x0E5A | `SONIC_FSMASH_MID_LOW` | 1557 | Sonic Forward Smash Mid Low (L1563) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3675 | 0x0E5B | `SONIC_FSMASH_LOW` | 1558 | Sonic Forward Smash Low (L1564) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 3 |
| 3676 | 0x0E5C | `KIRBY_SONIC_NSP_LOOP` | 1559 | Kirby Sonic Neutral Special Loop (L1565) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 6 |
| 3677 | 0x0E5D | `KIRBY_SONIC_NSPA_END_F` | 1560 | Kirby Sonic Neutral Special Air End F (L1566) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 3678 | 0x0E5E | `KIRBY_SONIC_NSPA_END_B` | 1561 | Kirby Sonic Neutral Special Air End B (L1567) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3679 | 0x0E5F | `KIRBY_SONIC_NSPG_END_F` | 1562 | Kirby Sonic Neutral Special Ground End F (L1568) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3680 | 0x0E60 | `KIRBY_SONIC_NSPG_END_B` | 1563 | Kirby Sonic Neutral Special Ground End B (L1569) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3681 | 0x0E61 | `SSONIC_VICTORY2` | 1564 | Super Sonic Victory 2 (L1570) | `src/SSonic/SSonic.asm` | 1 |
| 3682 | 0x0E62 | `SSONIC_CSS` | 1565 | Super Sonic CSS (L1571) | `src/SSonic/SSonic.asm` | 2 |
| 3683 | 0x0E63 | `RAINBOW_ROAD_HEADER` | 1566 | Rainbow Road Header (L1572) |  | 0 |
| 3684 | 0x0E64 | `RAINBOW_ROAD_STAGE` | 1567 | Rainbow Road Stage (L1573) |  | 0 |
| 3685 | 0x0E65 | `CRATERIA_RAIN_HITBOX` | 1568 | Crateria Acid Rain Hitbox (L1574) |  | 0 |
| 3686 | 0x0E66 | `CRATERIA_RAIN_GRAPHIC` | 1569 | Crateria Acid Rain Graphic (L1575) |  | 0 |
| 3687 | 0x0E67 | `RAINBOW_ROAD_CHOMP` | 1570 | Rainbow Road Chain Chomp (L1576) |  | 0 |
| 3688 | 0x0E68 | `RAINBOW_ROAD_CHOMP_HITBOX` | 1571 | Rainbow Road Chain Chomp Hitbox (L1577) |  | 0 |
| 3689 | 0x0E69 | `STG_PS2_HEADER` | 1572 | Pokemon Stadium 2 Header (L1578) |  | 0 |
| 3690 | 0x0E6A | `STG_PS2` | 1573 | Pokemon Stadium 2 Stage (L1579) |  | 0 |
| 3691 | 0x0E6B | `STG_PS2_BG` | 1574 | Pokemon Stadium 2 BG (L1580) |  | 0 |
| 3692 | 0x0E6C | `DRM_VICTORY_1` | 1575 | Dr. Mario Victory 1 (L1581) | `src/DrMario/DrMario.asm`, `src/NDrMario/NDrMario.asm` | 2 |
| 3693 | 0x0E6D | `SHEIK_MAIN` | 1576 | Sheik Main (L1582) | `src/Character.asm` | 1 |
| 3694 | 0x0E6E | `SHEIK_CHARACTER` | 1577 | Sheik Character (L1583) | `src/Character.asm` | 1 |
| 3695 | 0x0E6F | `SHEIK_PROJECTILE_HITBOX` | 1578 | Sheik Projectile Hitbox (L1584) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 3696 | 0x0E70 | `SHEIK_NEEDLE` | 1579 | Sheik Needle (L1585) |  | 0 |
| 3697 | 0x0E71 | `SHEIK_USPG_BEGIN` | 1580 | Sheik Up Special Ground Begin (L1586) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3698 | 0x0E72 | `SHEIK_USPG_END` | 1581 | Sheik Up Special Ground End (L1587) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3699 | 0x0E73 | `SHEIK_USPA_BEGIN` | 1582 | Sheik Up Special Air Begin (L1588) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3700 | 0x0E74 | `SHEIK_USPA_END` | 1583 | Sheik Up Special Air End (L1589) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3701 | 0x0E75 | `SHEIK_DSP_BEGIN` | 1584 | Sheik Down Special Begin (L1590) | `src/Sheik/Sheik.asm` | 1 |
| 3702 | 0x0E76 | `SHEIK_DSP_ATTACK` | 1585 | Sheik Down Special Attack (L1591) | `src/Sheik/Sheik.asm` | 1 |
| 3703 | 0x0E77 | `SHEIK_DSP_LANDING` | 1586 | Sheik Down Special Landing (L1592) | `src/Sheik/Sheik.asm` | 1 |
| 3704 | 0x0E78 | `SHEIK_DSP_RECOIL` | 1587 | Sheik Down Special Recoil (L1593) | `src/Sheik/Sheik.asm` | 1 |
| 3705 | 0x0E79 | `SHEIK_NSPG_BEGIN` | 1588 | Sheik Neutral Special Ground Begin (L1594) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3706 | 0x0E7A | `SHEIK_NSPG_CHARGE` | 1589 | Sheik Neutral Special Ground Charge (L1595) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3707 | 0x0E7B | `SHEIK_NSPG_SHOOT` | 1590 | Sheik Neutral Special Ground Shoot (L1596) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3708 | 0x0E7C | `SHEIK_APPEAR` | 1591 | Sheik Appear (L1597) |  | 0 |
| 3709 | 0x0E7D | `SHEIK_ATTACK_AIR_B` | 1592 | Sheik Attack air b (L1598) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3710 | 0x0E7E | `SHEIK_ATTACK_AIR_D` | 1593 | Sheik Attack air d (L1599) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3711 | 0x0E7F | `SHEIK_ATTACK_AIR_F` | 1594 | Sheik Attack air f (L1600) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3712 | 0x0E80 | `SHEIK_ATTACK_AIR_N` | 1595 | Sheik Attack air n (L1601) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3713 | 0x0E81 | `SHEIK_ATTACK_AIR_U` | 1596 | Sheik Attack air u (L1602) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3714 | 0x0E82 | `SHEIK_CROUCH` | 1597 | Sheik Crouch (L1603) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3715 | 0x0E83 | `SHEIK_CROUCH_END` | 1598 | Sheik Crouch end (L1604) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3716 | 0x0E84 | `SHEIK_CROUCH_IDLE` | 1599 | Sheik Crouch idle (L1605) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3717 | 0x0E85 | `SHEIK_DASH` | 1600 | Sheik Dash (L1606) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3718 | 0x0E86 | `SHEIK_DASH_ATTACK` | 1601 | Sheik Dash Attack (L1607) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3719 | 0x0E87 | `SHEIK_D_SMASH` | 1602 | Sheik Down Smash (L1608) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3720 | 0x0E88 | `SHEIK_D_TILT` | 1603 | Sheik Down Tilt (L1609) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3721 | 0x0E89 | `SHEIK_FALL` | 1604 | Sheik Fall (L1610) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3722 | 0x0E8A | `SHEIK_FALL_AERIAL` | 1605 | Sheik Fall Air (L1611) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3723 | 0x0E8B | `SHEIK_F_SMASH` | 1606 | Sheik Fsmash (L1612) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3724 | 0x0E8C | `SHEIK_F_SMASH_HIGH` | 1607 | Sheik Fsmash high (L1613) |  | 0 |
| 3725 | 0x0E8D | `SHEIK_F_SMASH_LOW` | 1608 | Sheik Fsmash low (L1614) |  | 0 |
| 3726 | 0x0E8E | `SHEIK_F_TILT_HIGH` | 1609 | Sheik Ftilt high (L1615) |  | 0 |
| 3727 | 0x0E8F | `SHEIK_F_TILT_LOW` | 1610 | Sheik Ftilt low (L1616) |  | 0 |
| 3728 | 0x0E90 | `SHEIK_F_TILT_MID_HIGH` | 1611 | Sheik Ftilt mid high (L1617) |  | 0 |
| 3729 | 0x0E91 | `SHEIK_F_TILT_MID_LOW` | 1612 | Sheik Ftilt mid low (L1618) |  | 0 |
| 3730 | 0x0E92 | `SHEIK_IDLE` | 1613 | Sheik Idle (L1619) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 14 |
| 3731 | 0x0E93 | `SHEIK_JAB_LOOP` | 1614 | Sheik Jab loop (L1620) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3732 | 0x0E94 | `SHEIK_JAB_LOOP_END` | 1615 | Sheik Jab loop End (L1621) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3733 | 0x0E95 | `SHEIK_JAB_LOOP_START` | 1616 | Sheik Jab loop Start (L1622) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3734 | 0x0E96 | `SHEIK_JAB_1` | 1617 | Sheik Jab1 (L1623) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3735 | 0x0E97 | `SHEIK_JAB_2` | 1618 | Sheik Jab2 (L1624) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3736 | 0x0E98 | `SHEIK_JUMP_AERIAL_B` | 1619 | Sheik Jump air B (L1625) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3737 | 0x0E99 | `SHEIK_JUMP_AERIAL_F` | 1620 | Sheik Jump air F (L1626) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3738 | 0x0E9A | `SHEIK_JUMP_B` | 1621 | Sheik Jump B (L1627) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3739 | 0x0E9B | `SHEIK_JUMP_F` | 1622 | Sheik Jump F (L1628) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3740 | 0x0E9C | `SHEIK_CLAP` | 1623 | Sheik Lose (L1629) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3741 | 0x0E9D | `SHEIK_RUN` | 1624 | Sheik Run (L1630) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3742 | 0x0E9E | `SHEIK_RUN_BRAKE` | 1625 | Sheik Runbrake (L1631) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3743 | 0x0E9F | `SHEIK_SELECTED` | 1626 | Sheik Selected (L1632) |  | 0 |
| 3744 | 0x0EA0 | `SHEIK_TURN` | 1627 | Sheik Turn (L1633) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3745 | 0x0EA1 | `SHEIK_TURN_RUN` | 1628 | Sheik Turn Run (L1634) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3746 | 0x0EA2 | `SHEIK_U_SMASH` | 1629 | Sheik Usmash (L1635) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3747 | 0x0EA3 | `SHEIK_U_TILT` | 1630 | Sheik Utilt (L1636) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3748 | 0x0EA4 | `SHEIK_WALK_1` | 1631 | Sheik Walk1 (L1637) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3749 | 0x0EA5 | `SHEIK_WALK_2` | 1632 | Sheik Walk2 (L1638) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3750 | 0x0EA6 | `SHEIK_WALK_3` | 1633 | Sheik Walk3 (L1639) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3751 | 0x0EA7 | `SHEIK_SHIELD_POSE` | 1634 | Sheik Shield Pose (L1640) | `src/Character.asm` | 2 |
| 3752 | 0x0EA8 | `SHEIK_VICTORY_1` | 1635 | Sheik Victory 1 (L1641) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3753 | 0x0EA9 | `SHEIK_F_TILT` | 1636 | Sheik FTilt (L1642) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3754 | 0x0EAA | `SHEIK_GRAB` | 1637 | Sheik Catch (L1643) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3755 | 0x0EAB | `SHEIK_GRAB_PULL` | 1638 | Sheik Catch Pull (L1644) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3756 | 0x0EAC | `SHEIK_TAUNT` | 1639 | Sheik Taunt (L1645) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3757 | 0x0EAD | `SHEIK_THROW_B` | 1640 | Sheik Throw B (L1646) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3758 | 0x0EAE | `SHEIK_THROW_F` | 1641 | Sheik Throw F (L1647) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3759 | 0x0EAF | `SHEIK_DOWN_BACK_D` | 1642 | Sheik Down Back D (L1648) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3760 | 0x0EB0 | `SHEIK_DOWN_BACK_U` | 1643 | Sheik Down Back U (L1649) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3761 | 0x0EB1 | `SHEIK_DOWN_BOUNCE_D` | 1644 | Sheik Down Bounce D (L1650) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 6 |
| 3762 | 0x0EB2 | `SHEIK_DOWN_BOUNCE_U` | 1645 | Sheik Down Bounce U (L1651) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 4 |
| 3763 | 0x0EB3 | `SHEIK_DOWN_FORWARD_D` | 1646 | Sheik Down Forward D (L1652) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3764 | 0x0EB4 | `SHEIK_DOWN_FORWARD_U` | 1647 | Sheik Down Forward U (L1653) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3765 | 0x0EB5 | `SHEIK_DOWN_STAND_D` | 1648 | Sheik Down Stand D (L1654) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 6 |
| 3766 | 0x0EB6 | `SHEIK_DOWN_STAND_U` | 1649 | Sheik Down Stand U (L1655) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm`, `src/TwelveCharBattle.asm` | 5 |
| 3767 | 0x0EB7 | `SHEIK_FALL_SPECIAL` | 1650 | Sheik Fall Special (L1656) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3768 | 0x0EB8 | `SHEIK_STUN` | 1651 | Sheik Stun (L1657) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 4 |
| 3769 | 0x0EB9 | `SHEIK_TECH` | 1652 | Sheik Tech (L1658) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3770 | 0x0EBA | `SHEIK_TEETER` | 1653 | Sheik Teeter (L1659) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3771 | 0x0EBB | `SHEIK_TEETER_START` | 1654 | Sheik Teeter Start (L1660) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3772 | 0x0EBC | `SHEIK_DOWN_ATTACK_D` | 1655 | Sheik Down Attack D (L1661) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3773 | 0x0EBD | `SHEIK_DOWN_ATTACK_U` | 1656 | Sheik Down Attack U (L1662) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3774 | 0x0EBE | `STG_NORFAIR_REMIX_HEADER` | 1657 | Norfair Remix Header (L1663) |  | 0 |
| 3775 | 0x0EBF | `STG_NORFAIR_REMIX` | 1658 | Norfair Remix Stage (L1664) |  | 0 |
| 3776 | 0x0EC0 | `STG_TALTAL_REMIX_HEADER` | 1659 | Tal Tal Heights Remix Header (L1665) |  | 0 |
| 3777 | 0x0EC1 | `STG_TALTAL_REMIX` | 1660 | Tal Tal Heights Remix Stage (L1666) |  | 0 |
| 3778 | 0x0EC2 | `STG_TOADSTURNPIKE_HEADER` | 1661 | Toad's Turnpike Header (L1667) |  | 0 |
| 3779 | 0x0EC3 | `STG_TOADSTURNPIKE` | 1662 | Toad's Turnpike Stage (L1668) |  | 0 |
| 3780 | 0x0EC4 | `STG_TOADSTURNPIKE_BG` | 1663 | Toad's Turnpike BG (L1669) |  | 0 |
| 3781 | 0x0EC5 | `SHEIK_VICTORY_2` | 1664 | Sheik Victory 2 (L1670) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3782 | 0x0EC6 | `SHEIK_NSPA_BEGIN` | 1665 | Sheik Neutral Special Air Begin (L1671) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3783 | 0x0EC7 | `SHEIK_NSPA_CHARGE` | 1666 | Sheik Neutral Special Air Charge (L1672) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3784 | 0x0EC8 | `SHEIK_NSPA_SHOOT` | 1667 | Sheik Neutral Special Air Shoot (L1673) | `src/Sheik/Sheik.asm` | 1 |
| 3785 | 0x0EC9 | `SHEIK_ENTRY_RIGHT` | 1668 | Sheik Entry Right (L1674) | `src/Sheik/Sheik.asm` | 1 |
| 3786 | 0x0ECA | `SHEIK_ENTRY_LEFT` | 1669 | Sheik Entry Left (L1675) | `src/Sheik/Sheik.asm` | 1 |
| 3787 | 0x0ECB | `SHEIK_DAMAGE_HIGH_1` | 1670 | Sheik DamageHigh1 (L1676) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3788 | 0x0ECC | `SHEIK_DAMAGE_HIGH_2` | 1671 | Sheik DamageHigh2 (L1677) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3789 | 0x0ECD | `SHEIK_DAMAGE_HIGH_3` | 1672 | Sheik DamageHigh3 (L1678) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3790 | 0x0ECE | `SHEIK_DAMAGE_MID_1` | 1673 | Sheik DamageMid1 (L1679) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3791 | 0x0ECF | `SHEIK_DAMAGE_MID_2` | 1674 | Sheik DamageMid2 (L1680) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3792 | 0x0ED0 | `SHEIK_DAMAGE_MID_3` | 1675 | Sheik DamageMid3 (L1681) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3793 | 0x0ED1 | `SHEIK_DAMAGE_LOW_1` | 1676 | Sheik DamageLow1 (L1682) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3794 | 0x0ED2 | `SHEIK_DAMAGE_LOW_2` | 1677 | Sheik DamageLow2 (L1683) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3795 | 0x0ED3 | `SHEIK_DAMAGE_LOW_3` | 1678 | Sheik DamageLow3 (L1684) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3796 | 0x0ED4 | `SHEIK_DAMAGE_AIR_1` | 1679 | Sheik DamageAir1 (L1685) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3797 | 0x0ED5 | `SHEIK_DAMAGE_AIR_2` | 1680 | Sheik DamageAir2 (L1686) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3798 | 0x0ED6 | `SHEIK_DAMAGE_AIR_3` | 1681 | Sheik DamageAir3 (L1687) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3799 | 0x0ED7 | `SHEIK_DAMAGE_FLY_HIGH` | 1682 | Sheik DamageFlyHigh (L1688) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3800 | 0x0ED8 | `SHEIK_DAMAGE_FLY_MID` | 1683 | Sheik DamageFlyMid (L1689) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3801 | 0x0ED9 | `SHEIK_DAMAGE_FLY_LOW` | 1684 | Sheik DamageFlyLow (L1690) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3802 | 0x0EDA | `SHEIK_DAMAGE_FLY_TOP` | 1685 | Sheik DamageFlyTop (L1691) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3803 | 0x0EDB | `SHEIK_DAMAGE_FLY_ROLL` | 1686 | Sheik DamageFlyRoll (L1692) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3804 | 0x0EDC | `SHEIK_TUMBLE` | 1687 | Sheik Tumble (L1693) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 20 |
| 3805 | 0x0EDD | `STG_SHEIKPLATFORM_HEADER` | 1688 | Sheik Board the Platforms Header (L1694) |  | 0 |
| 3806 | 0x0EDE | `STG_SHEIKPLATFORM_STAGE` | 1689 | Sheik Board the Platforms Stage (L1695) |  | 0 |
| 3807 | 0x0EDF | `STG_WINTERDL_HEADER` | 1690 | Winter Dreamland Header (L1696) |  | 0 |
| 3808 | 0x0EE0 | `STG_WINTERDL_STAGE` | 1691 | Winter Dreamland Stage (L1697) |  | 0 |
| 3809 | 0x0EE1 | `SHEIK_SHIELD_OFF` | 1692 | Sheik Shield Off (L1698) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3810 | 0x0EE2 | `SHEIK_SHIELD_ON` | 1693 | Sheik Shield On (L1699) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3811 | 0x0EE3 | `SHEIK_VICTORY_3` | 1694 | Sheik Victory 3 (L1700) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 4 |
| 3812 | 0x0EE4 | `SHEIK_1P_CPU` | 1695 | Sheik 1P CPU Pose (L1701) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3813 | 0x0EE5 | `SHEIK_1P` | 1696 | Sheik 1P Pose (L1702) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3814 | 0x0EE6 | `SHEIK_PUPPET_FALL` | 1697 | Sheik Puppet Fall (L1703) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3815 | 0x0EE7 | `SHEIK_PUPPET_UP` | 1698 | Sheik Puppet Up (L1704) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3816 | 0x0EE8 | `MTWO_VICTORY_IMAGE_BOTTOM` | 1699 | Mewtwo Ending Image 1 (L1705) | `src/SinglePlayer.asm` | 1 |
| 3817 | 0x0EE9 | `MTWO_VICTORY_IMAGE_TOP` | 1700 | Mewtwo Ending Image 2 (L1706) |  | 0 |
| 3818 | 0x0EEA | `KIRBY_SHEIK_NSPG_START` | 1701 | Kirby Sheik Neutral Special Ground Start (L1707) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3819 | 0x0EEB | `KIRBY_SHEIK_NSPG_CHARGE` | 1702 | Kirby Sheik Neutral Special Ground Charge (L1708) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3820 | 0x0EEC | `KIRBY_SHEIK_NSPG_SHOOT` | 1703 | Kirby Sheik Neutral Special Ground Shoot (L1709) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3821 | 0x0EED | `KIRBY_SHEIK_NSPA_START` | 1704 | Kirby Sheik Neutral Special Air Start (L1710) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3822 | 0x0EEE | `KIRBY_SHEIK_NSPA_CHARGE` | 1705 | Kirby Sheik Neutral Special Air Charge (L1711) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3823 | 0x0EEF | `KIRBY_SHEIK_NSPA_SHOOT` | 1706 | Kirby Sheik Neutral Special Air Shoot (L1712) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 3824 | 0x0EF0 | `STG_SHEIKTARGET_HEADER` | 1707 | Sheik Break the Targets Header (L1713) |  | 0 |
| 3825 | 0x0EF1 | `STG_SHEIKTARGET_STAGE` | 1708 | Sheik Break the Targets Stage (L1714) |  | 0 |
| 3826 | 0x0EF2 | `SHEIK_ITEM_SHOOT_AIR` | 1709 | Sheik Item Shoot Aerial (L1715) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 4 |
| 3827 | 0x0EF3 | `SHEIK_LANDING` | 1710 | Sheik Landing (L1716) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 16 |
| 3828 | 0x0EF4 | `ONETT_CAR` | 1711 | Onett Car (L1717) |  | 0 |
| 3829 | 0x0EF5 | `SHEIK_LANDING_AIR_F` | 1712 | Sheik Fair Landing (L1718) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3830 | 0x0EF6 | `SHEIK_LANDING_AIR_B` | 1713 | Sheik Bair Landing (L1719) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3831 | 0x0EF7 | `SHEIK_CLIFF_QUICK` | 1714 | Sheik Cliff Quick (L1720) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3832 | 0x0EF8 | `SHEIK_CLIFF_ATTACK_QUICK_1` | 1715 | Sheik Cliff Quick Attack 1 (L1721) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 4 |
| 3833 | 0x0EF9 | `SHEIK_CLIFF_ATTACK_QUICK_2` | 1716 | Sheik Cliff Quick Attack 2 (L1722) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3834 | 0x0EFA | `SHEIK_ENTER_PIPE` | 1717 | Sheik Enter Pipe (L1723) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3835 | 0x0EFB | `SHEIK_EXIT_PIPE` | 1718 | Sheik Exit Pipe (L1724) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3836 | 0x0EFC | `SHEIK_EXIT_PIPE_WALK` | 1719 | Sheik Exit Pipe Walk (L1725) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3837 | 0x0EFD | `SHEIK_CLIFF_SLOW` | 1720 | Sheik Cliff Slow (L1726) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3838 | 0x0EFE | `SHEIK_CLIFF_ATTACK_SLOW_1` | 1721 | Sheik Cliff Slow Attack 1 (L1727) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 6 |
| 3839 | 0x0EFF | `SHEIK_CLIFF_ATTACK_SLOW_2` | 1722 | Sheik Cliff Slow Attack 2 (L1728) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3840 | 0x0F00 | `SHEIK_CLIFF_CLIMB_QUICK_2` | 1723 | Sheik Cliff Climb Quick 2 (L1729) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3841 | 0x0F01 | `SHEIK_CLIFF_CLIMB_SLOW_2` | 1724 | Sheik Cliff Climb Slow 2 (L1730) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3842 | 0x0F02 | `BOWSER_VICTORY_IMAGE_BOTTOM` | 1725 | Bowser Ending Image 1 (L1731) | `src/SinglePlayer.asm` | 2 |
| 3843 | 0x0F03 | `BOWSER_VICTORY_IMAGE_TOP` | 1726 | Bowser Ending Image 2 (L1732) |  | 0 |
| 3844 | 0x0F04 | `CONKER_VICTORY_IMAGE_BOTTOM` | 1727 | Conker Ending Image 1 (L1733) | `src/SinglePlayer.asm` | 1 |
| 3845 | 0x0F05 | `CONKER_VICTORY_IMAGE_TOP` | 1728 | Conker Ending Image 2 (L1734) |  | 0 |
| 3846 | 0x0F06 | `GANON_VICTORY_IMAGE_BOTTOM` | 1729 | Ganon Ending Image 1 (L1735) | `src/SinglePlayer.asm` | 1 |
| 3847 | 0x0F07 | `GANON_VICTORY_IMAGE_TOP` | 1730 | Ganon Ending Image 2 (L1736) |  | 0 |
| 3848 | 0x0F08 | `MARTH_VICTORY_IMAGE_BOTTOM` | 1731 | Marth Ending Image 1 (L1737) | `src/SinglePlayer.asm` | 1 |
| 3849 | 0x0F09 | `MARTH_VICTORY_IMAGE_TOP` | 1732 | Marth Ending Image 2 (L1738) |  | 0 |
| 3850 | 0x0F0A | `SHEIK_VICTORY_IMAGE_BOTTOM` | 1733 | Sheik Ending Image 1 (L1739) | `src/SinglePlayer.asm` | 1 |
| 3851 | 0x0F0B | `SHEIK_VICTORY_IMAGE_TOP` | 1734 | Sheik Ending Image 2 (L1740) |  | 0 |
| 3852 | 0x0F0C | `WARIO_VICTORY_IMAGE_BOTTOM` | 1735 | Wario Ending Image 1 (L1741) | `src/SinglePlayer.asm` | 1 |
| 3853 | 0x0F0D | `WARIO_VICTORY_IMAGE_TOP` | 1736 | Wario Ending Image 2 (L1742) |  | 0 |
| 3854 | 0x0F0E | `SONIC_VICTORY_IMAGE_BOTTOM` | 1737 | Sonic Ending Image 1 (L1743) | `src/SinglePlayer.asm` | 2 |
| 3855 | 0x0F0F | `SONIC_VICTORY_IMAGE_TOP` | 1738 | Sonic Ending Image 2 (L1744) |  | 0 |
| 3856 | 0x0F10 | `YLINK_VICTORY_IMAGE_BOTTOM` | 1739 | Young Link Ending Image 1 (L1745) | `src/SinglePlayer.asm` | 1 |
| 3857 | 0x0F11 | `YLINK_VICTORY_IMAGE_TOP` | 1740 | Young Link Ending Image 2 (L1746) |  | 0 |
| 3858 | 0x0F12 | `WOLF_VICTORY_IMAGE_BOTTOM` | 1741 | Wolf Ending Image 1 (L1747) | `src/SinglePlayer.asm` | 1 |
| 3859 | 0x0F13 | `WOLF_VICTORY_IMAGE_TOP` | 1742 | Wolf Ending Image 2 (L1748) |  | 0 |
| 3860 | 0x0F14 | `DSAMUS_VICTORY_IMAGE_BOTTOM` | 1743 | Dark Samus Ending Image 1 (L1749) | `src/SinglePlayer.asm` | 1 |
| 3861 | 0x0F15 | `DSAMUS_VICTORY_IMAGE_TOP` | 1744 | Dark Samus Ending Image 2 (L1750) |  | 0 |
| 3862 | 0x0F16 | `LUCAS_VICTORY_IMAGE_BOTTOM` | 1745 | Lucas Ending Image 1 (L1751) | `src/SinglePlayer.asm` | 1 |
| 3863 | 0x0F17 | `LUCAS_VICTORY_IMAGE_TOP` | 1746 | Lucas Ending Image 2 (L1752) |  | 0 |
| 3864 | 0x0F18 | `DRM_VICTORY_IMAGE_BOTTOM` | 1747 | Doctor Mario Ending Image 1 (L1753) | `src/SinglePlayer.asm` | 1 |
| 3865 | 0x0F19 | `DRM_VICTORY_IMAGE_TOP` | 1748 | Doctor Mario Ending Image 2 (L1754) |  | 0 |
| 3866 | 0x0F1A | `FALCO_VICTORY_IMAGE_BOTTOM` | 1749 | Falco Ending Image 1 (L1755) | `src/SinglePlayer.asm` | 1 |
| 3867 | 0x0F1B | `FALCO_VICTORY_IMAGE_TOP` | 1750 | Falco Ending Image 2 (L1756) |  | 0 |
| 3868 | 0x0F1C | `SHEIK_ITEM_NEUTRAL` | 1751 | Sheik Item Neutral (L1757) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 8 |
| 3869 | 0x0F1D | `SHEIK_ITEM_TILT` | 1752 | Sheik Item Tilt (L1758) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 8 |
| 3870 | 0x0F1E | `SHEIK_ITEM_SMASH` | 1753 | Sheik Item Smash (L1759) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 8 |
| 3871 | 0x0F1F | `SHEIK_ITEM_DASH_ATTACK` | 1754 | Sheik Item Dash Attack (L1760) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 8 |
| 3872 | 0x0F20 | `SHEIK_ITEM_THROW_DASH` | 1755 | Sheik Item Dash Throw (L1761) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3873 | 0x0F21 | `SHEIK_HAMMER_MOVE` | 1756 | Sheik Hammer Move (L1762) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 10 |
| 3874 | 0x0F22 | `SHEIK_ITEM_THROW_AIR_U` | 1757 | Sheik Item Throw Air U (L1763) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 4 |
| 3875 | 0x0F23 | `SHEIK_ROLL_F` | 1758 | Sheik Roll F (L1764) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3876 | 0x0F24 | `SHEIK_ROLL_B` | 1759 | Sheik Roll B (L1765) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3877 | 0x0F25 | `SHEIK_LANDING_AIR_U` | 1760 | Sheik Uair Landing (L1766) | `src/NSheik/NSheik.asm`, `src/Sheik/Sheik.asm` | 2 |
| 3878 | 0x0F26 | `STG_GLACIAL_RIVER_REMIX_HEADER` | 1761 | Glacial River Remix Header (L1767) |  | 0 |
| 3879 | 0x0F27 | `STG_GLACIAL_RIVER_REMX_STAGE` | 1762 | Glacial River Remix Stage (L1768) |  | 0 |
| 3880 | 0x0F28 | `NWARIO_MAIN` | 1763 | Polygon Wario Main Character (L1769) | `src/Character.asm` | 1 |
| 3881 | 0x0F29 | `NWARIO_CHARACTER` | 1764 | Polygon Wario Character (L1770) | `src/Character.asm` | 1 |
| 3882 | 0x0F2A | `NLUCAS_MAIN` | 1765 | Polygon Lucas Main Character (L1771) | `src/Character.asm` | 1 |
| 3883 | 0x0F2B | `NLUCAS_CHARACTER` | 1766 | Polygon Lucas Character (L1772) | `src/Character.asm` | 1 |
| 3884 | 0x0F2C | `YLINK_GRAB` | 1767 | Young Link Grab (L1773) | `src/NYoungLink/NYoungLink.asm`, `src/YoungLink/YoungLink.asm` | 2 |
| 3885 | 0x0F2D | `YLINK_GRAB_PULL` | 1768 | Young Link Grab Pull (L1774) | `src/NYoungLink/NYoungLink.asm`, `src/YoungLink/YoungLink.asm` | 2 |
| 3886 | 0x0F2E | `MARINA_MAIN` | 1769 | Marina Main (L1775) | `src/Character.asm` | 1 |
| 3887 | 0x0F2F | `MARINA_CHARACTER` | 1770 | Marina Character (L1776) | `src/Character.asm` | 1 |
| 3888 | 0x0F30 | `MARINA_SHIELD_POSE` | 1771 | Marina Shield Pose (L1777) | `src/Character.asm` | 2 |
| 3889 | 0x0F31 | `MARINA_IDLE` | 1772 | Marina Idle (L1778) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 12 |
| 3890 | 0x0F32 | `MARINA_WALK_1` | 1773 | Marina Walk1 (L1779) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3891 | 0x0F33 | `MARINA_WALK_2` | 1774 | Marina Walk2 (L1780) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3892 | 0x0F34 | `MARINA_WALK_3` | 1775 | Marina Walk3 (L1781) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3893 | 0x0F35 | `MARINA_WALK_END` | 1776 | Marina WalkEnd (L1782) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3894 | 0x0F36 | `MARINA_DASH` | 1777 | Marina Dash (L1783) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3895 | 0x0F37 | `MARINA_RUN` | 1778 | Marina Run (L1784) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3896 | 0x0F38 | `MARINA_RUN_BRAKE` | 1779 | Marina RunBrake (L1785) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3897 | 0x0F39 | `MARINA_TURN` | 1780 | Marina Turn (L1786) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3898 | 0x0F3A | `MARINA_TURN_RUN` | 1781 | Marina TurnRun (L1787) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3899 | 0x0F3B | `MARINA_JUMP_F` | 1782 | Marina JumpF (L1788) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3900 | 0x0F3C | `MARINA_JUMP_B` | 1783 | Marina JumpB (L1789) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3901 | 0x0F3D | `MARINA_JUMP_AERIAL_F` | 1784 | Marina JumpAerialF (L1790) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3902 | 0x0F3E | `MARINA_JUMP_AERIAL_B` | 1785 | Marina JumpAerialB (L1791) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3903 | 0x0F3F | `MARINA_FALL` | 1786 | Marina Fall (L1792) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3904 | 0x0F40 | `MARINA_FALL_AERIAL` | 1787 | Marina FallAerial (L1793) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3905 | 0x0F41 | `MARINA_CROUCH` | 1788 | Marina Crouch (L1794) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3906 | 0x0F42 | `MARINA_CROUCH_IDLE` | 1789 | Marina CrouchIdle (L1795) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3907 | 0x0F43 | `MARINA_CROUCH_END` | 1790 | Marina CrouchEnd (L1796) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3908 | 0x0F44 | `MARINA_LANDING` | 1791 | Marina Landing (L1797) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 12 |
| 3909 | 0x0F45 | `MARINA_PLAT_DROP` | 1792 | Marina PlatDrop (L1798) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3910 | 0x0F46 | `MARINA_TEETER` | 1793 | Marina Teeter (L1799) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3911 | 0x0F47 | `MARINA_TEETERSTART` | 1794 | Marina Teeterstart (L1800) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3912 | 0x0F48 | `MARINA_DMG_HIGH_1` | 1795 | Marina DamageHigh1 (L1801) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3913 | 0x0F49 | `MARINA_DMG_HIGH_2` | 1796 | Marina DamageHigh2 (L1802) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3914 | 0x0F4A | `MARINA_DMG_HIGH_3` | 1797 | Marina DamageHigh3 (L1803) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3915 | 0x0F4B | `MARINA_DMG_MID_1` | 1798 | Marina DamageMid1 (L1804) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3916 | 0x0F4C | `MARINA_DMG_MID_2` | 1799 | Marina DamageMid2 (L1805) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3917 | 0x0F4D | `MARINA_DMG_MID_3` | 1800 | Marina DamageMid3 (L1806) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3918 | 0x0F4E | `MARINA_DMG_LOW_1` | 1801 | Marina DamageLow1 (L1807) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3919 | 0x0F4F | `MARINA_DMG_LOW_2` | 1802 | Marina DamageLow2 (L1808) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3920 | 0x0F50 | `MARINA_DMG_LOW_3` | 1803 | Marina DamageLow3 (L1809) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3921 | 0x0F51 | `MARINA_DMG_AIR_1` | 1804 | Marina DamageAir1 (L1810) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3922 | 0x0F52 | `MARINA_DMG_AIR_2` | 1805 | Marina DamageAir2 (L1811) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3923 | 0x0F53 | `MARINA_DMG_AIR_3` | 1806 | Marina DamageAir3 (L1812) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3924 | 0x0F54 | `MARINA_DMG_ELEC` | 1807 | Marina DamageElec (L1813) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3925 | 0x0F55 | `MARINA_DMG_FLY_HIGH` | 1808 | Marina DamageFlyHigh (L1814) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3926 | 0x0F56 | `MARINA_DMG_FLY_MID` | 1809 | Marina DamageFlyMid (L1815) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3927 | 0x0F57 | `MARINA_DMG_FLY_LOW` | 1810 | Marina DamageFlyLow (L1816) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3928 | 0x0F58 | `MARINA_DMG_FLY_TOP` | 1811 | Marina DamageFlyTop (L1817) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3929 | 0x0F59 | `MARINA_DMG_FLY_ROLL` | 1812 | Marina DamageFlyRoll (L1818) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3930 | 0x0F5A | `MARINA_TUMBLE` | 1813 | Marina Tumble (L1819) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 20 |
| 3931 | 0x0F5B | `MARINA_FALL_SPECIAL` | 1814 | Marina FallSpecial (L1820) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3932 | 0x0F5C | `MARINA_ENTER_PIPE` | 1815 | Marina EnterPipe (L1821) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3933 | 0x0F5D | `MARINA_EXIT_PIPE` | 1816 | Marina ExitPipe (L1822) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3934 | 0x0F5E | `MARINA_EXIT_PIPE_WALK` | 1817 | Marina ExitPipeWalk (L1823) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3935 | 0x0F5F | `MARINA_CEILING_BONK` | 1818 | Marina CeilingBonk (L1824) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3936 | 0x0F60 | `MARINA_DOWN_BNCE_D` | 1819 | Marina DownBounceD (L1825) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 6 |
| 3937 | 0x0F61 | `MARINA_DOWN_BNCE_U` | 1820 | Marina DownBounceU (L1826) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3938 | 0x0F62 | `MARINA_DOWN_STND_D` | 1821 | Marina DownStandD (L1827) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 6 |
| 3939 | 0x0F63 | `MARINA_DOWN_STND_U` | 1822 | Marina DownStandU (L1828) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm`, `src/TwelveCharBattle.asm` | 6 |
| 3940 | 0x0F64 | `MARINA_TECH_F` | 1823 | Marina TechF (L1829) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3941 | 0x0F65 | `MARINA_TECH_B` | 1824 | Marina TechB (L1830) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3942 | 0x0F66 | `MARINA_DOWN_FWRD_D` | 1825 | Marina DownForwardD (L1831) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3943 | 0x0F67 | `MARINA_DOWN_FWRD_U` | 1826 | Marina DownForwardU (L1832) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3944 | 0x0F68 | `MARINA_DOWN_BACK_D` | 1827 | Marina DownBackD (L1833) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3945 | 0x0F69 | `MARINA_DOWN_BACK_U` | 1828 | Marina DownBackU (L1834) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3946 | 0x0F6A | `MARINA_DOWN_ATK_D` | 1829 | Marina DownAttackD (L1835) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3947 | 0x0F6B | `MARINA_DOWN_ATK_U` | 1830 | Marina DownAttackU (L1836) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3948 | 0x0F6C | `MARINA_TECH` | 1831 | Marina Tech (L1837) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3949 | 0x0F6D | `MARINA_CLANG_RECOIL` | 1832 | Marina ClangRecoil (L1838) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3950 | 0x0F6E | `MARINA_CLF_CATCH` | 1833 | Marina CliffCatch (L1839) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3951 | 0x0F6F | `MARINA_CLF_WAIT` | 1834 | Marina CliffWait (L1840) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3952 | 0x0F70 | `MARINA_CLF_QUICK` | 1835 | Marina CliffQuick (L1841) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3953 | 0x0F71 | `MARINA_CLF_CLM_Q_1` | 1836 | Marina CliffClimbQuick1 (L1842) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3954 | 0x0F72 | `MARINA_CLF_CLM_Q_2` | 1837 | Marina CliffClimbQuick2 (L1843) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3955 | 0x0F73 | `MARINA_CLF_SLOW` | 1838 | Marina CliffSlow (L1844) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3956 | 0x0F74 | `MARINA_CLF_CLM_S_1` | 1839 | Marina CliffClimbSlow1 (L1845) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3957 | 0x0F75 | `MARINA_CLF_CLM_S_2` | 1840 | Marina CliffClimbSlow2 (L1846) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3958 | 0x0F76 | `MARINA_CLF_ATK_Q_1` | 1841 | Marina CliffAttackQuick1 (L1847) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3959 | 0x0F77 | `MARINA_CLF_ATK_Q_2` | 1842 | Marina CliffAttackQuick2 (L1848) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3960 | 0x0F78 | `MARINA_CLF_ATK_S_1` | 1843 | Marina CliffAttackSlow1 (L1849) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3961 | 0x0F79 | `MARINA_CLF_ATK_S_2` | 1844 | Marina CliffAttackSlow2 (L1850) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3962 | 0x0F7A | `MARINA_CLF_ESC_Q_1` | 1845 | Marina CliffEscapeQuick1 (L1851) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3963 | 0x0F7B | `MARINA_CLF_ESC_Q_2` | 1846 | Marina CliffEscapeQuick2 (L1852) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3964 | 0x0F7C | `MARINA_CLF_ESC_S_1` | 1847 | Marina CliffEscapeSlow1 (L1853) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3965 | 0x0F7D | `MARINA_CLF_ESC_S_2` | 1848 | Marina CliffEscapeSlow2 (L1854) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3966 | 0x0F7E | `MARINA_L_ITM_PICKUP` | 1849 | Marina LightItemPickup (L1855) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3967 | 0x0F7F | `MARINA_H_ITM_PICKUP` | 1850 | Marina HeavyItemPickup (L1856) |  | 0 |
| 3968 | 0x0F80 | `MARINA_ITM_DROP` | 1851 | Marina ItemDrop (L1857) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3969 | 0x0F81 | `MARINA_ITM_THROW_DASH` | 1852 | Marina ItemThrowDash (L1858) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3970 | 0x0F82 | `MARINA_ITM_THROW_F` | 1853 | Marina ItemThrowF (L1859) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 8 |
| 3971 | 0x0F83 | `MARINA_ITM_THROW_U` | 1854 | Marina ItemThrowU (L1860) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3972 | 0x0F84 | `MARINA_ITM_THROW_D` | 1855 | Marina ItemThrowD (L1861) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3973 | 0x0F85 | `MARINA_ITM_THROW_AIR_F` | 1856 | Marina ItemThrowAirF (L1862) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 8 |
| 3974 | 0x0F86 | `MARINA_ITM_THROW_AIR_U` | 1857 | Marina ItemThrowAirU (L1863) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3975 | 0x0F87 | `MARINA_ITM_THROW_AIR_D` | 1858 | Marina ItemThrowAirD (L1864) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3976 | 0x0F88 | `MARINA_HEAVY_ITM_THROW` | 1859 | Marina HeavyItemThrow (L1865) |  | 0 |
| 3977 | 0x0F89 | `MARINA_ITM_NEUTRAL` | 1860 | Marina ItemNeutral (L1866) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 8 |
| 3978 | 0x0F8A | `MARINA_ITM_TILT` | 1861 | Marina ItemTilt (L1867) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 8 |
| 3979 | 0x0F8B | `MARINA_ITM_SMASH` | 1862 | Marina ItemSmash (L1868) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 8 |
| 3980 | 0x0F8C | `MARINA_ITM_DASH` | 1863 | Marina ItemDash (L1869) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 8 |
| 3981 | 0x0F8D | `MARINA_ITM_SHOOT` | 1864 | Marina ItemShoot (L1870) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3982 | 0x0F8E | `MARINA_ITM_SHOOT_AIR` | 1865 | Marina ItemShootAir (L1871) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3983 | 0x0F8F | `MARINA_HAMMER_IDLE` | 1866 | Marina HammerIdle (L1872) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3984 | 0x0F90 | `MARINA_HAMMER_MOVE` | 1867 | Marina HammerMove (L1873) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 10 |
| 3985 | 0x0F91 | `MARINA_SHIELD_ON` | 1868 | Marina ShieldOn (L1874) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3986 | 0x0F92 | `MARINA_SHIELD_OFF` | 1869 | Marina ShieldOff (L1875) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3987 | 0x0F93 | `MARINA_ROLL_F` | 1870 | Marina RollF (L1876) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3988 | 0x0F94 | `MARINA_ROLL_B` | 1871 | Marina RollB (L1877) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3989 | 0x0F95 | `MARINA_STUN` | 1872 | Marina Stun (L1878) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3990 | 0x0F96 | `MARINA_GRAB` | 1873 | Marina Grab (L1879) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3991 | 0x0F97 | `MARINA_GRAB_PULL` | 1874 | Marina GrabPull (L1880) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3992 | 0x0F98 | `MARINA_THROW_F` | 1875 | Marina ThrowF (L1881) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3993 | 0x0F99 | `MARINA_THROW_B` | 1876 | Marina ThrowB (L1882) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3994 | 0x0F9A | `MARINA_CAPTURE_PULLED` | 1877 | Marina CapturePulled (L1883) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 3995 | 0x0F9B | `MARINA_THROWN_DKPULLED` | 1878 | Marina ThrownDKPulled (L1884) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3996 | 0x0F9C | `MARINA_THROWN_MARIO_BROS` | 1879 | Marina ThrownMarioBros (L1885) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3997 | 0x0F9D | `MARINA_THROWN_DK` | 1880 | Marina ThrownDK (L1886) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3998 | 0x0F9E | `MARINA_THROWN_1` | 1881 | Marina Thrown1 (L1887) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 3999 | 0x0F9F | `MARINA_THROWN_2` | 1882 | Marina Thrown2 (L1888) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4000 | 0x0FA0 | `MARINA_TAUNT` | 1883 | Marina Taunt (L1889) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4001 | 0x0FA1 | `MARINA_JAB_1` | 1884 | Marina Jab1 (L1890) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4002 | 0x0FA2 | `MARINA_JAB_2` | 1885 | Marina Jab2 (L1891) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4003 | 0x0FA3 | `MARINA_DASH_ATTACK` | 1886 | Marina DashAttack (L1892) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4004 | 0x0FA4 | `MARINA_FTILT_HIGH` | 1887 | Marina FTiltHigh (L1893) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4005 | 0x0FA5 | `MARINA_FTILT_MID_HIGH` | 1888 | Marina FTiltMidHigh (L1894) |  | 0 |
| 4006 | 0x0FA6 | `MARINA_FTILT` | 1889 | Marina FTilt (L1895) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4007 | 0x0FA7 | `MARINA_FTILT_MID_LOW` | 1890 | Marina FTiltMidLow (L1896) |  | 0 |
| 4008 | 0x0FA8 | `MARINA_FTILT_LOW` | 1891 | Marina FTiltLow (L1897) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4009 | 0x0FA9 | `MARINA_UTILT` | 1892 | Marina UTilt (L1898) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4010 | 0x0FAA | `MARINA_DTILT` | 1893 | Marina DTilt (L1899) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4011 | 0x0FAB | `MARINA_FSMASH_HIGH` | 1894 | Marina FSmashHigh (L1900) |  | 0 |
| 4012 | 0x0FAC | `MARINA_FSMASH` | 1895 | Marina FSmash (L1901) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4013 | 0x0FAD | `MARINA_FSMASH_LOW` | 1896 | Marina FSmashLow (L1902) |  | 0 |
| 4014 | 0x0FAE | `MARINA_USMASH` | 1897 | Marina USmash (L1903) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4015 | 0x0FAF | `MARINA_DSMASH` | 1898 | Marina DSmash (L1904) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4016 | 0x0FB0 | `MARINA_ATTACK_AIR_N` | 1899 | Marina AttackAirN (L1905) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4017 | 0x0FB1 | `MARINA_ATTACK_AIR_F` | 1900 | Marina AttackAirF (L1906) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4018 | 0x0FB2 | `MARINA_ATTACK_AIR_B` | 1901 | Marina AttackAirB (L1907) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4019 | 0x0FB3 | `MARINA_ATTACK_AIR_U` | 1902 | Marina AttackAirU (L1908) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4020 | 0x0FB4 | `MARINA_ATTACK_AIR_D` | 1903 | Marina AttackAirD (L1909) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4021 | 0x0FB5 | `MARINA_LANDING_AIR_F` | 1904 | Marina LandingAirF (L1910) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4022 | 0x0FB6 | `MARINA_LANDING_AIR_B` | 1905 | Marina LandingAirB (L1911) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4023 | 0x0FB7 | `MARINA_JAB_LOOP_START` | 1906 | Marina Jab Loop Start (L1912) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4024 | 0x0FB8 | `MARINA_JAB_LOOP` | 1907 | Marina Jab Loop (L1913) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4025 | 0x0FB9 | `MARINA_JAB_LOOP_END` | 1908 | Marina Jab Loop End (L1914) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4026 | 0x0FBA | `MARINA_ACTION_0DF` | 1909 | Marina Action_0DF (L1915) | `src/NMarina/NMarina.asm` | 1 |
| 4027 | 0x0FBB | `MARINA_ACTION_0E0` | 1910 | Marina Action_0E0 (L1916) | `src/NMarina/NMarina.asm` | 1 |
| 4028 | 0x0FBC | `MARINA_NSPG` | 1911 | Marina Neutral Special Ground (L1917) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4029 | 0x0FBD | `MARINA_NSPG_PULL` | 1912 | Marina Neutral Special Ground Pull (L1918) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4030 | 0x0FBE | `MARINA_NSPG_THROW` | 1913 | Marina Neutral Special Ground Throw (L1919) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4031 | 0x0FBF | `MARINA_NSPG_THROW_U` | 1914 | Marina Neutral Special Ground Up Throw (L1920) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4032 | 0x0FC0 | `MARINA_NSPG_THROW_D` | 1915 | Marina Neutral Special Ground Down Throw (L1921) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4033 | 0x0FC1 | `MARINA_NSPA` | 1916 | Marina Neutral Special Air (L1922) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4034 | 0x0FC2 | `MARINA_NSPA_PULL` | 1917 | Marina Neutral Special Air Pull (L1923) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4035 | 0x0FC3 | `MARINA_NSPA_THROW` | 1918 | Marina Neutral Special Air Throw (L1924) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4036 | 0x0FC4 | `MARINA_NSPA_THROW_U` | 1919 | Marina Neutral Special Air Up Throw (L1925) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4037 | 0x0FC5 | `MARINA_NSPA_THROW_D` | 1920 | Marina Neutral Special Air Down Throw (L1926) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4038 | 0x0FC6 | `MARINA_CARGO` | 1921 | Marina Cargo (L1927) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4039 | 0x0FC7 | `MARINA_CARGO_TURN` | 1922 | Marina Cargo Turn (L1928) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4040 | 0x0FC8 | `MARINA_CARGO_JUMPSQUAT` | 1923 | Marina Cargo JumpSquat (L1929) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 4041 | 0x0FC9 | `MARINA_VICTORY_1` | 1924 | Marina Victory 1 (L1930) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 4042 | 0x0FCA | `MARINA_VICTORY_2` | 1925 | Marina Victory 2 (L1931) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4043 | 0x0FCB | `MARINA_VICTORY_3` | 1926 | Marina Victory 3 (L1932) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4044 | 0x0FCC | `MARINA_CLAP` | 1927 | Marina Clap (L1933) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4045 | 0x0FCD | `MARINA_CONTINUE_FALL` | 1928 | Marina ContinueFall (L1934) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4046 | 0x0FCE | `MARINA_CONTINUE_UP` | 1929 | Marina ContinueUp (L1935) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4047 | 0x0FCF | `MARINA_1P_POSE` | 1930 | Marina 1P Pose (L1936) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4048 | 0x0FD0 | `MARINA_CARGO_WALK_1` | 1931 | Marina Cargo Walk1 (L1937) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4049 | 0x0FD1 | `MARINA_CARGO_WALK_2` | 1932 | Marina Cargo Walk2 (L1938) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4050 | 0x0FD2 | `MARINA_CARGO_WALK_3` | 1933 | Marina Cargo Walk3 (L1939) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4051 | 0x0FD3 | `MARINA_CARGO_AIR` | 1934 | Marina Cargo Air (L1940) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 4 |
| 4052 | 0x0FD4 | `MARINA_CARGO_THROW` | 1935 | Marina Cargo Throw (L1941) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 18 |
| 4053 | 0x0FD5 | `MARINA_CARGO_THROW_AIR` | 1936 | Marina Cargo Throw Air (L1942) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4054 | 0x0FD6 | `MARINA_CARGO_JUMP` | 1937 | Marina Cargo Jump (L1943) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4055 | 0x0FD7 | `MARINA_CARGO_SHAKE` | 1938 | Marina Cargo Shake (L1944) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4056 | 0x0FD8 | `MARINA_USPG` | 1939 | Marina Up Special Ground (L1945) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4057 | 0x0FD9 | `MARINA_USPA` | 1940 | Marina Up Special Air (L1946) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4058 | 0x0FDA | `MARINA_GEM_HITBOX` | 1941 | Marina Items Hitbox Info (L1947) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 4059 | 0x0FDB | `MARINA_GEM_IMAGES` | 1942 | Marina Item Images (L1948) |  | 0 |
| 4060 | 0x0FDC | `NBOWSER_MAIN` | 1943 | Polygon Bowser Main Character (L1949) | `src/Character.asm` | 1 |
| 4061 | 0x0FDD | `NBOWSER_CHARACTER` | 1944 | Polygon Bowser Character (L1950) | `src/Character.asm` | 1 |
| 4062 | 0x0FDE | `NWOLF_MAIN` | 1945 | Polygon Wolf Main Character (L1951) | `src/Character.asm` | 1 |
| 4063 | 0x0FDF | `NWOLF_CHARACTER` | 1946 | Polygon Wolf Character (L1952) | `src/Character.asm` | 1 |
| 4064 | 0x0FE0 | `NDRM_MAIN` | 1947 | Polygon Dr. Mario Main Character (L1953) | `src/Character.asm` | 1 |
| 4065 | 0x0FE1 | `MARINA_DSPG_BEGIN` | 1948 | Marina Down Special Ground Begin (L1954) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4066 | 0x0FE2 | `MARINA_DSPG_WAIT` | 1949 | Marina Down Special Ground Wait (L1955) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4067 | 0x0FE3 | `MARINA_DSPG_ABSORB` | 1950 | Marina Down Special Ground Absorb (L1956) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4068 | 0x0FE4 | `MARINA_DSPG_END` | 1951 | Marina Down Special Ground End (L1957) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4069 | 0x0FE5 | `MARINA_DSPG_PULL` | 1952 | Marina Down Special Ground Pull (L1958) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4070 | 0x0FE6 | `MARINA_DSPG_PULL_FAIL` | 1953 | Marina Down Special Ground Pull Fail (L1959) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4071 | 0x0FE7 | `MARINA_DSPA_BEGIN` | 1954 | Marina Down Special Air Begin (L1960) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4072 | 0x0FE8 | `MARINA_DSPA_WAIT` | 1955 | Marina Down Special Air Wait (L1961) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4073 | 0x0FE9 | `MARINA_DSPA_ABSORB` | 1956 | Marina Down Special Air Absorb (L1962) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4074 | 0x0FEA | `MARINA_DSPA_END` | 1957 | Marina Down Special Air End (L1963) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4075 | 0x0FEB | `MARINA_DSPA_PULL` | 1958 | Marina Down Special Air Pull (L1964) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4076 | 0x0FEC | `MARINA_DSPA_PULL_FAIL` | 1959 | Marina Down Special Air Pull Fail (L1965) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4077 | 0x0FED | `NSONIC_MAIN` | 1960 | Polygon Sonic Main Character (L1966) | `src/Character.asm` | 1 |
| 4078 | 0x0FEE | `NSONIC_CHARACTER` | 1961 | Polygon Sonic Character (L1967) | `src/Character.asm` | 1 |
| 4079 | 0x0FEF | `NSHEIK_MAIN` | 1962 | Polygon Sheik Main Character (L1968) | `src/Character.asm` | 1 |
| 4080 | 0x0FF0 | `NSHEIK_CHARACTER` | 1963 | Polygon Sheik Character (L1969) | `src/Character.asm` | 1 |
| 4081 | 0x0FF1 | `STG_MARINA_TARGET_HEADER` | 1964 | Marina Break the Targets Header (L1970) |  | 0 |
| 4082 | 0x0FF2 | `STG_MARINA_TARGET` | 1965 | Marina Break the Targets Stage (L1971) |  | 0 |
| 4083 | 0x0FF3 | `STG_DKING_REMIX_HEADER` | 1966 | Dragon King Remix Header (L1972) |  | 0 |
| 4084 | 0x0FF4 | `STG_DKING_REMIX_STAGE` | 1967 | Dragon King Remix Stage (L1973) |  | 0 |
| 4085 | 0x0FF5 | `STG_DKING_REMIX_BG` | 1968 | Dragon King Remix BG (L1974) |  | 0 |
| 4088 | 0x0FF8 | `MARINA_CPU_POSE` | 1969 | Marina CPU Pose (L1977) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4089 | 0x0FF9 | `MARINA_DSPG_STOW` | 1970 | Marina Down Special Ground Stow (L1978) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4090 | 0x0FFA | `MARINA_DSPA_STOW` | 1971 | Marina Down Special Stow (L1979) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 4091 | 0x0FFB | `YLINK_SPECIAL_GRAPHIC` | 1972 | Young Link Up/Down Special Graphic (L1980) | `src/Character.asm` | 2 |
| 4092 | 0x0FFC | `MARINA_TEAM_POSE` | 1973 | Marina Team Pose (L1981) | `src/SinglePlayerModes.asm` | 4 |
| 4093 | 0x0FFD | `DEDEDE_MAIN` | 1974 | Dedede Main (L1982) | `src/Character.asm` | 1 |
| 4094 | 0x0FFE | `DEDEDE_CHARACTER` | 1975 | Dedede Character (L1983) | `src/Character.asm` | 1 |
| 4095 | 0x0FFF | `DEDEDE_STAR` | 1976 | Dedede Stars (L1984) | `src/Character.asm` | 1 |
| 4096 | 0x1000 | `WADDLE_DEE_INFO` | 1977 | Waddle Dee Info (L1985) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 4097 | 0x1001 | `WADDLE_DEE_GFX` | 1978 | Waddle Dee graphic (L1986) |  | 0 |
| 4098 | 0x1002 | `DEDEDE_IDLE` | 1979 | Dedede Idle (L1987) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 12 |
| 4099 | 0x1003 | `DEDEDE_DASH` | 1980 | Dedede Dash (L1988) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4100 | 0x1004 | `DEDEDE_RUN` | 1981 | Dedede Run (L1989) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4101 | 0x1005 | `DEDEDE_RUN_BRAKE` | 1982 | Dedede Run Brake (L1990) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4102 | 0x1006 | `DEDEDE_TURN` | 1983 | Dedede Turn (L1991) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 6 |
| 4103 | 0x1007 | `DEDEDE_TURN_RUN` | 1984 | Dedede Turn Run (L1992) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4104 | 0x1008 | `DEDEDE_JUMP_F` | 1985 | Dedede Jump F (L1993) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4105 | 0x1009 | `DEDEDE_JUMP_B` | 1986 | Dedede Jump B (L1994) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4106 | 0x100A | `DEDEDE_JUMP_AERIAL_1` | 1987 | Dedede Jump Aerial 1 (L1995) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 6 |
| 4107 | 0x100B | `DEDEDE_JUMP_AERIAL_2` | 1988 | Dedede Jump Aerial 2 (L1996) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4108 | 0x100C | `DEDEDE_FALL` | 1989 | Dedede Fall (L1997) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4109 | 0x100D | `DEDEDE_FALL_AERIAL` | 1990 | Dedede Fall Aerial (L1998) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4110 | 0x100E | `DEDEDE_LANDING` | 1991 | Dedede Jump Squat (L1999) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 14 |
| 4111 | 0x100F | `DEDEDE_USP_BEGIN` | 1992 | Dedede Up Special Begin (L2000) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4112 | 0x1010 | `DEDEDE_USP_LOOP` | 1993 | Dedede Up Special Loop (L2001) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4113 | 0x1011 | `DEDEDE_USP_LAND` | 1994 | Dedede Up Special Land (L2002) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4114 | 0x1012 | `DEDEDE_USP_CANCEL` | 1995 | Dedede Up Special Cancel (L2003) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4115 | 0x1013 | `DEDEDE_SFALL` | 1996 | Dedede Special Fall (L2004) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4116 | 0x1014 | `DEDEDE_SFALL_LANDING` | 1997 | Dedede Special Fall Landing (L2005) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4117 | 0x1015 | `DEDEDE_ITEM_SHOOT` | 1998 | Dedede Item Shoot (L2006) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4118 | 0x1016 | `DEDEDE_ITEM_SHOOT_AIR` | 1999 | Dedede Item Shoot Air (L2007) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4119 | 0x1017 | `DEDEDE_DSP_BEGIN` | 2000 | Dedede Down Special Begin (L2008) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4120 | 0x1018 | `DEDEDE_DSP_CHARGE` | 2001 | Dedede Down Special Loop (L2009) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4121 | 0x1019 | `DEDEDE_DSP_SHOOT` | 2002 | Dedede Down Special End (L2010) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4122 | 0x101A | `DEDEDE_NSP_BEGIN` | 2003 | Dedede Neutral Special Begin (L2011) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4123 | 0x101B | `DEDEDE_NSP_LOOP` | 2004 | Dedede Neutral Special Loop (L2012) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4124 | 0x101C | `DEDEDE_NSP_PULL` | 2005 | Dedede Neutral Special Inhale (L2013) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4125 | 0x101D | `DEDEDE_NSP_INHALED_IDLE` | 2006 | Dedede Neutral Special Inhaled Idle (L2014) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4126 | 0x101E | `DEDEDE_NSP_INHALED_WALK` | 2007 | Dedede Neutral Special Inhaled Walk (L2015) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 6 |
| 4127 | 0x101F | `DEDEDE_NSP_SPIT` | 2008 | Dedede Neutral Special Spit (L2016) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4128 | 0x1020 | `DEDEDE_NSP_END` | 2009 | Dedede Neutral Special End (L2017) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4129 | 0x1021 | `DEDEDE_HAMMER_IDLE` | 2010 | Dedede Hammer Idle (L2018) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4130 | 0x1022 | `DEDEDE_HAMMER_MOVE` | 2011 | Dedede Hammer Move (L2019) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 10 |
| 4131 | 0x1023 | `DEDEDE_CROUCH` | 2012 | Dedede Crouch (L2020) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4132 | 0x1024 | `DEDEDE_CROUCH_IDLE` | 2013 | Dedede Crouch Idle (L2021) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4133 | 0x1025 | `DEDEDE_CROUCH_END` | 2014 | Dedede Crouch End (L2022) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4134 | 0x1026 | `DEDEDE_JAB_1` | 2015 | Dedede Jab 1 (L2023) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4135 | 0x1027 | `DEDEDE_JAB_2` | 2016 | Dedede Jab 2 (L2024) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4136 | 0x1028 | `DEDEDE_JAB_3` | 2017 | Dedede Jab 3 (L2025) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4137 | 0x1029 | `DEDEDE_DASH_ATTACK` | 2018 | Dedede Dash Attack (L2026) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4138 | 0x102A | `DEDEDE_U_TILT` | 2019 | Dedede Up Tilt (L2027) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4139 | 0x102B | `DEDEDE_ATTACK_AIR_F` | 2020 | Dedede Forward Air (L2028) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4140 | 0x102C | `DEDEDE_ATTACK_AIR_N` | 2021 | Dedede Neutral Air (L2029) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4141 | 0x102D | `DEDEDE_ATTACK_AIR_D` | 2022 | Dedede Down Air (L2030) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4142 | 0x102E | `DEDEDE_D_SMASH` | 2023 | Dedede Down Smash (L2031) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4143 | 0x102F | `DEDEDE_GRAB` | 2024 | Dedede Catch (L2032) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4144 | 0x1030 | `DEDEDE_GRAB_PULL` | 2025 | Dedede Catch Pull (L2033) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4145 | 0x1031 | `DEDEDE_THROW_F` | 2026 | Dedede Forward Throw (L2034) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4146 | 0x1032 | `DEDEDE_D_TILT` | 2027 | Dedede Down Tilt (L2035) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4147 | 0x1033 | `DEDEDE_U_SMASH` | 2028 | Dedede Up Smash (L2036) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4148 | 0x1034 | `DEDEDE_ATTACK_AIR_U` | 2029 | Dedede Up Air (L2037) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4149 | 0x1035 | `DEDEDE_THROW_B` | 2030 | Dedede Back Throw (L2038) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4150 | 0x1036 | `DEDEDE_ATTACK_AIR_B` | 2031 | Dedede Back Air (L2039) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4151 | 0x1037 | `DEDEDE_PUPPET_FALL` | 2032 | Dedede Puppet Fall (L2040) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4152 | 0x1038 | `DEDEDE_PUPPET_UP` | 2033 | Dedede Puppet Up (L2041) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4153 | 0x1039 | `DEDEDE_F_TILT` | 2034 | Dedede Forward Tilt (L2042) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4154 | 0x103A | `DEDEDE_F_SMASH` | 2035 | Dedede Forward Smash (L2043) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4155 | 0x103B | `DEDEDE_TAUNT` | 2036 | Dedede Taunt (L2044) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4156 | 0x103C | `DEDEDE_SHIELD_POSE` | 2037 | Dedede Shield Pose (L2045) | `src/Character.asm` | 2 |
| 4157 | 0x103D | `DEDEDE_SHIELD_ON` | 2038 | Dedede Shield On (L2046) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4158 | 0x103E | `DEDEDE_SHIELD_OFF` | 2039 | Dedede Shield Off (L2047) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4159 | 0x103F | `DEDEDE_SHIELD_DROP` | 2040 | Dedede Shield Drop (L2048) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4160 | 0x1040 | `DEDEDE_ROLL_F` | 2041 | Dedede Roll Forward (L2049) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4161 | 0x1041 | `DEDEDE_ROLL_B` | 2042 | Dedede Roll Backward (L2050) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4162 | 0x1042 | `DEDEDE_VICTORY_1` | 2043 | Dedede Victory 1 (L2051) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4163 | 0x1043 | `NMARINA_MAIN` | 2044 | Polygon Marina Main (L2052) | `src/Character.asm` | 1 |
| 4164 | 0x1044 | `NMARINA_CHARACTER` | 2045 | Polygon Marina Character (L2053) | `src/Character.asm` | 1 |
| 4165 | 0x1045 | `MARINA_VICTORY_IMAGE_BOTTOM` | 2046 | Marina Ending Image 1 (L2054) | `src/SinglePlayer.asm` | 1 |
| 4166 | 0x1046 | `MARINA_VICTORY_IMAGE_TOP` | 2047 | Marina Ending Image 2 (L2055) |  | 0 |
| 4167 | 0x1047 | `DEDEDE_WALK_1` | 2048 | Dedede Walk 1 (L2056) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4168 | 0x1048 | `DEDEDE_WALK_2` | 2049 | Dedede Walk 2 (L2057) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4169 | 0x1049 | `DEDEDE_WALK_3` | 2050 | Dedede Walk 3 (L2058) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4170 | 0x104A | `DEDEDE_CLIFF_QUICK` | 2051 | Dedede Cliff Quick (L2059) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4171 | 0x104B | `DEDEDE_CLIFF_CLIMB_QUICK_1` | 2052 | Dedede Cliff Climb Quick 1 (L2060) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4172 | 0x104C | `DEDEDE_CLIFF_CLIMB_QUICK_2` | 2053 | Dedede Cliff Climb Quick 2 (L2061) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4173 | 0x104D | `DEDEDE_CLIFF_SLOW` | 2054 | Dedede Cliff Slow (L2062) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4174 | 0x104E | `DEDEDE_CLIFF_CLIMB_SLOW_1` | 2055 | Dedede Cliff Climb Slow 1 (L2063) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4175 | 0x104F | `DEDEDE_CLIFF_CLIMB_SLOW_2` | 2056 | Dedede Cliff Climb Slow 2 (L2064) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4176 | 0x1050 | `DEDEDE_CLIFF_ATTACK_QUICK_1` | 2057 | Dedede Cliff Quick Attack 1 (L2065) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4177 | 0x1051 | `DEDEDE_CLIFF_ATTACK_QUICK_2` | 2058 | Dedede Cliff Quick Attack 2 (L2066) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4178 | 0x1052 | `DEDEDE_CLIFF_ATTACK_SLOW_1` | 2059 | Dedede Cliff Slow Attack 1 (L2067) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4179 | 0x1053 | `DEDEDE_CLIFF_ATTACK_SLOW_2` | 2060 | Dedede Cliff Slow Attack 2 (L2068) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4180 | 0x1054 | `DEDEDE_CLIFF_ESCAPE_QUICK_1` | 2061 | Dedede Cliff Quick Escape 1 (L2069) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4181 | 0x1055 | `DEDEDE_CLIFF_ESCAPE_QUICK_2` | 2062 | Dedede Cliff Quick Escape 2 (L2070) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4182 | 0x1056 | `DEDEDE_CLIFF_ESCAPE_SLOW_1` | 2063 | Dedede Cliff Slow Escape 1 (L2071) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4183 | 0x1057 | `DEDEDE_CLIFF_ESCAPE_SLOW_2` | 2064 | Dedede Cliff Slow Escape 2 (L2072) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4184 | 0x1058 | `DEDEDE_DAMAGE_HIGH_1` | 2065 | Dedede DamageHigh1 (L2073) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4185 | 0x1059 | `DEDEDE_DAMAGE_HIGH_2` | 2066 | Dedede DamageHigh2 (L2074) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4186 | 0x105A | `DEDEDE_DAMAGE_HIGH_3` | 2067 | Dedede DamageHigh3 (L2075) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4187 | 0x105B | `DEDEDE_DAMAGE_MID_1` | 2068 | Dedede DamageMid1 (L2076) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4188 | 0x105C | `DEDEDE_DAMAGE_MID_2` | 2069 | Dedede DamageMid2 (L2077) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4189 | 0x105D | `DEDEDE_DAMAGE_MID_3` | 2070 | Dedede DamageMid3 (L2078) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4190 | 0x105E | `DEDEDE_DAMAGE_LOW_1` | 2071 | Dedede DamageLow1 (L2079) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4191 | 0x105F | `DEDEDE_DAMAGE_LOW_2` | 2072 | Dedede DamageLow2 (L2080) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4192 | 0x1060 | `DEDEDE_DAMAGE_LOW_3` | 2073 | Dedede DamageLow3 (L2081) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4193 | 0x1061 | `DEDEDE_DAMAGE_AIR_1` | 2074 | Dedede DamageAir1 (L2082) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4194 | 0x1062 | `DEDEDE_DAMAGE_AIR_2` | 2075 | Dedede DamageAir2 (L2083) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4195 | 0x1063 | `DEDEDE_DAMAGE_AIR_3` | 2076 | Dedede DamageAir3 (L2084) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4196 | 0x1064 | `DEDEDE_DAMAGE_FLY_HIGH` | 2077 | Dedede DamageFlyHigh (L2085) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4197 | 0x1065 | `DEDEDE_DAMAGE_FLY_MID` | 2078 | Dedede DamageFlyMid (L2086) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4198 | 0x1066 | `DEDEDE_DAMAGE_FLY_LOW` | 2079 | Dedede DamageFlyLow (L2087) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4199 | 0x1067 | `DEDEDE_DAMAGE_FLY_TOP` | 2080 | Dedede DamageFlyTop (L2088) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4200 | 0x1068 | `DEDEDE_DAMAGE_FLY_ROLL` | 2081 | Dedede DamageFlyRoll (L2089) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4201 | 0x1069 | `DEDEDE_TUMBLE` | 2082 | Dedede Tumble (L2090) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 20 |
| 4202 | 0x106A | `DEDEDE_DAMAGE_ELEC` | 2083 | Dedede DamageElec (L2091) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4203 | 0x106B | `DEDEDE_DOWN_BOUNCE_D` | 2084 | Dedede DownBounceD (L2092) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 6 |
| 4204 | 0x106C | `DEDEDE_DOWN_BOUNCE_U` | 2085 | Dedede DownBounceU (L2093) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4205 | 0x106D | `DEDEDE_DOWN_STAND_D` | 2086 | Dedede DownStandD (L2094) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 6 |
| 4206 | 0x106E | `DEDEDE_DOWN_STAND_U` | 2087 | Dedede DownStandU (L2095) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm`, `src/TwelveCharBattle.asm` | 6 |
| 4207 | 0x106F | `DEDEDE_TECH_F` | 2088 | Dedede TechF (L2096) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4208 | 0x1070 | `DEDEDE_TECH_B` | 2089 | Dedede TechB (L2097) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4209 | 0x1071 | `DEDEDE_DOWN_FORWARD_D` | 2090 | Dedede DownForwardD (L2098) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4210 | 0x1072 | `DEDEDE_DOWN_FORWARD_U` | 2091 | Dedede DownForwardU (L2099) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4211 | 0x1073 | `DEDEDE_DOWN_BACK_D` | 2092 | Dedede DownBackD (L2100) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4212 | 0x1074 | `DEDEDE_DOWN_BACK_U` | 2093 | Dedede DownBackU (L2101) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4213 | 0x1075 | `DEDEDE_DOWN_ATTACK_D` | 2094 | Dedede DownAttackD (L2102) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4214 | 0x1076 | `DEDEDE_DOWN_ATTACK_U` | 2095 | Dedede DownAttackU (L2103) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4215 | 0x1077 | `DEDEDE_TECH` | 2096 | Dedede Tech (L2104) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4216 | 0x1078 | `STG_DEDEDE_TARGET_HEADER` | 2097 | Dedede Break the Targets Header (L2105) |  | 0 |
| 4217 | 0x1079 | `STG_DEDEDE_TARGET` | 2098 | Dedede Break the Targets Stage (L2106) |  | 0 |
| 4218 | 0x107A | `DEDEDE_USP_CEILING_BONK` | 2099 | Dedede Up Special Ceiling Bonk (L2107) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4219 | 0x107B | `DEDEDE_LANDING_AIR_U` | 2100 | Dedede Up Air Landing (L2108) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4220 | 0x107C | `DEDEDE_LANDING_AIR_X` | 2101 | Dedede Neutral/Forward Air Landing (L2109) |  | 0 |
| 4221 | 0x107D | `DEDEDE_LANDING_AIR_B` | 2102 | Dedede Back Air Landing (L2110) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4222 | 0x107E | `DEDEDE_LANDING_AIR_D` | 2103 | Dedede Down Air Landing (L2111) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4223 | 0x107F | `DEDEDE_ENTER_PIPE` | 2104 | Dedede Pipe Enter (L2112) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4224 | 0x1080 | `DEDEDE_EXIT_PIPE` | 2105 | Dedede Pipe Exit (L2113) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4225 | 0x1081 | `DEDEDE_EXIT_PIPE_WALK` | 2106 | Dedede Pipe Exit Walk (L2114) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4226 | 0x1082 | `KIRBY_MARINA_NSPG` | 2107 | Kirby Marina Neutral Special Ground (L2115) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4227 | 0x1083 | `KIRBY_MARINA_NSPG_PULL` | 2108 | Kirby Marina Neutral Special Ground Pull (L2116) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4228 | 0x1084 | `KIRBY_MARINA_NSPG_THROW` | 2109 | Kirby Marina Neutral Special Ground Throw (L2117) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4229 | 0x1085 | `KIRBY_MARINA_NSPG_THROW_U` | 2110 | Kirby Marina Neutral Special Ground Up Throw (L2118) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4230 | 0x1086 | `KIRBY_MARINA_NSPG_THROW_D` | 2111 | Kirby Marina Neutral Special Ground Down Throw (L2119) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4231 | 0x1087 | `KIRBY_MARINA_NSPA` | 2112 | Kirby Marina Neutral Special Air (L2120) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4232 | 0x1088 | `KIRBY_MARINA_NSPA_PULL` | 2113 | Kirby Marina Neutral Special Air Pull (L2121) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4233 | 0x1089 | `KIRBY_MARINA_NSPA_THROW` | 2114 | Kirby Marina Neutral Special Air Throw (L2122) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4234 | 0x108A | `KIRBY_MARINA_NSPA_THROW_U` | 2115 | Kirby Marina Neutral Special Air Up Throw (L2123) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4235 | 0x108B | `KIRBY_MARINA_NSPA_THROW_D` | 2116 | Kirby Marina Neutral Special Air Down Throw (L2124) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4236 | 0x108C | `DEDEDE_1P_POSE` | 2117 | Dedede 1P Pose (L2125) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4237 | 0x108D | `DEDEDE_CPU_POSE` | 2118 | Dedede CPU Pose (L2126) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4238 | 0x108E | `DEDEDE_CLIFF_CATCH` | 2119 | Dedede Cliff Catch (L2127) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4239 | 0x108F | `DEDEDE_CLIFF_WAIT` | 2120 | Dedede Cliff Wait (L2128) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4240 | 0x1090 | `DEDEDE_ITEM_THROW_DASH` | 2121 | Dedede ItemThrowDash (L2129) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4241 | 0x1091 | `DEDEDE_ITEM_THROW` | 2122 | Dedede ItemThrowF (L2130) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4242 | 0x1092 | `DEDEDE_ITEM_THROW_U` | 2123 | Dedede ItemThrowU (L2131) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4243 | 0x1093 | `DEDEDE_ITEM_THROW_D` | 2124 | Dedede ItemThrowD (L2132) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4244 | 0x1094 | `DEDEDE_ITEM_THROW_AIR` | 2125 | Dedede ItemThrowAirF (L2133) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4245 | 0x1095 | `DEDEDE_ITEM_THROW_AIR_U` | 2126 | Dedede ItemThrowAirU (L2134) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4246 | 0x1096 | `DEDEDE_ITEM_THROW_AIR_D` | 2127 | Dedede ItemThrowAirD (L2135) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4247 | 0x1097 | `DRACULAS_CASTLE_HEADER` | 2128 | Draculas Castle Header (L2136) |  | 0 |
| 4248 | 0x1098 | `DRACULAS_CASTLE_STAGE` | 2129 | Draculas Castle Stage (L2137) |  | 0 |
| 4249 | 0x1099 | `DRACULAS_CASTLE_BG` | 2130 | Draculas Castle BG (L2138) |  | 0 |
| 4250 | 0x109A | `DEDEDE_CAPTURE_PULLED` | 2131 | Dedede CapturePulled (L2139) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4251 | 0x109B | `DEDEDE_THROWN_DK_PULLED` | 2132 | Dedede ThrownDKPulled (L2140) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4252 | 0x109C | `DEDEDE_THROWN_MARIO_BROS` | 2133 | Dedede ThrownMarioBros (L2141) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4253 | 0x109D | `DEDEDE_THROWN_DK` | 2134 | Dedede ThrownDK (L2142) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4254 | 0x109E | `DEDEDE_THROWN_1` | 2135 | Dedede Thrown1 (L2143) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4255 | 0x109F | `DEDEDE_THROWN_2` | 2136 | Dedede Thrown2 (L2144) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4256 | 0x10A0 | `DEDEDE_HEAVY_ITEM_PICKUP` | 2137 | Dedede HeavyItemPickup (L2145) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4257 | 0x10A1 | `DEDEDE_HEAVY_ITEM_THROW` | 2138 | Dedede HeavyItemThrow (L2146) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4258 | 0x10A2 | `DEDEDE_TEETER` | 2139 | Dedede Teeter (L2147) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4259 | 0x10A3 | `DEDEDE_TEETER_START` | 2140 | Dedede TeeterStart (L2148) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4260 | 0x10A4 | `INVERTED_CASTLE_HEADER` | 2141 | Reverse Castle Header (L2149) |  | 0 |
| 4261 | 0x10A5 | `INVERTED_CASTLE_STAGE` | 2142 | Reverse Castle Stage (L2150) |  | 0 |
| 4262 | 0x10A6 | `DEDEDE_ITEM_NEUTRAL` | 2143 | Dedede ItemNeutral (L2151) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4263 | 0x10A7 | `DEDEDE_ITEM_TILT` | 2144 | Dedede ItemTilt (L2152) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4264 | 0x10A8 | `DEDEDE_ITEM_SMASH` | 2145 | Dedede ItemSmash (L2153) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4265 | 0x10A9 | `DEDEDE_ITEM_DASH_ATTACK` | 2146 | Dedede ItemDash (L2154) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 8 |
| 4266 | 0x10AA | `DEDEDE_ITEM_DROP` | 2147 | Dedede ItemDrop (L2155) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4267 | 0x10AB | `DEDEDE_LIGHT_ITEM_PICKUP` | 2148 | Dedede LightItemPickup (L2156) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4268 | 0x10AC | `DEDEDE_CEILINGBONK` | 2149 | Dedede CeilingBonk (L2157) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4269 | 0x10AD | `KIRBY_DEDEDE_NSP_INHALED_WALK` | 2150 | Kirby Dedede NSP Inhaled Walk (L2158) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 6 |
| 4270 | 0x10AE | `STG_DEDEDE_PLATFORM_HEADER` | 2151 | King Dedede Platforms Header (L2159) |  | 0 |
| 4271 | 0x10AF | `STG_DEDEDE_PLATFORM` | 2152 | King Dedede Platforms Stage (L2160) |  | 0 |
| 4272 | 0x10B0 | `DEDEDE_STARRING_LEFT` | 2153 | Dedede Starring Left (L2161) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4273 | 0x10B1 | `DEDEDE_STARRING_RIGHT` | 2154 | Dedede Starring Right (L2162) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4274 | 0x10B2 | `MARINA_ENTRY_GFX` | 2155 | Marina Entry GFX (L2163) | `src/Character.asm` | 1 |
| 4275 | 0x10B3 | `MARINA_ENTRY` | 2156 | Marina Entry (L2164) | `src/Marina/Marina.asm` | 2 |
| 4276 | 0x10B4 | `DK_THROWN_MARINA` | 2157 | DK ThrownMarina (L2165) | `src/Marina/Marina.asm` | 4 |
| 4277 | 0x10B5 | `FALCON_THROWN_MARINA` | 2158 | C.Falcon ThrownMarina (L2166) | `src/Marina/Marina.asm` | 9 |
| 4278 | 0x10B6 | `DEDEDE_CLANG` | 2159 | Dedede ClangRecoil (L2167) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4279 | 0x10B7 | `DEDEDE_STUN` | 2160 | Dedede Stun (L2168) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 4280 | 0x10B8 | `DEDEDE_CLAP` | 2161 | Dedede Clap (L2169) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4281 | 0x10B9 | `DEDEDE_TEAM_POSE` | 2162 | Dedede Team Pose (L2170) | `src/SinglePlayerModes.asm` | 3 |
| 4282 | 0x10BA | `STG_MT_DEDEDE_HEADER` | 2163 | Mt. Dedede Header (L2171) |  | 0 |
| 4283 | 0x10BB | `STG_MT_DEDEDE_STAGE` | 2164 | Mt. Dedede Stage (L2172) |  | 0 |
| 4284 | 0x10BC | `STG_MT_DEDEDE_BG` | 2165 | Mt. Dedede BG (L2173) |  | 0 |
| 4285 | 0x10BD | `DEDEDE_VICTORY_2` | 2166 | Dedede Victory 2 (L2174) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4286 | 0x10BE | `DEDEDE_VICTORY_IMAGE_BOTTOM` | 2167 | Dedede Ending Image 1 (L2175) | `src/SinglePlayer.asm` | 1 |
| 4287 | 0x10BF | `DEDEDE_VICTORY_IMAGE_TOP` | 2168 | Dedede Ending Image 2 (L2176) |  | 0 |
| 4288 | 0x10C0 | `DEDEDE_VICTORY_3` | 2169 | Dedede Victory 3 (L2177) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 4289 | 0x10C1 | `GOEMON_MAIN` | 2170 | Goemon Main (L2178) | `src/Character.asm` | 1 |
| 4290 | 0x10C2 | `GOEMON_CHARACTER` | 2171 | Goemon Character (L2179) | `src/Character.asm` | 1 |
| 4291 | 0x10C3 | `GOEMON_DSPG` | 2172 | Goemon DSPGround (L2180) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4292 | 0x10C4 | `GOEMON_DSP_PULL` | 2173 | Goemon DSPPull (L2181) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 4 |
| 4293 | 0x10C5 | `GOEMON_UTILT` | 2174 | Goemon Up Tilt (L2182) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4294 | 0x10C6 | `GOEMON_UAIR` | 2175 | Goemon Up Aerial (L2183) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4295 | 0x10C7 | `GOEMON_BAIR` | 2176 | Goemon Back Aerial (L2184) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4296 | 0x10C8 | `GOEMON_NAIR` | 2177 | Goemon Neutral Aerial (L2185) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4297 | 0x10C9 | `GOEMON_FAIR` | 2178 | Goemon Forward Aerial (L2186) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4298 | 0x10CA | `GOEMON_DAIR` | 2179 | Goemon Down Aerial (L2187) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4299 | 0x10CB | `GOEMON_DSPA` | 2180 | Goemon DSPAir (L2188) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4300 | 0x10CC | `GOEMON_DSPG_ATTACK` | 2181 | Goemon DSPGroundAttack (L2189) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4301 | 0x10CD | `GOEMON_DSPA_ATTACK` | 2182 | Goemon DSPAirAttack (L2190) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4302 | 0x10CE | `GOEMON_DSP_END` | 2183 | Goemon DSPEnd (L2191) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4303 | 0x10CF | `GOEMON_USMASH` | 2184 | Goemon Up Smash (L2192) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4304 | 0x10D0 | `GOEMON_RYO_HITBOX` | 2185 | Goemon Ryo Hitbox (L2193) | `src/Character.asm`, `src/TagTeam.asm` | 4 |
| 4305 | 0x10D1 | `GOEMON_RYO_GRAPHIC` | 2186 | Goemon Ryo Graphic (L2194) | `src/Character.asm` | 3 |
| 4306 | 0x10D2 | `GOEMON_RYO_FIRE_GRAPHIC` | 2187 | Goemon Ryo Fire Graphic (L2195) |  | 0 |
| 4307 | 0x10D3 | `GOEMON_RYO_SPARKLE_GRAPHIC` | 2188 | Goemon Ryo Sparkle Graphic (L2196) |  | 0 |
| 4308 | 0x10D4 | `GOEMON_IDLE` | 2189 | Goemon Idle (L2197) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 20 |
| 4309 | 0x10D5 | `GOEMON_FALL` | 2190 | Goemon Fall (L2198) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4310 | 0x10D6 | `GOEMON_FALL_AERIAL` | 2191 | Goemon Fall Aerial (L2199) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4311 | 0x10D7 | `GOEMON_JUMP_F` | 2192 | Goemon Jump Forward (L2200) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4312 | 0x10D8 | `GOEMON_JUMP_AIR_F` | 2193 | Goemon Jump Air Forward (L2201) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4313 | 0x10D9 | `GOEMON_JUMP_AIR_B` | 2194 | Goemon Jump Air Back (L2202) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4314 | 0x10DA | `GOEMON_JUMP_B` | 2195 | Goemon Jump Backward (L2203) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4315 | 0x10DB | `GOEMON_CROUCH_BEGIN` | 2196 | Goemon Crouch Begin (L2204) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4316 | 0x10DC | `GOEMON_CROUCH_IDLE` | 2197 | Goemon Crouch Idle (L2205) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4317 | 0x10DD | `GOEMON_CROUCH_END` | 2198 | Goemon Crouch End (L2206) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4318 | 0x10DE | `GOEMON_NSPG_BEGIN` | 2199 | Goemon NSPGroundBegin (L2207) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 6 |
| 4319 | 0x10DF | `GOEMON_NSPG_IDLE` | 2200 | Goemon NSPGroundIdle (L2208) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 6 |
| 4320 | 0x10E0 | `GOEMON_NSPG_END` | 2201 | Goemon NSPGroundEnd (L2209) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4321 | 0x10E1 | `GOEMON_NSPG_WALK_1` | 2202 | Goemon NSPGroundWalk1 (L2210) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4322 | 0x10E2 | `GOEMON_NSPG_WALK_2` | 2203 | Goemon NSPGroundWalk2 (L2211) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4323 | 0x10E3 | `GOEMON_NSPG_BWALK_1` | 2204 | Goemon NSPGroundBackWalk1 (L2212) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4324 | 0x10E4 | `GOEMON_NSPG_BWALK_2` | 2205 | Goemon NSPGroundBackWalk2 (L2213) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4325 | 0x10E5 | `GOEMON_DTILT` | 2206 | Goemon Down Tilt (L2214) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4326 | 0x10E6 | `GOEMON_FTILT_HIGH` | 2207 | Goemon Forward Tilt High (L2215) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4327 | 0x10E7 | `GOEMON_FTILT` | 2208 | Goemon Forward Tilt (L2216) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4328 | 0x10E8 | `GOEMON_FTILT_LOW` | 2209 | Goemon Forward Tilt Low (L2217) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4329 | 0x10E9 | `GOEMON_1P_CPU` | 2210 | Goemon CPU Pose (L2218) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4330 | 0x10EA | `GOEMON_JAB1` | 2211 | Goemon Jab 1 (L2219) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4331 | 0x10EB | `GOEMON_JAB2` | 2212 | Goemon Jab 2 (L2220) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4332 | 0x10EC | `GOEMON_JAB3` | 2213 | Goemon Jab 3 (L2221) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4333 | 0x10ED | `GOEMON_TAUNT` | 2214 | Goemon Taunt (L2222) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4334 | 0x10EE | `GOEMON_1P_POSE` | 2215 | Goemon 1P Pose (L2223) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4335 | 0x10EF | `GOEMON_FSMASH` | 2216 | Goemon Forward Smash (L2224) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4336 | 0x10F0 | `NFALCO_MAIN` | 2217 | Polygon Falco Main (L2225) | `src/Character.asm` | 1 |
| 4337 | 0x10F1 | `NFALCO_CHARACTER` | 2218 | Polygon Falco Character (L2226) | `src/Character.asm` | 1 |
| 4338 | 0x10F2 | `NGND_MAIN` | 2219 | Polygon Ganondorf Main (L2227) | `src/Character.asm` | 1 |
| 4339 | 0x10F3 | `NGND_CHARACTER` | 2220 | Polygon Ganondorf Character (L2228) | `src/Character.asm` | 1 |
| 4340 | 0x10F4 | `GOEMON_DSMASH` | 2221 | Goemon Down Smash (L2229) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4341 | 0x10F5 | `CHARACTER_BIOS_EXTENDED` | 2222 | Character Bios Extended (L2230) | `src/CharacterDataScreen.asm` | 1 |
| 4342 | 0x10F6 | `CHARACTER_IMAGES_EXTENDED` | 2223 | Character Images Extended (L2231) | `src/CharacterDataScreen.asm` | 1 |
| 4343 | 0x10F7 | `STG_EDO_HEADER` | 2224 | Edo Town Header (L2232) |  | 0 |
| 4344 | 0x10F8 | `STG_EDO_STAGE` | 2225 | Edo Town Stage (L2233) |  | 0 |
| 4345 | 0x10F9 | `STG_EDO_BG` | 2226 | Edo Town BG (L2234) |  | 0 |
| 4346 | 0x10FA | `GOEMON_DASH_ATTACK` | 2227 | Goemon Dash Attack (L2235) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4347 | 0x10FB | `STG_DEKU_DL_HEADER` | 2228 | Deku DL Header (L2236) |  | 0 |
| 4348 | 0x10FC | `STG_DEKU_DL_STAGE` | 2229 | Deku DL Stage (L2237) |  | 0 |
| 4349 | 0x10FD | `STG_ZEBES_LANDING_DL_HEADER` | 2230 | Zebes Landing DL Header (L2238) |  | 0 |
| 4350 | 0x10FE | `STG_ZEBES_LANDING_DL_STAGE` | 2231 | Zebes Landing Stage (L2239) |  | 0 |
| 4351 | 0x10FF | `GOEMON_CLOUD_INFO` | 2232 | Goemon Cloud Info (L2240) | `src/Character.asm`, `src/TagTeam.asm` | 4 |
| 4352 | 0x1100 | `GOEMON_CLOUD_GFX` | 2233 | Goemon Cloud Graphic (L2241) |  | 0 |
| 4353 | 0x1101 | `BULLET_BILL_GFX` | 2234 | Bullet Bill GFX (L2242) |  | 0 |
| 4354 | 0x1102 | `PEPPY_MAIN` | 2235 | Peppy Main (L2243) | `src/Character.asm` | 1 |
| 4355 | 0x1103 | `PEPPY_CHARACTER` | 2236 | Peppy Character (L2244) | `src/Character.asm` | 1 |
| 4356 | 0x1104 | `PEPPY_GUN` | 2237 | Peppy Gun (L2245) |  | 0 |
| 4357 | 0x1105 | `SLIPPY_MAIN` | 2238 | Slippy Main (L2246) | `src/Character.asm` | 1 |
| 4358 | 0x1106 | `SLIPPY_CHARACTER` | 2239 | Slippy Character (L2247) | `src/Character.asm` | 1 |
| 4359 | 0x1107 | `SLIPPY_LASER_HITBOX` | 2240 | Slippy Laser Hitbox (L2248) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 4360 | 0x1108 | `SLIPPY_LASER_GRAPHIC` | 2241 | Slippy Laser Graphic (L2249) | `src/Character.asm` | 1 |
| 4361 | 0x1109 | `SLIPPY_WEAPON` | 2242 | Slippy Weapon (L2250) |  | 0 |
| 4362 | 0x110A | `SLIPPY_GRAB_PULL` | 2243 | Slippy Grab Pull (L2251) | `src/Slippy/Slippy.asm` | 1 |
| 4363 | 0x110B | `SLIPPY_SELECT` | 2244 | Slippy Select (L2252) | `src/Slippy/Slippy.asm` | 2 |
| 4364 | 0x110C | `PEPPY_BARREL_ROLL` | 2245 | Peppy Barrel Roll (L2253) | `src/Peppy/Peppy.asm` | 2 |
| 4365 | 0x110D | `PEPPY_LASER_HITBOX` | 2246 | Peppy Laser Hitbox (L2254) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 4366 | 0x110E | `PEPPY_LASER_GFX` | 2247 | Peppy Laser GFX (L2255) | `src/Character.asm` | 1 |
| 4367 | 0x110F | `GOEMON_VICTORY_2` | 2248 | Goemon Victory 2 (L2256) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4368 | 0x1110 | `PEPPY_VICTORY_1` | 2249 | Peppy Victory 1 (L2257) | `src/Peppy/Peppy.asm` | 1 |
| 4369 | 0x1111 | `STG_GOEMONTARGETS` | 2250 | Goemon Break the Targets Stage (L2258) |  | 0 |
| 4370 | 0x1112 | `STG_GOEMONTARGETS_HEADER` | 2251 | Goemon Break the Targets Header (L2259) |  | 0 |
| 4371 | 0x1113 | `SLIPPY_REFLECT_GFX` | 2252 | Slippy Reflect Graphic (L2260) | `src/Character.asm` | 1 |
| 4372 | 0x1114 | `NESS_DUO_POSE` | 2253 | Ness Duo Pose (L2261) | `src/SinglePlayerModes.asm` | 1 |
| 4373 | 0x1115 | `LUCAS_DUO_POSE` | 2254 | Lucas Duo Pose (L2262) | `src/SinglePlayerModes.asm` | 2 |
| 4374 | 0x1116 | `GOEMON_CLAP` | 2255 | Goemon Clap (L2263) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4375 | 0x1117 | `GOEMON_CSS` | 2256 | Goemon CSS (L2264) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 4 |
| 4376 | 0x1118 | `KIRBY_DUO_POSE` | 2257 | Kirby Duo Pose (L2265) | `src/SinglePlayerModes.asm` | 1 |
| 4377 | 0x1119 | `DEDEDE_DUO_POSE` | 2258 | Dedede Duo Pose (L2266) | `src/SinglePlayerModes.asm` | 1 |
| 4378 | 0x111A | `STG_FIRST_REMIX` | 2259 | First Destination Remix Stage (L2267) |  | 0 |
| 4379 | 0x111B | `STG_FIRST_REMIX_HEADER` | 2260 | First Destination Remix Header (L2268) |  | 0 |
| 4380 | 0x111C | `STG_GOEMON_PLATFORMS_HEADER` | 2261 | Goemon Board the Platforms Header (L2269) |  | 0 |
| 4381 | 0x111D | `STG_GOEMON_PLATFORMS_STAGE` | 2262 | Goemon Board the Platforms Stage (L2270) |  | 0 |
| 4382 | 0x111E | `GOEMON_GRAB` | 2263 | Goemon Grab (L2271) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4383 | 0x111F | `GOEMON_GRAB_PULL` | 2264 | Goemon Grab Pull (L2272) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4384 | 0x1120 | `GOEMON_THROW_FORWARD` | 2265 | Goemon Forward Throw (L2273) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4385 | 0x1121 | `GOEMON_THROW_BACKWARD` | 2266 | Goemon Back Throw (L2274) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4386 | 0x1122 | `PEPPY_CSS` | 2267 | Peppy Character Select (L2275) | `src/Peppy/Peppy.asm` | 2 |
| 4387 | 0x1123 | `STG_TWILIGHT_CITY` | 2268 | Twilight City Stage (L2276) |  | 0 |
| 4388 | 0x1124 | `STG_TWILIGHT_CITY_HEADER` | 2269 | Twilight City Header (L2277) |  | 0 |
| 4389 | 0x1125 | `STG_TWILIGHT_CITY_BG` | 2270 | Twilight City BG (L2278) |  | 0 |
| 4390 | 0x1126 | `GOEMON_CLIFF_ATTACK_QUICK_1` | 2271 | Goemon CliffAttackQuick1 (L2279) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4391 | 0x1127 | `GOEMON_CLIFF_ATTACK_QUICK_2` | 2272 | Goemon CliffAttackQuick2 (L2280) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4392 | 0x1128 | `GOEMON_CLIFF_ATTACK_SLOW_1` | 2273 | Goemon CliffAttackSlow1 (L2281) |  | 0 |
| 4393 | 0x1129 | `GOEMON_CLIFF_ATTACK_SLOW_2` | 2274 | Goemon CliffAttackSlow2 (L2282) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4394 | 0x112A | `GOEMON_CLIFF_CLIMB_QUICK_2` | 2275 | Goemon CliffClimbQuick2 (L2283) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4395 | 0x112B | `GOEMON_CLIFF_CLIMB_SLOW_2` | 2276 | Goemon CliffClimbSlow2 (L2284) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4396 | 0x112C | `GOEMON_CLIFF_ESCAPE_QUICK_2` | 2277 | Goemon CliffEscapeQuick2 (L2285) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4397 | 0x112D | `GOEMON_CLIFF_ESCAPE_SLOW_1` | 2278 | Goemon CliffEscapeSlow1 (L2286) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4398 | 0x112E | `GOEMON_CLIFF_ESCAPE_SLOW_2` | 2279 | Goemon CliffEscapeSlow2 (L2287) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4399 | 0x112F | `GOEMON_CLIFF_WAIT` | 2280 | Goemon CliffWait (L2288) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4400 | 0x1130 | `PEPPY_DASH` | 2281 | Peppy Dash (L2289) | `src/Peppy/Peppy.asm` | 1 |
| 4401 | 0x1131 | `PEPPY_RUN` | 2282 | Peppy Run (L2290) | `src/Peppy/Peppy.asm` | 1 |
| 4402 | 0x1132 | `PEPPY_RUN_BRAKE` | 2283 | Peppy RunBrake (L2291) | `src/Peppy/Peppy.asm` | 1 |
| 4403 | 0x1133 | `PEPPY_TURN_RUN` | 2284 | Peppy TurnRun (L2292) | `src/Peppy/Peppy.asm` | 1 |
| 4404 | 0x1134 | `STG_MELRODE` | 2285 | Melrode Stage (L2293) |  | 0 |
| 4405 | 0x1135 | `STG_MELRODE_HEADER` | 2286 | Melrode Header (L2294) |  | 0 |
| 4406 | 0x1136 | `STG_MELRODE_BG` | 2287 | Melrode BG (L2295) |  | 0 |
| 4407 | 0x1137 | `PEPPY_GRENADE_HITBOX` | 2288 | Peppy Grenade Hitbox (L2296) | `src/TagTeam.asm` | 1 |
| 4408 | 0x1138 | `PEPPY_GRENADE_GRAPHIC` | 2289 | Peppy Grenade Graphic (L2297) |  | 0 |
| 4409 | 0x1139 | `PEPPY_DASH_ATTACK` | 2290 | Peppy Dash Attack (L2298) | `src/Peppy/Peppy.asm` | 1 |
| 4410 | 0x113A | `PEPPY_UTILT` | 2291 | Peppy Up Tilt (L2299) | `src/Peppy/Peppy.asm` | 1 |
| 4411 | 0x113B | `PEPPY_FAIR` | 2292 | Peppy Forward Air (L2300) | `src/Peppy/Peppy.asm` | 1 |
| 4412 | 0x113C | `PEPPY_BAIR` | 2293 | Peppy Back Air (L2301) | `src/Peppy/Peppy.asm` | 1 |
| 4413 | 0x113D | `PEPPY_UAIR` | 2294 | Peppy Up Air (L2302) | `src/Peppy/Peppy.asm` | 1 |
| 4414 | 0x113E | `LINK_DUO_POSE` | 2295 | Link Duo Pose (L2303) | `src/SinglePlayerModes.asm` | 1 |
| 4415 | 0x113F | `SHEIK_DUO_POSE` | 2296 | Sheik Duo Pose (L2304) | `src/SinglePlayerModes.asm` | 1 |
| 4416 | 0x1140 | `PEPPY_1P_POSE` | 2297 | Peppy 1P Pose (L2305) | `src/Peppy/Peppy.asm` | 1 |
| 4417 | 0x1141 | `SLIPPY_1P_POSE` | 2298 | Slippy 1P Pose (L2306) | `src/Slippy/Slippy.asm` | 1 |
| 4418 | 0x1142 | `GOEMON_DOWN_STAND_D` | 2299 | Goemon DownStandD (L2307) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4419 | 0x1143 | `GOEMON_DOWN_BACK_U` | 2300 | Goemon DownBackU (L2308) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4420 | 0x1144 | `GOEMON_DOWN_FORWARD_D` | 2301 | Goemon DownForwardD (L2309) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4421 | 0x1145 | `GOEMON_DOWN_BACK_D` | 2302 | Goemon DownBackD (L2310) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4422 | 0x1146 | `GOEMON_DOWN_FORWARD_U` | 2303 | Goemon DownForwardU (L2311) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4423 | 0x1147 | `GOEMON_DOWN_STAND_U` | 2304 | Goemon DownStandU (L2312) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` +1 | 6 |
| 4424 | 0x1148 | `GOEMON_DOWN_ATTACK_U` | 2305 | Goemon DownAttackU (L2313) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4425 | 0x1149 | `GOEMON_DOWN_ATTACK_D` | 2306 | Goemon DownAttackD (L2314) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4426 | 0x114A | `FALCO_CPU_POSE` | 2307 | Falco CPU Pose (L2315) | `src/Falco/Falco.asm` | 1 |
| 4427 | 0x114B | `SAMUS_DUO_POSE` | 2308 | Samus Duo Pose (L2316) | `src/SinglePlayerModes.asm` | 1 |
| 4428 | 0x114C | `DARK_SAMUS_DUO_POSE` | 2309 | Dark Samus Duo Pose (L2317) | `src/SinglePlayerModes.asm` | 1 |
| 4429 | 0x114D | `GOEMON_JUMPSQUAT` | 2310 | Goemon JumpSquat (L2318) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 27 |
| 4430 | 0x114E | `GOEMON_DASH` | 2311 | Goemon Dash (L2319) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4431 | 0x114F | `GOEMON_RUN` | 2312 | Goemon Run (L2320) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4432 | 0x1150 | `GOEMON_RUN_BRAKE` | 2313 | Goemon RunBrake (L2321) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4433 | 0x1151 | `GOEMON_TURN_RUN` | 2314 | Goemon TurnRun (L2322) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4434 | 0x1152 | `GOEMON_STUN` | 2315 | Goemon Stun (L2323) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 6 |
| 4435 | 0x1153 | `GOEMON_ENTER_PIPE` | 2316 | Goemon EnterPipe (L2324) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4436 | 0x1154 | `GOEMON_EXIT_PIPE` | 2317 | Goemon ExitPipe (L2325) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4437 | 0x1155 | `PIKA_DUO_POSE` | 2318 | Pikachu Duo Pose (L2326) | `src/SinglePlayerModes.asm` | 1 |
| 4438 | 0x1156 | `MTWO_DUO_POSE` | 2319 | Mewtwo Duo Pose (L2327) | `src/SinglePlayerModes.asm` | 1 |
| 4439 | 0x1157 | `GOEMON_TEAM_POSE` | 2320 | Goemon Team Pose (L2328) | `src/SinglePlayerModes.asm` | 4 |
| 4440 | 0x1158 | `FALCO_TEAM_POSE` | 2321 | Falco Team Pose (L2329) | `src/SinglePlayerModes.asm` | 3 |
| 4441 | 0x1159 | `GOEMON_VICTORY_IMAGE_BOTTOM` | 2322 | Goemon Ending Image 1 (L2330) | `src/SinglePlayer.asm` | 1 |
| 4442 | 0x115A | `GOEMON_VICTORY_IMAGE_TOP` | 2323 | Goemon Ending Image 2 (L2331) |  | 0 |
| 4443 | 0x115B | `KIRBY_GOEMON_RYO` | 2324 | Kirby Goemon Ryo (L2332) |  | 0 |
| 4444 | 0x115C | `GOEMON_PUPPET_UP` | 2325 | Goemon Puppet Up (L2333) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4445 | 0x115D | `GOEMON_USP_LOOP` | 2326 | Goemon Up Special Loop (L2334) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4446 | 0x115E | `GOEMON_USP_ATTACK` | 2327 | Goemon Up Special Attack (L2335) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4447 | 0x115F | `GOEMON_USP_JUMP` | 2328 | Goemon Up Special Jump (L2336) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4448 | 0x1160 | `GOEMON_SFALL` | 2329 | Goemon Special Fall (L2337) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4449 | 0x1161 | `GOEMON_ENTRY_GFX` | 2330 | Goemon Entry GFX (L2338) | `src/Character.asm` | 3 |
| 4450 | 0x1162 | `KIRBY_GOEMON_NSPG_BEGIN` | 2331 | Kirby Goemon NSP Ground Begin (L2339) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 8 |
| 4451 | 0x1163 | `KIRBY_GOEMON_NSPG_IDLE` | 2332 | Kirby Goemon NSP Ground Idle (L2340) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 8 |
| 4452 | 0x1164 | `KIRBY_GOEMON_NSPG_END` | 2333 | Kirby Goemon NSP Ground End (L2341) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4453 | 0x1165 | `KIRBY_GOEMON_NSPG_WALK_1` | 2334 | Kirby Goemon NSP Ground Walk1 (L2342) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4454 | 0x1166 | `KIRBY_GOEMON_NSPG_WALK_2` | 2335 | Kirby Goemon NSP Ground Walk2 (L2343) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4455 | 0x1167 | `KIRBY_GOEMON_NSPG_BWALK_1` | 2336 | Kirby Goemon NSP Ground BackWalk1 (L2344) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4456 | 0x1168 | `KIRBY_GOEMON_NSPG_BWALK_2` | 2337 | Kirby Goemon NSP Ground BackWalk2 (L2345) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4457 | 0x1169 | `GOEMON_USP_ESCAPE` | 2338 | Goemon Up Special Escape (L2346) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4458 | 0x116A | `BOWSER_DUO_POSE` | 2339 | Bowser Duo Pose (L2347) | `src/SinglePlayerModes.asm` | 2 |
| 4459 | 0x116B | `DK_DUO_POSE` | 2340 | Donkey Kong Duo Pose (L2348) | `src/SinglePlayerModes.asm` | 1 |
| 4460 | 0x116C | `PEPPY_DSP_GROUND` | 2341 | Peppy DSP Throw Ground (L2349) | `src/Peppy/Peppy.asm` | 1 |
| 4461 | 0x116D | `PEPPY_DSP_AIR` | 2342 | Peppy DSP Throw Air (L2350) | `src/Peppy/Peppy.asm` | 1 |
| 4462 | 0x116E | `PEPPY_DSP_IGNITE_GROUND` | 2343 | Peppy DSP Detonate Ground (L2351) | `src/Peppy/Peppy.asm` | 1 |
| 4463 | 0x116F | `PEPPY_DSP_IGNITE_AIR` | 2344 | Peppy DSP Detonate Air (L2352) | `src/Peppy/Peppy.asm` | 1 |
| 4464 | 0x1170 | `PEPPY_TAUNT` | 2345 | Peppy Taunt (L2353) | `src/Peppy/Peppy.asm` | 1 |
| 4465 | 0x1171 | `PEPPY_NSP_CHARGESTART` | 2346 | Peppy NSP Charge Start (L2354) | `src/Peppy/Peppy.asm` | 1 |
| 4466 | 0x1172 | `PEPPY_NSP_CHARGELOOP` | 2347 | Peppy NSP Charge Loop (L2355) | `src/Peppy/Peppy.asm` | 1 |
| 4467 | 0x1173 | `KIRBY_PEPPY_NSP_CHARGESTART` | 2348 | Kirby Peppy NSP Charge Start (L2356) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4468 | 0x1174 | `KIRBY_PEPPY_NSP_CHARGELOOP` | 2349 | Kirby Peppy NSP Charge Loop (L2357) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4469 | 0x1175 | `GOEMON_VICTORY_3` | 2350 | Goemon Victory Sudden Impact (L2358) | `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 2 |
| 4470 | 0x1176 | `PEPPY_SLIPPY_VICTORY_IMAGE_BOTTOM` | 2351 | Peppy and Slippy Ending Image 1 (L2359) | `src/SinglePlayer.asm` | 2 |
| 4471 | 0x1177 | `PEPPY_SLIPPY_VICTORY_IMAGE_TOP` | 2352 | Peppy and Slippy Ending Image 2 (L2360) |  | 0 |
| 4472 | 0x1178 | `GOEMON_TEETER` | 2353 | Goemon Teeter (L2361) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4473 | 0x1179 | `GOEMON_TEETER_START` | 2354 | Goemon Teeter Start (L2362) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4474 | 0x117A | `GOEMON_ENTRY` | 2355 | Goemon Entry (L2363) | `src/Goemon/Goemon.asm` | 2 |
| 4475 | 0x117B | `SLIPPY_DUO_POSE` | 2356 | Slippy Duo Pose (L2364) | `src/SinglePlayerModes.asm` | 1 |
| 4476 | 0x117C | `PEPPY_DUO_POSE` | 2357 | Peppy Duo Pose (L2365) | `src/SinglePlayerModes.asm` | 1 |
| 4477 | 0x117D | `GOEMON_SHIELD_POSE` | 2358 | Goemon Shield Pose (L2366) | `src/Character.asm` | 3 |
| 4478 | 0x117E | `GOEMON_SHIELD_ON` | 2359 | Goemon Shield On (L2367) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4479 | 0x117F | `GOEMON_SHIELD_OFF` | 2360 | Goemon Shield Off (L2368) | `src/Ebi/Ebi.asm`, `src/Goemon/Goemon.asm`, `src/NGoemon/NGoemon.asm` | 3 |
| 4480 | 0x1180 | `PEPPY_NSP_CHARGESTART_AIR` | 2361 | Peppy NSP Charge Start Air (L2369) | `src/Peppy/Peppy.asm` | 1 |
| 4481 | 0x1181 | `PEPPY_NSP_CHARGELOOP_AIR` | 2362 | Peppy NSP Charge Loop Air (L2370) | `src/Peppy/Peppy.asm` | 1 |
| 4482 | 0x1182 | `PEPPY_NSP_SHOOT_AIR` | 2363 | Peppy NSP Shoot Air (L2371) | `src/Peppy/Peppy.asm` | 1 |
| 4483 | 0x1183 | `KIRBY_PEPPY_NSP_SHOOT_AIR` | 2364 | Kirby Peppy NSP Shoot Air (L2372) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4484 | 0x1184 | `KIRBY_PEPPY_NSP_CHARGESTART_AIR` | 2365 | Kirby Peppy NSP Charge Start Air (L2373) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4485 | 0x1185 | `KIRBY_PEPPY_NSP_CHARGELOOP_AIR` | 2366 | Kirby Peppy NSP Charge Loop Air (L2374) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4486 | 0x1186 | `META_REMIX_STAGE` | 2367 | Meta Crystal Remix Stage (L2375) |  | 0 |
| 4487 | 0x1187 | `META_REMIX_HEADER` | 2368 | Meta Crystal Remix Header (L2376) |  | 0 |
| 4488 | 0x1188 | `DARK_SAMUS_IDLE` | 2369 | Dark Samus Idle (L2377) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 12 |
| 4489 | 0x1189 | `REMIX_RTTF_HEADER` | 2370 | Remix 1p Race to the Finish Header (L2378) |  | 0 |
| 4490 | 0x118A | `REMIX_RTTF_STAGE` | 2371 | Remix 1p Race to the Finish Stage (L2379) |  | 0 |
| 4491 | 0x118B | `REMIX_RTTF_FILE` | 2372 | Remix 1p Race to the Finish File (L2380) |  | 0 |
| 4492 | 0x118C | `GRIM_REAPERS_CAVERN_BG` | 2373 | Grim Reapers Cavern BG (L2381) |  | 0 |
| 4493 | 0x118D | `GRIM_REAPERS_CAVERN_STAGE` | 2374 | Grim Reapers Cavern Stage (L2382) |  | 0 |
| 4494 | 0x118E | `GRIM_REAPERS_CAVERN_HEADER` | 2375 | Grim Reapers Cavern Header (L2383) |  | 0 |
| 4495 | 0x118F | `BANJO_MAIN` | 2376 | Banjo Main (L2384) | `src/Character.asm` | 1 |
| 4496 | 0x1190 | `BANJO_CHARACTER` | 2377 | Banjo Character (L2385) | `src/Character.asm` | 1 |
| 4497 | 0x1191 | `BANJO_IDLE` | 2378 | Banjo Idle (L2386) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 14 |
| 4498 | 0x1192 | `BANJO_WALK_1` | 2379 | Banjo Walk1 (L2387) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4499 | 0x1193 | `BANJO_WALK_2` | 2380 | Banjo Walk2 (L2388) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4500 | 0x1194 | `BANJO_WALK_3` | 2381 | Banjo Walk3 (L2389) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4501 | 0x1195 | `BANJO_WALK_END` | 2382 | Banjo WalkEnd (L2390) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4502 | 0x1196 | `BANJO_DASH` | 2383 | Banjo Dash (L2391) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4503 | 0x1197 | `BANJO_RUN` | 2384 | Banjo Run (L2392) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4504 | 0x1198 | `BANJO_RUN_BRAKE` | 2385 | Banjo RunBrake (L2393) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4505 | 0x1199 | `BANJO_TURN` | 2386 | Banjo Turn (L2394) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4506 | 0x119A | `BANJO_TURN_RUN` | 2387 | Banjo TurnRun (L2395) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4507 | 0x119B | `BANJO_JUMP_F` | 2388 | Banjo JumpF (L2396) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4508 | 0x119C | `BANJO_JUMP_B` | 2389 | Banjo JumpB (L2397) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4509 | 0x119D | `BANJO_JUMP_AERIAL_F` | 2390 | Banjo JumpAerialF (L2398) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4510 | 0x119E | `BANJO_JUMP_AERIAL_B` | 2391 | Banjo JumpAerialB (L2399) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4511 | 0x119F | `BANJO_FALL` | 2392 | Banjo Fall (L2400) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4512 | 0x11A0 | `BANJO_FALL_AERIAL` | 2393 | Banjo FallAerial (L2401) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4513 | 0x11A1 | `BANJO_CROUCH` | 2394 | Banjo Crouch (L2402) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4514 | 0x11A2 | `BANJO_CROUCH_IDLE` | 2395 | Banjo CrouchIdle (L2403) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4515 | 0x11A3 | `BANJO_CROUCH_END` | 2396 | Banjo CrouchEnd (L2404) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4516 | 0x11A4 | `BANJO_LANDING` | 2397 | Banjo Landing (L2405) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 14 |
| 4517 | 0x11A5 | `BANJO_PLAT_DROP` | 2398 | Banjo PlatDrop (L2406) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4518 | 0x11A6 | `BANJO_TEETER` | 2399 | Banjo Teeter (L2407) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4519 | 0x11A7 | `BANJO_TEETERSTART` | 2400 | Banjo Teeterstart (L2408) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4520 | 0x11A8 | `BANJO_DMG_HIGH_1` | 2401 | Banjo DamageHigh1 (L2409) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4521 | 0x11A9 | `BANJO_DMG_HIGH_2` | 2402 | Banjo DamageHigh2 (L2410) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4522 | 0x11AA | `BANJO_DMG_HIGH_3` | 2403 | Banjo DamageHigh3 (L2411) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4523 | 0x11AB | `BANJO_DMG_MID_1` | 2404 | Banjo DamageMid1 (L2412) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4524 | 0x11AC | `BANJO_DMG_MID_2` | 2405 | Banjo DamageMid2 (L2413) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4525 | 0x11AD | `BANJO_DMG_MID_3` | 2406 | Banjo DamageMid3 (L2414) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4526 | 0x11AE | `BANJO_DMG_LOW_1` | 2407 | Banjo DamageLow1 (L2415) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4527 | 0x11AF | `BANJO_DMG_LOW_2` | 2408 | Banjo DamageLow2 (L2416) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4528 | 0x11B0 | `BANJO_DMG_LOW_3` | 2409 | Banjo DamageLow3 (L2417) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4529 | 0x11B1 | `BANJO_DMG_AIR_1` | 2410 | Banjo DamageAir1 (L2418) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4530 | 0x11B2 | `BANJO_DMG_AIR_2` | 2411 | Banjo DamageAir2 (L2419) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4531 | 0x11B3 | `BANJO_DMG_AIR_3` | 2412 | Banjo DamageAir3 (L2420) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4532 | 0x11B4 | `BANJO_DMG_ELEC` | 2413 | Banjo DamageElec (L2421) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4533 | 0x11B5 | `BANJO_DMG_FLY_HIGH` | 2414 | Banjo DamageFlyHigh (L2422) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4534 | 0x11B6 | `BANJO_DMG_FLY_MID` | 2415 | Banjo DamageFlyMid (L2423) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4535 | 0x11B7 | `BANJO_DMG_FLY_LOW` | 2416 | Banjo DamageFlyLow (L2424) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4536 | 0x11B8 | `BANJO_DMG_FLY_TOP` | 2417 | Banjo DamageFlyTop (L2425) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4537 | 0x11B9 | `BANJO_DMG_FLY_ROLL` | 2418 | Banjo DamageFlyRoll (L2426) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4538 | 0x11BA | `BANJO_TUMBLE` | 2419 | Banjo Tumble (L2427) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 20 |
| 4539 | 0x11BB | `BANJO_FALL_SPECIAL` | 2420 | Banjo FallSpecial (L2428) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4540 | 0x11BC | `BANJO_ENTER_PIPE` | 2421 | Banjo EnterPipe (L2429) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4541 | 0x11BD | `BANJO_EXIT_PIPE` | 2422 | Banjo ExitPipe (L2430) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4542 | 0x11BE | `BANJO_EXIT_PIPE_WALK` | 2423 | Banjo ExitPipeWalk (L2431) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4543 | 0x11BF | `BANJO_CEILING_BONK` | 2424 | Banjo CeilingBonk (L2432) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4544 | 0x11C0 | `BANJO_DOWN_BNCE_D` | 2425 | Banjo DownBounceD (L2433) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 6 |
| 4545 | 0x11C1 | `BANJO_DOWN_BNCE_U` | 2426 | Banjo DownBounceU (L2434) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4546 | 0x11C2 | `BANJO_DOWN_STND_D` | 2427 | Banjo DownStandD (L2435) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 6 |
| 4547 | 0x11C3 | `BANJO_DOWN_STND_U` | 2428 | Banjo DownStandU (L2436) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm`, `src/TwelveCharBattle.asm` | 6 |
| 4548 | 0x11C4 | `BANJO_TECH_F` | 2429 | Banjo TechF (L2437) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4549 | 0x11C5 | `BANJO_TECH_B` | 2430 | Banjo TechB (L2438) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4550 | 0x11C6 | `BANJO_DOWN_FWRD_D` | 2431 | Banjo DownForwardD (L2439) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4551 | 0x11C7 | `BANJO_DOWN_FWRD_U` | 2432 | Banjo DownForwardU (L2440) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4552 | 0x11C8 | `BANJO_DOWN_BACK_D` | 2433 | Banjo DownBackD (L2441) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4553 | 0x11C9 | `BANJO_DOWN_BACK_U` | 2434 | Banjo DownBackU (L2442) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4554 | 0x11CA | `BANJO_DOWN_ATK_D` | 2435 | Banjo DownAttackD (L2443) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4555 | 0x11CB | `BANJO_DOWN_ATK_U` | 2436 | Banjo DownAttackU (L2444) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4556 | 0x11CC | `BANJO_TECH` | 2437 | Banjo Tech (L2445) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4557 | 0x11CD | `BANJO_CLANG_RECOIL` | 2438 | Banjo ClangRecoil (L2446) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4558 | 0x11CE | `BANJO_CLF_CATCH` | 2439 | Banjo CliffCatch (L2447) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4559 | 0x11CF | `BANJO_CLF_WAIT` | 2440 | Banjo CliffWait (L2448) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4560 | 0x11D0 | `BANJO_CLF_QUICK` | 2441 | Banjo CliffQuick (L2449) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4561 | 0x11D1 | `BANJO_CLF_CLM_Q_1` | 2442 | Banjo CliffClimbQuick1 (L2450) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4562 | 0x11D2 | `BANJO_CLF_CLM_Q_2` | 2443 | Banjo CliffClimbQuick2 (L2451) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4563 | 0x11D3 | `BANJO_CLF_SLOW` | 2444 | Banjo CliffSlow (L2452) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4564 | 0x11D4 | `BANJO_CLF_CLM_S_1` | 2445 | Banjo CliffClimbSlow1 (L2453) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4565 | 0x11D5 | `BANJO_CLF_CLM_S_2` | 2446 | Banjo CliffClimbSlow2 (L2454) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4566 | 0x11D6 | `BANJO_CLF_ATK_Q_1` | 2447 | Banjo CliffAttackQuick1 (L2455) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4567 | 0x11D7 | `BANJO_CLF_ATK_Q_2` | 2448 | Banjo CliffAttackQuick2 (L2456) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4568 | 0x11D8 | `BANJO_CLF_ATK_S_1` | 2449 | Banjo CliffAttackSlow1 (L2457) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4569 | 0x11D9 | `BANJO_CLF_ATK_S_2` | 2450 | Banjo CliffAttackSlow2 (L2458) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4570 | 0x11DA | `BANJO_CLF_ESC_Q_1` | 2451 | Banjo CliffEscapeQuick1 (L2459) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4571 | 0x11DB | `BANJO_CLF_ESC_Q_2` | 2452 | Banjo CliffEscapeQuick2 (L2460) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4572 | 0x11DC | `BANJO_CLF_ESC_S_1` | 2453 | Banjo CliffEscapeSlow1 (L2461) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4573 | 0x11DD | `BANJO_CLF_ESC_S_2` | 2454 | Banjo CliffEscapeSlow2 (L2462) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4574 | 0x11DE | `BANJO_L_ITM_PICKUP` | 2455 | Banjo LightItemPickup (L2463) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4575 | 0x11DF | `BANJO_H_ITM_PICKUP` | 2456 | Banjo HeavyItemPickup (L2464) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4576 | 0x11E0 | `BANJO_ITM_DROP` | 2457 | Banjo ItemDrop (L2465) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4577 | 0x11E1 | `BANJO_ITM_THROW_DASH` | 2458 | Banjo ItemThrowDash (L2466) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4578 | 0x11E2 | `BANJO_ITM_THROW_F` | 2459 | Banjo ItemThrowF (L2467) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 8 |
| 4579 | 0x11E3 | `BANJO_ITM_THROW_U` | 2460 | Banjo ItemThrowU (L2468) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4580 | 0x11E4 | `BANJO_ITM_THROW_D` | 2461 | Banjo ItemThrowD (L2469) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4581 | 0x11E5 | `BANJO_ITM_THROW_AIR_F` | 2462 | Banjo ItemThrowAirF (L2470) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 8 |
| 4582 | 0x11E6 | `BANJO_ITM_THROW_AIR_U` | 2463 | Banjo ItemThrowAirU (L2471) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4583 | 0x11E7 | `BANJO_ITM_THROW_AIR_D` | 2464 | Banjo ItemThrowAirD (L2472) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4584 | 0x11E8 | `BANJO_HEAVY_ITM_THROW` | 2465 | Banjo HeavyItemThrow (L2473) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 8 |
| 4585 | 0x11E9 | `BANJO_ITM_NEUTRAL` | 2466 | Banjo ItemNeutral (L2474) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 8 |
| 4586 | 0x11EA | `BANJO_ITM_TILT` | 2467 | Banjo ItemTilt (L2475) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 8 |
| 4587 | 0x11EB | `BANJO_ITM_SMASH` | 2468 | Banjo ItemSmash (L2476) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 8 |
| 4588 | 0x11EC | `BANJO_ITM_DASH` | 2469 | Banjo ItemDash (L2477) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 8 |
| 4589 | 0x11ED | `BANJO_ITM_SHOOT` | 2470 | Banjo ItemShoot (L2478) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4590 | 0x11EE | `BANJO_ITM_SHOOT_AIR` | 2471 | Banjo ItemShootAir (L2479) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4591 | 0x11EF | `BANJO_HAMMER_IDLE` | 2472 | Banjo HammerIdle (L2480) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4592 | 0x11F0 | `BANJO_HAMMER_MOVE` | 2473 | Banjo HammerMove (L2481) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 10 |
| 4593 | 0x11F1 | `BANJO_SHIELD_ON` | 2474 | Banjo ShieldOn (L2482) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4594 | 0x11F2 | `BANJO_SHIELD_OFF` | 2475 | Banjo ShieldOff (L2483) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4595 | 0x11F3 | `BANJO_ROLL_F` | 2476 | Banjo RollF (L2484) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4596 | 0x11F4 | `BANJO_ROLL_B` | 2477 | Banjo RollB (L2485) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4597 | 0x11F5 | `BANJO_STUN` | 2478 | Banjo Stun (L2486) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4598 | 0x11F6 | `BANJO_GRAB` | 2479 | Banjo Grab (L2487) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4599 | 0x11F7 | `BANJO_GRAB_PULL` | 2480 | Banjo GrabPull (L2488) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4600 | 0x11F8 | `BANJO_THROW_F` | 2481 | Banjo ThrowF (L2489) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4601 | 0x11F9 | `BANJO_THROW_B` | 2482 | Banjo ThrowB (L2490) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4602 | 0x11FA | `BANJO_CAPTURE_PULLED` | 2483 | Banjo CapturePulled (L2491) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4603 | 0x11FB | `BANJO_THROWN_DKPULLED` | 2484 | Banjo ThrownDKPulled (L2492) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4604 | 0x11FC | `BANJO_THROWN_MARIO_BROS` | 2485 | Banjo ThrownMarioBros (L2493) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4605 | 0x11FD | `BANJO_THROWN_DK` | 2486 | Banjo ThrownDK (L2494) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4606 | 0x11FE | `BANJO_THROWN_1` | 2487 | Banjo Thrown1 (L2495) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4607 | 0x11FF | `BANJO_THROWN_2` | 2488 | Banjo Thrown2 (L2496) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4608 | 0x1200 | `BANJO_TAUNT` | 2489 | Banjo Taunt (L2497) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4609 | 0x1201 | `BANJO_JAB_1` | 2490 | Banjo Jab1 (L2498) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4610 | 0x1202 | `BANJO_JAB_2` | 2491 | Banjo Jab2 (L2499) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4611 | 0x1203 | `BANJO_DASH_ATTACK` | 2492 | Banjo DashAttack (L2500) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4612 | 0x1204 | `BANJO_FTILT_HIGH` | 2493 | Banjo FTiltHigh (L2501) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4613 | 0x1205 | `BANJO_FTILT_MID_HIGH` | 2494 | Banjo FTiltMidHigh (L2502) |  | 0 |
| 4614 | 0x1206 | `BANJO_FTILT` | 2495 | Banjo FTilt (L2503) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4615 | 0x1207 | `BANJO_FTILT_MID_LOW` | 2496 | Banjo FTiltMidLow (L2504) |  | 0 |
| 4616 | 0x1208 | `BANJO_FTILT_LOW` | 2497 | Banjo FTiltLow (L2505) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4617 | 0x1209 | `BANJO_UTILT` | 2498 | Banjo UTilt (L2506) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4618 | 0x120A | `BANJO_DTILT` | 2499 | Banjo DTilt (L2507) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4619 | 0x120B | `BANJO_FSMASH` | 2500 | Banjo FSmash (L2508) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4620 | 0x120C | `BANJO_USMASH` | 2501 | Banjo USmash (L2509) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4621 | 0x120D | `BANJO_DSMASH` | 2502 | Banjo DSmash (L2510) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4622 | 0x120E | `BANJO_ATTACK_AIR_N` | 2503 | Banjo AttackAirN (L2511) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4623 | 0x120F | `BANJO_ATTACK_AIR_F` | 2504 | Banjo AttackAirF (L2512) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4624 | 0x1210 | `BANJO_ATTACK_AIR_B` | 2505 | Banjo AttackAirB (L2513) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4625 | 0x1211 | `BANJO_ATTACK_AIR_U` | 2506 | Banjo AttackAirU (L2514) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4626 | 0x1212 | `BANJO_ATTACK_AIR_D` | 2507 | Banjo AttackAirD (L2515) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4627 | 0x1213 | `BANJO_LANDING_AIR_D` | 2508 | Banjo LandingAirD (L2516) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4628 | 0x1214 | `BANJO_LANDING_AIR_B` | 2509 | Banjo LandingAirB (L2517) |  | 0 |
| 4629 | 0x1215 | `BANJO_ENTRY_LEFT` | 2510 | Banjo Entry Left (L2518) | `src/Banjo/Banjo.asm` | 1 |
| 4630 | 0x1216 | `BANJO_ENTRY_RIGHT` | 2511 | Banjo Entry Right (L2519) | `src/Banjo/Banjo.asm` | 1 |
| 4631 | 0x1217 | `BANJO_JAB_3` | 2512 | Banjo Jab 3 (L2520) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4632 | 0x1218 | `BANJO_ACTION_0DF` | 2513 | Banjo Action_0DF (L2521) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4633 | 0x1219 | `BANJO_ACTION_0E0` | 2514 | Banjo Action_0E0 (L2522) | `src/Banjo/Banjo.asm` | 1 |
| 4634 | 0x121A | `BANJO_NSP_BEGIN` | 2515 | Banjo Neutral Special Begin (L2523) | `src/Banjo/Banjo.asm` | 2 |
| 4635 | 0x121B | `BANJO_NSP_FORWARD` | 2516 | Banjo Neutral Special Forward (L2524) | `src/Banjo/Banjo.asm` | 2 |
| 4636 | 0x121C | `BANJO_NSP_BACKWARD` | 2517 | Banjo Neutral Special Backward (L2525) | `src/Banjo/Banjo.asm` | 2 |
| 4637 | 0x121D | `BANJO_USP_BEGIN` | 2518 | Banjo Up Special Begin (L2526) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4638 | 0x121E | `BANJO_USP_ATTACK` | 2519 | Banjo Up Special Attack (L2527) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4639 | 0x121F | `BANJO_USP_RECOIL` | 2520 | Banjo Up Special Recoil (L2528) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4640 | 0x1220 | `BANJO_USP_LAND` | 2521 | Banjo Up Special Landing (L2529) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4641 | 0x1221 | `BANJO_USP_ATTACK_END` | 2522 | Banjo Up Special Attack End (L2530) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4642 | 0x1222 | `BANJO_DSP_GND_AIR` | 2523 | Banjo Down Special Ground Air (L2531) | `src/Banjo/Banjo.asm` | 1 |
| 4643 | 0x1223 | `BANJO_DSP_AIR` | 2524 | Banjo Down Special Air (L2532) | `src/Banjo/Banjo.asm` | 1 |
| 4644 | 0x1224 | `BANJO_DSP_LAND` | 2525 | Banjo Down Special Landing (L2533) | `src/Banjo/Banjo.asm` | 1 |
| 4645 | 0x1225 | `BANJO_DSP_LOOP` | 2526 | Banjo Down Special Loop (L2534) | `src/Banjo/Banjo.asm` | 1 |
| 4646 | 0x1226 | `BANJO_DSP_GND` | 2527 | Banjo Down Special Ground (L2535) | `src/Banjo/Banjo.asm` | 1 |
| 4647 | 0x1227 | `BANJO_VICTORY_1` | 2528 | Banjo Victory 1 (L2536) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 4 |
| 4648 | 0x1228 | `BANJO_VICTORY_2` | 2529 | Banjo Victory 2 (L2537) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4649 | 0x1229 | `BANJO_VICTORY_3` | 2530 | Banjo Victory 3 (L2538) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4650 | 0x122A | `BANJO_CLAP` | 2531 | Banjo Clap (L2539) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4651 | 0x122B | `BANJO_CONTINUE_FALL` | 2532 | Banjo ContinueFall (L2540) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4652 | 0x122C | `BANJO_CONTINUE_UP` | 2533 | Banjo ContinueUp (L2541) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4653 | 0x122D | `BANJO_1P_POSE` | 2534 | Banjo 1P Pose (L2542) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4654 | 0x122E | `KAZOOIE_EGG_GFX` | 2535 | Kazooie Egg GFX (L2543) |  | 0 |
| 4655 | 0x122F | `KAZOOIE_EGG_INFO` | 2536 | Kazooie Egg Info (L2544) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 4656 | 0x1230 | `NDSAMUS_MAIN` | 2537 | Polygon Dark Samus Main Character (L2545) | `src/Character.asm` | 1 |
| 4657 | 0x1231 | `SCUTTLE_TOWN_BG` | 2538 | Scuttle Town BG (L2546) |  | 0 |
| 4658 | 0x1232 | `SCUTTLE_TOWN` | 2539 | Scuttle Town (L2547) |  | 0 |
| 4659 | 0x1233 | `SCUTTLE_TOWN_HEADER` | 2540 | Scuttle Town Header (L2548) |  | 0 |
| 4660 | 0x1234 | `SCUTTLE_TOWN_BG_NIGHT` | 2541 | Scuttle Town BG Night (L2549) |  | 0 |
| 4661 | 0x1235 | `METAL_LUIGI_MAIN` | 2542 | Metal Luigi Main Character (L2550) | `src/Character.asm` | 1 |
| 4662 | 0x1236 | `METAL_LUIGI_CHARACTER` | 2543 | Metal Luigi Character (L2551) | `src/Character.asm` | 1 |
| 4663 | 0x1237 | `EBISUMARU_MAIN` | 2544 | Ebisumaru Main (L2552) | `src/Character.asm` | 1 |
| 4664 | 0x1238 | `EBISUMARU_CHARACTER` | 2545 | Ebisumaru Character (L2553) | `src/Character.asm` | 1 |
| 4665 | 0x1239 | `EBISUMARU_DSP_START` | 2546 | Ebisumaru DSP Start (L2554) | `src/Ebi/Ebi.asm` | 1 |
| 4666 | 0x123A | `EBISUMARU_DSP_LAND` | 2547 | Ebisumaru DSP Landing (L2555) | `src/Ebi/Ebi.asm` | 1 |
| 4667 | 0x123B | `EBISUMARU_DSP_RECOIL` | 2548 | Ebisumaru DSP Recoil (L2556) | `src/Ebi/Ebi.asm` | 1 |
| 4668 | 0x123C | `EBISUMARU_WALK_1` | 2549 | Ebisumaru Walk 1 (L2557) | `src/Ebi/Ebi.asm` | 1 |
| 4669 | 0x123D | `EBISUMARU_NAIR` | 2550 | Ebisumaru Attack Aerial N (L2558) | `src/Ebi/Ebi.asm` | 1 |
| 4670 | 0x123E | `EBISUMARU_UP_SPECIAL` | 2551 | Ebisumaru Up Special (L2559) | `src/Ebi/Ebi.asm` | 1 |
| 4671 | 0x123F | `EBISUMARU_UTILT` | 2552 | Ebisumaru Up Tilt (L2560) | `src/Ebi/Ebi.asm` | 1 |
| 4672 | 0x1240 | `EBISUMARU_RUN` | 2553 | Ebisumaru Run (L2561) | `src/Ebi/Ebi.asm` | 1 |
| 4673 | 0x1241 | `EBISUMARU_DASH_ATTACK` | 2554 | Ebisumaru Dash Attack (L2562) | `src/Ebi/Ebi.asm` | 1 |
| 4674 | 0x1242 | `BIG_BOOS_HAUNT_BG` | 2555 | Big Boo's Haunt BG (L2563) |  | 0 |
| 4675 | 0x1243 | `BIG_BOOS_HAUNT` | 2556 | Big Boo's Haunt Stage (L2564) |  | 0 |
| 4676 | 0x1244 | `BIG_BOOS_HAUNT_HEADER` | 2557 | Big Boo's Haunt Header (L2565) |  | 0 |
| 4677 | 0x1245 | `YOSHIS_ISLAND_BG` | 2558 | Yoshi's Island Melee BG (L2566) |  | 0 |
| 4678 | 0x1246 | `YOSHIS_ISLAND` | 2559 | Yoshi's Island Melee Stage (L2567) |  | 0 |
| 4679 | 0x1247 | `YOSHIS_ISLAND_HEADER` | 2560 | Yoshi's Island Melee Header (L2568) |  | 0 |
| 4680 | 0x1248 | `SCUTTLETOWN_ENEMY_INFO` | 2561 | Scuttletown Enemy Info (L2569) |  | 0 |
| 4681 | 0x1249 | `SCUTTLETOWN_ENEMY_GFX` | 2562 | Scuttletown Enemy GFX (L2570) |  | 0 |
| 4682 | 0x124A | `BANJO_SHIELD_POSE` | 2563 | Banjo Shield Pose (L2571) | `src/Character.asm` | 2 |
| 4683 | 0x124B | `EBISUMARU_1P_POSE` | 2564 | Ebisumaru 1P Pose (L2572) | `src/Ebi/Ebi.asm` | 1 |
| 4684 | 0x124C | `EBISUMARU_NSP_ATTACK` | 2565 | Ebisumaru NSP Attack (L2573) | `src/Ebi/Ebi.asm` | 2 |
| 4685 | 0x124D | `EBISUMARU_DSP_GROUNDED` | 2566 | Ebisumaru DSP Grounded (L2574) | `src/Ebi/Ebi.asm` | 1 |
| 4686 | 0x124E | `EBISUMARU_DASH` | 2567 | Ebisumaru Dash (L2575) | `src/Ebi/Ebi.asm` | 1 |
| 4687 | 0x124F | `KIRBY_CUBE` | 2568 | Kirby Cube (L2576) | `src/Hitbox.asm` | 1 |
| 4688 | 0x1250 | `BANJO_TARGET_HEADER` | 2569 | Banjo Break the Targets Header (L2577) |  | 0 |
| 4689 | 0x1251 | `BANJO_TARGET_STAGE` | 2570 | Banjo Break the Targets Stage (L2578) |  | 0 |
| 4690 | 0x1252 | `BANJO_ENTRY_EFFECTS` | 2571 | Banjo Entry Effects (L2579) | `src/Character.asm` | 2 |
| 4691 | 0x1253 | `BANJO_1P_CPU_POSE` | 2572 | Banjo 1p CPU Pose (L2580) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4692 | 0x1254 | `DSAMUS_TURNRUN` | 2573 | Dark Samus TurnRun (L2581) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 4693 | 0x1255 | `DSAMUS_RUNBRAKE` | 2574 | Dark Samus RunBrake (L2582) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 4694 | 0x1256 | `NMARTH_MAIN` | 2575 | Polygon Marth Main (L2583) | `src/Character.asm` | 1 |
| 4695 | 0x1257 | `NMARTH_CHARACTER` | 2576 | Polygon Marth Character (L2584) | `src/Character.asm` | 1 |
| 4696 | 0x1258 | `NMTWO_MAIN` | 2577 | Polygon Mewtwo Main (L2585) | `src/Character.asm` | 1 |
| 4697 | 0x1259 | `NMTWO_CHARACTER` | 2578 | Polygon Mewtwo Character (L2586) | `src/Character.asm` | 1 |
| 4698 | 0x125A | `NDEDEDE_MAIN` | 2579 | Polygon Dedede Main (L2587) | `src/Character.asm` | 1 |
| 4699 | 0x125B | `NDEDEDE_CHARACTER` | 2580 | Polygon Dedede Character (L2588) | `src/Character.asm` | 1 |
| 4700 | 0x125C | `SPAWNED_FEAR_BG` | 2581 | Spawned Fear BG (L2589) |  | 0 |
| 4701 | 0x125D | `SPAWNED_FEAR_STAGE` | 2582 | Spawned Fear Stage (L2590) |  | 0 |
| 4702 | 0x125E | `SPAWNED_FEAR_HEADER` | 2583 | Spawned Fear Header (L2591) |  | 0 |
| 4703 | 0x125F | `SMASHVILLE_REMIX_BG` | 2584 | Smashville Remix BG (L2592) |  | 0 |
| 4704 | 0x1260 | `SMASHVILLE_REMIX_STAGE` | 2585 | Smashville Remix Stage (L2593) |  | 0 |
| 4705 | 0x1261 | `SMASHVILLE_REMIX_HEADER` | 2586 | Smashville Remix Header (L2594) |  | 0 |
| 4706 | 0x1262 | `TRANSITION_SMASH_LOGO` | 2587 | Transition Smash Logo (L2595) | `src/Render.asm`, `src/Transitions.asm` | 2 |
| 4707 | 0x1263 | `KIRBY_EBI_HAT_MEAT` | 2588 | Kirby Ebi Hat Meat (L2596) |  | 0 |
| 4708 | 0x1264 | `NYLINK_MAIN` | 2589 | Polygon Young Link Main (L2597) | `src/Character.asm` | 1 |
| 4709 | 0x1265 | `NYLINK_CHARACTER` | 2590 | Polygon Young Link Character (L2598) | `src/Character.asm` | 1 |
| 4710 | 0x1266 | `STG_BANJO_PLATFORMS_HEADER` | 2591 | Banjo Board the Platforms Header (L2599) |  | 0 |
| 4711 | 0x1267 | `STG_BANJO_PLATFORMS_STAGE` | 2592 | Banjo Board the Platforms Stage (L2600) |  | 0 |
| 4712 | 0x1268 | `DRAGONKING_MAIN` | 2593 | Dragon King Main (L2601) | `src/Character.asm` | 1 |
| 4713 | 0x1269 | `DRAGONKING_CHARACTER` | 2594 | Dragon King Character (L2602) | `src/Character.asm` | 1 |
| 4714 | 0x126A | `DKING_CSS` | 2595 | Dragon King CSS (L2603) | `src/DragonKing/DragonKing.asm` | 1 |
| 4715 | 0x126B | `DKING_IDLE` | 2596 | Dragon King Idle (L2604) | `src/DragonKing/DragonKing.asm` | 3 |
| 4716 | 0x126C | `DKING_USMASH` | 2597 | Dragon King USmash (L2605) | `src/DragonKing/DragonKing.asm` | 1 |
| 4717 | 0x126D | `DKING_FTILT_HIGH` | 2598 | Dragon King FTiltHigh (L2606) | `src/DragonKing/DragonKing.asm` | 1 |
| 4718 | 0x126E | `DKING_FTILT` | 2599 | Dragon King FTilt (L2607) | `src/DragonKing/DragonKing.asm` | 1 |
| 4719 | 0x126F | `DKING_FTILT_LOW` | 2600 | Dragon King FTiltLow (L2608) | `src/DragonKing/DragonKing.asm` | 1 |
| 4720 | 0x1270 | `DKING_ATTACK_AIR_B` | 2601 | Dragon King AttackAirB (L2609) | `src/DragonKing/DragonKing.asm` | 1 |
| 4721 | 0x1271 | `DKING_LANDING_AIR_B` | 2602 | Dragon King LandingAirB (L2610) | `src/DragonKing/DragonKing.asm` | 1 |
| 4722 | 0x1272 | `DKING_JUMP_B` | 2603 | Dragon King JumpB (L2611) | `src/DragonKing/DragonKing.asm` | 1 |
| 4723 | 0x1273 | `DKING_FALL` | 2604 | Dragon King Fall (L2612) | `src/DragonKing/DragonKing.asm` | 2 |
| 4724 | 0x1274 | `DKING_DSMASH` | 2605 | Dragon King DSmash (L2613) | `src/DragonKing/DragonKing.asm` | 1 |
| 4725 | 0x1275 | `DKING_ATTACK_AIR_N` | 2606 | Dragon King AttackAirN (L2614) | `src/DragonKing/DragonKing.asm` | 1 |
| 4726 | 0x1276 | `STG_BANJO_PLATFORMS_BARREL_ANIMATIONS` | 2607 | Banjo Board the Platforms Barrel Animations (L2615) |  | 0 |
| 4727 | 0x1277 | `BANJO_TEAM_POSE` | 2608 | Banjo Team Pose (L2616) | `src/SinglePlayerModes.asm` | 3 |
| 4728 | 0x1278 | `CACODEMON` | 2609 | Cacodemon (L2617) |  | 0 |
| 4729 | 0x1279 | `POKEFLOATS_HEADER` | 2610 | Pokefloats Header (L2618) |  | 0 |
| 4730 | 0x127A | `POKEFLOATS_STAGE` | 2611 | Pokefloats Stage (L2619) |  | 0 |
| 4731 | 0x127B | `POKEFLOATS_BG` | 2612 | Pokefloats BG (L2620) |  | 0 |
| 4732 | 0x127C | `KIRBY_BANJO_NSP_START` | 2613 | Kirby Banjo Copy Neutral Special Start (L2621) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4733 | 0x127D | `KIRBY_BANJO_NSP_FORWARD` | 2614 | Kirby Banjo Copy Neutral Special Forward (L2622) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4734 | 0x127E | `KIRBY_BANJO_NSP_BACKWARD` | 2615 | Kirby Banjo Copy Neutral Special Backward (L2623) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4735 | 0x127F | `DKING_ATTACK_AIR_D` | 2616 | Dragon King AttackAirD (L2624) | `src/DragonKing/DragonKing.asm` | 1 |
| 4736 | 0x1280 | `DKING_JUMP_F` | 2617 | Dragon King JumpF (L2625) | `src/DragonKing/DragonKing.asm` | 1 |
| 4737 | 0x1281 | `DKING_JUMP_AERIAL_F` | 2618 | Dragon King JumpAerialF (L2626) | `src/DragonKing/DragonKing.asm` | 1 |
| 4738 | 0x1282 | `THUNDERBALL_HITBOX` | 2619 | Thunderball Hitbox (L2627) |  | 0 |
| 4739 | 0x1283 | `THUNDERBALL_GRAPHICS` | 2620 | Thunderball Graphics (L2628) |  | 0 |
| 4740 | 0x1284 | `DKING_JUMP_AERIAL_B` | 2621 | Dragon King JumpAerialB (L2629) | `src/DragonKing/DragonKing.asm` | 1 |
| 4741 | 0x1285 | `DKING_TAUNT` | 2622 | Dragon King Taunt (L2630) | `src/DragonKing/DragonKing.asm` | 1 |
| 4742 | 0x1286 | `DKING_ATTACK_AIR_F` | 2623 | Dragon King AttackAirF (L2631) | `src/DragonKing/DragonKing.asm` | 1 |
| 4743 | 0x1287 | `DKING_LANDING_AIR_F` | 2624 | Dragon King LandingAirF (L2632) | `src/DragonKing/DragonKing.asm` | 1 |
| 4744 | 0x1288 | `EBISUMARU_TAUNT` | 2625 | Ebisumaru Taunt (L2633) | `src/Ebi/Ebi.asm` | 1 |
| 4745 | 0x1289 | `DKING_VICTORY_2` | 2626 | Dragon King Victory 2 (L2634) | `src/DragonKing/DragonKing.asm` | 1 |
| 4746 | 0x128A | `EBISUMARU_ENTRY` | 2627 | Ebisumaru Entry (L2635) | `src/Ebi/Ebi.asm` | 2 |
| 4747 | 0x128B | `DKING_NSPG` | 2628 | Dragon King Neutral Special Ground (L2636) | `src/DragonKing/DragonKing.asm` | 1 |
| 4748 | 0x128C | `DKING_DSP_G` | 2629 | Dragon King Down Special Ground (L2637) | `src/DragonKing/DragonKing.asm` | 1 |
| 4749 | 0x128D | `DKING_DSP_A_BEGIN` | 2630 | Dragon King Down Special Air Begin (L2638) | `src/DragonKing/DragonKing.asm` | 1 |
| 4750 | 0x128E | `DKING_DSP_A_LOOP` | 2631 | Dragon King Down Special Air Loop (L2639) | `src/DragonKing/DragonKing.asm` | 1 |
| 4751 | 0x128F | `DKING_DSP_A_LANDING` | 2632 | Dragon King Down Special Air Landing (0x40000000) (L2640) | `src/DragonKing/DragonKing.asm` | 1 |
| 4752 | 0x1290 | `EBISUMARU_CSS` | 2633 | Ebisumaru CSS (L2641) | `src/Ebi/Ebi.asm` | 2 |
| 4753 | 0x1291 | `EBISUMARU_DUO_POSE` | 2634 | Ebisumaru Duo Pose (L2642) | `src/SinglePlayerModes.asm` | 1 |
| 4754 | 0x1292 | `GOEMON_DUO_POSE` | 2635 | Goemon Duo Pose (L2643) | `src/SinglePlayerModes.asm` | 1 |
| 4755 | 0x1293 | `EBISUMARU_VICTORY_1` | 2636 | Ebisumaru Victory 1 (L2644) | `src/Ebi/Ebi.asm` | 1 |
| 4756 | 0x1294 | `DKING_RUN` | 2637 | Dragon King Run (L2645) | `src/DragonKing/DragonKing.asm` | 1 |
| 4757 | 0x1295 | `DKING_ENTRY_L` | 2638 | Dragon King Entry Left (L2646) | `src/DragonKing/DragonKing.asm` | 1 |
| 4758 | 0x1296 | `DKING_ENTRY_R` | 2639 | Dragon King Entry Right (L2647) | `src/DragonKing/DragonKing.asm` | 1 |
| 4759 | 0x1297 | `DKING_JAB_1` | 2640 | Dragon King Jab 1 (L2648) | `src/DragonKing/DragonKing.asm` | 1 |
| 4760 | 0x1298 | `DKING_JAB_2` | 2641 | Dragon King Jab 2 (L2649) | `src/DragonKing/DragonKing.asm` | 1 |
| 4761 | 0x1299 | `DKING_JAB_LOOP_START` | 2642 | Dragon King Jab Loop Start (L2650) | `src/DragonKing/DragonKing.asm` | 1 |
| 4762 | 0x129A | `DKING_JAB_LOOP` | 2643 | Dragon King Jab Loop (L2651) | `src/DragonKing/DragonKing.asm` | 1 |
| 4763 | 0x129B | `DKING_JAB_LOOP_END` | 2644 | Dragon King Jab Loop End (L2652) | `src/DragonKing/DragonKing.asm` | 1 |
| 4764 | 0x129C | `DKING_UTILT` | 2645 | Dragon King UTilt (L2653) | `src/DragonKing/DragonKing.asm` | 1 |
| 4765 | 0x129D | `DKING_RUN_BRAKE` | 2646 | Dragon King RunBrake (L2654) | `src/DragonKing/DragonKing.asm` | 1 |
| 4766 | 0x129E | `BANJO_DUO_POSE` | 2647 | Banjo Duo Pose (L2655) | `src/SinglePlayerModes.asm` | 1 |
| 4767 | 0x129F | `CONKER_DUO_POSE` | 2648 | Conker Duo Pose (L2656) | `src/SinglePlayerModes.asm` | 1 |
| 4768 | 0x12A0 | `DKING_FSMASH_HIGH` | 2649 | Dragon King FSmash High (L2657) | `src/DragonKing/DragonKing.asm` | 1 |
| 4769 | 0x12A1 | `DKING_FSMASH_MID_HIGH` | 2650 | Dragon King FSmash Mid-High (L2658) | `src/DragonKing/DragonKing.asm` | 1 |
| 4770 | 0x12A2 | `DKING_FSMASH` | 2651 | Dragon King FSmash (L2659) | `src/DragonKing/DragonKing.asm` | 1 |
| 4771 | 0x12A3 | `DKING_FSMASH_MID_LOW` | 2652 | Dragon King FSmash Mid-Low (L2660) | `src/DragonKing/DragonKing.asm` | 1 |
| 4772 | 0x12A4 | `DKING_FSMASH_LOW` | 2653 | Dragon King FSmash Low (L2661) | `src/DragonKing/DragonKing.asm` | 1 |
| 4773 | 0x12A5 | `DKING_NSPA` | 2654 | Dragon King Neutral Special Air (L2662) | `src/DragonKing/DragonKing.asm` | 1 |
| 4774 | 0x12A6 | `DKING_ATTACK_AIR_U` | 2655 | Dragon King AttackAirU (L2663) | `src/DragonKing/DragonKing.asm` | 1 |
| 4775 | 0x12A7 | `DKING_USPG` | 2656 | Dragon King Up Special Ground (L2664) | `src/DragonKing/DragonKing.asm` | 1 |
| 4776 | 0x12A8 | `DKING_USPA` | 2657 | Dragon King Up Special Air (L2665) | `src/DragonKing/DragonKing.asm` | 1 |
| 4777 | 0x12A9 | `DKING_USPG_THROW` | 2658 | Dragon King Up Special Ground Throw (L2666) | `src/DragonKing/DragonKing.asm` | 1 |
| 4778 | 0x12AA | `DKING_USPA_THROW` | 2659 | Dragon King Up Special Air Throw (L2667) | `src/DragonKing/DragonKing.asm` | 1 |
| 4779 | 0x12AB | `DKING_USPA_LANDING` | 2660 | Dragon King Up Special Air Landing Throw (L2668) | `src/DragonKing/DragonKing.asm` | 1 |
| 4780 | 0x12AC | `BIG_SNOWMAN_HEADER` | 2661 | Big Snowman Header (L2669) |  | 0 |
| 4781 | 0x12AD | `BIG_SNOWMAN_STAGE` | 2662 | Big Snowman Stage (L2670) |  | 0 |
| 4782 | 0x12AE | `BIG_SNOWMAN_BG` | 2663 | Big Snowman BG (L2671) |  | 0 |
| 4783 | 0x12AF | `BANJO_USP_WALL_SPLAT` | 2664 | Banjo Up Special Wall Splat (L2672) | `src/Banjo/Banjo.asm`, `src/NBanjo/NBanjo.asm` | 2 |
| 4784 | 0x12B0 | `DKING_FTILT_MID_HIGH` | 2665 | Dragon King FTiltMidHigh (L2673) | `src/DragonKing/DragonKing.asm` | 1 |
| 4785 | 0x12B1 | `DKING_FTILT_MID_LOW` | 2666 | Dragon King FTiltMidLow (L2674) | `src/DragonKing/DragonKing.asm` | 1 |
| 4786 | 0x12B2 | `DKING_DASH_ATTACK` | 2667 | Dragon King Dash Attack (L2675) | `src/DragonKing/DragonKing.asm` | 1 |
| 4787 | 0x12B3 | `DKING_DTILT` | 2668 | Dragon King DTilt (L2676) | `src/DragonKing/DragonKing.asm` | 1 |
| 4788 | 0x12B4 | `DKING_JUMPSQUAT` | 2669 | Dragon King JumpSquat (L2677) | `src/DragonKing/DragonKing.asm` | 6 |
| 4789 | 0x12B5 | `SHEIK_LIGHT_ITEM_PICKUP` | 2670 | Sheik Light Item Pickup (L2678) | `src/Sheik/Sheik.asm` | 1 |
| 4790 | 0x12B6 | `SHEIK_ITEM_DROP` | 2671 | Sheik Item Drop (L2679) | `src/Sheik/Sheik.asm` | 1 |
| 4791 | 0x12B7 | `SHEIK_ITEM_SHOOT_GROUND` | 2672 | Sheik Item Shoot Ground (L2680) | `src/Sheik/Sheik.asm` | 2 |
| 4792 | 0x12B8 | `KIRBY_EBI_NSP_END` | 2673 | Kirby Ebisumaru Neutral Special End (L2681) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 4793 | 0x12B9 | `NGOEMON_MAIN` | 2674 | Polygon Goemon Main (L2682) | `src/Character.asm` | 1 |
| 4794 | 0x12BA | `NGOEMON_CHARACTER` | 2675 | Polygon Goemon Model (L2683) | `src/Character.asm` | 1 |
| 4795 | 0x12BB | `NCONKER_MAIN` | 2676 | Polygon Conker Main (L2684) | `src/Character.asm` | 1 |
| 4796 | 0x12BC | `NCONKER_CHARACTER` | 2677 | Polygon Conker Model (L2685) | `src/Character.asm` | 1 |
| 4797 | 0x12BD | `NBANJO_MAIN` | 2678 | Polygon Banjo Main (L2686) | `src/Character.asm` | 1 |
| 4798 | 0x12BE | `NBANJO_CHARACTER` | 2679 | Polygon Banjo Model (L2687) | `src/Character.asm` | 1 |
| 4799 | 0x12BF | `DRAGONKING_SHIELD_POSE` | 2680 | Dragon King Shield Pose (L2688) | `src/Character.asm` | 1 |
| 4800 | 0x12C0 | `DRAGONKING_SHIELD_ON` | 2681 | Dragon King Shield On (L2689) | `src/DragonKing/DragonKing.asm` | 1 |
| 4801 | 0x12C1 | `DRAGONKING_SHIELD_OFF` | 2682 | Dragon King Shield Off (L2690) | `src/DragonKing/DragonKing.asm` | 1 |
| 4802 | 0x12C2 | `KIRBY_DKING_NSP_G` | 2683 | Kirby Dragon King Neutral Special Ground (L2691) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4803 | 0x12C3 | `KIRBY_DKING_NSP_A` | 2684 | Kirby Dragon King Neutral Special Air (L2692) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 4804 | 0x12C4 | `DKING_FTHROW` | 2685 | Dragon King FThrow (L2693) | `src/DragonKing/DragonKing.asm` | 1 |
| 4805 | 0x12C5 | `DKING_BTHROW` | 2686 | Dragon King BThrow (L2694) | `src/DragonKing/DragonKing.asm` | 1 |
| 4806 | 0x12C6 | `DKING_DASH` | 2687 | Dragon King Dash (L2695) | `src/DragonKing/DragonKing.asm` | 1 |
| 4807 | 0x12C7 | `BANJO_VICTORY_IMAGE_BOTTOM` | 2688 | Banjo Ending Image 1 (L2696) | `src/SinglePlayer.asm` | 1 |
| 4808 | 0x12C8 | `BANJO_VICTORY_IMAGE_TOP` | 2689 | Banjo Ending Image 2 (L2697) |  | 0 |
| 4809 | 0x12C9 | `EBI_VICTORY_IMAGE_BOTTOM` | 2690 | Ebisumaru Ending Image 1 (L2698) | `src/SinglePlayer.asm` | 1 |
| 4810 | 0x12CA | `EBI_VICTORY_IMAGE_TOP` | 2691 | Ebisumaru Ending Image 2 (L2699) |  | 0 |
| 4811 | 0x12CB | `EPUFF_MAIN` | 2692 | EPuff Main (L2700) | `src/Character.asm` | 1 |
| 4812 | 0x12CC | `CRASH_MAIN` | 2693 | Crash Main (L2701) | `src/Character.asm` | 1 |
| 4813 | 0x12CD | `CRASH_CHARACTER` | 2694 | Crash Character (L2702) | `src/Character.asm` | 1 |
| 4814 | 0x12CE | `CRASH_IDLE` | 2695 | Crash Idle (L2703) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 14 |
| 4815 | 0x12CF | `CRASH_WALK1` | 2696 | Crash Walk1 (L2704) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4816 | 0x12D0 | `CRASH_WALK2` | 2697 | Crash Walk2 (L2705) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4817 | 0x12D1 | `CRASH_WALK3` | 2698 | Crash Walk3 (L2706) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4818 | 0x12D2 | `CRASH_WALK_END` | 2699 | Crash WalkEnd (L2707) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4819 | 0x12D3 | `CRASH_DASH` | 2700 | Crash Dash (L2708) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4820 | 0x12D4 | `CRASH_RUN` | 2701 | Crash Run (L2709) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4821 | 0x12D5 | `CRASH_RUN_BRAKE` | 2702 | Crash RunBrake (L2710) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4822 | 0x12D6 | `CRASH_TURN` | 2703 | Crash Turn (L2711) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4823 | 0x12D7 | `CRASH_TURN_RUN` | 2704 | Crash TurnRun (L2712) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4824 | 0x12D8 | `CRASH_JUMP_F` | 2705 | Crash JumpF (L2713) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4825 | 0x12D9 | `CRASH_JUMP_B` | 2706 | Crash JumpB (L2714) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4826 | 0x12DA | `CRASH_JUMP_AERIAL_F` | 2707 | Crash JumpAerialF (L2715) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4827 | 0x12DB | `CRASH_JUMP_AERIAL_B` | 2708 | Crash JumpAerialB (L2716) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4828 | 0x12DC | `CRASH_FALL` | 2709 | Crash Fall (L2717) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4829 | 0x12DD | `CRASH_FALL_AERIAL` | 2710 | Crash FallAerial (L2718) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4830 | 0x12DE | `CRASH_CROUCH` | 2711 | Crash Crouch (L2719) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4831 | 0x12DF | `CRASH_CROUCH_IDLE` | 2712 | Crash CrouchIdle (L2720) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4832 | 0x12E0 | `CRASH_CROUCH_END` | 2713 | Crash CrouchEnd (L2721) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4833 | 0x12E1 | `CRASH_LANDING` | 2714 | Crash Landing (L2722) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 12 |
| 4834 | 0x12E2 | `CRASH_SHIELD_DROP` | 2715 | Crash ShieldDrop (L2723) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4835 | 0x12E3 | `CRASH_TEETER` | 2716 | Crash Teeter (L2724) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4836 | 0x12E4 | `CRASH_TEETER_START` | 2717 | Crash TeeterStart (L2725) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4837 | 0x12E5 | `CRASH_STUN` | 2718 | Crash Stun (L2726) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4838 | 0x12E6 | `CRASH_DAMAGE_HIGH1` | 2719 | Crash DamageHigh1 (L2727) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4839 | 0x12E7 | `CRASH_DAMAGE_HIGH2` | 2720 | Crash DamageHigh2 (L2728) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4840 | 0x12E8 | `CRASH_DAMAGE_HIGH3` | 2721 | Crash DamageHigh3 (L2729) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4841 | 0x12E9 | `CRASH_DAMAGE_MID1` | 2722 | Crash DamageMid1 (L2730) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4842 | 0x12EA | `CRASH_DAMAGE_MID2` | 2723 | Crash DamageMid2 (L2731) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4843 | 0x12EB | `CRASH_DAMAGE_MID3` | 2724 | Crash DamageMid3 (L2732) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4844 | 0x12EC | `CRASH_DAMAGE_LOW1` | 2725 | Crash DamageLow1 (L2733) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4845 | 0x12ED | `CRASH_DAMAGE_LOW2` | 2726 | Crash DamageLow2 (L2734) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4846 | 0x12EE | `CRASH_DAMAGE_LOW3` | 2727 | Crash DamageLow3 (L2735) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4847 | 0x12EF | `CRASH_DAMAGE_AIR1` | 2728 | Crash DamageAir1 (L2736) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4848 | 0x12F0 | `CRASH_DAMAGE_AIR2` | 2729 | Crash DamageAir2 (L2737) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4849 | 0x12F1 | `CRASH_DAMAGE_AIR3` | 2730 | Crash DamageAir3 (L2738) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4850 | 0x12F2 | `CRASH_DAMAGE_FLY_HIGH` | 2731 | Crash DamageFlyHigh (L2739) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4851 | 0x12F3 | `CRASH_DAMAGE_FLY_MID` | 2732 | Crash DamageFlyMid (L2740) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4852 | 0x12F4 | `CRASH_DAMAGE_FLY_LOW` | 2733 | Crash DamageFlyLow (L2741) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4853 | 0x12F5 | `CRASH_DAMAGE_FLY_TOP` | 2734 | Crash DamageFlyTop (L2742) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4854 | 0x12F6 | `CRASH_DAMAGE_FLY_ROLL` | 2735 | Crash DamageFlyRoll (L2743) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4855 | 0x12F7 | `CRASH_DAMAGE_ELEC` | 2736 | Crash DamageElec (L2744) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4856 | 0x12F8 | `CRASH_TUMBLE` | 2737 | Crash Tumble (L2745) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 20 |
| 4857 | 0x12F9 | `CRASH_FALL_SPECIAL` | 2738 | Crash FallSpecial (L2746) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4858 | 0x12FA | `CRASH_CEILING_BONK` | 2739 | Crash CeilingBonk (L2747) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4859 | 0x12FB | `CRASH_DOWN_BOUNCE_D` | 2740 | Crash DownBounceD (L2748) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 6 |
| 4860 | 0x12FC | `CRASH_DOWN_BOUNCE_U` | 2741 | Crash DownBounceU (L2749) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4861 | 0x12FD | `CRASH_DOWN_STAND_D` | 2742 | Crash DownStandD (L2750) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 6 |
| 4862 | 0x12FE | `CRASH_DOWN_STAND_U` | 2743 | Crash DownStandU (L2751) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm`, `src/TwelveCharBattle.asm` | 6 |
| 4863 | 0x12FF | `CRASH_TECH_F` | 2744 | Crash TechF (L2752) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4864 | 0x1300 | `CRASH_TECH_B` | 2745 | Crash TechB (L2753) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4865 | 0x1301 | `CRASH_DOWN_FORWARD_D` | 2746 | Crash DownForwardD (L2754) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4866 | 0x1302 | `CRASH_DOWN_FORWARD_U` | 2747 | Crash DownForwardU (L2755) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4867 | 0x1303 | `CRASH_DOWN_BACK_D` | 2748 | Crash DownBackD (L2756) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4868 | 0x1304 | `CRASH_DOWN_BACK_U` | 2749 | Crash DownBackU (L2757) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4869 | 0x1305 | `CRASH_DOWN_ATK_D` | 2750 | Crash DownAttackD (L2758) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4870 | 0x1306 | `CRASH_DOWN_ATK_U` | 2751 | Crash DownAttackU (L2759) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4871 | 0x1307 | `CRASH_TECH` | 2752 | Crash Tech (L2760) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4872 | 0x1308 | `CRASH_CLANG_RECOIL` | 2753 | Crash ClangRecoil (L2761) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4873 | 0x1309 | `CRASH_SHIELD_ON` | 2754 | Crash ShieldOn (L2762) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4874 | 0x130A | `CRASH_SHIELD_OFF` | 2755 | Crash ShieldOff (L2763) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4875 | 0x130B | `CRASH_ROLL_F` | 2756 | Crash RollF (L2764) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4876 | 0x130C | `CRASH_ROLL_B` | 2757 | Crash RollB (L2765) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4877 | 0x130D | `CRASH_GRAB` | 2758 | Crash Grab (L2766) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4878 | 0x130E | `CRASH_GRAB_PULL` | 2759 | Crash GrabPull (L2767) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4879 | 0x130F | `CRASH_THROW_F` | 2760 | Crash ThrowF (L2768) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4880 | 0x1310 | `CRASH_THROW_B` | 2761 | Crash ThrowB (L2769) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4881 | 0x1311 | `CRASH_CAPTURE_PULLED` | 2762 | Crash CapturePulled (L2770) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4882 | 0x1312 | `CRASH_THROWN_DK_PULLED` | 2763 | Crash ThrownDKPulled (L2771) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4883 | 0x1313 | `CRASH_THROWN_MARIO_BROS` | 2764 | Crash ThrownMarioBros (L2772) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4884 | 0x1314 | `CRASH_THROWN_DK` | 2765 | Crash ThrownDK (L2773) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4885 | 0x1315 | `CRASH_THROWN2` | 2766 | Crash Thrown2 (L2774) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4886 | 0x1316 | `CRASH_THROWN1` | 2767 | Crash Thrown1 (L2775) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4887 | 0x1317 | `CRASH_TAUNT` | 2768 | Crash Taunt (L2776) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4888 | 0x1318 | `CRASH_CLF_CATCH` | 2769 | Crash CliffCatch (L2777) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4889 | 0x1319 | `CRASH_CLF_WAIT` | 2770 | Crash CliffWait (L2778) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4890 | 0x131A | `CRASH_CLF_Q` | 2771 | Crash CliffQuick (L2779) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4891 | 0x131B | `CRASH_CLF_CLM_Q1` | 2772 | Crash CliffClimbQuick1 (L2780) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4892 | 0x131C | `CRASH_CLF_CLM_Q2` | 2773 | Crash CliffClimbQuick2 (L2781) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4893 | 0x131D | `CRASH_CLF_S` | 2774 | Crash CliffSlow (L2782) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4894 | 0x131E | `CRASH_CLF_CLM_S1` | 2775 | Crash CliffClimbSlow1 (L2783) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4895 | 0x131F | `CRASH_CLF_CLM_S2` | 2776 | Crash CliffClimbSlow2 (L2784) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4896 | 0x1320 | `CRASH_CLF_ATK_Q1` | 2777 | Crash CliffAttackQuick1 (L2785) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4897 | 0x1321 | `CRASH_CLF_ATK_Q2` | 2778 | Crash CliffAttackQuick2 (L2786) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4898 | 0x1322 | `CRASH_CLF_ATK_S1` | 2779 | Crash CliffAttackSlow1 (L2787) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4899 | 0x1323 | `CRASH_CLF_ATK_S2` | 2780 | Crash CliffAttackSlow2 (L2788) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4900 | 0x1324 | `CRASH_CLF_ESC_Q1` | 2781 | Crash CliffEscapeQuick1 (L2789) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4901 | 0x1325 | `CRASH_CLF_ESC_Q2` | 2782 | Crash CliffEscapeQuick2 (L2790) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4902 | 0x1326 | `CRASH_CLF_ESC_S1` | 2783 | Crash CliffEscapeSlow1 (L2791) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4903 | 0x1327 | `CRASH_CLF_ESC_S2` | 2784 | Crash CliffEscapeSlow2 (L2792) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4904 | 0x1328 | `CRASH_L_ITM_PICKUP` | 2785 | Crash LightItemPickup (L2793) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4905 | 0x1329 | `CRASH_ITM_THROW_F` | 2786 | Crash ItemThrowF (L2794) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 8 |
| 4906 | 0x132A | `CRASH_ITM_THROW_U` | 2787 | Crash ItemThrowU (L2795) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4907 | 0x132B | `CRASH_ITM_THROW_D` | 2788 | Crash ItemThrowD (L2796) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4908 | 0x132C | `CRASH_ITM_THROW_DASH` | 2789 | Crash ItemThrowDash (L2797) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4909 | 0x132D | `CRASH_ITM_DROP` | 2790 | Crash ItemDrop (L2798) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4910 | 0x132E | `CRASH_ITM_THROW_AIR_F` | 2791 | Crash ItemThrowAirF (L2799) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 8 |
| 4911 | 0x132F | `CRASH_ITM_THROW_AIR_U` | 2792 | Crash ItemThrowAirU (L2800) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4912 | 0x1330 | `CRASH_ITM_THROW_AIR_D` | 2793 | Crash ItemThrowAirD (L2801) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4913 | 0x1331 | `CRASH_H_ITM_PICKUP` | 2794 | Crash HeavyItemPickup (L2802) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4914 | 0x1332 | `CRASH_H_ITM_THROW` | 2795 | Crash HeavyItemThrow (L2803) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 8 |
| 4915 | 0x1333 | `CRASH_ITM_JAB` | 2796 | Crash ItemJab (L2804) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 8 |
| 4916 | 0x1334 | `CRASH_ITM_TILT` | 2797 | Crash ItemTilt (L2805) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 8 |
| 4917 | 0x1335 | `CRASH_ITM_SMASH` | 2798 | Crash ItemSmash (L2806) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 8 |
| 4918 | 0x1336 | `CRASH_ITM_DASH` | 2799 | Crash ItemDash (L2807) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 8 |
| 4919 | 0x1337 | `CRASH_HAMMER_IDLE` | 2800 | Crash HammerIdle (L2808) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4920 | 0x1338 | `CRASH_HAMMER_WALK` | 2801 | Crash HammerWalk (L2809) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 10 |
| 4921 | 0x1339 | `CRASH_ITM_SHOOT` | 2802 | Crash ItemShoot (L2810) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4922 | 0x133A | `CRASH_ITM_SHOOT_AIR` | 2803 | Crash ItemShootAir (L2811) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 4 |
| 4923 | 0x133B | `CRASH_JAB1` | 2804 | Crash Jab1 (L2812) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4924 | 0x133C | `FOX_DUO_POSE` | 2805 | Fox Duo Pose (L2813) | `src/SinglePlayerModes.asm` | 1 |
| 4925 | 0x133D | `NDRM_CHARACTER` | 2806 | Polygon Dr. Mario Character (L2814) | `src/Character.asm` | 1 |
| 4926 | 0x133E | `CRASH_DASH_ATTACK` | 2807 | Crash DashAttack (L2815) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4927 | 0x133F | `CRASH_F_TILT_HIGH` | 2808 | Crash FTiltHigh (L2816) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4928 | 0x1340 | `CRASH_F_TILT` | 2809 | Crash FTilt (L2817) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4929 | 0x1341 | `CRASH_F_TILT_LOW` | 2810 | Crash FTiltLow (L2818) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4930 | 0x1342 | `CRASH_U_TILT` | 2811 | Crash UTilt (L2819) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4931 | 0x1343 | `CRASH_D_TILT` | 2812 | Crash DTilt (L2820) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4932 | 0x1344 | `CRASH_F_SMASH_HIGH` | 2813 | Crash FSmashHigh (L2821) |  | 0 |
| 4933 | 0x1345 | `CRASH_F_SMASH_MID_HIGH` | 2814 | Crash FSmashMidHigh (L2822) |  | 0 |
| 4934 | 0x1346 | `CRASH_F_SMASH` | 2815 | Crash FSmash (L2823) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4935 | 0x1347 | `CRASH_F_SMASH_MID_LOW` | 2816 | Crash FSmashMidLow (L2824) |  | 0 |
| 4936 | 0x1348 | `CRASH_F_SMASH_LOW` | 2817 | Crash FSmashLow (L2825) |  | 0 |
| 4937 | 0x1349 | `CRASH_U_SMASH` | 2818 | Crash USmash (L2826) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4938 | 0x134A | `CRASH_D_SMASH` | 2819 | Crash DSmash (L2827) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4939 | 0x134B | `CRASH_ATTACK_AIR_N` | 2820 | Crash AttackAirN (L2828) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4940 | 0x134C | `CRASH_ATTACK_AIR_F` | 2821 | Crash AttackAirF (L2829) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4941 | 0x134D | `CRASH_ATTACK_AIR_B` | 2822 | Crash AttackAirB (L2830) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4942 | 0x134E | `CRASH_ATTACK_AIR_U` | 2823 | Crash AttackAirU (L2831) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4943 | 0x134F | `CRASH_ATTACK_AIR_D` | 2824 | Crash AttackAirD (L2832) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4944 | 0x1350 | `CRASH_LANDING_AIR_F` | 2825 | Crash LandingAirF (L2833) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4945 | 0x1351 | `CRASH_LANDING_AIR_B` | 2826 | Crash LandingAirB (L2834) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4946 | 0x1352 | `CRASH_LANDING_AIR_U` | 2827 | Crash LandingAirU (L2835) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4947 | 0x1353 | `CRASH_ENTER_PIPE` | 2828 | Crash EnterPipe (L2836) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4948 | 0x1354 | `CRASH_EXIT_PIPE` | 2829 | Crash ExitPipe (L2837) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4949 | 0x1355 | `CRASH_EXIT_PIPE_WALK` | 2830 | Crash ExitPipeWalk (L2838) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4950 | 0x1356 | `CRASH_APPEAR_L` | 2831 | Crash AppearL (L2839) | `src/Crash/Crash.asm` | 1 |
| 4951 | 0x1357 | `CRASH_APPEAR_R` | 2832 | Crash AppearR (L2840) | `src/Crash/Crash.asm` | 1 |
| 4952 | 0x1358 | `CRASH_VICTORY_1` | 2833 | Crash Victory 1 (L2841) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4953 | 0x1359 | `CRASH_VICTORY_2` | 2834 | Crash Victory 2 (L2842) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4954 | 0x135A | `CRASH_VICTORY_3` | 2835 | Crash Victory 3 (L2843) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4955 | 0x135B | `CRASH_CLAP` | 2836 | Crash Clap (L2844) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4956 | 0x135C | `CRASH_CONTINUE_FALL` | 2837 | Crash Continue Fall (L2845) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4957 | 0x135D | `CRASH_CONTINUE_UP` | 2838 | Crash Continue Up (L2846) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4958 | 0x135E | `CRASH_1P_POSE` | 2839 | Crash 1P Pose (L2847) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4959 | 0x135F | `CRASH_CPU_POSE` | 2840 | Crash CPU Pose (L2848) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 4960 | 0x1360 | `CRASH_NSPG` | 2841 | Crash NSPG (L2849) | `src/Crash/Crash.asm` | 1 |
| 4961 | 0x1361 | `CRASH_NSPA` | 2842 | Crash NSPA (L2850) | `src/Crash/Crash.asm` | 1 |
| 4962 | 0x1362 | `CRASH_USPA` | 2843 | Crash USPA (L2851) |  | 0 |
| 4963 | 0x1363 | `CRASH_USPG` | 2844 | Crash USPG (L2852) | `src/Crash/Crash.asm` | 2 |
| 4964 | 0x1364 | `CRASH_USPLanding` | 2845 | Crash USPLanding (L2853) | `src/Crash/Crash.asm` | 1 |
| 4965 | 0x1365 | `CRASH_DSP_AIR_DIVE` | 2846 | Crash DSPAirDive (L2854) | `src/Crash/Crash.asm` | 1 |
| 4966 | 0x1366 | `TRANSITION_REMIX_LOGO` | 2847 | Transition Remix Logo (L2855) | `src/Render.asm`, `src/Transitions.asm` | 2 |
| 4967 | 0x1367 | `DL_BETA_DL` | 2848 | Dreamland Beta DL Stage (L2856) |  | 0 |
| 4968 | 0x1368 | `DL_BETA_DL_HEADER` | 2849 | Dreamland Beta DL Header (L2857) |  | 0 |
| 4969 | 0x1369 | `LMAO_CASTLE` | 2850 | LMAO Castle Stage (L2858) |  | 0 |
| 4970 | 0x136A | `LMAO_CASTLE_HEADER` | 2851 | LMAO Castle Header (L2859) |  | 0 |
| 4971 | 0x136B | `SLIPPY_SHOOT_GROUND` | 2852 | Slippy Shoot Ground (L2860) | `src/Slippy/Slippy.asm` | 1 |
| 4972 | 0x136C | `SLIPPY_SHOOT_AIR` | 2853 | Slippy Shoot Aerial (L2861) | `src/Slippy/Slippy.asm` | 1 |
| 4973 | 0x136D | `CRASH_SHIELD_POSE` | 2854 | Crash Shield Pose (L2862) | `src/Character.asm` | 2 |
| 4974 | 0x136E | `PEACH_MAIN` | 2855 | Peach Main (L2863) | `src/Character.asm` | 1 |
| 4975 | 0x136F | `PEACH_CHARACTER` | 2856 | Peach Character (L2864) | `src/Character.asm` | 1 |
| 4976 | 0x1370 | `PEACH_SHIELD_POSE` | 2857 | Peach Shield Pose (L2865) | `src/Character.asm` | 2 |
| 4977 | 0x1371 | `PEACH_IDLE` | 2858 | Peach Idle (L2866) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 14 |
| 4978 | 0x1372 | `PEACH_WALK1` | 2859 | Peach Walk1 (L2867) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4979 | 0x1373 | `PEACH_WALK2` | 2860 | Peach Walk2 (L2868) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4980 | 0x1374 | `PEACH_WALK3` | 2861 | Peach Walk3 (L2869) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4981 | 0x1375 | `PEACH_FLOAT` | 2862 | Peach Float (L2870) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4982 | 0x1376 | `PEACH_DASH` | 2863 | Peach Dash (L2871) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4983 | 0x1377 | `PEACH_RUN` | 2864 | Peach Run (L2872) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4984 | 0x1378 | `PEACH_RUN_BRAKE` | 2865 | Peach RunBrake (L2873) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4985 | 0x1379 | `PEACH_TURN` | 2866 | Peach Turn (L2874) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4986 | 0x137A | `PEACH_TURN_RUN` | 2867 | Peach TurnRun (L2875) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4987 | 0x137B | `PEACH_JUMP_F` | 2868 | Peach JumpF (L2876) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4988 | 0x137C | `PEACH_JUMP_B` | 2869 | Peach JumpB (L2877) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4989 | 0x137D | `PEACH_JUMP_AERIAL_F` | 2870 | Peach JumpAerialF (L2878) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4990 | 0x137E | `PEACH_JUMP_AERIAL_B` | 2871 | Peach JumpAerialB (L2879) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4991 | 0x137F | `PEACH_FALL` | 2872 | Peach Fall (L2880) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4992 | 0x1380 | `PEACH_FALL_AERIAL` | 2873 | Peach FallAerial (L2881) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4993 | 0x1381 | `PEACH_CROUCH` | 2874 | Peach Crouch (L2882) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4994 | 0x1382 | `PEACH_CROUCH_IDLE` | 2875 | Peach CrouchIdle (L2883) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4995 | 0x1383 | `PEACH_CROUCH_END` | 2876 | Peach CrouchEnd (L2884) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4996 | 0x1384 | `PEACH_LANDING` | 2877 | Peach Landing (L2885) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 12 |
| 4997 | 0x1385 | `PEACH_PLAT_DROP` | 2878 | Peach PlatformDrop (L2886) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 4998 | 0x1386 | `PEACH_TEETER` | 2879 | Peach Teeter (L2887) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 4999 | 0x1387 | `PEACH_TEETER_START` | 2880 | Peach Teeterstart (L2888) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5000 | 0x1388 | `PEACH_STUN` | 2881 | Peach Stun (L2889) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5001 | 0x1389 | `PEACH_DAMAGE_HIGH1` | 2882 | Peach DamageHigh1 (L2890) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5002 | 0x138A | `PEACH_DAMAGE_HIGH2` | 2883 | Peach DamageHigh2 (L2891) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5003 | 0x138B | `PEACH_DAMAGE_HIGH3` | 2884 | Peach DamageHigh3 (L2892) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5004 | 0x138C | `PEACH_DAMAGE_MID1` | 2885 | Peach DamageMid1 (L2893) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5005 | 0x138D | `PEACH_DAMAGE_MID2` | 2886 | Peach DamageMid2 (L2894) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5006 | 0x138E | `PEACH_DAMAGE_MID3` | 2887 | Peach DamageMid3 (L2895) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5007 | 0x138F | `PEACH_DAMAGE_LOW1` | 2888 | Peach DamageLow1 (L2896) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5008 | 0x1390 | `PEACH_DAMAGE_LOW2` | 2889 | Peach DamageLow2 (L2897) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5009 | 0x1391 | `PEACH_DAMAGE_LOW3` | 2890 | Peach DamageLow3 (L2898) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5010 | 0x1392 | `PEACH_DAMAGE_AIR1` | 2891 | Peach DamageAir1 (L2899) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5011 | 0x1393 | `PEACH_DAMAGE_AIR2` | 2892 | Peach DamageAir2 (L2900) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5012 | 0x1394 | `PEACH_DAMAGE_AIR3` | 2893 | Peach DamageAir3 (L2901) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5013 | 0x1395 | `PEACH_DAMAGE_FLY_HIGH` | 2894 | Peach DamageFlyHigh (L2902) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5014 | 0x1396 | `PEACH_DAMAGE_FLY_MID` | 2895 | Peach DamageFlyMid (L2903) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5015 | 0x1397 | `PEACH_DAMAGE_FLY_LOW` | 2896 | Peach DamageFlyLow (L2904) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5016 | 0x1398 | `PEACH_DAMAGE_FLY_TOP` | 2897 | Peach DamageFlyTop (L2905) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5017 | 0x1399 | `PEACH_DAMAGE_FLY_ROLL` | 2898 | Peach DamageFlyRoll (L2906) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5018 | 0x139A | `PEACH_DAMAGE_ELEC` | 2899 | Peach DamageElec (L2907) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5019 | 0x139B | `PEACH_TUMBLE` | 2900 | Peach Tumble (L2908) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 20 |
| 5020 | 0x139C | `PEACH_FALL_SPECIAL` | 2901 | Peach FallSpecial (L2909) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5021 | 0x139D | `PEACH_CEILING_BONK` | 2902 | Peach CeilingBonk (L2910) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5022 | 0x139E | `PEACH_DOWN_BOUNCE_D` | 2903 | Peach DownBounceD (L2911) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 6 |
| 5023 | 0x139F | `PEACH_DOWN_BOUNCE_U` | 2904 | Peach DownBounceU (L2912) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5024 | 0x13A0 | `PEACH_DOWN_STAND_D` | 2905 | Peach DownStandD (L2913) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 6 |
| 5025 | 0x13A1 | `PEACH_DOWN_STAND_U` | 2906 | Peach DownStandU (L2914) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm`, `src/TwelveCharBattle.asm` | 6 |
| 5026 | 0x13A2 | `PEACH_TECH_F` | 2907 | Peach TechF (L2915) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5027 | 0x13A3 | `PEACH_TECH_B` | 2908 | Peach TechB (L2916) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5028 | 0x13A4 | `PEACH_DOWN_FORWARD_D` | 2909 | Peach DownForwardD (L2917) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5029 | 0x13A5 | `PEACH_DOWN_FORWARD_U` | 2910 | Peach DownForwardU (L2918) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5030 | 0x13A6 | `PEACH_DOWN_BACK_D` | 2911 | Peach DownBackD (L2919) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5031 | 0x13A7 | `PEACH_DOWN_BACK_U` | 2912 | Peach DownBackU (L2920) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5032 | 0x13A8 | `PEACH_DOWN_ATK_D` | 2913 | Peach DownAttackD (L2921) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5033 | 0x13A9 | `PEACH_DOWN_ATK_U` | 2914 | Peach DownAttackU (L2922) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5034 | 0x13AA | `PEACH_TECH` | 2915 | Peach Tech (L2923) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5035 | 0x13AB | `PEACH_CLANG_RECOIL` | 2916 | Peach ClangRecoil (L2924) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5036 | 0x13AC | `PEACH_SHIELD_ON` | 2917 | Peach ShieldOn (L2925) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5037 | 0x13AD | `PEACH_SHIELD_OFF` | 2918 | Peach ShieldOff (L2926) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5038 | 0x13AE | `PEACH_ROLL_F` | 2919 | Peach RollF (L2927) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5039 | 0x13AF | `PEACH_ROLL_B` | 2920 | Peach RollB (L2928) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5040 | 0x13B0 | `PEACH_GRAB` | 2921 | Peach Grab (L2929) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5041 | 0x13B1 | `PEACH_GRAB_PULL` | 2922 | Peach GrabPull (L2930) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5042 | 0x13B2 | `PEACH_THROW_F` | 2923 | Peach ThrowF (L2931) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5043 | 0x13B3 | `PEACH_THROW_B` | 2924 | Peach ThrowB (L2932) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5044 | 0x13B4 | `PEACH_CAPTURE_PULLED` | 2925 | Peach CapturePulled (L2933) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5045 | 0x13B5 | `PEACH_THROWN_DK_PULLED` | 2926 | Peach ThrownDKPulled (L2934) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5046 | 0x13B6 | `PEACH_THROWN_MARIO_BROS` | 2927 | Peach ThrownMarioBros (L2935) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5047 | 0x13B7 | `PEACH_THROWN_DK` | 2928 | Peach ThrownDK (L2936) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5048 | 0x13B8 | `PEACH_THROWN1` | 2929 | Peach Thrown1 (L2937) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5049 | 0x13B9 | `PEACH_THROWN2` | 2930 | Peach Thrown2 (L2938) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5050 | 0x13BA | `PEACH_THROWN3` | 2931 | Peach Thrown3 (L2939) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5051 | 0x13BB | `PEACH_THROWN_FOX_B` | 2932 | Peach ThrownFoxB (L2940) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5052 | 0x13BC | `PEACH_TAUNT` | 2933 | Peach Taunt (L2941) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5053 | 0x13BD | `PEACH_CLF_CATCH` | 2934 | Peach CliffCatch (L2942) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5054 | 0x13BE | `PEACH_CLF_WAIT` | 2935 | Peach CliffWait (L2943) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5055 | 0x13BF | `PEACH_CLF_Q` | 2936 | Peach CliffQuick (L2944) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5056 | 0x13C0 | `PEACH_CLF_CLM_Q1` | 2937 | Peach CliffClimbQuick1 (L2945) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5057 | 0x13C1 | `PEACH_CLF_CLM_Q2` | 2938 | Peach CliffClimbQuick2 (L2946) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5058 | 0x13C2 | `PEACH_CLF_S` | 2939 | Peach CliffSlow (L2947) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5059 | 0x13C3 | `PEACH_CLF_CLM_S1` | 2940 | Peach CliffClimbSlow1 (L2948) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5060 | 0x13C4 | `PEACH_CLF_CLM_S2` | 2941 | Peach CliffClimbSlow2 (L2949) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5061 | 0x13C5 | `PEACH_CLF_ATK_Q1` | 2942 | Peach CliffAttackQuick1 (L2950) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5062 | 0x13C6 | `PEACH_CLF_ATK_Q2` | 2943 | Peach CliffAttackQuick2 (L2951) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5063 | 0x13C7 | `PEACH_CLF_ATK_S1` | 2944 | Peach CliffAttackSlow1 (L2952) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5064 | 0x13C8 | `PEACH_CLF_ATK_S2` | 2945 | Peach CliffAttackSlow2 (L2953) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5065 | 0x13C9 | `PEACH_CLF_ESC_Q1` | 2946 | Peach CliffEscapeQuick1 (L2954) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5066 | 0x13CA | `PEACH_CLF_ESC_Q2` | 2947 | Peach CliffEscapeQuick2 (L2955) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5067 | 0x13CB | `PEACH_CLF_ESC_S1` | 2948 | Peach CliffEscapeSlow1 (L2956) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5068 | 0x13CC | `PEACH_CLF_ESC_S2` | 2949 | Peach CliffEscapeSlow2 (L2957) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5069 | 0x13CD | `PEACH_L_ITM_PICKUP` | 2950 | Peach LightItemPickup (L2958) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5070 | 0x13CE | `PEACH_ITM_THROW_F` | 2951 | Peach ItemThrowF (L2959) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 8 |
| 5071 | 0x13CF | `PEACH_ITM_THROW_U` | 2952 | Peach ItemThrowU (L2960) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5072 | 0x13D0 | `PEACH_ITM_THROW_D` | 2953 | Peach ItemThrowD (L2961) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5073 | 0x13D1 | `PEACH_ITM_THROW_DASH` | 2954 | Peach ItemThrowDash (L2962) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5074 | 0x13D2 | `PEACH_ITM_DROP` | 2955 | Peach ItemDrop (L2963) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5075 | 0x13D3 | `PEACH_ITM_THROW_AIR_F` | 2956 | Peach ItemThrowAirF (L2964) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 8 |
| 5076 | 0x13D4 | `PEACH_ITM_THROW_AIR_U` | 2957 | Peach ItemThrowAirU (L2965) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5077 | 0x13D5 | `PEACH_ITM_THROW_AIR_D` | 2958 | Peach ItemThrowAirD (L2966) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5078 | 0x13D6 | `PEACH_H_ITM_PICKUP` | 2959 | Peach HeavyItemPickup (L2967) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5079 | 0x13D7 | `PEACH_H_ITM_THROW` | 2960 | Peach HeavyItemThrow (L2968) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 8 |
| 5080 | 0x13D8 | `PEACH_ITM_JAB` | 2961 | Peach ItemJab (L2969) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 8 |
| 5081 | 0x13D9 | `PEACH_ITM_TILT` | 2962 | Peach ItemTilt (L2970) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 8 |
| 5082 | 0x13DA | `PEACH_ITM_SMASH` | 2963 | Peach ItemSmash (L2971) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 8 |
| 5083 | 0x13DB | `PEACH_ITM_DASH` | 2964 | Peach ItemDash (L2972) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 8 |
| 5084 | 0x13DC | `PEACH_HAMMER_IDLE` | 2965 | Peach HammerIdle (L2973) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5085 | 0x13DD | `PEACH_HAMMER_WALK` | 2966 | Peach HammerWalk (L2974) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 10 |
| 5086 | 0x13DE | `PEACH_ITM_SHOOT` | 2967 | Peach ItemShoot (L2975) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5087 | 0x13DF | `PEACH_ITM_SHOOT_AIR` | 2968 | Peach ItemShootAir (L2976) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5088 | 0x13E0 | `PEACH_JAB1` | 2969 | Peach Jab1 (L2977) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5089 | 0x13E1 | `PEACH_JAB2` | 2970 | Peach Jab2 (L2978) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5090 | 0x13E2 | `PEACH_DASH_ATTACK` | 2971 | Peach DashAttack (L2979) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5091 | 0x13E3 | `SOCCER_HEADER` | 2972 | Soccer Stage Header (L2980) |  | 0 |
| 5092 | 0x13E4 | `SOCCER_STAGE` | 2973 | Soccer Stage (L2981) |  | 0 |
| 5093 | 0x13E5 | `PEACH_F_TILT` | 2974 | Peach FTilt (L2982) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5094 | 0x13E6 | `CONKER_ENTER_PIPE` | 2975 | Conker Pipe Enter (L2983) | `src/Conker/Conker.asm` | 1 |
| 5095 | 0x13E7 | `CONKER_EXIT_PIPE` | 2976 | Conker Pipe Exit (L2984) | `src/Conker/Conker.asm` | 1 |
| 5096 | 0x13E8 | `PEACH_U_TILT` | 2977 | Peach UTilt (L2985) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5097 | 0x13E9 | `PEACH_D_TILT` | 2978 | Peach DTilt (L2986) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5098 | 0x13EA | `PEACH_F_SMASH` | 2979 | Peach FSmash (L2987) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 10 |
| 5099 | 0x13EB | `PEACH_U_SMASH` | 2980 | Peach USmash (L2988) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5100 | 0x13EC | `PEACH_D_SMASH` | 2981 | Peach DSmash (L2989) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5101 | 0x13ED | `PEACH_ATTACK_AIR_N` | 2982 | Peach AttackAirN (L2990) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5102 | 0x13EE | `PEACH_ATTACK_AIR_F` | 2983 | Peach AttackAirF (L2991) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5103 | 0x13EF | `PEACH_ATTACK_AIR_B` | 2984 | Peach AttackAirB (L2992) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5104 | 0x13F0 | `PEACH_ATTACK_AIR_U` | 2985 | Peach AttackAirU (L2993) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5105 | 0x13F1 | `PEACH_ATTACK_AIR_D` | 2986 | Peach AttackAirD (L2994) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5106 | 0x13F2 | `PEACH_LANDING_AIR_F` | 2987 | Peach LandingAirF (L2995) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5107 | 0x13F3 | `PEACH_LANDING_AIR_B` | 2988 | Peach LandingAirB (L2996) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5108 | 0x13F4 | `PEACH_ENTER_PIPE` | 2989 | Peach EnterPipe (L2997) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5109 | 0x13F5 | `PEACH_EXIT_PIPE` | 2990 | Peach ExitPipe (L2998) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5110 | 0x13F6 | `PEACH_EXIT_PIPE_WALK` | 2991 | Peach ExitPipeWalk (L2999) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5111 | 0x13F7 | `PEACH_ENTRY_L` | 2992 | Peach Entry Left (L3000) | `src/Peach/Peach.asm` | 1 |
| 5112 | 0x13F8 | `PEACH_ENTRY_R` | 2993 | Peach Entry Right (L3001) | `src/Peach/Peach.asm` | 1 |
| 5113 | 0x13F9 | `DRM_VICTORY_2` | 2994 | Dr. Mario Victory 2 (L3002) | `src/DrMario/DrMario.asm`, `src/NDrMario/NDrMario.asm` | 2 |
| 5114 | 0x13FA | `PEACH_VICTORY_IMAGE_BOTTOM` | 2995 | Peach Victory Image Bottom (L3003) | `src/Peach/Peach.asm` | 1 |
| 5115 | 0x13FB | `PEACH_VICTORY_IMAGE_TOP` | 2996 | Peach Victory Image Top (L3004) |  | 0 |
| 5116 | 0x13FC | `CRASH_VICTORY_IMAGE_BOTTOM` | 2997 | Crash Victory Image Bottom (L3005) | `src/Crash/Crash.asm` | 1 |
| 5117 | 0x13FD | `CRASH_VICTORY_IMAGE_TOP` | 2998 | Crash Victory Image Top (L3006) |  | 0 |
| 5118 | 0x13FE | `PEACH_USPG` | 2999 | Peach USPG (L3007) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5119 | 0x13FF | `PEACH_USPA` | 3000 | Peach USPA (L3008) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5120 | 0x1400 | `PEACH_USP_OPEN` | 3001 | Peach USPOpen (L3009) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5121 | 0x1401 | `PEACH_USP_FLOAT` | 3002 | Peach USPFloat (L3010) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5122 | 0x1402 | `PEACH_USP_CLOSE` | 3003 | Peach USPClose (L3011) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5123 | 0x1403 | `ROY_U_TILT` | 3004 | Roy Up Tilt (L3012) | `src/Roy/Roy.asm` | 1 |
| 5124 | 0x1404 | `DSAMUS_WALK_1` | 3005 | Dark Samus Walk 1 (L3013) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 5125 | 0x1405 | `DSAMUS_WALK_2` | 3006 | Dark Samus Walk 2 (L3014) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 5126 | 0x1406 | `DSAMUS_DSP_LANDING` | 3007 | Dark Samus DSPLanding (L3015) | `src/DSamus/DSamus.asm` | 1 |
| 5127 | 0x1407 | `PEACH_TURNIPPULL` | 3008 | Peach Turnip Pull (L3016) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5128 | 0x1408 | `PEACH_NSP_G` | 3009 | Peach Neutral Special (L3017) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5129 | 0x1409 | `PEACH_NSP_A` | 3010 | Peach Neutral Special Air (L3018) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5130 | 0x140A | `PEACH_NSP_RECOIL` | 3011 | Peach Neutral Special Recoil (L3019) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5131 | 0x140B | `PEACH_BTT_HEADER` | 3012 | Peach Break the Targets Header (L3020) |  | 0 |
| 5132 | 0x140C | `PEACH_BTT` | 3013 | Peach Break the Targets Stage (L3021) |  | 0 |
| 5133 | 0x140D | `CRASH_NSPG_BLOCKED` | 3014 | Crash NSPG Blocked (L3022) | `src/Crash/Crash.asm` | 1 |
| 5134 | 0x140E | `CRASH_NSPA_BLOCKED` | 3015 | Crash NSPA Blocked (L3023) | `src/Crash/Crash.asm` | 1 |
| 5135 | 0x140F | `PEACH_VICTORY1` | 3016 | Peach Victory1 (L3024) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5136 | 0x1410 | `PEACH_VICTORY2` | 3017 | Peach Victory2 (L3025) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5137 | 0x1411 | `PEACH_VICTORY3` | 3018 | Peach Victory3 (L3026) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 4 |
| 5138 | 0x1412 | `PEACH_CLAP` | 3019 | Peach Clap (L3027) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5139 | 0x1413 | `PEACH_CONTINUE_FALL` | 3020 | Peach ContinueFall (L3028) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5140 | 0x1414 | `PEACH_CONTINUE_UP` | 3021 | Peach ContinueUp (L3029) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5141 | 0x1415 | `PEACH_1P_POSE` | 3022 | Peach 1PPose (L3030) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5142 | 0x1416 | `PEACH_CPU_POSE` | 3023 | Peach CPUPose (L3031) | `src/NPeach/NPeach.asm`, `src/Peach/Peach.asm` | 2 |
| 5143 | 0x1417 | `PEACH_TURNIP_INFO` | 3024 | Peach Turnip Info (L3032) | `src/Character.asm`, `src/TagTeam.asm` | 3 |
| 5144 | 0x1418 | `PEACH_TURNIP_GFX` | 3025 | Peach Turnip Graphics (L3033) |  | 0 |
| 5145 | 0x1419 | `PEACH_TEAM_POSE` | 3026 | Peach Team Pose (L3034) | `src/SinglePlayerModes.asm` | 3 |
| 5146 | 0x141A | `CRASH_SPIN_GFX` | 3027 | Crash Spin GFX (L3035) | `src/Character.asm` | 2 |
| 5147 | 0x141B | `CRASH_CSS` | 3028 | Crash CSS (L3036) | `src/Crash/Crash.asm`, `src/NCrash/NCrash.asm` | 2 |
| 5148 | 0x141C | `DSAMUS_NSP_START` | 3029 | Dark Samus NSP Start (L3037) | `src/DSamus/DSamus.asm` | 1 |
| 5149 | 0x141D | `DSAMUS_NSP_CHARGE` | 3030 | Dark Samus NSP Charge (L3038) | `src/DSamus/DSamus.asm` | 1 |
| 5150 | 0x141E | `DSAMUS_NSP_SHOOT` | 3031 | Dark Samus NSP Shoot (L3039) | `src/DSamus/DSamus.asm` | 1 |
| 5151 | 0x141F | `DSAMUS_NSP_START_AIR` | 3032 | Dark Samus NSP Start Air (L3040) | `src/DSamus/DSamus.asm` | 1 |
| 5152 | 0x1420 | `DSAMUS_NSP_CHARGE_AIR` | 3033 | Dark Samus NSP Charge Air (L3041) | `src/DSamus/DSamus.asm` | 1 |
| 5153 | 0x1421 | `DSAMUS_NSP_SHOOT_AIR` | 3034 | Dark Samus NSP Shoot Air (L3042) | `src/DSamus/DSamus.asm` | 1 |
| 5154 | 0x1422 | `CRASH_ENTRY` | 3035 | Crash Entry Object (L3043) | `src/Character.asm` | 2 |
| 5155 | 0x1423 | `DSAMUS_IDLE_CSS` | 3036 | Dark Samus Idle CSS (L3044) | `src/DSamus/DSamus.asm`, `src/NDSamus/NDSamus.asm` | 2 |
| 5156 | 0x1424 | `CRASH_DIG_GFX` | 3037 | Crash Dig GFX (L3045) |  | 0 |
| 5157 | 0x1425 | `CRASH_DSP_BEGIN` | 3038 | Crash DSP Begin (L3046) | `src/Crash/Crash.asm` | 1 |
| 5158 | 0x1426 | `CRASH_DSP_PLATFORM` | 3039 | Crash DSP Platform (L3047) | `src/Crash/Crash.asm` | 1 |
| 5159 | 0x1427 | `DISCOVERY_FALLS_BG` | 3040 | Discovery Falls BG (L3048) |  | 0 |
| 5160 | 0x1428 | `DISCOVERY_FALLS_STAGE` | 3041 | Discovery Falls Stage (L3049) |  | 0 |
| 5161 | 0x1429 | `DISCOVERY_FALLS_HEADER` | 3042 | Discovery Falls Header (L3050) |  | 0 |
| 5162 | 0x142A | `ROY_MAIN` | 3043 | Roy Main (L3051) | `src/Character.asm` | 1 |
| 5163 | 0x142B | `ROY_CHARACTER` | 3044 | Roy Character (L3052) | `src/Character.asm` | 1 |
| 5164 | 0x142C | `ROY_TAUNT` | 3045 | Roy Taunt (L3053) | `src/Roy/Roy.asm` | 1 |
| 5165 | 0x142D | `ROY_1P_POSE` | 3046 | Roy 1P Pose (L3054) | `src/Roy/Roy.asm` | 1 |
| 5166 | 0x142E | `ROY_VICTORY_1` | 3047 | Roy Victory 1 (L3055) | `src/Roy/Roy.asm` | 2 |
| 5167 | 0x142F | `ROY_DSPG_BEGIN` | 3048 | Roy Down Special Ground Begin (L3056) | `src/Roy/Roy.asm` | 1 |
| 5168 | 0x1430 | `ROY_DSPG_WAIT` | 3049 | Roy Down Special Ground Wait (L3057) | `src/Roy/Roy.asm` | 1 |
| 5169 | 0x1431 | `ROY_DSPG_END` | 3050 | Roy Down Special Ground End (L3058) | `src/Roy/Roy.asm` | 2 |
| 5170 | 0x1432 | `ROY_DSPA_BEGIN` | 3051 | Roy Down Special Air Begin (L3059) | `src/Roy/Roy.asm` | 1 |
| 5171 | 0x1433 | `ROY_DSPA_WAIT` | 3052 | Roy Down Special Air Wait (L3060) | `src/Roy/Roy.asm` | 1 |
| 5172 | 0x1434 | `ROY_DSPA_END` | 3053 | Roy Down Special Air End (L3061) | `src/Roy/Roy.asm` | 2 |
| 5173 | 0x1435 | `ROY_VICTORY_2` | 3054 | Roy Victory 2 (L3062) | `src/Roy/Roy.asm` | 1 |
| 5174 | 0x1436 | `ROY_VICTORY_3` | 3055 | Roy Victory 3 (L3063) | `src/Roy/Roy.asm` | 1 |
| 5175 | 0x1437 | `ROY_JAB_1` | 3056 | Roy Jab 1 (L3064) | `src/Roy/Roy.asm` | 1 |
| 5176 | 0x1438 | `ROY_DASH_ATTACK` | 3057 | Roy Dash Attack (L3065) | `src/Roy/Roy.asm` | 1 |
| 5177 | 0x1439 | `ROY_DUO_POSE` | 3058 | Roy Duo Pose (L3066) | `src/SinglePlayerModes.asm` | 1 |
| 5178 | 0x143A | `MARTH_DUO_POSE` | 3059 | Marth Duo Pose (L3067) | `src/SinglePlayerModes.asm` | 1 |
| 5179 | 0x143B | `CRASH_DSP_ATTACK` | 3060 | Crash DSP Attack (L3068) | `src/Crash/Crash.asm` | 1 |
| 5180 | 0x143C | `CRASH_TEAM_POSE` | 3061 | Crash Team Pose (L3069) | `src/SinglePlayerModes.asm` | 3 |
| 5181 | 0x143D | `CRASH_DSP_WAIT` | 3062 | Crash DSP Wait (L3070) | `src/Crash/Crash.asm` | 1 |
| 5182 | 0x143E | `CRASH_DSP_TURN` | 3063 | Crash DSP Turn (L3071) | `src/Crash/Crash.asm` | 1 |
| 5183 | 0x143F | `CRASH_BTT_HEADER` | 3064 | Crash Break the Targets Header (L3072) |  | 0 |
| 5184 | 0x1440 | `CRASH_BTT_STAGE` | 3065 | Crash Break the Targets Stage (L3073) |  | 0 |
| 5185 | 0x1441 | `DISCOVERY_FALLS_REMIX_STAGE` | 3066 | Discovery Falls Remix Stage (L3074) |  | 0 |
| 5186 | 0x1442 | `DISCOVERY_FALLS_REMIX_HEADER` | 3067 | Discovery Falls Remix Header (L3075) |  | 0 |
| 5187 | 0x1443 | `N64_REMIX_STAGE` | 3068 | N64 Remix Stage (L3076) |  | 0 |
| 5188 | 0x1444 | `N64_REMIX_HEADER` | 3069 | N64 Remix Header (L3077) |  | 0 |
| 5189 | 0x1445 | `RANDOM_MAIN` | 3070 | Random Main File (L3078) | `src/Character.asm` | 1 |
| 5190 | 0x1446 | `RANDOM_CHARACTER` | 3071 | Random Character File (L3079) | `src/Character.asm` | 1 |
| 5191 | 0x1447 | `RANDOM_IDLE` | 3072 | Random Idle (L3080) | `src/Random/Random.asm` | 21 |
| 5192 | 0x1448 | `RANDOM_SELECT` | 3073 | Random Selected Animation (L3081) | `src/Random/Random.asm` | 3 |
| 5193 | 0x1449 | `STG_CRASH_PLATFORMS_STAGE` | 3074 | Crash Board the Platforms Stage (L3082) |  | 0 |
| 5194 | 0x144A | `STG_CRASH_PLATFORMS_HEADER` | 3075 | Crash Board the Platforms Header (L3083) |  | 0 |
| 5195 | 0x144B | `DRL_MAIN` | 3076 | Dr. Luigi Main (L3084) | `src/Character.asm` | 1 |
| 5196 | 0x144C | `DRL_CHARACTER` | 3077 | Dr. Luigi Character (L3085) | `src/Character.asm` | 1 |
| 5197 | 0x144D | `DRL_UTILT` | 3078 | Dr. Luigi UTilt (L3086) | `src/DrLuigi/DrLuigi.asm` | 1 |
| 5198 | 0x144E | `DRL_JAB3` | 3079 | Dr. Luigi Jab3 (L3087) | `src/DrLuigi/DrLuigi.asm` | 1 |
| 5199 | 0x144F | `DRL_ATTACKAIRF` | 3080 | Dr. Luigi AttackAirF (L3088) | `src/DrLuigi/DrLuigi.asm` | 1 |
| 5200 | 0x1450 | `DRL_ATTACKAIRD` | 3081 | Dr. Luigi AttackAirD (L3089) | `src/DrLuigi/DrLuigi.asm` | 1 |
| 5201 | 0x1451 | `DRL_VICTORY_1` | 3082 | Dr. Luigi Victory 1 (L3090) | `src/DrLuigi/DrLuigi.asm` | 1 |
| 5202 | 0x1452 | `KIRBY_CRASH_NSP_G` | 3083 | Kirby Crash NSPG (L3091) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5203 | 0x1453 | `KIRBY_CRASH_NSP_A` | 3084 | Kirby Crash NSPA (L3092) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5204 | 0x1454 | `KIRBY_CRASH_NSP_G_BLOCKED` | 3085 | Kirby Crash NSPG Blocked (L3093) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5205 | 0x1455 | `KIRBY_CRASH_NSP_A_BLOCKED` | 3086 | Kirby Crash NSPA Blocked (L3094) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5206 | 0x1456 | `LANKY_MAIN` | 3087 | Lanky Main (L3095) | `src/Character.asm` | 1 |
| 5207 | 0x1457 | `LANKY_CHARACTER` | 3088 | Lanky Character (L3096) | `src/Character.asm` | 1 |
| 5208 | 0x1458 | `LANKY_SHIELD_POSE` | 3089 | Lanky Shield Pose (L3097) | `src/Character.asm` | 1 |
| 5209 | 0x1459 | `LANKY_IDLE` | 3090 | Lanky Idle (L3098) | `src/Lanky/Lanky.asm` | 6 |
| 5210 | 0x145A | `LANKY_WALK1` | 3091 | Lanky Walk1 (L3099) | `src/Lanky/Lanky.asm` | 1 |
| 5211 | 0x145B | `LANKY_WALK2` | 3092 | Lanky Walk2 (L3100) | `src/Lanky/Lanky.asm` | 1 |
| 5212 | 0x145C | `LANKY_WALK3` | 3093 | Lanky Walk3 (L3101) | `src/Lanky/Lanky.asm` | 1 |
| 5213 | 0x145D | `LANKY_WALK_END` | 3094 | Lanky WalkEnd (L3102) | `src/Lanky/Lanky.asm` | 1 |
| 5214 | 0x145E | `LANKY_DASH` | 3095 | Lanky Dash (L3103) | `src/Lanky/Lanky.asm` | 1 |
| 5215 | 0x145F | `LANKY_RUN` | 3096 | Lanky Run (L3104) | `src/Lanky/Lanky.asm` | 1 |
| 5216 | 0x1460 | `LANKY_RUN_BRAKE` | 3097 | Lanky RunBrake (L3105) | `src/Lanky/Lanky.asm` | 1 |
| 5217 | 0x1461 | `LANKY_TURN` | 3098 | Lanky Turn (L3106) | `src/Lanky/Lanky.asm` | 1 |
| 5218 | 0x1462 | `LANKY_TURN_RUN` | 3099 | Lanky TurnRun (L3107) | `src/Lanky/Lanky.asm` | 1 |
| 5219 | 0x1463 | `LANKY_JUMP_F` | 3100 | Lanky JumpF (L3108) | `src/Lanky/Lanky.asm` | 1 |
| 5220 | 0x1464 | `LANKY_JUMP_B` | 3101 | Lanky JumpB (L3109) | `src/Lanky/Lanky.asm` | 1 |
| 5221 | 0x1465 | `LANKY_JUMP_AERIAL_F` | 3102 | Lanky JumpAerialF (L3110) | `src/Lanky/Lanky.asm` | 1 |
| 5222 | 0x1466 | `LANKY_JUMP_AERIAL_B` | 3103 | Lanky JumpAerialB (L3111) | `src/Lanky/Lanky.asm` | 1 |
| 5223 | 0x1467 | `LANKY_FALL` | 3104 | Lanky Fall (L3112) | `src/Lanky/Lanky.asm` | 1 |
| 5224 | 0x1468 | `LANKY_FALL_AERIAL` | 3105 | Lanky FallAerial (L3113) | `src/Lanky/Lanky.asm` | 1 |
| 5225 | 0x1469 | `LANKY_CROUCH` | 3106 | Lanky Crouch (L3114) | `src/Lanky/Lanky.asm` | 1 |
| 5226 | 0x146A | `LANKY_CROUCH_IDLE` | 3107 | Lanky CrouchIdle (L3115) | `src/Lanky/Lanky.asm` | 1 |
| 5227 | 0x146B | `LANKY_CROUCH_END` | 3108 | Lanky CrouchEnd (L3116) | `src/Lanky/Lanky.asm` | 1 |
| 5228 | 0x146C | `LANKY_LANDING` | 3109 | Lanky Landing (L3117) | `src/Lanky/Lanky.asm` | 6 |
| 5229 | 0x146D | `LANKY_SHIELD_DROP` | 3110 | Lanky ShieldDrop (L3118) | `src/Lanky/Lanky.asm` | 2 |
| 5230 | 0x146E | `LANKY_TEETER` | 3111 | Lanky Teeter (L3119) | `src/Lanky/Lanky.asm` | 1 |
| 5231 | 0x146F | `LANKY_TEETER_START` | 3112 | Lanky TeeterStart (L3120) | `src/Lanky/Lanky.asm` | 1 |
| 5232 | 0x1470 | `LANKY_STUN` | 3113 | Lanky Stun (L3121) | `src/Lanky/Lanky.asm` | 2 |
| 5233 | 0x1471 | `LANKY_DAMAGE_HIGH1` | 3114 | Lanky DamageHigh1 (L3122) | `src/Lanky/Lanky.asm` | 1 |
| 5234 | 0x1472 | `LANKY_DAMAGE_HIGH2` | 3115 | Lanky DamageHigh2 (L3123) | `src/Lanky/Lanky.asm` | 1 |
| 5235 | 0x1473 | `LANKY_DAMAGE_HIGH3` | 3116 | Lanky DamageHigh3 (L3124) | `src/Lanky/Lanky.asm` | 2 |
| 5236 | 0x1474 | `LANKY_DAMAGE_MID1` | 3117 | Lanky DamageMid1 (L3125) | `src/Lanky/Lanky.asm` | 1 |
| 5237 | 0x1475 | `LANKY_DAMAGE_MID2` | 3118 | Lanky DamageMid2 (L3126) | `src/Lanky/Lanky.asm` | 1 |
| 5238 | 0x1476 | `LANKY_DAMAGE_MID3` | 3119 | Lanky DamageMid3 (L3127) | `src/Lanky/Lanky.asm` | 1 |
| 5239 | 0x1477 | `LANKY_DAMAGE_LOW1` | 3120 | Lanky DamageLow1 (L3128) | `src/Lanky/Lanky.asm` | 1 |
| 5240 | 0x1478 | `LANKY_DAMAGE_LOW2` | 3121 | Lanky DamageLow2 (L3129) | `src/Lanky/Lanky.asm` | 1 |
| 5241 | 0x1479 | `LANKY_DAMAGE_LOW3` | 3122 | Lanky DamageLow3 (L3130) | `src/Lanky/Lanky.asm` | 1 |
| 5242 | 0x147A | `LANKY_DAMAGE_AIR1` | 3123 | Lanky DamageAir1 (L3131) | `src/Lanky/Lanky.asm` | 1 |
| 5243 | 0x147B | `LANKY_DAMAGE_AIR2` | 3124 | Lanky DamageAir2 (L3132) | `src/Lanky/Lanky.asm` | 1 |
| 5244 | 0x147C | `LANKY_DAMAGE_AIR3` | 3125 | Lanky DamageAir3 (L3133) | `src/Lanky/Lanky.asm` | 1 |
| 5245 | 0x147D | `LANKY_DAMAGE_FLY_HIGH` | 3126 | Lanky DamageFlyHigh (L3134) | `src/Lanky/Lanky.asm` | 1 |
| 5246 | 0x147E | `LANKY_DAMAGE_FLY_MID` | 3127 | Lanky DamageFlyMid (L3135) | `src/Lanky/Lanky.asm` | 1 |
| 5247 | 0x147F | `LANKY_DAMAGE_FLY_LOW` | 3128 | Lanky DamageFlyLow (L3136) | `src/Lanky/Lanky.asm` | 1 |
| 5248 | 0x1480 | `LANKY_DAMAGE_FLY_TOP` | 3129 | Lanky DamageFlyTop (L3137) | `src/Lanky/Lanky.asm` | 2 |
| 5249 | 0x1481 | `LANKY_DAMAGE_FLY_ROLL` | 3130 | Lanky DamageFlyRoll (L3138) | `src/Lanky/Lanky.asm` | 1 |
| 5250 | 0x1482 | `LANKY_DAMAGE_ELEC` | 3131 | Lanky DamageElec (L3139) | `src/Lanky/Lanky.asm` | 2 |
| 5251 | 0x1483 | `LANKY_TUMBLE` | 3132 | Lanky Tumble (L3140) | `src/Lanky/Lanky.asm` | 10 |
| 5252 | 0x1484 | `LANKY_FALL_SPECIAL` | 3133 | Lanky FallSpecial (L3141) | `src/Lanky/Lanky.asm` | 1 |
| 5253 | 0x1485 | `LANKY_CEILING_BONK` | 3134 | Lanky CeilingBonk (L3142) | `src/Lanky/Lanky.asm` | 1 |
| 5254 | 0x1486 | `LANKY_DOWN_BOUNCE_D` | 3135 | Lanky DownBounceD (L3143) | `src/Lanky/Lanky.asm` | 3 |
| 5255 | 0x1487 | `LANKY_DOWN_BOUNCE_U` | 3136 | Lanky DownBounceU (L3144) | `src/Lanky/Lanky.asm` | 2 |
| 5256 | 0x1488 | `LANKY_DOWN_STAND_D` | 3137 | Lanky DownStandD (L3145) | `src/Lanky/Lanky.asm` | 3 |
| 5257 | 0x1489 | `LANKY_DOWN_STAND_U` | 3138 | Lanky DownStandU (L3146) | `src/Lanky/Lanky.asm`, `src/TwelveCharBattle.asm` | 3 |
| 5258 | 0x148A | `LANKY_TECH_F` | 3139 | Lanky TechF (L3147) | `src/Lanky/Lanky.asm` | 1 |
| 5259 | 0x148B | `LANKY_TECH_B` | 3140 | Lanky TechB (L3148) | `src/Lanky/Lanky.asm` | 1 |
| 5260 | 0x148C | `LANKY_DOWN_FORWARD_D` | 3141 | Lanky DownForwardD (L3149) | `src/Lanky/Lanky.asm` | 1 |
| 5261 | 0x148D | `LANKY_DOWN_FORWARD_U` | 3142 | Lanky DownForwardU (L3150) | `src/Lanky/Lanky.asm` | 1 |
| 5262 | 0x148E | `LANKY_DOWN_BACK_D` | 3143 | Lanky DownBackD (L3151) | `src/Lanky/Lanky.asm` | 1 |
| 5263 | 0x148F | `LANKY_DOWN_BACK_U` | 3144 | Lanky DownBackU (L3152) | `src/Lanky/Lanky.asm` | 1 |
| 5264 | 0x1490 | `LANKY_DOWN_ATK_D` | 3145 | Lanky DownAttackD (L3153) | `src/Lanky/Lanky.asm` | 1 |
| 5265 | 0x1491 | `LANKY_DOWN_ATK_U` | 3146 | Lanky DownAttackU (L3154) | `src/Lanky/Lanky.asm` | 1 |
| 5266 | 0x1492 | `LANKY_TECH` | 3147 | Lanky Tech (L3155) | `src/Lanky/Lanky.asm` | 1 |
| 5267 | 0x1493 | `LANKY_CLANG_RECOIL` | 3148 | Lanky ClangRecoil (L3156) | `src/Lanky/Lanky.asm` | 1 |
| 5268 | 0x1494 | `LANKY_SHIELD_ON` | 3149 | Lanky ShieldOn (L3157) | `src/Lanky/Lanky.asm` | 1 |
| 5269 | 0x1495 | `LANKY_SHIELD_OFF` | 3150 | Lanky ShieldOff (L3158) | `src/Lanky/Lanky.asm` | 1 |
| 5270 | 0x1496 | `LANKY_ROLL_F` | 3151 | Lanky RollF (L3159) | `src/Lanky/Lanky.asm` | 1 |
| 5271 | 0x1497 | `LANKY_ROLL_B` | 3152 | Lanky RollB (L3160) | `src/Lanky/Lanky.asm` | 1 |
| 5272 | 0x1498 | `LANKY_GRAB` | 3153 | Lanky Grab (L3161) | `src/Lanky/Lanky.asm` | 1 |
| 5273 | 0x1499 | `LANKY_GRAB_PULL` | 3154 | Lanky GrabPull (L3162) | `src/Lanky/Lanky.asm` | 1 |
| 5274 | 0x149A | `LANKY_THROW_F` | 3155 | Lanky ThrowF (L3163) | `src/Lanky/Lanky.asm` | 1 |
| 5275 | 0x149B | `LANKY_THROW_B` | 3156 | Lanky ThrowB (L3164) | `src/Lanky/Lanky.asm` | 1 |
| 5276 | 0x149C | `LANKY_CAPTURE_PULLED` | 3157 | Lanky CapturePulled (L3165) | `src/Lanky/Lanky.asm` | 2 |
| 5277 | 0x149D | `LANKY_THROWN_DK_PULLED` | 3158 | Lanky ThrownDKPulled (L3166) | `src/Lanky/Lanky.asm` | 1 |
| 5278 | 0x149E | `LANKY_THROWN_MARIO_BROS` | 3159 | Lanky ThrownMarioBros (L3167) | `src/Lanky/Lanky.asm` | 1 |
| 5279 | 0x149F | `LANKY_THROWN_DK` | 3160 | Lanky ThrownDK (L3168) | `src/Lanky/Lanky.asm` | 1 |
| 5280 | 0x14A0 | `LANKY_THROWN2` | 3161 | Lanky Thrown2 (L3169) | `src/Lanky/Lanky.asm` | 1 |
| 5281 | 0x14A1 | `LANKY_THROWN1` | 3162 | Lanky Thrown1 (L3170) | `src/Lanky/Lanky.asm` | 1 |
| 5282 | 0x14A2 | `LANKY_TAUNT` | 3163 | Lanky Taunt (L3171) | `src/Lanky/Lanky.asm` | 1 |
| 5283 | 0x14A3 | `LANKY_CLF_CATCH` | 3164 | Lanky CliffCatch (L3172) | `src/Lanky/Lanky.asm` | 1 |
| 5284 | 0x14A4 | `LANKY_CLF_WAIT` | 3165 | Lanky CliffWait (L3173) | `src/Lanky/Lanky.asm` | 1 |
| 5285 | 0x14A5 | `LANKY_CLF_Q` | 3166 | Lanky CliffQuick (L3174) | `src/Lanky/Lanky.asm` | 1 |
| 5286 | 0x14A6 | `LANKY_CLF_CLM_Q1` | 3167 | Lanky CliffClimbQuick1 (L3175) | `src/Lanky/Lanky.asm` | 1 |
| 5287 | 0x14A7 | `LANKY_CLF_CLM_Q2` | 3168 | Lanky CliffClimbQuick2 (L3176) | `src/Lanky/Lanky.asm` | 1 |
| 5288 | 0x14A8 | `LANKY_CLF_S` | 3169 | Lanky CliffSlow (L3177) | `src/Lanky/Lanky.asm` | 1 |
| 5289 | 0x14A9 | `LANKY_CLF_CLM_S1` | 3170 | Lanky CliffClimbSlow1 (L3178) | `src/Lanky/Lanky.asm` | 1 |
| 5290 | 0x14AA | `LANKY_CLF_CLM_S2` | 3171 | Lanky CliffClimbSlow2 (L3179) | `src/Lanky/Lanky.asm` | 1 |
| 5291 | 0x14AB | `LANKY_CLF_ATK_Q1` | 3172 | Lanky CliffAttackQuick1 (L3180) | `src/Lanky/Lanky.asm` | 1 |
| 5292 | 0x14AC | `LANKY_CLF_ATK_Q2` | 3173 | Lanky CliffAttackQuick2 (L3181) | `src/Lanky/Lanky.asm` | 1 |
| 5293 | 0x14AD | `LANKY_CLF_ATK_S1` | 3174 | Lanky CliffAttackSlow1 (L3182) | `src/Lanky/Lanky.asm` | 1 |
| 5294 | 0x14AE | `LANKY_CLF_ATK_S2` | 3175 | Lanky CliffAttackSlow2 (L3183) | `src/Lanky/Lanky.asm` | 1 |
| 5295 | 0x14AF | `LANKY_CLF_ESC_Q1` | 3176 | Lanky CliffEscapeQuick1 (L3184) | `src/Lanky/Lanky.asm` | 1 |
| 5296 | 0x14B0 | `LANKY_CLF_ESC_Q2` | 3177 | Lanky CliffEscapeQuick2 (L3185) | `src/Lanky/Lanky.asm` | 1 |
| 5297 | 0x14B1 | `LANKY_CLF_ESC_S1` | 3178 | Lanky CliffEscapeSlow1 (L3186) | `src/Lanky/Lanky.asm` | 1 |
| 5298 | 0x14B2 | `LANKY_CLF_ESC_S2` | 3179 | Lanky CliffEscapeSlow2 (L3187) | `src/Lanky/Lanky.asm` | 1 |
| 5299 | 0x14B3 | `LANKY_L_ITM_PICKUP` | 3180 | Lanky LightItemPickup (L3188) | `src/Lanky/Lanky.asm` | 1 |
| 5300 | 0x14B4 | `LANKY_ITM_THROW_F` | 3181 | Lanky ItemThrowF (L3189) | `src/Lanky/Lanky.asm` | 4 |
| 5301 | 0x14B5 | `LANKY_ITM_THROW_U` | 3182 | Lanky ItemThrowU (L3190) | `src/Lanky/Lanky.asm` | 2 |
| 5302 | 0x14B6 | `LANKY_ITM_THROW_D` | 3183 | Lanky ItemThrowD (L3191) | `src/Lanky/Lanky.asm` | 2 |
| 5303 | 0x14B7 | `LANKY_ITM_THROW_DASH` | 3184 | Lanky ItemThrowDash (L3192) | `src/Lanky/Lanky.asm` | 1 |
| 5304 | 0x14B8 | `LANKY_ITM_DROP` | 3185 | Lanky ItemDrop (L3193) | `src/Lanky/Lanky.asm` | 1 |
| 5305 | 0x14B9 | `LANKY_ITM_THROW_AIR_F` | 3186 | Lanky ItemThrowAirF (L3194) | `src/Lanky/Lanky.asm` | 4 |
| 5306 | 0x14BA | `LANKY_ITM_THROW_AIR_U` | 3187 | Lanky ItemThrowAirU (L3195) | `src/Lanky/Lanky.asm` | 2 |
| 5307 | 0x14BB | `LANKY_ITM_THROW_AIR_D` | 3188 | Lanky ItemThrowAirD (L3196) | `src/Lanky/Lanky.asm` | 2 |
| 5308 | 0x14BC | `LANKY_H_ITM_PICKUP` | 3189 | Lanky HeavyItemPickup (L3197) | `src/Lanky/Lanky.asm` | 1 |
| 5309 | 0x14BD | `LANKY_H_ITM_THROW` | 3190 | Lanky HeavyItemThrow (L3198) | `src/Lanky/Lanky.asm` | 4 |
| 5310 | 0x14BE | `LANKY_ITM_JAB` | 3191 | Lanky ItemJab (L3199) | `src/Lanky/Lanky.asm` | 4 |
| 5311 | 0x14BF | `LANKY_ITM_TILT` | 3192 | Lanky ItemTilt (L3200) | `src/Lanky/Lanky.asm` | 4 |
| 5312 | 0x14C0 | `LANKY_ITM_SMASH` | 3193 | Lanky ItemSmash (L3201) | `src/Lanky/Lanky.asm` | 4 |
| 5313 | 0x14C1 | `LANKY_ITM_DASH` | 3194 | Lanky ItemDash (L3202) | `src/Lanky/Lanky.asm` | 4 |
| 5314 | 0x14C2 | `LANKY_HAMMER_IDLE` | 3195 | Lanky HammerIdle (L3203) | `src/Lanky/Lanky.asm` | 1 |
| 5315 | 0x14C3 | `LANKY_HAMMER_WALK` | 3196 | Lanky HammerWalk (L3204) | `src/Lanky/Lanky.asm` | 5 |
| 5316 | 0x14C4 | `LANKY_ITM_SHOOT` | 3197 | Lanky ItemShoot (L3205) | `src/Lanky/Lanky.asm` | 2 |
| 5317 | 0x14C5 | `LANKY_ITM_SHOOT_AIR` | 3198 | Lanky ItemShootAir (L3206) | `src/Lanky/Lanky.asm` | 2 |
| 5318 | 0x14C6 | `LANKY_JAB1` | 3199 | Lanky Jab1 (L3207) | `src/Lanky/Lanky.asm` | 1 |
| 5319 | 0x14C7 | `LANKY_JAB2` | 3200 | Lanky Jab2 (L3208) | `src/Lanky/Lanky.asm` | 1 |
| 5320 | 0x14C8 | `LANKY_JAB3` | 3201 | Lanky Jab3 (L3209) | `src/Lanky/Lanky.asm` | 1 |
| 5321 | 0x14C9 | `LANKY_DASH_ATK` | 3202 | Lanky DashAttack (L3210) | `src/Lanky/Lanky.asm` | 1 |
| 5322 | 0x14CA | `LANKY_F_TILT_HIGH` | 3203 | Lanky FTiltHigh (L3211) | `src/Lanky/Lanky.asm` | 1 |
| 5323 | 0x14CB | `LANKY_F_TILT` | 3204 | Lanky FTilt (L3212) | `src/Lanky/Lanky.asm` | 1 |
| 5324 | 0x14CC | `LANKY_F_TILT_LOW` | 3205 | Lanky FTiltLow (L3213) | `src/Lanky/Lanky.asm` | 1 |
| 5325 | 0x14CD | `LANKY_U_TILT` | 3206 | Lanky UTilt (L3214) | `src/Lanky/Lanky.asm` | 1 |
| 5326 | 0x14CE | `LANKY_D_TILT` | 3207 | Lanky DTilt (L3215) | `src/Lanky/Lanky.asm` | 1 |
| 5327 | 0x14CF | `LANKY_F_SMASH_HIGH` | 3208 | Lanky FSmashHigh (L3216) | `src/Lanky/Lanky.asm` | 1 |
| 5328 | 0x14D0 | `LANKY_F_SMASH_MID_HIGH` | 3209 | Lanky FSmashMidHigh (L3217) |  | 0 |
| 5329 | 0x14D1 | `LANKY_F_SMASH` | 3210 | Lanky FSmash (L3218) | `src/Lanky/Lanky.asm` | 1 |
| 5330 | 0x14D2 | `LANKY_F_SMASH_MID_LOW` | 3211 | Lanky FSmashMidLow (L3219) |  | 0 |
| 5331 | 0x14D3 | `LANKY_F_SMASH_LOW` | 3212 | Lanky FSmashLow (L3220) | `src/Lanky/Lanky.asm` | 1 |
| 5332 | 0x14D4 | `LANKY_U_SMASH` | 3213 | Lanky USmash (L3221) | `src/Lanky/Lanky.asm` | 1 |
| 5333 | 0x14D5 | `LANKY_D_SMASH` | 3214 | Lanky DSmash (L3222) | `src/Lanky/Lanky.asm` | 1 |
| 5334 | 0x14D6 | `LANKY_ATTACK_AIR_N` | 3215 | Lanky AttackAirN (L3223) | `src/Lanky/Lanky.asm` | 1 |
| 5335 | 0x14D7 | `LANKY_ATTACK_AIR_F` | 3216 | Lanky AttackAirF (L3224) | `src/Lanky/Lanky.asm` | 1 |
| 5336 | 0x14D8 | `LANKY_ATTACK_AIR_B` | 3217 | Lanky AttackAirB (L3225) | `src/Lanky/Lanky.asm` | 1 |
| 5337 | 0x14D9 | `LANKY_ATTACK_AIR_U` | 3218 | Lanky AttackAirU (L3226) | `src/Lanky/Lanky.asm` | 1 |
| 5338 | 0x14DA | `LANKY_ATTACK_AIR_D` | 3219 | Lanky AttackAirD (L3227) | `src/Lanky/Lanky.asm` | 1 |
| 5339 | 0x14DB | `LANKY_LANDING_AIR_F` | 3220 | Lanky LandingAirF (L3228) | `src/Lanky/Lanky.asm` | 1 |
| 5340 | 0x14DC | `LANKY_LANDING_AIR_B` | 3221 | Lanky LandingAirB (L3229) | `src/Lanky/Lanky.asm` | 1 |
| 5341 | 0x14DD | `LANKY_LANDING_AIR_D` | 3222 | Lanky LandingAirD (L3230) | `src/Lanky/Lanky.asm` | 1 |
| 5342 | 0x14DE | `LANKY_ENTER_PIPE` | 3223 | Lanky EnterPipe (L3231) | `src/Lanky/Lanky.asm` | 1 |
| 5343 | 0x14DF | `LANKY_EXIT_PIPE` | 3224 | Lanky ExitPipe (L3232) | `src/Lanky/Lanky.asm` | 1 |
| 5344 | 0x14E0 | `LANKY_EXIT_PIPE_WALK` | 3225 | Lanky ExitPipeWalk (L3233) | `src/Lanky/Lanky.asm` | 1 |
| 5345 | 0x14E1 | `LANKY_APPEAR_L` | 3226 | Lanky AppearL (L3234) | `src/Lanky/Lanky.asm` | 1 |
| 5346 | 0x14E2 | `LANKY_APPEAR_R` | 3227 | Lanky AppearR (L3235) | `src/Lanky/Lanky.asm` | 1 |
| 5347 | 0x14E3 | `LANKY_VICTORY_1` | 3228 | Lanky Victory 1 (L3236) | `src/Lanky/Lanky.asm` | 2 |
| 5348 | 0x14E4 | `LANKY_VICTORY_2` | 3229 | Lanky Victory 2 (L3237) | `src/Lanky/Lanky.asm` | 1 |
| 5349 | 0x14E5 | `LANKY_VICTORY_3` | 3230 | Lanky Victory 3 (L3238) | `src/Lanky/Lanky.asm` | 1 |
| 5350 | 0x14E6 | `LANKY_CLAP` | 3231 | Lanky Clap (L3239) | `src/Lanky/Lanky.asm` | 1 |
| 5351 | 0x14E7 | `LANKY_CONTINUE_FALL` | 3232 | Lanky Continue Fall (L3240) | `src/Lanky/Lanky.asm` | 1 |
| 5352 | 0x14E8 | `LANKY_CONTINUE_UP` | 3233 | Lanky Continue Up (L3241) | `src/Lanky/Lanky.asm` | 1 |
| 5353 | 0x14E9 | `LANKY_1P_POSE` | 3234 | Lanky 1P Pose (L3242) | `src/Lanky/Lanky.asm` | 1 |
| 5354 | 0x14EA | `LANKY_DUO_POSE` | 3235 | Lanky Duo Pose (L3243) | `src/Lanky/Lanky.asm`, `src/SinglePlayerModes.asm` | 2 |
| 5355 | 0x14EB | `LANKY_NSPG` | 3236 | Lanky NSPG (L3244) | `src/Lanky/Lanky.asm` | 1 |
| 5356 | 0x14EC | `LANKY_NSPA` | 3237 | Lanky NSPA (L3245) | `src/Lanky/Lanky.asm` | 1 |
| 5357 | 0x14ED | `LANKY_USPG` | 3238 | Lanky USPGBegin (L3246) | `src/Lanky/Lanky.asm` | 1 |
| 5358 | 0x14EE | `LANKY_USPA` | 3239 | Lanky USPABegin (L3247) | `src/Lanky/Lanky.asm` | 1 |
| 5359 | 0x14EF | `LANKY_DSPG` | 3240 | Lanky DSPG (L3248) | `src/Lanky/Lanky.asm` | 1 |
| 5360 | 0x14F0 | `LANKY_DSPA` | 3241 | Lanky DSPA (L3249) | `src/Lanky/Lanky.asm` | 1 |
| 5361 | 0x14F1 | `STG_PEACH_PLATFORMS_STAGE` | 3242 | Peach Board the Platforms Stage (L3250) |  | 0 |
| 5362 | 0x14F2 | `STG_PEACH_PLATFORMS_HEADER` | 3243 | Peach Board the Platforms Header (L3251) |  | 0 |
| 5363 | 0x14F3 | `KING_OF_THE_HILL_GFX` | 3244 | King of the Hill GFX (L3252) | `src/KingOfTheHill.asm` | 1 |
| 5364 | 0x14F4 | `GOEMON_DSPG_BREAK` | 3245 | Goemon DSPG Break (L3253) | `src/Goemon/Goemon.asm` | 1 |
| 5365 | 0x14F5 | `GOEMON_DSPA_BREAK` | 3246 | Goemon DSPA Break (L3254) | `src/Goemon/Goemon.asm` | 1 |
| 5366 | 0x14F6 | `GOEMON_NSPA_END` | 3247 | Goemon NSPA End (L3255) | `src/Goemon/Goemon.asm` | 1 |
| 5367 | 0x14F7 | `KIRBY_GOEMON_NSPA_END` | 3248 | Kirby Goemon NSPA End (L3256) |  | 0 |
| 5368 | 0x14F8 | `KIRBY_ROY_SWORD` | 3249 | Kirby Roy Sword (L3257) |  | 0 |
| 5369 | 0x14F9 | `LANKY_DSP_IDLE` | 3250 | Lanky DSP Idle (L3258) | `src/Lanky/Lanky.asm` | 1 |
| 5370 | 0x14FA | `LANKY_DSP_WALK` | 3251 | Lanky DSP Walk (L3259) | `src/Lanky/Lanky.asm` | 1 |
| 5371 | 0x14FB | `LANKY_DSP_TURN` | 3252 | Lanky DSP Turn (L3260) | `src/Lanky/Lanky.asm` | 1 |
| 5372 | 0x14FC | `LANKY_DSP_JUMPSQUAT` | 3253 | Lanky DSP Jumpsquat (L3261) | `src/Lanky/Lanky.asm` | 2 |
| 5373 | 0x14FD | `LANKY_DSP_JUMP` | 3254 | Lanky DSP Jump (L3262) | `src/Lanky/Lanky.asm` | 1 |
| 5374 | 0x14FE | `LANKY_DSP_FALL` | 3255 | Lanky DSP Fall (L3263) | `src/Lanky/Lanky.asm` | 1 |
| 5375 | 0x14FF | `LANKY_DSP_END` | 3256 | Lanky DSP End (L3264) | `src/Lanky/Lanky.asm` | 2 |
| 5376 | 0x1500 | `LANKY_DSP_END_AIR` | 3257 | Lanky DSP End Air (L3265) | `src/Lanky/Lanky.asm` | 2 |
| 5377 | 0x1501 | `LANKY_DSP_DROP` | 3258 | Lanky DSP Pass (L3266) | `src/Lanky/Lanky.asm` | 1 |
| 5378 | 0x1502 | `LANKY_DSP_TAUNT` | 3259 | Lanky DSP Taunt (L3267) | `src/Lanky/Lanky.asm` | 1 |
| 5379 | 0x1503 | `LANKY_USP_MOVE` | 3260 | Lanky USP Move (L3268) | `src/Lanky/Lanky.asm` | 1 |
| 5380 | 0x1504 | `LANKY_USP_TURN` | 3261 | Lanky USP Turn (L3269) | `src/Lanky/Lanky.asm` | 1 |
| 5381 | 0x1505 | `LANKY_USP_END` | 3262 | Lanky USP End (L3270) | `src/Lanky/Lanky.asm` | 1 |
| 5382 | 0x1506 | `LANKY_PROJECTILE_HITBOX` | 3263 | Lanky Projectile Hitbox (L3271) | `src/Character.asm`, `src/TagTeam.asm` | 2 |
| 5383 | 0x1507 | `LANKY_PROJECTILE_GRAPHIC` | 3264 | Lanky Projectile Graphic (L3272) | `src/Character.asm` | 1 |
| 5384 | 0x1508 | `NPEACH_MAIN` | 3265 | Polygon Peach Main (L3273) | `src/Character.asm` | 1 |
| 5385 | 0x1509 | `NPEACH_CHARACTER` | 3266 | Polygon Peach Character (L3274) | `src/Character.asm` | 1 |
| 5386 | 0x150A | `KIRBY_LANKY_GUN` | 3267 | Kirby Lanky Gun (L3275) |  | 0 |
| 5387 | 0x150B | `KIRBY_LANKY_NSPG` | 3268 | Kirby Lanky NSPG (L3276) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5388 | 0x150C | `KIRBY_LANKY_NSPA` | 3269 | Kirby Lanky NSPA (L3277) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5389 | 0x150D | `LANKY_USP_DAMAGE` | 3270 | Lanky USP Damage (L3278) | `src/Lanky/Lanky.asm` | 1 |
| 5390 | 0x150E | `NCRASH_MAIN` | 3271 | Polygon Crash Main (L3279) | `src/Character.asm` | 1 |
| 5391 | 0x150F | `NCRASH_CHARACTER` | 3272 | Polygon Crash Character (L3280) | `src/Character.asm` | 1 |
| 5392 | 0x1510 | `LANKY_ENTRY` | 3273 | Lanky Entry Object (L3281) | `src/Character.asm` | 1 |
| 5393 | 0x1511 | `TIME_TWISTER_HEADER` | 3274 | Time Twister Header (L3282) |  | 0 |
| 5394 | 0x1512 | `TIME_TWISTER_STAGE` | 3275 | Time Twister Stage (L3283) |  | 0 |
| 5395 | 0x1513 | `TIME_TWISTER_BG` | 3276 | Time Twister BG (L3284) |  | 0 |
| 5396 | 0x1514 | `NSANITY_BG` | 3277 | N. Sanity Beach BG (L3285) |  | 0 |
| 5397 | 0x1515 | `SNOWGO_BG` | 3278 | Snow Go BG (L3286) |  | 0 |
| 5398 | 0x1516 | `FUTUREF_BG` | 3279 | Future Frenzy BG (L3287) |  | 0 |
| 5399 | 0x1517 | `KIRBY_CRASH_SPIN_GFX` | 3280 | Kirby Crash Spin GFX (L3288) |  | 0 |
| 5400 | 0x1518 | `TIME_TWISTER_SSS` | 3281 | Time Twister SSS Header (L3289) |  | 0 |
| 5401 | 0x1519 | `NSANITY_HEADER` | 3282 | N. Sanity Beach Header (L3290) |  | 0 |
| 5402 | 0x151A | `NSANITY_STAGE` | 3283 | N. Sanity Beach Stage (L3291) |  | 0 |
| 5403 | 0x151B | `SNOWGO_HEADER` | 3284 | Snow Go Header (L3292) |  | 0 |
| 5404 | 0x151C | `SNOWGO_STAGE` | 3285 | Snow Go Stage (L3293) |  | 0 |
| 5405 | 0x151D | `FUTUREF_HEADER` | 3286 | Future Frenzy Header (L3294) |  | 0 |
| 5406 | 0x151E | `FUTUREF_STAGE` | 3287 | Future Frenzy Stage (L3295) |  | 0 |
| 5407 | 0x151F | `DRL_VICTORY_IMAGE_BOTTOM` | 3288 | Dr. Luigi Ending Image 1 (L3296) | `src/DrLuigi/DrLuigi.asm` | 1 |
| 5408 | 0x1520 | `DRL_VICTORY_IMAGE_TOP` | 3289 | Dr. Luigi Ending Image 2 (L3297) |  | 0 |
| 5409 | 0x1521 | `ROY_VICTORY_IMAGE_BOTTOM` | 3290 | Roy Ending Image 1 (L3298) | `src/Roy/Roy.asm` | 1 |
| 5410 | 0x1522 | `ROY_VICTORY_IMAGE_TOP` | 3291 | Roy Ending Image 2 (L3299) |  | 0 |
| 5411 | 0x1523 | `PIANO_VICTORY_IMAGE_BOTTOM` | 3292 | Mad Piano Ending Image 1 (L3300) | `src/SinglePlayer.asm` | 1 |
| 5412 | 0x1524 | `PIANO_VICTORY_IMAGE_TOP` | 3293 | Mad Piano Ending Image 2 (L3301) |  | 0 |
| 5413 | 0x1525 | `YOUNG_LINK_CSS` | 3294 | Young Link CSS (L3302) | `src/YoungLink/YoungLink.asm` | 1 |
| 5414 | 0x1526 | `KIRBY_PEACH_NSP_G` | 3295 | Kirby Peach NSP Ground (L3303) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5415 | 0x1527 | `KIRBY_PEACH_NSP_A` | 3296 | Kirby Peach NSP Air (L3304) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5416 | 0x1528 | `KIRBY_PEACH_NSP_RECOIL` | 3297 | Kirby Peach NSP Recoil (L3305) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5417 | 0x1529 | `SONIC_DOWNBOUNCED` | 3298 | Sonic Down Bounce Up (L3306) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 9 |
| 5418 | 0x152A | `SONIC_DOWNBOUNCEU` | 3299 | Sonic Down Bounce Down (L3307) | `src/NSonic/NSonic.asm`, `src/SSonic/SSonic.asm`, `src/Sonic/Sonic.asm` | 6 |
| 5419 | 0x152B | `SSONIC_USPG_BEGIN` | 3300 | Super Sonic USP Ground Begn (L3308) | `src/SSonic/SSonic.asm` | 1 |
| 5420 | 0x152C | `SSONIC_USPG_END` | 3301 | Super Sonic USP Ground End (L3309) | `src/SSonic/SSonic.asm` | 1 |
| 5421 | 0x152D | `SSONIC_USPA_BEGIN` | 3302 | Super Sonic USP Air Begin (L3310) | `src/SSonic/SSonic.asm` | 1 |
| 5422 | 0x152E | `SSONIC_USPA_END` | 3303 | Super Sonic USP Air End (L3311) | `src/SSonic/SSonic.asm` | 1 |
| 5423 | 0x152F | `LANKY_VICTORY_IMAGE_BOTTOM` | 3304 | Lanky Kong Ending Image 1 (L3312) | `src/Lanky/Lanky.asm` | 1 |
| 5424 | 0x1530 | `LANKY_VICTORY_IMAGE_TOP` | 3305 | Lanky Kong Ending Image 2 (L3313) |  | 0 |
| 5425 | 0x1531 | `SSONIC_NSP_BEGIN` | 3306 | Super Sonic NSP Begin (L3314) | `src/SSonic/SSonic.asm` | 1 |
| 5426 | 0x1532 | `SSONIC_NSP_MOVE` | 3307 | Super Sonic NSP Move (L3315) | `src/SSonic/SSonic.asm` | 2 |
| 5427 | 0x1533 | `KIRBY_CUSTOM_HATS_2` | 3308 | Kirby Custom Hats 2 (L3316) | `src/TagTeam.asm` | 2 |
| 5428 | 0x1534 | `SSONIC_EMERALDS` | 3309 | Super Sonic Emeralds (L3317) | `src/Character.asm` | 1 |
| 5429 | 0x1535 | `SSONIC_DSP` | 3310 | Super Sonic DSP (L3318) | `src/SSonic/SSonic.asm` | 1 |
| 5430 | 0x1536 | `SSONIC_DSPA` | 3311 | Super Sonic DSPA (L3319) | `src/SSonic/SSonic.asm` | 1 |
| 5431 | 0x1537 | `MARINA_HEAVY_ITM_PICKUP` | 3312 | Marina HeavyItemPickup (L3320) | `src/Marina/Marina.asm`, `src/NMarina/NMarina.asm` | 2 |
| 5432 | 0x1538 | `DEDEDE_JUMP_AERIAL_3` | 3313 | Dedede Jump Aerial 3 (L3321) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 2 |
| 5433 | 0x1539 | `DEDEDE_JUMP_AERIAL_4` | 3314 | Dedede Jump Aerial 4 (L3322) | `src/Dedede/Dedede.asm`, `src/NDedede/NDedede.asm` | 4 |
| 5434 | 0x153A | `KIRBY_SSONIC_NSP_BEGIN` | 3315 | Kirby Super Sonic NSP Begin (L3323) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 2 |
| 5435 | 0x153B | `KIRBY_SSONIC_NSP_MOVE` | 3316 | Kirby Super Sonic NSP Move (L3324) | `src/JKirby/JKirby.asm`, `src/Kirby/Kirby.asm` | 4 |
| 5436 | 0x153C | `DKING_VICTORY_IMAGE_BOTTOM` | 3317 | Dragon King Ending Image 1 (L3325) | `src/DragonKing/DragonKing.asm` | 1 |
| 5437 | 0x153D | `DKING_VICTORY_IMAGE_TOP` | 3318 | Dragon King Ending Image 2 (L3326) |  | 0 |
| 5438 | 0x153E | `STAGE_ICONS_2` | 3319 | Stage Icons 2 (L3327) |  | 0 |
| 5439 | 0x153F | `DSAMUS_ENTRY` | 3320 | Dark Samus Entry (L3328) | `src/Character.asm` | 1 |
| 5440 | 0x1540 | `DSAMUS_APPEAR_L` | 3321 | Dark Samus AppearL (L3329) | `src/DSamus/DSamus.asm` | 1 |
| 5441 | 0x1541 | `DSAMUS_APPEAR_R` | 3322 | Dark Samus AppearR (L3330) | `src/DSamus/DSamus.asm` | 1 |
| 5442 | 0x1542 | `AUTUMN_HTP_HEADER` | 3323 | Autumn How to Play Header (L3331) |  | 0 |
| 5443 | 0x1543 | `AUTUMN_HTP_STAGE` | 3324 | Autumn How to Play Stage (L3332) |  | 0 |
| 5444 | 0x1544 | `GBOWSER_GRAB` | 3325 | Giga Bowser Grab (L3333) | `src/GBowser/gbowser.asm` | 1 |
| 5445 | 0x1545 | `GBOWSER_USMASH` | 3326 | Giga Bowser USmash (L3334) | `src/GBowser/gbowser.asm` | 1 |
| 5446 | 0x1546 | `GBOWSER_DSMASH` | 3327 | Giga Bowser DSmash (L3335) | `src/GBowser/gbowser.asm` | 1 |
| 5447 | 0x1547 | `GBOWSER_ATTACK_AIR_N` | 3328 | Giga Bowser AttackAirN (L3336) | `src/GBowser/gbowser.asm` | 1 |
| 5448 | 0x1548 | `GBOWSER_ATTACK_AIR_U` | 3329 | Giga Bowser AttackAirU (L3337) | `src/GBowser/gbowser.asm` | 1 |
| 5449 | 0x1549 | `GBOWSER_ATTACK_AIR_D` | 3330 | Giga Bowser AttackAirD (L3338) | `src/GBowser/gbowser.asm` | 1 |
| 5450 | 0x154A | `GBOWSER_UTILT` | 3331 | Giga Bowser UTilt (L3339) | `src/GBowser/gbowser.asm` | 1 |
| 5451 | 0x154B | `GBOWSER_FTILT_HIGH` | 3332 | Giga Bowser FTiltHigh (L3340) | `src/GBowser/gbowser.asm` | 1 |
| 5452 | 0x154C | `GBOWSER_FTILT` | 3333 | Giga Bowser FTilt (L3341) | `src/GBowser/gbowser.asm` | 1 |
| 5453 | 0x154D | `GBOWSER_FTILT_LOW` | 3334 | Giga Bowser FTiltLow (L3342) | `src/GBowser/gbowser.asm` | 1 |
| 5454 | 0x154E | `GBOWSER_TURN` | 3335 | Giga Bowser Turn (L3343) | `src/GBowser/gbowser.asm` | 1 |
| 4086 | 0x0FF6 | (no File.asm constant) | - | Marina Board the Platforms Header (L1975) | - | 0 |
| 4087 | 0x0FF7 | (no File.asm constant) | - | Marina Board the Platforms Stage (L1976) | - | 0 |

---

## Task B: loader and memory map

### B.1 ROM layout (`main.asm`)

| ROM range | Source | Content |
|---|---|---|
| 0x000000 - end of vanilla | `main.asm:9-10` `insert "roms/original.z64"` | Vanilla SSB64 image (xdelta-patched). Size is not stated in source (SSB64 US is 32 MiB; not verified here). |
| 0x000020 - 0x000033 | `main.asm:13-15` | ROM name `SMASH REMIX` written over the internal name. |
| 0x02C00000 - | `main.asm:18` `origin 0x02C00000`, `base 0x80400000` | Remix code and data image, assembled at RAM 0x80400000. Lies past a 32 MiB vanilla image. |
| 0x03000000 | `src/midi.asm:15` `MIDI_BANK` | Additional MIDI bank, appended with `MIDI_BANK_END`. |
| `MIDI_BANK_END` onward | `src/FGM.asm` `write_sounds` (origins `*_extended_origin`) | Extended FGM parameter maps, bank table, predictors, raw samples, microcode. Entries also patched inside vanilla ROM tables. |
| 0x03E7FFFF | `main.asm:337-338` `origin 0x3E7FFFF`, `db 0x00` | Pads the image to 0x03E80000 bytes (62 MiB). The comment on line 336 says "rom size = 64MB". The two disagree; the byte count is 0x03E80000 = 65,011,712. |

### B.2 RAM layout (`main.asm` lines 325-334)

Code and data are assembled at RAM base 0x80400000 in this order (after all `include`s):

1. All `src/*.asm` code and data, from RAM 0x80400000 upward (size not computed here).
2. `midi_memory_block`: `fill MIDI.largest_midi`, 16-byte aligned. Fixed block for music, replacing vanilla MIDI allocation (`src/midi.asm:708`).
3. `file_table`: `fill 0x620` bytes, 16-byte aligned. RAM copy of the loaded-file list (see B.4).
4. `custom_heap_address`: one word, initialized to `custom_heap`.
5. `custom_heap`: start of the dynamic heap. Everything from here up is free space.

Expansion RAM: `readme.md:14` requires the 8 MB expansion pak. The heap ceiling used in `src/TagTeam.asm:831` is 0x80800000
(`lui t1, 0x8080`), which matches an 8 MB pak mapped at 0x80000000-0x807FFFFF.

### B.3 Boot DMA (`src/Boot.asm`, scope `load_`)

1. `OS.patch_start(0x00001234, 0x80000634)` hooks the vanilla boot code: `j 0x80000438`.
2. `OS.patch_start(0x00001038, 0x80000438)` replaces the vanilla copy at 0x80000438:
   - `jal Global.dma_copy_` (vanilla DMA, a2=0x0100), original line kept.
   - `lui a0, 0x02C0` (ROM 0x02C00000), `lui a1, 0x8040` (RAM 0x80400000), `jal Global.dma_copy_` with
     `a2 = ((custom_heap - 0x80400000) >> 16) + 1) << 16`, i.e. the Remix image size rounded up to 64 KB.
   - `j load_` jumps into the freshly copied image.
3. `load_` (the start of the Remix image code after the patch blocks) runs `SRAM.check_saved_`, then `Toggles.load_` or
   `SRAM.initialize_`, then returns to vanilla at 0x80000638.

The source comment says the transfer is 0x400000 bytes. The real size is computed from `custom_heap`, so it depends on the
assembled image size. Not verified here.

### B.4 Runtime file loading

- `Render.load_file(id, ptr)` (macro, `src/Render.asm:549`) sets `a0 = id`, `a1 = ptr` and calls `Render.load_file_` (Render.asm:921).
- `Render.load_file_` writes the id to `temp_file_ID`, calls vanilla `0x800CDEEC` (file length), allocates with `0x80004980`
  (malloc, 16-byte aligned), then calls vanilla `0x800CDE04` (load file into that buffer). Vanilla routines read the reloc
  table in ROM. The Remix table (5455 entries, at ROM 0x1AC870 per the task brief) is not in this repo.
- Loaded-file list: `Global.files_loaded` = `0x800D6300` (`src/Global.asm:210`). Format (`Global.asm:206-209`): word file id,
  word RAM address. `Render.asm:2228` and `SinglePlayer.asm:1804` read it.
- File manager struct: `0x800D62E0` (used at `TagTeam.asm:1201`). `CharacterSelect.asm:1696-1699` writes the length
  `0x00000300` to struct offset 0x1C and `file_table` to offset 0x20, so the vanilla file manager uses the RAM `file_table`
  as its list.
  - Possible mismatch to verify: `file_table` is reserved as `fill 0x620` bytes (main.asm:331), but the length written is 0x300.
    If the length is in entries of 8 bytes, that is 0x1800 bytes, which does not fit 0x620. The unit of this length is not
    confirmed here.
- `custom_heap_address` (RAM word) is the bump pointer for dynamic allocation. It is reset to `custom_heap` at:
  `CharacterSelect.asm:2018-2020`, `GameEnd.asm:144-146`, and `Render.asm:2457-2459` (mode select). `TagTeam.asm:806-833` moves it to expansion RAM when the default heap has been moved.

### B.5 `.include` order (`main.asm`)

`main.asm` includes 225 source files (plus `assembler/N64.inc` at line 6). They are listed in order below. The purpose is the
file's header comment when one exists. Otherwise it is marked "inferred from name" (a guess from the file name, to be checked).
`include` paths are relative to the repo root (`src/...`), and several sub-includes (e.g. `src/Boot.asm` -> `OS.asm`, `Global.asm`,
`Toggles.asm`, `SRAM.asm`) are guarded by `if !{defined __...__}`.

| # | main.asm line | File | Purpose | Source of purpose |
|---:|---:|---|---|---|
| 1 | 20 | `src/OS.asm` | Core OS macros: patch_start/patch_end, align, TRUE/NULL, print helpers | inferred from name |
| 2 | 21 | `src/String.asm` | String insertion macro (String.insert) for in-ROM text | inferred from name |
| 3 | 22 | `src/Render.asm` | Render helpers: load_file (reloc file loader), textures, strings, display objects, heap | inferred from name |
| 4 | 23 | `src/Action.asm` | This file contains constants for known action IDs | file header |
| 5 | 24 | `src/File.asm` | This file contains constants for SmashRemix file IDs. | file header |
| 6 | 25 | `src/Boot.asm` | Boot patches (N64 logo, splash/victory images) and the DMA loader that copies the Remix image to RAM | inferred from name |
| 7 | 26 | `src/Settings.asm` | Persistent settings state (inferred from name) | inferred from name |
| 8 | 27 | `src/Moveset.asm` | Moveset.asm (Fray) | file header |
| 9 | 28 | `src/Command.asm` | Command.asm (Fray) | file header |
| 10 | 29 | `src/Timeouts.asm` | Timeouts.asm (Cyjorg) (additional timeouts found by bit) | file header |
| 11 | 30 | `src/GFX.asm` | GFX patches and helpers (inferred from name) | inferred from name |
| 12 | 31 | `src/BGM.asm` | Background music selection and volume | inferred from name |
| 13 | 32 | `src/Camera.asm` | Camera patches (inferred from name) | inferred from name |
| 14 | 33 | `src/Color.asm` | Color.asm (generated by color.c) | file header |
| 15 | 34 | `src/Combo.asm` | Combo.asm (by Fray) | file header |
| 16 | 35 | `src/Crash.asm` | Crash Bandicoot shared hooks (inferred from name; Crash/ dir holds the fighter) | inferred from name |
| 17 | 36 | `src/Credits.asm` | Credits sequence, incl. load_files_ (inferred from name) | inferred from name |
| 18 | 37 | `src/FD.asm` | Final Destination related patches (inferred from name) | inferred from name |
| 19 | 38 | `src/FGM.asm` | Sound effects: FGM tables, new samples/banks, ROM writes (write_sounds) | inferred from name |
| 20 | 39 | `src/GameEnd.asm` | Match end flow; resets custom_heap_address | inferred from name |
| 21 | 40 | `src/Global.asm` | Vanilla global RAM/ROM symbols (dma_copy_, screen IDs, match_info, files_loaded) | inferred from name |
| 22 | 41 | `src/Handicap.asm` | Handicap feature (inferred from name) | inferred from name |
| 23 | 42 | `src/Hazards.asm` | Hazards.asm [bit] | file header |
| 24 | 43 | `src/Projectile.asm` | Projectile support (inferred from name) | inferred from name |
| 25 | 44 | `src/Hitbox.asm` | Hitbox.asm (by Fray) | file header |
| 26 | 45 | `src/Item.asm` | Item handling and custom items (inferred from name) | inferred from name |
| 27 | 46 | `src/Joypad.asm` | Controller input (inferred from name) | inferred from name |
| 28 | 47 | `src/Menu.asm` | Menu code (inferred from name) | inferred from name |
| 29 | 48 | `src/Pause.asm` | Pause.asm (original by Fray) | file header |
| 30 | 49 | `src/Practice.asm` | Practice.asm (by Fray and bit) | file header |
| 31 | 50 | `src/Spawn.asm` | Spawn logic (inferred from name) | inferred from name |
| 32 | 51 | `src/Stages.asm` | Stage tables, stage_file_table and stage loading | inferred from name |
| 33 | 52 | `src/Toggles.asm` | Toggle options, Toggles.load_ (SRAM-backed), toggle menus | inferred from name |
| 34 | 53 | `src/Cheats.asm` | Cheat codes (inferred from name) | inferred from name |
| 35 | 54 | `src/TimedStock.asm` | TimedStock.asm (original by Danny_SsB and Mada0) | file header |
| 36 | 55 | `src/Shield.asm` | Shield visuals/logic (inferred from name) | inferred from name |
| 37 | 56 | `src/Training.asm` | Training.asm (functions by Fray, menu implementation by Cyjorg) | file header |
| 38 | 57 | `src/ComboMeter.asm` | Combo meter HUD (inferred from name) | inferred from name |
| 39 | 58 | `src/VsStats.asm` | Versus stats screen (inferred from name) | inferred from name |
| 40 | 59 | `src/Widescreen.asm` | Widescreen patches (inferred from name) | inferred from name |
| 41 | 60 | `src/AA.asm` | Anti-aliasing toggle (inferred from name) | inferred from name |
| 42 | 61 | `src/Japan.asm` | for toggles related to Japanese features | file header |
| 43 | 62 | `src/FPS.asm` | FPS display/limit (inferred from name) | inferred from name |
| 44 | 63 | `src/SinglePlayer.asm` | Single-player (1P) mode, incl. custom victory image table | inferred from name |
| 45 | 64 | `src/Skeleton.asm` | Skeleton/template code (inferred from name) | inferred from name |
| 46 | 65 | `src/Surface.asm` | Surface/ground handling (inferred from name) | inferred from name |
| 47 | 66 | `src/SinglePlayerModes.asm` | Single-player game modes (Remix 1P modes, file loading routines) | inferred from name |
| 48 | 67 | `src/SinglePlayerMenus.asm` | Single-player menus | inferred from name |
| 49 | 68 | `src/HRC.asm` | HRC Remix mode (inferred from Global.screen REMIX_MODES comment) | inferred from name |
| 50 | 69 | `src/Bonus.asm` | Bonus stage CSS/loading (inferred from Global.screen BONUS_* names) | inferred from name |
| 51 | 70 | `src/VsRemixMenu.asm` | Versus Remix options menu | inferred from name |
| 52 | 71 | `src/TwelveCharBattle.asm` | 12-character battle mode, loads its own file pointers | inferred from name |
| 53 | 72 | `src/TagTeam.asm` | Tag team mode; heap/file-table adjustments | inferred from name |
| 54 | 73 | `src/KingOfTheHill.asm` | King of the Hill mode | inferred from name |
| 55 | 74 | `src/Smashketball.asm` | Smashketball mode | inferred from name |
| 56 | 75 | `src/TugOfWar.asm` | Tug of War mode | inferred from name |
| 57 | 76 | `src/Size.asm` | Character size changes (inferred from name) | inferred from name |
| 58 | 77 | `src/CharEnvColor.asm` | Character environment color (inferred from name) | inferred from name |
| 59 | 78 | `src/SwordTrail.asm` | Sword trail effect | inferred from name |
| 60 | 79 | `src/GFXRoutine.asm` | GFX routines and port_override GFX table | inferred from name |
| 61 | 80 | `src/Damage.asm` | Damage system (inferred from name) | inferred from name |
| 62 | 81 | `src/Knockback.asm` | This file contains a randomizer for knockback angles. | file header |
| 63 | 82 | `src/InputDelay.asm` | Input delay option (inferred from name) | inferred from name |
| 64 | 83 | `src/InputDisplay.asm` | On-screen input display; loads button images | inferred from name |
| 65 | 84 | `src/StockMode.asm` | Stock mode (inferred from name) | inferred from name |
| 66 | 85 | `src/Stereo.asm` | Stereo audio option (inferred from name) | inferred from name |
| 67 | 86 | `src/Stamina.asm` | Stamina mode (inferred from name) | inferred from name |
| 68 | 87 | `src/Practice_1P.asm` | Practice_1P.asm (code by goombapatrol) | file header |
| 69 | 88 | `src/SinglePlayerEnemy.asm` | 1P enemy behavior (inferred from name) | inferred from name |
| 70 | 89 | `src/PlayerTag.asm` | Player tag/indicator (inferred from name) | inferred from name |
| 71 | 90 | `src/Reflect.asm` | This file allows us to use custom reflect routines for the reflecting player. Usually related to an action change or fgm/gfx | file header |
| 72 | 91 | `src/Transitions.asm` | Screen transitions; pointers moved to expansion RAM | inferred from name |
| 73 | 92 | `src/ZCancel.asm` | Z-cancel mechanic | inferred from name |
| 74 | 93 | `src/Tripping.asm` | Tripping.asm (code by goombapatrol) | file header |
| 75 | 94 | `src/PokemonAnnouncer.asm` | PokemonAnnouncer.asm [bit] | file header |
| 76 | 95 | `src/FootStool.asm` | Adds the Brawl mechanic allowing players to footstool jump off eachother | file header |
| 77 | 96 | `src/AirDodge.asm` | Subroutines used by AirDodge. | file header |
| 78 | 97 | `src/JabLock.asm` | Optional toggle for a "Jab Lock" mechanic. | file header |
| 79 | 98 | `src/LedgeJump.asm` | Allows players to jump while hanging from ledge | file header |
| 80 | 99 | `src/PerfectShield.asm` | Adds the Perfect Shield Melee mechanic. | file header |
| 81 | 100 | `src/SpotDodge.asm` | Adds spot dodge mechanic to smash remix | file header |
| 82 | 101 | `src/Parry.asm` | Parry mechanic | inferred from name |
| 83 | 102 | `src/Rage.asm` | Rage mechanic (inferred from name) | inferred from name |
| 84 | 103 | `src/SpecialZoom.asm` | Special move zoom camera | inferred from name |
| 85 | 104 | `src/AerialAttackFastFall.asm` | Adds the ability to fast fall while performing a normal aerial attack. A mechanic intrduced in smash melee | file header |
| 86 | 105 | `src/LedgeTrump.asm` | Removes the ability to ledge hog other players, the trumped player will fall off the ledge | file header |
| 87 | 106 | `src/Hitstun.asm` | Hitstun mechanic (inferred from name) | inferred from name |
| 88 | 107 | `src/DI.asm` | Directional influence (DI) | inferred from name |
| 89 | 108 | `src/WallTeching.asm` | Wall teching mechanic | inferred from name |
| 90 | 109 | `src/VsDemo.asm` | Versus demo mode (inferred from name) | inferred from name |
| 91 | 110 | `src/Teams.asm` | Teams mode (inferred from name) | inferred from name |
| 92 | 111 | `src/ChargeSmashAttacks.asm` | Coded by HaloFactory | file header |
| 93 | 112 | `src/Gallery.asm` | Gallery screen (inferred from name) | inferred from name |
| 94 | 113 | `src/Poison.asm` | Apply poison effects | file header |
| 95 | 114 | `src/DragonKingHUD.asm` | Dragon King HUD | inferred from name |
| 96 | 115 | `src/Accessibility.asm` | Accessibility.asm (code by goombapatrol) | file header |
| 97 | 116 | `src/BlastZone.asm` | Blast zone handling | inferred from name |
| 98 | 117 | `src/MagnifyingGlass.asm` | Magnifying glass feature (inferred from name) | inferred from name |
| 99 | 118 | `src/DKMode.asm` | Coded by HaloFactory | file header |
| 100 | 119 | `src/Walljump.asm` | Walljump.asm (code by Fray, modified by goom) | file header |
| 101 | 120 | `src/Speed.asm` | Speed.asm (code by goombapatrol) | file header |
| 102 | 121 | `src/OnHit.asm` | On-hit hooks (inferred from name) | inferred from name |
| 103 | 122 | `src/SingleButtonMode.asm` | SingleButtonMode.asm (code by goombapatrol) | file header |
| 104 | 123 | `src/SFXReplace.asm` | SFX replacement (inferred from name) | inferred from name |
| 105 | 124 | `src/StaleMoves.asm` | StaleMoves.asm (code by goombapatrol) | file header |
| 106 | 127 | `src/Character.asm` | Character table: define_character macro; File.* ids in each character struct | inferred from name |
| 107 | 128 | `src/AI.asm` | AI.asm (by bit) | file header |
| 108 | 129 | `src/VanillaLv10Attacks.asm` | Vanilla level-10 attacks (inferred from name) | inferred from name |
| 109 | 130 | `src/CharacterSelect.asm` | Character select screen; repoints file_table/heap | inferred from name |
| 110 | 131 | `src/CharacterSelectDebugMenu.asm` | Character select debug menu; reads file_table entries | inferred from name |
| 111 | 132 | `src/Costumes.asm` | Costume palettes (inferred from name) | inferred from name |
| 112 | 133 | `src/Fireball.asm` | This file allows for copies of Mario's fireball projectile to be generated. | file header |
| 113 | 134 | `src/ResultsScreen.asm` | thanks to tehzz for providing documentation | file header |
| 114 | 135 | `src/CharacterDataScreen.asm` | Coded by MarioReincarnate | file header |
| 115 | 136 | `src/linkshared.asm` | This file contains shared functions by Link Clones. | file header |
| 116 | 137 | `src/captainshared.asm` | This file contains shared functions by Captain Falcon Clones. | file header |
| 117 | 138 | `src/dkshared.asm` | This file contains shared functions by DK Clones. | file header |
| 118 | 139 | `src/nessshared.asm` | This file contains shared functions by Ness Clones. | file header |
| 119 | 140 | `src/jigglypuffkirbyshared.asm` | This file contains shared functions by Jigglypuff and Kirby Clones. | file header |
| 120 | 141 | `src/yoshishared.asm` | This file contains shared functions by Yoshi Clones. | file header |
| 121 | 142 | `src/pikashared.asm` | This file contains shared functions by Pika Clones. | file header |
| 122 | 143 | `src/samusshared.asm` | This file contains shared functions by Samus Clones. | file header |
| 123 | 144 | `src/marioshared.asm` | This file contains shared functions by Mario and others | file header |
| 124 | 145 | `src/foxshared.asm` | Shared functions for Fox clones (inferred from name) | inferred from name |
| 125 | 146 | `src/CustomGrabAction.asm` | Custom grab actions (inferred from name) | inferred from name |
| 126 | 148 | `src/MetalMario/MetalMario.asm` | This file contains action edits for Metal Mario. | file header |
| 127 | 150 | `src/Falco/Phantasm.asm` | Phantasm.asm (Fray) | file header |
| 128 | 151 | `src/Falco/Falco.asm` | This file contains file inclusions, action edits, and assembly for Falco. | file header |
| 129 | 153 | `src/Ganondorf/Ganondorf.asm` | This file contains file inclusions, action edits, and assembly for Ganondorf. | file header |
| 130 | 155 | `src/YoungLink/YoungLinkSpecial.asm` | This file contains subroutines used by Young Link's special moves. | file header |
| 131 | 156 | `src/YoungLink/YoungLink.asm` | This file contains file inclusions, action edits, and assembly for Young Link. | file header |
| 132 | 158 | `src/DrMario/DrMario.asm` | This file contains file inclusions, action edits, and assembly for Dr. Mario. | file header |
| 133 | 160 | `src/Wario/WarioSpecial.asm` | This file contains subroutines used by Wario's special moves. | file header |
| 134 | 161 | `src/Wario/Wario.asm` | This file contains file inclusions, action edits, and assembly for Wario. | file header |
| 135 | 163 | `src/DSamus/DSamusSpecial.asm` | This file contains file inclusions, action edits, and assembly for Dark Samus. | file header |
| 136 | 164 | `src/DSamus/DSamus.asm` | This file contains file inclusions, action edits, and assembly for Dark Samus. | file header |
| 137 | 166 | `src/ELink/ELink.asm` | This file contains file inclusions, action edits, and assembly for ELink. | file header |
| 138 | 168 | `src/JSamus/JSamus.asm` | This file contains file inclusions, action edits, and assembly for JSamus. | file header |
| 139 | 170 | `src/JNess/JNess.asm` | This file contains file inclusions, action edits, and assembly for JNess. | file header |
| 140 | 172 | `src/Lucas/LucasSpecial.asm` | This file contains subroutines used by Lucas' special moves. | file header |
| 141 | 173 | `src/Lucas/Lucas.asm` | This file contains file inclusions, action edits, and assembly for Lucas. | file header |
| 142 | 175 | `src/JLink/JLink.asm` | J Link fighter (file inclusions, action edits, assembly) | inferred from name |
| 143 | 177 | `src/JFalcon/JFalcon.asm` | This file contains file inclusions, action edits, and assembly for J Captain Falcon. | file header |
| 144 | 179 | `src/JFox/JFox.asm` | This file contains file inclusions, action edits, and assembly for JFox. | file header |
| 145 | 181 | `src/JMario/JMario.asm` | This file contains file inclusions, action edits, and assembly for JMario. | file header |
| 146 | 183 | `src/JLuigi/JLuigi.asm` | This file contains file inclusions, action edits, and assembly for JLuigi. | file header |
| 147 | 185 | `src/JDK/JDK.asm` | This file contains file inclusions, action edits, and assembly for JDK. | file header |
| 148 | 187 | `src/EPika/EPika.asm` | This file contains file inclusions, action edits, and assembly for EPika. | file header |
| 149 | 189 | `src/JPuff/JPuff.asm` | This file contains file inclusions, action edits, and assembly for JPuff. | file header |
| 150 | 191 | `src/EPuff/EPuff.asm` | This file contains file inclusions, action edits, and assembly for EPuff. | file header |
| 151 | 193 | `src/JYoshi/JYoshi.asm` | This file contains file inclusions, action edits, and assembly for JYoshi. | file header |
| 152 | 195 | `src/JPika/JPikaSpecial.asm` | This file contains subroutines used by JPika's special moves. | file header |
| 153 | 196 | `src/JPika/JPika.asm` | This file contains file inclusions, action edits, and assembly for JPika. | file header |
| 154 | 198 | `src/ESamus/ESamus.asm` | This file contains file inclusions, action edits, and assembly for ESamus. | file header |
| 155 | 200 | `src/Bowser/BowserSpecial.asm` | This file contains subroutines used by Bowser's special moves. | file header |
| 156 | 201 | `src/Bowser/Bowser.asm` | This file contains file inclusions, action edits, and assembly for Bowser. | file header |
| 157 | 203 | `src/GBowser/GBowser.asm` | This file contains file inclusions, action edits, and assembly for Giga Bowser. | file header |
| 158 | 205 | `src/Piano/PianoSpecial.asm` | This file contains subroutines used by Piano's special moves. | file header |
| 159 | 206 | `src/Piano/Piano.asm` | This file contains file inclusions, action edits, and assembly for Piano. | file header |
| 160 | 208 | `src/Wolf/WolfSpecial.asm` | This file contains subroutines used by Wolf's special moves. | file header |
| 161 | 209 | `src/Wolf/Wolf.asm` | This file contains file inclusions, action edits, and assembly for Wolf. | file header |
| 162 | 211 | `src/Conker/ConkerSpecial.asm` | This file contains subroutines used by Conker's special moves. | file header |
| 163 | 212 | `src/Conker/Conker.asm` | This file contains file inclusions, action edits, and assembly for Conker. | file header |
| 164 | 214 | `src/Mewtwo/MewtwoSpecial.asm` | This file contains subroutines used by Mewtwo's special moves. | file header |
| 165 | 215 | `src/Mewtwo/Mewtwo.asm` | This file contains file inclusions, action edits, and assembly for Mewtwo. | file header |
| 166 | 217 | `src/Marth/MarthSpecial.asm` | This file contains subroutines used by Marth's special moves. | file header |
| 167 | 218 | `src/Marth/Marth.asm` | This file contains file inclusions, action edits, and assembly for Marth. | file header |
| 168 | 220 | `src/Sonic/SonicSpecial.asm` | This file contains subroutines used by Sonic's special moves. | file header |
| 169 | 221 | `src/Sonic/Sonic.asm` | This file contains file inclusions, action edits, and assembly for Sonic. | file header |
| 170 | 223 | `src/Sandbag/Sandbag.asm` | This file contains file inclusions, action edits, and assembly for Sheik. | file header |
| 171 | 225 | `src/SSonic/SSonic.asm` | This file contains file inclusions, action edits, and assembly for Super Sonic. | file header |
| 172 | 227 | `src/Sheik/SheikSpecial.asm` | This file contains subroutines used by Sheik's special moves. | file header |
| 173 | 228 | `src/Sheik/Sheik.asm` | This file contains file inclusions, action edits, and assembly for Sheik. | file header |
| 174 | 230 | `src/NWario/NWario.asm` | This file contains file inclusions, action edits, and assembly for Wario. | file header |
| 175 | 232 | `src/NLucas/NLucas.asm` | This file contains file inclusions, action edits, and assembly for Polygon Lucas. | file header |
| 176 | 234 | `src/Marina/MarinaSpecial.asm` | This file contains subroutines used by Marina's special moves. | file header |
| 177 | 235 | `src/Marina/Marina.asm` | This file contains file inclusions, action edits, and assembly for Ultra-InterGalactic-Cybot G Marina Liteyears. | file header |
| 178 | 237 | `src/NBowser/NBowser.asm` | This file contains file inclusions, action edits, and assembly for NBowser. | file header |
| 179 | 239 | `src/NWolf/NWolf.asm` | This file contains file inclusions, action edits, and assembly for Polygon Wolf. | file header |
| 180 | 241 | `src/NDrMario/NDrMario.asm` | This file contains file inclusions, action edits, and assembly for Polygon Dr. Mario. | file header |
| 181 | 243 | `src/NSonic/NSonic.asm` | This file contains file inclusions, action edits, and assembly for Polygon Sonic. | file header |
| 182 | 245 | `src/NSheik/NSheik.asm` | This file contains file inclusions, action edits, and assembly for Polygon Sheik. | file header |
| 183 | 247 | `src/Dedede/DededeSpecial.asm` | This file contains subroutines used by Dedede's special moves. | file header |
| 184 | 248 | `src/Dedede/Dedede.asm` | This file contains file inclusions, action edits, and assembly for Dedede. | file header |
| 185 | 250 | `src/NMarina/NMarina.asm` | This file contains file inclusions, action edits, and assembly for the Ultra-InterPolygon-Fighter N Marina Pointy-Ears. | file header |
| 186 | 252 | `src/Goemon/GoemonSpecial.asm` | This file contains subroutines used by Goemon's special moves. | file header |
| 187 | 253 | `src/Goemon/Goemon.asm` | This file contains file inclusions, action edits, and assembly for Goemon. | file header |
| 188 | 255 | `src/NFalco/NFalco.asm` | This file contains file inclusions, action edits, and assembly for Polygon Falco. | file header |
| 189 | 257 | `src/NGanondorf/NGanondorf.asm` | This file contains file inclusions, action edits, and assembly for Polygon Ganondorf. | file header |
| 190 | 259 | `src/Peppy/PeppySpecial.asm` | This file contains subroutines used by Peppy Hare's special moves. | file header |
| 191 | 260 | `src/Peppy/Peppy.asm` | This file contains file inclusions, action edits, and assembly for Peppy. | file header |
| 192 | 262 | `src/Slippy/SlippySpecial.asm` | This file contains subroutines used by Slippy Toad's special moves. | file header |
| 193 | 263 | `src/Slippy/Slippy.asm` | This file contains file inclusions, action edits, and assembly for Slippy Toad. | file header |
| 194 | 265 | `src/Banjo/BanjoSpecial.asm` | This file contains subroutines used by Banjo's special moves. | file header |
| 195 | 266 | `src/Banjo/Banjo.asm` | This file contains file inclusions, action edits, and assembly for Banjo the Honey Bear and Kazooie the Breegull. | file header |
| 196 | 268 | `src/NDSamus/NDSamus.asm` | This file contains file inclusions, action edits, and assembly for Polygon Dark Samus. | file header |
| 197 | 270 | `src/MLuigi/MLuigi.asm` | This file contains file inclusions, action edits, and assembly for MLUIGI. | file header |
| 198 | 272 | `src/Ebi/EbiSpecial.asm` | This file contains subroutines used by Ebi's special moves. | file header |
| 199 | 273 | `src/Ebi/Ebi.asm` | This file contains file inclusions, action edits, and assembly for Ebi. | file header |
| 200 | 275 | `src/NMarth/NMarth.asm` | This file contains file inclusions, action edits, and assembly for Polygon Marth. | file header |
| 201 | 277 | `src/NMewtwo/NMewtwo.asm` | This file contains file inclusions, action edits, and assembly for Polygon Mewtwo. | file header |
| 202 | 279 | `src/NDedede/NDedede.asm` | This file contains file inclusions, action edits, and assembly for Dedede. | file header |
| 203 | 281 | `src/NYoungLink/NYoungLink.asm` | This file contains file inclusions, action edits, and assembly for Young Link. | file header |
| 204 | 283 | `src/DragonKing/DragonKingSpecial.asm` | This file contains file inclusions, action edits, and assembly for Dragon King. | file header |
| 205 | 284 | `src/DragonKing/DragonKing.asm` | This file contains file inclusions, action edits, and assembly for Dragon King. | file header |
| 206 | 286 | `src/NGoemon/NGoemon.asm` | This file contains file inclusions, action edits, and assembly for NGoemon. | file header |
| 207 | 288 | `src/NConker/NConker.asm` | This file contains file inclusions, action edits, and assembly for NConker. | file header |
| 208 | 290 | `src/NBanjo/NBanjo.asm` | This file contains file inclusions, action edits, and assembly for NBanjo the Honey Bear and NKazooie the Breegull. | file header |
| 209 | 292 | `src/Crash/CrashSpecial.asm` | This file contains subroutines used by Crash Bandicoots's special moves. | file header |
| 210 | 293 | `src/Crash/Crash.asm` | This file contains file inclusions, action edits, and assembly for Crash Bandicoot. | file header |
| 211 | 295 | `src/Peach/PeachSpecial.asm` | This file contains subroutines used by Princess Peach's special moves. | file header |
| 212 | 296 | `src/Peach/Peach.asm` | This file contains file inclusions, action edits, and assembly for Peach. | file header |
| 213 | 298 | `src/Roy/RoySpecial.asm` | This file contains subroutines used by Roy's special moves. | file header |
| 214 | 299 | `src/Roy/Roy.asm` | This file contains file inclusions, action edits, and assembly for Roy. | file header |
| 215 | 301 | `src/DrLuigi/DrLuigi.asm` | This file contains file inclusions, action edits, and assembly for DrLuigi. | file header |
| 216 | 303 | `src/Lanky/LankySpecial.asm` | This file contains subroutines used by Lanky's special moves. | file header |
| 217 | 304 | `src/Lanky/Lanky.asm` | This file contains file inclusions, action edits, and assembly for the one and only Lanky Kong. | file header |
| 218 | 306 | `src/NPeach/NPeach.asm` | This file contains file inclusions, action edits, and assembly for NPeach. | file header |
| 219 | 308 | `src/NCrash/NCrash.asm` | This file contains file inclusions, action edits, and assembly for Polygon Crash Bandicoot. | file header |
| 220 | 311 | `src/Kirby/Kirby.asm` | This file contains file inclusions, action edits, and assembly for Kirby. | file header |
| 221 | 312 | `src/KirbyHats.asm` | This file allows for adding Kirby "hats" without needing to modify Kirby's original files. | file header |
| 222 | 314 | `src/JKirby/JKirby.asm` | This file contains file inclusions, action edits, and assembly for JKirby. | file header |
| 223 | 317 | `src/Random/Random.asm` | This file contains file inclusions, action edits, and assembly for Sheik. | file header |
| 224 | 320 | `src/MIDI.asm` | MIDI.asm (Fray) | file header |
| 225 | 323 | `src/PAL.asm` | PAL build variant (MAKE_PAL) | inferred from name |

(Total source includes: 225.)

Notes on the list:
- Header comments that look copy-pasted and do not match the file: `src/Sandbag/Sandbag.asm` and `src/Random/Random.asm` say "for Sheik".
- Several fighter files include a `*Special.asm` helper before the fighter file (e.g. `Lucas/LucasSpecial.asm` then `Lucas/Lucas.asm`).

## Open questions / not verified

- Vanilla ROM size and the exact 0x1AC870 reloc table layout: taken from the task brief, not verified in this repo.
- Code image size (end of `custom_heap`) and the DMA length: require an assembled build (bass is Windows-only).
- Whether `file_table` length 0x300 is in entries or bytes (see B.4).
- The 2 ids 0x0FF6/0x0FF7 are named only in `roms/filename_overrides.txt`; their intended `File.*` symbols are not defined.
- 0xC1B: the overrides name says "Remix 1p Kirby Stage File", but the code comments at `KirbyHats.asm:673` and `SinglePlayerModes.asm:5932` say "kirby hats remix". Treat the name as unconfirmed.
