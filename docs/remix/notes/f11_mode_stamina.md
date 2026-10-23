# F11 mode: Stamina, Timed Stock and Teams (research note for the port, ROADMAP M6 / T6.1)

Fonte: `.toolchain/pkgs/smashremix-src/src/Stamina.asm` (2103 linhas, lido por completo), `TimedStock.asm` (237 linhas, lido por completo), `Teams.asm` e referências cruzadas (`grep` por `team`, `Global.vs.teams`, `Stamina.`, `TimedStock`). Decomp lido em `decomp/src/`. Nada foi compilado nem rodado. Onde o asm não fecha, está marcado como **incerto**.

Convenção de nomes: `Stamina.X` é o label do `scope Stamina` no asm; RAM é o segundo argumento de `OS.patch_start(ROM, RAM)`. Os endereços de comentário do asm e a coluna "decomp" de `docs/remix/gameplay_m_z.md` não foram usados como prova; a correspondência com o decomp é feita pelo nome e pelo corpo da função.

---

## 1. O que é o modo

**Stamina:** cada lutador começa com 200 de HP (`TOTAL_HP` guarda 199 e a exibição soma 1). O dano é acumulado como porcentagem normal e subtraído do HP. Quando o dano passa do total (`TOTAL_HP < percent`), o lutador é nocauteado uma única vez, sem reviver por stock, e a partida acaba quando não sobra lutador com stock. O modo fixa 1 stock por jogador, faz knockback como se o lutador estivesse em 20% sempre, e não tem limite de stock para os outros modos.

**Timed Stock** (`game_mode 3`, bits stock+time): a partida tem tempo e stock ao mesmo tempo. Quando o tempo acaba, vence quem tem mais pontos calculados por `score_fix_` (KOs menos mortes; ou stocks restantes no Tug of War). Empate no tempo leva a morte súbita, salvo em KOTH e Smashketball.

Seletores:
- `Global.vs.game_mode` (0x800A4D0B, `VS_MODE` no asm): 1 = time, 3 = stock (ou timed stock após o patch), 5 = Stamina (`STAMINA_MODE`). Global.asm (linha 136) diz "1 = time, 2 = stock, 3 = both"; o asm do TimedStock diz que o valor de stock passou de 2 para 3. Remix usa 3.
- `Global.vs.teams` (0x800A4D0A): 1 quando Stamina Team, Time Team, Stock Team.
- `VsRemixMenu.vs_mode_flag` (`VsRemixMenu.asm:25`): modo de página 2 (`mode.KOTH` 3, `SMASHKETBALL` 4, `TUG_OF_WAR` 5). Stamina e Timed Stock não usam esse flag; são só valores de `game_mode`.
- `Stamina.VS_MODE` é só alias de `Global.vs.game_mode`.
- Global.match_info (0x800A50E8) é um ponteiro para a struct de stocks por porta (ver seção 3).
- `SinglePlayerModes.singleplayer_mode_flag == REMIX_1P_ID (4)` só aparece nos ramos de Giga Bowser (1P Remix).

Estado de consumidores externos ao arquivo, todos por `game_mode == 5`:
- `Poison.asm:132-151` (clamp de porcentagem e pula stock derrotado).
- `SpecialZoom.asm:141-143` (desliga o zoom especial no Stamina).
- `TwelveCharBattle.asm:5383-5385` (checa o modo).
- `Size.asm:136` chama `Stamina.stamina_armor_fix_`.
- `AirDodge.asm:54` comenta que o reset do air dodge vem de `Stamina.asm`.

---

## 2. Fluxo

Menu VS (página 1) -> botão Rule (índices novos 2 = Stamina FFA, 5 = Stamina Team) -> CSS (HP com scroller) -> seleção de stage -> partida -> resultados.

Ganchos por tela, com o nome da função vanilla por correspondência (sem `no match found` quando há nome):

