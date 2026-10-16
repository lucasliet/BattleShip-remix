# Smash Remix 2.0.1 no port PC: roadmap definitivo

Este é o **único plano** da implementação do Remix. Quem trabalhar no Remix (agente ou humano) segue este arquivo na
ordem, sem abrir frentes fora dele. Ele substitui os planos antigos (`docs/remix_port_map_2026-10-12.md`, `docs/remix/implementation_plan.md`, removidos;
o contrato dos blobs foi para `docs/remix/torch_factories.md`) e os de `~/.claude/plans/`. Widescreen/aspect ratio está **fora do escopo** (o jogo fica em 4:3).

## Como usar (protocolo de trabalho)

1. No início de cada sessão, e depois de cada compressão de contexto, ler a seção **Posição atual** e a lista de tasks.
2. Trabalhar **só na primeira task não concluída** do milestone atual. Uma task bloqueada recebe `BLOQUEADA: <motivo>`
   e a próxima é feita. Nunca começar o milestone seguinte com tasks abertas no atual, salvo bloqueio registrado.
3. Cada task só é marcada `[x]` depois de passar no critério de aceite (build + o teste descrito). Ao marcar, anotar
   ao lado a evidência curta (commit, stage id, captura).
4. Fim de milestone = **checkpoint**: doc em `docs/bugs/` (ou atualização da existente) com linha no índice, commit no
   decomp e no repositório principal **sem trailers de atribuição**, atualização da memória
   `project_remix_port_checkpoint.md` e da **Posição atual** abaixo. Também é checkpoint quando o usuário escrever
   "checkpoint".
5. Workers (Agent) sempre com `model: "haiku"`, no máximo 20 em paralelo, **uma task pequena por worker** (um stage,
   um item, um subsistema por fase), com brief em `docs/remix/f*_brief.md`. O lead compila, roda e verifica.
6. Bug significativo corrigido vira doc em `docs/bugs/<topic>_<YYYY-MM-DD>.md` com linha em `docs/bugs/README.md`.
7. Responder ao usuário em português. Reportar "Remix jogável" só ao fim do M9.

## Regras fixas

- Toolchain e dependências só em `.toolchain/` (sem brew, pip de sistema, sudo, `/opt`, `/usr/local`, `~/.cache`,
  `~/.cmake`). Python: `.toolchain/host/venv/bin/python -I`. `builtin cd`.
- Build: `PATH=$PWD/.toolchain/host/ninja:$PATH .toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build --target ssb64 -j 4`
  (caminhos absolutos; arquivo novo em `decomp/src` pede `cmake -S $PWD -B $PWD/build`).
- Bytes derivados da ROM nunca em arquivos versionados. Fonte autorizada: `.toolchain/pkgs/smashremix-src`.
- Sem fallback para o pai nem hack de contenção: o comportamento é o do Remix. Desvio necessário vira doc em `docs/bugs/`.
- Código em inglês, sem comentários `//` que repetem o código. `#ifdef REGION_REMIX` em arquivo vanilla.
- Referências em comentários: `torch/` e `libultraship/` são repositórios independentes, então seus comentários não citam
  docs deste repositório (descrevem o formato no próprio código). `decomp/` pode citar `docs/bugs/` e `docs/remix/notes/`
  (convenção do projeto). Notas de trabalho ficam versionadas em `docs/remix/notes/`, nunca em `build/`.
- Testes: skill `remix-smoke-tests` (`.claude/skills/remix-smoke-tests/`: `vs.sh`, `special_matrix.sh`, `stage_sweep.sh`,
  `item_cpu_sweep.sh`, `css.sh`, `sheet.py`, inputs e leitura de resultados); capturas em `build/agent-shots/vs/`.
  Crash `CAMetalLayer nextDrawable` é flake do macOS: repetir antes de investigar.

## Posição atual

