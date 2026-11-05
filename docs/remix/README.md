# Smash Remix no port: índice da documentação

**Plano e progresso: só em [`ROADMAP.md`](ROADMAP.md)** (tasks, milestones, posição atual, estimativa). Os demais arquivos desta
pasta são referência ou ferramenta de trabalho; nenhum guarda plano nem estado de progresso. [`parity.md`](parity.md) é a tabela
de paridade por arquivo `.asm` (gerada por `tools/gen_remix_parity.py`).

ROMs necessárias (com SHA), versão do Remix (2.0.1) e passo a passo dos builds desktop e Switch: [`build_and_roms.md`](build_and_roms.md).

Bugs corrigidos e decisões ficam em [`../bugs/README.md`](../bugs/README.md) (arquivos `remix_*`). Testes em jogo: skill
`remix-smoke-tests` (`.claude/skills/remix-smoke-tests/`).

## Referência do Remix (como ele funciona, lido do código-fonte)

| Arquivo | Conteúdo |
|---|---|
| [`fighters.md`](fighters.md) | Como o Remix adiciona lutadores: `define_character`, tabelas, arquivos, ações |
| [`gameplay_a_l.md`](gameplay_a_l.md), [`gameplay_m_z.md`](gameplay_m_z.md) | Inventário dos hooks (`OS.patch_start`) por arquivo `.asm`, com o mapa para as funções do decomp |
| [`menus.md`](menus.md) | CSS, menu do Remix (Menu/Toggles/Settings), telas novas, modos, título; lista completa de toggles |
| [`stages_and_moveset.md`](stages_and_moveset.md) | Tabelas de stage (`add_stage`, `function_table`, classes, hazards) e o framework de moveset e comandos custom |
| [`audio.md`](audio.md) | Sistema de áudio expandido: bancos ctl/tbl, sequências, SFX/FGM, layout em ROM, lista de BGMs |
| [`file_table_and_loader.md`](file_table_and_loader.md) | Tabela de arquivos (ids `File.asm`) e carregador do Remix |
| [`css_reference.md`](css_reference.md) | CSS: como o do port funciona e os dados do CSS do Remix (grade 3x10, tabelas, offsets) |
| [`modes_base.md`](modes_base.md) | Base comum dos modos de M6: modo da partida, Timed Stock e texto posicionado do HUD |
| `../../tests/remix/` | Testes de unidade headless das regras dos modos (12CB, nomes e regras dos modos 1P): `cmake --build build --target run_remix_mode_tests` |
| [`t9_1b_checklist.md`](t9_1b_checklist.md) | Lista da passada de escuta e jogo do usuário (T9.1b), com comandos e o que esperar |
| [`decomp_fkind_audit.md`](decomp_fkind_audit.md) | Inventário dos sítios do decomp indexados pela contagem de lutadores (124 entradas, risco por sítio) |

## Pipeline de dados (Torch e port)

| Arquivo | Conteúdo |
|---|---|
| [`torch_factories.md`](torch_factories.md) | Fábricas do Torch (`REMIXFIGHTERS`, `REMIXAUDIO`) e o contrato dos blobs `remix/fighters` e `audio/*` |
| [`fighter_table_extraction.md`](fighter_table_extraction.md) | Onde a tabela de lutadores está na ROM e as janelas RAM para ROM (`tools/remix_dump_fighters.py`) |
| [`audio_extraction.md`](audio_extraction.md) | Reconstrução dos bancos de áudio (`tools/remix_dump_audio.py`) |
| [`roster_data.md`](roster_data.md) | `port/remix/remix_roster_data.h` gerado de `Character.asm` (`tools/gen_remix_roster.py`) |
| [`roster_loader.md`](roster_loader.md) | Quem faz o quê no carregamento do roster (`remix_roster.cpp`, `ftport.c`) |

## Briefs de workers (instruções fixas para agentes Haiku)

| Arquivo | Escopo |
|---|---|
| [`f5_worker_brief.md`](f5_worker_brief.md) | Base: referências, regras de tradução MIPS para C, LP64, decodificação de `OS.copy_segment` |
| [`f5_item_worker_brief.md`](f5_item_worker_brief.md) | Item de lutador (interface, layout de arquivos, relatório) |
| [`f5_shared_patch_brief.md`](f5_shared_patch_brief.md) | Patches de `*shared.asm` em funções vanilla |
| [`f6_standard_item_worker_brief.md`](f6_standard_item_worker_brief.md) | Item padrão do Remix |
| [`f12_mode_worker_brief.md`](f12_mode_worker_brief.md) | Um passo do port de um modo Remix (M6: Stamina, Tug of War, KOTH, Tag Team, Smashketball, 12CB, 1P, Training) |
| [`f7_leftover_worker_brief.md`](f7_leftover_worker_brief.md) | Um grupo de `REMIX-TODO` |
| [`f7_hazard_worker_brief.md`](f7_hazard_worker_brief.md) | Hazard de um stage, com o primer de idiomas do asm e seus equivalentes no port |
| [`f8_toggle_worker_brief.md`](f8_toggle_worker_brief.md) | Um toggle de gameplay (API `gmRemixToggle`, notas, como o lead testa) |
| [`f9_announcer_worker_brief.md`](f9_announcer_worker_brief.md) | PokemonAnnouncer: o núcleo e os grupos de ganchos |
| [`f9_damage_worker_brief.md`](f9_damage_worker_brief.md) | Ações de dano do `Damage.asm`: stun/flashbang e bury/plunge |
| [`f10_css_worker_brief.md`](f10_css_worker_brief.md) | Grade Remix de 30 retratos em uma tela de CSS (1P, treino, bônus), copiando o padrão do VS |
| [`f10_panel_worker_brief.md`](f10_panel_worker_brief.md) | Uma opção do painel por porta do CSS (`gmRemixPanel`, registro `port/remix/remix_player_panel.*`) |
| [`f10_menu_worker_brief.md`](f10_menu_worker_brief.md) | Um menu ou tela do Remix (botão, página, tela de dados) no código dos menus vanilla |
| [`f11_mode_research_brief.md`](f11_mode_research_brief.md) | Pesquisa de um modo de jogo do Remix: relatório com fluxo, ganchos, estado, HUD e plano de port em passos |

Os briefs mandam gravar as notas do trabalho em [`notes/`](notes/): uma por lutador/golpe (`<id>_<NOME>[_<MOVE>].md`), item
(`item_<Nome>.md`), patch compartilhado (`shared_*.md`) e tarefa (`f7_*.md`, `f8_*.md`, `f9_*.md`, `f10_*.md`, `f11_mode_*.md`). São os desvios, as incertezas e os
`REMIX-TODO` de cada porte; o código do decomp as cita. Notas das Fases 4 e 5 do M11: `f16_css_panel_menu.md` (menu do painel na
CSS), `f17_vs_css_modes.md` (CSS dos modos e fila do Tag Team), `f18_classic_sonic.md`, `f19_koth_smashketball.md`,
`f20_tagteam_stamina.md`, `f21_1p_leftovers.md`, `f22_teams_yellow.md` e `i7_part_image.md` (costumes extras por parte).

## Regras desta pasta

- Referência descreve como as coisas são; estado e plano vão para o `ROADMAP.md`. Um doc novo entra neste índice.
- Nenhum byte derivado da ROM entra aqui (valores vêm do código-fonte do Remix).
- `torch/` e `libultraship/` não citam estes docs (repositórios independentes); o `decomp/` pode citar `../bugs/` e `notes/`.
