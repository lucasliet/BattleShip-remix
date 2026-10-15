# Reloc chain walk derailed by a chain-time texture fixup (2026-10-14)

**Symptom.** Smash Remix stage Mementos (stage id 0x3B, `SSB64_VS_STAGE=59`) crashed while loading:
`mpCollisionAllocLinesGetCountTotal` dereferenced `PORT_RESOLVE(line_info) == NULL`. The stdout had
`lbReloc bridge: file_id 2217 intern chain walk STOPPED (off=59979 steps=167 copySize=113200) — corrupt chain word`.

**Cause.** `lbRelocLoadAndRelocFile` (`port/bridge/lbreloc_bridge.cpp`) walks the intern chain and, at each slot that is
the `w1` of a `G_SETTIMG` or `G_VTX`, `portRelocFixupTextureFromChain` (`lbreloc_byteswap.cpp`) byte-swaps the target
texture or vertex range right away. In file 2217 (`STG_MEMENTOS`) the SETTIMG at chain step 113 (slot word 18567) sizes
its texture from the following `G_LOADBLOCK` as 512 words starting at word 19718. That range covers word 19950, which is
the chain descriptor of step 167. The BSWAP turned that `{next, target}` pair into garbage (`next = 59979` words, past the
end of the file), the walk stopped, and every pointer slot after it stayed an untranslated descriptor. The undecoded file
has a valid chain of 229 steps ending in `0xFFFF` (checked with `SSB64_DUMP_FILE_ID=2217` and the table entry at
`0x1AC870 + 2217 * 12` of the Remix ROM: intern `0x32E7`, extern `0xFFFF`).

**Fix.** Before walking, the bridge records every slot of the file's intern and extern chains
(`portRelocBeginChainWalk`), and the chain-time texture and vertex fixups stop at the first recorded slot inside their
range (`wordsBeforeChainSlot`); a texel or vertex array never contains a pointer slot, so a range that reaches one was sized
past the real data. The set is dropped after both walks (`portRelocEndChainWalk`). The runtime texture fixup already refused
ranges containing tokenized slots.

**Verification.** Stage 59 runs 1500 frames and renders (red Persona stage with platforms); no `chain walk STOPPED` line in
stdout; full stage sweep 41 to 215 after the change.
