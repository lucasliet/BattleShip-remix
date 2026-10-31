# F14 / T8.1: pesquisa dos asm do Smash Remix sem equivalente no port

Escopo: 21 arquivos de `.toolchain/pkgs/smashremix-src/src/`. Só leitura. Fonte dos nomes de função vanilla: `build/remixasm/refs/vanilla_funcs.txt`, que lista a função cujo endereço inicial é o mais próximo abaixo do hook (aproximação, não prova; onde o hook cai em overlay sem nome exato, a função citada é indicativa).

Método da busca no port: `grep -i` por nome do arquivo, nome do toggle, nomes de scope e constantes em `decomp/src`, `decomp/include` e `port/`. "none" significa que a busca não achou nada conclusivo; "unknown" significa que achou algo que não prova nem descarta o equivalente.

Correções à auditoria `docs/remix/notes/f14_parity_audit.md`, confirmadas nesta pesquisa: SinglePlayerMenus, VsRemixMenu, OnHit, PokemonAnnouncer e Skeleton (`REMIX-TODO(Skeleton)` existe em `decomp/src/ft/ftdisplaymain.c:965`), Practice (flash on Z-cancel tem equivalente em `ftremixzcancel.c:189`) e Menu (desvio aceito no ROADMAP, linha 238) estão mal classificados lá. Walljump tem todos os patches comentados.

Tamanho: S = um gancho; M = algumas funções; L = subsistema novo.

---

## Walljump.asm

- **Faz:** pulo na parede (walljump), com ação própria, momentum horizontal e contador de pulos. O arquivo tem 4 hooks (`checkwall`, `walljumpaction`, `wj_momentum`, `wj_jumpflag`, em 0x80140300, 0x8013FDC4, 0x8013FEC0, 0x8013FED0), mas **todos os `OS.patch_start` estão comentados**, então nada é aplicado no build atual.
- **Toggle/menu:** nenhum. Em `Toggles.asm` só existe "Wall Teching" (outra mecânica). Default: n/a.
- **Port:** none. Busca `wall.?jump` sem resultado em `decomp/` e `port/`.
- **Tamanho:** M (ação nova, 4 pontos de física, estado; precisa decidir se o Remix realmente ativa isso).

## DKMode.asm

- **Faz:** modo DK e modo Lanky. Redimensiona cabeça e braços (`_resize_head`, hook em `ftDisplayMainProcDisplay`, 0x800F296C) e troca os ids de personagem por uma tabela `fighter_DK_mode` de ~40 entradas. Ativação por easter egg: letras D/K/L nas tabelas de id dos retratos da 12CB, gravadas em `dk_mode_enabled` (1 = DK, 2 = Lanky).
- **Toggle/menu:** nenhum no `Toggles.asm`; é ativado pelos retratos. Default: OFF (0).
- **Port:** none. `dk.?mode`, `DK_MODE` e `fighter_DK_mode` sem resultado. "Lanky" existe como lutador (`decomp/src/ft/ftremix/lanky/`), não como o modo.
- **Tamanho:** M (tabela de 40 entradas, redimensionamento e detecção do easter egg).

## Cheats.asm

- **Faz:** 5 patches permanentes, sem toggle: desbloquear tudo (0x800A3DEA), sem anti-aliasing no display list do f3dex (0x80032BF8), stock sempre numérico (0x8010F8D4), música desligada (0x800216FC) e um quinto patch (0x80105C08) sem descrição.
- **Toggle/menu:** nenhum próprio. "Disable Anti-Aliasing" (`entry_disable_aa`) é um toggle separado. Default: n/a.
- **Port:** unknown. Existe `port_cheat_unlock_all` (`port/enhancements/UnlockCheats.cpp`, CVar `gCheats.UnlockAll`, usado em `decomp/src/lb/lbbackup.c:266`), mas não há vínculo comprovado com o patch 0x800A3DEA.
- **Tamanho:** S (um gancho por cheat; o desbloqueio já tem análogo no port).

## timeouts.asm

