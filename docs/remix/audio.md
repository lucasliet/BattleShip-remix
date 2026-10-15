# Smash Remix: sistema de áudio (ctl/tbl/sbk) e mapa de ROM

Fonte: `.toolchain/pkgs/smashremix-src` (ASM do Remix). ROM verificada: `baserom.remix.z64` (64 MB) e `baserom.us.z64` (vanilla, 16 MB). Pesquisa somente leitura; nenhum arquivo do repositório foi alterado além deste relatório.

## 0. Resumo: onde os bancos expandidos estão de fato

| Banco | Ponteiro vanilla/Remix (tabela em ROM) | Blob de ROM (header) | Dados expandidos |
|---|---|---|---|
| Música: instrumentos `B1_sounds1_ctl` | `0x3D75C` (`midi.asm:758`) | `0x252B560`, 0x6720 B | **Bank map movido para ROM `0x2C11170` = RAM `0x80411170`** (= `0x800472D0 + 0x3C9EA0`). Confirmado pelo padrão do cabeçalho ALBank (`count=0x47`, 71 instrumentos) encontrado só nesse endereço. |
| Música: samples `B1_sounds1_tbl` | `0x3D760` (`midi.asm:762`) | `0x2531C80`, 0x116970 B | Samples novos de instrumentos anexados em `MIDI_BANK_END` (ROM `0x3000000`+), `midi.asm:829-836`. |
| SFX: `B1_sounds2_ctl` | `0x3D750` (`FGM.asm:238`) | `0x26485F0`, 0xFBA0 B | Extensão em ROM `0x2C13200` = RAM `0x80413200` = `CTL_TABLE_PC(0x8004D9F0) + 0x3C5810`. |
| SFX: `B1_sounds2_tbl` | `0x3D754` (`FGM.asm:240`) | `0x2658190`, 0x2DC1E0 B | Raw SFX novos em `MIDI_BANK_END` após instrumentos, `FGM.asm:520-532`. |
| Sequências (músicas): `S1_music.sbk` | `0x3D768` (`midi.asm:13`) | `0x2504750`, tabela 0x26E10 B | Entradas 0 e 47..374 apontam para `0x3000000`..`0x330C710`. Entradas 1..46 continuam no blob vanilla. |

Correção ao que estava registrado no primeiro mapeamento do port (2026-10-12, doc removido) e em `yamls/remix/audio.yml`: o offset `ctl+0x3C9EA0` lido literalmente como offset de arquivo (`0x28F5400`) cai em dados que não são um bank válido. Os offsets do Remix são relativos à base do ctl **em RAM**; o ROM correspondente é a RAM menos `0x7D800000` (segmento de código: `main.asm:12-13`, `origin 0x2C00000` = `base 0x80400000`). Os mesmos valores valem para o SFX: `ctl+0x3C5810` não é `0x2A0DE00` (esse trecho é padding `0xFF`, 4096 bytes), e sim `0x2C13200`. Também não encontrei os placeholders `0xFFFFFFFF` nesse bloco: nos primeiros 4 KB há 1 byte `0xFF`; os campos são valores reais (ex.: `003c5800 003c9510 003cb218 3f7e0004`).

Verificação: `0x2C11170` contém `0047 0000 0000 7d00 0000 2a90` (header do bank 71 instrumentos), idêntico ao header vanilla exceto pela contagem; em `0x252B560+0x6660` (bank vanilla) a ROM está zerada, como o relatório diz.

## 1. Como o Remix expande os bancos em runtime

### 1.1 Música (bank 1 = instrumentos de música)
- `midi.asm:758-762`: lê ctl e tbl do vanilla (`0x3D75C`, `0x3D760`). `INST_CTL_TABLE_PC = 0x800472D0`.
- `midi.asm:923-949` (`move_instrument_bank_map`): copia o bank map (`OS.move_segment(INST_BANK_MAP, 0xB8)`) para `moved_inst_bank_map` no segmento de código, acrescenta `dw instrument_parameters_n - INST_CTL_TABLE_PC` para cada instrumento novo, grava a contagem (`dh instrument_count + 1`) e aponta `ctl+4` para o novo bank map. `instrument_count` começa em `0x2A` (`midi.asm:763`).
- DMA em boot: hook `OS.patch_start(0x20148, 0x8001F548)` (`midi.asm:1252-1254`) chama `jal 0x80002CA0` (dmaCopy vanilla) com `a0 = moved_inst_bank_map_origin` (ROM), `a1 = moved_inst_bank_map` (RAM), `a2 = 0x10 + instrument_count*4` (`midi.asm:1259-1264`). Ou seja: o bank map é DMAdo do ROM `0x2C11170`, não do blob `0x252B560`.
- Samples de instrumento (`add_instrument_sample`, `midi.asm:795-846`): raw em `MIDI_BANK_END` (`midi.asm:829-833`), ponteiro `SAMPLE_RAW - INST_SAMPLE_DATA` (relativo ao tbl `0x2531C80`).
### 1.2 Sequências (músicas, S1_music.sbk)
- `MUSIC_TABLE` = valor em `0x3D768` (`midi.asm:13`). Cabeçalho: `'S1'` (0x5331) em +0, contagem `s16` em +2 (**`0x0177` = 375 entradas**, confere com `midi_count` final), entradas de 8 bytes a partir de +4: `u32 offset` (relativo ao início do sbk) e `u32 size` (`midi.asm:95-98`).
- `MIDI_BANK = 0x3000000` (`midi.asm:15`) é o início do espaço extra. `move_dream_land_midi` (`midi.asm:24-40`) move o Dream Land (ID 0) para lá.
- Cada música: `insert_midi` (`midi.asm:69-105`): `ID = (MUSIC_TABLE_END - MUSIC_TABLE)/8`, acrescenta a entrada no fim da tabela (`MUSIC_TABLE_END`), insere o `.bin` em `MIDI_BANK_END`, atualiza `dh midi_count` em `MUSIC_TABLE+2` (`midi.asm:108-109`) e `largest_midi` (`midi.asm:112-114`, usado para dimensionar `midi_memory_block`, `main.asm:320-326`).
- Variantes: `insert_external_midi` (`midi.asm:120`), `insert_midi` 6 argumentos com toggle (`:177`), `insert_extra_midi` (`:184`, sem título, ordem -1), `insert_named_extra_midi` (`:191`).
- Limites: IDs são sequenciais a partir de `0x2F` (47). Não há tabela de tamanho fixo; o limite prático é o `s16` da contagem e o espaço da ROM (`main.asm:337`: `origin 0x3E7FFFF`, ROM de 64 MB). Hooks de sound test que mostram o número máximo: `midi.asm:691-697` (`dh midi_count`, `midi_count-1`).
- Aleatoriedade: `BGM.asm:278-327` e `BGM.asm:372-` (tabelas de BGM aleatórias, `fill 4*MIDI.midi_count`).

