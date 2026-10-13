# Smash Remix 2.0.1 — mapa para o port (2026-10-12)

Resultado de um mapeamento somente-leitura feito por 5 subagentes (Haiku) e conferido por amostragem.
Nada aqui implementa suporte ao Remix; é o inventário do que precisa mudar.

Legenda: **[V]** = reconferido direto nos ROMs nesta sessão; **[A]** = relatado por agente, não reconferido;
**[?]** = incerto / não investigado.

ROM analisado: `baserom.remix.z64` (gitignored, copiado de `~/Downloads/Smash Remix 2.0.1.z64`),
SHA1 `13716381ed5e29be2606e0f4724b18fd00789d04`.

## Veredito

O Remix **não é uma variante de dados do US** (como o JP). É um patch binário de MIPS (injeta código em ROM
expandida) + milhares de arquivos novos. Suportá-lo exige quatro frentes independentes:

1. variante de build/extração (`remix`) com tabela de arquivos própria;
2. reconciliar a decomp C com as mudanças de código do Remix;
3. roster expandido (68 lutadores novos sobre os 27 originais) e tabelas indexadas por fkind/gkind/BGM;
4. orçamento de memória/armazenamento no Switch.

Estratégia já indicada pela sessão de pesquisa do opencode (ai-memory `BattleShip`,
`sessions/5e847c2f-…`): tratar o Remix como **mod de código e conteúdo integrado ao port**, não como ROM
expandida consumida diretamente. Fonte pública do Remix: `github.com/JSsixtyfour/smashremix` (assembly).

## 1. ROM

| Item | Vanilla US | Remix |
|---|---|---|
| Tamanho | 0x1000000 (16 MiB) | **0x3E80000 (62,5 MiB)** [V] — não 64 MB; nada em 0x3E80000..0x4000000 |
| Nome / país | `SMASH BROTHERS` / `NALE` | `SMASH REMIX` / **`NALE`** (mesmo código de país) [V] |
| Tabela reloc | 0x1AC870, 2132 arquivos [V] | 0x1AC870, **5455 arquivos** (ids 0..5454) [V] |
| Início dos dados | 0x1B2C6C [V] | **0x1BC830** [V] (tabela cresce 3323×12 B) |
| Fim dos dados | 0xAC7340 [V] | 0x24A42DA [V] |
| Arquivos comprimidos (VPK0) | 499 [V] | 1401 [V] |

- **`0x41F08` não é a tabela de arquivos** [V]. O word 0 é só o endereço da tabela (`0x001AC870`) e os bytes
  são idênticos nos dois ROMs. O item "R0" do handoff original estava errado nesse ponto.
- Dos 2132 ids vanilla: 1 igual (id 0), 2064 com conteúdo idêntico mas **offset deslocado**, 67 com conteúdo
  alterado; ids 2132..5454 são **novos** (3323, dos quais 2408 não comprimidos) [A].
- 2942 entradas terminam além de 16 MiB (primeiro id 2513) [A].
- Maiores novos/alterados por tamanho descomprimido: 2701 (260 KB), 167 alterado (253 KB; vanilla 168 KB),
  2564, 5394, 2565, 3684, 3779, 4341, 17 alterado (163 KB), 4496 e a série 2163..2232 (158 928 B cada) [A].
- **CORREÇÃO (2026-10-12, conferido por id):** os ids vanilla 0..2131 **são estáveis**: 2065 têm o mesmo
  conteúdo no mesmo id e 67 foram editados no mesmo id; só o offset de dados desloca (a tabela cresceu).
  `reloc_data.us.h` serve como está para 0..2131; ids 2132..5454 são novos. O "mapa de ids por conteúdo"
  (R1 antigo) não é necessário. Vale também: VPK0 do Remix pode ter tamanho de header não múltiplo de 4
  (ex.: id 5439 = 11833 B vs tabela 11832); o Torch agora decodifica no tamanho do header e trunca.

