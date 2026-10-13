# Smash Remix: como novos fighters são adicionados

Documento de referência para o port. Somente leitura sobre o código do Remix; nada foi compilado nem executado.

## Escopo, convenções e limites

- Raiz do Remix analisada (`R`): `/Users/lucas/Projetos/Pessoal/BattleShip/.toolchain/pkgs/smashremix-src` (commit `5e04fe7`, "Smash Remix 2.0.1"). Todas as citações `src/...:linha` são relativas a `R`.
- Decomp (`D`): `/Users/lucas/Projetos/Pessoal/BattleShip/decomp`. Citações `decomp/...` são relativas a `D`.
- Endereços de ROM (`origin` no bass) são offsets de arquivo do `roms/original.z64` (US). Endereços RAM são os `base` do bass ou os da chamada `OS.patch_start(ROM, RAM)`.
- O build (`patch.bat:1`) gera `logfile.log` com `bass -sym`, mas esse log não está no repositório. Por isso, os endereços RAM finais das tabelas **novas** não são conhecidos; só os offsets simbólicos. Os endereços RAM das tabelas **vanilla** foram tirados do próprio código (quando citados) ou derivados (indicado).
- O mapeamento RAM -> função do decomp foi feito só por endereço (comentários `// 0x8XXXXXXX` antes das funções em `decomp/src`, 5679 funções anotadas). Na faixa `0x8010xxxx`–`0x8013xxxx` o SSB64 usa overlays que compartilham endereços; nesses casos o nome do decomp pode ser de outro overlay. Ver a seção 5.

## 1. Numeração de fkind (IDs de fighter)

### 1.1 Vanilla (0x00–0x1A)

A numeração vanilla está no `scope id` (`src/Character.asm:2315-2362`). Os IDs são os mesmos do decomp (`decomp/src/ft/ftdef.h:1102-1139`, `nFTKind*`), exceto que o decomp tem 27 entradas + `nFTKindNull`, e o Remix usa 0x1B e 0x1C para dois slots extras.

| ID hex | ID dec | Nome Remix (`id.X`) | Observação |
|---|---|---|---|
| 0x00 | 0 | MARIO | |
| 0x01 | 1 | FOX | |
| 0x02 | 2 | DONKEY (alias DK) | |
| 0x03 | 3 | SAMUS | |
| 0x04 | 4 | LUIGI | |
| 0x05 | 5 | LINK | |
| 0x06 | 6 | YOSHI | |
| 0x07 | 7 | CAPTAIN (alias CAPTAIN_FALCON, FALCON) | Captain Falcon |
| 0x08 | 8 | KIRBY | |
| 0x09 | 9 | PIKACHU (alias PIKA) | |
| 0x0A | 10 | JIGGLY (alias JIGGLYPUFF, PUFF) | |
| 0x0B | 11 | NESS | Último ID de personagem jogável vanilla. `Character.asm:54` só aceita parent `<= 0xB`. |
| 0x0C | 12 | BOSS | Master Hand (`variant_type.SPECIAL`, `src/Character.asm:2603-2632`) |
| 0x0D | 13 | METAL | Metal Mario (`define_character(METAL)`, `src/Character.asm:3558`) |
| 0x0E | 14 | NMARIO | Polygon Mario (vanilla) |
| 0x0F | 15 | NFOX | Polygon Fox |
| 0x10 | 16 | NDONKEY (alias NDK) | Polygon DK |
| 0x11 | 17 | NSAMUS | Polygon Samus |
| 0x12 | 18 | NLUIGI | Polygon Luigi |
| 0x13 | 19 | NLINK | Polygon Link |
| 0x14 | 20 | NYOSHI | Polygon Yoshi |
| 0x15 | 21 | NCAPTAIN (alias NFALCON) | Polygon Captain Falcon |
| 0x16 | 22 | NKIRBY | Polygon Kirby |
| 0x17 | 23 | NPIKACHU (alias NPIKA) | Polygon Pikachu |
| 0x18 | 24 | NJIGGLY (alias NPUFF) | Polygon Jigglypuff |
| 0x19 | 25 | NNESS | Polygon Ness |
| 0x1A | 26 | GDONKEY (alias GDK) | Giant DK (`variant_type.SPECIAL`, `src/Character.asm:2603-2632`) |
| 0x1B | 27 | PLACEHOLDER (no código, o fighter RANDOM) | `src/Character.asm:2356`; RANDOM é o primeiro `define_character` (`:3355`) |
| 0x1C | 28 | NONE (no código, clone de Mario não selecionável) | `src/Character.asm:2357`; criado em `:3357-3362` com `dw 0x80117810` (struct de Mario, ver `decomp/src/ft/ftdata.c:556-557`) |

Os ID 0x0D (METAL) e 0x0E–0x19 (polygons vanilla) são criados com `define_character(name)` de forma curta (`src/Character.asm:36-51`), que só lê a struct, os arrays e o action array do ID já existente.

### 1.2 Fighters adicionados (0x1B, 0x1D–0x60)

Cada `define_character(name, parent, ...)` completo recebe o próximo ID. Regra: `constant id.{name}((STRUCT_TABLE_END - STRUCT_TABLE) / 0x4)` (`src/Character.asm:79-80`). Como `STRUCT_TABLE_END` começa em `STRUCT_TABLE + 0x6C` (27 entradas, `src/Character.asm:19-20`) e cada define adiciona uma entrada de 4 bytes (`:217-220`), a ordem de chamada no arquivo define o ID. O clone de Mario (`:3357-3362`) ocupa o 0x1C, por isso FALCO é 0x1D e não 0x1C.

Os comentários `// 0xNN - NOME` do arquivo batem com essa numeração (`src/Character.asm:3354-3815`); conferido linha a linha.

Total: `NUM_CHARACTERS = NUM_VANILLA_CHARACTERS(27) + 2 + ADD_CHARACTERS(68) = 97` (0x61) (`src/Character.asm:15, 17, 27`). Os 68 de `ADD_CHARACTERS` são FALCO..NCRASH (0x1D–0x60); RANDOM e o clone são os "+2".

Os 69 fighters definidos com `define_character(nome, parent, ...)` (todos em `src/Character.asm`, linhas 3355–3815):

