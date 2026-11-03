# N64 addresses the Remix asm uses, resolved with `tools/n64_addr_to_symbol.py` (M11 I5)

Tool: `python3 tools/n64_addr_to_symbol.py 0x80131580 [...] [--near 0x300]`. It reads the `// 0xADDRESS` comments and the
address-bearing names of `decomp/src`, plus `build/remixasm/remix.sym`. Overlays share one RAM range, so pick the owner whose
file is loaded in the scene the asm runs in. Test: `python3 tools/test_n64_addr_to_symbol.py`.

| Address | Owner | Where the asm uses it | Consequence for the port |
|---|---|---|---|
| 0x80131580 | `gIFCommonPlayerInterface` (`if/ifcommon.c:442`), first byte `is_magnify_display` | ZCancel.asm 165-167 (1P final boss instant KO), SpecialZoom.asm (saves and restores it) | the "match off" byte is the magnify flag; the 1P boss KO gate (Fase 3) reads `gIFCommonPlayerInterface.is_magnify_display` |
| 0x80131AD8 | `sEFGroundActor` (`ef/efground.c:1357`) | Hazards.asm 8429 (Big Boo's Haunt laugh gate) | the word is the first field of `EFGroundActor`; BBH owner (Fase 3) |
| 0x80132704 | no owner in a battle overlay (candidates are menu overlays); in SpecialZoom `create_gfx` it replaces a `lwc1` of the vanilla function it clones | SpecialZoom.asm 1015 (wallpaper `rotate.y`) | the plane shows with rotation 0 (seen on screen in `kozoom3`); value not recoverable from the decomp, kept as the default and documented |
| 0x800F9C8C, 0x800F9CC4 | `mpCollisionCheckProjectRWall` (`mp/mpcollision.c:2972`, +0x5C and +0x94) | DragonKing lag patches | the hooks sit inside the wall projection test; Fase 3 |
| 0x8014A24C | Remix label `JigglypuffKirbyShared.kirby_fthrow_1_fix._return` | Bowser fthrow | Remix code, not vanilla; Fase 3 |
| 0x8019C230 | none found | Marina floats | the address is in the vanilla data of a fighter overlay; read from the ROM image in Fase 3 |