- **Faz:** 11 patches trocam o `bne` do timer de 18000 frames (5 min) por `b` nas telas de título, modo de seleção, 1P, opções, resultados e créditos. Impede a troca automática de tela.
- **Toggle/menu:** nenhum; sempre ativo. Default: n/a.
- **Port:** none. Busca `timeout` em `decomp/` e `port/` só acha coisas sem relação (`Updater.cpp`, `itremixsupermushroom.c`).
- **Tamanho:** S se o port tiver os timers; precisa confirmar primeiro se existe timer de tela no port.

## VsDemo.asm

- **Faz:** deixa personagens Remix aparecerem na tela VS Demo (luta de demonstração). Usa a tabela de texturas de nome 1P (`SinglePlayer.name_texture_table`), corrige offsets, habilita personagens focados e não focados e os stages Remix. 5 patches, em overlay sem nomes exatos no refs.
- **Toggle/menu:** nenhum (zero referências a `Toggles`). Default: n/a.
- **Port:** none. `vs.?demo|vsdemo` sem resultado. `ftremixdemoscripts.c` e `scautodemo.c` são de outros escopos.
- **Tamanho:** M (tela de demo, textura de nome e seleção de personagens).

## AerialAttackFastFall.asm

- **Faz:** permite fast fall durante ataque aéreo (nair, fair, bair, uair, dair; 5 hooks em 0x80129E38-0x80129E88). Lê o toggle; ligado, entra na rotina que permite fast fall; desligado, usa a rotina padrão.
- **Toggle/menu:** "Fast Fall Aerials" (`Toggles.asm:2438`), em Gameplay Settings. Default: OFF nas 4 builds (CE, TE, NE, JP).
- **Port:** none. Só o símbolo `REMIX_SYM_Toggles__entry_fast_fall_aerials` (`port/remix/remix_symbols.h:4062`), sem nenhum leitor no decomp ou no port. `ftremixvanillaprocs.c:497` mapeia 0x800D9160 para `ftPhysicsApplyAirVelDriftFastFall`, mas isso não é o gancho do toggle.
- **Tamanho:** S (um leitor do toggle no caminho de ataque aéreo).

## CharacterSelectDebugMenu.asm

- **Faz:** menu de debug na seleção de personagens. Botão de debug por porta, páginas de opções (escala, esqueleto etc.), criação de botões e rótulos, paginação e setas. 25 patches, 2745 linhas.
- **Toggle/menu:** "CSS Panel Menu" (`Toggles.asm:2381`). Default: ON em CE, NE e JP; OFF em TE.
- **Port:** partial. O painel por porta existe (`decomp/src/gm/gmremixpanel.c`, `port/remix/remix_player_panel.h`, ROADMAP T5.2 fechado). Não achei botão nem menu de debug do CSS (`debug (button|menu)` sem resultado).
- **Tamanho:** L (menu de páginas com renderização e entrada, sobre o painel que já existe).

## Crash.asm

- **Faz:** patches do depurador de erro do N64. Zera as máscaras de botão (a0 = 0) nas checagens de `syErrorReportCPUBreakFault`, `syErrorPrintf` e `syErrorFileLoaderThread8`, e abre o crash debugger sem a sequência de botões (0x80023204, 0x8002366C, 0x80023480). Não tem relação com o lutador Crash Bandicoot (`crash/`).
- **Toggle/menu:** nenhum; sempre ativo. Default: n/a.
- **Port:** unknown. `decomp/src/sys/debug.c` tem `syDebugReportCPUBreakFault`, mas não há vínculo com a abertura automática. No PC o tratamento de erro é outro.
- **Tamanho:** S, ou n/a se a decisão for não portar o depurador do N64.

## Menu.asm