| ID hex | ID dec | Nome | Parent | Linha | Arquivo main (`File.*`) | attrib (0x60) | variant_type |
|---|---|---|---|---|---|---|---|
| 0x1B | 27 | RANDOM | CAPTAIN | 3355 | File.RANDOM_MAIN | 0x2A0 | SPECIAL |
| 0x1D | 29 | FALCO | FOX | 3677 | File.FALCO_MAIN | 0x474 | NA |
| 0x1E | 30 | GND | CAPTAIN | 3679 | File.GND_MAIN | 0x48C | NA |
| 0x1F | 31 | YLINK | LINK | 3681 | File.YLINK_MAIN | 0x760 | NA |
| 0x20 | 32 | DRM | MARIO | 3683 | File.DRM_MAIN | 0x4A4 | NA |
| 0x21 | 33 | WARIO | MARIO | 3685 | File.WARIO_MAIN | 0x51C | NA |
| 0x22 | 34 | DSAMUS | SAMUS | 3687 | File.DSAMUS_MAIN | 0x6BC | SPECIAL |
| 0x23 | 35 | ELINK | LINK | 3689 | File.ELINK_MAIN | 0x708 | E |
| 0x24 | 36 | JSAMUS | SAMUS | 3691 | 0x0D9 | 0x610 | J |
| 0x25 | 37 | JNESS | NESS | 3693 | File.JNESS_MAIN | 0x5BC | J |
| 0x26 | 38 | LUCAS | NESS | 3695 | File.LUCAS_MAIN | 0x618 | SPECIAL |
| 0x27 | 39 | JLINK | LINK | 3697 | File.JLINK_MAIN | 0x708 | J |
| 0x28 | 40 | JFALCON | CAPTAIN | 3699 | File.JFALCON_MAIN | 0x488 | J |
| 0x29 | 41 | JFOX | FOX | 3701 | File.JFOX_MAIN | 0x46C | J |
| 0x2A | 42 | JMARIO | MARIO | 3703 | File.JMARIO_MAIN | 0x428 | J |
| 0x2B | 43 | JLUIGI | LUIGI | 3705 | File.JLUIGI_MAIN | 0x580 | J |
| 0x2C | 44 | JDK | DONKEY | 3707 | File.JDK_MAIN | 0x4A4 | J |
| 0x2D | 45 | EPIKA | PIKACHU | 3709 | File.EPIKA_MAIN | 0x41C | E |
| 0x2E | 46 | JPUFF | JIGGLYPUFF | 3711 | File.JPUFF_MAIN | 0x474 | J |
| 0x2F | 47 | EPUFF | JIGGLYPUFF | 3713 | File.EPUFF_MAIN | 0x474 | E |
| 0x30 | 48 | JKIRBY | KIRBY | 3715 | File.JKIRBY_MAIN | 0x808 | J |
| 0x31 | 49 | JYOSHI | YOSHI | 3717 | File.JYOSHI_MAIN | 0x47C | J |
| 0x32 | 50 | JPIKA | PIKACHU | 3719 | File.JPIKA_MAIN | 0x41C | J |
| 0x33 | 51 | ESAMUS | SAMUS | 3721 | File.ESAMUS_MAIN | 0x610 | E |
| 0x34 | 52 | BOWSER | YOSHI | 3723 | File.BOWSER_MAIN | 0x47C | NA |
| 0x35 | 53 | GBOWSER | YOSHI | 3725 | File.GBOWSER_MAIN | 0x47C | SPECIAL |
| 0x36 | 54 | PIANO | MARIO | 3727 | File.PIANO_MAIN | 0x42C | SPECIAL |
| 0x37 | 55 | WOLF | FOX | 3729 | File.WOLF_MAIN | 0x4C4 | NA |
| 0x38 | 56 | CONKER | FOX | 3731 | File.CONKER_MAIN | 0x5DC | NA |
| 0x39 | 57 | MTWO | YOSHI | 3733 | File.MTWO_MAIN | 0x548 | NA |
| 0x3A | 58 | MARTH | CAPTAIN | 3735 | File.MARTH_MAIN | 0x524 | NA |
| 0x3B | 59 | SONIC | FOX | 3737 | File.SONIC_MAIN | 0x58C | NA |
| 0x3C | 60 | SANDBAG | CAPTAIN | 3739 | File.SANDBAG_MAIN | 0x488 | SPECIAL |
| 0x3D | 61 | SSONIC | FOX | 3741 | File.SSONIC_MAIN | 0x58C | SPECIAL |
| 0x3E | 62 | SHEIK | CAPTAIN | 3743 | File.SHEIK_MAIN | 0x4B0 | NA |
| 0x3F | 63 | MARINA | CAPTAIN | 3745 | File.MARINA_MAIN | 0x560 | NA |
| 0x40 | 64 | DEDEDE | CAPTAIN | 3747 | File.DEDEDE_MAIN | 0x5A4 | NA |
| 0x41 | 65 | GOEMON | MARIO | 3750 | File.GOEMON_MAIN | 0x91C | NA |
| 0x42 | 66 | PEPPY | FOX | 3752 | File.PEPPY_MAIN | 0x474 | SPECIAL |
| 0x43 | 67 | SLIPPY | FOX | 3754 | File.SLIPPY_MAIN | 0x4AC | SPECIAL |
| 0x44 | 68 | BANJO | CAPTAIN | 3756 | File.BANJO_MAIN | 0x898 | NA |
| 0x45 | 69 | MLUIGI | LUIGI | 3758 | File.METAL_LUIGI_MAIN | 0x418 | SPECIAL |
| 0x46 | 70 | EBI | MARIO | 3760 | File.EBISUMARU_MAIN | 0x9C0 | SPECIAL |
| 0x47 | 71 | DRAGONKING | CAPTAIN | 3762 | File.DRAGONKING_MAIN | 0x5F4 | SPECIAL |
| 0x48 | 72 | CRASH | MARIO | 3764 | File.CRASH_MAIN | 0x5CC | NA |
| 0x49 | 73 | PEACH | FOX | 3766 | File.PEACH_MAIN | 0x59C | NA |
| 0x4A | 74 | ROY | CAPTAIN | 3768 | File.ROY_MAIN | 0x4D4 | SPECIAL |
| 0x4B | 75 | DRL | LUIGI | 3770 | File.DRL_MAIN | 0x568 | SPECIAL |
| 0x4C | 76 | LANKY | MARIO | 3772 | File.LANKY_MAIN | 0x580 | SPECIAL |
| 0x4D | 77 | NWARIO | MARIO | 3777 | File.NWARIO_MAIN | 0x2B0 | POLYGON |
| 0x4E | 78 | NLUCAS | NESS | 3779 | File.NLUCAS_MAIN | 0x308 | POLYGON |
| 0x4F | 79 | NBOWSER | YOSHI | 3781 | File.NBOWSER_MAIN | 0x2D0 | POLYGON |
| 0x50 | 80 | NWOLF | FOX | 3783 | File.NWOLF_MAIN | 0x2BC | POLYGON |
| 0x51 | 81 | NDRM | MARIO | 3785 | File.NDRM_MAIN | 0x2B0 | POLYGON |
| 0x52 | 82 | NSONIC | FOX | 3787 | File.NSONIC_MAIN | 0x30C | POLYGON |
| 0x53 | 83 | NSHEIK | CAPTAIN | 3789 | File.NSHEIK_MAIN | 0x2B4 | POLYGON |
| 0x54 | 84 | NMARINA | CAPTAIN | 3791 | File.NMARINA_MAIN | 0x2B8 | POLYGON |
| 0x55 | 85 | NFALCO | FOX | 3793 | File.NFALCO_MAIN | 0x2BC | POLYGON |
| 0x56 | 86 | NGND | CAPTAIN | 3795 | File.NGND_MAIN | 0x32C | POLYGON |
| 0x57 | 87 | NDSAMUS | SAMUS | 3797 | File.NDSAMUS_MAIN | 0x3D4 | POLYGON |
| 0x58 | 88 | NMARTH | CAPTAIN | 3799 | File.NMARTH_MAIN | 0x2CC | POLYGON |
| 0x59 | 89 | NMTWO | YOSHI | 3801 | File.NMTWO_MAIN | 0x2D4 | POLYGON |
| 0x5A | 90 | NDEDEDE | CAPTAIN | 3803 | File.NDEDEDE_MAIN | 0x2B4 | POLYGON |
| 0x5B | 91 | NYLINK | LINK | 3805 | File.NYLINK_MAIN | 0x2F0 | POLYGON |
| 0x5C | 92 | NGOEMON | MARIO | 3807 | File.NGOEMON_MAIN | 0x3F0 | POLYGON |
| 0x5D | 93 | NCONKER | FOX | 3809 | File.NCONKER_MAIN | 0x3FC | POLYGON |
| 0x5E | 94 | NBANJO | CAPTAIN | 3811 | File.NBANJO_MAIN | 0x4A8 | POLYGON |
| 0x5F | 95 | NPEACH | FOX | 3813 | File.NPEACH_MAIN | 0x428 | POLYGON |
| 0x60 | 96 | NCRASH | MARIO | 3815 | File.NCRASH_MAIN | 0x2C0 | POLYGON |

Contagens derivadas dos parâmetros:
- `variant_type.NA` = 18 (FALCO, GND, YLINK, DRM, WARIO, BOWSER, WOLF, CONKER, MTWO, MARTH, SONIC, SHEIK, MARINA, DEDEDE, GOEMON, BANJO, CRASH, PEACH). Só estes contam em `NUM_REMIX_FIGHTERS` (`src/Character.asm:355-357`).
- `variant_type.POLYGON` = 20 (0x4D NWARIO a 0x60 NCRASH). Contam em `NUM_POLYGONS` (`src/Character.asm:347-352`).
- `variant_type.J` = 12, `variant_type.E` = 4, `variant_type.SPECIAL` = 15 (RANDOM, DSAMUS, LUCAS, GBOWSER, PIANO, SANDBAG, SSONIC, PEPPY, SLIPPY, MLUIGI, EBI, DRAGONKING, ROY, DRL, LANKY).