- **Milestone:** M2. **Task:** T2.1 (GFX + GFXRoutine); T1.5b (fades Time Twister/Scuttle) fica pendente de M1.
- **Último checkpoint:** CP1, 2026-10-16 (ver `git log`; M1 fechado exceto T1.5b). Índice dos docs: `docs/remix/README.md`.
- **Estimativa geral:** ~61% (cálculo no fim deste arquivo; recalcular a cada checkpoint pela tabela de paridade, T0.2).

---

## M0. Governança

- [x] T0.1 Este roadmap, referenciado em `AGENTS.md`, na memória e no plano da sessão.
- [x] T0.2 `docs/remix/parity.md` gerado por `tools/gen_remix_parity.py`: uma linha por `.asm` do Remix (exceto
  `Moveset/`, `costumes/`, `stages/*.asm` de sprite, `Req/`), com status (`ported` / `partial` / `todo` / `n/a`), arquivo C
  e verificação. Status inicial a partir dos arquivos `ftremix`/`itremix`/`grremix` e dos `REMIX-TODO`. É a medida de
  progresso dos checkpoints. **Aceite:** tabela gerada, contagem por status no topo. *(feito: 287 linhas; fighters 74%, itens 53%, stages 29%, áudio 30%, menus/modos 1%, IA 0%)*

## M1. Estabilizar o que já existe (roster, itens, stages)

- [x] T1.1 Mementos (stage 59) não carrega: um fixup de textura da cadeia de relocação (SETTIMG de 512 words) cobre um
  descritor ainda não visitado e o BSWAP descarrila a cadeia. Correção: os fixups da cadeia param no primeiro slot
  pendente (`portRelocBeginChainWalk`). **Aceite:** stage 59 roda 1500 frames, sem `chain walk STOPPED` no stdout,
  sweep de stages sem regressão; doc em `docs/bugs/`. *(feito: stage 59 renderiza; sweep 174/175, só o 149 do T1.2; doc `reloc_chain_fixup_overwrites_pending_slot_2026-10-14.md`)*
- [x] T1.2 Dream Greens (stage 149) crash em `gcDrawDObjTree` (`grDisplayLayer0PriProcDisplay`). **Aceite:** roda 1500 frames. *(feito: `dream_greens_hardcoding_1-4` e `_animation_1-4` em `grpupupu.c`, tabela lida da imagem de RAM; Dream Land vanilla intacto)*
- [x] T1.3 Verificar pickup e efeito dos itens padrão em partida. *(feito em 2026-10-15 com `item_cpu_sweep.sh`, ver a tabela
  de `remix_standard_items_2026-10-14.md`: Super/Poison Mushroom (Size ligado), Blue Shell (transform corrigido), Lightning
  (Size e cancelamento do cogumelo), Deku Nut, Franklin, Pitfall, Dango, P-Wing, Stopwatch, Golden Gun (bala dourada
  dispara, 12 tiros, dano 99: Mario 99% e Yoshi 101% em `cpu_53`), Basketball (cai e quica; o resto vai para o T6.1);
  Psyduck é conteúdo desativado no Remix 2.0.1, n/a.)*
- [x] T1.4 Comportamento dos 15 hazards portados verificado em partida (2026-10-15): Corneria, Pirate Land, Rainbow Road,
  Spawned Fear, Metallic Madness, GB Land, Turnpike (semáforo e veículos), Onett (carro buzina em ~3150 e cruza em ~3180),
  Japes (KlapTrap aparece e anima, corrigido), água do Great Bay e do Twilight (`walk_left/right.txt`), bumper do Casino e
  bumpers do RTTF (criados). Achados corrigidos: crash do Starmie (`starmie_no_victim_2026-10-15.md`), KlapTrap invisível,
  checagem de intro compartilhada (`grRemixHazardsMatchHasStarted`). Tabela em `remix_stages_and_hazards_2026-10-14.md`.
  O som do MMM (FGM 0x564) depende do áudio Remix (M3).
