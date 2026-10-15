# Shared patches: JigglypuffKirbyShared and MarioShared (pipe_turn)

Sources: `.toolchain/pkgs/smashremix-src/src/jigglypuffkirbyshared.asm` (1257 lines) and
`.toolchain/pkgs/smashremix-src/src/marioshared.asm` (254 lines). Vanilla addresses are looked up in
`build/remixasm/refs/vanilla_funcs.txt`. Line numbers below are asm lines.

Port style: the vanilla routine gets the clone ids as `#ifdef REGION_REMIX` case labels or `||` terms
(the asm's ID checks, 1:1), and the helpers live in `decomp/src/ft/ftremix/ftremixpuffkirbyshared.c`
(declared in `ftremixhooks.h`, block "Jigglypuff and Kirby clones").

Helper file: `decomp/src/ft/ftremix/ftremixpuffkirbyshared.c`
- `ftRemixDededeJumpAerialMultiplier(jumps_used)`: Dedede.asm `jump_multiplier_table` (L733-741): 0, 0, 68, 58, 52, 0.
- `ftRemixIsPipeTurnFkind(fkind)`: Character.pipe_turn table; only entry set in the pkgs source is DRL
  (DrLuigi.asm L150, `Character.table_patch_start(pipe_turn, Character.id.DRL, 0x1)`).
- `ftRemixKirbyCopyAmmo(fp)`: kirby_ammo_ (copy_id == BOWSER -> halfword 0x0AE2 = 0x14).
- `ftRemixKirbyCutterWeaponDesc(fp, vanilla)`: kirby_special_struct_fix (JKIRBY -> copy of vanilla WPDesc with p_weapon = p_file_main).
- `ftRemixKirbyStoneHealthMax / Mid / ReleaseDuration`: kirby_rock, kirby_rock_2, kirby_rock_3 (J values 50 / 25 / 160-14).

## JigglypuffKirbyShared

| asm scope | RAM | vanilla function (file:line) | status | notes |
|---|---|---|---|---|
| jump_fix_1 (L11-40) | 0x8013FF68 | ftCommonJumpAerialMultiSetStatus (ftcommonjumpaerial.c:186) | applied | status switch: JKIRBY, DEDEDE, NDEDEDE -> Kirby branch; JPUFF, EPUFF -> Purin branch |
| jump_fix_2 (L43-87) | 0x8014004C | same | applied | vel switch: JKIRBY -> Kirby table; JPUFF/EPUFF -> Purin table; DEDEDE/NDEDEDE -> Dedede multiplier (uncertain, see U3) |
| jump_fix_3 (L90-115) | 0x801401E0 | ftCommonJumpAerialCheckInterruptCommon (:288) | applied | gate of the multi-jump block: adds JKIRBY, DEDEDE, NDEDEDE, JPUFF, EPUFF |
| jump_fix_4 (L118-147) | 0x8013FCB0 | ftCommonJumpAerialProcPhysics (ftcommonjumpaerial.c:98) | applied | air-drift switch: JKIRBY, DEDEDE, NDEDEDE -> Kirby; JPUFF, EPUFF -> Purin |
| jump_fix_5 (L150-179) | 0x80140238 | ftCommonJumpAerialCheckInterruptCommon (:288) | applied | inner status-range switch: same mapping as jump_fix_1 |
| kirby_boomerangfix_1 (L182-199) | 0x8016D3BC | wpLinkBoomerangCheckOwnerCatch (wp/wplink/wplinkboomerang.c:301) | applied | JKIRBY joins the Kirby branch |
| kirby_boomerangfix_2 (L202-219) | 0x8016D330 | wpLinkBoomerangClearGObjs (wplinkboomerang.c:284) | applied | JKIRBY joins the Kirby branch |
| kirby_special_struct_fix (L222-249) | 0x8016BEA0 | wpKirbyCutterMakeWeapon (wp/wpkirby/wpkirbycutter.c:140) | applied | desc pointer swapped for JKIRBY (helper); see U4 |
| kirby_infinite_1 (L252-269) | 0x8014F1CC | ftCommonAttack100LoopKirbyUpdateEffect (ftcommonattack100.c:111) | applied | JKIRBY joins the Kirby check |
| kirby_fthrow_1_fix (L272-303) | 0x8014A238 | ftCommonThrowSetStatus (ftcommonthrow.c:73) | partial | JKIRBY joins the Kirby ThrowF branch. NOT applied: PokemonAnnouncer.seismic_toss_announcement_ (L297) and the BOWSER/GBOWSER branch (L285-290, t3 = 0xE5). REMIX-TODO(PokemonAnnouncer), marker in ftcommonthrow.c |
| kirby_fthrow_2_fix (L306-323) | 0x8014A34C | ftCommonThrowSetStatus, is_kirby gate (ftcommonthrow.c ~line 150) | applied | JKIRBY joins the ThrowF is_ignore_dead gate; see U5 |
| kirby_blast_fix_1 (L326-361) | 0x800D79B0 | ftManagerDestroyFighterWeapons (ftmanager.c:574) | partial | JKIRBY -> kirby boomerang destroy; YLINK, ELINK, JLINK -> link destroy. NOT applied: BOWSER `sh 0x14, 0x0ADE(v1)` refill (L347-348), v1 not identified; see U6 |
| kirby_load_with_hat_fix_ (L364-383) | 0x800D7E54 | ftManagerInitFighter, Kirby init case (ftmanager.c ~791) | applied | JKIRBY joins the Kirby init case; see U7 |
| kirby_taunt_fix_1 (L386-404) | 0x8014E6F8 | ftCommonAppealSetStatus (ftcommonappeal.c:21) | applied | JKIRBY joins the copy-loss check |
| kirby_taunt_fix_2 (L406-425) | 0x8014E70C | n/a | skipped | commented out in the asm |
| pokemon_spawn_fix (L428-456) | 0x8013DA40 | ftCommonAppearUpdateEffects (ftcommonentry.c:94) | applied | EPIKA, JPIKA, JPUFF, EPUFF get the pokeball rays. Does not depend on the Remix pokemon item set, so no Item.Pokemon TODO |
| pokemon_hat_fix_1 (L459-483) | 0x800F1F30 | ftDisplayMainDrawDefault (ftdisplaymain.c:762) | applied | Pikachu accessory: EPIKA, JPIKA added |
| pokemon_hat_fix_2 (L486-510) | 0x800F20E0 | same | applied | Purin accessory (2 sites, `case 0` and the default): JPUFF, EPUFF added |
| kirby_cpu_inhale (L513-536) | 0x80136C1C | n/a (CPU) | skipped | REMIX-TODO(AI) |
| kirby_cpu_inhale_2 (L539-558) | 0x80138F08 | n/a (CPU) | skipped | REMIX-TODO(AI) |
| kirby_cpu_inhale_3 (L561-614) | 0x801370F0 | n/a (CPU) | skipped | REMIX-TODO(AI) |
| kirby_cpu_inhale_4 (L617-639) | 0x800E23E8 | n/a (CPU) | skipped | REMIX-TODO(AI) |
| fully_charged_check_ (L643-660) | 0x800EAC24 | ftParamMakeEffect (ftparam.c:2026) | applied | JKIRBY added as a label of the Kirby charge-effect case; see U8 |
| kirby_power_steal_check_ (L664-681) | 0x801620FC | ftKirbySpecialNCatchProcUpdate (ftkirbyspecialn.c:175) | applied | victim JKIRBY joins the Kirby steal branch |
| kirby_power_stolen_check_ (L685-702) | 0x8014C00C | ftCommonThrownCommonStarUpdatePhysics (ftcommoncapturekirby.c:285, is_kirby block) | applied | JKIRBY joins the Kirby branch |
| kirby_power_change_ (L706-726) | 0x80161ED4 | ftKirbySpecialNInitPassiveVars (ftkirbyspecialn.c:87) | applied | copy_id JPUFF, EPUFF -> copypurin_unk reset |
| kirby_ammo_ (L730-746) | 0x80161F80 | ftKirbySpecialNCopyInitCopyVars (ftkirbyspecialn.c:111) | applied | after the copy_id store; helper ftRemixKirbyCopyAmmo |
| kirby_rock (L749-766) | 0x80161424 | ftKirbySpecialLwSetDamageResist (ftkirbyspeciallw.c:44) | applied | max damage 50 for JKIRBY, else vanilla |
| kirby_rock_2 (L770-787) | 0x80161374 | ftKirbySpecialLwUpdateColAnim (ftkirbyspeciallw.c:16) | applied | MID threshold 25 for JKIRBY, else vanilla; see U9 |
| kirby_rock_3 (L791-808) | 0x801614E4 | ftKirbySpecialLwCheckRelease (ftkirbyspeciallw.c:74) | applied | duration threshold 160-14 for JKIRBY, else vanilla |
| multi_jump_recovery_fix_ (L812-854) | 0x80135198 | none (AI overlay; the decomp has no ft function here) | skipped | REMIX-TODO(AI): CPU recovery ledge logic. Listed in scope but CPU-only |
| remix_skip_offensive_usp_air_ (L856-883) | 0x80133F70 | none (AI overlay) | skipped | REMIX-TODO(AI): comment says it stops the CPU from using the attack |
| upspecial_struct_jkirby (L886-890) | data | used by kirby_special_struct_fix | applied | rebuilt from the vanilla WPDesc, see U4 |
| recovery_logic, ledge_check (L894-1033) | CPU | n/a | skipped | REMIX-TODO(AI) |
| cpu_post_process, dsp_check, _dsp_logic (L1034-1175) | CPU | n/a | skipped | REMIX-TODO(AI) |
| cpu_attack_weight, cpu_attack_weight_kirby (L1176-1257) | CPU | n/a | skipped | REMIX-TODO(AI) |

## MarioShared

| asm scope | RAM | vanilla function (file:line) | status | notes |
|---|---|---|---|---|
| pipe_turn_enter (L204-227) | 0x80142200 | ftCommonDokanStartSetStatus (ftcommondokan.c:73) | applied | Mario-turn condition adds ftRemixIsPipeTurnFkind (DRL) |
| pipe_turn_exit (L230-252) | 0x8014275C | ftCommonDokanEndSetStatus (ftcommondokan.c:276) | applied | non-Mario condition adds `!ftRemixIsPipeTurnFkind` |

## Uncertainties

- U1 (L11-40 jump_fix_1 vs L18-28): the asm assigns DEDEDE and NDEDEDE to the Kirby status path. Ported literally.
- U2 (L77-86): the Dedede branch reconstructs the velocity from the tail at 0x80140090, which I did not decode. Ported as table[jumps_used] * (stick / 80), the same formula as the Kirby table.
- U3 (L67, L72): the decay constant 80.0 (0x42A0) is the divisor the vanilla velocity formula already uses (`stick_range_y / F_CONTROLLER_RANGE_MAX`), so no separate constant is needed in the port.
- U4 (L886-890): the struct copy is ROM 0x103CEC (0x40 bytes), which I assume is vanilla's WPDesc data. The port copies `dWPKirbyCutterWeaponDesc` and swaps `p_weapon`. Word 2 of the asm struct (0x04) was not checked against the vanilla kind.
- U5 (L318, L322): `addiu at, r0, 0x0016` is taken to be nFTKirbyStatusThrowF. Not verified against Kirby.asm.
- U6 (L347-348): `sh 0x14, 0x0ADE(v1)`, v1 not identified. Not ported.
- U7 (L371-375): the second compare `beq v0, t3` runs after `addiu v0, JKIRBY`, so as written it compares JKIRBY with t3. The intent (JKIRBY takes the Kirby branch) is ported.
- U8 (L643-660): `j_0x800EAC84` is taken to be the Kirby charge-effect case in ftParamMakeEffect.
- U9 (L777-782): the 0x16 compare is taken to be FTKIRBY_STONE_HEALTH_MID (22 U / 25 J), as the decomp uses MID first.
- U10 (L285-290): BOWSER/GBOWSER forward throw sets t3 = 0xE5 before the shared Kirby path. Not ported (see the REMIX-TODO).

## REMIX-TODO summary

- REMIX-TODO(PokemonAnnouncer): kirby_fthrow_1_fix announcement for JKIRBY, BOWSER, GBOWSER (ftcommonthrow.c, marker in code).
- REMIX-TODO(AI): multi_jump_recovery_fix_, remix_skip_offensive_usp_air_, kirby_cpu_inhale*, recovery_logic, ledge_check, cpu_post_process(_kirby), dsp_check, _dsp_logic, cpu_attack_weight(_kirby).
- Not a TODO but not ported: kirby_blast_fix_1 BOWSER ammo refill (U6), kirby_fthrow_1_fix Bowser branch (U10).
- Kirby copy subsystem (copy_id >= 27): not touched, only the ID checks listed above.