Observação para o port: o decomp (`decomp/src/ft/ftdef.h:1102-1139`) tem 27 fighters e `dFTManagerDataFiles[nFTKindEnumCount + 1]` (`decomp/src/ft/ftdata.c:129-137`). O Remix tem 97 slots, mais o `NULL` implícito de fim de tabela não é usado (a tabela é indexada por ID, não percorrida até NULL).

## 2. Tabelas que o Remix estende ou realoca

Convenção do Remix: a tabela vanilla é **copiada** para um novo local (`move_*`, `OS.copy_segment`) e preenchida até `NUM_CHARACTERS`; novas entradas são escritas por `add_to_table`/`table_patch_start`. O "novo local" é um label de build, sem endereço fixo no código-fonte.

Regra de endereço usada pelo código: ponteiros de dados `ptr - 0x80084800` dão o offset de ROM (`src/Character.asm:38-47`). Isso vale para a struct table (ROM `0x92610` = RAM `0x80116E10`, `src/Character.asm:19` e `decomp/src/ft/ftdata.c:128-129`). Para as demais tabelas de dados, o RAM foi derivado com `ROM + 0x80084800` e **não foi verificado**.

### 2.1 Estrutura principal

| Tabela | Origem vanilla (ROM / RAM) | Entrada | Indexada por | Tratamento Remix | Refs |
|---|---|---|---|---|---|
| STRUCT_TABLE (ponteiros para a struct do fighter) | ROM `0x92610` / RAM `0x80116E10` (citado) | 4 B | `id*4` | Crescida por `STRUCT_TABLE_END` a cada define; `STRUCT_TABLE_END` começa em +0x6C | `src/Character.asm:19-20, 80, 217-220, 3359-3361`; equivalente decomp `dFTManagerDataFiles`, `decomp/src/ft/ftdata.c:129` |
| Struct do fighter (0x78 B) | nova, criada por `define_character` | 0x78 B | — | campos 0x00–0x74 (seção 3.3) | `src/Character.asm:105-185` |
| ACTION_ARRAY_TABLE (ponteiro para o action array) | ROM `0xA6F40` (`ACTION_ARRAY_TABLE_ORIGINAL`) | 4 B | `id*4` | Copiada 27 entradas, depois `fill` até `NUM_CHARACTERS` | `src/Character.asm:22, 221-222, 3373-3376`; Kirby em `:1105-1107` |
| Shared action array | ROM `0xA45D8` | 0x14 B por ação | ID de ação < 0xDC | Não mexido; ações < 0xDC são compartilhadas | `src/Character.asm:24, 396-398` |
| SEGMENT_SIZE_TABLE | nova | 0xC B | `id*0xC` | `fill NUM_CHARACTERS*0xC` | `src/Character.asm:3366-3367` |
| Mario param array | ROM `0x92680`, ponteiro em `0x93074` | 0xC B | — | Movido com `OS.move_segment(0x92680, 0x990)` | `src/Character.asm:1042-1050` |
| Kirby param + action arrays | ponteiros via struct de Kirby | 0xC / 0x14 B | — | Estendidos com 155 ações (`extend_kirby_arrays(155)`); ponteiro gravado para KIRBY e NKIRBY | `src/Character.asm:1056-1113, 3380` |
| Param array / action array / menu array por fighter | copiados do parent | 0xC / 0x14 / 0xC B | índice da ação | copiados do parent e preenchidos com slots novos | `src/Character.asm:188-212` (menu fixo em 0xF entradas) |

### 2.2 Tabelas indexadas por ID (`move_*`, `id_table*`)

As macros `move_table`, `move_table_12`, `move_table_14`, `move_jab_3_table`, `move_rapid_jab_table`, `id_table`, `id_table_12` estão em `src/Character.asm:854-1040`. Total de invocações: `move_table` 19, `move_table_12` 8, `move_table_14` 2, `move_jab_3_table` 2, `move_rapid_jab_table` 4, `id_table` 4, `id_table_12` 4 (43 tabelas).

| Tabela (label) | Macro | Origem ROM | Entrada | Linha |
|---|---|---|---|---|
| jab_3_timer | move_jab_3_table | 0x106C90 (DISABLED=0x8014EBA4) | 4 | `:2067` |
| jab_3_action | move_jab_3_table | 0x106CF8 (DISABLED=0x8014EC30) | 4 | `:2068` |
| jab_3 (enable por ID) | scope novo | — | 4 | `:2069-2104` |
| rapid_jab_begin_action | move_rapid_jab_table | 0x106D60 | 4 | `:2107` |
| rapid_jab_loop_action | move_rapid_jab_table | 0x106DC0 | 4 | `:2108` |
| rapid_jab_ending_action | move_rapid_jab_table | 0x106E20 | 4 | `:2109` |
| rapid_jab_unknown | move_rapid_jab_table | 0x106E80 | 4 | `:2110` |
| rapid_jab (enable por ID) | scope novo | — | 4 | `:2111-2144` |
| kirby_air_nsp / air_nsp / air_usp / air_dsp | move_table | 0x103490 / 0x1034FC / 0x103568 / 0x1035D4 | 4 | `:2149, 2150-2152` |
| kirby_ground_nsp / ground_nsp / ground_usp / ground_dsp | move_table | 0x103640 / 0x1036AC / 0x103720 / 0x103790 | 4 | `:2153-2156` |
| entry_action | move_table | 0x102EE0 | 8 | `:2159` |
| entry_script | move_table | 0x106AA4 | 4 | `:2160` |
| initial_script | move_table | 0xAB294 | 4 | `:2163` |
| grounded_script | scope novo (25 vanilla + NNESS + GDK) | 0xAB304 | 4 | `:2164-2178` |
| gfx_routine_end | scope novo (vanilla + NMARIO/NFOX) | 0xAB6AC | 4 | `:2179-2193` |
| electric_hit | move_table | 0xA6FB4 | 4 | `:2194` |
| down_bound_fgm | move_table | 0xA8170 | 2 | `:2195` |
| crowd_chant_fgm | move_table | 0xA81A8 | 2 | `:2196` |
| yoshi_egg | move_table | 0x103160 | 0x1C | `:2197` |
| remix_1p_end_bgm | scope novo (IDs de MIDI) | — | 2 | `:2200-2215` |
| ai_behaviour | move_table | 0x102B04 | 4 | `:2237` |
| ai_attack_prevent | move_table | 0x1065E4 | 4 | `:2279` |
| ai_aerial_chase | move_table_14 | 0x1066A4 | 4 | `:2280` |
| ai_long_range | move_table_14 | 0x106954 | 4 | `:2281` |
| menu_zoom | move_table_12 | 0x108370 | 4 | `:2284` |
| default_costume | move_table_12 | 0xA7030 | 8 | `:2286` |
| variant_original | id_table_12 | — | 4 | `:2287` |
| vs_record | id_table_12 | — | 4 | `:2289` |
| winner_fgm | move_table_12 | 0x158148 | 4 | `:2290` |
| label_height | id_table_12 | — | 4 | `:2291` |
| str_wins_lx | move_table_12 | 0x158654 | 4 | `:2292` |
| str_winner_ptr | move_table_12 | 0x158690 | 4 | `:2293` |
| str_winner_lx | move_table_12 | 0x1586C0 | 4 | `:2294` |
| str_winner_scale | move_table_12 | 0x1586F0 | 4 | `:2295` |
| winner_bgm | move_table_12 | 0x158A08 | 4 | `:2296` |
| singleplayer_vs_preview | id_table_12 | — | 4 | `:2298` |
| fireball | move_table | 0x107070 | 4 | `:2301` |
| kirby_fireball | move_table | 0x1070E0 | 4 | `:2302` |
| thrown_hitbox / f_thrown_action / b_thrown_action / falcon_dive_id | id_table | — | 4 | `:2305-2308` |

