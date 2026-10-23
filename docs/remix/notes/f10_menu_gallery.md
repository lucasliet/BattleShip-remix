# F10 Remix Gallery (ROADMAP T5.3): pesquisa e primeiro port

Fonte: `.toolchain/pkgs/smashremix-src/src/Gallery.asm` (1491 linhas), mais `Toggles.asm` (view_gallery, 2330),
`Boot.asm` (VICTOR_ID 0x801322E0), `SinglePlayer.asm` (set_ending_image, 2328-2340), `File.asm` (VICTORY_IMAGE ids).

## O que a Gallery mostra

- Uma imagem de vitória por personagem, a mesma tela do Congratulations (cena 0x37). A Gallery não tem ID de cena próprio:
  força `Global.screen.CONGRATULATIONS` enquanto `status.active` é verdadeiro (`screen_change_`, Gallery.asm 1265-1290).
- Cada imagem tem música própria: `bgm_table[gallery_index]` aponta para um array `[count, bgm ids...]`. A tecla A
  troca de faixa dentro do array (`_a_press_normal`, 1077-1095). Se o índice de música for 0, toca a BGM da galeria
  (`MIDI.id.GALLERY`, 0x2F+ no `bgm_to_gallery_table`).
- Modo idle (START): embaralha `idle_table` (40 entradas) e avança sozinho a cada `IDLE_TIME(150)` s, com música aleatória
  (`populate_idle_table_`, 1190; `main_`, 952-996).
- Drum kit: L, R e Z tocam sons de `fgm_drum_kit` (18 pares, `DRUM_KIT_ENTRY_COUNT(18)`, 767-793).

## Paginação e controles (`main_`, 929-1186)

| Entrada | Efeito |
|---|---|
| Stick X >= 40 / <= -40 (qualquer jogador) | próxima / anterior imagem, com fade rápido (`FADE_LENGTH_NORMAL` 45, `check_sticks_` 1238) |
| D-pad direita / esquerda (held) | próxima / anterior, fade `FADE_LENGTH_QUICK` (4) |
| A | troca faixa de música da imagem atual |
| B | sai da galeria (`_leave_gallery`, 1156): grava o índice em `Toggles.entry_view_gallery + 4`, restaura `normal_options`, volta à tela anterior `status.previous_screen` e toca BGM do menu |
| START | liga/desliga idle (`status.idle`) |
| L / R / Z | drum kit |

Wrap de paginação: `_cycle_image` (1129) faz wrap em 0..`id_table.SIZE - 1` (40).

## Estado (`status`, Gallery.asm 19-35, endereço = `Gallery.status`)

| Offset | Campo | Significado |
|---|---|---|
| 0x00 | active | 1 enquanto a galeria está aberta |
| 0x01 | index | índice de imagem (0..39) |
| 0x02 | fade_length | duração do fade (45 normal, 4 rápido, 100 idle) |
| 0x03 | music_index | 0 = BGM da galeria, >0 = faixa da imagem |
| 0x04 | idle | 0 desligado, 1 idle modo 1 (sequência), 2 idle modo 2 (aleatório) |
| 0x05 | idle_index | posição em `idle_table` |
| 0x06 | idle_timer | contador (halfword) |
| 0x08 | previous_screen | cena de retorno (Data 0x3A ou Options 0x39) |

## Tabelas e IDs de arquivo

- `id.*` (0x00..0x27, 40 entradas): índice de galeria, Gallery.asm 33-74. Ex.: MARIO 0x00, DK 0x01, ... PIANO 0x26, REMIX 0x27.
- `id_table` (Gallery.asm 445-487, 40 bytes): índice de galeria -> `Character.id`. A ordem é a ordem de exibição:
  MARIO, DK, LINK, SAMUS, YOSHI, KIRBY, FOX, PIKACHU, LUIGI, CAPTAIN, NESS, JIGGLYPUFF, GND, YLINK, DRM, FALCO, WARIO,
  BOWSER, WOLF, CONKER, MTWO, MARTH, SONIC, SHEIK, MARINA, DEDEDE, GOEMON, BANJO, CRASH, PEACH, DSAMUS, LUCAS, SLIPPY,
  ROY, DRL, LANKY, DRAGONKING, EBI, PIANO, BOSS.
- `bgm_to_gallery_table` (507-763): BGM id -> índice de galeria (de `add_bgm_to_gallery`, cerca de 280 entradas).
- Arquivos de imagem: a imagem de vitória de cada personagem é o par BOTTOM/TOP de `File.asm` (56 constantes
  `*_VICTORY_IMAGE_*`, por exemplo MTWO 0x0EE8/0x0EE9, BOWSER 0xF02/0xF03, SINGLEPLAYER 0x92D/0x92E). Os personagens
  remix usam `SinglePlayer.custom_victory_file_table` (SinglePlayer.asm 2303, indexado por `(char_id - 12) * 2`).
