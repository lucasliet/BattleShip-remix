# N64 addresses the Remix asm uses, resolved to decomp symbols

Covers the addresses the Remix asm reads or writes that the decomp names differently, and what each means for the port. Tool: `tools/n64_addr_to_symbol.py`
(`.toolchain/host/venv/bin/python -I tools/n64_addr_to_symbol.py 0x80131580 [...] [--near 0x300]`). It reads the `// 0xADDRESS` comments and the address-bearing names of
`decomp/src`, plus `build/remixasm/remix.sym`. Overlays share one RAM range, so pick the owner whose file is loaded in the scene the asm runs in.
Test: `.toolchain/host/venv/bin/python -I tools/test_n64_addr_to_symbol.py`.
Per-patch resolution of every `OS.patch_start` (by function prologue in the vanilla ROM): `tools/audit_remix_patches.py`.

| Address | Owner | Where the asm uses it | Port consequence |
|---|---|---|---|
| 0x80131580 | `gIFCommonPlayerInterface` (`if/ifcommon.c`), first byte `is_magnify_display` | ZCancel.asm 165-167 (1P final boss instant KO), SpecialZoom.asm (saves and restores it) | the "match off" byte is the magnify flag; the 1P boss KO gate (`ftremixzcancel.c`) reads `gIFCommonPlayerInterface.is_magnify_display` |
| 0x80131AD8 | `sEFGroundActor` (`ef/efground.c`) | Hazards.asm 8429 (Big Boo's Haunt laugh gate) | the word is the first field of `EFGroundActor`; `gr/grremix/grremixbbh.c` reproduces the gate |
| 0x80132704 | battle overlay ftcomputer code (delta 0x800855C0, checked with the AI.asm patch at 0x80131DA4 and the `ftComputerSetCommandWaitShort` prologue 0x27BDFFE8 at 0x80132564): the word is the branch 0x10000007 | SpecialZoom.asm 1015 (wallpaper `rotate.y`) | read as a float it is about 2.5e-29, so the rotation is 0; `gmremixspecialzoomwallpaper.c` sets 0 |
| 0x800E9C8C, 0x800E9CC4 | `ftParamProcPauseEffect`, `ftParamProcResumeEffect` (`ft/ftparam.c`) | DragonKingSpecial.asm 885-886 and 943-944 (`lui 0x800F` plus `addiu 0x9C8C`/`0x9CC4`; the low half is signed, so the high half is 0x800E, not 0x800F) | the Kirby transitions set `proc_lagstart`/`proc_lagend` to these two |
| 0x8014A24C | Remix label `JigglypuffKirbyShared.kirby_fthrow_1_fix._return` | Bowser fthrow | Remix code, not vanilla |
| 0x8018C230 | battle overlay data (delta 0x800855C0, ROM 0x106C70): the floats pi, pi/2, pi, -pi/2 | MarinaSpecial.asm `throw_turn_` (`lui 0x8019` plus `lwc1 0xC230..0xC23C`; the low half is signed, so the address is 0x8018C230) | `ftremixmarinansp.c` uses the four floats |