`move_table_12` monta 27 entradas: 12 vanilla, BOSS (copia 1), METAL (copia 1), 12 polygons (cópia das 12 primeiras), GDK (cópia de DK). Ver `src/Character.asm:913-936`. `move_table_14` é igual, com 14 vanilla (`:938-958`).

### 2.3 Tabelas novas (sem origem vanilla)

| Tabela | Entrada | Indexada por | Linha | Notas |
|---|---|---|---|---|
| action_string | 4 (ponteiro) | `id*4` | `src/Character.asm:2386-2420` | nomes de ações para o modo treino |
| sound_type | 1 | `id` | `:2424-2459` | U ou J para hitbox SFX |
| sound_type_J | 48 B (copiada de ROM `0xA4500`) | fixo | `:2462-2480` | hook `apply_sound_type_` |
| variants | 4 B x4 (D-pad) | `id*16` | `:2524-2563` | variantes por direção |
| variants_with_same_model | 4 B x4 | `id*16` | `:2566-2600` | variantes com mesmo modelo |
| variant_type | 1 | `id` | `:2603-2634` | NA/POLYGON/J/E/SPECIAL/UNUSED; para J/E copia GFX extra da parent (`src/Character.asm:169-174`) |
| kirby_hat_id / kirby_inhale_struct | 0xC B | `id*0xC` | `:2636-2790` | copy ability |
| costume_shield_color | 4 B (ponteiro) + bytes | `id*4` | `:2793-2843` | cor do escudo por costume |
| fighter_reflect | 1 | `id` | `:3173-3209` | reflete projéteis |
| fighter_DK_mode | 4 B | `id*4` | `:3213-3250` | partes para DK mode |
| magnifying_glass_zoom | 2 | `id*2` | `:3255-3262` | |
| recovery_logic, cpu_post_process, cpu_attack_weight, lv10_ai_behaviour, on_hit, on_action_changed, custom_capture_action, custom_capture_dk_interrupt, nsp_shoot_custom_move | 4 | `id*4` | `:2912-3064` | jump tables de hooks de IA/lógica |
| ness_jump, pipe_turn | 1 | `id` | `:3068-3087` | |
| yoshi_egg_color | 2 | `id*2` | `:3089-3099` | |
| action_replace_map | 4 | `id*4` | `:3105-3170` | troca de ação em runtime |
| BTT_TABLE / BTP_TABLE | 1 | `id` | `:3397-3466` | stage de bônus vanilla (`0xFF` = nenhuma) |
| REMIX_BTT_TABLE / REMIX_BTP_TABLE | 1 | `id` | `:3468-3536` | stage de bônus Remix 1P |
| SEGMENT_SIZE_TABLE, ACTION_ARRAY_TABLE | — | — | `:3366-3376` | já citadas em 2.1 |
| High score SRAM | blocos `SRAM.block` | — | `:3384-3393`, `:3538-3553` | Extended (NUM-12)*0x20; Remix 1P, Allstar (NUM*0x20); Multiman, Cruel, Bonus3, HRC (NUM*4) |
| polygon_id_table (SinglePlayerModes) | 4 | `(id - NWARIO)*4` | `src/SinglePlayerModes.asm:4029-4030`, escrito em `src/Character.asm:2902-2905` | |

### 2.4 Tabelas de costume, CSS, resultados e single player

Tabelas `extra_costumes_table` e `stock_icon_palette_array_table` (ponteiros por ID, `NUM_CHARACTERS*4`): `src/Costumes.asm:1690-1695`. `register_extra_costumes_for_char` grava nelas (`src/Costumes.asm:370-392`). Cada fighter tem `parts_table` em `src/costumes/<Nome>.asm` (ex.: `src/costumes/Mario.asm:4` NUM_EXTRA_COSTUMES, `:12` NUM_COSTUMES, `:14-16` parts_table).

Atributos de polygons corrigidos no arquivo principal: `src/Costumes.asm:1761-1870` (`origin 0x94490 / dw 0x2B0`, etc.).

Tabelas de tela de seleção (CSS), todas dimensionadas por `NUM_CHARACTERS`, escritas pelo macro `add_to_css` (`src/CharacterSelect.asm:7134-7159`):

| Tabela | Linha | Entrada |
|---|---|---|
| alt_req_table (requisitos alternativos) | `src/CharacterSelect.asm:310-312` | 4 |
| fgm_table | `:2963` | 2 |
| white_circle_size_table | `:3001` | 4 (float) |
| selection_action_table | `:3037` | 4 |
| series_logo_offset_table | `:3401` | 4 |
| name_texture_table | `:3533` | 4 |
| id_table (CSS) | `:3920` | 1 |
| portrait_offset_table / portrait_offset_by_character_table | `:3935, :3947` | 4 |
| portrait_id_table | `:3988` | 1 |

Outras tabelas de fighter:
- `name_texture_table` e `name_delay_table` do single player: `src/SinglePlayer.asm:1732, 1925`, escritas por `add_to_single_player` (`src/SinglePlayer.asm:2707-2716`).
- Resultados: `add_to_results_screen` (`src/resultsscreen.asm:671-...`) grava `winner_fgm` e tabelas de logo.
- Preload de arquivos de tag team: `add_preload` (`src/TagTeam.asm:127-...`).
- Lista de alternativas: `add_alt_req_list` (`src/CharacterSelect.asm:321-...`).
- Tabela de arquivos: `move_filetable_` patcha ROM `0x499F8` (RAM `0x800CE018`) para ampliar a tabela para `0x300` entradas e apontar para `file_table` (`src/CharacterSelect.asm:1687-1700`). Não localizei a definição do label `file_table` nesta busca.

## 3. Como um novo fighter é definido

### 3.1 Assinatura do macro

`define_character(name, parent, file_1, ..., file_9, attrib_offset, add_actions, bool_jab_3, bool_inhale_copy, btt_stage_id, btp_stage_id, remix_btt_stage_id, remix_btp_stage_id, sound_type, variant_type)` (`src/Character.asm:53`). 21 argumentos. Os comentários de cada um estão em `src/Character.asm:3651-3675`.

Exemplo real (Falco, `src/Character.asm:3677`):

```
define_character(FALCO, FOX, File.FALCO_MAIN, 0x0D0, 0, File.FALCO_CHARACTER, 0x13A, 0x0D2, 0x15A, 0x0A1, 0x013C, 0x474, 0x0, OS.TRUE, OS.TRUE, Stages.id.BTT_FALCO, Stages.id.BTP_FALCO, Stages.id.BTT_FALCON, Stages.id.BTP_GND, sound_type.U, variant_type.NA)
```

### 3.2 Passos executados pelo macro

1. Rejeita o fighter se `parent > 0xB` (`src/Character.asm:54-55`).
2. Lê do parent, a partir da ROM vanilla, o ponteiro da struct, a param array, a menu array e a action array (`:38-47`, `:56-70`).
3. Calcula o tamanho da param array = tamanho do parent + `add_actions` (`:72-74`).
4. Define o ID (`:79-80`) e cria a tabela de ponteiros de arquivos (`:92-97`).
5. Escreve a struct de 0x78 bytes (`:105-184`), com os file IDs (`:110-128`), o atributo (`:174-175`) e os ponteiros de array.
6. Copia param array do parent e preenche até `param_array_size` (`:188-194`).
7. Copia action array do parent e preenche `add_actions * 0x14` (`:196-203`).
8. Cria menu array de 0xF entradas copiada do parent (`:205-211`).
9. Registra a struct em `STRUCT_TABLE_END` e a action array em `ACTION_ARRAY_TABLE` (`:213-222`).
10. Preenche as tabelas indexadas por ID copiando a entrada do parent (`add_to_table`, `:225-346`). Casos específicos: jab 3 e rapid jab (`:225-245`, regra por parent, ex. `MARIO/LUIGI/LINK/CAPTAIN/NESS` para jab 3), special moves (`:247-256`), entry (`:258-261`), other (`:263-275`), menu (`:276-296`), gfx routine end (`:289-296`), projectile (`:297-300`), ID overrides (`:301-305`), Kirby inhale (`:307-318`), BTT/BTP e Remix BTT/BTP (`:319-330`), sound type (`:331-335`), variant type (`:336-340`), escudo (`:341-345`), polygon (`:346-355`).
11. Conta `NUM_POLYGONS` e `NUM_REMIX_FIGHTERS` (`:347-357`).

