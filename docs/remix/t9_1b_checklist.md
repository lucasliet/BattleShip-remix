# Manual pass: listening and play checklist

Covers the checks that need a person at the controls: what to open, what to expect, what to note. Automated checks are in `.claude/skills/remix-smoke-tests/`; the
audio background is in `docs/remix/audio.md`. The game runs from `build/` with `BattleShip-Remix.o2r` and `f3d.o2r` present. For each item answer "ok" or describe the defect (where, what was heard or seen, what the Remix does).

## How to open the game

```
cd build && ./BattleShip-Remix
```

Shortcuts to go straight to the point (environment variables before the command):

| Goal | Command |
|---|---|
| VS match on a stage | `SSB64_START_SCENE=22 SSB64_VS_FKINDS=0,8 SSB64_VS_STAGE=<id> ./BattleShip-Remix` (P1 human) |
| Sound Test | `SSB64_START_SCENE=59 ./BattleShip-Remix` |
| Training without the CSS | `SSB64_TRAIN_FKINDS=<human>,<cpu> SSB64_VS_STAGE=<id> ./BattleShip-Remix` |
| Training with menu options | add `SSB64_REMIX_TRAINING="ShieldBreak=1;Oos=3;Tech=4"` |
| DK Mode (test shortcut for the portrait easter egg) | `SSB64_DK_MODE=<0 off, 1 DK, 2 Lanky> ./BattleShip-Remix` |

Keyboard (libultraship default, same as the original BattleShip): **WASD is the analog stick and the arrow keys are the C buttons**, and a tap on a C button makes
the fighter jump. To walk use WASD (X = A, C = B, Z = Z, E = L, R = R, Space = Start).

Remix fighter ids equal the port fkind (Crash 72, Marina 63, Young Link 31, Lanky 76, Marth 58).

## Expected music per stage

| Stage | Id | Expected title |
|---|---|---|
| Dream Land | 6 | Dream Land - Kirby Super Star |
| Deku Tree | 41 | Kokiri Forest - The Legend of Zelda: Ocarina of Time |
| Pokemon Stadium | 45 | Trainer Battle - Pokemon Red & Blue |
| Taltal | 46 | Link's Awakening Medley - The Legend of Zelda: Link's Awakening |
| Glacial | 47 | River Stage - Marvel vs. Capcom 2 |
| WarioWare | 48 | WarioWare, Inc. - WarioWare, Inc.: Mega Microgame$! |
| Battlefield | 49 | Battlefield - Super Smash Bros. Brawl |
| Dr. Mario | 51 | Fever - Dr. Mario |

With the toggle "Music Title at Match Start" on (the default) the title "track - game" appears on screen during the 3, 2, 1 count, and the log prints
`music title: bgm N "..."`. The alternative music is drawn 35% of the time, so a different title can be legitimate.

## 1. Music and stages

- [ ] For each stage of the table above: the track is the Remix one for that stage and does not skip or stutter in the first 20 s; the title on screen is the expected one and
  the sound is that piece of music.
- [ ] Peach's Castle II (id 68): opens like the original Peach's Castle (no Banzai Bill, which is dead code in the Remix).
- [ ] Victory music of a Remix fighter (end a match with Crash, Marina or Marth) and the results screen.
- [ ] Sound Test: browse, play one Remix track and one vanilla track, wrap back to the first at the end of the list.

## 2. Voices and SFX

- [ ] Hits of Remix fighters (Crash, Marina, Marth and others of your choice): the sound follows the hit, with no silence and no repeated sound.
- [ ] J (Japanese) voices on the stage and fighters where the Remix swaps the sound.

## 3. Menus and graphics

- [ ] Title, CSS (the port's default CSS) and stage select.
- [x] Pause: "R: Legend" labels in the right position, not overlapping "1P PAUSE" (the "1P PAUSE" title, the A+B+Z+R: RESET line and "R: Legend" in the corner).
- [x] KOTH and Smashketball results: extra lines on the end screen; a stock match end screen shows KOs, TKO, Pts, Place and "Press A for Match Stats".

## 4. Training

- [ ] Port menu, Remix > Training: each entry changes the behaviour (Shield Break Infinite holds the CPU in shield; Spam Practice repeats the move at the interval; Percent applies
  the value when the match is created; Stage BG swaps the background).
- [ ] Training music (Remix > Training > Music): choosing a track changes the match music immediately, and the hammer and the star keep their own theme; when they end, the chosen
  track returns.
- [x] OOS against a real shield (Grab out of shield).
- [ ] The per-port options (Type, Character, Costume, Spawn) are saved between sessions, like the other Remix > Training ones; **Auto** is "what the CSS picked" and any other value holds in
  every match until set back. If a Training match shows up full of CPUs, check the P1 to P4 Type options first; "Reset the Training options" clears everything.
- [ ] Per-port chain (Remix > Training, P1 to P4 Type, Character, Costume, Spawn): change and press D-pad left (reset) to rebuild the match with the chosen fighters.
- [x] Menu Tech: fsmash from the platform at 30%, the CPU touches the floor and rolls: status 73 for Roll Left, 74 for Roll Right, 81 for In Place.
- [ ] CPU DI (`DiType=3;DiStrength=1;DiDirection=1;DiFirstHit=1`).

## 5. Other checks

- [x] Surface conveyor, bury and plunge, Game Speed in Training, return of Lightning to the mode size, Full Results and Salty Runback. Salty Runback: hold exactly A+B+Z+R+START (or
  A+B+Z+R+D-Left with the alternative option) while the match ends, with no other buttons.

## Reporting

A short list per section, in the form "section, item, ok or defect". Each defect is fixed with a regression test.
