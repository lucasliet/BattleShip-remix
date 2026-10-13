# Smash Remix: design do CSS no port (fase F3)

Data: 2026-10-12. Escopo: somente leitura. Nenhum código foi alterado, nenhum build foi feito, nada foi commitado.
Único arquivo criado: este documento.

Convenções deste documento:
- `mnplayersvs.c:N` é `decomp/src/mn/mnplayers/mnplayersvs.c`, linha N. Os demais caminhos são relativos à raiz do repositório.
- `CharacterSelect.asm:N` é `.toolchain/pkgs/smashremix-src/src/CharacterSelect.asm`, linha N.
- Quando uma afirmação não foi verificada diretamente, ela está marcada como **(não verificado)**.

---

## 0. Resumo executivo

- O CSS VS do port hoje é o **vanilla de 12 retratos** (grade 2×6, 45 px por célula). Não há nenhum caminho Remix no CSS: `mnplayersvs.c` não tem ramo `REGION_REMIX`.
- O CSS do Remix é uma grade **3×10 de 30 slots**, mais dois bookends (bônus e aleatório). Os retratos vêm do arquivo de reloc **0xA05**, e as imagens dos bookends do arquivo **0xA06**. Os dois arquivos já estão no `.o2r` gerado pelo Torch (`build/BattleShip-Remix.o2r`).
- Os arquivos de nome (0x11) e de logo/emblema (0x14) são os mesmos ids vanilla, mas o Remix usa **offsets** diferentes nas tabelas dele. Os retratos do Remix não usam o arquivo 0x13 vanilla.
- Quase todas as tabelas do CSS do port têm 12 entradas indexadas por fkind ou por retrato. Com fkind ≥ 12 elas leem fora do limite, e os sites com guarda (`ARRAY_COUNT`) só aparecem em alguns pontos.
- **Problema de convenção encontrado:** o port mapeia o id Remix `0x1E` (GND) para fkind **28**, que é exatamente `nFTKindNull` (`ftdef.h:1139`). Como `nFTKindNull` é o sentinela de "slot vazio" e aparece em 130 pontos de `decomp/src`, o GND ficaria invisível para o CSS. Isso precisa de decisão antes da F3.
- Os nomes VS dos fighters novos **não têm entradas** em `name_texture_table` nem em `series_logo_offset_table` (`CharacterSelect.asm:3561-3565`, `3401-3433`). A fonte desses nomes no Remix precisa ser confirmada (ver seção 4).

---

## 1. Como o CSS VS do port funciona hoje

### 1.1 Arquivos carregados

- Lista de ids: `dMNPlayersVSFileIDs` (`mnplayersvs.c:28-42`): índice 0 = `llMNPlayersCommonFileID` (0x11), 1 = `llMNCommonFileID` (0x0), 2 = `llFTEmblemSpritesFileID` (0x14), 3 = `llMNSelectCommonFileID` (0x15), 4 = `llMNPlayersGameModesFileID` (id não verificado), 5 = `llMNPlayersPortraitsFileID` (0x13), 6 = `llMNPlayersSpotlightFileID` (0x16), 7 = `llMNPlayersDifficultyFileID` (0x18, só PORT).
- Ids confirmados em `tools/reloc_data_symbols.us.txt:24,41-48`: `llMNCommonFileID=0x0`, `llMNPlayersCommonFileID=0x11`, `llMNPlayersPortraitsFileID=0x13`, `llFTEmblemSpritesFileID=0x14`, `llMNSelectCommonFileID=0x15`, `llMNPlayersSpotlightFileID=0x16`, `llMNPlayersDifficultyFileID=0x18`.
- Carga: `mnPlayersVSLoad` configura o relocador (`mnplayersvs.c:5440-5453`) e chama `lbRelocLoadFilesListed(dMNPlayersVSFileIDs, sMNPlayersVSFiles)` em `mnplayersvs.c:5456`. `sMNPlayersVSFiles` tem o mesmo tamanho de `dMNPlayersVSFileIDs` (`mnplayersvs.c:134`). Ou seja, acrescentar ids a essa lista basta para carregar mais arquivos.
- Buffers de status: `sMNPlayersVSForceStatusBuffer[64]` (`mnplayersvs.c:125`) e `sMNPlayersVSStatusBuffer[256]` (`mnplayersvs.c:131`). O comentário em `mnplayersvs.c:120-124` diz que o buffer original foi ampliado por causa de um sintético (Crash) e que a capacidade já estourou uma vez ("Force Status Buffer is full").

### 1.2 Layout da grade (portrait index 0..11)

- Posição X inicial e velocidade por retrato: tabelas `portrait_pos_x[12]` e `portrait_vel[12]` em `mnPlayersVSGetNextPortraitX` (`mnplayersvs.c:233-260`). Os X são 25, 70, 115, 160, 205, 250 para as colunas 0..5, repetidos para a segunda linha.
- Posição do fundo (wallpaper) por retrato: `pos[12]` em `mnPlayersVSSetPortraitWallpaperPosition` (`mnplayersvs.c:288-302`). Linha 0 em y=36, linha 1 em y=79, x=-35 ou 310 fora da faixa.
- Criação dos 12 retratos: `mnPlayersVSMakePortraitAll` (`mnplayersvs.c:515-523`) faz `for (i = nFTKindPlayableStart; i <= nFTKindPlayableEnd; i++)`, ou seja, 12 retratos, de 0 a 11.
- Sombra de bloqueado e sprite do retrato: `mnPlayersVSMakePortraitShadow` (`mnplayersvs.c:413-465`) e `mnPlayersVSMakePortrait` (`mnplayersvs.c:468-512`).
- Posição do cursor "puck" sobre o retrato: `mnPlayersVSCenterPuckInPortrait` (`mnplayersvs.c:3883-3897`): se `portrait >= 6`, x = `portrait*45 - 270 + 36` e y = 89; senão, x = `portrait*45 + 36` e y = 46.