| # | asm (linha) | RAM | Tela | O que faz | Função vanilla (decomp) |
|---|---|---|---|---|---|
| 1 | Stamina.asm:20 `stamina_display_` | 0x8010E9FC | CSS/Versus HUD (tela 0x16) | Mostra HP em vez de porcentagem | `ifCommonPlayerDamageUpdateDigits` (if/ifcommon.c:594) |
| 2 | :66 `stamina_dark_` | 0x8010EC28 | HUD | Escurece o HP conforme o restante | mesmo ponto acima (`+0x334`) |
| 3 | :142 `stamina_dark_fix_2` | 0x8010E95C | HUD | Ajusta o escurecimento para HP > 300 | mesmo ponto |
| 4 | :186 `stamina_knockback_` | 0x800E3F64 | Partida, dano | Knockback com 20% fixo | `ftMainProcessHitCollisionStatsMain` (ft/ftmain.c:2801) |
| 5 | :215 `stamina_knockback_item_` | 0x800E4224 | Partida, item | Idem para item | mesmo ponto |
| 6 | :273 `stamina_knockback_throw_` | 0x800E9E70 | Partida, arremesso/tornado | Idem para arremesso | `ftParamGetCommonKnockback` (ft/ftparam.c) |
| 7 | :302 `stamina_knockback_projectile_` | 0x800E4100 | Partida, projétil | Idem para projétil | `ftMainProcessHitCollisionStatsMain` |
| 8 | :332 `stamina_ko_` (chamada, não hook) | - | Partida | Toca som de KO, acende flicker vermelho, chama 0x8013C120 | `ftCommonDeadUpdateScore` (ft/ftcommon/ftcommondead.c:53) |
| 9 | :382 `stamina_player_control_` | 0x80113A60 | Partida | Tira controle do jogador e troca FGM de "player defeated" | `ifCommonBattleUpdateScoreStocks` (if/ifcommon.c:2956) |
| 10 | :433 `stamina_main_` | 0x800EA270 | Partida, dano | Decide KO quando `TOTAL_HP < novo percent` | `ftParamUpdateDamage` (ft/ftparam.c:1814) |
| 11 | :514 `stamina_ko_action` | 0x800E6F3C | Partida, troca de action | Força ação de KO (DownBounceD ou Tumble) fora de ações protegidas | `ftMainSetStatus` (ft/ftmain.c) |
| 12 | :686 `stamina_shield_prevent` | 0x80148CE0 | Partida, escudo | Impede escudo após KO | `ftCommonGuardOnCheckInterruptSuccess` (ft/ftcommon/ftcommonguard1.c:481) |
| 13 | :746 `stamina_ko_prevent` | 0x8013C168 | Partida, morte | Impede morte de novo com stock -1 | `ftCommonDeadInitStatusVars` (ftcommondead.c:223) |
| 14 | :786 `stamina_ko_prevent_front` | 0x8013C9B8 | Partida, blast superior | Idem | `ftCommonDeadUpFallProcUpdate` (ftcommondead.c:468) |
| 15 | :824 `stamina_stock` | 0x800D8034 | Partida, início | Stock = 1 | `ftManagerMakeFighter` (ft/ftmanager.c) |
| 16 | :852 `stamina_stock_2` | 0x800D8014 | Partida, início | Idem (segundo ponto) | `ftManagerMakeFighter` |
| 17 | :888 `stamina_fix_1` | 0x801651A0 | Fim de partida | Trata modo 5 como 3 (tempo/stock) | `ftPublicProcUpdate` (ft/ftpublic.c:307), incerto |
| 18 | :908 `stamina_fix_2` | 0x8013BEB8 | Morte | Trata 5 como stock/tempo, chama `PokemonAnnouncer.pokemon_ko_sounds_` | `ftCommonDeadUpdateScore` (+0x154) |
| 19 | :956 `stamina_fix_3` | 0x80113170 | HUD, timer | Trata 5 como 3 | `ifCommonTimerFuncRun` (if/ifcommon.c:2737) |
| 20 | :976 `stamina_fix_4` | 0x8013BD08 | Morte, rumble | Trata 5 como 2 | `ftCommonDeadUpdateRumble` (ftcommondead.c:26) |
| 21 | :996 `stamina_fix_5` | 0x8013BF0C | Morte, score | Trata 5 como 2 | `ftCommonDeadUpdateScore` (+0x1A8) |
| 22 | :1016 `stamina_fix_6` | 0x8013BFA4 | Morte, respawn | Impede respawn infinito | `ftCommonDeadCheckRebirth` (ftcommondead.c:137) |
| 23 | :1036 `stamina_fix_7` | 0x8013BFB8 | Morte, star KO | Estoca stock em -1 quando star KO no último stock | `ftCommonDeadCheckRebirth` (+0x24) |
| 24 | :1068 `stamina_down_wait_down` | 0x8014423C | Down | Remove down wait quando sem HP | `ftCommonDownWaitProcUpdate` (ftcommondownwaitbounce.c:21) |
| 25 | :1104 `stamina_down_wait_input` | 0x8014425C | Down | Remove entrada de down wait | `ftCommonDownWaitProcInterrupt` (ftcommondownwaitbounce.c:34) |
| 26 | :1140 `stamina_down_bounce_input` | 0x80144360 | Down bounce | Idem para bounce | `ftCommonDownBounceProcUpdate` (ftcommondownwaitbounce.c:64) |
| 27 | :1176 `stamina_ledge` | 0x80144C34 | Ledge | Proíbe agarrar a beirada sem HP | `ftCommonCliffCatchSetStatus` (ftcommoncliffcatchwait.c:38) |
| 28 | :1221 `stamina_percent_prevent` | 0x800EA2B4 | Dano | Grava dano só até o HP; no último golpe completa o dano | `ftParamUpdateDamage` (+0x6C) |
| 29 | :1286 `percent_port_refresh` | 0x8018D48C | Início de partida | Zera `percent_port[]` | sem nome de decomp claro. **no match found** (ver risco) |
| 30 | :1306 `stamina_invis_prevent` | 0x8013C150 | Morte | Evita invisibilidade em certas actions | `ftCommonDeadInitStatusVars` (+0x30) |
| 31 | :1364 `action_hit_crash_prevent` | 0x800E825C | Hit | Evita crash ao ser atingido em actions específicas | `ftParamStopVoiceRunProcDamage` (ft/ftparam.c) |
| 32 | :1429 `egg_crash_prevent` | 0x8013C134 | Morte, egg lay | Evita crash na morte durante EggLay | `ftCommonDeadInitStatusVars` (+0x14) |
| 33 | :1474 `stamina_armor_fix_` (via Size.asm) | - | Armadura | Zera armadura quando HP acabou | sem nome de decomp. **no match found** |
| 34 | :1515 `overkill_prevent` | 0x80113A24 | Fim de partida | Impede KO duplo; ajusta contagem de stock/time | `ifCommonBattleUpdateScoreStocks` (+0x198) |
| 35 | :1564 `vs_menu` (rule, 4 hooks) | 0x8013420C, 0x801326D0 | Menu VS | Rule passa de 4 para 6 opções (índice máximo 5) | `mnVSModeFuncRun`/menu VS em decomp/src/mn/mnvsmode/mnvsmode.c (nome exato não verificado) |
| 36 | :1576 `add_to_rule_button_` (14 hooks) | 0x801322BC..0x80133AA8 | Menu VS | Novos índices 2 (Stamina), 3 (Time Team), 4 (Stock Team), 5 (Stamina Team); imagem da Stamina; posição; set_mode | mnvsmode.c; nomes exatos não verificados |
| 37 | :1698 `add_to_stock_time_button_` | 0x80132D94..0x80132E14 | Menu VS | Troca "MIN" por "H.P." quando Stamina | mnvsmode.c, incerto |
| 38 | :1778 `add_to_stock_time_value_` | 0x80132B8C..0x124D98 | Menu VS | Valor do HP (0..999), posição de 3 dígitos, scroll | mnvsmode.c, incerto |
| 39 | :1968 (init Rule) | 0x801336E8 | Menu VS, entrada | Índice inicial do botão Rule a partir de `game_mode` e flag de time | mnvsmode.c, incerto |
| 40 | :1984 `vs_css` (3 dígitos) | 0x80134168 | CSS | Scroller com 3 dígitos | mnvsmode/CSS, incerto |
| 41 | :1990 `stamina_scrolling_` | 0x8013838C, 0x80138410 | CSS | Scroller do HP, faixa 1..999 | CSS VS (decomp/src/sc/ ou mn/mnvsmode, nome não verificado) |
| 42 | :2064 `stamina_hp_display_` | 0x801341C4, 0x80134264 | CSS | Imagem "H.P." e valor do HP | CSS VS, incerto |