### 1.3 SFX/FGM (bank 2 = `B1_sounds2`)
- Leitura de bases: `CTL_TABLE` = `0x3D750`, `RAW_SAMPLE_DATA` = `0x3D754`, `SFX_FGM_MAP` = `0x3D790`, `FGM_MICROCODE_MAP` = `0x3D798` (`FGM.asm:238-250`). Constantes originais: `ORIGINAL_FGM_COUNT=0x2B7`, `ORIGINAL_SFX_FGM_COUNT=0x1D0`, `ORIGINAL_SAMPLE_COUNT=0x142` (`FGM.asm:260-262`).
- Registro: `add_sound` (`FGM.asm:314`), `add_sound_advanced` (`:375`), `add_fgm` (`:426`), `reserve_fgm` (`:452`, não usado). Cada `add_sound` cria 1 sample, 1 SFX, 1 FGM e 1 entrada de sound test; `add_fgm` cria FGM e sound test, e SFX só se `sfx_id == -1`.
- Registros: `FGM.asm:989-1838`, fechados por `write_sounds()` (`FGM.asm:1841`). Após o macro: `OS.patch` de `FGM.asm:945-951` ajusta o sound test (`0xF4 + new_sound_test_count`).
- `write_sounds()` (`FGM.asm:460-960`) escreve em ordem: `parameters_map_extended` (`0x10` por sample, em ROM `0x2C13200`, `:462-464`); `bank_table_extended` (`:466-487`); `predictors_extended` (`:490-510`, cada bloco `0xA8`); raw de samples em `MIDI_BANK_END` (`:520-532`); `sfx_fgm_table` (`:534-550`); `fgm_microcode` (`:552-568`); mapa de voz (`:571-`); e atualiza contagens `SFX_FGM_MAP` (`dw 0x1D0 + new_sfx_count`, `:676-677`) e `FGM_MICROCODE_MAP` (`dw 0x2B7 + new_fgm_count`, `:680-681`).
- Hooks de volume e play: ver seção 4.

### 1.4 Números medidos (registro estático)
| Item | Novos | Faixa de ID |
|---|---|---|
| Samples de SFX (ALSound) | 815 | `0x8141`..`0x848F` (33089..33903) |
| SFX (sfx_fgm) | 824 | `0x1D0`..`0x507` (464..1287) |
| FGM (ids de áudio tocados) | 850 | `0x2B7`..`0x608` (695..1544) |
| Sound test de voz | 850 | 244..1093 |
| Músicas (MIDI) | 328 | 47..374 (`0x2F`..`0x176`) |

Observação: o FGM `0x2B7` (695) é o primeiro novo e é marcado como placeholder em `FGM.asm:989`; `FGM.asm:962-974` trata `0x2B7` como cancelamento de SFX. Ou seja, o primeiro FGM novo tem comportamento especial.

## 2. Layout em ROM (para extração)

Todos os blobs são **ROM bruta sem compressão**, fora do LBReloc (`yamls/remix/audio.yml:1-3`). Os dados expandidos estão em ROM bruta (faixas 0x2C..., 0x30...) e não em arquivos reloc comprimidos. Não há byte-swap na extração; a conversão endian é feita no carregamento (`audio.yml:5-9`).