### 1.3 Retrato → fkind e fkind → retrato

- `mnPlayersVSGetFighterKind(portrait)` (`mnplayersvs.c:365-374`): tabela `fkinds[12]` = Luigi, Mario, DK, Link, Samus, Falcon, Ness, Yoshi, Kirby, Fox, Pikachu, Purin. **Não há guarda**: `fkinds[portrait]` lê fora se `portrait >= 12`.
- `mnPlayersVSGetPortrait(fkind)` (`mnplayersvs.c:377-398`): tabela `portraits[12]` = 1, 9, 2, 4, 0, 3, 7, 5, 8, 10, 11, 6. Tem guarda PORT em `mnplayersvs.c:392` (`(u32)fkind >= ARRAY_COUNT(portraits)` → retorna 0, ou seja, o Mario). O comentário em `mnplayersvs.c:386-391` diz que isso mascara um OOB real encontrado com ASan.

### 1.4 Texturas: retratos, nomes, emblemas

- Retrato: arquivo `sMNPlayersVSFiles[5]` = **0x13** (`llMNPlayersPortraitsFileID`). Sprites indexados por `llMNPlayersPortraits*Sprite` (`mnplayersvs.c:474-479`, `tools/reloc_data_symbols.us.txt:2619` mostra `llMNPlayersPortraitsMarioSprite = 0x4728`).
- Nome: arquivo `sMNPlayersVSFiles[0]` = **0x11**, tabela `name_offsets[12]` (`mnplayersvs.c:638-645`), com valores como `llMNPlayersCommonMarioTextSprite = 0x1838` (`tools/reloc_data_symbols.us.txt:2547`).
- Emblema (logo da série): arquivo `sMNPlayersVSFiles[2]` = **0x14**, tabela `emblem_offsets[12]` (`mnplayersvs.c:629-636`), com `llFTEmblemSpritesMarioSprite = 0x618` (`tools/reloc_data_symbols.us.txt:2636`).
- Função que monta nome e emblema: `mnPlayersVSMakeNameAndEmblem` (`mnplayersvs.c:617-680`). Tem `if (fkind != nFTKindNull)`, mas **não tem guarda de tamanho** nas linhas 652 e 670: `emblem_offsets[fkind]` e `name_offsets[fkind]` leem fora para fkind ≥ 12. Hoje isso não dispara porque o CSS VS só seleciona os 12 vanilla, mas fica latente.

### 1.5 Modelo do lutador no CSS

- `mnPlayersVSMakeFighter` (`mnplayersvs.c:1871-1938`) chama `ftManagerDestroyFighter` e `ftManagerMakeFighter(&desc)` com `desc.fkind = fkind`, `desc.is_skip_shadow_setup = TRUE` (PORT, linha 1883-1889). Ou seja, carrega o lutador **completo** (não o "alt req" do Remix, ver seção 2.5).
- Escala: `port_fighter_scale(fkind)` (`mnplayersvs.c:1921`), com fallback para `dSCSubsysFighterScales[fkind]` só no ramo `#else`.
- Figura de custo: `mnPlayersVSGetFreeCostumeRoyal` (`mnplayersvs.c:1750`) e `mnPlayersVSSelectFighterPuck` (`mnplayersvs.c:194-204`) passam `select_button` (0..3, os 4 botões C) como índice de cor em `ftParamGetCostumeCommonID(fkind, color)` (`decomp/src/ft/ftparam.c:2890-2899`). No port essa função lê `port_fighter_costume_row(fkind)` (registro).

### 1.6 Teste de acerto do cursor / puck

- Puck para um retrato: `mnPlayersVSGetPuckFighterKind` (`mnplayersvs.c:3370-3412`). Calcula `pos_x = puck.x + 13`, `pos_y = puck.y + 12`. Linha 0 se `35 < y < 79`, linha 1 se `78 < y < 122`. Coluna = `(pos_x - 25) / 45`, válida para `24 < pos_x < 295`. A linha 1 soma 6 ao índice.
- Cursor sobre o puck: `mnPlayersVSCheckPuckInRange` (`mnplayersvs.c:2431-2450`): `cursor.x + 25` dentro de `[puck.x, puck.x+26]` e `cursor.y + 3` dentro de `[puck.y, puck.y+24]`.
- Aleatório: `mnPlayersVSRandFighterKind` (`mnplayersvs.c:3900-3914`) sorteia `syUtilsRandTimeUCharRange(nFTKindPlayableEnd + 1)`, ou seja, 0..11, com rejeição de `CheckFighterCrossed` (sempre FALSE, `mnplayersvs.c:263-266`) e `CheckFighterLocked`. **O sorteio do port nunca escolhe sintéticos.**
- Bloqueio: `mnPlayersVSCheckFighterLocked` (`mnplayersvs.c:323-342`) usa `sMNPlayersVSFighterMask`, que é `u16` (`mnplayersvs.c:99`). A máscara com `1 << fkind` só serve até fkind 15.

### 1.7 Arrays fixas de 12 (ou dependentes do fkind) no CSS VS