Observações do fluxo:
- O HP é escolhido no CSS (scroller 41 e 42) e guardado em `TOTAL_HP` (seção 3). A stage é vanilla; não achei gancho de Stamina em stage select.
- Os hooks 29 e 33 não têm nome claro no decomp. Não usar o endereço do comentário como prova.
- Resultados: a pontuação de Stamina passa pelo `score_fix_` do TimedStock (seção 4), cuja correspondência é `mnVSResultsGetPointsDirect` (decomp/src/mn/mnvsmode/mnvsresults.c:2680). O corpo bate: `KOs - TKO`.

---

## 3. Estado e dados

Globais e constantes:
- `Stamina.TOTAL_HP` (dw, valor 0xC7 = 199). É o HP inicial menos 1. Lido/escrito em: exibição (1, 2), KO (`stamina_main_`, `stamina_ko_action`, down/ledge/stamina_fix), CSS (scroller, valor padrão), Poison (`Poison.asm:151`). Escrito pelo scroller do CSS (`stamina_scrolling_`, `_set`). Endereço RAM não está no fonte (label de assembler). **incerto**.
- `Stamina.STAMINA_MODE` = 0x05. `Stamina.VS_MODE` = 0x800A4D0B.
- `percent_port[4]` (dw, 4 portas): flag de "já morto" usada por `stamina_percent_prevent`, `stamina_ko_prevent`, `action_hit_crash_prevent` e `overkill_prevent`. Zerado em `percent_port_refresh`. É estado estático global: a porta morta fica marcada até o próximo `percent_port_refresh`.
- `VsRemixMenu.global_game_mode`, `global_time`, `global_stocks`, `global_teams`, `global_stockmode_table`, `global_stock_count_table`, `global_previous_stock_count_table` (VsRemixMenu.asm:28-60). Salvos antes do CSS, restaurados depois.
- `TugOfWar.teams` (TugOfWar.asm:18) e `KingOfTheHill.teams` são guardados do mesmo jeito (VsRemixMenu.asm:442-449).
- `Smashketball.scoreboard` e `Smashketball.winner_scores`, `KingOfTheHill.scores`: usados pelo `score_fix_`.
- `StockMode.initial_stock_count_table` (StockMode.asm:29): stocks iniciais por porta.
- `SinglePlayerModes.singleplayer_mode_flag` e `REMIX_1P_ID` (= 4): só para Giga Bowser.
- `SinglePlayerModes` difficulty em 0x800A493A (lido pelo ramo de Giga Bowser).