Scripts/CSVs gerados pelos agentes ficam em
`/private/tmp/claude-501/-Users-lucas-Projetos-Pessoal-BattleShip/24e8d755-6d25-4e59-a145-c4b93cdfd650/scratchpad/remix-map/`
(`rom-table/dump_reloc_tables.py`, `rom-table/*.csv`, `codediff/*`). Diretório temporário: copiar o que for
virar ferramenta para `tools/` após revisão.

## 2. Como o Remix altera o código [A, stub conferido [V]]

- Diff 0..0x1000000: 14,4 MB diferem (86 %); a maior parte é **relocação de dados**, não edição.
  Dos 2158 itens dos yamls, 2077 são idênticos em offset deslocado (1783 com delta `+0x93F2E`); 72 não
  casaram (MNMain, MNTitle, NMarioMain, NYoshiModel, LinkModel, `S1_music_sbk`, `fgm_tbl`, …) [?].
- Imagem principal (0x1000..0x1AC870): 6714 words diferentes em 1878 clusters. 1704 são J/JAL;
  **1636 apontam para 0x80400000..0x805A0000** (hooks em código expandido).
- **Loader do código expandido** [V]: em ROM 0x1234 o vanilla tem `jal 0x80002CA0`; o Remix troca por
  `j 0x80000438`. O stub em ROM 0x1038 (RAM 0x80000438) refaz o DMA original, chama `0x80002CA0` com
  ROM `0x02C00000` → RAM `0x80400000`, tamanho `0x1A0000`, e salta para `0x8046F368`.
  Mapeamento: `RAM = ROM - 0x02C00000 + 0x80400000`. Um segundo DMA dentro dele carrega ROM 0x02C11170 → RAM
  0x80411170 (0x128 bytes).
- Layout da região expandida: usada 0x1000000..0x2940000; padding 0xFF até 0x2C00000; código de hooks
  0x2C00000..0x2DA0000; zeros; usada 0x3000000..0x3DF0000.
- **Consequência:** o port compila C nativo; o código de hook do Remix (~1,7 MB de MIPS) precisa ser
  **reimplementado em C**, não executado. Não há mapa de linker do Remix no repo; a pesquisa do opencode
  encontrou 86 `.asm` em `src` do repo público, e `Character.asm` declara 68 personagens novos.
- [?] Se `0x80002CA0` é exatamente `syDmaReadRom`, o caminho de retorno de `0x8046F368`, e se
  0x1000000..0x2940000 / 0x3000000..0x3DF0000 são carregados por DMA.

## 3. Pipeline de extração e build [A]

| Componente | Onde | O que muda para `remix` |
|---|---|---|
| Mapa hash → config | `config.yml:8-32` | 3ª entrada com o SHA1 acima; `path: yamls/remix` |
| Seletor de tabela do Torch | `torch/src/factories/ssb64/RelocFactory.h:31-44` | **Caso novo**: hoje `NALE` cai no `default:` (US) e leria dados em 0x1B2C6C |
| Geradores | `tools/generate_yamls.py:13-16,38-40,544-547`, `generate_reloc_stubs.py:60-62`, `generate_reloc_table.py:28-30` | linha `remix` (endereço 0x1AC870, 5455) |
| Nomes por id | `tools/generate_yamls.py` (mapas de ordinais US) | remapa por conteúdo (ver §1) |
| CMake | `CMakeLists.txt:120-124,647-694,739-743,1319-1362,1411` | `SSB64_VERSION=remix`, `REGION_REMIX`, nome do app/o2r **distinto** (senão sobrescreve o US), excluir hires, CSS-assets aponta p/ ROM US |
| Seletores de região | `decomp/include/reloc_data.h:14-19`, `port/resource/RelocFileTable.h:15-20` | ramo `REGION_REMIX` (hoje `#error`) |
| Guardas `#if REGION_US/JP` | `decomp/src/sys/main.c`, `scheduler.c`, `taskman.c`, `dma.c`, `audio.h`, `piint.h`, `port/stubs/port_diag_stubs.c` | revisar cada um |
| Primeira execução | `port/first_run.cpp:30-44,305-315`, `port/port.cpp:1091-1092` | strings/ROM base/regiões do Remix |
| IDs literais | `port/bridge/lbreloc_byteswap.cpp:975,985,1719` (`file_id == 104`), `port/css_icons/port_css_stage_assets.cpp:781` | verificar contra ids Remix |
| Android | `port/android_torch_bridge.cpp:44-54` | fora de escopo |