- [x] T1.4b Fechar os `REMIX-TODO` dos hazards que não dependem de subsistema (`grep -rn REMIX-TODO decomp/src/gr/grremix
  decomp/src/it/itremix`): animações do Robot Bee e do Cacodemon (mesma técnica do KlapTrap), modelo do canhão e lançamento
  da bala do Pirate Land, animação e hitbox do Rainbow chomp, bumper do Casino, campos nomeados dos itens Car/KlapTrap.
  Os que dependem de Size, CharEnvColor, Poison, GFX ou áudio ficam marcados com o subsistema (T2.x/M3) e não contam aqui.
  **Aceite:** o grep só lista TODOs com tag de subsistema, e cada hazard alterado foi visto agindo de novo.
  *(feito em 2026-10-16: animações do KlapTrap (nada na janela: ele nada pela água), Robot Bee (entra pelo canto
  superior esquerdo) e Cacodemon via `grRemixHazardsItemFileTop` (ponteiro do header+0xE0 menos 0x7E8, base das trilhas);
  bala do canhão explode no fim da animação do GObj; modelo do canhão (`gcAddMObjAll`); chomp do Rainbow Road com modelo e
  `gcAddAnimAll` (0x8000F8F4 = `gcAddMObjAll`, 0x8000BED8 = `gcAddAnimAll`); DL do bumper do Casino (agora o bumper azul com
  estrela do Sonic). Sobram só TODOs de campos sem nome do ITStruct/WPStruct (0x158, 0x2D3, 0x33E, 0x1D0...), do tail
  0x8016F8C0 no blast zone e dos subsistemas (Size, CharEnvColor, Poison, GFX, áudio).)*
- [x] T1.5 Hazards restantes portados (2026-10-16, workers Haiku; compilam, as partidas rodam sem crash): Time Twister
  (`grremixtimetwister.c`), Scuttle Town (`grremixscuttle.c`; o asm do Remix 2.0.1 desliga o minion com `b _end`, o port
  espelha com `GRREMIX_SCUTTLE_MINIONS_ENABLED`), ScuttleMinion e BulletBill (itens 0x3E e 0x3D em `itremixitems.c`), Peach
  Castle II (Banzai factory), hitbox do bumper (Peach e Casino), bombas do Bowser's Stadium (vistas agindo: bombas e
  explosões, arquivo `BOWSER_BOMB_HEADER` 0x9B2), nuvens e `platform_movement_` do Yoshi, wind/downtime do Dream Land,
  `mk_remix_hardcoding_1-8`, Dream Greens anim 4 (já feito em T1.2). Smashketball (stage 90) renderiza; a bola vem do modo
  (M6). Notas em `docs/remix/notes/f7_hazard_*.md`. Pendências que saíram daqui: T1.5b, `mk_9` (IA, M7), Dracula/Reapers
  (T2.2), wind/downtime só aparecem com o toggle Whispy (M4).
- [ ] T1.5b Fades do Time Twister (210) e do Scuttle Town (191): busca da warp ball / paleta no grupo 0x2 (wallpaper), troca
  do background e do BGM, zoom de câmera (`Camera.zoom_to_max_`); o state machine já está em C e fica parado sem essa busca.
  **Aceite:** captura da transição de fundo nos dois stages. *(adiado para depois do CP1: precisa entender o wallpaper
  do stage no port, a troca de imagem de fundo é o que falta; a busca da warp ball já está descrita no `REMIX-TODO(T1.5b)`.)*