- Offset dentro do arquivo: 0x20718 (ver `port/fighter_registry.h` e `mncongra.c` 291), o mesmo para BOTTOM e TOP.
- Rodapé da galeria no arquivo de Data: `OS.read_word(0x80133164) + 0x4EE8` (botão da Galeria, Gallery.asm 1382),
  altura 22.

## Como é aberta

1. **Data menu (cena 0x3A)**: `data_menu.add_button_` (Gallery.asm 1339) cria um 4º botão "Gallery" (índice 3),
   posição y 0x4337 (183.0), x 0x4214 (37.0) com o texto no x 0x42A0 (80.0). O cursor e o wrap ganham 4 posições
   (`cursor_`, 1406-1489). A entrada de A em `0x80132A1C` (índice 3) grava `status.active = 1`, `index = 0`,
   `music_index = 0`, `idle = 0`, `previous_screen = 0x3A` e vai para CONGRATULATIONS.
2. **Other Screens (Toggles)**: `Toggles.asm 2669` `entry_view_gallery` (INT 0..39, "View Gallery"). `view_gallery`
   (2330) lê o índice salvo, ativa a galeria e grava `previous_screen = Global.screen.OPTION`. Esta entrada fica no menu
   de Remix Settings, que não é redesenhado no port.

## O que foi portado nesta rodada

- `decomp/src/mn/mncommon/mnremixgallery.c/.h` (novo): estado da galeria, paginação (L/R, D-pad, stick com borda),
  B para sair (cena anterior), recarga da cena do Congratulations a cada página.
- `decomp/src/mn/mncommon/mncongra.c`: sob `#ifdef REGION_REMIX`, `mnCongraActorFuncRun` entrega o input à galeria
  (sem o A/B/START de avanço do vanilla) e `mnCongraStartScene` troca `sMNCongraFighterKind` pela entrada da galeria.
- `decomp/src/sc/scmanager.c`: `SSB64_START_GALLERY=1` (sob PORT e REGION_REMIX, junto de `SSB64_START_SCENE`) ativa a
  galeria com retorno a `nSCKindData` e define `scene_curr = scene_prev = nSCKindCongra`.

Verificação de sintaxe: `scripts/remix/syntax-check.sh` OK nos três arquivos. Nenhum build ou jogo foi executado (regra
do brief: quem roda é o lead). Os dois avisos `fb16`/`fb32` em `mncongra.c` já existiam.

## Como verificar (lead)

- Regenerar o cmake (o `GLOB_RECURSE` pega `mnremixgallery.c`).
- `SSB64_START_GALLERY=1 <binário remix>`: deve abrir a imagem do índice 0 (Mario). Stick direito ou L/R deve mudar a
  imagem (um reload por página). B deve voltar à tela de Data (`nSCKindData`).
- Conferir se o reload de cena Congratulations -> Congratulations não trava na troca de página (ponto de risco: nunca
  testado; a transição vanilla só recarrega do Congratulations para o Title).

## O que falta (REMIX-TODO)

- `REMIX-TODO(gallery-remix-fighters)`: 28 das 40 entradas (índices 12..39, personagens remix, fighter kinds 13+).
  Falta o mapeamento Character.id -> fkind da Remix (`port_fighter_remix_1p_ending_image_file_id` é indexado por fkind).
  Opção: ler `SinglePlayer.custom_victory_file_table` do RAM image (exige símbolo em `EXTRA_SYMBOLS` de
  `tools/gen_remix_actions.py`).
- `REMIX-TODO(gallery-data-entry)`: o botão "Gallery" do Data menu (4º item) não foi portado. O asm o adiciona na posição
  índice 3 do Data menu (`mndata.c`), com cursor, wrap e posição do texto. Não é um "sexto" item como sugerido no brief;
  o asm adiciona o quarto.
- `REMIX-TODO(gallery-audio)`: música por imagem (A), BGM da galeria, drum kit (L/R/Z) e FGM de troca.
- `REMIX-TODO(gallery-idle)`: modo idle (START), tabela embaralhada e avanço automático.
- `REMIX-TODO(gallery-fade)`: os fades do asm (45/4 frames) não são aplicados; o port troca de página sem fade
  (via reload da cena).
- Entrada em Remix Settings > Other Screens (`view_gallery`): fica para o menu de Remix, que não é redesenhado.