| Local | Tabela | Índice | Guarda no port |
|---|---|---|---|
| `mnplayersvs.c:235,241` | `portrait_pos_x[12]`, `portrait_vel[12]` | retrato | não |
| `mnplayersvs.c:290` | `pos[12]` (fundo) | retrato | não |
| `mnplayersvs.c:367` | `fkinds[12]` | retrato | não |
| `mnplayersvs.c:379` | `portraits[12]` | fkind | sim (`:392`) |
| `mnplayersvs.c:417,472` | `offsets[12]` (sombra, retrato) | fkind via `GetFighterKind` | não |
| `mnplayersvs.c:629` | `emblem_offsets[12]` | fkind | **não** |
| `mnplayersvs.c:638` | `name_offsets[12]` | fkind | **não** |
| `mnplayersvs.c:2896` | `announce_names[12]` | fkind | sim (`:2916-2922`) |
| `mnplayersvs.c:4467` | `sizes[12]` (spotlight) | fkind | sim (`:4483`) |
| `mnplayersvs.c:99` | `sMNPlayersVSFighterMask` (`u16`) | fkind | n/a (`1 << fkind`) |

O mesmo padrão aparece nos três CSS de 1P/treino/bônus: `mnplayers1pgame.c:593,748,757,2193,3222`, `mnplayers1ptraining.c:309,1497,2825`, `mnplayers1pbonus.c:452`. O `decomp_fkind_audit.md` já lista esses sítios (entradas 10 e 157), mas não o problema específico de `nFTKindNull`.

### 1.8 Registro (`port/fighter_registry.h`)

- O `FighterDescriptor` tem `css_spotlight_scale` (`fighter_registry.h:118`), `costume_count` (`:174`), `results_*` e `results_emblem_*` (`:219` em diante). **Não tem** campos para retrato, nome, emblema, portrait de bookend ou posição no grid. Esses campos precisam ser acrescentados (ou, mais simples, vir de uma tabela Remix por id, ver seção 3).
- `port_fighter_register` redimensiona o registro (`fighter_registry.h`, declaração de `port_fighter_register`), então os 68 sintéticos cabem sem mexer no enum.

---

## 2. Dados do Remix para o mesmo CSS

### 2.1 Ids: enum do decomp, registro do port e Remix

- Enum do decomp (`decomp/src/ft/ftdef.h:1102-1139`): `nFTKindPlayableStart=0` (Mario) ... `nFTKindPlayableEnd=nFTKindNess=11`; `nFTKindBoss=12`, `nFTKindMMario=13`; `nFTKindNStart=14` ... `nFTKindNEnd=25`; `nFTKindGDonkey=26`; `nFTKindEnumCount=27`; **`nFTKindNull=28`** (`:1139`).
- Ids Remix (`port/remix/remix_roster_data.h:53-150`): 0x00..0x1A coincidem com fkind 0..26 (incluindo Boss 0x0C, Metal 0x0D, polígonos 0x0E..0x19 e GDonkey 0x1A). 0x1B = RANDOM e 0x1C = NONE não são lutadores.
- Port (`REMIX_ID_TO_PORT_FKIND(id) = id - 2`, `remix_roster_data.h:31`): para ids 0x1D..0x60, fkind = 27..94.
- `Character.id` do Remix usa esses mesmos valores (`Character.asm:2315-2360`, `define_character` em `Character.asm:80`).

**Colisão:** GND é id 0x1E (`remix_roster_data.h:83`), logo fkind **28** = `nFTKindNull`. Os 130 usos de `nFTKindNull` em `decomp/src` (contagem feita neste estudo) passam a tratar o GND como "slot vazio". Esse é o problema mais grave do plano (seção 5).

### 2.2 Layout do CSS VS do Remix

- Constantes: `START_X(29)`, `START_Y(34)`, `START_VISUAL(10)`, `NUM_ROWS(3)`, `NUM_COLUMNS(10)`, `NUM_PORTRAITS=30`, `BOOKEND_BONUS_PORTRAIT=30`, `BOOKEND_RANDOM_PORTRAIT=31`, `PORTRAIT_WIDTH(24)`, `PORTRAIT_HEIGHT(24)`, `PORTRAIT_SCALE(0x3F50 = 0.8125)` (`CharacterSelect.asm:3570-3590`).
- Grade de 30 slots (`CharacterSelect.asm:3599-3635`):
  - Linha 1: MARINA, DRM, LUIGI, MARIO, DONKEY, LINK, SAMUS, CAPTAIN, GND, SONIC.
  - Linha 2: DEDEDE, YLINK, NESS, YOSHI, KIRBY, FOX, PIKACHU, JIGGLYPUFF, FALCO, SHEIK.
  - Linha 3: GOEMON, CRASH, WARIO, PEACH, BOWSER, WOLF, CONKER, MTWO, MARTH, BANJO.
- Ids por slot (Remix → fkind do port):

| Slot | Personagem | id Remix | fkind port | Obs. |
|---|---|---|---|---|
| 0 | MARINA | 0x3F | 61 | |
| 1 | DRM | 0x20 | 30 | |
| 2 | LUIGI | 0x04 | 4 | vanilla |
| 3 | MARIO | 0x00 | 0 | vanilla |
| 4 | DONKEY | 0x02 | 2 | vanilla |
| 5 | LINK | 0x05 | 5 | vanilla |
| 6 | SAMUS | 0x03 | 3 | vanilla |
| 7 | CAPTAIN | 0x07 | 7 | vanilla |
| 8 | GND | 0x1E | **28 = nFTKindNull** | colisão |
| 9 | SONIC | 0x3B | 57 | |
| 10 | DEDEDE | 0x40 | 62 | |
| 11 | YLINK | 0x1F | 29 | |
| 12 | NESS | 0x0B | 11 | vanilla |
| 13 | YOSHI | 0x06 | 6 | vanilla |
| 14 | KIRBY | 0x08 | 8 | vanilla |
| 15 | FOX | 0x01 | 1 | vanilla |
| 16 | PIKACHU | 0x09 | 9 | vanilla |
| 17 | JIGGLYPUFF | 0x0A | 10 | vanilla |
| 18 | FALCO | 0x1D | 27 | |
| 19 | SHEIK | 0x3E | 60 | |
| 20 | GOEMON | 0x41 | 63 | |
| 21 | CRASH | 0x48 | 70 | |
| 22 | WARIO | 0x21 | 31 | |
| 23 | PEACH | 0x49 | 71 | |
| 24 | BOWSER | 0x34 | 50 | |
| 25 | WOLF | 0x37 | 53 | |
| 26 | CONKER | 0x38 | 54 | |
| 27 | MTWO | 0x39 | 55 | |
| 28 | MARTH | 0x3A | 56 | |
| 29 | BANJO | 0x44 | 66 | |

  (Regra usada: fkind = id − 2 para ids 0x1D..0x60. Os valores acima foram recalculados à mão; o gerador do passo 1 deve calcular por código, não à mão.)