No Switch o `.o2r` vem do SD (sem Torch no console), então a extração Remix precisa rodar no desktop.

## 4. Superfície de jogo [A]

**Lutadores** — `port/fighter_registry.{h,cpp}` já suporta `fkind` além de 27 (vetor indexado por fkind);
só o seeding vanilla registra algo (`decomp/src/ft/ftport.c:149-214`, `PORT_VANILLA_FKIND_COUNT 27`).
Nenhum lutador sintético (Crash) está registrado no repo — o código de conteúdo ficaria num mod externo.
Vários acessores `port_fighter_*` não têm ponto de chamada ainda (team_costume, remix_1p_end_bgm, kirby_hat,
crowd_chant, action_string, costume_count, charge_smash_frames, default_costumes).
Limites a ampliar: `nFTKindEnumCount = 27` (`ftdef.h:1138`), `dFTManagerDataFiles[EnumCount+1]` (`ftdata.c:129`),
`gSCManagerFighterFileSizes` (`scmanager.c:66`), `p_script[2][EnumCount]` (`fttypes.h:693`), tabelas de CSS e
congra de 12 posições (`mnplayersvs.c:4478`, `mncongra.c:24,273`), tabela de cópia do Kirby (27).
Comentário velho: `ftmanager.c:745` diz EnumCount 28; o valor real é 27 (Null = 28).

**Stages** — 41 gkinds (0..40); `dMPCollisionGroundFileInfos[41]` (`mpcollision.c:137-180`); ids de arquivo são
constantes de compilação (`reloc_data.us.h`); `dGRMainSetupProcMakeList[9]`; CSS com 2 páginas × 10 slots
(`mnmaps.c`). Tabelas gkind curtas já causaram bugs
(`docs/bugs/inishie_efgrounddatas_array_short_2026-05-02.md`,
`docs/bugs/training_mode_page2_stage_oob_2026-08-31.md`).

**Cenas/modos** — `SCKind` (~62 entradas), `dSCManagerOverlays` (65), despacho em `scManagerRunLoop`
(`scmanager.c:1083`). Modo novo = `SCKind` + slot de overlay + `case` + opção de menu
(`mnmodeselect.c:164-446`).

**Música** — 47 BGMs (0..46) em `gm/gmsound.h:30-79`; mapa id→faixa está dentro do `S1_music_sbk`
(0xB277B0, `yamls/us/audio.yml`); ids duplicados como literais em `port/enhancements/ShuffleMusic.cpp:12-39`
e `MusicSelection.cpp:14-17`. O `S1_music_sbk` está entre os 72 itens alterados no Remix.

**Mods existentes** (`docs/modding.md`) — TinyCC + funchook, só desktop (CMake já desliga em Android/UWP;
disponibilidade no devkitA64 não verificada). Mods adicionam código, eventos e arquivos; **não** adicionam
linhas de gkind, ids de cena ou BGMs sem mexer no decomp. `tools/merge_mod.py` não é carregador de mods.

## 5. Switch

Dados armazenados do Remix: ~36,6 MB (offset do sentinela 36 600 490) contra ~9,5 MB no vanilla (9 520 852).
Sem medição de RAM/armazenamento no Tegra; sem avaliação de TinyCC/funchook no devkitA64 [?].

## 6. Plano em fases (proposta)

