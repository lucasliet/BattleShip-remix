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

- **Milestone:** M5 fechado (CP5, sobras no T8.2). M6 (modos) em andamento: T6.0 feito; Stamina (T6.1a), Tug of War (T6.1b), KOTH (T6.1c), Tag Team (T6.1d) e Smashketball (T6.1e) jogáveis em partida com sobras listadas em cada task; T6.D1 e T6.D2 (passe de limpeza) feitas; 12-Character Battle (T6.1f) jogável na partida, no CSS e nos resultados com sobras listadas na task (CSS de 24 retratos, portas 3 e 4, reset de pausa, jogos seguidos); próxima task T6.2 (1P), depois T6.3 (Training), e as sobras do M6; antes do CP6 ficam T6.D3 a T6.D6 (testes de unidade, arquivos grandes, revisão dos workers, Tag Team). M4 fechado (CP4). M2 fechado (CP2b) exceto T2.9b e T2.11. T1.5b segue pendente de M1.
- **Último checkpoint:** CP6d, 2026-10-27 (passe de limpeza, testes de unidade, 12CB, menu 1P; antes CP6c, CP6b, CP6a, CP5) (ver `git log`). Índice dos docs: `docs/remix/README.md`.
- **Estimativa geral:** ~82% (corrigida no CP6d, ver a tabela no fim deste arquivo) (cálculo no fim deste arquivo; recalcular a cada checkpoint pela tabela de paridade, T0.2).

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

- [x] T2.1 GFX + GFXRoutine (2026-10-16, `docs/bugs/remix_gfx_and_gfxroutine_2026-10-16.md`): 33 scripts e 9 blocos de textura novos
  no banco comum de partículas (ids 0x5C-0x7C vistos em tela, 32 de 33; harness `SSB64_REMIX_GFX_TEST`), 50 rotinas de colanim
  (0x56-0x87) em `dGMColScriptsDescs` com `port_override` (harness `SSB64_REMIX_COLANIM_TEST`), contenção de
  `ftParamCheckSetColAnimID` removida; Mewtwo, Goemon, Dedede, Sheik, Pwing, Stopwatch, Basketball, Franklin, Flashbang e Deku Nut
  ligados. Abertos (anotados no doc): câmera do Lightning, spin do Crash, nível de carga do Sheik, clone do efeito de chão 0x64.
- [x] T2.2 CharEnvColor (2026-10-16): tabelas override/moveset/state em `ftremixenvcolor.c`, lidas no draw do lutador
  (`ftdisplaymain.c`, depois do evento de SR); Cloaking Device com fade de 32 frames, velocidade e flicker (vistos: Kirby e
  Yoshi translúcidos no `item_cpu_sweep 45`), Dracula e Reapers escrevem a cor de luz do stage (vistos: 176 rosado, 190
  vermelho), moveset_table de Sonic Super/Mewtwo/Sheik no USP. Desvio: o asm troca o display list de material para alfa
  (`custom_display_lists_*`, 40 DLs de ajuste por lutador); o port usa só o alfa do env color, então partes com render mode
  especial (língua do Yoshi, escudo do GND) podem aparecer sem o ajuste. Reaparece como T2.2b se visto em partida.
- [x] T2.3 Reflect (2026-10-16, `ftremixreflect.c`): hitbox de reflect CUSTOM (`FTSpecialColl.kind` = id << 16 | 3) com a
  rotina por id (0 Franklin Badge com o círculo verde no torso, 1 absorção da Marina, 2 absorção do Dedede, que agora também
  liga `absorb_setup_` no NSP), reflectabilidade forçada de Samus bomb, Thunder, Onix, Blastoise, Koffing, bola do Pirate
  Land e laser da Bee, absorção de projétil/item (Thunder trail derruba a cabeça), inversão da direção sem olhar o lr. Vistos:
  os tiros do Fox somem na Marina com o brilho de absorção, e o Dedede reage ao tiro (harness `SSB64_VS_P2_HUMAN=1` com
  `inputs/reflect_test*.txt`). Os hooks de CPU (`Reflect.AI`) ficam no M7.
- [x] T2.4 Size fases b e c (2026-10-16, notas `docs/remix/notes/f7_size_b_*.md` e `f7_size_c_*.md`): pesos (8 sites), velocidades
  de andar/dash/corrida, pulo e jump squat, plataforma de respawn, etiqueta, armadura passiva, tamanho de hitbox e dano,
  escudo/ovo do Yoshi, ondas de impacto, vento do Kirby, soco do Falcon, bola do Dragon King, projétil do Link, Arwing e
  animação de entrada do Mario. Visto: Super Mushroom aumenta Mario e Kirby e volta ao normal (`item_cpu_sweep 46`), matriz
  de 240 runs sem erro depois da onda. Abertos (TODO(Size) no código): sombra do jogador, `kirby_ausp_`, ovo no `lay`,
  Samus gun blast e USP do Kirby, Pikachu/Ness gfx, 7 efeitos de entrada, itens (nuvem), fator do Pwing no pulo aéreo e smash
  carregado (`ChargeSmash`). Fontes de estado GIANT/TINY (`match_state_table`, Lightning e laser da Bee) seguem em T2.4b.
- [x] T2.4b (2026-10-19, validado pelo lead) Estado de tamanho da partida (`Size.match_state_table`): `ftRemixSizeGetMatchState`/
  `SetMatchState` em `ftremixsize.c`, o multiplicador inicial do lutador vem do estado (GIANT 2.25, TINY 0.5; fora das telas How to
  Play e Auto-Demo), e o Lightning, o laser do Metallic Madness e o Super/Poison Mushroom restauram para o tamanho do modo. O
  estado por porta vem do painel (T5.2: `SSB64_REMIX_PANEL="Size.1=1;Size.2=2"`). Visto em partida:
  Mario gigante e Kirby pequeno nascem no tamanho do modo, o Lightning encolhe o Mario gigante, Super Mushroom e Lightning rodam
  3000 frames sem erro. A volta do Lightning ao tamanho do modo só foi lida no código (o item forçado reaparece a cada 60 ticks e a
  contagem de 600 nunca zera no teste).
- [x] T2.5 PokemonAnnouncer (2026-10-19, validado pelo lead): núcleo `if/ifremixannouncer.c` (tabelas, seleção NAME/KO/GENERAL/STANDARD,
  timers, abertura, 1 minuto, sono, "There goes the battle" e TIME UP) e `ifremixhitannouncer.c` (hit e throw por faixa de
  knockback e dano), ligado no setup do VS e do 1P; ganchos em Crash, Mewtwo, Sonic, Wolf, Marth, Pikachu, Samus, Jigglypuff, Fire
  Flower/Charizard, Pokémon, KO, recuperação, taunt, clang, stun e times. Com `Pokemon Announcer=1` (todas as stages) o log de FGM
  mostra clipes do anunciador (ids 0x45B..) em Mario, Samus, Pikachu, Jigglypuff, Crash, Peppy e Kirby: 15 de 15 testes de golpe
  tocaram pelo menos um, sem erro; com o toggle padrão, Dream Land não toca nenhum. **Falta:** Captain falcon punch (o site está
  num `.asm` compartilhado sem equivalente em C), Seismic Toss de Bowser/Giga Bowser (o caminho `kirby_fthrow_1_fix` deles não está
  portado), o histórico de burrow por porta (`BurrowAgain` nunca toca) e o som de fallback que o asm toca quando a seleção falha.
  Quem decide se o som é natural é a escuta do usuário (T9.1b).