- Bônus (bookend, `CharacterSelect.asm:23-35`): `bonus_chars` = DSAMUS 0x22, LUCAS 0x26, PEPPY 0x42, SLIPPY 0x43, ROY 0x4A, DRL 0x4B, LANKY 0x4C, DRAGONKING 0x47, EBI 0x46, PIANO 0x36 (10 entradas, `NUM_BONUS_CHARS(10)`, `:35`).
- Índice do bônus por porta: `bonus_char_index_vs/1p/training/bonus` (`CharacterSelect.asm:39-62`), 4 bytes cada.

### 2.3 Tabelas indexadas por `Character.id` (Remix)

- `id_table` (slot → Character.id): `CharacterSelect.asm:3914-3927`, 30 entradas, `OS.align(4)`.
- `portrait_offset_table` (slot → offset de retrato em 0xA05): `CharacterSelect.asm:3930-3945`, 30 entradas + BONUS_BOOKEND + RANDOM_BOOKEND. Esses dois usam o arquivo 0xA06 (`:3775`, `:3791`).
- `portrait_offset_by_character_table` (id → offset): `CharacterSelect.asm` após `:3946`. **Só tem entradas até id 0x1C.** O uso está em `SinglePlayerModes.asm:7611`.
- `portrait_id_table` (id → índice do retrato, com casos especiais NMARIO, GDONKEY, METAL, BOSS, PLACEHOLDER e NONE): `CharacterSelect.asm:3982-4024`. Os ids novos recebem índice pela própria grade (`while NUM_SLOTS`, `:3995-4012`), então **este mapeamento cobre os novos ids**.
- `series_logo_offset_table` (id → offset no arquivo 0x14): `CharacterSelect.asm:3401-3433`. **Só tem entradas até id 0x1C**; de 0x1D em diante é `fill` (zeros) (`:3433`).
- `name_texture_table` (id → offset no arquivo 0x11): `CharacterSelect.asm:3533-3565`. **Mesma situação**: só até 0x1C, depois `fill` (`:3565`).
- `variants.table` (id × 4 variantes D-pad): `Character.asm:2524-2563`, preenchida só até 0x1A; o resto é `id.NONE`.

### 2.4 Offsets de textura

Arquivo de retratos **0xA05** (`CHARACTER_PORTRAITS`, `File.asm:448`; `yamls/remix/reloc_misc.yml:3906-3911`, `file_id: 2565`, tamanho descomprimido 0x30AA8). Offsets em `CharacterSelect.asm:3075-3170`:

- Vanilla: MARIO 0x1078, FOX 0x2138, DK 0x31F8, SAMUS 0x42B8, LUIGI 0x5378, LINK 0x6438, YOSHI 0x74F8, CAPTAIN 0x85B8, KIRBY 0x9678, PIKACHU 0xA738, JIGGLY 0xB7F8, NESS 0xC8B8.
- Custom (grade): MARINA 0x217B8, DRM 0x10BB8, GND 0xEA38, SONIC 0x1E578, SHEIK 0x206F8, DEDEDE 0x22878, YLINK 0xFAF8, FALCO 0xD978, GOEMON 0x23938, WARIO 0x12D38, BOWSER 0x14EB8, WOLF 0x1A278, CONKER 0x1B338, MTWO 0x1C3F8, MARTH 0x1D4B8, **CRASH 0x2AE58+0x10**, **PEACH 0x2BF10+0x10**, **BANJO 0x26B68+0x10**.
- Bônus: DSAMUS 0x11C78, LUCAS 0x13DF8, **PEPPY 0x249E8+0x10**, **SLIPPY 0x25AA8+0x10**, **ROY 0x2CFC8+0x10**, **DRL 0x2E080+0x10**, **LANKY 0x2F138+0x10**, **DRAGONKING 0x29D98+0x10**, **EBI 0x27C28+0x10**, PIANO 0x191B8.
- Bookends: BONUS_BOOKEND **0x3B28** e RANDOM_BOOKEND **0x53E8** (`:3077-3078`) no arquivo 0xA06.
- Os valores com `+0x10` são os que o próprio Remix comenta em `CharacterSelect.asm:3437` (prefixo `DF000000 00000000`). Ou seja, o offset real é o da constante mais 16 bytes.

Arquivo de nomes **0x11** (`MNPlayersCommon`, `yamls/remix/reloc_menus.yml:117-123`, `file_id: 17`, descomprimido 0x28008). Constantes em `CharacterSelect.asm:3437-3530`: MARIO 0x1838, FOX 0x25B8, DK 0x1FF8, SAMUS 0x2358, LUIGI 0x1B18, LINK 0x2BA0, YOSHI 0x2ED8, CAPTAIN 0x3998, KIRBY 0x28E8, PIKACHU 0x32F8, JIGGLY 0x3DB8, NESS 0x35B0 (vanilla, iguais aos símbolos vanilla de `reloc_data_symbols.us.txt:2547-2558`), e para os novos: GND 0x11AA8, FALCO 0x11F88, YLINK 0x12468, DRM 0x12948, WARIO 0x175C8, BOWSER 0x18618, WOLF 0x19588, CONKER 0x19A68, MARTH 0x1A428, SONIC 0x1B2C8, SHEIK 0x1BC88, MARINA 0x1C168, DEDEDE 0x1C648, GOEMON 0x1F228, CRASH 0x251B8+0x10, PEACH 0x25B68+0x10, BANJO 0x21CF8+0x10, e outros.

