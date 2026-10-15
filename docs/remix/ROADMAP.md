# Smash Remix 2.0.1 no port PC: roadmap definitivo

Este é o **único plano** da implementação do Remix. Quem trabalhar no Remix (agente ou humano) segue este arquivo na
ordem, sem abrir frentes fora dele. Ele substitui `docs/remix/implementation_plan.md` (histórico) e os planos antigos em
`~/.claude/plans/`. Widescreen/aspect ratio está **fora do escopo** (o jogo fica em 4:3).

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
- Testes: `vs.sh` (`SSB64_VS_FKINDS`, `SSB64_VS_STAGE=<decimal>`, `SSB64_VS_ITEMS`, `SSB64_REMIX_FORCE_ITEM`,
  `SHOTS=`) e `css.sh` (fluxo CSS) em `/Users/lucas/.claude/jobs/b7b2d850/tmp/`; capturas em `build/agent-shots/vs/`.
  Crash `CAMetalLayer nextDrawable` é flake do macOS: repetir antes de investigar.

## Posição atual

- **Milestone:** M1. **Task:** T1.3 (itens). Feito: todos os 13 itens pegos/usados pelos CPUs com efeito conferido,
  exceto o Golden Gun, cujo projétil dourado agora dispara (12 tiros, sem crash) mas falta conferir o visual em captura;
  depois disso, fechar T1.3 e seguir para T1.4.
- **Último checkpoint:** ver `git log` (checkpoint T1.1-T1.3 de 2026-10-14).
- **Estimativa geral:** ~58% (cálculo no fim deste arquivo; recalcular a cada checkpoint pela tabela de paridade, T0.2).

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
- [ ] T1.3 *(em andamento: harness `cpusweep.sh` com 3 lutadores e `SSB64_REMIX_ITEM_LOG=1`; ok: Super/Poison Mushroom
  (Size ligado), Blue Shell (transform corrigido), Lightning (Size e cancelamento do cogumelo ligados), Deku Nut, Franklin,
  Pitfall, Dango, P-Wing, Stopwatch; Basketball cai e quica (procs do ITDesc e estado por bola), resto vai para o T6.1;
  Psyduck é conteúdo desativado no Remix 2.0.1 (`pokemon_table` comentada), n/a; Golden Gun: projétil portado, falta captura)*
  Verificar pickup e efeito dos itens padrão em partida (`SSB64_REMIX_FORCE_ITEM`, CPU ou input script que pega o
  item): Super Mushroom, Poison Mushroom, Blue Shell, Lightning, Deku Nut, Franklin Badge, Pitfall, Golden Gun (com o
  projétil, patches 1 e 3), Dango, P-Wing, Stopwatch, Basketball, Psyduck. Cada bug vira correção. **Aceite:** captura
  do efeito de cada item; tabela no doc `remix_standard_items_2026-10-14.md`.
- [ ] T1.4 Verificar o comportamento dos 15 hazards portados (Great Bay, Twilight City, MMM, BBH, Corneria, GB Land,
  Pirate Land, Jungle Japes + KlapTrap, Casino Night, Onett + Car, Toad's Turnpike, Rainbow Road, Remix RTTF, Metallic
  Madness + RobotBee, Spawned Fear + Cacodemon): captura no frame em que o hazard age, e fechar os `REMIX-TODO` de cada
  um (lista em `remix_stages_and_hazards_2026-10-14.md`, seção Open). **Aceite:** cada stage com captura do hazard agindo.
- [ ] T1.5 Hazards restantes: Time Twister, Scuttle Town + ScuttleMinion, Dracula's Castle/Reapers (depois de T2.2),
  `peach_bumper_hitbox_`, bombas do Bowser's Stadium, nuvens do Yoshi's Island, `platform_movement_`, vento e downtime
  do Dream Land, `mk_remix_hardcoding_1-4`, `dream_greens_hardcoding_animation_4`, Smashketball (stage). **Aceite:** idem T1.4.
- [ ] T1.6 Stage select: enquadramento do preview nos stages custom, emblema de série (`series_logo_table`), Training
  sem tabelas vanilla fora do limite, variantes DL/Omega/Remix (`variant_table`), roulette do Random, layout tournament.
  **Aceite:** captura do seletor em 3 páginas, uma variante e uma partida em Training num stage custom.