### 3.3 Campos da struct do fighter x FTData (decomp)

Struct Remix (`src/Character.asm:105-185`) comparada com `struct FTData` (`decomp/src/ft/fttypes.h:111-144`). O mapeamento de arquivo e de atributo é claro; o das action/param arrays é uma correspondência por forma (inferida).

| Offset Remix | Campo Remix (argumento) | Campo FTData | Observação |
|---|---|---|---|
| 0x00 | file 1 "main file" (`file_1`) | `file_main_id` | `decomp/src/ft/fttypes.h:113` |
| 0x04 | file 2 "primary moveset" (`file_2`) | `file_mainmotion_id` | inferido (movesets principais) |
| 0x08 | file 3 "secondary moveset" (`file_3`) | `file_submotion_id` | inferido |
| 0x0C | file 4 "character/model" (`file_4`) | `file_model_id` | nome do comentário bate |
| 0x10 | file 5 "shield pose" (`file_5`) | `file_shieldpose_id` | nome do comentário bate |
| 0x14–0x20 | files 6–9 "misc" (`file_6..9`) | `file_special1..4_id` | inferido |
| 0x24 | tamanho do segmento (auto) | `file_main_size` | |
| 0x28–0x48 | ponteiros de arquivo 1–9 (auto) | `p_file_main ... p_file_special4` | |
| 0x4C–0x5C | GFX (Yoshi/Kirby/Ness), copiado ou zerado | `particles_*` (parcial) | inferido |
| 0x60 | `attrib_offset` (offset dos atributos no file 1) | `o_attributes` | `decomp/src/ft/fttypes.h:137`; uso `decomp/src/ft/ftmanager.c:849` |
| 0x64 / 0x6C | param array (ponteiro e tamanho) | `mainmotion` / `mainmotion_array_count` (`fttypes.h:139-140`) | `FTMotionDesc {anim_file_id, offset, anim_desc}` (`decomp/src/ft/fttypes.h:92-97`) corresponde a cada entrada de 0xC (animation, command, flags) |
| 0x68 | menu array (0xF entradas) | `submotion` / `submotion_array_count` | inferido |
| 0x70 | ponteiro para o tamanho da menu array | — | sem equivalente direto |
| 0x74 | tamanho do segmento de animação (auto) | `file_anim_size` (`fttypes.h:142`) | |

Formatos binários das entradas (todos little endian, big endian na N64; bass):
- Param array (0xC B): `animation`, `command`, `flags` (`src/Character.asm:388-407`, escrita em `:414-427`).
- Action array (0x14 B): `dh (param_offset << 6) | staling`, `dh unknown`, `dw asm1`, `dw asm2`, `dw asm3`, `dw asm4` (`src/Character.asm:505-536`, `:623-700`).
- Menu array (0xC B por entrada, 0xF entradas): `animation`, `command`, `flags` (`src/Character.asm:481-503`).

Ações: IDs de ação < 0xDC são compartilhados. Ações do fighter começam em 0xDC: `Action.X = (action_array_size(parent) / 0x14) + n + 0xDC` (`src/Character.asm:679-685`). Entradas novas são criadas com `add_new_action` (`:623`), que consome os slots `add_actions` declarados (`:554`, `:631` avisam quando faltam slots).

### 3.4 Arquivo de definição por fighter (exemplo: Falco)

`src/Falco/Falco.asm`:
- Inclui o moveset via `insert "moveset/X.bin"` (linhas 2–53) e a IA `include "AI/Attacks.asm"` (`:54`).
- Parâmetros de ação: `Character.edit_action_parameters(FALCO, Action.X, File.FALCO_ANIM, MOVESET_LABEL, flags)` (`:57-96`).
- ASM de ação: `Character.edit_action(FALCO, 0xE1, staling, asm1, ...)` (`:101-102`).
- Menu: `Character.edit_menu_action_parameters` (`:105-111`).
- Tabelas por ID com `Character.table_patch_start(...)`: `variants` (`:114`), `menu_zoom` (`:122`), `crowd_chant_fgm` (`:127`), `kirby_inhale_struct` (`:132`), `remix_1p_end_bgm` (`:144`).
- Costume: `Character.set_default_costumes(id, 0, 1, 4, 5, 1, 2, 3)` (`:137`), `Teams.add_team_costume(YELLOW, FALCO, 0x5)` (`:138`, macro em `src/Teams.asm:182`), `Character.set_costume_shield_colors(...)` (`:141`).

`set_default_costumes` grava 7 bytes em `default_costume` (`src/Character.asm:805-818`). `set_costume_shield_colors` grava a lista de cores e o ponteiro em `costume_shield_color` (`src/Character.asm:1115-1152`).

Os IDs de ação do Falco vêm dos slots do Fox (ver seção 4): `add_actions = 0x0` (`src/Character.asm:3677`).

## 4. Clone (parent) e código compartilhado

### 4.1 Regra de parent

- O parent precisa ser um ID vanilla `<= 0x0B` (`src/Character.asm:54-55`, `:366-367`). Não se clona outro fighter Remix diretamente.
- Para polygons (variantes N), o Remix usa `Character.polygon_setup(NOME, PAI_REMIX)` (`src/Character.asm:2847-2910`), que copia a IA, a ação de texto, o costume default, a música e registra o ID em `SinglePlayerModes.polygon_id_table`. Exemplo: `src/NFalco/NFalco.asm:90` (`polygon_setup(NFALCO, FALCO)`), com parent declarado `FOX` (`src/Character.asm:3793`).
- Cada tabela por ID é preenchida pelo `add_to_table`, que copia a entrada do parent (`src/Character.asm:842-845`). Por isso um clone herda as tabelas do parent, a não ser que o arquivo do fighter sobrescreva.

### 4.2 Casos observados

- **Falco** (`src/Character.asm:3677`): parent FOX, `add_actions = 0`. Reusa os slots de ação de Fox 0xE1/0xE2 (`Laser`/`LaserAir` de Fox, `src/Action.asm:725-726`) e sobrescreve as entradas com as do Phantasm (`src/Falco/Falco.asm:101-102`; constantes em `:156-157`). O moveset é próprio (`src/Falco/moveset/*.bin`, `:2-53`). **Não** passa por `foxshared.asm`: esse arquivo só patcha as tabelas `recovery_logic` e `cpu_post_process` para FOX e JFOX (`src/foxshared.asm:129, 131, 180, 182`). Falco não está nessa lista.
- **NFalco** (`src/NFalco/NFalco.asm`): parent FOX (`src/Character.asm:3793`). Usa os labels de moveset de `Falco.*` (ex.: `Falco.RUN` em `NFalco.asm:14`, e os demais `Falco.*` nas linhas seguintes), ou seja, reaproveita o código de Falco por referência de label. Define `menu_zoom` (`NFalco.asm:74`), `ai_attack_prevent` (`:79`), `action_string` (`:85`) e chama `polygon_setup(NFALCO, FALCO)` (`:90`).
- **Marth** (`src/Marth/Marth.asm`): parent CAPTAIN (`src/Character.asm:3735`), 8 ações novas (`add_actions = 8`). Novas ações com `Character.add_new_action(MARTH, ...)` (`src/Marth/Marth.asm:462-465`); tabela `air_nsp` por ID (`:467`). Usa `Moveset.shared.*` (`src/Moveset.asm:288` scope `shared`) e constantes `Kirby.Action` (13 ocorrências). Roy (`src/Roy`) referencia Marth.
- **Sonic** (`src/Sonic/Sonic.asm`): parent FOX (`src/Character.asm:3737`), 18 ações novas (`add_actions = 18`). Ações novas com `add_new_action` (`src/Sonic/Sonic.asm:317-321`) e subrotinas `SonicDSP.*`. Usa `Sonic.classic_table` (3 ocorrências) e `Kirby.Action` (24 ocorrências).