Memória de partida (asm, sem nomes no fonte; use o endereço):
- Struct de stocks por porta: ponteiro em 0x800A50E8 (`Global.match_info`). Por porta, stride 0x74 (cálculo `port*116`). Byte de stock em +0x2B; -1 (0xFF) = derrotado.
- Lutador (struct do player, offsets usados): +0x08 id de personagem; +0x0D porta; +0x14 stock (usado por `stamina_fix_7`); +0x23 tipo de jogador (0 = CPU?); +0x24 action; +0x2C porcentagem; +0x6C dano feito (usado por `stamina_percent_prevent`); +0x7E4 armadura passiva; +0x7E8 armadura de yoshi; +0x7F0 dano recebido neste frame; +0x14C estado cinético (grounded/air); +0x18D flag de invisibilidade; +0xB18 word de action (limpo em `action_hit_crash_prevent`); +0x0B18 word zerado em `egg_crash_prevent`. **incerto** quanto ao sentido de +0x14 e +0x23, e se +0xB18 e +0x0B18 são o mesmo campo.
- KOs e mortes da porta: RAM 0x80149B80 (KOs) e 0x80149B90 (mortes), via `lui 0x8014` + offset 0x9B80/0x9B90 (`TimedStock.asm:110, 128`).
- Pontuação por porta: `Global.vs.p1` + `Global.vs.P_DIFF` * porta, dano final em +0x4C (`TimedStock.asm:88-93`).

Constantes:
- Ações protegidas (`stamina_ko_action`, linhas 541-678): intervalos de Action.DeadS..., damage actions 0x25..0x39, Tornado, barrel, DownBounceD..., CliffCatch..CliffEscapeSlow2, Grab, capturas até 0xBD, DK Cargo..HeavyItemThrowF, Marina Cargo até 0xFC. Estes números são do asm; conferir antes de portar.
- Giga Bowser HP (1P Remix): difficulty < 2 => 200; < 4 => 250; senão 300. Comentário do asm diz "medium", mas o limite é 2.
- Knockback fixo: `addiu ... 0x14` = 20%.
- HP de exibição: 200 (TOTAL_HP+1). Clamps do scroller: mínimo 1, máximo 0x3E6 (998 guardado, 999 exibido). Nota: `_scroll_left` e `_scroll_right` usam 0x3E6 como teto, e o comentário em `_stamina` diz 1000. **incerto** qual valor vale.
- Passos do scroller: 1 (abaixo de 10), 10 (entre 10 e 990), e de 990 para cima, 1.

