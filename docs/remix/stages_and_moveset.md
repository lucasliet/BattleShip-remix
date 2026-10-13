# Smash Remix: stages and moveset/action framework

Fonte: `/Users/lucas/Projetos/Pessoal/BattleShip/.toolchain/pkgs/smashremix-src` (MIPS assembly, bass).
Caminhos abaixo são relativos a `src/` dessa árvore, salvo indicação. Os números de linha foram
verificados com grep/sed. Os mapeamentos de endereço RAM para funções do decomp são aproximados (ver seção 4).

## 1. Stages

### 1.1 Como uma fase é adicionada

Toda fase nova entra por uma única chamada de macro: `add_stage(name, display_name, bgm, ...)`
(`Stages.asm:4777`), invocada em `Stages.asm:5054-5285` (177 chamadas, incluindo BTT/BTP).

- `new_stages` (`Stages.asm:4678`) incrementa a cada chamada, e o ID é `0x28 + new_stages` (`Stages.asm:4780`).
  Por isso os IDs novos começam em `0x29` (`DEKU_TREE`) e vão até `0xD7` (`HTP_FALL`).
- A macro escreve `class_table`, `bonus_pointer_table`, `string_table`, `alternate_music_table`,
  `variant_table`, custom item spawn rates, `series_logo_table`, `tournament_hazard_mode_table`
  e `stage_hazard_table`, todas em offsets derivados do ID (`Stages.asm:4800-4850`).
- Música padrão: `set_default_music` grava `track+1` em `default_music_track_table` (`Stages.asm:4688`).
- Posição de preview: `add_position_array(stage, x, y, z)` (`Stages.asm:4858`).
- Animação de fundo: `add_bg_animation` (`Stages.asm:4996`) e `set_bg_type` (`Stages.asm:5018`).
- Ordem de exibição nas configurações: `order` em `add_stage`, e no fim de `Stages.asm` há uma ordenação alfabética
  automática dos stages custom (`Stages.asm:5288+`).
- Arquivos de dados por fase vivem em `src/stages/*.asm` e `*.bin` (56 arquivos). Ex.: `stages/edo.asm`
  inclui `sprite_data` via `insert` e define `NUM_SPRITE_INFO_ARRAYS` e `STAGE_FILE_OFFSET` (`stages/edo.asm:2-5`).
  Esses são incluídos dentro de `bg_info` em `Stages.asm:4891-4995`.

Não existe um arquivo "stage por stage" com tabelas próprias. Tudo é um conjunto de tabelas globais indexadas por ID,
preenchidas pelas macros. Para portar, o dado relevante é a lista de chamadas `add_stage` mais as tabelas acima.

### 1.2 IDs (`Stages.asm:20-246`)

Vanilla: `0x00`-`0x28` (`PEACHS_CASTLE` 0x00 ... `BTP_NESS` 0x28, `BTX_LAST`). BTT/BTP ficam em `0x11`-`0x28`.
Novos: `0x29`-`0xD7` (ex.: `DEKU_TREE` 0x29, `FIRST_DESTINATION` 0x2A, `BATTLEFIELD` 0x31, `DR_MARIO` 0x33,
`DRAGONKING` 0x35, `FRAYS_STAGE` 0x37, `TOH` 0x38, `FOD` 0x39, `MEMENTOS` 0x3B, `N64` 0x3E, `SMBO` 0x42,
`BLUE` 0x48, `ONETT` 0x49, `FROSTY` 0x4B, `SMASHVILLE2` 0x4C, `GERUDO` 0x63, `DRACULAS_CASTLE` 0xB0,
`TWILIGHT_CITY` 0xBA, `REMIX_RTTF` 0xBD, `SCUTTLE_TOWN` 0xBF, `TIME_TWISTER` 0xD2, `HTP_FALL` 0xD7).
`MAX_STAGE_ID` = 0xD7 (`Stages.asm:243`). `RANDOM` = 0xDE (`Stages.asm:246`).

A lista completa com nome de exibição, classe, ordem e série está nas 177 linhas de `add_stage` (`Stages.asm:5054-5285`).
Exemplo de uma linha: `add_stage(deku_tree, "Deku Tree", ..., class.BATTLE, ..., ZELDA, Hazards.type.NONE, 64)` (`Stages.asm:5054`).
Nomes de exibição completos: extrair com `grep -n '^\s*add_stage(' Stages.asm`.

