# F11 modo: 12-Character Battle (twelvecb)

Nota de pesquisa para o M6 (T6.1). Fonte lida: `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm` (5454 linhas, lido por completo nas
partes de lógica), e os pontos de contato em `VsRemixMenu.asm`, `CharacterSelect.asm`, `Render.asm`, `GameEnd.asm`, `Pause.asm`,
`StockMode.asm`, `DKMode.asm`, `CharacterSelectDebugMenu.asm`, `Global.asm`, `File.asm`. Lado do port lido: `decomp/src/gm/gmremixgameend.c`,
`decomp/src/sc/sccommon/scvsbattle.c`, `decomp/src/mn/mnvsmode/mnremixvsmode.{c,h}`. Nada foi compilado nem rodado.

Convenção das linhas: `arquivo.asm:linha`, sempre no Remix. Endereços RAM são os do segundo argumento de `OS.patch_start(rom, ram)`.

## 1. O que é o modo

**Regras:** cada jogador tem um pool de 48 stocks (12 personagens x 4 stocks, `config.pX.stocks_remaining`, 0x30). Cada personagem
(portrait) começa com 4 stocks; a partida é Stock sem tempo, FFA, 1 vs 1 (portas 1 e 2 apenas). Cada KO tira um stock do pool do jogador
e do portrait que o perdeu; quando o portrait chega a 0 ele fica derrotado e não pode ser escolhido de novo. A partida acaba quando o pool
de um jogador chega a 0 (`status = COMPLETE`, `update_stocks_remaining_`, `TwelveCharBattle.asm:5399-5424`).

**Pontuação:** por jogador, "melhor personagem" é o personagem com mais TKOs a favor (stocks tirados enquanto ele jogava); empate desempata
com menos TKOs contra (`set_best_characters_`, `TwelveCharBattle.asm:3866`). Mostrado no CSS (`Best Character`, `TKOs`).

**Flags e toggles que selecionam o modo:**
- `VsRemixMenu.vs_mode_flag` = `mode.TWELVE_CB` (1). Escrito pelo botão da página 2 (`VsRemixMenu.asm:80-100` tabela `remix_menu_button_table`,
  byte 0x06; escrita em `VsRemixMenu.asm:366`).
- `TwelveCharBattle.twelve_cb_flag` = 1. Escrito pelo botão (byte 0x07, `VsRemixMenu.asm:368`). Lido em ~80 pontos (CSS, Render, Pause,
  GameEnd, StockMode, CharacterSelectDebugMenu). Zerado no retorno ao modo de seleção (`Render.asm:2459-2461`, `Toggles.mode_select_setup_`).
- `VsRemixMenu.page_flag` (byte 0x06 da tabela): 1 na linha 12CB (`VsRemixMenu.asm:93`); as linhas de página 2 têm `page_flag` 1 só na escolha de "Remix Modes" (`:89`).
- `Global.vs.game_mode` = 2 (STOCK), `Global.vs.teams` = 0, `Global.vs.stocks` = `stocks` (padrão `dw 3`, 0-based, quer dizer 4 stocks). Escritos
  em `before_css_setup_` (`TwelveCharBattle.asm:366-391`). Depois `force_ffa_and_stock_` (`TwelveCharBattle.asm:1665`) confirma FFA e stock no CSS.
- `Global.current_screen` 0x10 (VS CSS) e 0x16 (VS battle) e 0x18 (results) são lidos como condição em vários pontos.
- `SinglePlayerModes.singleplayer_mode_flag`: a 12CB não usa; `defeated_action_override_` só checa o valor 4 (Remix 1P).
- `Global.match_info`: só usado pelo caminho de salty runback (`GameEnd.asm:~100`), que a 12CB desliga.

## 2. Fluxo

Ordem real. Os screen ids estão em `Global.asm:58-122` (0x09 VS menu, 0x10 VS CSS, 0x15 STAGE_SELECT "applies to all modes", 0x16 VS_BATTLE,
0x18 RESULTS).