Escritores principais: `stamina_main_` (dano), `stamina_ko_` (KO), `stamina_stock*` (stock), `stamina_scrolling_` e menu VS (HP e regra), `percent_port_refresh` (flags).

---

## 4. Lógica durante a partida (ordem)

1. Início (`stamina_stock`, `stamina_stock_2`): stock = 1 para todos.
2. Dano (`ftParamUpdateDamage` -> `stamina_main_`): o novo percent é calculado. Se `game_mode == 5` e tela 0x16: lê stock. Se stock == -1 (já derrotado), sai. Se `TOTAL_HP < novo percent`, chama `stamina_ko_`.
3. `stamina_percent_prevent` (chamado pelo mesmo caminho de dano, gancho 28): grava dano só até o limite de HP. Se o jogador já está em `percent_port` = 1, não grava. Caso contrário, marca a porta como morta e completa o dano até o limite (`TOTAL_HP - dano_atual + 1`).
4. `stamina_ko_`: toca som de KO, flicker vermelho (0x66), chama 0x8013C120 (estado de KO). Não zera stock: quem zera é `ftCommonDeadUpdateScore` (vanilla) quando a action de KO termina.
5. `stamina_ko_action`: força a action de KO (DownBounceD se no chão, Tumble se no ar) quando o HP acabou, exceto ações protegidas listadas na seção 3. Para Giga Bowser (1P Remix), usa o limite pela dificuldade.
6. `stamina_player_control_`: tira o controle do jogador após o FGM de derrota.
7. Morte e reaparecimento: `stamina_fix_6` e `stamina_fix_7` impedem respawn infinito e KO duplo do último stock. `stamina_ko_prevent` e `stamina_ko_prevent_front` impedem nova morte com stock -1. `overkill_prevent` impede KO duplo em blast zone (usa `percent_port`).
8. Fim de partida (`ifCommonBattleUpdateScoreStocks`/ `overkill_prevent`): quando sobra um jogador, a partida termina pelo caminho de stock (tratado como modo 3 pelos fixes 17-23).
9. Knockback: enquanto `game_mode == 5`, todos os cálculos de knockback usam 20% (hooks 4-7).
10. Escudo, ledge, down e egg: o HP zerado bloqueia essas ações (hooks 12, 24-27, 32).

Timed Stock em partida (`TimedStock.asm`):
- `pick_scoring_` (0x801373C4): modo 1 => tempo; se timer == 0 => tempo; senão stock. Cada escolha chama `calculate_time_score_` (0x801373F4) ou `calculate_stock_score_` (0x801373CC). **Nenhum dos dois tem correspondência de nome no decomp** (ver seção 2 e risco 4).
- `score_fix_` (0x80136C08): chamado quando o timer chega a 0.
  - Tempo: KOs menos mortes (ou KOTH: `KingOfTheHill.scores`; Smashketball: `Smashketball.winner_scores`).
  - Stock: `((stock inicial - mortes + 1) << 12) - dano_final`, se sobra stock. Sem stock, só `(... ) << 12` sem dano (`beqz v0, _end`).
  - Tug of War: `(stocks_restantes << 24) + ((KOs - mortes) << 12)` + dano.
- `prevent_sudden_death_` (0x8018D604 em RAM): em tempo, se empatar, entra em morte súbita. Em KOTH, nunca. Em Smashketball, morte súbita só se o placar estiver empatado (seta o tempo em 0x64 = 100 e a flag de morte súbita; endereço exato da flag não conferido). Em outros modos, segue o caminho normal (`addiu a0,a0,0x4EF8` original).
- `show_full_results_` (0x801376E4): mostra sempre o resultado completo (KOs e TKOs) se `Toggles.entry_full_results` estiver ligado.
- `ifCommonTimerFuncRun` (fix 19 em Stamina e hook 0x1389F8 do TimedStock): mostra o contador de stock só quando o modo é tempo, não em stock.

---

## 5. HUD e texto