Classes (`Stages.asm:516-523`): `BATTLE 0`, `RTTF 1`, `BTP 2`, `BTT 3` (SSS usa classe própria, ver `Stages.asm:5281`).

### 1.3 Tabelas (todas indexadas por stage ID)

| Tabela | Linha | Entrada | Conteúdo |
|---|---|---|---|
| `stage_file_table` | `Stages.asm:4114` | 8 bytes (`dw header`, `dw type`) | ID de arquivo header e tipo de loader (`type.*`, `Stages.asm:252-272`). Acesso com `id << 3` em `Stages.asm:1156-1190`. |
| `function_table` | `Stages.asm:3185` | 4 bytes | Ponteiro para o ground-maker da fase; 0 = bonus (`Stages.asm:3186-3212`). |
| `class_table` | `Stages.asm:4333` | 1 byte | Classe (BATTLE/RTTF/BTP/BTT). |
| `bonus_pointer_table` | `Stages.asm:4379` | 4 bytes | Ponteiro para palavras BTT/BTP (`btx_words_*`). |
| `string_table` | `Stages.asm:4443` | 4 bytes | Ponteiro para o nome de exibição. |
| `alternate_music_table` | `Stages.asm:4495` | 8 bytes | `dh` occasional, rare, rare2 (0xFFFF = nenhum). |
| `default_music_track_table` | `Stages.asm:4680` | 2 bytes | Track padrão + 1. |
| `variant_table` | `Stages.asm:4511` | 8 bytes | IDs de variantes (DL, Omega, Remix 1-6). |
| `custom_item_spawn_rate_table` | `Stages.asm:4528` | `Item.NUM_ITEMS` bytes por fase | Pesos de itens custom. |
| `series_logo_table` | `Stages.asm:4536` | 1 byte | Logo da série (`CharacterSelect.series_logo`, `CharacterSelect.asm`). |
| `tournament_hazard_mode_table` | `Stages.asm:4584` | 1 byte | Modo de hazard forçado no torneio. |
| `stage_hazard_table` | `Stages.asm:4632` | 1 byte | `Hazards.type` (NONE/HAZARDS/MOVEMENT/BOTH). |
| `background_table` | `Stages.asm:3895` | 1 byte | ID da fase "pai" para o fundo. |
| `bg_type.table` | `Stages.asm:2778-2800` | 4 bytes | Tipo de fundo (DEFAULT, STATIC, SECTORZ, BONUS3, LAST). |
| `bg_animation_.table` | `Stages.asm:~2735-2770` | 4 bytes | Ponteiro para `bg_info_*` (sprites animados). |
| `zoom_table` | `Stages.asm:3670` | float32 | Zoom do preview. |
| `position_table` | `Stages.asm:3891` | 4 bytes | Ponteiro para `position_array` (x,y,z floats). |
| `icon_offset_table` | `Stages.asm:3407` | 4 bytes | Offset do ícone no arquivo de ícones. |
| `image_table` | `Stages.asm:2942` | `NUM_ICONS*4*NUM_PAGES` | Endereços RAM dos ícones, preenchidos em runtime. |
| `random_table` | `Stages.asm:2629` | `MAX_STAGE_ID+1-25` bytes | Lista de sorteio. Preenchida em runtime por `add_stage_to_random_list_` (`Stages.asm:2567`), chamado por 18 `add_to_list(...)` (`Stages.asm:5364+`). |
| `stage_table_normal` / `_tournament` | `Stages.asm:2952` / `3054` | 1 byte por slot | Layout da tela de seleção: `db id`, 0x11/0x12 = RANDOM. |
| `stage_table_pages` | `Stages.asm:2931` | 2 words | NUM_PAGES para normal (5) e torneio (3). |

Layout da grade (`Stages.asm:541-545`): `NUM_ROWS 3`, `NUM_COLUMNS 6`, `NUM_ICONS 18` por página, `NUM_PAGES 5`
para o modo normal.

Musica: `set_default_music` (`Stages.asm:4688`) e `alternate_music_table` (`Stages.asm:4495`). BGM IDs vêm de `MIDI.id.*`.

### 1.4 Hazards e ground procs

- `Hazards.asm`: `type` (`Hazards.asm:16-22`), `mode` (`Hazards.asm:25-30`), macro `hazard_toggle` (`Hazards.asm:34`).
  Os procs de hazard são `scope` por fase: `dream_land_wind_` (75), `sector_z_arwing_` (86), `mushroom_kingdom_pow_` (97),
  `planet_zebes_acid_` (151), `hyrule_castle_tornadoes_` (259), `congo_jungle_barrel_` (298), `saffron_city_pokemon_` (383),
  `peach_bumper_` (537), `platform_movement_` (654), `add_barrel_` (720), `add_item_` (938), `rushing_water` (1870), etc.
  Total 9772 linhas.