Arquivo de logos/emblemas **0x14** (`FTEmblemSprites`, `yamls/remix/reloc_fighters_common.yml:9-14`, `file_id: 20`, descomprimido 0xF858). Constantes `series_logo.offset.*` em `CharacterSelect.asm:3184-3400`. Os valores vanilla batem com os símbolos: `series_logo.offset.MARIO_BROS = 0x618` (= `llFTEmblemSpritesMarioSprite`), `EARTHBOUND = 0x3F78` (= `llFTEmblemSpritesMotherSprite`).

### 2.5 Os arquivos já estão no `.o2r`

Verificado no `build/BattleShip-Remix.o2r` (zip, 5483 entradas, lido com `zipfile`):

- `reloc_misc/file_2565` (199416 bytes; retratos Remix, descomprimido 0x30AA8 = 199336 + 80 bytes de cabeçalho)
- `reloc_misc/file_2566` (24800 bytes; imagens do CSS / bookends)
- `reloc_menus/MNPlayersCommon` (163928 bytes; nomes)
- `reloc_menus/MNPlayersPortraits` (150128 bytes; arquivo 0x13 vanilla)
- `reloc_fighters_common/FTEmblemSprites` (63656 bytes; logos)
- `remix/fighters` (286556 bytes; blob de lutadores da F2)

O mapa de ids → caminho está em `port/resource/RelocFileTable.remix.cpp:2578` (`"reloc_misc/file_2565", /* 2565 */`). Portanto, **as texturas de retrato, nome e logo são textos de reloc que o Torch já extrai** e o port pode carregá-las pelo id do arquivo, como faz hoje com `lbRelocLoadFilesListed`.

Os **dados de tabela** (`id_table`, `portrait_id_table`, `name_texture_table`, `series_logo_offset_table`, `bonus_chars`, `variants.table`) estão no código da imagem Remix, não em arquivos de reloc. Não há ROM factory pronta para isso; a rota recomendada é **gerar esses valores a partir do código-fonte** (como `tools/gen_remix_roster.py` já faz para o roster), sem copiar bytes da ROM.

### 2.6 Diferenças importantes entre o port e o Remix

- O port carrega retratos do **0x13** (`mnplayersvs.c:35`), enquanto o Remix carrega **0xA05** (`CharacterSelect.asm:4585`, `File.asm:448`). Os valores de offset são diferentes (MARIO: 0x4728 no 0x13 vanilla, 0x1078 no 0xA05). **Não verificado** se as artes são iguais; a comparação visual é um teste de aceitação da F3.
- Nomes vanilla (0x11) e logos vanilla (0x14) coincidem com as tabelas do Remix para os 12 vanilla (conferido em `reloc_data_symbols.us.txt:2547,2636,2645` e `CharacterSelect.asm:3437-3450, 3184-3200`).
- Para fighters novos **não existe entrada VS** em `name_texture_table` (`CharacterSelect.asm:3561-3565`). Para 1P, o Remix usa `SinglePlayer.name_texture.X + 0x10` (`SinglePlayerModes.asm:3136-3184, 4373-4376`). **Não verificado** de onde o VS obtém o nome dos novos fighters.

### 2.7 Carregamento do modelo no CSS

- O Remix usa `load_character_model_` (`CharacterSelect.asm:77-113`): carrega só o `main` e o `model` por meio de `alt_req_table` / `alt_malloc_table` (`:195-310`), e no CSS não carrega todos os arquivos do personagem.
- O port carrega o lutador inteiro por `ftManagerMakeFighter` (`mnplayersvs.c:1893`). Isso é mais pesado e depende do buffer de status (seção 4).

### 2.8 Costumes e variantes

- Costumes (`num_costumes` do roster, `remix_roster_data.h` 6ª coluna). **Vinte** sintéticos no intervalo 0x1D..0x60 têm `num_costumes = 0`: FALCO, GND, DRM, DSAMUS, LUCAS, GBOWSER, PIANO, MARTH, SONIC, SANDBAG, SSONIC, DEDEDE, PEPPY, MLUIGI, EBI, DRAGONKING, CRASH, ROY, DRL, LANKY. Isso confere com o enunciado. A causa é que não existe `costumes/<Nome>.asm` para eles (por exemplo, `costumes/` tem `NCrash.asm` e `NFalco.asm` com `NUM_COSTUMES(6)`, mas não `Crash.asm` nem `Falco.asm`).
- Atenção: o comentário de `fighter_registry.h:169-174` diz que "Crash = 6". Isso vale para **NCRASH** (polígono), não para CRASH (0x48). Divergência entre comentário e fonte; a fonte é a referência.
- Seleção de costume no CSS (VS): o botão C passa `select_button` 0..3 como índice de cor (`mnplayersvs.c:194-204`), e `FTCostume` tem `royal[4]` (`fighter_registry.h`, comentário em "Costumes"). Fighters com `costume_count` 6 (polígonos) não conseguem ser escolhidos além do índice 3 por esse caminho.
- Variantes D-pad (Remix): `variants.table` (`Character.asm:2524-2563`) só cobre os vanilla. Para os novos ids a tabela é `NONE`, e não encontrei nenhum patch que a estenda (busca por `variants` em `.toolchain/pkgs/smashremix-src/src`, saída vazia fora de `CharacterSelect.asm`/`TwelveCharBattle.asm`/`TagTeam.asm`). Logo, **os fighters novos não têm variante D-pad no CSS**. A regra de variante do Remix é `variant_offset` por porta (`CharacterSelect.asm:2720`) lido em `:585-600`, com `D-UP/D-DOWN/D-LEFT/D-RIGHT` definidos em `Character.asm:2527-2530`.
- Variantes polígono, metal e giant no port: os ids 0x0D (Metal), 0x0E..0x19 (polígonos) e 0x1A (GDonkey) já correspondem aos fkinds vanilla 13..26 (mesma numeração). Ou seja, o port já tem os lutadores, mas não o seletor de variante no CSS.