- **Faz:** framework de menus do Remix. Desenha entradas, cursor, títulos, e importa e exporta valores para a SRAM. É a UI da tela de Toggles.
- **Toggle/menu:** n/a (biblioteca).
- **Port:** replaced. O menu do port é outro (`port/gui/PortMenu.cpp`, `port/remix/remix_toggles.*`). O ROADMAP (linha 238) registra o desvio: "o menu desenhado pelo Remix (`Menu.asm`) não é replicado, o do port tem as mesmas entradas e valores".
- **Tamanho:** n/a (desvio já aceito). L se a decisão mudar para replicar o desenho do asm.

## Settings.asm

- **Faz:** um gancho em 0x800A1B48, logo depois de gravar as configurações VS, escreve em `Global.vs` os valores de torneio: game_mode 3, time 8, stocks 3, team_attack 1, item_frequency 0. O comentário diz "This should not be called", mas o `j` está ativo.
- **Toggle/menu:** nenhum. Default: n/a. "Tournament" aparece só como perfil de toggles (`port/remix/remix_toggles.h:5`).
- **Port:** unknown. Nenhuma busca por defaults de torneio em `Global.vs` ou no VS acha vínculo conclusivo.
- **Tamanho:** S (um gancho que grava cinco bytes).

## SRAM.asm

- **Faz:** layout e leitura e gravação da SRAM Remix (`save_`, `load_`, `check_saved_`, `mark_saved_`, `initialize_`). Hooks em `lbBackupWrite` e `lbBackupIsSramValid` (0x800D462C, 0x800D4670). Os blocos de toggles são reservados com `SRAM.block`.
- **Toggle/menu:** nenhum; é infraestrutura. Default: n/a.
- **Port:** unknown. O save do port é `port/port_save.cpp` (a SRAM como arquivo), e `port/remix/remix_ram_image.cpp` cuida da imagem. Nenhuma correspondência por nome para `save_`/`load_` do asm.
- **Tamanho:** S se só for preciso manter compatibilidade de layout; n/a se o port mantiver o próprio save.

## SFXReplace.asm

- **Faz:** troca os sons de passo, corrida, pouso e pouso forte por tabelas Remix (`STEP_SOUND_TABLE` e afins). Hooks em `sfx_replace` (0x800DF864, perto de `ftMainParseMotionEvent`) e `sfx_replace_down_bounce` (0x80144470, perto de `ftCommonDownBounceUpdateEffects`).
- **Toggle/menu:** nenhum. Default: n/a.
- **Port:** unknown. Busca `sfx_replace`, `STEP_SOUND` e `hard.?landing` sem resultado. O ROADMAP (T3.2) fechou os SFX, mas não diz se essa troca está ativa.
- **Tamanho:** M (tabelas de passo e pouso por personagem e chamada no evento de movimento).

## Practice.asm

- **Faz:** duas melhorias de prática. (1) `overlay_colour_` (hook 0x800E260C) pinta o lutador conforme o estado (landing, turn). (2) `flash_on_z_cancel_` (hook 0x80150AE8) pisca o pouso dentro da janela de Z-cancel.
- **Toggle/menu:** "Color Overlays" (`Toggles.asm:2382`, default OFF nas 4 builds) e "Flash On Z-Cancel" (`Toggles.asm:2384`, default OFF nas 4 builds). Ambos estão em Miscellaneous.
- **Port:** partial. Flash on Z-cancel: `decomp/src/ft/ftremix/ftremixzcancel.c:189` (`ftRemixZCancelFlashOnLanding`, cita Practice.asm). Color overlay: none, só o símbolo `REMIX_SYM_Toggles__entry_practice_overlay` (`port/remix/remix_symbols.h:4136`).
- **Tamanho:** M (o efeito de cor por estado precisa de uma operação de cor no render do lutador).

## SinglePlayerEnemy.asm

- **Faz:** permite que outra porta humana seja o inimigo ou a CPU nos modos 1P (`enemy_port`, `enemy_selected`, `onep_control_port`, `onep_control_respawn`, `onep_control_shield`, `_afk_1p`, `setup_`). 10 patches.
- **Toggle/menu:** nenhum toggle no `Toggles.asm`; o comentário "when toggle is on" não tem toggle lido. A escolha vem do menu 1P (`SinglePlayerMenus.asm`). Default: n/a.
- **Port:** none. `enemy_port|SinglePlayerEnemy|enemy.?select` sem resultado (os achados de "enemy" são KOs de Multi-Man, outro assunto).
- **Tamanho:** L (novo fluxo 1P: mapeamento de portas, controle, respawn, escudo e AFK).