- [ ] T1.7 Sweep completo: 78 fighters × NSP/DSP/USP, ids de stage 41 a 215, 17 itens. **Aceite:** zero crash fora de flake.
- **CP1** (checkpoint).

## M2. Subsistemas de gameplay (destravam os `REMIX-TODO` dos fighters, itens e stages)

- [ ] T2.1 GFX + GFXRoutine: dados `src/gfx/*` no `.o2r` via Torch e no `lbParticle`, rotinas de colanim com id >= 0x56 em
  `dGMColScriptsDescs`, hooks de `GFX.asm`/`GFXRoutine.asm`. **Aceite:** efeitos com `REMIX-TODO(GFX)` aparecem (Mewtwo,
  Crash, Dedede, Goemon, Sheik, Flashbang, fireball do Dr. Mario).
- [ ] T2.2 CharEnvColor: tabelas e override de cor por porta (`ftremixenvcolor.c`). **Aceite:** Cloaking Device com fade,
  Dracula/Reapers com a cor do stage.
- [ ] T2.3 Reflect: nibbles CUSTOM, dispatch (`ftremixreflect.c`), Franklin Badge, absorb de Marina/Dedede.
  **Aceite:** projétil refletido/absorvido nos três casos.
- [ ] T2.4 Size fases (b) física/peso e (c) cosméticos; estados GIANT/TINY do Super/Poison Mushroom e do laser do
  Metallic Madness. **Aceite:** Super Mushroom aumenta o lutador com física coerente e volta ao normal.
- [ ] T2.5 PokemonAnnouncer (`ftremixannouncer.c`, ~30 inserções). **Aceite:** fala em pelo menos 3 golpes.
- [ ] T2.6 Kirby: `KirbyHats.asm` (magic hat, `restore_magic_hat_`, `clear_magic_hat_charge`) e copy dos poderes Remix.
  **Aceite:** Kirby copia 3 lutadores Remix e usa o NSP com o chapéu certo.
- [ ] T2.7 Outros sistemas de golpe: `Knockback.asm` (aleatorizador de ângulo), `Hitbox.asm`, `command.asm` restante,
  `CustomGrabAction.asm`, `Surface.asm`, `Blastzone.asm`, `Spawn.asm`, `Camera.asm`, `Damage.asm`, `Poison.asm`.
- [ ] T2.8 `grep -rn REMIX-TODO decomp/src` sem ocorrências fora de AI e de modos/menus (que têm milestone próprio).
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
- [ ] T4.3 `grRemixHazardsAreOn`/`grRemixMovementIsOn` e o switch de itens lendo os toggles; perfis CE/TE/NE/JP.
- **CP4**.

## M5. CSS, menus e telas

- [ ] T5.1 CSS de 30 slots + bookend de bônus + variantes (polygon, metal, J, E), tabela de figurinos, Random com pool e toggle.
- [ ] T5.2 Painel por porta do CSS (`css/*.asm`).
- [ ] T5.3 VS menu com a página Remix Modes, Mode Select, transições, resultados (logos, vitórias, estatísticas de VS),
  créditos estendidos, Character Data dos 97, name tag / player tag, Gallery.
- [ ] T5.4 Remoção dos hacks de contenção que sobrarem no CSS/menus (lista no doc do CSS).
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
- [ ] T10.2 Validação em hardware pelo usuário (checklist de `docs/switch_port_2026-10-12.md`), RAM com o `.o2r` de 60 MB.
- **CP10**.

---

## Estimativa de progresso (atualizar em cada checkpoint)

| Área | Peso | Feito |
|---|---|---|
| Fighters (roster, specials, projéteis, entradas) | 35% | ~85% |
| Itens | 10% | ~70% |
| Stages (loader, seletor, hazards) | 20% | ~65% |
| Áudio | 10% | ~50% |
| Settings, menus, modos | 20% | ~15% |
| Verificação final | 5% | 0% |
| **Total ponderado** | | **~58%** |

A partir de T0.2 a estimativa vem da tabela de paridade (`ported` / total por área).