### 2.9 Bookend bônus

- Mecânica: quando o token está sobre o portrait 30 (bookend bônus), segurar Z ou R troca o índice em `bonus_chars` da porta, com ciclo de 10 e wrap (`CharacterSelect.asm:2636-2700`, incluindo `_set_bonus_char` em `:2683-2692`). Tem atraso de 30 frames e passo de 8 frames após o início (`:2680-2684`).
- Não há animação de rolagem: o bookend é uma imagem estática do arquivo 0xA06 (`:3775`), e o personagem escolhido aparece como token. **O "scroll" é índice, não animação.**

### 2.10 Random do Remix

- `get_random_char_id_` (`CharacterSelect.asm:1127-1170`): sorteia dentro de `NUM_SLOTS` (30) ou de `TwelveCharBattle.NUM_SLOTS` conforme o modo; se o toggle de random não for DEFAULT, inclui os bônus (`:1157-1165`). O toggle "Random Select" vem de `Toggles.asm` (não verificado no port: o port não tem o menu de toggles).
- Anti-repetição: `recent_randoms_table` com `RRL_NUM_SLOTS(6)` (`CharacterSelect.asm:1320-1342`).
- Sorteio por tabela: `id_table[index]` (`:1188-1190`, `:1247-1251`) e bônus por `bonus_chars` (`:1242-1245`).

### 2.11 Hit test do Remix (token)

- Função: `get_character_id_` (`CharacterSelect.asm:~449-480`, hook por modo). Cálculo na faixa normal (`:520-553`):
  - y < 72 (`sltiu t0, v1, 72`), senão NONE. Ou seja, 3 linhas de 24 px.
  - x < 240 (`PORTRAIT_WIDTH * NUM_COLUMNS`), senão checa o bookend aleatório (`_check_random`, `:~620`).
  - x < 0 → checa o bookend bônus (`_check_bonus`, `:~625-640`), na faixa `x >= -15` (`BOOKEND_WIDTH`).
  - Coluna = x / 24, linha = y / 24, índice = linha*10 + coluna (`:547-554`); se índice ≥ 30 → NONE.
- Variante: se o token está parado em y = 85 e o cursor está em estado de aponte (`cursor state == 0`), devolve NONE (`:~575-581`).

---

## 3. Plano de implementação (passos ordenados, ≤ 5 arquivos cada)

Premissas: o port é Remix-only (o usuário informou), mas o plano preserva o caminho vanilla quando é barato (`#ifdef REGION_REMIX` ou lookup de tabela). A CMake diz que `REGION_REMIX` "só seleciona a tabela de arquivos e o pipeline de assets" (`CMakeLists.txt:748`), então o `#ifdef` é a forma correta de isolar o layout novo.

### Passo 0: decisão sobre o sentinela (bloqueia F3)

Problema: `nFTKindNull = 28` colide com GND (remix 0x1E → fkind 28).

Opções:
- **A (recomendada):** manter `nFTKindNull=28` e mapear GND para o próximo fkind livre (95), ajustando `REMIX_ID_TO_PORT_FKIND` (`remix_roster_data.h:31`) e o gerador `tools/gen_remix_roster.py`. Muda o contrato da F2 só para um id. Risco baixo, o registro já redimensiona (`port_fighter_register`).
- **B:** mover `nFTKindNull` para fora de 27..94 (por exemplo 0xFF). Muda o valor de 130 usos e pode quebrar arrays dimensionados por `nFTKindEnumCount + 1` (`decomp_fkind_audit.md`, entrada 6 e `ftdata.c:129`). Risco maior; exige auditoria antes.

Verificação: teste de unidade que compara `REMIX_ID_TO_PORT_FKIND(id)` para os 68 ids contra o conjunto de fkinds, sem colisão com `nFTKindNull`.

### Passo 1: gerador de dados do CSS (tabelas Remix → C)

- Arquivos (3): `tools/gen_remix_css.py` (novo, lê `CharacterSelect.asm` e `Character.asm`, resolve `series_logo.offset.*`, `portrait_offsets.*`, `name_texture.*`, `Character.id.*`, `+0x10`), `port/remix/remix_css_data.h` (novo, gerado), `port/remix/remix_roster_data.h` (só se a opção A do passo 0 for escolhida).
- Conteúdo gerado: `remix_css_slot_ids[30]`, `remix_css_bonus_ids[10]`, `remix_css_portrait_offset[97]` (0xA05), `remix_css_name_offset[97]` (0x11), `remix_css_logo_offset[97]` (0x14), `remix_css_variant_ids[97][4]`, e os dois offsets de bookend.
- Regra: os valores vêm do código-fonte. Nenhum byte da ROM vai para arquivo versionado (`implementation_plan.md`, "Regras").
- Verificação: rodar o gerador duas vezes e comparar (idempotência); conferir que MARIO=0x1078, CRASH=0x2AE58+0x10, BONUS_BOOKEND=0x3B28, RANDOM_BOOKEND=0x53E8.