- **R0 — identidade e extração**: caso `remix` no `RelocFactory` (5455 @ 0x1AC870, dados 0x1BC830); entrada no
  `config.yml`; extrair um `.o2r` completo. Critério: Torch lê os 5455 arquivos sem erro e os 1401 VPK0 descomprimem.
- **R1 — yamls Remix** (feito: `tools/generate_yamls.py --version remix`; ids 0..2131 = US). Resta localizar
  áudio/partículas fora da tabela (os 72 itens) e analisar os 67 arquivos alterados.
- **R2 — variante de build**: `SSB64_VERSION=remix`, `REGION_REMIX`, nome de app/o2r distinto, guardas `#if REGION_*`.
  Critério: compila e chega ao menu (sem roster novo).
- **R3 — reconciliação de código**: comparar fonte do Remix (assembly público) com `decomp/` e portar mudanças
  de lógica em C, começando pelos 36 arquivos já identificados pela pesquisa do opencode.
- **R4 — roster**: expandir fkind/EnumCount e as tabelas da §4; registrar lutadores no `fighter_registry`; CSS.
- **R5 — stages, música, modos**: gkinds novos, BGMs, SCKinds.
- **R6 — Switch**: orçamento de memória, scripting estático (sem TinyCC), validação em hardware.

R0 e R1 não dependem de R3 e podem ser feitos primeiro.

## 7. Estado da implementação (2026-10-12, fim da sessão 1)

Feito e verificado por compilação/execução:

- **R0**: Torch extrai os 5455 arquivos + 26 segmentos de áudio/partículas (`yamls/remix/`), `.o2r` de 29 MB.
  VPK0 com tamanho de header não alinhado (id 5439) tratado em `torch/.../RelocFactory.cpp`.
- **R2**: `SSB64_VERSION=remix` é o padrão; `REGION_REMIX` + `REGION_US` juntos; app `BattleShip-Remix`.
  Compila para macOS (`build/BattleShip-Remix`) e Switch (`build-switch/BattleShip-Remix.nro`).
- **Execução desktop**: com o áudio vanilla no lugar, o jogo roda 60 s sem crash. **Não validado visualmente.**
- Correções de infraestrutura encontradas no caminho: shader Metal do LUS (`thread const` em texture/sampler/int
  não compila no Metal atual), `portSignalHandler` só existe no Windows, SDL2 contido sem render.

Descobertas sobre o áudio (bloqueiam o som do Remix; R5):

- `B1_sounds1_ctl` (música): o Remix **zera** o bank original em `0x6660` e aponta o header para `0x3c9ea0`
  (fora do blob de 26 400 B). O banco real fica num blob expandido, ainda não localizado no ROM.
- `B1_sounds2_ctl` (SFX): instrumento `0x18` declara **1137** sons (vanilla 322); os 322 primeiros são idênticos,
  os 815 novos têm structs em `ctl+0x3c5810..` com campos `0xFFFFFFFF` (placeholders preenchidos em runtime).
- `portAudioLoadAssets` agora valida offsets contra o tamanho do blob (`fits`). Um banco "silencioso" falso
  derruba o sintetizador (`n_env.c`), então não é alternativa.
- **Provisório de teste**: `build/BattleShip-Remix.testaudio.o2r` = `.o2r` do Remix com as 8 entradas `audio/*`
  trocadas pelas da ROM US (copiar sobre `build/BattleShip-Remix.o2r`; o build restaura o `.o2r` puro).

Pendente (R3/R4/R5): offsets fixos dentro de arquivos alterados (`o_attributes` dos `*Main`: MMario 0x2A8→0x2BC,
NMario 0x298→0x2B0, NFox 0x2A4→0x2BC, NKirby 0x2C0→0x2F8; símbolos `ll*` de modelos 300–313 re-layout),
roster/menus (append-only, offsets vanilla válidos), música e banco de SFX do Remix.