- HP no HUD (`stamina_display_`, `stamina_dark_`, `stamina_dark_fix_2`): número do HP no lugar do percentual, com escurecimento pelo ratio HP/total. Não usa fonte, usa o digit sprite do HUD vanilla (`ifCommonPlayerDamageUpdateDigits`). Portável com sprite vanilla; não precisa de texto novo.
- Menu VS: texto "H.P." e imagem "Stamina" (arquivo 0x006, offset 0x6D28 / 0x6B28 / 0x6E28). Vêm de arquivos de sprite do Remix. Sem fonte nova, mas precisa do arquivo 0x006 do Remix, que o port deve ler por símbolo (`port/remix/remix_symbols.h`) ou pelo RAM image.
- Valor do HP no CSS: 3 dígitos (`vs_css` 1984) e posição X diferente por número de dígitos (0xCD, 0xD1). Sprite vanilla.
- Não encontrei texto escrito em C no modo. A pop-up "KO" usa FGM 0xA4 para o scroll e o FGM de derrota vanilla.
- A port desenha texto só por overlay ImGui (`port/remix/remix_overlay.h`) e sprites vanilla. Esta mecânica não precisa de overlay.

---

## 6. Dependências

- **StockMode.asm**: `initial_stock_count_table` (usado no score de stock e em `stamina_fix_*`). Stamina só troca o valor de stock para 1 no início. Se o stock table for mudado por outro modo, o cálculo de score pode divergir.
- **TimedStock.asm**: `score_fix_`, `pick_scoring_`, `prevent_sudden_death_`, `show_full_results_`. Usado por Stamina (modo 5 é tratado como 3 pelos fixes), por KOTH, Smashketball, Tug of War. Qualquer mudança em `score_fix_` afeta esses quatro modos.
- **Poison.asm**: clamp de dano no HP de Stamina e pula stock derrotado (`Poison.asm:132-160`). Em `remix` já há `REMIX-TODO(Stamina)` em `decomp/src/ft/ftremix/ftremixpoison.c:148` e `:250`.
- **SpecialZoom.asm**: skip de zoom especial em Stamina (`gmremixspecialzoom.c:263, 275` tem `REMIX-TODO(Stamina)`).
- **Size.asm**: `stamina_armor_fix_` (`ftremixsizebody.c:63` tem a nota de que não há port).
- **TwelveCharBattle.asm**: checa `game_mode == 5` no ponto 5383.
- **Neutral Spawns (Spawn.asm)**: `load_spawn_` (0x800FAF64, RAM) faz spawn neutro para modo de times e 1v1 com `valid_teams` e `valid_singles`. Sem checagem de Stamina. Stamina Team usa esse caminho.
- **Results screen**: `mnVSResultsGetPointsDirect`, `mnVSResultsOrderResults` (decomp/src/mn/mnvsmode/mnvsresults.c:2680, 2692). Ordena por score e, em morte súbita, por lugar.
- **AirDodge.asm:54**: reset da flag de air dodge no hit (`action_hit_crash_prevent`).
- **PokemonAnnouncer.asm**: `stamina_fix_2` chama `pokemon_ko_sounds_`.
- **Remix port atual**: sem port de Stamina. `REMIX-TODO(Stamina)` em 3 arquivos (acima).

---

## 7. Plano de port

Cada passo é uma sessão de um worker. Ordem pensada para que o lead veja algo na tela cedo.

**Teams (regra de times) entram antes de Stamina Team**, por dependência. Teams só muda o que já existe no decomp: `Global.vs.teams` e `Spawn`.