### Passo 2: ids de arquivo do Remix para o CSS

- Arquivos (2): `port/remix/remix_css_files.h` (novo: `REMIX_CSS_PORTRAITS_FILE_ID 0xA05`, `REMIX_CSS_IMAGES_FILE_ID 0xA06`), `decomp/src/mn/mnplayers/mnplayersvs.c`.
- Mudança: em `dMNPlayersVSFileIDs` (`mnplayersvs.c:28-42`), sob `#ifdef REGION_REMIX`, acrescentar os dois ids e manter o índice 5 vanilla para o caminho antigo. `sMNPlayersVSFiles` cresce sozinho (`:134`).
- Verificação: build de `ssb64`; log de `lbRelocLoadFilesListed` mostra os dois arquivos carregados; `Render`/`sMNPlayersVSFiles[8]` não é nulo.

### Passo 3: geometria da grade 3×10 e hit test (puro, testável)

- Arquivos (4): `port/remix/remix_css_layout.h` e `.cpp` (novos; funções puras `remixCssPortraitX(index)`, `remixCssPortraitY(index)`, `remixCssPortraitAtPoint(x, y)`, com constantes do `CharacterSelect.asm:3570-3590`), `decomp/src/mn/mnplayers/mnplayersvs.c` (chamadas em `mnPlayersVSGetNextPortraitX` `:233`, `mnPlayersVSSetPortraitWallpaperPosition` `:288`, `mnPlayersVSCheckPuckInRange` `:2431`, `mnPlayersVSGetPuckFighterKind` `:3370`), e o arquivo de teste.
- Regras: y = 44 + linha*24 (`START_Y + START_VISUAL` + linha*`PORTRAIT_HEIGHT`, `:3620-3626`); x via `portrait_x_position` (`:3884-3899`); hit test com y < 72 e x < 240, bookend bônus x ∈ [-15, 0), bookend aleatório x ∈ [240, 255).
- Verificação: teste de unidade em host para os 30 slots e os bookends (matriz de pontos de borda). Como não sei como `test_remix_blobs.py` é executado, **(não verificado)** qual é o runner de testes C++ do port.

### Passo 4: retrato, nome e logo por tabela (remove os arrays de 12)

- Arquivos (3): `decomp/src/mn/mnplayers/mnplayersvs.c` (`mnPlayersVSMakePortrait` `:468`, `mnPlayersVSMakeNameAndEmblem` `:617`, `mnPlayersVSMakePortraitAll` `:515`, `mnPlayersVSGetFighterKind` `:365`, `mnPlayersVSGetPortrait` `:377`), `port/remix/remix_css_data.h` (passo 1), `port/remix/remix_roster.h` (acessor de id → fkind, se a opção A do passo 0 estiver adotada).
- Mudança: `mnPlayersVSGetFighterKind(portrait)` vira `remix_css_slot_ids[portrait]` convertido para fkind; `mnPlayersVSGetPortrait(fkind)` vira busca inversa por id. Retrato: `sMNPlayersVSFiles[8]` com `remix_css_portrait_offset[id]`. Nome: `sMNPlayersVSFiles[0]` com `remix_css_name_offset[id]`. Logo: `sMNPlayersVSFiles[2]` com `remix_css_logo_offset[id]`.
- Atenção: para ids novos, os nomes e logos vêm de tabelas que estão **zeradas** (seção 2.3 e 2.6). Este passo só fica correto depois da seção 4, item 1.
- Verificação: screenshot do CSS com os 12 vanilla mudando de arte de 0x13 para 0xA05 (comparação visual); log de "offset zero" para qualquer slot.

### Passo 5: sorteio e posição do puck no Remix

- Arquivos (3): `decomp/src/mn/mnplayers/mnplayersvs.c` (`mnPlayersVSRandFighterKind` `:3900`, `mnPlayersVSCenterPuckInPortrait` `:3883`, `mnPlayersVSSpotlightProcUpdate` `:4464`), `port/remix/remix_css_data.h`, `port/remix/remix_css_layout.cpp`.
- Mudança: o sorteio passa a usar `remix_css_slot_ids` mais os bônus (conforme o toggle), com a mesma exclusão de bloqueado. Puck usa `remixCssPortraitX/Y`. O spotlight usa `port_fighter_css_spotlight_scale` para ids fora de 0..11 (já no código, `:4483`).
- Verificação: 10 000 sorteios em teste unitário, sem repetir slot dentro da janela de 6 (anti-repetição), sem sorteio de NONE/PLACEHOLDER.

### Passo 6: custo, variante e bookend bônus (depois do VS)

- Costume: estender `mnPlayersVSSelectFighterPuck` (`mnplayersvs.c:194`) para índice > 3 quando `costume_count > 4`. Arquivos: `mnplayersvs.c`, `fighter_registry.h`.
- Variante: ler `variant_offset` por porta e mapear para id via tabela (`remix_css_variant_ids`). Arquivos: `mnplayersvs.c`, `remix_css_data.h`.
- Bookend bônus: desenhar o bookend (0xA06, offset 0x3B28) e trocar o índice de `bonus_chars` com Z/R. Arquivos: `mnplayersvs.c`, `remix_css_layout.cpp`.
- 1P, treino e bônus (`mnplayers1pgame.c`, `mnplayers1ptraining.c`, `mnplayers1pbonus.c`): repetir os passos 2 a 5 em fases separadas, cada uma com um arquivo de layout próprio. Não entram no VS.

---

## 4. Riscos e questões em aberto

### 4.1 Top 3 riscos

