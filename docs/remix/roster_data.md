# Roster data gerado do código-fonte do Remix

Arquivo gerado: `port/remix/remix_roster_data.h` (97 entradas, ids 0x00..0x60, `kRemixRoster`).
Gerador: `tools/gen_remix_roster.py` (python3 do sistema, só stdlib). Regerar com:

    python3 tools/gen_remix_roster.py [caminho/para/smashremix-src]

Padrão: `.toolchain/pkgs/smashremix-src`. Lê `src/Character.asm` (tabela `scope id`,
`variant_type`/`sound_type`, chamadas `define_character`) e `src/costumes/*.asm`
(`NUM_COSTUMES`, `NUM_EXTRA_COSTUMES`, `register_extra_costumes_for_char`).

## Decisões

- `parent_remix_id` é o parent real do `define_character` (ids 0x00..0x0B). Vanilla e 0x1C ficam em 0xFF.
- `variant_type` usa o valor que o `define_character` grava na tabela (`table_patch_start`), não a tabela estática.
  Por isso 0x1B sai como SPECIAL (a tabela estática diz UNUSED).
- `has_define_character == false` marca slots sem `define_character` (vanilla e 0x1C). Nesses,
  `add_actions`, `file_*`, `attrib_offset`, `jab3` e `inhale_copy` são placeholders zero/NULL/false.
- Ids seguem a ordem do macro: RANDOM = 0x1B, 0x1C é o clone do Mario (pulado), depois 0x1D em diante.
  Cada comentário `// 0xNN - NOME` é conferido contra o id calculado (id divergente aborta).

## Lacunas conhecidas

- 24 entradas com `num_costumes == 0` (desconhecido): BOSS, METAL, RANDOM, NONE, FALCO, GND, DRM, DSAMUS,
  LUCAS, GBOWSER, PIANO, MARTH, SONIC, SANDBAG, SSONIC, DEDEDE, PEPPY, MLUIGI, EBI, DRAGONKING, CRASH, ROY,
  DRL, LANKY. Nenhum `costumes/*.asm` os registra diretamente.
- Labels de comentário divergentes no fonte (só aviso, o id confere): SSONIC (`SUPER SONIC`),
  MLUIGI (`METAL`), EBI (`EBISUMARU`).
- `file_*` são o texto do argumento: `File.X` ou literal hex (ex.: `0x0D9`), sem resolver o valor RAM.
- `kRemixRoster` é `static const` em header: cada unidade de tradução que incluir o header tem sua cópia
  (aviso `-Wunused-const-variable` em TUs que não usam a tabela).