- [x] T2.6 Kirby (2026-10-16, validado pelo lead): `ftremixkirbycopy.c` (tabela `Character.kirby_inhale_struct.table` do Remix
  no lugar da tabela de 27 linhas: copy_id, chapéu, escala e dano da estrela dos 97 lutadores), `ftremixkirbyhats.c` (33 chapéus
  extras, arquivos `KIRBY_CUSTOM_HATS` 0xA8D e 0x1533), `ftRemixBindKirbyActions` + `port_fighter_extend_mainmotion` (as 238
  ações e os motions que o Remix acrescenta ao Kirby), `kirby_copy_handler` (tabelas `kirby_ground_nsp`/`kirby_air_nsp`),
  `ftRemixPowerData` (arquivos especiais do lutador copiado, 22 pontos de weapon/item) e `ftremixkirbypower.c` (`magic_hat_on`,
  `taunt_loses_power`, `hat_loss_prevent_`, `kirby_power_loss_prevent_`, `set_absorbed_*`; padrões do jogo sem menu). Corrigido no
  loader o `chain_fixup_vertex`, que girava a display list do chapéu do Dedede. Doc: `docs/bugs/remix_kirby_copy_actions_2026-10-16.md`.
  **Aceite:** os 16 NSP copiados do harness (`SSB64_REMIX_KIRBY_COPY_TEST=<fkind>`, copiado na partida com `vs.sh 8,<fkind>`) rodam
  sem crash; DSamus mostra o tiro escuro, Goemon o chapéu, Dedede o chapéu com o corpo. `spawn_with_hat` (iniciar com chapéu do
  menu) fica no M4/M5 (CSS e opções).
- [x] T2.7 Outros sistemas de golpe (2026-10-16, workers Haiku, validado pelo lead: build limpo, matriz 240/240, 175 stages,
  13 itens): Knockback (ângulos aleatórios, throws), Poison (acid do Spawned Fear com DOOM_ACID), Damage (GFX de shadow/stun,
  rotinas de colanim 7..0xC), Surface (fricção), Blastzone (som do fundo), Camera (tipo MK, zoom do Venom/GB Land), Spawn
  (respawn fixo de DL beta e Sector Z Remix), CustomGrabAction (Wario/Piano), `command.asm` (create_hitbox_, direção). Notas
  em `docs/remix/notes/f7_knockback_hitbox.md`, `f7_damage_poison.md`, `f7_surface_blastzone_camera_spawn.md`,
  `f7_customgrab_command.md`.
- [x] T2.7b (2026-10-19) Neutral spawns (CP4c), blastzone warp e câmera de pausa (M4), exibição de hitbox (CP4c) e agora: esteiras de
  Surface para lutadores e itens (`grRemixSurfaceConveyorPush`, ganchos em `ftMainProcPhysicsMap` e `itProcessProcItemMain`), ações
  STUN/DEKU_STUN/FLASHBANG (`ftremixdamagestun.c`) e BURY/PLUNGE (`ftremixbury.c`) com o `wind push` enterrado. Build limpo e
  matrizes sem erro; o flashbang do Peppy apareceu em partida (explosão e tintura do lutador). **Sem efeito visto:** a esteira
  (nenhuma linha com material 0x1F..0x22 foi alcançada em RTTF ou Time Twister) e o bury/plunge (nenhuma fonte de dano BURY no
  teste), os dois entram na lista do T9.1.
- [x] T2.8 Sobras (2026-10-16): Piano `extra_part_visibility_` e Marina `prevent_item_despawn_` aplicados. Ficam: beam do
  Waddle Doo (0x800E44EC, sem site C equivalente) e `clear_items_in_1p` (o port já limpa os itens a cada cena).
- [x] T2.9 Contenções dos fighters (2026-10-16): o fallback de joint de captura (`ftcommoncapturepulled.c`) e o `throw_desc` NULL
  (`ftcommonthrown2.c`) ficam como substitutos documentados de comportamento indefinido do próprio Remix: o atributo
  `joint_itemheavy_id` do Bowser é 31 (herdado do Yoshi, `define_character(BOWSER, YOSHI, ...)`) e o modelo dele não tem o joint 31;
  na N64 o ponteiro nulo leria a página zero da RDRAM. O mesmo vale para golpes custom sem `SetThrow` (Banjo USP). Não há
  comportamento do Remix para copiar. O guard de TransN dos polígonos (`ftMainSetStatus`) segue sem causa provada: **T2.9b**
  (abaixo).
- [ ] T2.9b Causa do guard de TransN dos polígonos (hipótese: a linha de motion mantém 0x40000000 do Appear do pai mas a animação
  é a Wait sem trilha de TransN, então o walker do figatree desloca as trilhas por um joint). Verificar contando as palavras do
  figatree dos polígonos contra a árvore de joints com a flag ligada; se confirmado, o guard vira a regra do Remix.
- [x] T2.10 Auditoria `docs/remix/decomp_fkind_audit.md` (2026-10-16, escopo ft/ e ef/): Kirby inhale lê `copy[victim_fp->fkind]` (tabela
  Remix, antes lia a linha do pai) e o mesmo em `efmanager.c` (escala da estrela); a tabela de offsets do Falcon Dive usa
  `Character.falcon_dive_id.table` (`ftRemixFalconDiveOffsetRow`); `ftcommonspecialair.c` e os loops de `ftmanager.c` já usam o
  registry; o script de dano arremessado (`p_script[id][fkind]`) indexa a linha do pai, porque o índice acima de 27 cairia na linha do
  outro script (indefinido no Remix). Os sítios de CSS, save (`vs_records`, `spgame_records`, `fighter_mask` de 16 bits) e opening
  room ficam em T5.1b e T5.5.
- [ ] T2.11 `grep -rn REMIX-TODO decomp/src` sem ocorrências fora de AI e de modos/menus (que têm milestone próprio). *Executar depois
  de T2.4b (15 Size), T2.5 (11 PokemonAnnouncer) e M4 (6 toggles); em 2026-10-16 são 168 linhas, o resto é rastreio fino de hazards e
  itens (campos de `ITStruct` sem nome, slot de blast zone 0x398, GFX, display) para resolver em lotes de workers por arquivo.*
- **CP2**.
- [x] **CP2d** (checkpoint a pedido, 2026-10-19): nada de código novo desde o CP2c; registrado o achado dos resultados brancos (T5.3) e o estado do CSS (grade 3x10 com os 30 retratos Remix, sem bookends, sem sorteio, costumes de lutador Remix sempre 0 porque `ftport.c` só liga `costume_row` aos vanilla; os dados de costume do Remix estão em `Character.default_costume.table` e `Costumes.select_.num_costumes`, e o ciclo de costume do Remix usa C esquerda/direita para costume e C cima/baixo para o tom).
- [x] **CP2c** (checkpoint, 2026-10-19): T2.4b, T2.5 e T2.7b fechados (ver acima); T2.9b e T2.11 seguem abertos (T2.11 vira lote de workers no M8, 169 linhas).

## M3. Áudio

- [x] T3.1 BGM (2026-10-16, validado pelo lead): o port já tocava os bancos reconstruídos (375 sequências); faltava o BGM de vitória
  por lutador (`Character.winner_bgm.table`, `mnremixwinbgm.c`: a entrada é um `add_victory_bgm(bgm)` e o id vem do `ori a1`) e a cena de
  resultados não carregava os arquivos de lutadores Remix (`mnvsresults.c`). Dump de áudio por run (`SSB64_AUDIO_DUMP_PATH`,
  `SSB64_AUDIO_DUMP_SECONDS`, `vs.sh` grava `audio.wav`, `scripts/wavstat.py` mede RMS por segundo). **Aceite:** 5 stages Remix
  (41, 60, 100, 150, 200) com som nos 24 s capturados (RMS 1000..7000, o BGM do stage é o `default_music_track_table` menos 1);
  vitória: Falco toca 69, DSamus 82, Dedede 210 (as ids de `FALCO_VICTORY`, `DSAMUS_VICTORY`, `DEDEDE_VICTORY`). O BGM dos menus e
  o volume do BGM dependem dos toggles (M4). A cena de resultados sai branca no atalho `SSB64_VS_STOCKS` (também com lutadores
  vanilla), então o visual dela é verificado no T5.3.