### 4.3 Como o código compartilhado é parametrizado

- `*shared.asm` (`linkshared`, `captainshared`, `dkshared`, `nessshared`, `jigglypuffkirbyshared`, `yoshishared`, `pikashared`, `samusshared`, `marioshared`, `foxshared`) são incluídos globalmente em `main.asm:136-146`. Cada um só edita as tabelas por ID dos fighters que lhe interessam. Exemplo: `jigglypuffkirbyshared.asm:1027-1031` (recovery_logic de JIGGLYPUFF, JPUFF, EPUFF).
- Para uma ação de um moveset comum (ex.: um comando de um parent), `Character.add_concurrent_commands(name, action, command)` (`src/Character.asm:433-462`) cria uma versão nova do comando que chama o original com `CONCURRENT_STREAM`, sem alterar o original.
- Para checar se um ID é polygon em código, o Remix compara com `NUM_CHARACTERS - NUM_POLYGONS` (ex.: `src/AI.asm:2513`), por isso os polygons precisam ocupar o final da faixa de IDs de forma contínua (`0x4D–0x60`).

## 5. Hooks: pontos do código vanilla que consultam as tabelas

Fonte: `origin` + `base` e `OS.patch_start(ROM, RAM)` em `src/Character.asm` e `src/Costumes.asm`. O nome do decomp é o último comentário `// 0x8XXXXXXX` que antecede a posição mais próxima. O offset `+0xNN` é a distância desde o início dessa função anotada. Offsets grandes (acima de 0x100) indicam que a função anotada pode não ser a que contém o hook.

Top 5 (mais críticos para o boot e o spawn):

1. `assign_segments_`: ROM `0x5284C`, RAM `0x800D704C`, troca o limite `s7` para `NUM_CHARACTERS` (`src/Character.asm:1171-1176`). Decomp: `ftManagerSetupFileSize+0x6C` (`decomp/src/ft/ftmanager.c:75`).
2. `load_segment_sizes_`: ROM `0x52CC8`, RAM `0x800D74C8`, usa `STRUCT_TABLE` e `SEGMENT_SIZE_TABLE` (`src/Character.asm:1185-1188`). Decomp: `ftManagerAllocFighter+0x334` (`decomp/src/ft/ftmanager.c:150`).
3. `assign_file_pointers_`: ROM `0x53040`, RAM `0x800D7840`, `lli s1, NUM_CHARACTERS` (`src/Character.asm:1197-1200`). Decomp: `ftManagerSetupFilesPlayablesAll+0x14` (`decomp/src/ft/ftmanager.c:366`).
4. `move_action_array_table_`: ROM `0x62BD0`, RAM `0x800E73D0`, lê o ponteiro de action array em `ACTION_ARRAY_TABLE` com carry no half superior (`src/Character.asm:1207-1219`). Decomp: `ftMainSetStatus+0x4AC` (`decomp/src/ft/ftmain.c:4579`).
5. `get_initial_script_`: ROM `0x53588` / `0x535A8`, RAM `0x800D7D88` / `0x800D7DA8`, lê o script inicial por ID (`src/Character.asm:1580-1590`). Decomp: `ftManagerInitFighter+0x398` (`decomp/src/ft/ftmanager.c:478`).

Lista completa (92 hooks nos dois arquivos, em ordem de arquivo):