- `Surface.asm` (565 linhas): tipos de superfície novos (`new_surface_count`, `Surface.asm:8-14`), atrito, bounce, clipping, FGM por superfície.
- `Blastzone.asm` (431 linhas): wrap de tela (`loop_screen_*`, 111-321) e pular SFX de blast zone (`skip_bottom_blast_sfx`, 90).
- `Bonus.asm` (894 linhas): tela de seleção de bonus, `update_stage_and_icons_` (564), `get_bonus_stage_index_` (645).
  A lista de IDs BTT/BTP está em `Bonus.asm:740+`.
- `FD.asm` (136 linhas): faz Final Destination jogável em VS (`fix_`, 17) e corrige música (`music_fix_`, 58).

### 1.5 Arquivos RELOC (file IDs)

Convenção (`Stages.asm` header e `File.asm`): cada fase usa três file IDs consecutivos, `HEADER`, `STG` (+1) e `BG` (+2).
Exemplo: `STG_BATTLEFIELD_HEADER(0x871)`, `STG_BATTLEFIELD(0x872)`, `STG_BATTLEFIELD_BG(0x873)` (`File.asm:44-46`).
`STG_DEKU_HEADER(0x874)` a `STG_DEKU_BG(0x876)` (`File.asm:47-49`).
`File.asm` tem 313 constantes `STG_*` (`grep -c "constant STG_" File.asm`).

- O `header` em `Stages.asm:277-512` coincide com esses IDs (ex.: `header.DEKU_TREE` = `0x0874`, `Stages.asm:~321`).
- Stages vanilla têm IDs de ROM sem nome em `File.asm` (ex.: `header.PEACHS_CASTLE` = `0x0103`, `Stages.asm:278`).
- Ícones da seleção: `File.STAGE_ICONS(0xA04)` (`File.asm:447`), carregado em `Stages.asm:2831`.
  `STAGE_ICONS_2(0x153E)` (`File.asm:3319`) não é usado em lugar nenhum.
- Na porta BattleShip, `yamls/remix/reloc_misc.yml` já lista os mesmos IDs como `file_2161`, `file_2162`... (ex.: `file_id: 2161` na linha 275, que equivale a 0x871).
  Ou seja, os IDs são os mesmos, mas os nomes ainda não foram aplicados.

### 1.6 Funções vanilla hookadas (Stages.asm e stages-relacionados)

Endereço ROM/RAM de cada `OS.patch_start`, com o decomp mais próximo (ver seção 4 sobre a precisão):