- [x] T3.2 SFX e vozes (2026-10-16, validado pelo lead): os blobs `audio/fgm_tbl` e `fgm_ucd` já são autossuficientes (1288 e 1545 entradas,
  todos os offsets dentro do blob) e os SFX tocam. Log opt-in `SSB64_AUDIO_FGM_LOG=1` (`[fgm] play id=.. table=..` em
  `func_800269C0_275C0`, a função que o jogo chama; `func_80026A10_27610` é outra). Centro do pan do hit SFX no Sector Z Remix
  (`Stereo.asm`, `lbcommon.c`). **Aceite:** matriz de especiais de 10 lutadores (29, 34, 38, 40, 58, 64, 65, 77, 80, 90) toca 25 a 70
  FGMs cada, 7 deles com ids novos (>= 0x2B7: Falco 720/724, DSamus 1477/1492, 38: 831/832, 58: 844 e 856..858, Dedede 1082..1101, 65:
  1114..1280) e nenhum fora da tabela (1545); os outros 3 usam só ids vanilla. Fora daqui: voz japonesa e `entry_stereo_fix` são toggles
  (M4); o caso HRC do Stereo é do T6.2; a fala do MMM (FGM 0x564) só toca no evento do hazard.
- [x] T3.3 Sound test (2026-10-16, validado pelo lead): `mnsoundtest.c` com 375 músicas e 1094 vozes (vanilla 244 + 850), mapas
  `MIDI.extended_music_map_table` e `FGM.extended_voice_map_table` lidos da imagem (`mnremixsoundtest.c`), L/R seguram 50 por toque,
  número com 4 dígitos. Vistos: Music 201 toca o BGM 200, Music 375 (wrap com ←) toca o BGM 374, Voice 101 toca o FGM 424, Voice 1094
  toca o FGM 1544 (0x608, o último), com som no dump. Scripts `inputs/soundtest_browse.txt` e `soundtest_wrap.txt`
  (`SSB64_START_SCENE=59`).
- [x] **CP3** (checkpoint, 2026-10-16).

## M4. Settings e toggles do Remix

- [x] T4.1 Framework dos settings (2026-10-18, validado pelo lead): o registro vem da imagem do Remix (`remix_toggles.cpp`): 531 toggles
  (nós `Menu.entry` das cinco listas, título, faixa, tabela de strings) com os quatro perfis (CE/TE/NE/JP) de
  `Toggles.profile_defaults_*`, ordem conferida contra os arrays (531 de 531 iguais). Valor em CVar `gRemix.Toggle.<n>` (persistente),
  leitura `gmRemixToggle(<nome>)` e `gmRemixToggleVS(<nome>)` (guarda de 1P/Bonus, `gmremixtoggle.h`), override por execução
  `SSB64_REMIX_TOGGLES="Titulo=valor;..."` e `SSB64_REMIX_PROFILE=0..3`. A interface é a seção **Remix** do menu do port (uma barra
  lateral por grupo, perfis em botões); o botão Settings do Mode Select e o atalho L do CSS/SSS abrem esse menu e ficam no T5.3.
  Desvio: o menu desenhado pelo Remix (`Menu.asm`) não é replicado, o do port tem as mesmas entradas e valores.
- [x] T4.2 Toggles de gameplay *(fechado 2026-10-19; o que sobrou foi redistribuído abaixo)*: lista de `docs/remix/menus.md` 2.4, um arquivo por sistema, workers Haiku com
  `docs/remix/f8_toggle_worker_brief.md` e validação do lead. **Feitos** (build limpo, matriz 6 lutadores x 3 golpes sem erro com todos
  desligados e todos ligados): Hitstun, DI, Air Dodging (status novos 0x2D/0x3A/0x3B com a opção ligada), Tripping, Z-Cancel (+ Punish
  e Flash), Move Staling, Jab Locking, Footstool Jumping, Edge C-Jumping, Ledge Trumping, Wall Teching, Spot Dodging, Perfect Shielding,
  Parrying, Rage, Charge Smashes, Game Speed e Single Button Mode (parciais: ligação no Training e no filtro de entrada),
  Hitlag, Japanese Sounds, Shield Stun, Japanese Stun/Sleep, Momentum Slide (Japan.asm; faltam os sons do Link). Notas em
  `docs/remix/notes/f8_*.md` (cada uma diz o título, os valores e o que ver). **Terceira onda (2026-10-18, portados e validados por build limpo e matriz sem erro, com tudo desligado e tudo ligado):** Item
  Containers, All Items R Drop, Pokémon picker/vozes e Stopwatch (`Item.asm`), Camera Mode, Cinematic Entry e Disable HUD pausa
  (`Camera.asm`), Special Zoom, BlastZone Warp e BlastZone GFX, Combo Meter (4 toggles; setup chamado depois do `SetupFiles` do VS e
  do 1P, o hook original em `syTaskmanLoadScene` abortava por carregar arquivo antes do reloc), Screenshake e Flash Guard, Disable
  AA e FPS (mapeados ao MSAA e à janela de stats do port), Hold To Pause, BGM Volume, SFX Volume, Play Music, Random Music.
  **Quarta onda (2026-10-19, build limpo, matriz 6 lutadores x 3 golpes sem erro com tudo desligado e com os toggles novos ligados):**
  Neutral Spawns (tabela por stage; o primeiro teste mostrou o cano flutuando por ler o stage do CSS e não o da partida, corrigido para
  `gSCManagerBattleState->gkind`), Default CPU LVL (V.S.), Skip Results Screen (a partida termina e volta ao CSS), Salty Runback e
  Always Show Full Results (`gmremixgameend.c`), Menu Music no CSS do VS (`gmremixmenumusic.c`), Disable HUD no modo ALL em VS e
  Training (`ifremixhud.c`, HUD sumiu na captura), Model Display e Special Model Display (caixas de hitbox e losangos de ECB vistos
  em partida, `ftremixhitboxdisplay.c`). **Quinta onda (2026-10-19):** Single Button Mode (filtro no `ftMainProcessInput` do jogador humano, validado em partida: `=1` ignora o
  B, `=2` deixa passar) e Game Speed (VS validado com `=5`; no Training o gancho `ftRemixSpeedTrainingRunFrames` entrou em
  `sc1PTrainingModeUpdateAll`, só build, o teste fica no T6.3). **Redistribuído:** os outros 12 ganchos de Menu Music, Music Title at
  Match Start, Input Display, Random Select, Full Results (a cena de resultados ainda renderiza em branco no atalho de VS) e os grupos
  de pause 0xE a 0x10 vão para o T5.3; os hooks de CPU do Single Button Mode e Improved AI para o T7.1; 12 Character Battle do
  runback e reset de heap do runback para o T6.1; Skip Start Cheer (o decomp não tem a chamada do FGM 0x0272), Advanced Hurtbox
  Display (o port só desenha um cuboide) e os sons JP do Link para o T8.1.
  **Aceite:** cada toggle muda o comportamento num teste curto.
  Efeito conferido até aqui: Air Dodging (status de esquiva no ar aparecem), Neutral Spawns, Disable HUD ALL, Special Model Display e
  Skip Results Screen, Single Button Mode e Game Speed.
- [x] T4.3 (2026-10-19, validado pelo lead) `grRemixHazardsAreOn`/`grRemixMovementIsOn` leem Hazard Mode (Hazard Mode=3 apaga os logs do
  canhão do Pirate Land), Whispy Mode, Yoshi cloud anims, Random Music do GB Land e o warp do Blastzone; o switch de itens lê o
  registro (`itRemixStandardItemIsOn`/`GetWeight`); containers, All Items R Drop e o seletor de Pokémon já estavam ligados.
  **Efeito conferido em partida** (Dream Land, Poké Ball forçada, 2400 frames, log `[remix-item]`): com todos os Pokémon desligados
  menos o Snorlax, os dois sorteios foram Snorlax (0x21); com tudo ligado saíram 0x25 e 0x2A; `Item Containers=1` deixa a máscara em
  0xFFF8 (caixas, barris e cápsulas fora, ovo mantido fora do Yoshi) e o padrão e `=3` em 0xFFFF. Os perfis CE/TE/NE/JP são dado do
  registro do T4.1 (`SSB64_REMIX_PROFILE`). O que depende do menu (item switch interativo, Settings) fica no T5.3.