| # | Tela (screen id) | Ponto no asm | Patched RAM | O que faz |
|---|---|---|---|---|
| 1 | VS Game Mode, página 2 (0x09) | `VsRemixMenu.asm:335` (A em botão genérico, hook 0x80133CD0) e `:366-368` | 0x80133CD0 | Grava `vs_mode_flag=1`, `page_flag=1`, `twelve_cb_flag=1`, depois muda para a tela 0x10. |
| 2 | Entrada no CSS (0x10) | `VsRemixMenu.asm:477` (hook 0x8013AF4C) chama `mode_setup_table[1]` = `TwelveCharBattle.before_css_setup_` (`:366`) | 0x8013AF4C | Seta stock, FFA e time (ver seção 1). |
| 3 | CSS (0x10) | `TwelveCharBattle.asm:1665` `force_ffa_and_stock_` (hook 0x8013AF20); `Render.asm:2329-2333` chama `TwelveCharBattle.setup_` no CSS | 0x8013AF20 | Aponta ponteiros do CSS para as tabelas 12CB, reaplica stock, restaura portrait_id das structs de CSS. |
| 4 | CSS (0x10), por porta | `TwelveCharBattle.asm:453` `setup_ports_and_characters_` (hook 0x8013ACBC); `:593` `ignore_ports_3_and_4_` (0x8013A2E0) | | Escolha de personagem, validade, porta 3 e 4 ignoradas. |
| 5 | CSS, pausa | `TwelveCharBattle.asm:3523` `gentlemens_reset_` (hooks 0x801144D0, 0x80114490, 0x8011407C); `Pause.asm:256-260` | | Reset de jogo (ver seção 4, item 9). |
| 6 | Seleção de stage (0x15) | Sem hook 12CB. Fluxo VS vanilla. | | Não verificado neste arquivo: confirmar pelo `scene_curr` do CSS. |
| 7 | Início da partida, antes do início | `VsRemixMenu.asm:628` (hook 0x8013AA40) chama `TwelveCharBattle.start_match_setup_` (`:426`) | 0x8013AA40 | Guarda stocks do CSS e tabela StockMode. |
| 8 | Partida (0x16) | `Render.asm:2297` (`_vs`, chama `TwelveCharBattle.game_setup_`) | | `game_setup_` (`:5262`): marca `STATUS_STARTED`, avança `current_game`, grava a struct do jogo (char, stocks, portrait). |
| 9 | Partida (0x16), a cada KO | `TwelveCharBattle.asm:5350` (hook 0x8013BEF0) e `:5354` (0x8013BF48) | 0x8013BEF0, 0x8013BF48 | `update_stocks_remaining_` (`:5349`): só age com tela 0x16 e `twelve_cb_flag`. |
| 10 | Fim da partida (0x18 results) | `GameEnd.asm:26-115` (update_screen_, hook ROM 0x10B204, RAM 0x8018E314); `GameEnd.asm:81-84` | 0x8018E314 | Com 12CB: salty runback desligado (`:84`); se "skip results" ligado vai para o CSS (0x10). |
| 11 | Results (0x18) | `TwelveCharBattle.asm:1872` (hook 0x80136788) chama `update_css_header_._results` (`:1904`), com `_12cb_results` (`:1931`) | 0x80136788 | Cabeçalho "12-Char. Battle" e o texto "Game N". |
| 12 | Voltar ao VS menu (0x09) | `VsRemixMenu.asm:546` (hook 0x80138108) chama `TwelveCharBattle.leave_css_setup_` (`:394`) | 0x80138108 | Restaura stocks e tabela StockMode. |

Caminho do CSS para o modo: a 12CB começa no CSS (0x10) e volta ao CSS a cada jogo quando não termina (runback vai para 0x10 com
skip-results; com resultados vai para 0x18 e depois 0x10). Não há hook de stage select do modo.

## 3. Estado e dados

**Globais do modo** (`TwelveCharBattle.asm`, struct `config` em `:38-75`):