| Arquivo:linha | RAM | Função no decomp (aprox.) | Motivo |
|---|---|---|---|
| `Stages.asm:562` | 80104C24 | `grWallpaperMakeDecideKind` (grwallpaper.c:360) | `id.RTTF` / bonus skip |
| `Stages.asm:605,611` | 801056D4/EC | `grMainSetupMakeGround` (grmainsetup.c:30) | Despacho do ground-maker por `function_table` |
| `Stages.asm:673` | 80114C9C | `ifCommonAnnounceEndMessage` (ifcommon.c:3554) | Condição de gameset/falha para BTT/BTP |
| `Stages.asm:714,720,726` | 8013C2C0/408/550 | `ftCommonDead*SetStatus` (ftcommondead.c:239, 280) | Checagem de classe |
| `Stages.asm:756` | 80105720 | `grMainSetupMakeGround` | Despacho por classe |
| `Stages.asm:800-868` | 8018D3C0.. | cena de título/seleção (vários) | Treinamento e seleção |
| `Stages.asm:893-1349` | 80132xxx-80134xxx | tela de seleção (`mnmaps.c`, `sc1pintro.c`, `mnplayers1pbonus.c`) | Layout, texto, logos |
| `Stages.asm:2698` | 800FC29C | `mpCollisionInitGroundData` (mpcollision.c:4108) | Dados de colisão da fase (`dMPCollisionGroundFileInfos`) |
| `Stages.asm:2704-2730` | 80131B10/20, 80116AE4/AFC/B7C | `mnVSResultsFuncLights` / `efGroundMakeAppearActor` | Random e efeitos |
| `Stages.asm:2778,2781` | 80104C34/40 | `grWallpaperMakeDecideKind` | Tipo de fundo (`bg_type`) |
| `Stages.asm:5323,5331,5455` | 0014F774, 00138C9C, 138C50 | seleção/random | Ajustes finais de UI |
| `FD.asm:18` | 80104C84 | `grWallpaperMakeDecideKind` | Habilita FD em VS |
| `FD.asm:59` | 8011486C | `ifCommonBattleUpdateInterfaceAll` (ifcommon.c:3394) | Música do FD |
| `Hazards.asm:76-585` | 80105F34.. 8010B4B4 | `grPupupu`, `grSector`, `grInishie`, `grZebes`, `grHyrule`, `grJungle`, `grYamabuki`, `grCastle` | Hazards por fase (ver nota) |
| `Hazards.asm:165,166` | 801374E8 | `mnPlayers1PBonusPuckSpriteOffsets` (dado, não função) | Endereço de dado |
| `Surface.asm:197-454` | 800D8BDC, 800E21DC, 8016187C, 800E5C74, 800E3D38, 80108208, 800E23D8, 8016F8E4 | `ftPhysicsApplyGroundVelFriction`, `ftMainProcPhysicsMap`, `ftMainGetGroundHitObstacle`, `ftMainUpdateDamageStatGround`, `itProcessProcItemMain` | Superfícies / física |
| `Blastzone.asm:91-322` | 800E234C, 8013CE9C, 8013CECC, 8013CF20, 8013CF30, 8013CE68 | `ftMainProcPhysicsMap`, `ftCommonDeadCheckInterruptCommon` (ftcommondead.c:578) | Blast zones e KO |
| `Bonus.asm:228,414` | 80133358, 80136A8C | `mnPlayers1PTrainingCheckCostumeUsed`, `mnPlayers1PGameCheckStockArrowPress` | Seleção bonus |

Nota sobre hazards: `grZebesMakeGround` (0x80108448), `grCastleMakeGround` (0x8010B4AC), `grJungleMakeGround` (0x80109FB4)
e `grInishieMakeGround` (0x80109C0C) são os endereços exatos das funções `function.*` em `Stages.asm:499-512`,
confirmados pelo mapeamento exato.

## 2. Moveset e action framework

### 2.1 Visão geral

Cada personagem tem: um array de parâmetros de ação, um array de ações e um arquivo de moveset.

- Array de ações (`Character.asm:388-430` para leitura; `Character.asm:505-546` para escrita): entradas de `0x14` bytes:
  `dh (param_offset<<6)|staling`, depois `dw main`, `dw interrupt/other`, `dw physics`, `dw collision`.
  Os quatro `dw` são sub-rotinas MIPS, e as três últimas correspondem aos "subprocessos" do ftmain/ftcommon.
- Array de parâmetros (`Character.asm:414-428`): entradas de `0xC` bytes: `dw animation`, `dw command offset`, `dw flags`.
  `-1` significa "manter o valor do pai".
- Ações compartilhadas: `0x000`-`0x0DB` (`Action.asm:4-~446`). Ações exclusivas por personagem: a partir de `0xDC`
  (ex.: `Action.MARIO` `0x0DC`-`0x0E4`, `Action.asm:685-720`). Cada personagem tem seus próprios IDs de ação nesse intervalo.
- Dispatch de estado: `Action.change(action_id, fsm)` (`Action.asm:2098`) chama `0x800E6F24` (ftMainSetStatus).

### 2.2 Como um personagem novo é definido

- `define_character(name, parent, file_1 ... file_9, attrib_offset, add_actions, bool_jab_3, bool_inhale_copy, btt, btp, remix_btt, remix_btp, sound_type, variant_type)` (`Character.asm:53`).
  Copia o array de parâmetros e de ações do pai (`OS.copy_segment`, `Character.asm:201-202`), mais `add_actions` slots extras.
  O pai é lido do ROM original (`read32 ... "../roms/original.z64"`, `Character.asm:36-52`).
- Falco: `define_character(FALCO, FOX, ...)` (`Character.asm:3677`). `add_actions = 0`, então reaproveita os slots `0xE1`-`0xE8` do Fox.
- Wolf: `define_character(WOLF, FOX, ...)` (`Character.asm:3729`), também com `add_actions = 0`.
- Registro: `main.asm` inclui `src/Falco/Phantasm.asm` (150), `src/Falco/Falco.asm` (151), `src/Wolf/WolfSpecial.asm` (208), `src/Wolf/Wolf.asm` (209).

