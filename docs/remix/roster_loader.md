# Carregador do roster Remix (`remix/fighters`)

Estado: em uso. O `.o2r` do Remix contém `remix/fighters` e o carregador registra os 68 lutadores sintéticos (78 fkinds
jogáveis com os vanilla).

Formato do blob e ids: ver "Contrato dos blobs" em `docs/remix/torch_factories.md`.

## Arquivos

| Arquivo | Responsabilidade |
|---|---|
| `port/remix/remix_roster.{h,cpp}` | Lê o recurso, valida o cabeçalho e os limites, monta os 68 sintéticos (ids 0x1D..0x60) e os registra. Não inclui headers do decomp. |
| `port/remix/remix_synth_ft.h` | API C com valores simples que cria e libera o `FTData` de um sintético. |
| `decomp/src/ft/ftport.c` | Implementa `port_fighter_synth_ft_create/destroy`: clona o `FTData` do pai e sobrescreve os campos do Remix. |
| `port/port.cpp` | Chama `remix_roster_load()` logo após `port_fighter_seed_vanilla()`, dentro de `#ifdef REGION_REMIX`. |

Por que a montagem do `FTData` fica no C: `fttypes.h` precisa de `ssb_types.h`, que não está no caminho de include do
port (C++). O `CMakeLists.txt` proíbe incluir `decomp/include` no port. O `ftport.c` já é o ponto de ligação entre
decomp e registry, então a construção ficou lá. Nenhum header do decomp entrou em código C++.

## Fluxo

1. `loadBlob`: `Ship::Context` → `LoadResource("__OTR__remix/fighters")`. Ausente ou não-Blob: `warn` e `false`.
2. Cabeçalho: tamanho >= 16, magic `RMXF`, versão 1, `count == std::size(kRemixRoster)` (97), e os 97 registros
   cabem no blob.
3. Para cada id 0x1D..0x60, lê o registro (0x80 B, little-endian) e confere `remix_id` contra o índice e contra
   `kRemixRoster[id].remix_id`.
4. Pai: `parent_remix_id` deve ser vanilla (< 27). Pai vanilla usa fkind = id (identidade). `REMIX_ID_TO_PORT_FKIND`
   (`id - 2`) vale só a partir de 0x1D, então não é usado para pais.
5. Arrays de movimento: `mainmotion` e `menu` (submotion). Cada linha tem 12 bytes. Offset relativo ao início da
   região de movimentos (`16 + 97 * 0x80`). Limites verificados antes de ler.
6. Tudo ou nada: todos os 68 são montados antes de qualquer `port_fighter_register`. Se um falhar, nada é registrado e
   o registry vanilla fica intacto. Chamadas repetidas com sucesso não refazem o trabalho.

## Fallback de campos (sintético = clone do pai)

FTData (`port_fighter_synth_ft_create`):
- Copiado do pai: `particles_script_lo/hi`, `particles_texture_lo/hi`, e todo campo não listado abaixo.
- Vindo do Remix: `file_*_id` (9), `file_main_size`, `o_attributes`, `file_anim_size`, `mainmotion`, `submotion`
  e seus contadores.
- Slots de runtime próprios (não apontam para o pai): `p_file_*` (9), `p_particle`, `submotion_array_count`.
  Ficam dentro do bloco alocado pelo C e são liberados por `port_fighter_synth_ft_destroy`.

FighterDescriptor:
- Cópia completa do descritor do pai. Herdados sem mudança: special descs e handlers, entry appear, opening descs,
  costume row, scale, skeleton anim base, yoshi egg, FGMs, computer attack list, CSS (motion/attack1), spotlight,
  hooks SR (shield hitlag, ECB, kirby hat, crowd chant, action strings, AI routines, recovery/weight), BGM/imagem de
  final 1P, default costumes, charge smash, custom capture (action/DK), resultados (announce FGM, `results_name_lx/
  scale`, `wins_lx`) e emblema (`results_emblem_*`).
- Substituídos: `ft_data` (aponta para o FTData do sintético) e `results_name` (nome do roster). Sem isso o resultado
  mostraria o nome do pai.

## Lacunas conhecidas

- Ações e especiais próprias do Remix não existem ainda (fase F5). Os handlers de special, descs e rotinas de IA
  continuam sendo os do pai. Um sintético joga como o pai, com o `FTData` e os movimentos do Remix.
- Código do decomp que indexa arrays por `fkind` sem guarda ainda pode ler fora do limite (fase F4,
  `decomp_fkind_audit.md`). O registry só cobre os acessores já redirecionados.
- `fp->fkind` vale o do sintético (27..94). Ramos `switch (fkind)` do decomp que não têm caso para ele caem no `default`
  (ex.: efeito de entrada em `ftPortVanillaEntryMakeEffect`, sem efeito para sintéticos).
- `particles_*` (scripts e texturas de partícula) são do pai. Sem dados próprios do Remix no blob.
- Retrato, nomes e tabelas CSS (fase F3) não estão ligados aqui.
- Suposições do formato, a confirmar com o Torch: (a) offsets de movimento são relativos à região de movimentos (depois
  dos 97 registros); (b) `menu_*` é o `submotion`; (c) `file_ids` seguem a ordem de `FTData`
  (main, mainmotion, submotion, model, shieldpose, special1..4).
- Sem teste automatizado de parsing: o blob de teste precisa vir do Torch, e o projeto proíbe testes com providers
  reais (não se aplica aqui, mas o recurso ainda não existe no `.o2r`).