| Segmento | ROM início | Tamanho | Observação |
|---|---|---|---|
| S1_music_sbk | 0x2504750 | 0x26E10 | tabela de 375 entradas + 1..46 vanilla |
| sequências novas (ID 47..374) | 0x3001F40..0x330C710 | ~3.1 MB | entrada 0 (Dream Land) em 0x3000000, tamanho 0x1F3F |
| B1_sounds1_ctl (header) | 0x252B560 | 0x6720 | bank 0 aponta para 0x3C9EA0 |
| bank map música (novo) | 0x2C11170 | 0x10 + 71*4 = 0x12C | DMA em boot; RAM 0x80411170 |
| B1_sounds1_tbl | 0x2531C80 | 0x116970 | samples vanilla de instrumento |
| B1_sounds2_ctl | 0x26485F0 | 0xFBA0 | cabeçalho vanilla |
| bloco de extensão SFX | 0x2C13200 | 0x10 * 815 = 0x3260 (início do parameters map) | demais blocos seguem em RAM 0x80413200+ |
| B1_sounds2_tbl | 0x2658190 | 0x2DC1E0 | raw vanilla |
| fgm_unk | 0x2934370 | 0x820 | yamls/remix/audio.yml |
| fgm_tbl (SFX_FGM_MAP) | 0x2934B90 | 0x2DD0 | |
| fgm_ucd (FGM_MICROCODE) | 0x2937960 | 0x4B20 | |
| fim dos segmentos | 0x293C480 | | |
| samples novos de instrumento e SFX | a partir de MIDI_BANK_END após as 328 músicas | | ordem: musicas, instrumentos, SFX |

Conversão RAM para ROM no segmento de código: `ROM = RAM - 0x7D800000` (validado em 0x80411170 -> 0x2C11170 e 0x80413200 -> 0x2C13200). Os offsets de ctl são relativos à base RAM do ctl (`0x800472D0` para música, `0x8004D9F0` para SFX), não à base do arquivo.

Ressalva: o asm lê `../roms/original.z64`, que não está no repositório (`roms/` contém só um .txt). Os valores acima vêm dos ROMs presentes no BattleShip; os ponteiros `0x3D750`..`0x3D7A0` batem entre o Remix e os valores `read32` do asm.

Como extrair do ROM (para o port): ler o header de `S1_music` e os 375 pares (offset,size) em `0x2504750`; para cada par, `ROM = 0x2504750 + offset`. Bank de música: ler `0x2C11170` (count s16 em +0, offsets `u32` em +0x10 relativos ao ctl). Bank de SFX: ctl em `0x26485F0` + extensão em `0x2C13200`. Sem decodificar a extensão, o port não tem os 815 SFX novos.

## 3. Lista de BGMs adicionadas (ID, nome, jogo de origem)

Fonte: `src/midi.asm:359-686` (insert_midi/extra/named). Títulos com `-` são extras sem título. Ordem é a do registro; o ID é o índice na tabela `S1_music`. Os IDs 0..46 são vanilla e não são listados aqui.