## SinglePlayerMenus.asm

- **Faz:** tela 1P Remix: botões do menu 1P, paginação, cursor com volta, saída da página 2 e criação dos botões. 34 patches, com hooks em funções como `mn1PModeMake1PGame` (0x80131E34) e `mn1PModeInitVars` (0x80132910).
- **Toggle/menu:** nenhum. Default: n/a.
- **Port:** ported. `decomp/src/mn/mn1pmode/mnremix1pmode.c` cita o asm e `mnremix1pmode.h` tem `menu_button_table` e `remix_menu_button_table`. A auditoria F14 marcou unknown: correção.
- **Tamanho:** S (sobras do 1P no T8.2 do ROADMAP).

## VsRemixMenu.asm

- **Faz:** menu VS com o botão "Remix Modes" (página 1) e os cinco modos Remix (página 2). Também configura o modo (`mode_specific_setup_`) e troca o anúncio ao alternar FFA e Team (`update_announcer_on_toggle_`). 32 patches.
- **Toggle/menu:** nenhum. Default: n/a.
- **Port:** ported. `decomp/src/mn/mnvsmode/mnremixvsmode.c` cita o asm; `decomp/src/gm/gmremixmodecss.c` cita `mode_specific_setup_`. ROADMAP T5.3 ("Remix Modes") fechado. A auditoria F14 marcou unknown: correção.
- **Tamanho:** S (sobras no T8.2).

## SwordTrail.asm

- **Faz:** tipos customizados de sword trail: struct do trail, `initial_setup_`, `axis_setup_`, `second_check_` e `draw_trail_` (hooks em 0x800E6740 a 0x800E682C, 0x800F253C e 0x800F1060). 7 patches.
- **Toggle/menu:** nenhum. Default: n/a.
- **Port:** partial. Só o flag de afterimage do Link: `decomp/src/ft/ftremix/ftremixlinkshared.c:27-28` (`ftRemixLinkSharedHasSwordTrail`, usado em `decomp/src/ft/ftmain.c:4512`). Tipos customizados e o desenho do trail não aparecem (`new_trail|trail_type|custom.*sword` sem resultado).
- **Tamanho:** L (malha e desenho do trail, struct e hooks de render; ROADMAP T8.1 cita SwordTrail).

## Skeleton.asm

- **Faz:** força o caminho de display de esqueleto (`0x0800`) por porta, controlado por `enable_for_port` (0 por padrão). Hooks em `skeleton_mode_1_` (0x800F24B0, `ftDisplayMainDrawAll`) e `skeleton_mode_2_` (0x800F2228, `ftDisplayMainDrawSkeleton`).
- **Toggle/menu:** sem entrada em `Toggles.asm`. É o interruptor "Skeleton" por porta do painel CSS (ROADMAP T5.2b). Default: OFF.
- **Port:** partial. O caminho vanilla de esqueleto existe (`decomp/src/ft/ftdisplaymain.c:860`, `ftDisplayMainDrawSkeleton`). O forçado por porta não: `decomp/src/ft/ftdisplaymain.c:965` tem `REMIX-TODO(Skeleton)`. A auditoria F14 disse que não existe: correção.
- **Tamanho:** S (um gancho em `ftDisplayMainDrawAll` com o valor por porta do painel).

## MagnifyingGlass.asm

- **Faz:** zoom customizado do modelo dentro da lupa. `apply_custom_zoom_` (0x80110E80) lê `Character.magnifying_glass_zoom` por `char_id`, e há uma `custom_matrix_array` por porta.
- **Toggle/menu:** nenhum. Default: n/a.
- **Port:** none para o zoom por personagem. `magnifying_glass_zoom` e `magnify.*zoom` acham só `magnify_scale` do vanilla (`decomp/src/if/ifcommon.c:1512-1515`). A lupa vanilla existe (`ifCommonPlayerMagnifyUpdateViewport`, `ifcommon.c:1633`).
- **Tamanho:** S (tabela por personagem e um gancho no viewport da lupa).