1. **P1. Constantes e estado de Stamina (sem efeito)**. Criar `decomp/src/gm/gmremixstamina.h` e `.c` com `TOTAL_HP` (199), `REMIX_STAMINA_MODE` (5), e funções `remixStaminaIsActive()`, `remixStaminaGetTotalHP()`, `remixStaminaIsDefeated(player)`. Teste de unidade com fakes. Aceite: build sem mudança visível.
2. **P2. Modo 3 para Timed Stock (FFA e Teams)**. Em `decomp/src/mn/mnvsmode/mnvsmode.c` (ou `mnremixvsmode.c`) setar `game_mode = 3` quando a regra é Timed Stock. Depende do menu VS já portado (T5.3, feito). Aceite: no VS menu, Time/Stock rule "Stock+Time" grava 3.
3. **P3. Score de Timed Stock (`score_fix_`)**. Novo arquivo `decomp/src/sc/sccommon/scremixtimedstock.c` com a fórmula de score (KOs-mortes, stock + dano, Tug of War). Ligar em `mnVSResultsGetPointsDirect` com `#ifdef REGION_REMIX`. Teste de unidade para cada modo. Depende de P1 para o modo 5, pode ser feito antes com modo 3 só. Aceite: partida de tempo termina com vencedor pelo score.
4. **P4. Sudden death em Timed Stock**. Editar `scVSBattleSetScoreCheckSuddenDeath` (decomp/src/sc/sccommon/scvsbattle.c:394) com guarda remix para KOTH e Smashketball. Depende de P3. Aceite: empate em tempo abre morte súbita; KOTH não.
5. **P5. Stock = 1 em Stamina**. Em `ftManagerMakeFighter` (decomp/src/ft/ftmanager.c:821), setar stock 1 quando `game_mode == 5`. Depende de P1. Aceite: partida Stamina começa com 1 stock por jogador (contar pela UI).
6. **P6. Knockback 20% em Stamina**. Em `ftMainProcessHitCollisionStatsMain` (ft/ftmain.c:2801) e `ftParamGetCommonKnockback` (ft/ftparam.c), trocar a porcentagem usada por 20 quando Stamina. Depende de P1. Aceite: knockback igual ao de 20% em qualquer dano.
7. **P7. Dano e KO em Stamina (`stamina_main_`, `stamina_ko_`)**. Em `ftParamUpdateDamage` (ft/ftparam.c:1814), chamar `remixStaminaTryKO` se `TOTAL_HP < percent`. Usa `percent_port`. Depende de P1, P5. Aceite: fighter cai ao passar do HP e stock vira -1. **Risco alto**: ponto de maior incerteza (seção 8, risco 2).
8. **P8. Ações de KO forçadas e protegidas (`stamina_ko_action`)**. Em `ftMainSetStatus` (ft/ftmain.c), lista de ações protegidas e DownBounceD/Tumble. Depende de P7. Aceite: lutador em HP zerado cai em DownBounce no chão e Tumble no ar; ações protegidas terminam.
9. **P9. Respawn e morte única (`stamina_fix_6`, `stamina_fix_7`, `stamina_ko_prevent`)**. Em `ftCommonDeadCheckRebirth` (ft/ftcommon/ftcommondead.c:137) e `ftCommonDeadInitStatusVars` (ftcommondead.c:223). Depende de P7. Aceite: lutador sem HP não reaparece; star KO no último stock não repete morte.
10. **P10. Fim de partida em Stamina (`overkill_prevent`, `stamina_fix_1..5`)**. Em `ifCommonBattleUpdateScoreStocks` (if/ifcommon.c:2956) e `ftCommonDeadUpdateScore` (ft/ftcommon/ftcommondead.c:53). Depende de P7, P9. Aceite: último jogador vivo ganha; KO duplo não conta.
11. **P11. Escudo, ledge, down, egg (hooks 12, 24-27, 30-32)**. Um arquivo por grupo: `gmremixstaminaguard.c` (escudo, armadura), `gmremixstaminadown.c` (down wait, down bounce), `gmremixstaminaledge.c` (ledge). Depende de P7. Aceite: sem ação de escudo, down ou ledge com HP zerado.
12. **P12. Poison e SpecialZoom (remix TODO)**. Trocar os 3 `REMIX-TODO(Stamina)` em `ftremixpoison.c`, `gmremixspecialzoom.c`. Depende de P1. Aceite: dano de veneno clampado em HP em Stamina.
13. **P13. HUD: número do HP e escurecimento (`stamina_display_`, `stamina_dark_*`)**. Em `ifCommonPlayerDamageUpdateDigits` (if/ifcommon.c:594), mostrar `TOTAL_HP + 1 - dano` quando Stamina. Depende de P1. Aceite: HUD mostra 200 no início e cai conforme o dano; tela de captura.
14. **P14. Menu VS: opção Stamina no botão Rule e "H.P." (`vs_menu`, `add_to_rule_button_`, `add_to_stock_time_button_`)**. Em `mnvsmode.c` (botão Rule) e `mnremixvsmode.c`. Depende de P2. Aceite: seis opções no botão Rule; "H.P." quando Stamina. **Imagem 0x006 precisa de caminho de textura** (ver seção 5).
15. **P15. CSS: scroller de HP (`stamina_scrolling_`, `stamina_hp_display_`)**. No CSS VS (arquivo a confirmar). Depende de P1, P14. Aceite: scroller vai de 1 a 999 e o valor muda na tela. Lembrar o clamp (risco 5).
16. **P16. Teams em Stamina e Timed Stock**. Ativar `Spawn.load_spawn_` para Stamina Team (já é neutro por `Global.vs.teams`), e conferir se o caminho de times de KOTH e Tug of War aceita o time de Stamina. Depende de P5, P7. Aceite: Stamina Team tem dois times e spawn neutro.
17. **P17. Giga Bowser HP (1P Remix)**. Em `ftCommonDeadUpdateScore` ou em `stamina_ko_action` porta, limite 200/250/300 pela dificuldade. Depende de P8. Aceite: GBowser cai em 200, 250 ou 300 conforme dificuldade. Marcar como "não tocar até T6.2", pois é 1P.

