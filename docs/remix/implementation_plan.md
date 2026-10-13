# Plano de implementação do Smash Remix no port (2026-10-12)

Base: relatórios em `docs/remix/` (fighters, menus, áudio, stages/moveset, hooks A–L e M–Z, extração de tabelas e de áudio).

## Arquitetura

O Remix é um mod de código. O port **não executa o MIPS dele**: os dados são extraídos do ROM no Torch (vão para o
`.o2r`) e a lógica é reescrita em C. O port já tem o mecanismo para lutadores "sintéticos" (`fkind >= 27`) em
`port/fighter_registry.{h,cpp}` e `decomp/src/ft/ftport.c`, que espelha o modelo do Remix ("SR" nos comentários).
Portanto **não crescemos `nFTKindEnumCount`**: os 70 lutadores novos entram pelo registry.

Mapeamento de ids: id do Remix `0x00..0x1A` = fkind vanilla 0..26. `0x1B` (RANDOM) e `0x1C` (clone não selecionável de
Mario) não são lutadores. `0x1D..0x60` (68 lutadores) → fkind do port `29..96` (`fkind = remix_id`, identidade; 27 e 28 = `nFTKindEnumCount` e `nFTKindNull` ficam sem lutador).

## Fases

| Fase | Entrega | Verificação |
|---|---|---|
| F1 | Torch gera os blobs de áudio do Remix (`audio/*`) e `remix/fighters` | saída idêntica byte a byte às ferramentas Python |
| F2 | Port carrega o blob de lutadores e registra os 68 sintéticos no registry (clone do pai, com dados do Remix) | build desktop; log lista 68 lutadores |
| F3 | CSS/menus com os lutadores novos (tabelas CSS por slot, retratos, nomes) | CSS mostra os novos |
| F4 | Corrigir os sites do decomp que indexam arrays por `fkind` sem guarda (auditoria `decomp_fkind_audit.md`) | luta com lutador novo sem OOB |
| F5 | Ações próprias dos lutadores novos (MIPS → C), por lutador | luta jogável |
| F6 | Stages (215), música (375 BGMs), modos | |
| F7 | Switch: RAM/armazenamento, validação em hardware | |

## Formatos (contratos entre Torch e port)

Todos os inteiros são **little-endian** (o blob é lido por código do port, não pelo jogo N64).

### `remix/fighters` (recurso BLOB no `.o2r`)

```
Header (16 B): u32 magic = 'RMXF' (0x46584D52), u32 version = 1, u32 count (= 97), u32 reserved = 0
Records: count x 0x80 B, na ordem do struct table (índice = id do Remix)
  +0x00 u32 remix_id
  +0x04 u32 file_ids[9]        // main, mainmotion, submotion, model, shieldpose, special1..4 (mesma ordem de FTData)
  +0x28 u32 file_main_size
  +0x2C u32 o_attributes
  +0x30 u32 file_anim_size
  +0x34 u32 mainmotion_count
  +0x38 u32 mainmotion_off     // offset em bytes a partir do início do blob de FTMotionDesc[]
  +0x3C u32 menu_count
  +0x40 u32 menu_off           // idem
  +0x44.. reservado (zeros) até 0x80
Depois dos records: arrays FTMotionDesc, 12 B cada: u32 anim_file_id, u32 command_offset, u32 flags
```

`FTMotionDesc` = `{ anim_file_id, offset, flags }` do decomp (`decomp/src/ft/fttypes.h`); no ROM os três campos são
big-endian e o Torch converte para little-endian. Origem dos dados: struct table em ROM `0x92610` (97 ponteiros RAM para
structs de 0x78 B). Tradução RAM→ROM: `RAM - 0x80084800` (dados vanilla), `RAM - 0x7D800000` (segmento expandido
`0x80400000+`), `RAM - 0x80288A20` (janela de overlay `0x80390000..0x80400000`). Referência: `tools/remix_dump_fighters.py`.

### `audio/*` (BLOBs, mesmos nomes de hoje)

`audio/B1_sounds1_ctl`, `audio/B1_sounds1_tbl`, `audio/B1_sounds2_ctl`, `audio/B1_sounds2_tbl`, `audio/S1_music_sbk`:
os arquivos reconstruídos por `tools/remix_dump_audio.py` (ctl = imagem de RAM: blob vanilla no começo, imagem de
código em `RAM - 0x7D800000`, zeros no meio). Assim o `audio_bridge.cpp` continua lendo os mesmos nomes.
`fgm_*` e `particles` continuam como segmentos simples do `yamls/remix/audio.yml`.

## Dados vindos do código-fonte do Remix (não do ROM)

`port/remix/remix_roster_data.h`: por lutador, `remix_id`, nome, `parent` (vanilla), `add_actions`, `num_costumes`,
`variant_type`, `sound_type`. Fonte: `.toolchain/pkgs/smashremix-src/src/Character.asm` (`define_character`),
`src/costumes/*.asm`. O código-fonte do Remix foi autorizado pelo usuário para uso neste projeto.

## Regras

- Dados derivados do ROM nunca vão para arquivos versionados (só para o `.o2r` e `build/`).
- Dependências só dentro de `.toolchain/` (ver `AGENTS.md`).
- Build com `-j 4`, caminhos absolutos no `cmake --build`.
