# I8: reference emulator for visual comparison (M11 Fase 1)

Setup and capture of the real Smash Remix ROM (`baserom.remix.z64`) in a cycle-accurate emulator, to compare screens with the port.
Everything lives in `.toolchain/ref-emu` (git-ignored; mupen64plus and angrylion are GPL and never enter the repository). The
screenshots are copyrighted game frames, so they stay in `build/ref-shots/` and are not committed.

## Use

```
scripts/macos/setup-ref-emu.sh                       # once: builds mupen64plus 2.6.0, cxd4 LLE RSP, angrylion RDP plus, our input plugin
scripts/ref-emu/capture.sh baserom.remix.z64 <frames,...> build/ref-shots/<name> [input.txt]
.toolchain/host/venv/bin/python scripts/ref-emu/sheet.py build/ref-shots/<name> 4 320   # contact sheet
```

- A 3000 frame run takes about 25 seconds (pure interpreter, no speed limit). Clean the output directory before a run: the
  screenshots are numbered from 000 and a new run does not remove the old ones.
- The input file has the port's format (`<from> <to> <port> <buttons|-> [x y]`) and is read by
  `scripts/ref-emu/input-script.c` (frame = poll of port 0). `REF_EMU_INPUT_TRACE=1` prints every change of the pad value.
- Frame numbers of `--testshots` count presented frames, the poll counter runs about 13 percent ahead, and the port counts every VI:
  align by content, never by index.
- The title ignores START at poll 900; A at about 1500 opens the Mode Select. Menus accept input only a few hundred frames after
  they appear. `scripts/ref-emu/inputs/vs_match.txt` reaches a 1-stock VS match on Peach's Castle (Ness vs Bowser) and walks P1 off
  the stage; `vs_stats.txt` continues to the results and opens Match Stats (A at 4400, R at 4550, B at 4700); `vs_pause.txt` pauses
  at 3500.
- The read tool sometimes needs two tries to open a PNG written a moment earlier.

## What the Remix does (read from the captures, 2026-11-03)

Match Stats on the results screen (shots at `stats`):

- The page background dims everything; the normal rows (KOs, TKO, Pts, Place) and the header are hidden, the 1P and 2P pointers stay
  (dimmed).
- The "Press (A) for Match Stats" prompt and its A glyph stay visible, dimmed under the background. The port hid them; fixed in
  `mnremixvsstatsview.c` (the A sprite stays, the prompt text is drawn at 31 percent white while the page shows).
- The page headers "1P" and "2P" carry the character stock icon on their left (the port draws only the text; T8.3).
- The B glyph is green with a "B", R is a grey trigger shape, matching the sprites the port now draws (I2).

Pause (shots at `pause`):

- A character close-up (model of the paused fighter), the "1P PAUSE" title with a D-pad icon at the top left.
- A music block: "(note) Peach's Castle" with the series name under it, then "D-pad Right: Next Music Track" and "D-pad Down: Random Music
  Track" (D-pad glyphs), then the camera rows with A, B, C-stick, Z glyphs, and at the bottom "A B + Z + R : RESET" and "R : LEGEND" with
  glyphs. The port draws only the camera rows and "R: Legend" as text (T8.3, Fase 2).