| Arquivo:linha | ROM (origin) | RAM (base) | Scope | Função decomp mais próxima (offset) |
|---|---|---|---|---|
| Character.asm:1173 | 0x5284C | 0x800D704C | `assign_segments_` | ftManagerSetupFileSize+0x6C (ft/ftmanager.c) |
| Character.asm:1187 | 0x52CC8 | 0x800D74C8 | `load_segment_sizes_` | ftManagerAllocFighter+0x334 (ft/ftmanager.c) |
| Character.asm:1199 | 0x53040 | 0x800D7840 | `assign_file_pointers_` | ftManagerSetupFilesPlayablesAll+0x14 (ft/ftmanager.c) |
| Character.asm:1209 | 0x62BD0 | 0x800E73D0 | `move_action_array_table_` | ftMainSetStatus+0x4AC (ft/ftmain.c) |
| Character.asm:1234 | 0xC956C | 0x8014EB2C | `set_jab_3_timer_` | ftCommonAttack12SetStatus+0x54 (ft/ftcommon/ftcommonattack1.c) |
| Character.asm:1237 | 0xC9580 | 0x8014EB40 | `set_jab_3_timer_` | ftCommonAttack12SetStatus+0x68 (ft/ftcommon/ftcommonattack1.c) |
| Character.asm:1254 | 0xC9618 | 0x8014EBD8 | `get_jab_3_action_` | ftCommonAttack13SetStatus+0x24 (ft/ftcommon/ftcommonattack1.c) |
| Character.asm:1257 | 0xC9624 | 0x8014EBE4 | `get_jab_3_action_` | ftCommonAttack13SetStatus+0x30 (ft/ftcommon/ftcommonattack1.c) |
| Character.asm:1272 | 0xC9A20 | 0x8014EFE0 | `check_jab_3_` | ftCommonAttack13CheckGoto+0x0 (ft/ftcommon/ftcommonattack1.c) |
| Character.asm:1294 | 0xC9B58 | 0x8014F118 | `get_rapid_jab_begin_action_` | ftCommonAttack100StartSetStatus+0x24 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1298 | 0xC9B68 | 0x8014F128 | `get_rapid_jab_begin_action_` | ftCommonAttack100StartSetStatus+0x34 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1315 | 0xC9E10 | 0x8014F3D0 | `get_rapid_jab_loop_action_` | ftCommonAttack100LoopSetStatus+0x10 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1319 | 0xC9E20 | 0x8014F3E0 | `get_rapid_jab_loop_action_` | ftCommonAttack100LoopSetStatus+0x20 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1336 | 0xC9EAC | 0x8014F46C | `get_rapid_jab_ending_action_` | ftCommonAttack100EndSetStatus+0x10 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1340 | 0xC9EBC | 0x8014F47C | `get_rapid_jab_ending_action_` | ftCommonAttack100EndSetStatus+0x20 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1357 | 0xC9FC4 | 0x8014F584 | `fix_rapid_jab_unknown_` | ftCommonAttack100StartCheckInterruptCommon+0x98 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1361 | 0xC9FDC | 0x8014F59C | `fix_rapid_jab_unknown_` | ftCommonAttack100StartCheckInterruptCommon+0xB0 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1376 | 0xC9F2C | 0x8014F4EC | `check_rapid_jab_` | ftCommonAttack100StartCheckInterruptCommon+0x0 (ft/ftcommon/ftcommonattack100.c) |
| Character.asm:1397 | 0xCB91C | 0x80150EDC | `get_kirby_air_nsp_` | ftKirbySpecialAirNSetStatusSelect+0xC (ft/ftcommon/ftcommonspecialair.c) |
| Character.asm:1416 | 0xCBA64 | 0x80151024 | `get_air_nsp_` | ftCommonSpecialAirCheckInterruptCommon+0x11C (ft/ftcommon/ftcommonspecialair.c) |
| Character.asm:1435 | 0xCB9B4 | 0x80150F74 | `get_air_usp_` | ftCommonSpecialAirCheckInterruptCommon+0x6C (ft/ftcommon/ftcommonspecialair.c) |
| Character.asm:1454 | 0xCB9F8 | 0x80150FB8 | `get_air_dsp_` | ftCommonSpecialAirCheckInterruptCommon+0xB0 (ft/ftcommon/ftcommonspecialair.c) |
| Character.asm:1473 | 0xCBAAC | 0x8015106C | `get_kirby_ground_nsp_` | ftKirbySpecialNSetStatusSelect+0xC (ft/ftcommon/ftcommonspecialn.c) |
| Character.asm:1492 | 0xCBB60 | 0x80151120 | `get_ground_nsp_` | ftCommonSpecialNCheckInterruptCommon+0x88 (ft/ftcommon/ftcommonspecialn.c) |
| Character.asm:1511 | 0xCBBE4 | 0x801511A4 | `get_ground_usp_` | ftCommonSpecialHiCheckInterruptCommon+0x44 (ft/ftcommon/ftcommonspecialhi.c) |
| Character.asm:1529 | 0xCBC68 | 0x80151228 | `get_ground_dsp_` | ftCommonSpecialLwCheckInterruptCommon+0x48 (ft/ftcommon/ftcommonspeciallw.c) |
| Character.asm:1547 | 0xB8664 | 0x8013DC24 | `get_entry_action_` | ftCommonAppearSetStatus+0x44 (ft/ftcommon/ftcommonentry.c) |
| Character.asm:1554 | 0xB86A4 | 0x8013DC64 | `get_entry_action_` | ftCommonAppearSetStatus+0x84 (ft/ftcommon/ftcommonentry.c) |
| Character.asm:1567 | 0xB86B8 | 0x8013DC78 | `get_entry_script_` | ftCommonAppearSetStatus+0x98 (ft/ftcommon/ftcommonentry.c) |
| Character.asm:1584 | 0x53588 | 0x800D7D88 | `get_initial_script_` | ftManagerInitFighter+0x398 (ft/ftmanager.c) |
| Character.asm:1587 | 0x535A8 | 0x800D7DA8 | `get_initial_script_` | ftManagerInitFighter+0x3B8 (ft/ftmanager.c) |
| Character.asm:1604 | 0x59C08 | 0x800DE408 | `get_grounded_script_` | mpCommonSetFighterLandingParams+0xA0 (mp/mpcommon.c) |
| Character.asm:1607 | 0x59C14 | 0x800DE414 | `get_grounded_script_` | mpCommonSetFighterLandingParams+0xAC (mp/mpcommon.c) |
| Character.asm:1622 | 0x65148 | 0x800E9948 | `get_gfx_routine_end_` | ftParamResetStatUpdateColAnim+0x74 (ft/ftparam.c) |
| Character.asm:1641 | 0x65300 | 0x800E9B00 | `get_electric_hit_effect_` | ftParamCheckSetSkeletonColAnimID+0xC (ft/ftparam.c) |
| Character.asm:1661 | 0xBEEA0 | 0x80144460 | `get_down_bound_fgm_` | ftCommonDownBounceUpdateEffects+0x38 (ft/ftcommon/ftcommondownwaitbounce.c) |
| Character.asm:1681 | 0xDF56C | 0x80164B2C | `get_crowd_chant_fgm_` | ftPublicTryStartCall+0x7C (ft/ftpublic.c) |
| Character.asm:1698 | 0xC7840 | 0x8014CE00 | `get_yoshi_egg_parameters_` | ftCommonYoshiEggSetDamageCollCollisions+0x4 (ft/ftcommon/ftcommoncaptureyoshi.c) |
| Character.asm:1709 | 0x7E89C | 0x8010309C | `get_yoshi_egg_size_` | efManagerYoshiEggLayMakeEffect+0x3C (ef/efmanager.c) |
| Character.asm:1716 | 0x7E8E4 | 0x801030E4 | `get_yoshi_egg_size_` | efManagerYoshiEggLayMakeEffect+0x84 (ef/efmanager.c) |
| Character.asm:1727 | 0xD08F4 | 0x80155EB4 | `get_fireball_struct_` | ftMarioSpecialNProcAccessory+0x50 (ft/ftchar/ftmario/ftmariospecialn.c) |
| Character.asm:1731 | 0xD0900 | 0x80155EC0 | `get_fireball_struct_` | ftMarioSpecialNProcAccessory+0x5C (ft/ftchar/ftmario/ftmariospecialn.c) |
| Character.asm:1749 | 0xD1464 | 0x80156A24 | `get_kirby_fireball_struct_` | ftKirbyCopyMarioSpecialNProcAccessory+0x50 (ft/ftchar/ftkirby/ftkirbycopymariospecialn.c) |
| Character.asm:1753 | 0xD1470 | 0x80156A30 | `get_kirby_fireball_struct_` | ftKirbyCopyMarioSpecialNProcAccessory+0x5C (ft/ftchar/ftkirby/ftkirbycopymariospecialn.c) |
| Character.asm:1772 | 0xADAD0 | 0x80133090 | `get_ai_behaviour_struct_` | mnVSItemSwitchFuncStart+0x0 (mn/mnvsmode/mnvsitemswitch.c) |
| Character.asm:1780 | 0xADB80 | 0x80133140 | `get_ai_behaviour_struct_` | mnPlayers1PTrainingMakeLabels+0x0 (mn/mnplayers/mnplayers1ptraining.c) |
| Character.asm:1792 | 0xADF10 | 0x801334D0 | `get_ai_attack_prevent_table_` | mnVSModeMakeButtonViewport+0x4 (mn/mnvsmode/mnvsmode.c) |
| Character.asm:1811 | 0xAE2C8 | 0x80133888 | `get_ai_aerial_chase_table_` | mnVSOptionsInitVars+0x10 (mn/mnvsmode/mnvsoptions.c) |
| Character.asm:1829 | 0xB3700 | 0x80138CC0 | `get_ai_long_range_table_` | mnPlayersVSMakeHandicapLevelCamera+0x14 (mn/mnplayers/mnplayersvs.c) |
| Character.asm:1848 | 0x00132E58 | 0x80134BD8 | `menu_zoom_patches` | mnPlayers1PGameMakeTotalHiScore+0x24 (mn/mnplayers/mnplayers1pgame.c) |
| Character.asm:1853 | 0x00142F14 | 0x80133934 | `menu_zoom_patches` | scStaffrollNameProcDisplay+0x4 (sc/sccommon/scstaffroll.c) |
| Character.asm:1857 | 0x0013D364 | 0x80135164 | `menu_zoom_patches` | scStaffrollFuncDraw+0x4C (sc/sccommon/scstaffroll.c) |
| Character.asm:1864 | 0x0014A23C | 0x8013420C | `menu_zoom_patches` | mvOpeningRoomGetDroppedFighterKind+0x58 (mv/mvopening/mvopeningroom.c) |
| Character.asm:1871 | 0x00152A8C | 0x801338EC | `menu_zoom_patches` | mnVSResultsSetFighterScale+0x0 (mn/mnvsmode/mnvsresults.c) |
| Character.asm:1875 | 0x00178AFC | 0x8013209C | `menu_zoom_patches` | mnScreenAdjustFuncRun+0x0 (mn/mnoption/mnscreenadjust.c) |
| Character.asm:1878 | 0x00179D10 | 0x801332B0 | `menu_zoom_patches` | mnPlayersVSSetGateLUT+0x4 (mn/mnplayers/mnplayersvs.c) |
| Character.asm:1882 | 0x15E4E4 | 0x80132494 | `menu_zoom_patches` | mnCharactersSetFighterScale+0x0 (mn/mndata/mncharacters.c) |
| Character.asm:1886 | 0x12A8B0 | 0x80131EF0 | `menu_zoom_patches` | mnStartupStartScene+0x0 (mn/mncommon/mnstartup.c) |
| Character.asm:1897 | 0x5B67C | 0x800DFE7C | `thrown_redirect_fix_` | ftMainParseMotionEvent+0xD8C (ft/ftmain.c) |
| Character.asm:1925 | 0xC4CA8 | 0x8014A268 | `f_thrown_action_fix_` | ftCommonThrowSetStatus+0x80 (ft/ftcommon/ftcommonthrow.c) |
| Character.asm:1949 | 0xC4CC8 | 0x8014A288 | `b_thrown_action_fix_` | ftCommonThrowSetStatus+0xA0 (ft/ftcommon/ftcommonthrow.c) |
| Character.asm:1977 | 0xC7BC8 | 0x8014D188 | `falcon_dive_x_fix_` | ftCommonCaptureCaptainUpdatePositions+0x98 (ft/ftcommon/ftcommoncapturecaptain.c) |
| Character.asm:2001 | 0xC7C08 | 0x8014D1C8 | `falcon_dive_y_fix_` | ftCommonCaptureCaptainUpdatePositions+0xD8 (ft/ftcommon/ftcommoncapturecaptain.c) |
| Character.asm:2025 | 0xDCABC | 0x8016207C | `get_extended_inhale_struct_` | ftKirbySpecialNCatchProcUpdate+0x4 (ft/ftchar/ftkirby/ftkirbyspecialn.c) |
| Character.asm:2030 | 0xC6F5C | 0x8014C51C | `get_extended_inhale_struct_` | ftCommonThrownKirbyStarSetStatus+0x14 (ft/ftcommon/ftcommoncapturekirby.c) |
| Character.asm:2035 | 0x7F504 | 0x80103D04 | `get_extended_inhale_struct_` | efManagerCaptureKirbyStarMakeEffect+0xC (ef/efmanager.c) |
| Character.asm:2040 | 0x7F344 | 0x80103B44 | `get_extended_inhale_struct_` | efManagerCaptureKirbyStarProcUpdate+0x1C (ef/efmanager.c) |
| Character.asm:2045 | 0xDC960 | 0x80161F20 | `get_extended_inhale_struct_` | ftKirbySpecialNCopyInitCopyVars+0x14 (ft/ftchar/ftkirby/ftkirbyspecialn.c) |
| Character.asm:2050 | 0x53660 | 0x800D7E60 | `get_extended_inhale_struct_` | ftManagerInitFighter+0x470 (ft/ftmanager.c) |
| Character.asm:2483 | 0x5E4A0 | 0x800E2CA0 | `apply_sound_type_` | ftMainPlayHitSFX+0x7C (ft/ftmain.c) |
| Character.asm:2995 | 0x632A4 | 0x800E7AA4 | `on_action_changed_hook` | ftMainSetStatus+0xB80 (ft/ftmain.c) |
| Character.asm:3117 | 0x62798 | 0x800E6F98 | `replace_action` | ftMainSetStatus+0x74 (ft/ftmain.c) |
| Costumes.asm:621 | 0x001361B4 | 0x80137F34 | `select_` | ftComputerFollowObjectiveRecover+0x10 (ft/ftcomputer.c) |
| Costumes.asm:627 | 0x00144BF8 | 0x80135618 | `select_` | mnPlayers1PTrainingUpdateCostume+0x38 (mn/mnplayers/mnplayers1ptraining.c) |
| Costumes.asm:633 | 0x13ED5C | 0x80136B5C | `select_` | mnPlayers1PBonusInitVars+0x8 (mn/mnplayers/mnplayers1pbonus.c) |
| Costumes.asm:643 | 0x14B628 | 0x801355F8 | `select_` | mnPlayers1PTrainingUpdateCostume+0x18 (mn/mnplayers/mnplayers1ptraining.c) |
| Costumes.asm:1051 | 0x001361EC | 0x80137F6C | `disable_update` | ftComputerFollowObjectiveRecover+0x48 (ft/ftcomputer.c) |
| Costumes.asm:1059 | 0x0013679C | 0x8013851C | `disable_update` | func_ovl3_8013837C+0x1A0 (ft/ftcomputer.c) |
| Costumes.asm:1064 | 0x00137D04 | 0x80139A84 | `disable_update` | ftComputerFollowObjectiveAlly+0x24 (ft/ftcomputer.c) |
| Costumes.asm:1069 | 0x00144C28 | 0x80135648 | `disable_update` | mnPlayersVSCheckTeamSelectInRangeAll+0x14 (mn/mnplayers/mnplayersvs.c) |
| Costumes.asm:1098 | 0x136770 | 0x801384F0 | `cpu_costumes_` | func_ovl3_8013837C+0x174 (ft/ftcomputer.c) |
| Costumes.asm:1103 | 0x145010 | 0x80135A30 | `cpu_costumes_` | mnPlayers1PGameMakePortraitFlash+0x34 (mn/mnplayers/mnplayers1pgame.c) |
| Costumes.asm:1195 | 0x53850 | 0x800D8050 | `force_hi_lo_poly_` | ftManagerMakeFighter+0x114 (ft/ftmanager.c) |
| Costumes.asm:1200 | 0x649C0 | 0x800E91C0 | `force_hi_lo_poly_` | ftParamSetModelPartDetailAll+0x28 (ft/ftparam.c) |
| Costumes.asm:1284 | 0x144C04 | 0x80135624 | `prevent_illegal_sound_` | mnPlayers1PTrainingUpdateCostume+0x44 (mn/mnplayers/mnplayers1ptraining.c) |
| Costumes.asm:1290 | 0x141194 | 0x80131BB4 | `prevent_illegal_sound_` | mnMapsGetSlotPosition+0x4 (mn/mnmaps/mnmaps.c) |
| Costumes.asm:1296 | 0x12FF50 | 0x80131CD0 | `prevent_illegal_sound_` | mvOpeningClashVoidProcDisplay+0x4 (mv/mvopening/mvopeningclash.c) |
| Costumes.asm:1304 | 0x001361C8 | 0x80137F48 | `prevent_illegal_sound_` | ftComputerFollowObjectiveRecover+0x24 (ft/ftcomputer.c) |
| Costumes.asm:1311 | 0x44764 | 0x800C8D84 | `check_for_custom_costume_` | lbCommonSetupTreeDObjs+0x25C (lb/lbcommon.c) |
| Costumes.asm:1316 | 0x446C0 | 0x800C8CE0 | `check_for_custom_costume_` | lbCommonSetupTreeDObjs+0x1B8 (lb/lbcommon.c) |
| Costumes.asm:1584 | 0x53090 | 0x800D7890 | `icon_palette_override_` | ftManagerSetupFilesAllKind+0x24 (ft/ftmanager.c) |
| Costumes.asm:1587 | 0x530A8 | 0x800D78A8 | `icon_palette_override_` | ftManagerSetupFilesAllKind+0x3C (ft/ftmanager.c) |