| Nome | Tamanho | Quem escreve | Observação |
|---|---|---|---|
| `twelve_cb_flag` | dw | `VsRemixMenu.asm:368`, `Render.asm`, `Toggles.mode_select_setup_` | 1 = 12CB ativa. |
| `stocks` | dw (3) | `before_css_setup_` (`:366`), `leave_css_setup_` (`:394`), `start_match_setup_` (`:426`) | Stock do CSS, 0-based. |
| `config.status` | dw | `handle_reset_` (`:2122`), `game_setup_` (`:5262`), `update_stocks_remaining_` (`:5349`), `gentlemens_reset_` | 0 NOT_STARTED, 1 STARTED, 2 COMPLETE. |
| `config.num_stocks` | dw (3) | `update_stock_fields_` (`:1567`) | 0-based. |
| `config.current_game` | dw (-1) | `game_setup_`, `handle_reset_`, `gentlemens_reset_` | Índice da partida (0-based, até 23). |
| `config.pX.stocks_remaining` | dw (0x30) | `update_stock_fields_` (`:1567`), `update_stocks_remaining_` (`:5399`), `gentlemens_reset_` | Pool do jogador (12 x (num+1)). |
| `config.pX.match[24]` | 8 bytes cada | `game_setup_`, `gentlemens_reset_`, `update_stocks_remaining_` | Por jogo: `+0` char, `+1` stocks iniciais, `+2` stocks finais (0xFF = sem stocks), `+3` portrait, `+4` was_random. |
| `config.pX.character_set` | dw | CSS | Índice do conjunto de portraits. |
| `config.pX.best_character*` | dw | `set_best_characters_` (`:3866`) | Melhor personagem, TKOs a favor e contra. |
| `config.stocks_by_portrait_id[24]` | 24 bytes (0x03) | `update_stock_fields_`, `update_stocks_remaining_` (`:5399`), `handle_reset_`, `gentlemens_reset_` | Stocks 0-based por portrait. **Uma tabela só para os dois jogadores.** |
| `reset_requested` | dw | `gentlemens_reset_` (`:3720`) | Pedido de reset pendente (0 nenhum, 1 aguardando aprovação). |
| `portrait_cycle_timer` | dh x2 | CSS | Temporizador de ciclo de portrait. |
| `copy_timer`, `copying_flag` | dh x2, dw | CSS | Cópia de conjunto de portraits. |
| `reset_button_pointer`, `previous_portrait_id_pointer` | dw | `setup_` (`:5048`) | Ponteiros de objetos de HUD. |

**Tabelas:**
- `layout.u`, `layout.j`, `layout.pv`, `layout.r` (`:81-196`): 24 slots cada. Cada personagem aparece 2 vezes (slots 1 e 5, etc.). Ex. u: LUIGI, MARIO, DONKEY, LINK, LUIGI, MARIO...
  Cada jogo tem 12 personagens distintos x 2 portraits = 24 portraits.
- `character_set_table` (`:324`): 6 conjuntos (u, j, pv, r, p1, p2). `NUM_PRESETS(4)`. `id_table_*`, `portrait_offset_table_*`, `portrait_id_table_*`
  são gerados por macro (`:259-322`).
- `portrait_x_position` (`:346`), `portrait_velocity` (`:231`).
- `defeated_array` (`:3198`): 0x20+ entradas de `add_defeat_parameters` (anim, moveset, flags) para o personagem derrotado no CSS.
- `string_*` (`:5252-5290`): textos do CSS e do reset.

**Arquivos (File ids) carregados pelo modo:**
- `File.CHARACTER_PORTRAITS` = 0xA05 e `File.CSS_IMAGES` = 0xA06 (`File.asm:448-449`). Ids 0xA00+ são ids de extensão Remix; confirmar que
  não existem no ROM vanilla antes de afirmar que são só Remix.
- 0x24 ("x" image, `TwelveCharBattle.asm:5066`), 0xC5 (botões, `:5067`), 0x22 (coroa do vencedor, `:5142`), 0xC4 (ícone de start no reset, `:5258` em `game_setup_`, `:~5280`).
- Fonte: `Render.load_font()`.
- Offsets de imagens dentro de arquivos: `0x2048` (12CB), `0x2738` (Tag Team), `0x29A8` (KoTH), `0x30F0` (Tug of War), `0x2C18`/`0x2E88` (Smashketball 1/2)
  para o cabeçalho (`:1904-1930`, `:1932`); `0xECE8`, `0xEDC8`, `0x187A8`, `0xE2A0`, `0x1D50` para botões e coroa.