- [x] **CP4** (checkpoint, 2026-10-19): sweeps de regressão com a build final: 175 de 175 stages com `rc=0` e nenhuma captura em branco, 13 de 13 itens no sweep de CPUs sem abort/assert (12 quedas do compositor do macOS passaram na repetição).

## M5. CSS, menus e telas

- [x] T5.1 *(fechado no CP5; as sobras desta task estão no T8.2)* CSS VS (`mnplayersvs.c`, referência em `docs/remix/css_reference.md`): a grade 3x10 e os dados do Remix já estão
  ligados. **Feito em 2026-10-20** (`docs/remix/notes/f10_css_costumes_variants.md`): costumes e sombras no estilo 19XX (C
  direita/esquerda trocam o costume até `num_costumes`, C cima/baixo a sombra 0..2, sem bloqueio de cor repetida), tabela
  `default_costume` lida da imagem para os 97 lutadores, variantes pelo D-pad (cima especial, baixo polígono, esquerda J,
  direita E; verificado com o Mario: Metal, Polygon, J), toggle "Random Select" no sorteio do CPU (só código), variante
  centrada no retrato do lutador base, e a correção do crash de `grRemixCameraAdjustZoomRange` na CSS. Falta: costumes extras
  além de `num_costumes` (tabelas de partes de `costumes/*.asm` e `check_for_custom_costume_`), C no team battle, ícones do
  D-pad e bandeiras J/E das variantes, bookend de bônus (arquivo 0xA06, offset 0x3B28; Z/R troca o índice de `bonus_chars` com 30
  frames de atraso), botão L de sorteio e lista `recent_randoms_table` (6 sorteios sem repetir), logos VS dos lutadores novos
  (as tabelas `name_texture_table` e `series_logo_offset_table` do Remix só vão até 0x1C; achar a fonte dos logos; os nomes já
  aparecem), lutador com 0 costumes tratado como uma cor, offsets `+0x10` dos sprites novos conferidos contra
  `lbRelocGetFileData`, retratos 0x13 contra 0xA05, e o bug candidato do `scale.vec.f.y` repetido (nunca `.z`) nos três ramos do
  spotlight (`mnplayersvs.c`, `mnplayers1pgame.c`, `mnplayers1ptraining.c`). **Aceite:** capturas do CSS com um lutador de 6
  costumes (feito: Falco), uma variante (feito) e o bookend de bônus.
- [x] T5.1b CSS de 1P, treino e bônus (`mnplayers1pgame.c`, `mnplayers1ptraining.c`, `mnplayers1pbonus.c`): 2026-10-20, três
  workers Haiku (`docs/remix/f10_css_worker_brief.md`, notas `notes/f10_css_*.md`), grade compartilhada em
  `mnplayers/mnremixcss.c`. Verificado pelo lead: as quatro telas (treino, 1P, bônus 1 e 2) abrem com os 30 retratos, o
  1P com Sheik cicla costume e sombra, o puck do CP no treino cai na célula certa. Sobra: o shade fica numa `static` por tela
  (o struct de slot está em `mntypes.h`), sorteio L e música de menu só no VS, `ftManagerSetupFilesAllKind` dos Remix no
  FuncStart do 1P (a lista dos 12 vanilla continua), e o teste em partida de 1P/treino/bônus com lutador Remix fica nos
  M6.
- [x] T5.1c *(fechado no CP5; as sobras estão no T8.2)* Stage select: **feito em 2026-10-20**: emblema de série na moeda de madeira (`mnmaps/mnremixemblem.c`, lê
  `Stages.series_logo_table` e `CharacterSelect.series_logo.table` da imagem; verificado em Mute City, Spiral Mountain e Final
  Destination), variantes por C-baixo (`mnmaps/mnremixstagevariant.c`, lê `Stages.variant_table`; verificado em Meta Crystal:
  DL, Remix e Metal Cavern, volta ao padrão; nome, emblema, preview e a seleção final usam a variante, a memória do cursor
  guarda o stage base). Falta: texto do tipo de variante sob o preview (`layout`), roulette do RANDOM (C-baixo sobre RANDOM,
  `random_stage_roulette`), layout tournament (toggle `sss_layout`, `tournament_hazard_mode_table`).
  **Aceite:** capturas de um stage com emblema (feito), uma variante (feito) e a roulette.
- [x] T5.2 Painel por porta do CSS (`css/*.asm`): **feito em 2026-10-22**. Os valores por porta ficam no registro
  `port/remix/remix_player_panel.*` (21 opções: as 20 do painel mais `StockCount`; CVar `gRemix.Panel.<Opção>.P<n>`, override de
  execução `SSB64_REMIX_PANEL="Size.2=1;Damage.1=50"`), a interface é a seção Remix > Player Panel P1..P4 do menu do port (o painel
  desenhado pelo Remix não é replicado, como no T4.2) e as partidas leem o registro por `gmRemixPanel(OPÇÃO, porta)`
  (`gm/gmremixpanel.*`; nos modos Bônus e 1P só a porta humana vale). Pesquisa: `docs/remix/notes/f10_css_port_panel.md`; brief dos
  workers: `docs/remix/f10_panel_worker_brief.md`; notas por opção `docs/remix/notes/f10_panel_*.md`. Ganchos prontos e vistos em
  partida (capturas): Damage (80%), Size (Giant), Visibility (Dark), StartWith e TauntItem (item na mão), PoisonDmg (12% no P2),
  KirbyHat (boné do Mario e do DK), Shield (amarelo), TauntButton (C-Baixo faz a provocação sem pulo), PlayerTag (seta), StockMode
  Manual (1 stock termina a partida em 1 KO), DpadControl Stickless (D-pad anda). Portados sem captura própria: Knockback,
  InputDelay, ModelDisplay, Handicap (byte do descritor; a leitura da tabela de knockback é limitada às 36 linhas), PRACTICE_1P.
- [x] T5.2b *(fechado no CP5; movido para o T8.2)* Sobras do painel por porta: Skeleton (precisa forçar o caminho de esqueleto do display, `REMIX-TODO(Skeleton)` em
  `ftdisplaymain.c`), D-pad Map (macros Smash/Tilt/Special, não portado), DpadControl (valor parcial e toque duplo, pulo e dash dos
  modos J, máscara de escudo), Shield modo Costume e cor do ovo do Yoshi, StockMode "Last" e os modos que também leem a tabela (Tag
  Team, Tug of War, 12CB, KotH, Smashketball: M6), Practice_1P (segurar L reinicia a fase, sem custo de continue), Input Display
  (HUD, vai com o T5.3), nomes das tags (o Remix os digita no menu, não existem na imagem), Start With no Treino (conferir se passa
  pelo GO), Poison Heal um frame depois e a trava de stamina. **Aceite:** cada item fechado com captura ou riscado com motivo.