1. **Colisão de `nFTKindNull` com GND (fkind 28).** Verificado: `ftdef.h:1139` (Null=28), `remix_roster_data.h:31,83` (GND→28), 130 usos de `nFTKindNull` em `decomp/src`. Efeito: o GND some do CSS e de qualquer checagem `!= nFTKindNull`. Mitigação: passo 0.
2. **Fighters novos sem nomes e logos no VS.** Verificado: `name_texture_table` e `series_logo_offset_table` só têm entradas até 0x1C (`CharacterSelect.asm:3565`, `:3433`), e a busca por patches não encontrou nenhuma extensão. Efeito: texto e logo errados ou lixo (leitura do offset 0 do arquivo). A fonte de nome para VS é **não verificada**; candidatos: os constantes `name_texture.*` com `+0x10` (`:3437-3530`) usados pelo 1P (`SinglePlayerModes.asm:3136-3184`). Mitigação: confirmar no código-imagem ou pelo ROM antes do passo 4.
3. **Arrays de 12 e carga pesada do lutador completo.** Verificado: `mnplayersvs.c:235,241,290,367,417,472,629,638` sem guarda, `:125` (buffer de 64 com histórico de estouro), `:99` (`u16` para máscara por `1 << fkind`). Efeito: OOB silencioso ou panic "Force Status Buffer is full" ao rodar o CSS com 68 sintéticos. Mitigação: trocar todas as arrays por lookup de tabela ou registro (passos 3 a 5), dimensionar o buffer para o pior caso medido e usar máscara de tamanho adequado.

### 4.2 Demais questões

- **CSS model loading para sintéticos.** O port carrega o lutador completo (`mnplayersvs.c:1893`); o Remix carrega só `main` e `model` (`CharacterSelect.asm:77-113`). Não verifiquei se o registro (F2) já deixa o `FTData` do sintético disponível no CSS sem carga extra. **(não verificado)**.
- **Costumes (20 fighters com `num_costumes = 0`).** Confirmado na fonte: não há `costumes/<Nome>.asm` para eles. O que o CSS mostra com zero costumes (cor padrão? cicla?) **(não verificado)**. Recomendação: tratar `0` como "uma cor" no CSS, sem cair na tabela de cores do parent.
- **Crash: divergência entre comentário e fonte.** `fighter_registry.h:169-174` diz `Crash = 6`; a fonte tem `NCrash.asm` com 6 e `CRASH` (0x48) sem arquivo, ou seja, 0. Usar a fonte.
- **Variantes (polígono/metal/giant).** Os fkinds 13..26 já existem no port com a numeração do Remix. O que falta é o seletor D-pad. Para os novos fighters não há variante definida no Remix (`Character.asm:2524-2563`), então a variante fica restrita aos vanilla.
- **Bookend bônus.** O "scroll" é índice cíclico por Z/R (`CharacterSelect.asm:2683-2692`), não animação. Risco baixo, mas o bookend precisa do nome do bônus atual, o que depende da questão 4.1 (item 2).
- **Retratos do 0x13 vs 0xA05.** Os 12 retratos vanilla podem mudar de arte ao trocar de arquivo. Não verifiquei visualmente. Teste de aceitação do passo 4.
- **Offsets com `+0x10`.** O Remix soma 16 bytes aos offsets dos novos sprites. Não verifiquei se o `lbRelocGetFileData` do port usa a mesma base (o `.o2r` tem 80 bytes de cabeçalho a mais que o descomprimido, como visto na seção 2.5). **(não verificado)**. Teste: ler o sprite de um offset `+0x10` e comparar com o que o Remix mostra.
- **Bug candidato no z-scale do spotlight.** Nos três ramos do spotlight (`mnplayersvs.c:4484-4490`, `mnplayers1pgame.c:3233-3246`, `mnplayers1ptraining.c:2836-2850`) o `scale.vec.f.y` é atribuído duas vezes e o `.z` nunca. Não sei se o original também faz isso. **(não verificado)**.
- **Toggle "Random Select" e menus de Remix no port.** Não verifiquei se o port tem os toggles do Remix (`Toggles.asm`); o sorteio do passo 5 depende disso.
- **Vanilla preservado.** O plano troca arrays por tabelas. Para não perder o caminho vanilla, cada tabela nova é indexada pelo id Remix, e os ids 0..26 coincidem com os fkinds vanilla (seção 2.1). Por isso o caminho vanilla continua válido.

---

## 5. O que foi verificado e o que não foi

Verificado diretamente nesta análise:
- Layout, tabelas e funções do CSS VS do port (seção 1), com linhas citadas.
- Ids de arquivo (`reloc_data_symbols.us.txt`, `yamls/remix/*.yml`, `RelocFileTable.remix.cpp:2578`) e presença de `file_2565`, `file_2566`, `MNPlayersCommon`, `MNPlayersPortraits`, `FTEmblemSprites` no `.o2r` gerado.
- Layout, constantes, offsets de retrato, nome e logo do Remix (seções 2.2 a 2.4).
- Ids e fkinds (seção 2.1), contagem de 20 sintéticos com 0 costumes, e a ausência de variantes para novos ids.
- Ausência de entradas VS para novos ids em `name_texture_table` e `series_logo_offset_table`.

Não verificado (marcado no texto):
- Se as artes de retrato do 0x13 e do 0xA05 são iguais.
- Base dos offsets `+0x10` no `.o2r` e no `lbRelocGetFileData`.
- Fonte de nomes e logos VS dos fighters novos (pode estar no código-imagem ou na ROM, não nos fontes lidos).
- Comportamento do CSS com `num_costumes = 0`.
- Runner de testes C++ do port.
- Toggles do Remix no port.
- Se a carga do lutador completo no CSS cabe nos buffers atuais.

Nenhum build foi executado. Nenhum código foi alterado. Nenhum commit foi feito. A instalação de qualquer dependência não ocorreu.