### 2.3 Como um arquivo de fighter estende o pai

Exemplo `Falco/Falco.asm`:
- Moveset: `insert NAME, "moveset/X.bin"` (Falco.asm:7-53). Labels ficam disponíveis para os parâmetros.
  `Moveset.GO_TO(X)` encadeia para loops (Falco.asm:7).
- Parâmetros de ação: `Character.edit_action_parameters(FALCO, Action.X, anim, moveset, flags)` (Falco.asm:56-~99).
  Aqui `anim=File.*` e `moveset` é o label do `.bin`.
- Sub-rotinas: `Character.edit_action(FALCO, 0xE1, -1, 0x800D94C4, Phantasm.ground_subroutine_, -1, -1)` (Falco.asm:101-102).
  Os argumentos são: main, interrupt, physics, collision. Aqui `Phantasm.*` são sub-rotinas MIPS escritas no próprio Falco.
- Menu: `edit_menu_action_parameters` (Falco.asm:~105-112).
- Strings de ação: `table_patch_start(action_string, ...)` (Falco.asm:236).
- Hooks em código vanilla: `OS.patch_start` em Falco.asm:244-338 e Phantasm.asm:257-266 (ex.: `0xD66E0 -> 0x8015BCA0`, Fox special N).

### 2.4 Macros de moveset e encoding (`moveset.asm`)

`moveset.asm` define as macros, e o código de cada comando está no próprio macro. Tabela resumida (linha da macro):

| Macro | Linha | Opcode (halfword ou word) | Observação |
|---|---|---|---|
| `GO_TO(addr)` | 48 | `dw 0x90000000, addr` | salto |
| `LOOP(n)` / `END_LOOP` | 55 / 62 | `0x8000` / `0x84000000` | laço |
| `THROW_DATA(addr)` | 68 | `dw 0x30000000, addr` | dados de arremesso |
| `WAIT(t)` | 75 | `dh 0x0400, t` | espera |
| `AFTER(t)` | 82 | `dh 0x0800, t` | depois de t frames |
| `HITBOX(...)` | 108 | `dh 0x0C00+...` (4 halfwords + `0x18000000` no fim) | hitbox completo |
| `END_HITBOXES` | 123 | `dw 0x18000000` | |
| `SLOPE_CONTOUR(s)` | 134 | `dh 0xBC00, s` | |
| `HURTBOX_RESET` / `HURTBOX` / `HURTBOXES` | 142 / 149 / 157 | `0x6C000000` / `0x7000+bone<<3` / `0x7400` | |
| `CONCURRENT_STREAM(addr)` | 164 | `dw 0xB8000000, addr` | |
| `SUBROUTINE(addr)` / `RETURN` | 171 / 178 | `0x88000000` / `0x8C000000` | |
| `END` | 184 | `dw 0` | fim |
| `CREATE_GFX(...)` | 190 | `dw 0x98000000 \| ...` | efeito |
| `RANDOM_SFX(...)` | 198 | `db 0xD6` ... `dw addr` | comando custom |
| `GO_TO_FILE(off)` | 205 | `dh 0xDB00, off` | comando custom |
| `SFX` / `VOICE` / `VOICE_2` | 211 / 218 / 226 | `0x3800` / `0x4400` / `0x4800` | |
| `ATTACK_VOICE` | 234 | `dw 0x50000000` | |
| `SCREEN_SHAKE(i)` | 241 | `dh 0x9800` + intensidade | |
| `HIDE_ITEM` / `SHOW_ITEM` | 259 / 265 | `0xC0000000` / `0xC0000001` | |
| `SET_FLAG(id)` | 272 | `0x5400/5800/5C00` + `0x0001` | flags da struct do jogador |

`moveset.asm:12` também hooka `get_cmd_address_` (0x800E7940) para aceitar um ponteiro de moveset em vez de offset.
Valores >= `0x80000000` são tratados como ponteiro (`moveset.asm:12-45`).
`moveset.asm:280-313` lista os `.bin` de armas compartilhadas (BEAMSWORD, BAT, FAN, STARROD, ITEM_THROW...).

### 2.5 Comandos custom (`command.asm`)