Hooks fora de `Character.asm`/`Costumes.asm` que também consultam as tabelas (não auditados por completo): `src/SinglePlayer.asm` (`NUM_CHARACTERS * 0x06` nos blocos de `:81, :153, :225, :319`), `src/AI.asm:2511-2554` (checagem de polygon), `src/CharacterSelect.asm:1687-1700` (file table), `src/SinglePlayerModes.asm:4029`.

### 5.1 Ressalvas

- Nenhum build foi feito; os endereços das tabelas novas (labels) não são conhecidos. Para obtê-los, rodar `bass -sym` (como em `patch.bat:1`) e ler o `logfile.log`.
- A faixa `0x8010xxxx`–`0x8013xxxx` tem overlays (ex.: `func_ovl3_8013837C` em `decomp/src/ft/ftcomputer.c`). Os nomes marcados nessa faixa podem não ser da mesma função no build do Remix. Os hooks de `Costumes.asm` (`select_`, `disable_update`, `cpu_costumes_`, `prevent_illegal_sound_`) e os de `menu_zoom_patches` estão nessa faixa.
- O `thrown_redirect_fix_` (RAM `0x800DFE7C`) cai em `ftMainParseMotionEvent+0xD8C`, offset grande; a função correta não foi confirmada.
- O decomp usa 27 IDs (`decomp/src/ft/ftdef.h:1102-1139`); o port precisa ampliar essas tabelas (`dFTManagerDataFiles`, `decomp/src/ft/ftdata.c:129`) para 97 entradas se for seguir o modelo do Remix. Não foi verificado o que o port hoje faz (`port/fighter_registry.cpp` não foi lido neste trabalho).