- Base `0x8013C4A0` para textura de arquivo 0x11 (`CharacterSelect.asm:2792` no VS).

**Strings** (`:5252-5290` e `string_*` no `setup_`): "Stocks Remaining", "Character Set", "Default" / "Japanese" / "Polygon" / "Remix" / "Custom",
"Best Character", "TKOs", "Game   " (com número em 5-6), "Game reset requested by  P." (dígito em 0x18), "If accepted, stock counts will be reset.",
"Accept:", "Decline:", ": Set All", ": Presets", ": Random", ": Copy".

## 4. Lógica dentro da partida

Ordem de execução (eventos, não quadros):

1. **Confirmação no CSS (por porta)**. `setup_ports_and_characters_` (`:453`): se o personagem é aleatório (`random_char_flag_vs`), usa placeholder; se
   o personagem não é válido na porta ou sem stocks, cai para `Character.id.NONE` (`:519-580`). Porta 3 e 4 são forçadas N/A (`:496`).
   Jogador vencedor da partida anterior fica preso no mesmo portrait (`prevent_defeated_char_select_`, `:2860`; `get_last_match_portrait_and_stocks_`, `:1312`).
2. **Início da partida** (`game_setup_`, `:5262`). Para cada porta: escreve `config.pX.match[current]` com char, stocks iniciais (`Global.vs.pX+0x0B`),
   stocks finais iguais, portrait, flag random. Se random, limpa o flag e recoloca o placeholder na tabela. `status = STARTED`, `current_game += 1`.
3. **Stocks iniciais do jogador na partida** (`set_initial_stock_count_`, `:3380`, hook 0x8018D4AC via `StockMode`): se é o primeiro jogo do
   portrait (`current_game < 0`), usa `stocks_by_portrait_id[portrait]`. Senão, se o stock final do jogador na partida anterior (0-based) for maior que 0, usa esse valor (ganhou com stocks sobrando). Caso contrário usa `stocks_by_portrait_id[portrait do jogo atual]`. Com 0 stock final, o valor do portrait já é 0 (gravado no KO), então o resultado é o mesmo do ramo anterior.
4. **Por KO** (`update_stocks_remaining_`, `:5349`). Só com tela 0x16.
   - `t5 = stocks_restantes_do_jogador - 1` (0-based; 0xFF quando zerou).
   - Decrementa `config.pX.stocks_remaining` (`:5399-5406`). Se zerou, `status = COMPLETE`.
   - Grava `ending = t5` no jogo atual e `stocks_by_portrait_id[portrait] = t5`.
   - Não toca em `StockMode.previous_stock_count_table` (o ramo de Stamina e StockMode vanilla só roda sem 12CB, `:5366-5388`).
5. **Fim de jogo** (vanilla Stock). Sem tempo no modo Stock (comentário de `force_ffa_and_stock_`, `:1661`). O hook de `check_match_end_`
   (VsRemixMenu, RAM 0x80113180) só troca o valor para KOTH e Smashketball; a 12CB cai no ramo normal (`VsRemixMenu.asm:721-752`).
6. **Results** (0x18). `GameEnd.update_screen_` decide: salty runback (desligado na 12CB, `GameEnd.asm:84`), skip-results (`Toggles.entry_skip_results_screen`, com
   ON RESET vai para o CSS se houve reset), ou resultados (0x18). Vencedor: a coroa (0x22, `:5142-5150`) fica no lado do jogador que ainda tem stocks.