- Os bytes `0xD0`-`0xFF` são comandos custom. `load_command_` (`command.asm:14`) lê o byte e pula para `command_table[cmd-0xD0]`.
  Tabela em `command.asm:1051`: `0xD0` fsm (velocidade de frame), `0xD1` armour, `0xD2` hitbox direction,
  `0xD3` translation multiplier, `0xD4` y velocity, `0xD5` fast fall, `0xD6` random SFX, `0xD7` kinetic state,
  `0xD8` hitbox FGM, `0xD9` env color, `0xDA` switch direction, `0xDB` go to moveset file, `0xDC` L voice SFX.
  `0xDD`-`0xFF` estão livres (`OS.NULL`).
- `command_table_2` (`command.asm:1101`) é uma variante usada por `load_command_2_` (`command.asm:54`), com comandos
  pulados (tamanho 8). Os hooks ficam em `command.asm:1152-1172`: `0x800E0388`, `0x800E0548`, `0x800E072C`.
- `change_action_` (`command.asm:93`) é hookado em `0x800E6F2C` (`command.asm:1168-1170`), dentro de ftMainSetStatus.
- Hooks de hitbox: `end_hitbox_` (`command.asm:150`, 0x800E852C), `create_hitbox_` (`command.asm:190`, 0x800DF1F0),
  `texture_form_fix_` (`command.asm:232`, 0x800E962C).

### 2.6 Projéteis, skeleton e spawn

- `Projectile.asm:13-29`: IDs de projétil custom, com `VANILLA_LAST_ID 0x1F`. Cada `add_projectile(NAME)` define `id.NAME`.
  Os 12 projéteis são sequenciais a partir de 0x20.
- `Skeleton.asm:~24-61`: dois hooks (`0x800F24B0` -> `ftDisplayMainDrawAll`, `0x800F2228` -> `ftDisplayMainDrawSkeleton`)
  que trocam o modo de renderização para skeleton (valor 0x0800 no registro).
- `Spawn.asm:19,103`: hooks em `0x800FC8B0` e `0x800FAF64` (`mpCollisionGetMapObjPositionID` e `mpCollisionGetPlayerMapObjPosition`),
  para ajustar posição de spawn.

## 3. Dados reutilizáveis x lógica MIPS

Reutilizáveis direto em C (são dados ou especificação de formato):
- Arquivos `.bin` de moveset (`src/Falco/Moveset/*.bin`, `src/Wolf/Moveset/*.bin`, `src/moveset/*.bin`). São streams de comando no formato de `moveset.asm`.
  Basta um parser que decodifique os opcodes da tabela da seção 2.4.
- Tabelas de stage (seção 1.3): são arrays simples de bytes, shorts e ponteiros. Podem virar tabelas C indexadas por ID.
- Lista de IDs de stage, ação, projétil e file (`Stages.asm:20-246`, `Action.asm:4+`, `Projectile.asm:13+`, `File.asm`).
- Parâmetros de ação (array de 0xC) e de ação (0x14): são structs fixas.
- `stages/*.bin` (sprite data de fundo) e `stages/profiles/*.asm` (perfis de toggles, como `vanilla.asm`, `competitive.asm`).

Precisam ser reescritos em C contra o decomp:
- Todas as sub-rotinas MIPS (`Phantasm.asm`, `WolfSpecial.asm`, `Wolf.asm`, procs de hazard em `Hazards.asm` e de superfície em `Surface.asm`).
- Os hooks `OS.patch_start` (seções 1.6 e 2.5). Em C, substituem-se por chamadas diretas ou por tabelas de ponteiros.
- Os `command_table` de ~50 funções MIPS de `command.asm`.
- O mecanismo de `edit_action_parameters`: ele lê o ROM original em tempo de build (`read16 ... "../roms/original.z64"`).
  Na porta isso deve virar acesso direto às tabelas do decomp, sem depender do ROM.

## 4. Limitações deste levantamento

- Os mapeamentos RAM -> função do decomp usam o comentário de endereço mais próximo anterior (`// 0x8010XXXX`) nos `.c`.
  Isso é aproximado: vários endereços caem no meio de uma função. Os casos exatos (ex.: `0x80108448`, `0x8010B4AC`,
  `0x800E6F24`) aparecem em `Stages.asm` e `Action.asm`. Os demais devem ser confirmados contra o mapa de símbolos antes de editar.
- Não foi lido linha a linha `Hazards.asm` (9772 linhas) nem `Spawn.asm` (3013 linhas). Só o layout foi verificado.
- Não foi consultada memória de projeto do ai-memory para BattleShip: não há `.ai-memory.toml` nem projeto indexado.
