# Metal: redirect-to-Z fills painted color over the scene (VS results screen all white)

**Status:** FIXED (libultraship Metal backend). Found while porting Smash Remix (ROADMAP T5.3).

## Symptom

On macOS (Metal) the VS results screen rendered a solid cream (about 255,255,247) frame behind and after the photo-wipe
transition, for every fighter pair (vanilla Mario/Kirby too). Without the transition the scene drew correctly.

## Root cause

`lbTransitionMakeCamera` creates a camera with `COBJ_FLAG_ZBUFFER`. Every camera with that flag ends its pass with a
depth clear written through the colour path: `SETCIMG` pointing at the Z image, then `SETFILLCOLOR 0xFFFCFFFC` and
`FILLRECT`. The transition camera draws last, so its clear runs after the whole scene.

Fast3D routes those draws through the redirect-to-Z path (`RdpColorImageIsZBuffer`) and asks the backend to mask
framebuffer colour writes with `SetColorWriteMask(false)`. The OpenGL and Direct3D 11 backends implement it; the Metal
backend did not (the base class default is a no-op), so the Z-clear quad was written to the colour buffer as cream
(0xFFFC in RGBA5551) on top of the finished frame. A camera that clears Z first in the frame hides the same bug,
which is why only this late clear showed up.

Found by diffing a normalised GBI trace of frame 990 with and without the transition: the only structural change was
the extra `SETCIMG(z) + FILLRECT FFFC` pass at the end of the frame.

## Fix

`libultraship/src/fast/backends/gfx_metal.cpp` and `include/fast/backends/gfx_metal.h`:

- `SetColorWriteMask(bool)` stores the state.
- `ShaderProgramMetal` keeps its vertex/fragment functions, vertex descriptor and sRGB mode, and builds a second
  pipeline state per MSAA level with `ColorWriteMaskNone` on first use (`GetNoColorPipelineState`).
- `DrawTriangles` re-binds the pipeline when the colour-write state changes (`mLastColorWrite`).

## Verification

Mario/Kirby and Peppy/Crash VS matches with a KO, transition enabled: door fold, star mask and results screen draw
normally (frames 800 to 1300).