7. **Volta ao CSS** (0x10) para o próximo jogo. `handle_reset_` (`:2122`) não roda aqui; só no reset.
8. **Reset (gentlemen's)**. Pausa. Um jogador pede (`reset_requested = 0` -> 1, mostra texto), o outro aprova. `_reset` (`:3640`)
   volta `current_game` e devolve ao pool `(inicial - final)` do jogo removido (`:3654-3690`). Para random, recoloca o placeholder.
   Mensagem de texto só com a flag; o botão de reset de pausa (`Pause.asm:256-260`) não desenha o aviso durante o pedido.
9. **Runback**. Não é salty runback (desligado). Volta ao CSS, `load_additional_characters_` (vanilla, `CharacterSelect.asm:1985-2020`) reseta
   `custom_heap_address` na entrada do CSS. Ver seção 8.
10. **Melhor personagem** (`set_best_characters_`, `:3866`). Chamado a cada entrada do CSS (`setup_`, `:5048`). Varre os jogos, soma TKOs por
    par (char+portrait). Um personagem é "novo" quando a chave (char<<16 | portrait) muda. Escolhe maior `for`, desempata com menor `against`.
11. **Spawn, câmera, itens, stage**. Não há hooks 12CB para isso. Usa vanilla VS e `gmRemixNeutralSpawnApply` (`scvsbattle.c:141`) que vale para todos os VS.

**Derrota no CSS** (`defeated_action_override_`, `:2935`): não é in-match. Quando o personagem não tem stocks, o menu usa `defeated_array` (animação
DownStandU com moveset de derrota, `:3195`) no CSS. Na tela 0x18 só o Sonic tem override (`:2955-3005`).

## 5. HUD e texto

CSS (todos feitos com `Render.draw_string` / `Render.draw_number`, texto vanilla de fonte; o port precisa de overlay ImGui):
- "Stocks Remaining" e os números p1 (azul, `0xB00000FF`) e p2 (`0x4040C0FF`), no grupo GROUP_ALWAYS (`:5050-5100`).
- "Character Set" e nome do conjunto (p1, p2), grupo GROUP_NOT_STARTED. Setas de conjunto com texturas (`0xECE8`, `0xEDC8`).
- "Best Character" + nome + TKOs (`for`), grupo GROUP_STARTED. Largura máxima 72 (`apply_max_width_`).
- Botão reset (textura `0x187A8`), grupo GROUP_STARTED.
- Coroa do vencedor (arquivo 0x22, `0xE2A0`), grupo GROUP_COMPLETED.
- Legendas D-pad: ": Presets", ": Random", ": Copy", ": Set All" (`string_dpad_*`, `string_update_all`).
- Ícones e contadores de stock nos portraits (CharacterSelect `update_stock_icon_and_count_indicators_`, `draw_disabled_rectangle_` para derrotados, `:2788`).
- Indicadores de variante (`update_variant_indicator`, `:2237`, `update_variant_indicators_`, `:3779`) e de portrait custom
  (`draw_custom_portrait_indicators_`, `:4615`, texto "explica como trocar retratos").
- Aviso de reset: "Game reset requested by P." + 2 linhas + "Accept:" / "Decline:" com texturas de botão (`:3540-3580`).

Resultados (0x18): "12-Char. Battle" no cabeçalho (`update_css_header_._vs` e `_results`, `:1877-1950`), e "Game N" (`:1932-1948`, `string_game`).

Texto: tudo acima passa por `Render.draw_string` com fonte vanilla. O port desenha texto só pelo overlay ImGui (`port/remix/remix_overlay.h`),
então cada string precisa de caminho no overlay. Os sprites de cabeçalho e botão são texturas de arquivo vanilla e podem usar o caminho de sprite do port.

## 6. Dependências

- **StockMode** (`StockMode.asm`): `set_initial_stock_count_` (`:3380`) é hook compartilhado; a 12CB tem ramo próprio. A opção "Last" do painel
  não existe no port (T5.2b, `decomp/src/gm/gmremixstockmode.c` REMIX-TODO). A 12CB ignora essa tabela, mas `leave_css_setup_` e `start_match_setup_`
  copiam a `stockmode_table` para `VsRemixMenu.global_stockmode_table` (`:394`, `:426`).
- **TimedStock**: não usado (Stock sem tempo).
- **Stamina** (`Stamina.VS_MODE`): o ramo de Stamina em `update_stocks_remaining_` roda só fora da 12CB (`:5366`).
- **Neutral Spawns**: `gmRemixNeutralSpawnApply` roda para todo VS (`scvsbattle.c:141`).
- **Results screen** (`mnvsresults.c`, `VsRemixMenu.asm:1007-1047` `add_row_to_results_`): linhas de modo nos resultados; a 12CB não tem hook de linha.
- **GameEnd** (`GameEnd.asm` e `gmremixgameend.c:107`): o port ainda permite salty runback na 12CB (REMIX-TODO documentado).
- **Pause**: `Pause.asm:256-260` (não desenha o legend durante o pedido de reset); `gentlemens_reset_` (`TwelveCharBattle.asm:3523`).
- **CharacterSelect** (vanilla): ~60 sites com `twelve_cb_flag` em `CharacterSelect.asm`, entre eles `draw_portraits_` (`:2433` chama `check_input_`),
  `refresh_stock_indicators_`, `update_stock_icon_and_count_indicators_`, `load_additional_characters_`. A 12CB não é um CSS separado no código, é o CSS
  VS com ramos.
- **DKMode** (`DKMode.asm:155-181`): easter egg que lê `TwelveCharBattle.id_table_p1/p2` e chama `update_portrait_id_table_`.
- **CharacterSelectDebugMenu** (`:383`, `:507`): ramos 12CB no menu de debug.
- **Heap**: `custom_heap_address` é resetado em `CharacterSelect.asm:2018-2020`, em `GameEnd.asm:144-146` (salty runback, não roda na 12CB),
  e em `Render.asm:2457-2459` (modo de seleção). `reset_heap_slot_` (`CharacterSelect.asm:4350`) limpa slots não usados.

## 7. Plano de port

Estado atual do port (lido): `mnremixvsmode.c` guarda `gMNVSModeRemixMode` = 1 quando o botão 12CB é escolhido (`f10_menu_vsmodes.md`),
mas não seta `twelve_cb_flag` nem muda o CSS. Não há símbolo `TwelveCharBattle` em `port/remix/`. `gmremixgameend.c:107` não lê a flag.
O CSS é o VS vanilla do port (`mnplayersvs.c`).

Cada passo é um arquivo ou um grupo de hooks. Passos marcados com (dep:) dependem de outro modo ou de milestone posterior.

1. **Estado 12CB** (novo `decomp/src/gm/gmremixtwelvecb.c` e `.h`). Struct `config` (status, num_stocks, current_game, pX match[23], stocks_remaining,
   stocks_by_portrait_id[24], melhores). Funções puras: reset, `update_stock_fields(stocks)`, `get_stocks_remaining(port, char)`, `record_ko(port, remaining)`.
   Testes headless em C (sem CSS). **Aceite do lead:** o teste passa; nada muda na tela.
2. **Melhor personagem** (`gmremixtwelvecb.c`, função `calculate_best_characters`). Mesma regra de `set_best_characters_`, com teste de fixtures
   (duas portas, troca de personagem, sem stocks). **Aceite:** teste passa.
3. **Flag e entrada** (`mnremixvsmode.c` e `mnvsmode.c`). Ao escolher 12CB, setar `gmRemixTwelveCBActive` (novo global) e `vs_mode_flag`-equivalente.
   Zerar no retorno ao menu. **Aceite do lead:** ao escolher 12CB o CSS abre com cabeçalho "12-Char. Battle" (sem layout novo ainda).
4. **Setup de stock e FFA ao entrar no CSS** (`before_css` e `force_ffa_and_stock`, em `mnplayersvs.c` e no ponto de entrada do CSS). Setar
   `game_mode = STOCK`, `teams = 0`, `stocks = 3`. **Aceite:** o CSS mostra "Stocks Remaining 48" para p1 e p2.
5. **Layout de 24 slots** (`gmremixtwelvecb_layout.c`, dados por conjunto `u`, `j`, `pv`, `r`, `p1`, `p2`). Teste: 24 slots, 12 personagens distintos
   x 2. **Aceite:** teste; no CSS, os 24 retratos aparecem no grid 8x3. (dep: T8.2 CSS VS, roster Remix em `remix_roster_data.h`)
6. **Portas 1 e 2 apenas** (`mnplayersvs.c`: esconder painéis 3 e 4, ignorar token nas portas 3 e 4). **Aceite:** só dois painéis.
7. **Escolha válida e derrota no CSS** (`is_character_valid_for_port`, `get_valid_portrait_for_char_and_port`, `prevent_defeated_char_select`,
   travar vencedor no mesmo portrait). Teste de regra + tela. **Aceite:** derrotado não é escolhível; vencedor fica no mesmo portrait.
8. **Alterar stocks no CSS** (`handle_stock_count_change`, faixa 0..99 antes do início, bloqueio com FGM ILLEGAL depois do início). Teste de limites.
   **Aceite:** sobe e desce 0..99 com som certo.
9. **Início da partida** (`game_setup` no caminho de `scVSBattleStartBattle`, `scvsbattle.c:269`, e `desc.stock_count` em `scvsbattle.c:354`).
   Para 12CB, `desc.stock_count = config.stocks_by_portrait_id[portrait] + 1` (ou o valor de `set_initial_stock_count`). **Aceite:** a segunda partida começa com os stocks restantes do vencedor.
10. **Contabilidade de KO** (`record_ko` ligado no hook de perda de stock do VS). Ponto exato a encontrar: o hook RAM 0x8013BEF0 não tem
    função decomp com esse endereço (ver seção 8). **Aceite:** o pool cai a cada KO; COMPLETE quando zera. Teste de regra.
11. **Fim de jogo** (`gmremixgameend.c`): quando 12CB, não permitir salty runback; se `status != COMPLETE`, ir para o CSS (0x10); se COMPLETE, resultados.
    Fecha o REMIX-TODO da linha 107. **Aceite:** segurar A+B+Z+R+START no fim não reinicia a partida; volta ao CSS.
12. **Cabeçalho e "Game N" nos resultados** (`mnvsresults.c` ou o caminho de cabeçalho). Texto pelo overlay. (dep: overlay ImGui de texto)
    **Aceite:** o texto aparece no lugar certo.
13. **Reset de pausa** (`gentlemens_reset`, `Pause` com dois jogadores). **Aceite:** o jogador 1 pede, o jogador 2 aprova, o jogo volta.
    (dep: T8.1 Pause)
14. **Melhor personagem e TKOs no CSS** (HUD de `set_best_characters` com overlay de texto). (dep: passo 12 e overlay)
15. **Heap no runback** (T6.1: reset de heap do runback). Confirmar que a entrada do CSS reseta `custom_heap_address` (ver seção 8). **Aceite:** 10 jogos seguidos sem crash.
16. **Smoke de 12CB** (lead, com `remix-smoke-tests`): entrar, escolher, perder stocks, runback, reset, voltar ao menu.

## 8. Riscos e incertezas

- **Endereços vanilla sem nome no decomp.** Para todos os hooks de `TwelveCharBattle.asm` e `VsRemixMenu.asm` (RAM 0x8013xxxx, 0x800E7558,
  0x8018D4AC, 0x80113180, 0x801144xx), não há função decomp com o mesmo endereço e mesma tela. Por isso o mapa "função vanilla" é `no match found`,
  e não usei os comentários de endereço. Quatro comentários de endereço do decomp conflitam com o uso do asm:
  - `0x80136788`: `mnplayers1pgame.c:2721` diz `mnPlayers1PGameCheckLevelArrowLInRange` (1P). O hook 12CB em `TwelveCharBattle.asm:1872` é o cabeçalho de resultados. Overlay diferente. **`no match found`.**
  - `0x8013A664`: `mnplayersvs.c:5136` diz `mnPlayersVSSetSceneData` (vazia). `VsRemixMenu.asm:605/667` chama de `mnBattleSaveMatchInfo()`. **Conflito.**
  - `0x8013BDAC`: `mnplayersvs.c:98` diz `sMNPlayersVSGameRule`. A 12CB escreve nesse endereço como `game_mode` (`TwelveCharBattle.asm:370-373`). **Conflito**, pode ser a mesma variável.
  - `0x8013760C`: `mnplayersvs.c:3489` diz `mnPlayersVSSetCursorGrab` (segura o token). `prevent_token_pickup_` (`TwelveCharBattle.asm:3302`) é a mesma área. **Candidato, não verificado.**
- **Comentários de código que não batem com o código:**
  - `handle_stock_count_change_` `_decrement` (`:1994-1995`): o comentário diz "if stocks should be 99, then stock count is 12", o código escreve 99 (`lli a0, 99`).
    Pode ser o comportamento certo (wrap para 99); verificar no jogo.
  - `handle_reset_` (`:2122`): o comentário diz `config.num_stocks = 0`, o código escreve `-1` (`:2134`, `sw t1, 0x0004(t0)`). O efeito (forçar a recontagem) funciona com -1.
  - `defeated_action_override_` (`:2955`, `:3006`): os comentários dizem "0x0E = vs css" e "0x0E = 1p title card", mas `Global.asm:~70` diz que 0x0E é `_1P_LOADING_SCREEN` e 0x10 é `VS_CSS`. Usar o valor numérico e a constante de `Global.asm`.
- **Tabela temporária de `set_best_characters_`** (`:3866-3990`): 13 entradas de 0x10 na pilha (`addiu sp, sp, -0xD0`), 13 é o máximo por jogador. Um jogador só troca de personagem depois de uma derrota
  (`prevent_defeated_char_select` impede o contrário), e há 12 personagens por jogador, então o máximo fica dentro do limite. Mas não há checagem de limite no código.
  O port precisa de array limitado (com teste de limite) ou de contagem explícita.
- **Pool de stocks por portrait, não por jogador**: `stocks_by_portrait_id` é uma tabela só (24 entradas) para os dois jogadores (`update_stocks_remaining_`, `:5399-5406`).
  Se os dois escolherem o mesmo portrait, os stocks são compartilhados. Não sei se é intencional. Verificar no jogo.
- **Duplicatas no grid**: cada personagem aparece em 2 slots (ex. LUIGI em 1 e 5). Cada slot tem seus próprios stocks. O pool do jogador (48) é a soma, então um
  personagem pode ser derrotado num slot e usado no outro. Confirmar se é regra desejada.
- **Stock 0-based vs exibido**: `stocks = 3` (0-based) é 4 stocks, pelo comentário em `update_stock_fields_` e `config`. A exibição do CSS
  (`0xBD80`, `TwelveCharBattle.asm:1717`) também usa esse valor. Confirmar no jogo que o HUD mostra 4.
- **Heap no runback**: a 12CB não passa pelo reset de `GameEnd.asm:144-146` (salty runback é desligado). A entrada do CSS passa pelo
  `load_additional_characters_` com reset (`CharacterSelect.asm:2018-2020`), que é o hook VS em `0x8013B1D8` (`:1989`). O caminho do CSS da 12CB é
  o VS, então esse reset deveria valer. Não verifiquei se o port chama o mesmo caminho. Ver passo 15.
- **Runback não é heap reset**: `setup_` (`:5048`) não reseta o heap; chama `load_additional_characters_._loop` com `s0`/`s1` depois de já ter setado os slots (`:5202-5208`).
- **Porta 2 em posição de 4P** (`change_2p_panel_position_`, `:694-792`, 19 hooks de CSS). Maior risco de regressão visual no CSS VS. Não li cada hook.
- **Mais de 80 pontos com `twelve_cb_flag` fora do arquivo**. Qualquer port parcial que não ligue a flag deixa a 12CB em estado misto (ex.: o CSS mostra 12CB, mas o fim da partida vai pelo caminho VS).
- **Não verifiquei** o caminho de stage select (0x15) e a saída dos resultados (0x18) para a tela de modo (0x09). Só vi os hooks listados.
- **Não li** os trechos de `check_input_` (`:4056-4464`), `get_random_char_id_` (`:4466`), `update_character_set_` (`:2417`) e `draw_custom_portrait_indicators_` (`:4615-5048`)
  em detalhe. Estão mapeados pelos comentários e nomes, não por linha por linha.