- [x] T5.3 *(fechado em 2026-10-22; o que sobrou está no T5.3b)* Menus e telas do Remix, sem redesenhar o framework de menus do
  Remix (as opções ficam no menu do port): tela de resultados do VS deixa de ser branca (2026-10-20, backend Metal sem
  `SetColorWriteMask`, `docs/bugs/metal_redirect_z_color_write_2026-10-20.md`); página Remix Modes do menu VS (`mnvsmode/mnremixvsmode.c`:
  botão Remix Modes, cinco botões de modo, B volta, A grava o modo em `gMNVSModeRemixMode` e abre o CSS; visto em partida, o modo só
  é lido no M6); Menu Music em todas as telas com hook no Remix (`gmremixmenumusic.c`, `safe_id`); Music Title at Match Start
  (overlay do port `remix_overlay`/`remix_music_title`, 308 títulos gerados de `BGM.asm`; visto: "Dream Land - Kirby Super Star"
  durante a contagem); Input Display (overlay ImGui `remix_input_display`, controle por porta no topo ou embaixo conforme a opção do
  painel; visto no P1 topo e P2 base); créditos estendidos (`credits/remix/` mais `tools/gen_remix_credits.py`; visto: "The
  Smashfather" e os colaboradores do Remix depois do staff vanilla); Character Data com a ordem de 30 lutadores do Remix
  (`mndata/mnremixchardata.c`; visto: Mario, Bowser, DK, Link, Sheik, Yoshi, Kirby; o VS Record fica com os 12 vanilla como no
  Remix); Gallery (`mncommon/mnremixgallery.c`, `SSB64_START_GALLERY=1`; vista com Mario, DK e Link, 12 das 40 entradas); Item Switch
  página 2 já existia (2026-10-14); estatísticas de VS (só o rastreio, `mnremixvsstats.c`); transições restauradas 0x27 e 0x2F;
  Full Results já ligado (`gmremixgameend.c`). Conferido em jogo ao fim: pausa e retomada em VS, resultados por KO, Auto-Demo sem queda
  (o estado inicial `nFTKindNull` do demo agora vira Mario/Fox), Character Data sem estourar o status buffer (256 entradas).
  Notas: `docs/remix/notes/f10_menu_*.md`. **Substituídos pelo menu do port (decisão do T4.2):** botão Settings do Mode Select, atalho L
  do Settings, Mode Select e o menu de toggles.
- [x] T5.3b *(fechado no CP5; movido para o T8.2)* Sobras das telas: desenho da página de estatísticas do VS (`REMIX-TODO(results-text)`, precisa de texto: overlay ou
  fonte), linhas de modo no resultado (`results-mode-rows`: KotH, Smashketball), logos de transição Smash/Remix (ids acima de
  `RELOC_FILE_COUNT`), glifos extras dos créditos e o speed assist, botão Gallery no Data menu e as 28 entradas da Gallery de
  lutadores Remix, emblema e nomes de golpe do Character Data dos 18 lutadores extras, legend do R e dos controles de câmera da
  pausa (grupos 0xE-0x10: não existem no port), RANDOM ALL do Menu Music, Random Select do CPU (só lido no código) e o texto de
  legenda do item switch. **Aceite:** cada item fechado com captura ou riscado com motivo (os de M6 migram para lá).
- [x] T5.5 Registros de save para 97 lutadores: 2026-10-20, `GMCOMMON_FIGHTERS_SAVE_NUM` (97 sob Remix) dimensiona
  `vs_records`, `spgame_records` e as matrizes de KO/partidas em `lbtypes.h`; as guardas de registro (`sc1pmanager.c`,
  `sc1pbonusstage.c`, `mnvsresults.c` por `ARRAY_COUNT`) seguem o tamanho novo; as máscaras de 16 bits (`fighter_mask`,
  desbloqueio) continuam só para os 12 vanilla; `lbBackupInitRemixDefaults` copia o padrão da linha 0 (melhor tempo de bônus de
  60 min) para as linhas 12..96. O save do Remix passa de 8 KB para 123 KB (duas cópias); um save antigo falha o checksum e
  recomeça. Verificado: duas partidas VS Peppy×Crash seguidas gravam e relêem o registro (`games_played` 2 nos dois). A 1P com
  lutador Remix e as telas de recorde/Character Data de 97 ficam no M6/T5.3.
- [x] T5.4 Hacks de contenção do CSS/menus (lista em `docs/bugs/remix_css_and_parent_fallbacks_2026-10-12.md`): 2026-10-22. O limite
  de `ftParamCheckSetColAnimID` já é só guarda (as rotinas GFX do Remix estão carregadas, `[remix-gfx] ... routines loaded`); a
  queda do CSS por arquivo não carregado, a heap do figatree e o `SetupFilesAllKind` do pai ficam porque as rotinas herdadas
  leem esses globais. As guardas do `ftCommonCapturePulledRotateScale` (joint de captura ausente) e do `ftCommonThrownGetThrowDesc`
  (sem `SetThrow`) deixam de ser silenciosas: gravam uma linha no log na primeira vez, e a matriz de 7 lutadores (Bowser, Banjo, Crash,
  Goemon, Captain, Dedede e Kirby, NSP/DSP/USP) não as acionou; o T9.1 as procura nas varreduras.
- [x] **CP5a** (checkpoint a pedido, 2026-10-20): resultados do VS deixam de ser brancos (backend Metal), CSS VS com costumes,
  sombras, variantes, L aleatório e Random Select, CSS de 1P/treino/bônus com a grade Remix, stage select com emblemas e variantes,
  save de 97 lutadores, crash da CSS corrigido (`remix_css_camera_zoom_null_battle_state_2026-10-20.md`,
  `metal_redirect_z_color_write_2026-10-20.md`). Verificado pelo lead em jogo (capturas em `build/agent-shots/vs/`). Commits:
  `libultraship` (Metal), `decomp` e repositório principal.
- [x] **CP5** (2026-10-22): M5 fechado: CSS (T5.1, T5.1b, T5.1c), save de 97 lutadores (T5.5), painel por porta (T5.2: registro, menu do port e 20 ganchos de partida), menus e telas (T5.3: Remix Modes, Menu Music, Music Title, Input Display, créditos, Character Data, Gallery, resultados) e as guardas de contenção (T5.4). Sobras no T8.2.

## M6. Modos

Pesquisa feita em 2026-10-22 (workers Haiku, `docs/remix/f11_mode_research_brief.md`): um relatório por modo em
`docs/remix/notes/f11_mode_<nome>.md` com fluxo, ganchos (asm contra função do decomp), estado, lógica de partida, HUD, dependências,
plano de port em passos e riscos. Cada task abaixo executa o plano do seu relatório, passo a passo (um worker por passo, o lead
compila e confere em tela). Os relatórios dizem `no match found` onde o decomp não tem a função; nenhum endereço de ROM foi verificado.
O modo escolhido na página Remix Modes do menu VS chega em `gMNVSModeRemixMode` (`mn/mnvsmode/mnremixvsmode.h`).

- [x] T6.0 Base comum dos modos (feito 2026-10-23, referência em [`modes_base.md`](modes_base.md)): `gm/gmremixmode.c/.h` (`gmRemixModeGet`
  lê `gMNVSModeRemixMode` ou `SSB64_REMIX_MODE`, log `remix mode:` na partida), Timed Stock (regra Stock grava tempo|stock = 3, resultados
  e `score_fix_` em `mnVSResultsInitRankings`/`GetPointsDirect`, sem morte súbita, ícones de stock) e `port/remix/remix_hud_text`
  (texto posicionado em coordenadas N64 por ImGui, decisão do HUD de texto). **Visto:** partida de 1 minuto, 3 stocks, 3 lutadores:
  timer 01:00, TIME UP, resultados ordenados pelo score de stock menos dano (Yoshi com 15% em 2o); `SSB64_REMIX_MODE=koth` aparece
  no log; rótulo de teste do HUD centrado na tela. As tabelas de StockMode já existiam (`gmRemixStockModeCount`).
- [ ] T6.1a Stamina (`f11_mode_stamina.md`, 17 passos, inclui Timed Stock e as mudanças de Teams): HP no lugar do dano, barra, KO por HP.
  Estado (CP6b): P1-P5 feitos pelo lead (`gm/gmremixstamina.*`, regra Stamina, 1 stock, Timed Stock em `gmremixmode`); P6 knockback 20%, P7 dano/KO, P8 ação de KO,
  P9 morte única/respawn, P11 escudo/ledge/down/egg/armor, P12 veneno/zoom, P13 HUD e P14/P15 menu (Rule com Stamina, HP, CSS) portados por workers, relatórios
  em `notes/f12_stamina_port.md`. **Visto em partida** (`SSB64_VS_STAMINA=<hp>`, 3 lutadores, HP 8): HUD com o HP restante e escurecimento, um stock por lutador, Mario e Yoshi saem por HP,
  a partida termina com Kirby vencendo e lugares 1/2/3 certos (P10 resolvido: a causa era o atalho de teste `SSB64_VS_FKINDS`, que não definia `players[].team`/`player`, então
  o Yoshi tinha time 0; corrigido em `scmanager.c`, não era bug do Stamina). **Aberto:** P16 Teams, P17 Giga Bowser (T6.2); menu P14/15, P8 (ação de KO: Yoshi ficou caído), P9, P11, P12 sem conferência em tela.
- [ ] T6.1b Tug of War (`f11_mode_tugofwar.md`, 9 passos): bugs prováveis do asm na quebra de escudo e no crédito de time (conferir antes de copiar). Feitos: P1/P3 (`gm/gmremixtugofwar.*`, último atacante, R1 e R2 corrigidos), P2 (ciclo de CSS de `gmremixmode`), P4 (crédito de KO e revive em `ftCommonDeadUpdateScore`), P5 (lugar do revivido) e P6 (placar de resultados). **Visto em partida** (`SSB64_REMIX_MODE=tugofwar SSB64_VS_STOCKS=2`, 1v1 Mario contra Kirby, Mario sai da beirada): cada KO credita um stock ao Kirby (log `tugofwar: stock credit`, stock_count 1 -> 2 -> 3, HUD com 3 ícones contra 1 do Mario). Faltam P7 (CSS sem limite de contagem, hoje o teto vanilla é 99 stocks), P8 (anúncio FGM 479 no CSS), conferência de times, revive em tela e do placar; título do CSS (arquivo da imagem `0x30F0` não identificado).
- [ ] T6.1c King of the Hill (`f11_mode_koth.md`, 15 passos): colina, pontos por tempo, equipes. Feito (CP6c): `gm/gmremixkoth*.c` (escolha da colina por linha de chão válida, rei com buffer de contestação de 30 quadros, pontuação por quadros, timer de vitória de 210 e fim por GAMESET, faixa colorida desenhada em lista de display no mundo, medidores pelo overlay `remix_hud_bar_put`, atraso de respawn de 120 quadros e câmera desligada, pontuação dos resultados, sem morte súbita), seletor de meta no CSS (`mnremixgoalpicker`). Correções do asm: a pontuação do time conta uma vez por quadro (o asm somava por jogador e podia pular a meta). **Visto em partida** (`SSB64_REMIX_MODE=koth SSB64_KOTH_LINE=2 SSB64_KOTH_GOAL=4`): faixa vermelha sobre a plataforma do Mario, barra do Mario enchendo, timer de vitória, GAMESET e tela de resultados com Mario vencedor. Faltam: faixa em polilinhas de várias linhas e cores de equipe conferidas em tela, brilho dos lutadores (GFXRoutine KOTH), linha de tempo nos resultados (`results-mode-rows`), título e anúncio do CSS, barras mais altas em Dragon King, IA que busca a colina (T7.1), conferência de equipes e do respawn em tela.
- [ ] T6.1d Tag Team (`f11_mode_tagteam.md`, 15 passos). Decisão: a fila de personagens não é desenhada no CSS (o framework de menu do Remix não é redesenhado): o slot 0 é o personagem escolhido no CSS e os slots 1 a 5 de cada porta vêm do menu do port (Remix > Tag Team, Random por padrão). Feito (CP6c): `gm/gmremixtagteam.c` (fila por porta resolvida no início da partida, troca do lutador ao perder um stock em `ftCommonRebirthDownSetStatus`: arquivos do novo personagem, `ftManagerMakeFighter`, itens e projéteis passam ao novo dono, `fighter_gobj` e dano do HUD atualizados), modo Timed Stock com stocks e times salvos pelo ciclo de CSS. **Visto em partida** (`SSB64_REMIX_MODE=tagteam SSB64_VS_STOCKS=3`, Mario cai da beirada): `tagteam: port 0 swaps fkind 0 for 73` e a Peach entra no lugar do Mario, 2 stocks restantes. Faltam: ícones de stock por personagem da fila (`REMIX-TODO(tagteam-icons)`), roubo de stock em times (`REMIX-TODO(tagteam-steal)`), título e anúncio do CSS, carregamento de variantes e de Sonic classic, personagens que já estão na fila de outra porta (costumes), conferência de times e do heap com troca repetida.
- [ ] T6.1e Smashketball (`f11_mode_smashketball.md`, 15 passos). Feito (CP6c): `gm/gmremixsmashketball*.c` (estado, tipo basquete/futebol escolhido no menu do port Remix > Modes ou `SSB64_SMASH_TYPE`, palco 0x5A/0xD1 no início da partida, bola gerada após 0x135 quadros, detecção de cesta e de gol em `itRemixBasketballCheckHoop`, placar por time e `winner_scores`, cestas colocadas na altura do modo, placar nativo do palco com tempo e pontos, fim por meta (100 = sem meta) com buzina, morte súbita em empate sem tempo e a 0%, spawns neutros dos dois times, pontuação dos resultados). **Visto em partida** (`SSB64_REMIX_MODE=smashketball SSB64_SMASH_BALL_X=3700 SSB64_SMASH_GOAL=1`): a bola cai pela cesta, `Home 1 Away 0`, GAMESET e resultados com RED WINS; com `SSB64_VS_TIME=1` e 0 a 0 a partida entra em morte súbita a 0% sem timer. Faltam: seletor de tipo e título no CSS vanilla, exigir exatamente dois times no START, cores de equipe nos dígitos, câmera que inclui a bola, canto da torcida a partir de 3 cestas, linha de resultados, bola dourada do futebol, anúncio do CSS, IA da bola (T7.1), conferência com 3 e 4 jogadores.
- [ ] T6.1f 12-Character Battle (`f11_mode_twelvecb.md`, 16 passos): CSS próprio, runback e reset de heap do runback (T4.2/T6.1).
  Estado (2026-10-27): P1 e P2 feitos: `gm/gmremixtwelvecbrules.*` (pools de stocks, stocks iniciais, contabilidade de KO, melhor personagem; vetor de jogos de 48 em vez dos 24 do asm, que estoura em batalha longa) com a suíte `tests/remix/test_twelvecb_rules.c` (40 checagens, mutação verificada). P3, P4, P9, P10 e P11 feitos na partida (`gm/gmremixtwelvecb.*`: início do jogo antes de criar os lutadores, stocks iniciais pelo pool, KO em `ftCommonDeadUpdateScore`, sem salty runback em `gmRemixGameEndSelectScene`, reset ao sair do CSS; retrato do personagem no conjunto padrão: slots 0 a 11 para os 12 originais, 12 a 23 para os demais). **Visto em partida** (`SSB64_REMIX_MODE=12cb SSB64_VS_STOCKS=1`, Mario cai): log `twelvecb: game 0 ... pool 12`, `port 0 KO, stocks left -1, pool 11`, e a partida vai aos resultados. P7 em parte: `gmRemixTwelveCBCheckSlot` em `mnPlayersVSCheckReady` bloqueia o START com personagem derrotado e com as portas 3 e 4 ocupadas (compila, partida sem regressão no modo padrão e no 12CB; **o bloqueio no CSS ainda não foi visto em tela**). P12 e P14 feitos pelo overlay (`gm/gmremixtwelvecbhud.*`, `remix_hud_text_putf`, `remix_modes_fighter_name`): título e "Game N" no CSS e nos resultados, "P1 Stocks 48" e "P2 Stocks 48" no CSS; **visto em tela** (`css.sh` com `SSB64_REMIX_MODE=12cb css_pick_mario.txt`, e a partida de 1 stock até os resultados com "12-Char. Battle  Game 1"); a linha "Best <personagem> N TKOs" só aparece depois do primeiro jogo e **não foi vista em tela** (o harness não repete o CSS). **Faltam** (CSS e HUD, ainda não vistos): P5, P6 e P8 (CSS de 24 retratos, esconder as portas 3 e 4, contador de stocks 0 a 99 com bloqueio depois do início), o retrato travado do vencedor, P13 (reset de pausa, depende do T8.1), P15 (heap em 10 jogos seguidos), P16 (smoke com dois jogos seguidos pelo CSS).
- [ ] T6.2 1P (`f11_mode_singleplayer.md`, 19 passos): Remix 1P, All-Star, Multiman e Cruel Multiman, Home-Run Contest, BTT/BTP dos lutadores
  Remix, Practice 1P (L reinicia a fase), 1P com lutador Remix (entra aqui o que ficou sem ver no T5.1b); riscos: `multiman_ko_count_stop`,
  loop do `_assign_stage` do All-Star, unidade do HRC.
  Estado (2026-10-27): P1 feito: `sc/sc1pmode/sc1premixmode.*` (ids do asm, `gSC1PRemixMode`, `sc1PRemixModeGet/Is`, override de teste `SSB64_REMIX_1P_MODE=standard|bonus3|multiman|cruel|remix1p|allstar|hrc`) e `sc1premixmodename.*` com a suíte `tests/remix/test_1p_mode_names.c`; nenhum comportamento mudou ainda. P2 feito: menu 1P com as duas páginas do Remix (`mn/mn1pmode/mnremix1pmode.*` com a tabela do asm, ganchos em `mn1pmode.c`: seis botões na página 1 com Bonus 3 e Remix Modes, cinco na página 2, cursor com volta ao modo, B volta de página, música sem reiniciar); **visto em tela** (`SSB64_START_SCENE=8` com script de entrada: página 1, página 2, Multi-Man abre o CSS do Bonus 2 e o B volta à página 2 com o cursor no Multi-Man). P3 feito (`bonus2_css_2_1p`): `sc/sc1pmode/sc1premixbonusexit.*` é chamado por `mnPlayers1PBonusFuncRun` no START do CSS do Bonus 2; com Bonus 3, Multi-Man, Cruel ou HRC passa o RNG de 1 a 64 passos pelo frame, leva a cena ao 1P (`nSCKind1PGame`) com lutador, traje, tempo 5 e fase 0 e grava o save; Remix 1P e All-Star não passam por esse CSS e os bônus vanilla seguem como antes. As regras puras (`sc1premixmoderules.*`) têm a suíte `tests/remix/test_1p_mode_rules.c` (71 checagens no total). **Visto em tela** (`SSB64_START_SCENE=8`: Remix Modes, Multi-Man, CSS do Bonus 2, DK, START): a cena 52 abre com o DK na primeira fase do 1P (Hyrule, tempo 05:00). Ainda sem lógica de modo: a partida é o 1P padrão. **Faltam** do P3: sorteio de lutador "Random" (o CSS do Bonus 2 não tem Random) e o título do CSS com o nome do modo. P9 feito (`sc/sc1pmode/sc1premixmultiman.*`, regras puras em `sc1premixmoderules.*`; 86 checagens): Multi-Man e Cruel começam na fase Zako (Duel Zone) do 1P, com 1 stock, sem item, sem team attack, tempo infinito, knockback dos polígonos 0x15 (Cruel 0x18) e do jogador 0x16 só no Cruel, polígonos que reaparecem para sempre como um dos 12 Fighting Polygons ao acaso (`sc1PGameSpawnEnemyTeamNext`), KOs que não encerram a partida nem escrevem as estatísticas de bônus (o contador de KOs é próprio e para quando o jogador cai; o asm deixa de gravar as estatísticas, `crash_prevent`). **Visto em partida** (menu, Multi-Man, DK, START): Duel Zone, DK com 0%, três polígonos e novos polígonos de kinds 14 a 25 depois, sem crash. **Faltam** (visto só o início, não o fim): o HUD com a grade de 30 ícones do Zako ainda aparece em vez do contador de KOs (P10), o fim da partida e a volta ao CSS do Bonus 2 (P12), pular o preview (P11), `multiman_ko_count_stop` do Cruel conferido em partida, e o botão de reset. P10 feito: a grade de 30 ícones do Zako não é criada no Multi-Man e o contador "KOs N" sai pelo overlay (slot 28, `sc1PRemixMultimanDrawHud` em `sc1PGameFuncUpdate`); **visto em tela** "KOs 0" e **no log** `multiman: enemy KO, kos=1` seguido de `player KO, kos=1` (o contador parou), mas o número 1 em si não foi capturado em tela (o harness não sabe o frame do KO). Hoje o KO do jogador leva à tela de continue do 1P, que o Remix troca pelo retorno ao CSS do Bonus 2 (P12). P11 em parte: Multi-Man e Cruel pulam a tela de título (`sc1PIntroStartScene` em `sc1PManagerUpdateScene`, como o `_initial_screen_set`); o preview do asm (`_preview_skip`, `_preview_song_skip`) não foi comparado com o preview do port. P12 em parte: sem tela de continue, o KO do jogador encerra a corrida e volta ao CSS do Bonus 2 (`sc1PRemixMultimanLeaveRun`; **visto**, log `multiman: run ended, kos=0 restart=0` e cena 20), e o reset reinicia a corrida (`sc1PRemixMultimanRestartsRun`, testado nas regras puras, **não visto em partida**). **Faltam** do Multi-Man: botão e gráfico de reset (`_reset_button`, `_reset`), gravar o recorde de KOs (`_ko_save`) e mostrá-lo no CSS, `_percent_fix`, Bonus 3. Próximo: P13 a P15 (Home-Run Contest), depois Remix 1P e All-Star.
- [ ] T6.3 Training (`f11_mode_training.md`, 14 passos; não existe save state no Remix): menu Z, D-pad (congelar, avançar, trocar modelo, reset),
  shield break, OOS, DI practice, spam, spawn custom, música, fundo; opções de CPU dependem do M7; conferir Game Speed no Training e o
  `gITRemixExtendedToggles` zerado em `sc1ptrainingmode.c:700`.
- [x] T6.D1 Atalhos de teste dos modos num só módulo (feito 2026-10-27, débito apontado na autoavaliação do CP6c): `remix_debug_env_int` e
  `remix_debug_env_text` em `port/remix/remix_debug.*` substituem os `getenv`/`atoi` soltos e os `extern` em linha de `gmremixmode`,
  `gmremixkoth`, `gmremixsmashketball` e dos atalhos `SSB64_VS_STOCKS/TIME/STAMINA` de `scmanager.c` (os `getenv` anteriores ao M6 ficam).
  **Visto:** build limpo; KOTH (`SSB64_KOTH_GOAL`, `SSB64_KOTH_LINE`) e Smashketball (`SSB64_SMASH_TYPE`, `SSB64_SMASH_GOAL`) em partida. Commit `57ecf6a`.
- [x] T6.D2 `gm/gmremixmode.c` (335 linhas, três papéis) dividido (feito 2026-10-27): `gmremixmode.c` (modo da partida, 89 linhas),
  `gmremixmodecss.c` (perfis e ciclo de CSS por modo, 204) e `gmremixtimedstock.c` (regras de Timed Stock, 69); a API de `gmremixmode.h` não mudou.
  **Visto:** build limpo e partida Tug of War com Timed Stock (`rules=3`). Commit `aa4441b`.
- [ ] T6.D3 (infraestrutura pronta em 2026-10-27: `tests/remix/`, alvo `run_remix_mode_tests`, suíte do 12CB; faltam as suítes dos outros modos) Testes de unidade das regras puras dos modos (regra do projeto: todo comportamento novo tem teste; hoje só há validação em partida).
  Executável headless de um comando (alvo CMake `remix_mode_tests`, sem ROM nem janela), cobrindo: Timed Stock (`gmRemixTimedStockScore`
  com e sem Tug of War, `UsesTimeScore`, `IsTimeUp` com tempo infinito, `AllowsSuddenDeath`), ciclo de CSS (`BeforeCSS`/`LeaveCSS`/`StartMatch`:
  perfil forçado, stocks/meta/times guardados por modo, restauro do vanilla), pontuação e vitória do KOTH (rei com buffer, contagem por time
  uma vez por quadro, meta), crédito de KO e revive do Tug of War, fila e troca do Tag Team (`remix_modes_tagteam_fighters`, Random sem repetir),
  placar e meta do Smashketball, Stamina (HP, KO, morte única). Se a regra depende de estado do jogo, extrair a parte pura para função
  que recebe os dados por parâmetro, sem mudar o comportamento. Um teste novo a cada modo que entrar daqui em diante (T6.1f, T6.2, T6.3).
  **Aceite:** o alvo roda e passa; documentado em `docs/remix/README.md` e no AGENTS do projeto.
- [ ] T6.D4 Arquivos de modo acima de 300 linhas (regra de arquivos pequenos): `gmremixkoth.c` (425) e `gmremixsmashketball.c` (410) divididos por
  responsabilidade (estado e pontuação, colina/bola, fim de partida), como já foi feito com `kothhill/draw/hud` e `smashketballboard`.
  **Aceite:** build e a mesma partida de validação do modo.
- [ ] T6.D5 Revisão linha a linha do código portado pelos workers (Stamina P6 a P15, Tug of War P1/P3): duplicação, funções longas, comentários
  que repetem código, `REMIX-TODO` sem dono; correções com build e partida do modo. **Aceite:** achados corrigidos ou registrados como `REMIX-TODO(<modo>)`.
- [ ] T6.D6 Ícones de stock do Tag Team por personagem da fila e roubo de stock em times (`REMIX-TODO(tagteam-icons)`, `REMIX-TODO(tagteam-steal)`),
  hoje divergentes do Remix.
- [x] **CP6d** (checkpoint a pedido, 2026-10-27): passe de limpeza (T6.D1 atalhos de teste em `remix_debug_env_*`, T6.D2 `gmremixmode.c` dividido em três), infraestrutura de testes de unidade (`tests/remix`, alvo `run_remix_mode_tests`, 57 checagens: regras do 12CB e nomes dos modos 1P), 12-Character Battle jogável (pool de stocks, KO, sem salty runback, texto do CSS e dos resultados, bloqueio de START no CSS; sobras: CSS de 24 retratos e conjuntos, portas 3 e 4, reset de pausa, jogos seguidos), T6.2 P1 (modo 1P e `SSB64_REMIX_1P_MODE`) e P2 (páginas do menu 1P, vistas em tela). Novas tasks abertas: T6.D3 a T6.D6. Correção da estimativa: a soma da tabela estava errada (35×0,96 + 10×0,70 + 20×0,80 + 10×0,90 + 20×0,80 = 81,6, não 87,6); o total honesto do CP6c já era ~82%, não 88%. Estimativa ~82%. Não conferido em tela: bloqueio de START do 12CB, linha "Best" do 12CB, modos 1P além do menu.
- [x] **CP6c** (checkpoint a pedido, 2026-10-23): Stamina com fim de partida validado (P10 era o atalho de teste sem `team`), Tug of War (crédito de KO, revive, placar), King of the Hill (colina, rei, meta, faixa e medidores, respawn), Smashketball (bola, cestas, placar nativo, morte súbita, spawns, tipo), Tag Team (fila pelo menu do port, troca de lutador). Infra nova: `gmremixmode` (ciclo de CSS por modo), `remix_hud_text` (texto e barras ImGui), `remix_modes` (opções de modo no menu do port). Atalhos de teste: `SSB64_REMIX_MODE`, `SSB64_VS_STAMINA`, `SSB64_VS_TIME`, `SSB64_KOTH_LINE/GOAL`, `SSB64_SMASH_TYPE/GOAL/BALL_X`. Estimativa ~88%.
- [x] **CP6b** (checkpoint a pedido, 2026-10-23): T6.0 fechado (base dos modos, Timed Stock, texto do HUD, ciclo de CSS por modo `gmRemixModeBeforeCSS/LeaveCSS/StartMatch` com perfil por modo, `SSB64_REMIX_MODE`, `SSB64_VS_STAMINA`, `SSB64_VS_TIME`), Stamina portada exceto P10/P16/P17, primeiros passos de Tug of War e KOTH. O build compila; só T6.0 e o HUD do Stamina foram vistos em partida.
- [x] **CP6a** (checkpoint a pedido, 2026-10-22): sem código novo desde o CP5; oito relatórios de pesquisa dos modos e a lista de tasks do M6 reescrita a partir deles.
- **CP6** (fecha com T6.D3 a T6.D6 feitas; elas valem como parte do M6, não são opcionais).

## M7. IA

- [ ] T7.1 `AI.asm` (70 hooks), toggle Improved AI, hooks de CPU do Single Button Mode, os `cpu_*` marcados `REMIX-TODO(AI)` nos fighters,
  `VanillaLv10Attacks.asm`. **Aceite:** CPU nível 9 usa especiais e recupera com 10 lutadores Remix.
- **CP7**.

## M8. Restante da paridade

- [ ] T8.1 *(inclui Skip Start Cheer, Advanced Hurtbox Display e os sons JP do Link)* Todas as linhas `todo`/`partial` de `docs/remix/parity.md` que não couberam acima (Pause, Japan, Accessibility,
  Cheats, SwordTrail, Skeleton, SingleButtonMode, SpecialZoom, VsDemo e outros).
- [ ] T8.2 Sobras do M5: **CSS VS** (costumes extras além de `num_costumes`, C no team battle, ícones do D-pad e bandeiras J/E, bookend de
  bônus, `recent_randoms`, logos VS dos lutadores novos, lutador de 0 costumes, offsets `+0x10`, retratos 0x13 contra 0xA05, bug do
  `scale.vec.f.y` repetido), **stage select** (texto do tipo de variante, roulette do RANDOM, layout tournament), **painel por porta**
  (Skeleton, D-pad Map, DpadControl parcial/J/escudo, Shield Costume e ovo do Yoshi, StockMode Last e os modos que leem a tabela,
  Practice_1P L-retry, nomes das tags, Start With no Treino, Poison Heal e trava de stamina), **telas** (página de estatísticas do VS
  com texto, linhas de modo nos resultados, logos de transição, glifos e speed assist dos créditos, botão Gallery e 28 entradas,
  emblema e golpes dos 18 extras do Character Data, legends da pausa, RANDOM ALL, Random Select do CPU, legenda do item switch).
  Cada item com captura ou riscado com motivo.
- **CP8**.

## M9. Verificação final no desktop

- [ ] T9.1 Sweep completo (roster × specials, stages, itens) e partidas reais via CSS: 4 jogadores, cada modo, Training. Conferir também o que ficou sem efeito visto: esteira de Surface, bury/plunge, Game Speed no Training, volta do Lightning ao tamanho do modo, Full Results e Salty Runback.
- [ ] T9.1b Passagem de escuta pelo usuário (o lead não ouve): alguns stages, uma música de vitória, o Sound Test, SFX de golpes de
  lutadores Remix e os gráficos de menu do Remix; os `audio.wav` de cada run ficam em `build/agent-shots/vs/<run>/`.
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
| Fighters (roster, specials, projéteis, entradas) | 35% | ~96% |
| Itens | 10% | ~70% |
| Stages (loader, seletor, hazards) | 20% | ~80% |
| Áudio | 10% | ~90% |
| Settings, menus, modos | 20% | ~82% |
| Verificação final | 5% | 0% |
| **Total ponderado** | | **~82%** (35×0,96 + 10×0,70 + 20×0,80 + 10×0,90 + 20×0,82 = 82,0; corrigido no CP6d, a soma anterior de 87,6 estava errada) |

A partir de T0.2 a estimativa vem da tabela de paridade (`ported` / total por área).