| ID | arquivo (src/music/*.bin) | título | jogo de origem | ordem |
|---|---|---|---|---|
| 47 | GANONDORF_BATTLE | Ganondorf Battle | ocarina | 111 |
| 48 | CORNERIA | Corneria | starfox | 54 |
| 49 | KOKIRI_FOREST | Kokiri Forest | ocarina | 140 |
| 50 | DR_MARIO | Fever | drm | 84 |
| 51 | GAME_CORNER | Game Corner | pokemongold | 108 |
| 52 | SMASHVILLE | Town Hall and Tom Nook's Store | acww | 258 |
| 53 | STONECARVING_CITY | Stonecarving City | warioshake | 238 |
| 54 | FIRST_DESTINATION | Final Destination (Melee) | melee | 90 |
| 55 | COOLCOOLMOUNTAIN | Snow Mountain | sm64 | 224 |
| 56 | GODDESSBALLAD | Ballad of the Goddess | skyward | 12 |
| 57 | SARIA | Saria's Song | ocarina | 215 |
| 58 | TOWEROFHEAVEN | Luna Ascension | toh | 150 |
| 59 | FOD | Gourmet Race (Melee) | kirbysuperstar | 119 |
| 60 | MUDA | Muda Kingdom | sml | 168 |
| 61 | MEMENTOS | Last Surprise | persona5 | 143 |
| 62 | SPIRAL_MOUNTAIN | Spiral Mountain | banjokazooie | 229 |
| 63 | N64 | Dire, Dire Docks | sm64 | 67 |
| 64 | MUTE_CITY | Mute City | fzero | 172 |
| 65 | BATTLEFIELD | Battlefield | brawl | 19 |
| 66 | MADMONSTER | Mad Monster Mansion | banjokazooie | 154 |
| 67 | GANON_VICTORY | - | - | - |
| 68 | YOUNGLINK_VICTORY | - | - | - |
| 69 | FALCO_VICTORY | - | - | - |
| 70 | DRMARIO_VICTORY | - | - | - |
| 71 | MELEE_MENU | - | - | - |
| 72 | GREEN_GREENS | Green Greens | kirbydreamland | 123 |
| 73 | NORFAIR | Brinstar Depths | metroid | 36 |
| 74 | BOWSERBOSS | Koopa's Theme | sm64 | 142 |
| 75 | POKEMON_STADIUM | Trainer Battle | pokemonred | 259 |
| 76 | BOWSERROAD | Koopa's Road | sm64 | 141 |
| 77 | BOWSERFINAL | Ultimate Koopa | sm64 | 265 |
| 78 | SMB3OVERWORLD | Super Mario Bros. 3 (Melee) | smb3 | 243 |
| 79 | DELFINO | Delfino Plaza | sunshine | 66 |
| 80 | VS_KLUNGO | Vs. Klungo | banjo2 | 274 |
| 81 | BIG_BLUE | Big Blue | fzero | 23 |
| 82 | DSAMUS_VICTORY | - | - | - |
| 83 | ONETT | Onett | earthbound | 181 |
| 84 | ZEBES_LANDING | Upper Brinstar | supermetroid | 269 |
| 85 | FROSTY_VILLAGE | Frosty Village | dkr | 102 |
| 86 | EASTON_KINGDOM | Easton Kingdom | sml | 77 |
| 87 | WING_CAP | Powerful Mario | sm64 | 196 |
| 88 | RBY_GYMLEADER | Gym Leader Battle | pokemonred | 126 |
| 89 | KITCHEN_ISLAND | Wario Land | warioland | 278 |
| 90 | GLACIAL | River Stage | mvc2 | 208 |
| 91 | DK_RAP | DK Rap | dk64 | 69 |
| 92 | WARIO_VICTORY | - | - | - |
| 93 | MACHRIDER | Mach Rider (Melee) | machrider | 152 |
| 94 | POKEFLOATS | Red & Blue Medley | pokemonred | 203 |
| 95 | GERUDO_VALLEY | Gerudo Valley | ocarina | 112 |
| 96 | POP_STAR | Pop Star | kirby64 | 194 |
| 97 | STAR_WOLF | Star Wolf | starfox64 | 233 |
| 98 | STARRING_WARIO | Starring Wario! | ddrmm | 235 |
| 99 | LUCAS_VICTORY | - | - | - |
| 100 | POKEMON_CHAMPION | Champion Battle | pokemonred | 44 |
| 101 | ANIMAL_CROSSING | Title Theme (Wild World) | acww | 255 |
| 102 | HYRULE_TEMPLE | Temple Theme (Melee) | zelda2 | 248 |
| 103 | POLLYANNA | Pollyanna (Melee) | earthboundb | 193 |
| 104 | SAMBA_DE_COMBO | Samba de Combo | mother3 | 214 |
| 105 | PORKY_MEDLEY | Master Porky Medley | mother3 | 160 |
| 106 | UNFOUNDED_REVENGE | Unfounded Revenge | mother3 | 268 |
| 107 | THE_DAYS_WHEN_MY_MOTHER_WAS_THERE | The Days When My Mother Was There | persona5 | 250 |
| 108 | BRAWL | - | - | - |
| 109 | NBA_JAM | NBA Jam Medley | nbajam | 174 |
| 110 | KENGJR | Call Me Jr. | mlb | 42 |
| 111 | CLOCKTOWER | Clock Tower | mvc2 | 50 |
| 112 | BEIN_FRIENDS | Bein' Friends | earthboundb | 21 |
| 113 | KK_RIDER | Go K.K. Rider! | animal_crossing | 115 |
| 114 | SNAKEY_CHANTEY | Snakey Chantey | dkc2 | 222 |
| 115 | TAZMILY | Mom's Hometown | mother3 | 165 |
| 116 | FLAT_ZONE | Flat Zone | melee | 92 |
| 117 | FLAT_ZONE_2 | Flat Zone II | brawl | 93 |
| 118 | YOSHI_GOLF | Yoshi's Island (Mario Golf) | mariogolf | 286 |
| 119 | FINALTEMPLE | Great Temple/Dark Link | zelda2 | 121 |
| 120 | OBSTACLE | Obstacle Course | yoshis_island | 176 |
| 121 | EVEN_DRIER_GUYS | Even Drier Guys | mother3 | 82 |
| 122 | FZERO_MEDLEY | F-Zero Medley | fzero | 105 |
| 123 | PEACH_CASTLE | Princess Peach's Castle (Melee) | smb | 197 |
| 124 | BANJO_MAIN | Main Title (Banjo-Kazooie) | banjokazooie | 157 |
| 125 | BOWSER_VICTORY | - | - | - |
| 126 | MULTIMAN | Multi-Man Melee | melee | 169 |
| 127 | TABUU | Boss Battle Song 2 | brawl | 32 |
| 128 | GANGPLANK | Gang-Plank Galleon | dkc | 109 |
| 129 | FD_BRAWL | Final Destination (Brawl) | brawl | 89 |
| 130 | ASTRAL_OBSERVATORY | Astral Observatory | majora | 8 |
| 131 | ARIA_OF_THE_SOUL | Aria of the Soul | persona | 6 |
| 132 | PAPER_MARIO_BATTLE | Battle Fanfare | papermario | 17 |
| 133 | KING_OF_THE_KOOPAS | King of the Koopas | papermario | 138 |
| 134 | MRPATCH | Mr. Patch | banjo2 | 167 |
| 135 | SKERRIES | K. Rool's Acid Punk | dkl | 134 |
| 136 | BEWARE_THE_FORESTS_MUSHROOMS | Beware the Forest's Mushrooms | smrpg | 22 |
| 137 | FIGHT_AGAINST_BOWSER | Fight Against Bowser | smrpg | 85 |
| 138 | DKR_BOSS | Boss Challenges | dkr | 33 |
| 139 | CRESCENT_ISLAND | Crescent Island | dkr | 59 |
| 140 | CONKER_VICTORY | - | - | - |
| 141 | RITH_ESSA | Rith Essa | jetforce | 207 |
| 142 | TARGET_TEST | Targets! | melee | 247 |
| 143 | VENOM | Venom | starfox | 271 |
| 144 | SURPRISE_ATTACK | Surprise Attack | starfox2 | 245 |
| 145 | BK_FINALBATTLE | Final Battle (Banjo-Kazooie) | banjokazooie | 87 |
| 146 | MEWTWO_VICTORY | - | - | - |
| 147 | OLE | Ole! | conker | 180 |
| 148 | WINDY | Windy and Co. | conker | 283 |
| 149 | STARFOX_MEDLEY | Star Fox Medley (Melee) | starfox | 232 |
| 150 | DATADYNE | dataDyne Central: Defection | pd | 62 |
| 151 | INVESTIGATION_X | dataDyne Central: Investigation X | pd | 63 |
| 152 | CRADLE | Antenna Cradle | goldeneye | 4 |
| 153 | MM_TITLE | Opening Title (Mischief Makers) | mischiefmakers | 184 |
| 154 | ESPERANCE | Esperance | mischiefmakers | 81 |
| 155 | SLOPRANO | Sloprano | conker | 221 |
| 156 | WOLF_VICTORY | - | - | - |
| 157 | NSMB | Overworld Theme | nsmb | 188 |
| 158 | JUNGLEJAPES | Jungle Japes (Melee) | dkc | 133 |
| 159 | FOREST_INTERLUDE | Forest Interlude | dkc2 | 97 |
| 160 | TOADS_TURNPIKE | Toad's Turnpike | mk64 | 256 |
| 161 | GB_MEDLEY | Game Boy Medley | ssbr | 107 |
| 162 | BUBBLY | Bubbly Clouds | kirbydreamland | 38 |
| 163 | ROADTOCERULEANCITY | Road to Cerulean City | pokemonred | 209 |
| 164 | LEVEL1_WARIO | Stage 1 | warioland | 231 |
| 165 | MABE | Mabe Village | awakening | 151 |
| 166 | REST | Rest Area | ssbr | 205 |
| 167 | FE_MEDLEY | Fire Emblem Medley | fire_emblem | 91 |
| 168 | YOSHI_TALE | Yoshi's Tale | yoshis_story | 288 |
| 169 | FLOWER_GARDEN | Flower Garden | yoshis_island | 94 |
| 170 | WILDLANDS | Wildlands | yoshis_island_ds | 281 |
| 171 | VS_MARX | Vs. Marx | kirbysuperstar | 275 |
| 172 | MARTH_VICTORY | - | - | - |
| 173 | SS_AQUA | S.S. Aqua | pokemongold | 213 |
| 174 | METAL_BATTLE | Metal Battle | melee | 162 |
| 175 | SLIDER | Slider | sm64 | 220 |
| 176 | MULTIMAN2 | Multi-Man Melee 2 | melee | 170 |
| 177 | FIRE_EMBLEM | Together We Ride (Melee) | fire_emblem | 257 |
| 178 | KANTO_WILD_BATTLE | Kanto Wild Pokemon Battle | pokemongold | 136 |
| 179 | SMB2OVERWORLD | Super Mario Bros. 2 Overworld | smb2 | 242 |
| 180 | PIRATELAND | Pirate Land | marioparty2 | 191 |
| 181 | TROPICALISLAND | Yoshi's Tropical Island | marioparty | 289 |
| 182 | FLYINGBATTERY | Flying Battery | sonic3 | 95 |
| 183 | OPEN_YOUR_HEART | Open Your Heart | sonicadventure | 182 |
| 184 | SONIC2_BOSS | Sonic 2 Boss | sonic2 | 225 |
| 185 | SONIC_VICTORY | - | - | - |
| 186 | CASINO_NIGHT | Casino Night Zone | sonic2 | 43 |
| 187 | MONKEY_WATCH | Monkey Watch | rhfever | 166 |
| 188 | SONIC2_SPECIAL | Sonic 2 Special Stage | sonic2 | 226 |
| 189 | SONICCD_SPECIAL | Sonic CD Special Stage | soniccd | 227 |
| 190 | GIANTWING | Giant Wing | sonicfighters | 114 |
| 191 | EMERALDHILL | Emerald Hill Zone | sonic2 | 80 |
| 192 | LIVE_AND_LEARN | Live and Learn | sonicadventure2 | 148 |
| 193 | STARDUST | Stardust Speedway B Mix | soniccd | 234 |
| 194 | GREEN_HILL_ZONE | Green Hill Zone | sonic1 | 124 |
| 195 | CHEMICAL_PLANT | Chemical Plant Zone | sonic2 | 46 |
| 196 | BABY_BOWSER | Baby Bowser | yoshis_island | 11 |
| 197 | WIDE_UNDERWATER | Ocean Medley | marioparty | 178 |
| 198 | METALLIC_MADNESS | Metallic Madness | soniccd | 163 |
| 199 | EVERYTHING | Everything (Super Sonic) | sonicfighters | 83 |
| 200 | ROCKSOLID | Rock Solid | conker | 210 |
| 201 | RAINBOWROAD | Rainbow Road | mk64 | 201 |
| 202 | MK64_CREDITS | Victory Lap | mk64 | 272 |
| 203 | RACEWAYS | Raceways | mk64 | 199 |
| 204 | LINKS_AWAKENING_MEDLEY | Link's Awakening Medley | awakening | 145 |
| 205 | CORRIDORS_OF_TIME | Corridors of Time | chrono | 55 |
| 206 | KIRBY_64_BOSS | Kirby 64 Boss | kirby64 | 139 |
| 207 | WALUIGI_PINBALL | Waluigi Pinball | mkds | 277 |
| 208 | MARINA_VICTORY | - | - | - |
| 209 | SHEIK_VICTORY | - | - | - |
| 210 | DEDEDE_VICTORY | - | - | - |
| 211 | SMB2_MEDLEY | Super Mario Bros. 2 Medley | smb2 | 241 |
| 212 | SMW_TITLECREDITS | Super Mario World Title/Credits | smw | 244 |
| 213 | DRAGONKING | Dragon King | dragonking | 75 |
| 214 | DEDEDE | King Dedede's Theme | kirbydreamland | 137 |
| 215 | DRACULAS_CASTLE | Dracula's Castle | castlevania_sotn | 73 |
| 216 | IRON_BLUE_INTENTION | Iron-Blue Intention | castlevania_bloodlines | 132 |
| 217 | DRACULAS_TEARS | Dracula's Tears | castlevania_dos | 74 |
| 218 | WARIOWARE | WarioWare, Inc. | warioware | 279 |
| 219 | BLOODY_TEARS | Bloody Tears | castlevania_2 | 26 |
| 220 | FROZEN_HILLSIDE | Frozen Hillside | kirbyair | 103 |
| 221 | MK_REVENGE | Meta Knight's Revenge | kirbysuperstar | 161 |
| 222 | SOCCER_MENU | Main Menu (ISS64) | isoccer | 155 |
| 223 | TROUBLE_MAKER | Trouble Maker | mischiefmakers | 262 |
| 224 | MAIN_MENU2 | - | - | - |
| 225 | WL2_PERFECT | Perfect! | warioland2 | 189 |
| 226 | CONTROL | Control Center | goldeneye | 53 |
| 227 | OEDO_EDO | Edo Castle Medley | goemon | 78 |
| 228 | BIS_THEGRANDFINALE | In The Final | marioluigi_bis | 131 |
| 229 | GOEMON_VICTORY | - | - | - |
| 230 | MAJORA_MIDBOSS | Middle Boss Battle | majora | 164 |
| 231 | WATCH_THEME | - | - | - |
| 232 | KAI_HIGHWAY | Kai Highway | mysticalninja | 135 |
| 233 | SMW_ATHLETIC | Athletic Theme | smw | 9 |
| 234 | CRATERIA_MAIN | Crateria Surface | supermetroid | 57 |
| 235 | SNES_RAINBOW | Rainbow Road (SNES) | smk | 202 |
| 236 | BRAWL_OOT | Ocarina of Time Medley | ocarina | 177 |
| 237 | BOSS_E | Boss E | starfox64 | 34 |
| 238 | MARINE_FORTRESS | Marine Fortress | waverace | 159 |
| 239 | TWILIGHT_CITY | Twilight City | waverace | 263 |
| 240 | SOUTHERNISLAND | Southern Island/Main Theme | waverace | 228 |
| 241 | QUEST64_BATTLE | Battle Theme (Quest 64) | quest64 | 18 |
| 242 | MUSICAL_CASTLE | Gorgeous Musical Castle | mysticalninja | 118 |
| 243 | DECISIVE | Decisive | ogrebattle64 | 65 |
| 244 | BATTLEFIELDV2 | Battlefield Ver. 2 | brawl | 20 |
| 245 | BOB | Main Theme (Super Mario 64) | sm64 | 156 |
| 246 | AREA6 | Area 6 | starfox64 | 5 |
| 247 | HILLTOPCHASE | Hilltop Chase | kirbysuperstar | 128 |
| 248 | STATUS | Status | mariotennis | 236 |
| 249 | FF4BOSS | Boss Encounter | ff4 | 35 |
| 250 | GRIMREAPERSCAVERN | Grim Reaper's Cavern | jackbros | 125 |
| 251 | WORLD_OF_ENVY | Shitto Kai / World of Envy | smtif | 218 |
| 252 | SHANTAEMEDLEY | Day/Night Traveling | shantae | 64 |
| 253 | BURNINGTOWN | Burning Town | shantae | 40 |
| 254 | SHANTAEBOSS | Boss Battle (Shantae) | shantae | 30 |
| 255 | FORTRESS_BOSS | Fortress Boss | smw | 98 |
| 256 | HORROR_LAND | Horror Land | marioparty2 | 130 |
| 257 | AC_TITLE | Animal Crossing Theme | animal_crossing | 3 |
| 258 | DARKWORLD | Dark World | lttp | 61 |
| 259 | FILESELECT_SM64 | - | - | - |
| 260 | ITSATRAP_SM64 | - | - | - |
| 261 | BANJO_VICTORY | - | - | - |
| 262 | BLASTCORPS_MENU | - | - | - |
| 263 | FRAPPE_SNOWLAND | Frappe Snowland | mk64 | 100 |
| 264 | SMRPG_BATTLE | Fight Against Monsters | smrpg | 86 |
| 265 | TRAVELING | Traveling | goepachisuro | 260 |
| 266 | CHILL | Chill | drm | 47 |
| 267 | ROLL | Roll's Theme | megamanbc | 211 |
| 268 | STICKERBRUSH_SYMPHONY | Stickerbrush Symphony | dkc2 | 237 |
| 269 | DOOM1 | DOOM Medley | doom | 71 |
| 270 | RUNNING_FROM_EVIL | Running From Evil | doom2 | 212 |
| 271 | CONKER_THE_KING | - | - | - |
| 272 | GRABBAG | Grabbag | dukenukem3d | 120 |
| 273 | WITHMILASDIVINEPROTECTION | With Mila's Divine Protection | fe_gaiden | 284 |
| 274 | DKCTITLE | Opening (Donkey Kong) | dkc | 183 |
| 275 | PLANTATION | Plantation | cavestory | 192 |
| 276 | BATTLE_GOLD_SILVER | Gold & Silver Medley | pokemongold | 116 |
| 277 | BIG_BOO | Haunted House | sm64 | 127 |
| 278 | DKING_VICTORY | - | - | - |
| 279 | 7AM | 7AM | animal_crossing | 1 |
| 280 | DKROPTIONS | - | - | - |
| 281 | GALLERY | - | - | - |
| 282 | QUEQUE | Que Que | drm64 | 198 |
| 283 | MK64MENU | - | - | - |
| 284 | GOLDENROD_CITY | Goldenrod City | pokemongold | 117 |
| 285 | CLOCKTOWN | Clock Town | majora | 51 |
| 286 | BUMPERCROPBUMP | Bumper Crop Bump | kirby64 | 39 |
| 287 | VSRIDLEY | Vs. Ridley | supermetroid | 276 |
| 288 | GANONMEDLEY | Ganon Battle Medley | zelda | 110 |
| 289 | FUGUE | Little Fugue | ssbr | 147 |
| 290 | NUTTY_NOON | Nutty Noon | kirbyreturn | 175 |
| 291 | GHOSTGULPING | Ghost Gulping | papermario | 113 |
| 292 | SMB2BOSS | Super Mario Bros. 2 Boss | smb2 | 240 |
| 293 | FZERO_CLIMBUP | Climb Up! And Get the Last Chance! | fzero_x | 49 |
| 294 | JFG_SELECT | Character Select (Jet Force Gemini) | jetforce | 45 |
| 295 | YOSHI_SKA | Yoshi's Song | yoshis_story | 287 |
| 296 | BOARD_SHOP | Board Shop | sbk | 28 |
| 297 | FLANDRES_THEME | U.N. Owen Was Her? | 2hu6 | 264 |
| 298 | NIGHTMARE | Final Boss (Nightmare's Battle) | kirbyadventure | 88 |
| 299 | THE_ALOOF_SOLDIER | The Aloof Soldier | gga | 249 |
| 300 | WENDYS_HOUSE | Wendy's House | sbk2 | 280 |
| 301 | DANGEROUS_FOE | Battle Against a Dangerous Foe | earthboundb | 14 |
| 302 | BIG_SNOWMAN | Big Snowman | sbk | 24 |
| 303 | PIKA_CUP | Pika Cup Battles 1-3 | stadium | 190 |
| 304 | ASHLEYS_THEME | Ashley's Theme | wwtouched | 7 |
| 305 | SILVER_MOUNTAIN | Silver Mountain | sbk | 219 |
| 306 | WIZPIG | Wizpig Challenge | dkr | 285 |
| 307 | TALENTSTUDIO | Talent Studio Medley | talentstudio | 246 |
| 308 | BATTLE_C1 | Battle C1 | smtiv | 16 |
| 309 | PLAY_A_MINIGAME | - | - | - |
| 310 | CREDITS_BRAWL | Credits (Brawl) | ssb | 58 |
| 311 | CRASH_VICTORY | - | - | - |
| 312 | PD_PAUSE | - | - | - |
| 313 | PEACH_VICTORY | - | - | - |
| 314 | PORKY | Porky Means Business! | earthbound | 195 |
| 315 | UNDERGROUND | Underground Theme | smb | 266 |
| 316 | UNDERGROUND_HURRY | Underground Theme (Hurry Up!) | smb | 267 |
| 317 | CRASH3 | Time Twister | crash3 | 253 |
| 318 | NSANITYBEACH | N. Sanity Beach | crash | 173 |
| 319 | CTR_MENU | - | - | - |
| 320 | DISCOVERYFALLS | Discovery Falls | dinoplanet | 68 |
| 321 | CRASHBONUS | Bonus | crash_xs | 29 |
| 322 | DK_MEDLEY | Donkey Kong Medley | dkarc | 70 |
| 323 | SM64STAFF | Staff Roll (Super Mario 64) | sm64 | 230 |
| 324 | BIG_BRIDGE | Clash on the Big Bridge | ff5 | 48 |
| 325 | HOGWILD | Hog Wild | crash | 129 |
| 326 | MARATHON | Marathon 2 Title Theme | marathon2 | 158 |
| 327 | FORGONE | Foregone Destruction | ut99 | 96 |
| 328 | MORRIGAN | Theme of Morrigan | mvc | 252 |
| 329 | ELADARD | Eladard | starfox2 | 79 |
| 330 | MADMAZEMAUL | Mad Maze Maul | dk64 | 153 |
| 331 | ORANGSPRINT | OrangSprint | dk64 | 186 |
| 332 | DRAKE_LAKE | Drake Lake | waverace | 76 |
| 333 | BUBBLEGUM_KK | Bubblegum K.K. | acnewleaf | 37 |
| 334 | FZEROX_MEDLEY | F-Zero X Medley | fzero_x | 106 |
| 335 | FREEZE | Freeze! | papermario | 101 |
| 336 | TITANIA | Titania | starfox | 254 |
| 337 | THEATER | Theater | kirby64 | 251 |
| 338 | ENEMYCARD | - | - | - |
| 339 | SHEVAT | Shevat, the Wind is Calling | xenogears | 217 |
| 340 | CORTEX | Dr. Neo Cortex | crash2 | 72 |
| 341 | WILY_FIELD | Wily's Field | megamanscr | 282 |
| 342 | SMS_BOSS | Boss Battle (Sunshine) | sunshine | 31 |
| 343 | COBALT | Cobalt Coast | heypika | 52 |
| 344 | VS_DSAMUS | Vs. Dark Samus | metroidprime3 | 273 |
| 345 | STRIKE_THE_EARTH | Strike the Earth! | shovelknight | 239 |
| 346 | VAMPIREKILLER | Vampire Killer | castlevania | 270 |
| 347 | KOOPA_BROS | Attack of the Koopa Bros. | papermario | 10 |
| 348 | REDIAL | Redial | bombermanhero | 204 |
| 349 | AGAVE | Agave | snp | 2 |
| 350 | RAIDBLUE | Raid Blue | snp | 200 |
| 351 | RISKNECK | Risk One's Neck | snp | 206 |
| 352 | SHERBETLAND | Sherbet Land | mariohoops | 216 |
| 353 | DEATH_MOUNTAIN | Dark Mountain Forest | lttp | 60 |
| 354 | BATTLE_AMONG_FRIENDS | Battle Among Friends | kirby64 | 15 |
| 355 | BUTTER_BUILDING | Butter Building | kirbyadventure | 41 |
| 356 | MURASAKI | Murasaki Forest | mother3 | 171 |
| 357 | SKYWORLD | Overworld (Kid Icarus) | kidicarus | 187 |
| 358 | FE6_MEDLEY | Binding Blade Medley | fe6 | 25 |
| 359 | SNOWGO | Snow Go | crash2 | 223 |
| 360 | FUTUREFRENZY | Future Frenzy | crash3 | 104 |
| 361 | LANKY_VICTORY | - | - | - |
| 362 | SILVERSURFER | Level 1 (Silver Surfer) | silversurfer | 144 |
| 363 | OPUS_13 | Opus 13 | castlevania_rob | 185 |
| 364 | FOURSIDE | Fourside | earthbound | 99 |
| 365 | BALLOONFIGHT | Balloon Fight | balloonfight | 13 |
| 366 | LIPS_THEME | Lip's Theme | paneldepon | 146 |
| 367 | CRASHBASH_LOADING | Crash Bash Loading Screen | crashbash | 56 |
| 368 | TREASURE_TROVE_COVE | Treasure Trove Cove | banjokazooie | 261 |
| 369 | STAGESELBM64 | - | - | - |
| 370 | OLDKINGCOAL | Old King Coal | banjo2 | 179 |
| 371 | SONIC_R | - | - | - |
| 372 | GREENGARDEN | Green Garden | bomberman | 122 |
| 373 | BLUE_RESORT | Blue Resort | bomberman | 27 |
| 374 | LOST | Lost | dream | 149 |

## 4. Hooks em funções vanilla

| Função (RAM) | Uso | Fonte |
|---|---|---|
| `0x80020AB4` | BGM play (a1 = BGM ID) | `BGM.asm:21` |
| `0x80020A74` | BGM stop | `BGM.asm:25` |
| `0x80020B38` | BGM volume (hook do volume mestre) | `BGM.asm:32`, `:35-53` |
| `0x800269C0` | FGM/SFX play (a0 = fgm_id) | `FGM.asm:165-182`; usado em `PokemonAnnouncer.asm` (ex.: `:105`, `:151`), `SFXReplace.asm:208` |
| `0x80020E64` | FGM volume; patch em `0x21A64` | `FGM.asm:188`, `:193-205` |
| `0x8039076C` | checagem de botão no sound test | `FGM.asm:108` |
| `0x80002CA0` | dmaCopy vanilla usado para recarregar o bank map | `midi.asm:1259` |
| `0x8001F548` (patch em ROM 0x20148) | recarga do bank map no boot | `midi.asm:1252-1254` |
| `0x800DF864` (patch 0x5B064) | `sfx_replace` (passos/pouso) | `SFXReplace.asm:19-21` |
| `0x80144470` (patch 0xBEEB0) | `sfx_replace_down_bounce` | `SFXReplace.asm` |
| `0x800C86C4` (patch 0x440A4) | correção de pan do hit SFX | `Stereo.asm` |
| `0x801137C4` (patch 0x8EFC4) | rotina do anunciador Pokémon | `PokemonAnnouncer.asm:1741` |
| `0x800472D0` | RAM do ctl de música (INST_CTL_TABLE_PC) | `midi.asm:759` |
| `0x8004D9F0` | RAM do ctl SFX (CTL_TABLE_PC) | `FGM.asm:258` |

## 5. Pendências e riscos

- `main.asm:320` inclui `src/MIDI.asm` e `FGM.asm:11` inclui `MIDI.asm`, mas o arquivo é `src/midi.asm` (minúsculo). Funciona no macOS case-insensitive e quebra em Linux case-sensitive.
- Tamanho exato da extensão SFX (blocos de predictors, bank table e raw novos) não foi medido: exige montar o asm com armips. Só os ponteiros âncora foram verificados.
- Não há `.ai-memory.toml` no BattleShip e o escopo `JRickey/BattleShip` não existe no servidor de memória; as regras do projeto não foram carregadas. As regras globais foram lidas.
- `piano` (`FGM.asm:1180-1184`) e demais personagens usam `.aifc` de `src/<Personagem>/sounds/`; os raw entram pelo mesmo `add_sound`.

