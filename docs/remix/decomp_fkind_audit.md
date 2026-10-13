# Auditoria: sitios indexados pela contagem de fighters (nFTKind*) - alvo 97 fighters

Repositorio: /Users/lucas/Projetos/Pessoal/BattleShip (somente leitura; unico arquivo escrito: este relatorio).
Data: 2026-10-12. Escopo: decomp/src (ft, mn, sc, gm, lb, ef, it, wp, gr, sys, mv, if, mp), decomp/include, port/ (fighter_registry, port.cpp, enhancements, css_icons, bridge).

## Metodo

1. Simbolos de contagem: `nFTKindEnumCount`, `nFTKindNull`, `PORT_VANILLA_FKIND_COUNT`, `nFTKindPlayableEnd`, `GMCOMMON_FIGHTERS_PLAYABLE_NUM`, `FTKIRBY_COPY_TABLE_COUNT`.
2. Declaracoes de array dimensionadas por nFTKind* ou por literal (12, 27, 28, 0x1B, 54).
3. Indices com nomes `fkind`, `fp->fkind`, `copy_id`, `victim_fp->fkind`, `gkind`, `costume`, `fkinds[]`, `portraits[]` etc (grep em decomp/src, decomp/include, port/).
4. Contagem de inicializadores com script auxiliar (fora do repo, em $TMPDIR) e conferencia manual de varios casos. Para tabelas com blocos `#ifdef PORT` / `#else` a contagem pode dobrar; essas linhas estao marcadas "verify".
5. Para cada sitio, verificacao da diretiva de pre-processador mais proxima (PORT, #else, #endif) para saber se existe ramo guardado.

Nao foi compilado nem executado nada. Os numeros de linha sao os do estado atual do checkout.

## Resumo

Entradas agrupadas: **124** (cada entrada cobre um simbolo ou um grupo de linhas do mesmo padrao). Cerca de 30 entradas de tabela estatica precisam crescer ou virar registry; 28 sao tabelas CSS de 12 slots.

### Contagem por diretorio e tipo de mudanca

| Diretorio | Sitios | INIT | REG | DIM | SAVE | CSS | COSTUME | NONE | Risco H |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| ft/ | 38 | 24 | 7 | 5 | 0 | 0 | 0 | 2 | 9 |
| ef/ | 4 | 2 | 1 | 0 | 0 | 0 | 0 | 1 | 2 |
| mn/ | 44 | 0 | 13 | 0 | 5 | 23 | 1 | 2 | 4 |
| mv/ | 2 | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| sc/ | 18 | 3 | 1 | 1 | 4 | 5 | 1 | 3 | 5 |
| gr/ | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| lb/ | 6 | 0 | 0 | 0 | 6 | 0 | 0 | 0 | 5 |
| sys/ | 2 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 |
| gm/ | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| it/ | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| wp/ | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| if/ | 2 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 0 |
| port/ | 4 | 0 | 2 | 0 | 0 | 0 | 0 | 2 | 0 |
| **Total** | **124** | **30** | **24** | **6** | **15** | **28** | **3** | **18** | **25** |

Riscos: H = 25 entradas (leitura/escrita fora dos limites com fkind >= 27 ou mascara de 16 bits), M = 71, L = 28.

### Resposta direta

- Precisam de crescimento de inicializador ou de tabela (INIT + CSS): **58** entradas.
- Podem usar o registry (REG): **24** entradas, mais as **6** de DIM que so trocam o limite por uma capacidade.
- Dependem de mudanca de formato de save (SAVE): **15** entradas (mascara `fighter_mask` de 16 bits e `spgame_records`/`vs_records` de 12 linhas dentro de `LBBackupData`).
- Nao precisam de mudanca: **18** (NONE) e **3** de costume (COSTUME) que dependem de outra contagem.

## Legenda

- INIT: tabela estatica com 27 linhas (ou 28/12/54 conforme o caso) precisa crescer para 97 (70 linhas novas) ou ser preenchida pelo registry.
- REG: deve passar por um accessor `port_fighter_*` (fighter_registry); ja existe ramo PORT em parte dos sitios.
- DIM: limite de loop ou dimensao derivada de `nFTKindEnumCount`; precisa de uma constante de capacidade separada.
- SAVE: registro persistente de SRAM ou mascara de 16 bits (`fighter_mask`); exige mudanca de formato de save com migracao.
- CSS: tabela de 12 slots (jogaveis apenas) usada na selecao de personagem/menus; exige registry ou extensao das paginas de CSS.
- COSTUME: indexada por costume (LUT/variacao), nao por fkind.
- NONE: nao precisa mudanca (guardada, nao e fkind, ou indexa estagio/gkind).

## Top 10 sitios mais arriscados

| # | Local | Motivo | Correcao minima |
|---|---|---|---|
| 1 | ft/ftcommon/ftcommoncapturecaptain.c:27-28 | `offset_add[capture_fp->fkind]` sem guarda e sem accessor no registry | Adicionar campo no FighterDescriptor + accessor + ramo PORT |
| 2 | ef/efmanager.c:5966 | `copy[fp->fkind].effect_scale` sem guarda; tabela de copy tem 27 linhas | Guardar com `fkind < PORT_VANILLA_FKIND_COUNT` ou pelo registry |
| 3 | ef/efmanager.c:6027 | `copy[ftGetStruct(fighter_gobj)->fkind].effect_scale` sem guarda | Idem item 2 |
| 4 | ft/ftcommon/ftcommonspecialair.c:172 | `dFTKirbySpecialAirNStatusList[copy_id]` sem guarda visivel (guarda so existe em specialn.c:94) | Clamp copy_id ou rotear por port_fighter_special_handler |
| 5 | ft/ftchar/ftkirby/ftkirbyspecialn.c:201 | `copy[victim_fp->fkind].copy_id` com victim sintetico | Clamp do victim fkind antes do indice |
| 6 | ft/ftmanager.c:96-100 e :187 | Loops ate `nFTKindEnumCount`; se o enum crescer, linhas 27..96 de `dFTManagerDataFiles` e `gSCManagerFighterFileSizes` ficam zeradas e `data->p_file_main` vira NULL | Iterar sobre a contagem do registry, nao sobre o enum |
| 7 | ft/ftmain.c:582 (ramo PORT) | `p_script[script_id][fkind]` sem checagem de faixa; struct de dimensao `nFTKindEnumCount` | Checar faixa e/ou usar registry; regenerar dados de dano se a struct mudar |
| 8 | lb/lbbackup.c:131-136; sc/sc1pmode/sc1pmanager.c:424-488; decomp/src/lb/lbtypes.h (fighter_mask u16) | `1 << fkind` em mascara de 16 bits; synth >= 16 faz shift invalido/truncado | Mascara maior (u8[13] ou u128) + migracao de save |
| 9 | sc/sc1pmode/sc1pmanager.c:219-280; mn/mndata/mnvsrecord.c:161-184,1751; mn/mnplayers/mnplayers1pbonus.c:969-1188 | `spgame_records[fkind]` / `vs_records[fkind]` (12 linhas, SRAM) indexados sem guarda por fkind de synth | Guardar fkind < 12 ou criar registro de save por fkind |
| 10 | mn/mnplayers/mnplayers1pgame.c:607,771,780,2228 (e pares dos outros 3 mnplayers) | Tabelas CSS de 12 linhas (`portraits`, `emblem_offsets`, `name_offsets`, `announce_names`, `sizes`) indexadas pelo fkind sem guarda; `sizes[fkind]` em 3244 fica no ramo #else | Ler pelo registry ou estender tabelas para 97 |
| 11 | mn/mndata/mncharacters.c:1838,1876,2241 | `special_motions`, `dMNCharactersAttack1MotionDescs`, `motion_names` (CSS, 12 linhas) sem guarda; `css_attack1_motion_descs` existe no registry mas sem accessor | Adicionar accessor e ramo PORT |

## Ganchos do registry (port/fighter_registry.h / .cpp)

- Estrutura: `FighterDescriptor` (port/fighter_registry.h) com ~45 campos; armazenamento `std::vector<std::unique_ptr<FighterDescriptor>>` (port/fighter_registry.cpp:13-46). Sem capacidade fixa; `port_fighter_register` redimensiona. Bound check em `get()`. Isso ja atende 97 sem mudar a estrutura.
- Seed: `port_fighter_seed_vanilla()` em decomp/src/ft/ftport.c:150-200, com `PORT_VANILLA_FKIND_COUNT 27` (ftport.c:67). Chamado de port/port.cpp:1170-1174. Preenche apenas 0..26; linhas 27..96 precisam de `port_fighter_register` no MOD_INIT.
- Gaps no seed: `css_motion_special = 0`; `css_attack1_motion_descs`, `kirby_hat_id`, `default_costumes`, `team_costume`, `charge_smash_frames`, `crowd_chant_fgm`, `remix_1p_end_bgm`, `costume_count`: nao preenchidos pelo seed (so por mod).
- Accessors definidos mas SEM call site no decomp/port (0 refs fora do header/cpp):
  - `port_fighter_descriptor`
  - `port_fighter_for_each` (o comentario diz que e usado por dimensionamento do figatree heap, mas ftmanager.c:96/187 nao o usam)
  - `port_fighter_action_string`
  - `port_fighter_charge_smash_frames`
  - `port_fighter_costume_count` (conta de LUTs por costume, relevante para if/ifcommon.c e mnvsresults.c)
  - `port_fighter_default_costumes`
  - `port_fighter_team_costume`
  - `port_fighter_crowd_chant_fgm`
  - `port_fighter_kirby_hat_id`
  - `port_fighter_remix_1p_end_bgm`
  - `port_set_results_emblem_base`: sem chamador. Consequencia: `port_fighter_results_emblem` sempre retorna 0 e o emblema de synth nunca e desenhado (mnvsresults.c:700-720).
- Accessors com call sites (para referencia): special_handler (8), scale (9), ecb_override (2), results_* (1 cada), css_spotlight_scale (4), data (5), register/seed (PORT only).
- Sem accessor nenhum (precisam de campo + accessor): `offset_add` (ftcommoncapturecaptain.c), tabelas CSS 12 linhas (portraits, emblem_offsets, name_offsets, announce_names, sizes, fkinds, pages, special_motions, dMNCharactersAttack1MotionDescs, motion_names), `copy[]` do Kirby (efmanager.c:5966/6027 e ftkirbyspecialn.c:201), `thrown_status`, `p_script` (ramo PORT sem accessor), `spgame_records`/`vs_records` (save).

## Tabelas por diretorio

Colunas: Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init (static/computed) | Tipo de mudanca | Guarda | Risco | Nota.
Tamanho "12" = tabela so de personagens jogaveis; "27" = contagem vanilla (fkind 0..26). "verify" = contagem nao confirmada por causa de `#ifdef`.

### ft/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| ft/ftdef.h:1138 | nFTKindEnumCount (enum) | 27 | - | - | static enum | DIM | - | M | Single source of truth; bumping it silently changes every [nFTKindEnumCount] array and loop below. Use a separate PORT_FKIND_CAPACITY for the 97 layout. |
| ft/ftdata.c:129 | dFTManagerDataFiles | nFTKindEnumCount+1 (28) | fkind | FTData* | static (27 rows + NULL) | INIT+REG | PORT-guarded readers (ftmanager.c:303,326,382,398,847 use port_fighter_data) | H | Rows 27..96 must come from port_fighter_register; the vanilla array itself cannot be grown without new rows. |
| ft/fttypes.h:693 | FTMotionDamageScript.p_script[2][nFTKindEnumCount] | 27 per script | fkind (2nd dim) | u32 (PORT) / void* | struct member (static data in relocData) | INIT | unguarded at ftmain.c:582 (PORT branch) | H | Struct size changes with the enum; relocated damage data baked per fighter. Needs regen of damage reloc data or a registry lookup. |
| ft/ftmain.c:86 | dFTMainSpecialStatusDescs | 27 (unsized, counter) | fkind (fp->fkind) | FTStatusDesc* | static | INIT+REG | PORT branch exists at ftmain.c:4858 (#else = vanilla); accessor port_fighter_special_descs (ftmain.c) | M | Row 27+ needs MOD_INIT registration; special_descs_count is 0 for vanilla rows. |
| ft/ftmain.c:4818 | D_ovl1_80390D20 (decl scsubsys/scsubsysdata.c:83) | 28 (static) | fp->fkind | FTOpeningDesc* | static | REG | PORT-guarded (#else at 4817 is vanilla); port_fighter_opening_descs | M | Table is in ovl1 data; no synth row without registry. |
| ft/ftmain.c:582 | p_script[script_id][fkind] (PORT branch) | [2][27] | fp->fkind (no range check) | u32 | static | REG | unguarded index in PORT branch | H | Reads OOB for fkind >= 27 when script token is consulted. |
| ft/ftmain.c:596,602 | p_script[script_id][fkind] (vanilla branch) | [2][27] | fp->fkind | void* | static | INIT | vanilla-only branch | L | Same struct as above; only non-PORT builds. |
| ft/ftcommon/ftcommonentry.c:14 | dFTCommonEntryAppearStatusIDs | [27][2] | fp->fkind | s32 | static | INIT+REG | PORT-guarded (ftcommonentry.c:215 #else); port_fighter_entry_appear | M | Synth rows must provide entry_appear_status[2]. |
| ft/ftcommon/ftcommonentry.c:20-40 | ftPortVanillaEntryMakeEffect switch (ftport.c:~68) | 27 cases | fp->fkind | - | switch | NONE | vanilla-only switch with default: | L | Synth falls to default (no effect); fine, but registry entry_make_effect covers it. |
| ft/ftcommon/ftcommonspecialn.c:14 | dFTKirbySpecialNStatusList | 27 (counter/awk) | fp->passive_vars.kirby.copy_id | fn ptr | static | INIT+REG | guard copy_id>=27 at ftcommonspecialn.c:94 (synth path); line 116 unguarded | M | Index is copy_id (copy power), not the Kirby fkind. |
| ft/ftcommon/ftcommonspecialn.c:46 | dFTCommonSpecialNStatusList | 27 | fp->fkind | fn ptr | static | INIT+REG | PORT-guarded (line 151 #else); port_fighter_special_handler | M | Needs 70 more handlers or registry rows. |
| ft/ftcommon/ftcommonspecialhi.c:14 | dFTCommonSpecialHiStatusList | 27 | fp->fkind | fn ptr | static | INIT+REG | PORT-guarded (line 74 #else) | M | Same as N list. |
| ft/ftcommon/ftcommonspeciallw.c:14 | dFTCommonSpecialLwStatusList | 27 | fp->fkind | fn ptr | static | INIT+REG | PORT-guarded (line 77 #else) | M | Same as N list. |
| ft/ftcommon/ftcommonspecialair.c:13 | dFTKirbySpecialAirNStatusList | 27 | fp->passive_vars.kirby.copy_id | fn ptr | static | INIT+REG | unguarded at line 172 (no PORT branch above) | H | Kirby copy_id past 26 calls OOB fn-pointer table entry. |
| ft/ftcommon/ftcommonspecialair.c:45 | dFTCommonSpecialAirNStatusList | 27 | fp->fkind | fn ptr | static | INIT+REG | PORT-guarded (line 225 #else) | M | - |
| ft/ftcommon/ftcommonspecialair.c:77 | dFTCommonSpecialAirHiStatusList | 27 | fp->fkind | fn ptr | static | INIT+REG | PORT-guarded (line 193 #else) | M | - |
| ft/ftcommon/ftcommonspecialair.c:109 | dFTCommonSpecialAirLwStatusList | 27 | fp->fkind | fn ptr | static | INIT+REG | PORT-guarded (line 207 #else) | M | - |
| ft/ftcommon/ftcommonspecialn.c:94 | copy_id >= nFTKindEnumCount guard (Kirby N) | - | copy_id | - | check | NONE | clamp present | L | Compares against nFTKindEnumCount: if the enum grows this guard silently stops routing 27..96 to registry. |
| ft/ftcommon/ftcommondownwaitbounce.c:109 | dFTCommonDataDownBounceSFX (decl ftcommondata.c:82) | 27 | fp->fkind | u16 | static | INIT+REG | PORT-guarded (line 109 #else); port_fighter_down_bounce_fgm | M | - |
| ft/ftcommondata.c:114 | dFTCommonDataPublicFighterCallFGMs (used ftpublic.c:101, ftport.c:193) | 27 | fkind | u16 | static | INIT+REG | PORT-guarded (ftpublic.c:101 #else); port_fighter_public_call_fgm | M | - |
| ft/ftcomputer.c:2719 | dFTComputerAttackList (use ftcomputer.c:3997) | 27 | this_fp->fkind | FTComputerAttack* | static | INIT+REG | PORT-guarded (#else); port_fighter_computer_attack_list | M | Synth needs its own attack list. |
| ft/ftcommon/ftcommoncaptureyoshi.c:15 | dFTCommonYoshiEggDamageCollDescs (use :321, ef/efmanager.c:5519) | 27 | fp->fkind | ftCommonYoshiEggDesc | static | INIT+REG | PORT-guarded; port_fighter_yoshi_egg_damage_coll | M | - |
| ft/ftcommon/ftcommoncapturecaptain.c:14-28 | offset_add (Vec2h, from gFTDataCaptainMainMotion reloc) | reloc (length unverified) | capture_fp->fkind | Vec2h | reloc data | INIT | unguarded line 27-28; NO accessor exists | H | Synth captured by Captain up-B reads past relocated table; add registry field and accessor. |
| ft/ftcommon/ftcommonthrow.c:107,116 | attr->thrown_status[catch_fp->fkind] | reloc [54] (dXxxMain_thrown_status[54]) | catch_fp->fkind (victim) | FTThrownStatus | reloc data | INIT | clamped by catch_vk (line 91) for >= nFTKindEnumCount | M | 54 rows < 97 needed; clamp already maps synths to a vanilla row. |
| ft/ftparam.c:43 | dFTParamSkeletonColAnimIDs | 27 | fp->fkind | s32 | static | INIT+REG | PORT-guarded (ftparam.c:1484 #else); port_fighter_skeleton_col_anim_base | M | - |
| ft/ftparam.c:81 | dFTParamCostumeIDs (FTCostume royal[4]/team[3]/develop) | ~27 (verify) | fkind | FTCostume | static | INIT+REG | PORT-guarded (2898 #else); port_fighter_costume_row | M | port_fighter_costume_count/team_costume/default_costumes have NO call sites. |
| ft/ftparam.c:2899,2913,2927 | ftParamGetCostumeCommonID / Team / Develop lookups | - | fkind | - | read | REG | #else guard (vanilla) | M | Same table as above. |
| ft/ftmanager.c:17 | FTKIRBY_COPY_TABLE_COUNT | 27 (define) | - | - | define | DIM | - | M | Kirby copy table has one row per inhalable fighter; 27 is hard-coded in comments and loops. |
| ft/ftmanager.c:96-100 | setup loop: i < nFTKindEnumCount, reads gSCManagerFighterFileSizes[i], dFTManagerDataFiles[i] | 27 | i | - | loop | DIM | unguarded (no PORT branch) | H | If enum grows, rows 27..96 are NULL/0 in the arrays: dereference of data->p_file_main at 103+ crashes. Must iterate the registry count instead. |
| ft/ftmanager.c:187 | figatree-heap loop: i < (nFTKindEnumCount + ARRAY_COUNT(gSCManagerFighterFileSizes))/2 | 27 (computed) | i | - | loop | DIM | unguarded | H | Same as above; port_fighter_for_each exists but has no call site. |
| ft/ftmanager.c:26 (+ scmanager.c:66) | gSCManagerFighterFileSizes[nFTKindEnumCount] | 27 | fkind (loop index) | FTFileSize | static zero | INIT | - | H | Must grow to registry capacity; zero-init for 27..96 unless registry fills it. |
| ft/ftmanager.c:303,326,382,398,847 | dFTManagerDataFiles[fkind] (vanilla branch) | 28 | fkind | FTData* | static | REG | PORT branch uses port_fighter_data | M | Vanilla branch only; reads OOB without PORT. |
| ft/ftmanager.c:744-753 | FTKirbyCopy byteswap loop | FTKIRBY_COPY_TABLE_COUNT (27) | i | u16 | loop | DIM | PORT-guarded (#if PORT) | M | Comment documents the 27 cap; copy rows exist only for vanilla. |
| ft/ftmanager.c:765-783 | copy[copy_id].copy_modelpart_id; copy[nFTKindKirby] | 27 | copy_id (clamped <27) | s16 | reloc | REG | PORT branch clamps copy_id; synth uses port_kirby_get_pending_hat | L | Already designed for synths. |
| ft/ftport.c:67,154-190 | PORT_VANILLA_FKIND_COUNT (27) seed loop + desc fields | 27 | fkind | FighterDescriptor | computed seed | REG | seed only; synth rows via port_fighter_register | M | Seed only fills 0..26; the 97 layout needs MOD_INIT (or table) rows for 27..96. |
| ft/ftport.c:189 | desc.scale = fkind <= nFTKindPlayableEnd ? dSCSubsysFighterScales[fkind] : 1.0f | 12 (scales) | fkind | f32 | computed | REG | guarded to playable | L | Only correct while nFTKindPlayableEnd stays 11. |
| ft/ftchar/ftkirby/ftkirbyspecialn.c:133 | copy[copy_id].copy_modelpart_id | 27 | copy_id | FTKirbyCopy | reloc | INIT | guard present at line 118 (#endif, confirm) | M | Synth copy_id yields OOB unless guarded at call site. |
| ft/ftchar/ftkirby/ftkirbyspecialn.c:201 | copy[victim_fp->fkind].copy_id | 27 | victim fkind | FTKirbyCopy | reloc | INIT | unguarded | H | Kirby inhaling a synth victim reads OOB copy row. |

### ef/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| ef/efmanager.c:5519 | dFTCommonYoshiEggDamageCollDescs[fp->fkind] | 27 | fkind | struct | static | REG | PORT-guarded (#else at 5518) | M | - |
| ef/efmanager.c:5966 | copy[fp->fkind].effect_scale | 27 | fp->fkind | FTKirbyCopy | reloc | INIT | unguarded | H | Synth fighter's copy effect read OOB. |
| ef/efmanager.c:6027 | copy[ftGetStruct(fighter_gobj)->fkind].effect_scale | 27 | fkind | FTKirbyCopy | reloc | INIT | unguarded | H | Same as 5966. |

### mn/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| mn/mnplayers/mnplayers1pgame.c:581-607 | fkinds[] (CSS slot->fkind), portraits[fkind] (return at 607) | 12 | CSS slot / fkind | s32 | static | CSS | unguarded | M | Playable-only (12); synth fkind = OOB. |
| mn/mnplayers/mnplayers1pgame.c:748-780 | emblem_offsets[fkind], name_offsets[fkind] | 12 | fkind | intptr_t | static | CSS | unguarded | M | Need registry offsets or CSS extension. |
| mn/mnplayers/mnplayers1pgame.c:1830-1832 | dSCSubsysFighterScales[fkind] | 12 | fkind | f32 | static | REG | PORT-guarded (#else at 1829); port_fighter_scale | M | Vanilla branch only. |
| mn/mnplayers/mnplayers1pgame.c:2193,2228 | announce_names[sMNPlayers1PGameSlot.fkind] | 12 | fkind | u16 | static | CSS | unguarded at 2228 | M | Synth announce: needs port_fighter_results_announce_fgm-like accessor. |
| mn/mnplayers/mnplayers1pgame.c:3222,3244-3246 | sizes[fkind] | 12 | fkind | f32 | static | CSS | guarded? (3243 #else) vanilla branch | M | - |
| mn/mnplayers/mnplayers1pgame.c:1502,1539,1558,1362 | spgame_records[fkind]; _luts[fp->costume] | 12 (save); costume | - | - | - | SAVE/COSTUME | unguarded | H | Save record access by synth fkind; costume LUT indexed by costume. |
| mn/mnplayers/mnplayers1ptraining.c:297-323 | fkinds[], portraits[fkind] | 12 | CSS slot / fkind | s32 | static | CSS | unguarded | M | Same as 1pgame. |
| mn/mnplayers/mnplayers1ptraining.c:463-506 | emblem_offsets[fkind], name_offsets[fkind] | 12 | fkind | intptr_t | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayers1ptraining.c:1206-1208 | dSCSubsysFighterScales[fkind] | 12 | fkind | f32 | static | REG | #else vanilla branch (port_fighter_scale used elsewhere in file) | M | - |
| mn/mnplayers/mnplayers1ptraining.c:1497,1532 | announce_names[slot fkind] | 12 | fkind | u16 | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayers1ptraining.c:2825 | sizes[fkind] | 12 | fkind | f32 | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayers1pbonus.c:440-466 | fkinds[], portraits[fkind] | 12 | CSS slot / fkind | s32 | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayers1pbonus.c:611-643 | emblem_offsets[fkind], name_offsets[fkind] | 12 | fkind | intptr_t | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayers1pbonus.c:1416-1418 | dSCSubsysFighterScales[fkind] | 12 | fkind | f32 | static | REG | vanilla branch | M | - |
| mn/mnplayers/mnplayers1pbonus.c:1727,1762 | announce_names[fkind] | 12 | fkind | u16 | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayers1pbonus.c:2616,2638 | sizes[fkind] | 12 | fkind | f32 | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayers1pbonus.c:969-1188 | spgame_records[fkind] (bonus times/counts) | 12 (save) | fkind | - | - | SAVE | unguarded | H | Save record by synth fkind. |
| mn/mnplayers/mnplayersvs.c:367-397 | fkinds[], portraits[fkind] | 12 | CSS slot / fkind | s32 | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayersvs.c:629-670 | emblem_offsets[fkind], name_offsets[fkind] | 12 | fkind | intptr_t | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayersvs.c:1927-1929 | dSCSubsysFighterScales[fkind] | 12 | fkind | f32 | static | REG | vanilla branch (#else at 1926) | M | - |
| mn/mnplayers/mnplayersvs.c:2896,2925 | announce_names[slot fkind] | 12 | fkind | u16 | static | CSS | unguarded | M | - |
| mn/mnplayers/mnplayersvs.c:4467 | sizes[fkind] | 12 | fkind | f32 | static | CSS | unguarded | M | - |
| mn/mndata/mncharacters.c:1358-1381 | fkinds[] (12), pages[fkind] (12) | 12 | fkind | s32 | static | CSS | unguarded | M | Page table sized to playable. |
| mn/mndata/mncharacters.c:1398-1450 | offsets[fkind] (sprite) | 12 | fkind | intptr_t | static | CSS | unguarded | M | - |
| mn/mndata/mncharacters.c:1546-1605 | dobjdescs[fkind], mobjsubs[fkind], matanim_joints[fkind] (PORT list = 12 entries; vanilla branch lists 12 too) | 12 | fkind | intptr_t | static | CSS | unguarded (ifdef only selects element form) | M | Emblem models for playable only. |
| mn/mndata/mncharacters.c:1640-1747 | offsets[fkind] / pos[fkind] | 12 (pos size unverified) | fkind | - | static | CSS | unguarded | M | Verify pos[] length. |
| mn/mndata/mncharacters.c:1803-1805 | dSCSubsysFighterScales[fkind] | 12 | fkind | f32 | static | REG | #else vanilla branch | M | - |
| mn/mndata/mncharacters.c:1826-1838 | special_motions[fkind]->motions | 12 | fkind | ptr | static | CSS | unguarded | M | - |
| mn/mndata/mncharacters.c:1066-1876 | dMNCharactersAttack1MotionDescs[fkind][8] (also CSS attack1 in registry css_attack1_motion_descs, no accessor) | 12 x 8 | fkind | MNCharactersMotion | static | CSS+REG | unguarded | M | Registry has field but no accessor/call site. |
| mn/mndata/mncharacters.c:2177,2241 | motion_names[fkind][motion_kind][3] | 12 x 3 (verify) | fkind | intptr_t | static | CSS | unguarded | M | - |
| mn/mnvsmode/mnvsresults.c:306,360 | announce_names[wk] (vanilla < 27), port_fighter_results_announce_fgm (>=27) | 12 | wk | u32 | static | REG | guarded at >= nFTKindEnumCount | M | Vanilla 12..26 unguarded but unreachable in VS. |
| mn/mnvsmode/mnvsresults.c:652-726 | dobjdescs/mobjsubs/matanim_joints[win_fkind] | 12 | win_fkind | intptr_t | static | REG | guarded at line 700 (>= nFTKindEnumCount) | M | Guard compares against nFTKindEnumCount (see DIM note). |
| mn/mnvsmode/mnvsresults.c:1041-1043 | dSCSubsysFighterScales[fkind] | 12 | fkind | f32 | static | REG | vanilla branch | M | - |
| mn/mnvsmode/mnvsresults.c:1098-1123 | pos_y_kinds[fkind][4] | 12 x 4 | fkind | f32 | static | REG | ternary guard >= nFTKindEnumCount | M | - |
| mn/mnvsmode/mnvsresults.c:1336-1380 | x_fkinds[winner], wlx | 12 | winner | f32 | static | REG | guarded | M | - |
| mn/mnvsmode/mnvsresults.c:1409-1493 | names[fkind], pos_x[fkind], scales[fkind] | 12/13/14 (verify) | fkind | char*/f32 | static | REG | guarded at 1485 (>= 27 -> port_fighter_results_name) | M | Vanilla path unguarded for 12..26. |
| mn/mnvsmode/mnvsresults.c:2008-2012 | _luts[fp->costume] | costume | costume | - | - | COSTUME | unguarded | L | - |
| mn/mncommon/mncongra.c:24,273-279 | dMNCongraPictures[] (12) indexed by sMNCongraFighterKind | 12 | fkind | MNCongraPicture | static | REG | guarded at 279 (>= nFTKindEnumCount) | M | Guard uses nFTKindEnumCount (DIM note). |
| mn/mndata/mnvsrecord.c:92-98 | sMNVSRecordBattleScoreFighterKinds / RankingFighterKindOrder / IndivFighterKinds [(PlayableEnd-PlayableStart)+1] | 12 (computed) | index i | s32 | computed | SAVE | - | M | Playable-only tables; sized from enum, so they follow enum range. |
| mn/mndata/mnvsrecord.c:161-184,1458-1497,1751 | vs_records[fkind].* | 12 (save) | fkind | - | - | SAVE | unguarded | H | Save record access. |
| mn/mndata/mnvsrecord.c:133,268-1857 | fkinds[], offsets[fkind], rank[fkinds[i]] etc | 12 (rank/stats via enum-derived sizes) | fkind | s32 | static/computed | CSS | unguarded | M | - |
| mn/mnvsmode/mnvsitemswitch.c | nITKind arrays (not fighter) | - | - | - | - | NONE | - | L | Not fighter-indexed. |
| mn/mnmaps/mnmaps.c | gkind-indexed tables (positions, offsets, subtitles) | stage count | gkind (stage) | - | static | NONE | - | L | Stage kind, out of scope for fighter count. |
| mn/mncommon/mnmessage.c:313-324 | fkinds[] (4) and fighter_mask OR with 1 << fkinds[id] | 4 | id | s32 | static | SAVE | unguarded | H | Unlock sets a bit in the u16 fighter_mask. |

### mv/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| mv/mvopening/mvopeningroom.c:695-731 | translates[fkind], scales[fkind] (fkind from random fkinds[8]) | unverified (~27?) | fkind | Vec3f | static | INIT | unguarded | M | Opening room only uses 8 fkinds; verify table length. |
| mv/mvopening/mvopeningroom.c:1194-1228 | fkinds[8] random pick | 8 | rand index | s32 | static | NONE | - | L | Pick list only, not fkind-indexed. |

### sc/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| sc/scsubsys/scsubsysdata.c:83 | D_ovl1_80390D20 | 28 | fkind | FTOpeningDesc* | static | INIT+REG | see ftmain.c:4818 | M | Duplicate of ft row. |
| sc/scsubsys/scsubsysdata.c:115 | dSCSubsysFighterScales | 12 | fkind | f32 | static | INIT+REG | used by 10 call sites (mn/sc), most unguarded | H | Synth/vanilla boss kinds (>=12) read OOB; port_fighter_scale exists (9 refs). |
| sc/scmanager.c:66 | gSCManagerFighterFileSizes | 27 | fkind | FTFileSize | static zero | INIT | - | H | See ft/ftmanager.c:26. |
| sc/sc1pmode/sc1pintro.c:668-702 | name_offsets[fkind] (counter 23, verify) | 12/23 (verify) | fkind | intptr_t | static | CSS | unguarded | M | - |
| sc/sc1pmode/sc1pintro.c:1266-1694 | cobj_descs[fkind][cobj_id] (cobj_descs[][6]) | 12 | fkind | CObjDesc | static | CSS | unguarded | M | - |
| sc/sc1pmode/sc1pintro.c:1324-1782 | camanim_joints[fkind] | 12-24 (verify) | fkind | intptr_t | static | CSS | unguarded | M | - |
| sc/sc1pmode/sc1pintro.c:1958-2020 | fighter_voices[fkind] (12) | 12 | fkind | u32 | static | CSS | unguarded | M | - |
| sc/sc1pmode/sc1pintro.c:1994-2013 | announce_wait_tics[fkind] | 12 | fkind | u32 | static | CSS | guarded (ARRAY_COUNT ? : 0 at 2011) | L | Only one guarded site; 2013 is same table unguarded. |
| sc/sc1pmode/sc1pintro.c:1458,2131 | fkinds[] (14 items) | 14 | index | s32 | static | NONE | - | L | Pick lists. |
| sc/sc1pmode/sc1pintro.c:1515 | sSC1PIntroFigatreeHeaps[i - nFTKindNStart + 1] | 32 (heap slots) | i | void* | static | DIM | - | L | Heap array by slot, not fkind; SC1PIntroFigatreeHeap[32]. |
| sc/sc1pmode/sc1pchallenger.c:271-273 | dSCSubsysFighterScales[fkind] | 12 | fkind | f32 | static | REG | #else (line 270) | M | - |
| sc/sc1pmode/sc1pmanager.c:219-280 | spgame_records[fkind] (save) | 12 | fkind | - | - | SAVE | unguarded | H | Save record access by synth fkind. |
| sc/sc1pmode/sc1pmanager.c:424-488 | 1<<fkind; fighter_mask (u16) | 16-bit mask | fkind | - | - | SAVE | unguarded | H | Shift of >= 16 truncates or is UB. |
| sc/sc1pmode/sc1pbonusstage.c:586-1296 | spgame_records[gSCManagerSceneData.bonus_fkind / fkind] | 12 (save) | fkind | - | - | SAVE | unguarded | H | - |
| sc/sc1pmode/sc1pgame.c:1283 | sSC1PGameEnemyVariations[costume] | costume | costume | - | - | COSTUME | unguarded | L | - |
| sc/sc1pmode/sc1pgame.c:2421 | dFTManagerDataFiles[nFTKindNess] | constant | - | - | - | NONE | - | L | Constant index. |
| sc/sccommon/scautodemo.c:526-541 | local u16 fighter_mask; scAutoDemoGetShuffledFighterKind(mask) | 16-bit mask | fkind | u16 | computed | SAVE | - | M | Mask helper limited to 16 fighters. |
| sc/sccommon/scautodemo.c:567 | dSCAutoDemoGroundOrder[demo_gkind_order] | stage | - | - | static | NONE | - | L | Stage kind. |

### gr/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| gr/grmainsetup.c:35 | dGRMainSetupProcMakeList[gkind] | stage | gkind | - | static | NONE | - | L | Stage kind; out of scope. |

### ef/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| ef/efground.c:1821-1826 | dEFGroundDatas[gkind] | stage | gkind | - | static | NONE | - | L | Stage kind. |

### lb/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| lb/lbtypes.h:228-237 | ko_count, player_count_tallies, played_against [GMCOMMON_FIGHTERS_PLAYABLE_NUM=12] | 12 | fkind | u16 | struct | SAVE | - | H | Save struct layout; growing changes sizeof(LBBackupData) and the checksum. |
| lb/lbtypes.h:256,267 | vs_records[12], spgame_records[12] (LBBackupVSRecord, LBBackup1PRecord) | 12 | fkind | struct | - | SAVE | - | H | Persistent SRAM; needs save-version migration. |
| lb/lbtypes.h:262-270 (fighter_mask u16, characters_fkind u8) | fighter_mask 16-bit | bit per fkind | fkind | u16 | struct | SAVE | - | H | Bitmask cannot hold 97 fighters. |
| lb/lbdef.h:105-133 | LBBACKUP_MASK_FIGHTER(kind)=(1<<(kind)); CHARACTER_MASK_ALL/UNLOCK/STARTER | bits 0..11 | fkind | int | macro | SAVE | - | H | Shift up to 96 is UB on 32-bit int. |
| lb/lbbackup.c:84-88 | lbBackupFighterIsLocked: fkind < GMCOMMON_FIGHTERS_PLAYABLE_NUM guard, then 1<<fkind | 12 | fkind | u32 | fn | SAVE | clamped to playable | M | Synths are never locked (returns FALSE). |
| lb/lbbackup.c:131-136 | 1<<players[i].fkind & fighter_mask (no range check) | 16-bit | fkind | - | - | SAVE | unguarded | H | Synth transfer state can shift past 31. |

### sys/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| sys/netpeer.c:454, sys/netreplay.c:109,158 | metadata->fighter_kinds[player] | 4 players | player | u8 | struct | NONE | - | L | Per-player, not fkind-sized. |
| sys/unref.c:8 | arg0[0x1B] = arg4 | 27 (literal) | - | - | unreferenced | NONE | - | L | Literal 0x1B coincidence in unreferenced code; not verified as fighter-related. |

### gm/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| gm/* | no fkind-indexed arrays found (gmcollision attack_id only) | - | - | - | - | NONE | - | L | - |

### it/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| it/* | no fkind-indexed arrays found (monster_id, attack ids) | - | - | - | - | NONE | - | L | - |

### wp/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| wp/* | no fkind-indexed arrays found | - | - | - | - | NONE | - | L | - |

### if/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| if/ifcommon.c:1113-1404 | _luts[fp->costume], stock_luts[fp->costume] | costume | costume | - | - | COSTUME | unguarded | L | Costume LUT count is per-fighter (registry costume_count, no call site). |
| if/ifcommon.c:653 | fkind == nFTKindBoss compare | - | - | - | - | NONE | - | L | - |

### port/

| Local | Simbolo / tabela | Tamanho | Indexado por | Tipo elem. | Init | Tipo | Guarda | Risco | Nota |
|---|---|---|---|---|---|---|---|---|---|
| port/fighter_registry.h/.cpp | vector-backed registry (sRegistry); no fixed capacity | dynamic | fkind | FighterDescriptor* | runtime | REG | - | L | Already capacity-free; no bound check on fkind > INT_MAX; get() bounds-checks. |
| port/port.cpp:106-110,1170-1174 | seed + comments referencing nFTKindEnumCount | 27 | - | - | computed | REG | - | L | Comment-only references; update when the layout changes. |
| port/hooks/list/FighterEvent.h:31-76 | int32 fkind / resolved_fkind event fields | dynamic | fkind | int32 | - | NONE | - | L | - |
| port/enhancements/*, port/css_icons/*, port/bridge/* | no fkind-indexed arrays (css_icons index gkind = stage) | - | - | - | - | NONE | - | L | - |

## Observacoes para a execucao

- Crescer `nFTKindEnumCount` de 27 para 97 e a mudanca mais perigosa: (a) o registry usa `>= nFTKindEnumCount` como criterio de synth em varios lugares (mncongra.c:279, mnvsresults.c:355/700/1112/1380/1485, ftcommonspecialn.c:94, ftcommonthrow.c:91, ftmain.c:4843, mnplayers* e port.cpp:106); com 97 o criterio passa a ser "fkind >= 97", e fighters 27..96 caem nos ramos vanilla. Trocar esses criterios por `PORT_VANILLA_FKIND_COUNT` (ou uma constante de vanilla) antes de crescer o enum; (b) arrays `[nFTKindEnumCount]` crescem sozinhos, mas os inicializadores nao, e as linhas faltantes ficam zeradas sem erro de compilacao.
- Os inicializadores de 27 linhas sao o gargalo principal: 70 novas entradas por tabela x ~25 tabelas.
- Dados de dano (`p_script`) e copy do Kirby vem de relocData gerado; crescer exige regerar esses arquivos.
- Save: `LBBackupData` (lb/lbtypes.h:256-270) tem checksum e e gravado em SRAM por DMA (lb/lbbackup.c:17-50). Mudar `vs_records`/`spgame_records`/`fighter_mask` altera o layout; exige versao de save e migracao.
- Fora de escopo: indices `gkind` (estagios: mnmaps, ef/efground, mp/mpcollision, gr/grmainsetup, port/css_icons/port_css_stage_assets.cpp) e `nITKind`/`nMPLineKind`, que nao sao contagem de fighters.
- Limitacao: contagens de inicializadores vieram de um script de contagem por chaves/virgulas e de leitura manual; onde marcado "verify", confirmar antes de decidir o tamanho final. Um teste de compilacao com `static_assert(sizeof(tabela)/sizeof(*tabela) == nFTKindEnumCount)` nos arquivos de tabela pegaria os inicializadores incompletos.