- [x] T1.6 Stage select (2026-10-16): 85 stages em 10 páginas (`stage_table_normal`), previews de todos renderizados com
  zoom/posição exatos das tabelas do Remix (`SSB64_MAPS_START_GKIND` + `stage_select_sweep.sh`; os modelos grandes vazam do
  quadro como o Peach's Castle vanilla, não há scissor), Training sem leitura fora das tabelas vanilla
  (`mnmaps.c`, `gkind < ARRAY_COUNT(dMNMapsTrainingModeWallpaperIDs)`; capturado com Deku Tree em Training). Emblema de série
  (`series_logo_table`), variantes DL/Omega/Remix (`variant_table`), roulette do Random e layout tournament passam a ser
  menus e estão em T5.1b (dependem dos sprites de série do CSS).
- [x] T1.7 Sweep completo: 78 fighters × NSP/DSP/USP, ids de stage 41 a 215, 17 itens. **Aceite:** zero crash fora de flake. *(2026-10-16: stages 41-215 = 175/175 `rc=0` e nenhuma captura em branco; 13 itens padrão sem crash
  com os eventos esperados; 80 lutadores (0-11 e 29-96) × NSP/DSP/USP = 240 execuções `rc=0` e zero linhas de abort/assert.)*
- [x] **CP1** (checkpoint, 2026-10-16).

## M2. Subsistemas de gameplay (destravam os `REMIX-TODO` dos fighters, itens e stages)

- [ ] T2.1 GFX + GFXRoutine: primeiro o pipeline de dados (`src/gfx/*.bin` no `.o2r` via fábrica do Torch, carregado no
  `lbParticle`), depois as 51 rotinas de colanim (ids >= 0x56) em `dGMColScriptsDescs`, 3 hooks triviais e um
  `port_override` em `ftremixgfx.c`; GFXRoutine não funciona sem os dados do GFX. Remove a contenção de
  `ftParamCheckSetColAnimID` (rejeita ids fora da tabela, `remix_css_and_parent_fallbacks_2026-10-12.md` fix 3). **Aceite:** efeitos com `REMIX-TODO(GFX)` aparecem (Mewtwo,
  Crash, Dedede, Goemon, Sheik, Flashbang, fireball do Dr. Mario).
- [ ] T2.2 CharEnvColor: 3 tabelas e `ftRemixEnvColorGetOverride(port)` em `ftremixenvcolor.c`, 2 hooks nos getters de env
  color; os 40 display lists do asm viram override de material no draw do port (desvio documentado). **Aceite:** Cloaking Device com fade,
  Dracula/Reapers com a cor do stage.
- [ ] T2.3 Reflect: nibbles CUSTOM no hitbox de reflect e dispatch em `ftremixreflect.c` (Franklin reflete sem virar,
  Marina/Dedede absorvem; `itRemixFranklinBadgeReflectInitial` já existe), site 0x800E663C a mapear; hooks de CPU ficam no M7.
  **Aceite:** projétil refletido/absorvido nos três casos.
- [ ] T2.4 Size fases (b) física/peso e (c) cosméticos (`Size.asm`, 73 sites; 4 `adjust_ecb_display_` e 1 `fix_size_`
  ainda sem mapa); estados GIANT/TINY (`Size.match_state_table`) do Super/Poison Mushroom, Lightning e laser do Metallic
  Madness; hooks de entrada (update do Arwing do Wolf `Size.arwing.update_routine`, Blue Falcon, estrela do Kirby, ovo do Yoshi). **Aceite:** Super Mushroom aumenta o lutador com física coerente e volta ao normal.
- [ ] T2.5 PokemonAnnouncer (`ftremixannouncer.c`, ~30 inserções mecânicas; conferir os ids de FGM no banco portado antes). **Aceite:** fala em pelo menos 3 golpes.
- [ ] T2.6 Kirby: `KirbyHats.asm` (magic hat, `restore_magic_hat_`, `clear_magic_hat_charge`) e copy dos poderes Remix.
  **Aceite:** Kirby copia 3 lutadores Remix e usa o NSP com o chapéu certo.
- [ ] T2.7 Outros sistemas de golpe: `Knockback.asm` (aleatorizador de ângulo), `Hitbox.asm`, `command.asm` restante,
  `CustomGrabAction.asm`, `Surface.asm`, `Blastzone.asm`, `Spawn.asm`, `Camera.asm`, `Damage.asm`, `Poison.asm`.
- [ ] T2.8 Sobras dos fighters e itens: patch do Piano em 0x800E7220 (`extra_part_visibility_`), knockback do beam do Waddle Doo
  (0x800E44EC), `prevent_item_despawn_` da Marina, `clear_items_in_1p`. O counter do Roy fica fora: o próprio Remix o
  desativou (comentado em `MarthSpecial.asm`).
- [ ] T2.9 Remover as contenções restantes dos fighters, trocando pelo comportamento do Remix: fallback de joint de captura
  (`ftcommoncapturepulled.c`) e `throw_desc` NULL (`ftcommonthrown2.c`), fixes 6 e 7 de
  `remix_css_and_parent_fallbacks_2026-10-12.md`; o guard de TransN dos polígonos em `ftMainSetStatus`
  (`remix_entry_effects_and_polygon_entry_2026-10-14.md`, achar a causa na tabela de hidden parts).
- [ ] T2.10 Percorrer a auditoria `docs/remix/decomp_fkind_audit.md` (inventário dos 124 sítios indexados por fighter): rever as linhas de risco alto (Kirby `copy_id` em
  `ftcommonspecialair.c`, tabelas do opening room, `vs_records`/`spgame_records` para 97 lutadores) (o doc fica como inventário de referência).
- [ ] T2.11 `grep -rn REMIX-TODO decomp/src` sem ocorrências fora de AI e de modos/menus (que têm milestone próprio).
- **CP2**.

## M3. Áudio

- [ ] T3.1 BGM: música do stage (padrão e alternativas), menus, vitória por lutador (`resultsscreen.asm`). **Aceite:**
  dump WAV (`/tmp/ssb64_dump.wav` é o default do port) com áudio não nulo em 5 stages custom e no resultado.
- [ ] T3.2 SFX e vozes novos (FGM 0x2B7 a 0x608): golpes, itens e hazards tocam o som certo; voz japonesa e
  `Stereo.asm`. **Aceite:** checagem por log de FGM tocado em 10 golpes de lutadores Remix.
- [ ] T3.3 Sound test com as 375 faixas.
- **CP3**.

## M4. Settings e toggles do Remix

- [ ] T4.1 Framework de menu do Remix (`Menu.asm`, `Toggles.asm`, `Settings.asm`) em C, botão Settings no Mode Select,
  atalho L no CSS/SSS, armazenamento persistente (equivalente ao `SRAM.asm` no save do port).
- [ ] T4.2 Toggles de gameplay da lista de `docs/remix/menus.md` 2.4 (hitstun, hitlag, DI, shield stun, Z-cancel,
  tripping, momentum slide, perfect shield, parry, rage, stale moves, air dodge, ledge jump/trump, jab lock, wall teching,
  footstool, charge smash, combo meter, input display, salty runback, neutral spawns, hazard mode e os demais),
  um arquivo por sistema. **Aceite:** cada toggle muda o comportamento num teste curto.
- [ ] T4.3 `grRemixHazardsAreOn`/`grRemixMovementIsOn` e o switch de itens lendo os toggles; perfis CE/TE/NE/JP; toggles de
  item que faltam: seleção de Pokémon, containers, drop de item ao morrer, música aleatória (`Toggles.entry_random_music`,
  usado pelo GB Land).
- **CP4**.

## M5. CSS, menus e telas

- [ ] T5.1 CSS VS (`mnplayersvs.c`, referência em `docs/remix/css_reference.md`): a grade 3x10 e os dados do Remix já estão
  ligados. Falta: costumes com índice > 3 para lutadores com `costume_count` > 4 (`mnPlayersVSSelectFighterPuck`, hoje só 4
  botões C; polígonos têm 6), seletor de variantes pelo D-pad (`remix_css_variant_ids`: polígono, metal, J, E), bookend de bônus
  (arquivo 0xA06, offset 0x3B28; Z/R troca o índice de `bonus_chars` com 30 frames de atraso), Random com o pool e o toggle
  "Random Select" (`recent_randoms_table`, 6 sorteios sem repetir), nomes e logos VS dos lutadores novos (as tabelas
  `name_texture_table` e `series_logo_offset_table` do Remix só vão até 0x1C; achar a fonte dos nomes), lutador com 0 costumes
  tratado como uma cor, offsets `+0x10` dos sprites novos conferidos contra `lbRelocGetFileData`, retratos 0x13 contra 0xA05, e
  o bug candidato do `scale.vec.f.y` repetido (nunca `.z`) nos três ramos do spotlight (`mnplayersvs.c`, `mnplayers1pgame.c`,
  `mnplayers1ptraining.c`). **Aceite:** capturas do CSS com um lutador de 6 costumes, uma variante e o bookend de bônus.
- [ ] T5.1b CSS de 1P, treino e bônus (`mnplayers1pgame.c`, `mnplayers1ptraining.c`, `mnplayers1pbonus.c`): repetir o que o VS
  já tem (grade, retratos, nomes, logos, sorteio), uma tela por vez; hoje usam as tabelas de 12 entradas (a auditoria
  `decomp_fkind_audit.md` lista os sítios). **Aceite:** cada tela abre com os 78 lutadores, sem leitura fora do limite.
- [ ] T5.1c Stage select: emblema de série na moeda de madeira (`series_logo_table` + sprites de série do CSS, hoje em branco
  para os stages Remix), variantes DL/Omega/Remix (`variant_table`), roulette do Random, layout tournament.
  **Aceite:** capturas de um stage com emblema, uma variante e a roulette.
- [ ] T5.2 Painel por porta do CSS (`css/*.asm`).
- [ ] T5.3 VS menu com a página Remix Modes, Mode Select, transições, resultados (logos, vitórias, estatísticas de VS),
  créditos estendidos, Character Data dos 97, name tag / player tag, Gallery.
- [ ] T5.4 Remoção dos hacks de contenção que sobrarem no CSS/menus (lista em `docs/bugs/remix_css_and_parent_fallbacks_2026-10-12.md`).
- **CP5**.

## M6. Modos

- [ ] T6.1 VS: 12-Character Battle, Tag Team, King of the Hill, Smashketball, Tug of War, Stamina, Timed Stock, Teams.
- [ ] T6.2 1P: Remix 1P, All-Star, Multiman, Home-Run Contest, BTT/BTP dos lutadores Remix, Practice 1P.
- [ ] T6.3 Training: extensões de `Training.asm`, menu e HUD de itens, spawn por taunt e item no início da partida.
- **CP6**.

## M7. IA

- [ ] T7.1 `AI.asm` (70 hooks), toggle Improved AI, os `cpu_*` marcados `REMIX-TODO(AI)` nos fighters,
  `VanillaLv10Attacks.asm`. **Aceite:** CPU nível 9 usa especiais e recupera com 10 lutadores Remix.
- **CP7**.

## M8. Restante da paridade

- [ ] T8.1 Todas as linhas `todo`/`partial` de `docs/remix/parity.md` que não couberam acima (Pause, Japan, Accessibility,
  Cheats, SwordTrail, Skeleton, SingleButtonMode, SpecialZoom, VsDemo e outros).
- **CP8**.

## M9. Verificação final no desktop

- [ ] T9.1 Sweep completo (roster × specials, stages, itens) e partidas reais via CSS: 4 jogadores, cada modo, Training.
- [ ] T9.2 Vanilla (`REGION_US`) sem regressão: build e partida.
- [ ] T9.3 Relatório ao usuário em português: Remix jogável, desvios documentados.
- **CP9**.

## M10. Switch

- [ ] T10.1 Build `.nro` do `REGION_REMIX` (`scripts/switch/build.sh`), pacote com o `.o2r` do Remix.
- [ ] T10.2 Validação em hardware pelo usuário (checklist de `docs/switch_port_2026-10-12.md`), RAM com o `.o2r` de 60 MB
  (os dados do Remix na ROM somam ~36,6 MB contra ~9,5 MB do vanilla) e armazenamento no SD.
- **CP10**.

---

## Estimativa de progresso (atualizar em cada checkpoint)

| Área | Peso | Feito |
|---|---|---|
| Fighters (roster, specials, projéteis, entradas) | 35% | ~85% |
| Itens | 10% | ~70% |
| Stages (loader, seletor, hazards) | 20% | ~80% |
| Áudio | 10% | ~50% |
| Settings, menus, modos | 20% | ~15% |
| Verificação final | 5% | 0% |
| **Total ponderado** | | **~61%** |

A partir de T0.2 a estimativa vem da tabela de paridade (`ported` / total por área).