Total: 17 passos.

---

## 8. Riscos e incertezas

1. **Mapeamento vanilla por nome**. A maioria dos ganchos foi mapeada pelo nome e pelo papel na função, não pelo disassembly. Os endereços dos comentários do asm e a coluna "decomp" de `gameplay_m_z.md` não foram usados como prova. Para cada passo, o worker deve conferir o endereço RAM com o disassembly antes de editar.
2. **Ponto de KO**: `stamina_main_` é chamado no caminho de dano e pode rodar antes ou depois do `ftCommonDeadUpdateScore`. A ordem entre "marcar morte" e "tirar stock" é a parte mais frágil. Não confirmado em jogo.
3. **Estado estático `percent_port[]`**: é global, sem reset entre partidas exceto por `percent_port_refresh` (hook 29, sem nome no decomp). Partida nova sem esse reset herda "porta morta". Risco de bug real, não verificado.
4. **Sem nome de decomp para `calculate_time_score_` (0x801373F4) e `calculate_stock_score_` (0x801373CC)**. Não achei a correspondência; o port deve criar função própria, com teste.
5. **Clamp do scroller de HP**: 0x3E6 (998) guardado contra 1000 no comentário. Não sei qual vale. Confirmar em jogo com captura do CSS.
6. **Comentário de Giga Bowser**: diz "medium", mas o código usa 2 como limite. Seguir o código e não o comentário.
7. **Offsets do lutador**: +0x14 (stock usado em `stamina_fix_7`) e +0x23 (tipo de jogador) não estão claros no asm. Usar `Global.match_info` para stock, não o campo do lutador, até confirmar.
8. **Endereço de `TOTAL_HP`**: é label do assembler, sem RAM no fonte. Port deve ler o endereço pelo símbolo do RAM image (`port/remix/remix_symbols.h`) ou criar variável própria em C. **Não** deixar dois valores separados (CSS e partida).
9. **`stamina_fix_1`** tem `andi` sobre `t7` com `t7` vindo de `v0` sem checagem clara. Não sei se está correto. Confirmar em jogo com partida de Stamina até o fim.
10. **Texto e sprite**: "H.P." e a imagem 0x006 vêm de arquivos do Remix. O port precisa de caminho de textura (ver `f10_menu_vsmodes.md`, risco do Sprite vs texture).
11. **Teams**: em Stamina Team o cálculo de morte conta "stock/team count" (`overkill_prevent`). Se o port tratar time por time de outro jeito, o fim de partida diverge.
12. **Não rodei nada**. Nenhum número acima foi confirmado em build ou em jogo.

---

## Teams: o que muda a regra vanilla (escopo pedido)

Não é o tema de cores de `Teams.asm` (esse é aparência). A regra de times muda em:
- `Spawn.asm` (`load_spawn_`, `valid_teams`, `valid_singles`): spawn neutro quando há dois times.
- `TugOfWar.asm` (`teams`, linhas 17-191, 506): Tug of War com equipes.
- `KingOfTheHill.asm` (`teams`) e `VsRemixMenu.asm` (`global_teams`, 404-451, 567-568): salvar e restaurar a flag de times.
- `Smashketball.asm` (155-159, 399-445, 1348-1462): exige exatamente 2 times para começar.
- `Stamina.asm` (índices 3, 4, 5 do botão Rule; `overkill_prevent`).
- `TimedStock.asm` (`0x001241A0`, `0x001241B4`): a entrada de Team passa a usar modo 3.
- Cosméticos que leem `Global.vs.teams` e não mudam regra: `ComboMeter.asm:249`, `DragonKingHUD.asm:228`, `CharacterSelect.asm:6754`, `Costumes.asm:783-786, 1074`. Não entram no plano de regra.

---

## Resumo para o lead

- Arquivo: `docs/remix/notes/f11_mode_stamina.md`.
- Plano: 17 passos (P1 a P17), incluindo Teams e Timed Stock.
- Maior risco: ordem entre marcação de KO (`stamina_main_`) e perda de stock (`ftCommonDeadUpdateScore`), e o estado estático `percent_port[]`.