## OnHit.asm

- **Faz:** um gancho em `ftParamStopVoiceRunProcDamage` (0x800E8244). Quando há entrada, chama `Character.on_hit.table[char_id]`. É a infraestrutura para efeitos ao acertar um lutador.
- **Toggle/menu:** nenhum. Default: n/a.
- **Port:** ported. `decomp/src/ft/ftremix/ftremixchartables.c:85` (`ftRemixRunOnHit`), que cita `OnHit.asm 0x800E8244` na linha 6. A auditoria F14 marcou todo: correção.
- **Tamanho:** S (não encontrei pendência própria).

## PokemonAnnouncer.asm

- **Faz:** anunciador do Pokémon Stadium: `there_goes_`, `delay_`, `pokemon_name_`, sons de KO, combo, anúncio de hit e de arremesso. 29 patches em várias funções vanilla (por exemplo `ifCommonBattleEndPlaySoundQueue`, 0x801137C4).
- **Toggle/menu:** "Pokemon Announcer" (`Toggles.asm:2556`), opções STADIUM, ALL STAGES e OFF. Default: STADIUM em CE e NE; OFF em TE e JP.
- **Port:** ported. `decomp/src/if/ifremixannouncer.c` cita o asm; `decomp/src/if/ifremixhitannouncer.c` cobre o hit. ROADMAP T2.5 fechado. Sobra: `REMIX-TODO(PokemonAnnouncer)` em `decomp/src/ft/ftcommon/ftcommonthrow.c:107`.
- **Tamanho:** S (a sobra de arremesso).

---

## Tabela resumo

| file | does | port state | size |
|---|---|---|---|
| Walljump.asm | pulo na parede; patches comentados (inativo) | none | M |
| DKMode.asm | modo DK e Lanky por easter egg na 12CB | none | M |
| Cheats.asm | desbloquear tudo, sem AA, stock numérico, sem música | unknown | S |
| timeouts.asm | 11 patches que impedem troca de tela por timer de 5 min | none | S |
| VsDemo.asm | personagens Remix na tela VS Demo | none | M |
| AerialAttackFastFall.asm | fast fall em ataque aéreo (toggle OFF) | none | S |
| CharacterSelectDebugMenu.asm | menu de debug na CSS (toggle CSS Panel Menu) | partial | L |
| Crash.asm | depurador de erro do N64 (sem relação com o lutador) | unknown | S ou n/a |
| Menu.asm | framework do menu de Toggles | replaced (desvio ROADMAP:238) | n/a |
| Settings.asm | defaults de torneio em Global.vs no VS | unknown | S |
| SRAM.asm | layout e save da SRAM | unknown (port_save.cpp) | S ou n/a |
| SFXReplace.asm | troca de SFX de passo e pouso | unknown | M |
| Practice.asm | Color Overlays (none); Flash On Z-Cancel (ported) | partial | M |
| SinglePlayerEnemy.asm | porta humana como inimigo ou CPU no 1P | none | L |
| SinglePlayerMenus.asm | tela 1P Remix e seus botões | ported (mnremix1pmode.c) | S |
| VsRemixMenu.asm | menu VS e modos Remix | ported (mnremixvsmode.c) | S |
| SwordTrail.asm | tipos customizados de sword trail | partial (só Link) | L |
| Skeleton.asm | esqueleto forçado por porta (toggle do painel CSS) | partial (REMIX-TODO:965) | S |
| MagnifyingGlass.asm | zoom por personagem na lupa | none | S |
| OnHit.asm | tabela de efeitos ao acertar | ported (ftRemixRunOnHit) | S |
| PokemonAnnouncer.asm | anunciador Pokémon Stadium (toggle) | ported (T2.5) | S |
