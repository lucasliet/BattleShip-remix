# F15: CSS da 12-Character Battle (spec de port)

Pesquisa somente leitura. Fonte (Remix): `.toolchain/pkgs/smashremix-src/src/` (`TwelveCharBattle.asm` = TCB, `CharacterSelect.asm` = CS,
`Render.asm` = R, `VsRemixMenu.asm` = VRM). Lado do port: `decomp/src/mn/mnplayers/mnplayersvs.c` (= MPV) e `decomp/src/gm/gmremixtwelvecb*.c`.
Contexto de rotas: `docs/remix/notes/f11_mode_twelvecb.md`. Nada foi compilado nem rodado. Coordenadas em unidades N64 (320x240).
Floats do asm decodificados (`0x43200000` = 160.0). Cores `0xRRGGBBAA`.

## 0. Como ler a tela do screenshot do port

A tela mostrada em `build/agent-shots/vs/font12/frame_120.png` (1280x960) NAO e o quadro 320x240 inteiro: o port corta o overscan
de 10 px (`port/gui/PortMenu.cpp:1038`, `.Overscan.Enabled` default true, horizontal 10). Conversao para conferir screenshots:
`px_x = (u_x - 10) * 4.267`, `px_y = (u_y - 10) * 4.364`. **Risco:** `port/remix/remix_hud_text.cpp` (`GameRect`, linha 58) mapeia 320x240 sem
cortar o overscan, entao todo texto do overlay sai deslocado (x: erro de ate 37 px em u=300; o texto "12-Char. Battle" atual em y=236
aparece em px 940 em vez de fora do quadro). Antes de posicionar texto do CSS, o overlay precisa aplicar o mesmo corte (ler o CVar).

## 1. Aparencia do CSS com `twelve_cb_flag` ligada

O CSS 12CB nao e outro CSS: e o CSS VS do Remix com ramos (CS tem ~60 pontos de `twelve_cb_flag`). Diferencas visiveis:

| Item | O que muda | Fonte (asm) |
|---|---|---|
| Grade | **24 retratos, 3 linhas x 8 colunas**, celula 30x30 (arquivo 32x32 em escala **1.0**, sem o 0.8125 da grade normal). Cada metade tem 4 colunas: **P1 usa colunas 0-3 (ids 0-3, 8-11, 16-19), P2 usa colunas 4-7 (ids 4-7, 12-15, 20-23)**. Cada personagem aparece 2x por conjunto (slots n e n+4), um por jogador | TCB:79-80, 334-357; CS draw_portraits_ `CS:3606-3758`, flag em `a0` |
| Posicao final X por coluna | 32, 62, 92, 122 (esquerda), 168, 198, 228, 258 (direita). `START_VISUAL(10)+START_X(30)+30*col`, -8 nas colunas 0-3, +8 nas 4-7. Buraco central 154..168 | TCB:346-357 (`portrait_x_position`) |
| Posicao Y por linha | 35, 65, 95 (`30*row + START_Y(34)+START_VISUAL(10) + (25-34)`). Ultima linha termina em 127 (imagem 32 de altura, passo 30, 2 px de sobreposicao) | CS:3668-3676 (`_get_y`), TCB:335-341 |
| Slide-in | Retratos nascem em x=-20 (colunas 0-3) ou x=310 (4-7) e deslizam ate o X final com velocidade por coluna `1.9, 3.9, 7.8, 11.8, -11.8, -7.8, -3.8, -1.8` (sem passar do alvo). Rotina vanilla 0x80131EE4 = `mnPlayersVSPortraitProcUpdate` | TCB:231-239; CS:3850-3862 (patch 0x80131DC8) |
| Camadas | Textura do retrato: room 0x1B, group 0x12 (port: gobj link 18, DL link 27 como `mnPlayersVSMakeRemixPortrait`). Sem wallpaper de fogo atras. Fonte das imagens: arquivo 0xA05 (`REMIX_CSS_PORTRAITS_FILE_ID`) no offset `kRemixCssPortraitOffset[fkind]` | CS:3606+; `port/remix/remix_css_data.h:108` |
| Retrato derrotado | Retangulo preto semitransparente 30x30, cor `0x000000A0`, alpha ligado, room 0x1B group 0x12, na posicao viva do retrato (segue o slide). Criado/destruido quando `stocks_by_portrait[id] == 0xFF` | TCB:2788-2855 |
| Indicador de variante no retrato | No canto superior esquerdo (ulx/uly do retrato), flags 0x0201: J = `CSS_IMAGES(0xA06)+0x558` escala 0.5625; E = `0xA06+0x3B8` escala 0.5625; Polygon = `0xA06+0x13A8` escala 1.0; outras variantes = stock icon do personagem escala 1.0; variante tipo SPECIAL ou NA: nao desenha. Tipo vem de `Character.variant_type.table` (port: `RemixRosterEntry.variant_type`, `remix_roster_data.h:43`) | TCB:2237-2346 |
| Cabecalho "Free-for-all" | Mesmo sprite, mesma posicao (27, 24) do MPV `mnPlayersVSMakeLabels:1857`; so troca a imagem para o offset **0x2048 do arquivo 0x12** (`sMNPlayersVSFiles[4]`, `llMNPlayersGameModesFileID`). Cor do sprite continua `colors[is_team_battle=0]` = `E3AC04` (efeito depende do formato da imagem, ver secao 6). Offsets dos demais modos no mesmo gancho: Tag Team 0x2738, KoTH 0x29A8, Tug of War 0x30F0, Smashketball 0x2C18 / 0x2E88 | TCB:1865-1902 (hook 0x80134430 = dentro de `mnPlayersVSMakeLabels`) |
| Alternar FFA/Team | Desligado: `mnPlayersVSCheckGameModeInRange` retorna 0 | VRM:968-989 (hook 0x80135270 = `mnPlayersVSCheckGameModeInRange`) |
| Seletor STOCK | Mesmo sprite e posicao (140, 22) e numero em (210/214, 23). Faixa 1..99, so muda com `config.status == NOT_STARTED` (senao FGM `ILLEGAL`); cada mudanca chama `update_stock_fields_` (pool = 12 x stocks, `stocks_by_portrait` = stocks-1 em todos) e redesenha os indicadores | TCB:1956-2118 |
| Botao BACK | Mesmo (244, 23). Como P3/P4 nao tem cursor, **segurar B por 45 quadros em P3 ou P4 volta ao menu VS** (tela 9) | TCB:3727-3775 |
| Paineis 3P/4P | **Nao sao criados**: `InitSlot(2)` e `InitSlot(3)` pulados (cursor, token e painel). Ports 3/4 forcados a N/A sem personagem; controle 3/4 tratado como desligado; checagem de clique/token limitada a 2 | TCB:592-690 (hooks 0x8013B0A8 = `mnPlayersVSInitSlotAll`, 0x801367B4 = `mnPlayersVSCheckPlayerKindSelectAllPlayer`, 0x801396B8 = `mnPlayersVSPuckAdjustPlaced`, 0x8013A2E0 = `mnPlayersVSUpdateControllerOrders`) |
| Painel 2P | Fica na posicao do painel 4P: **todo `start_x` do 2P passa de 69 para 207** (`0xCF` = 3*69). Resultado: cartao vermelho/azul em x=229 (P1 continua em x=22), portas em 188 e 295, scissor das portas = `Shutter4P` (229..295, y 126..217), modelo do lutador no x de 3 (`3*840-1250 = 1270`), cursor inicial em (244, 170), holofote do slot 1 no x de 3, botao HMN/CPU, "2P", CP Level/Handicap (setas e valor), nome/emblema todos relativos ao start_x 207 | TCB:694-1016 (19 hooks) |
| Centro entre os paineis | Livre (x ~91..229) para os textos da secao 2 | TCB:5093+ |
| Token/puck | Sem escala do token em 12CB (usa o puck vanilla, offsets 11/-14, igual ao do port hoje). Posicao automatica (CPU/recall): token ~ (X desenhado +2, Y desenhado +2..4) | CS:808-960, CS:1430-1530 |
| Hit-test do token | Ver 4.3 | TCB:1023-1118 |
| Flash branco ao pousar o token | Mesma regra do port (`mnPlayersVSMakePortraitFlash:3013`), mas posicao do retrato 12CB: ulx +14 (colunas 0-3) / +30 (4-7) em relacao a formula normal, equivalente a celula 12CB + ~1 px | TCB:1544-1561 |

### 1.1 Conjuntos de personagens (layouts), 24 slots cada

Linha 1 = slots 1-8 (colunas 0-7, o grupo de 4 se repete), linha 2 = slots 9-16, linha 3 = 17-24. Os 4 ultimos de cada grupo de 8 repetem os 4 primeiros.
Ids = id Remix = fkind do port. `TCB:81-197`, `TCB:316-332`.

| Conjunto | Linha 1 (4 personagens) | Linha 2 | Linha 3 | Texto |
|---|---|---|---|---|
| 0 `u` Default | Luigi 4, Mario 0, DK 2, Link 5 | Samus 3, Captain 7, Ness 0xB, Yoshi 6 | Kirby 8, Fox 1, Pikachu 9, Jigglypuff 0xA | "Default" |
| 1 `j` Japanese | JLuigi 0x2B, JMario 0x2A, JDK 0x2C, JLink 0x27 | JSamus 0x24, JFalcon 0x28, JNess 0x25, JYoshi 0x31 | JKirby 0x30, JFox 0x29, JPika 0x32, JPuff 0x2E | "Japanese" |
| 2 `pv` Polygon | NLuigi 0x12, NMario 0xE, NDonkey 0x10, NLink 0x13 | NSamus 0x11, NCaptain 0x15, NNess 0x19, NYoshi 0x14 | NKirby 0x16, NFox 0xF, NPikachu 0x17, NJiggly 0x18 | "Polygon" |
| 3 `r` Remix | DRM 0x20, GND 0x1E, YLink 0x1F, Falco 0x1D | DSamus 0x22, Wario 0x21, Lucas 0x26, Bowser 0x34 | Wolf 0x37, Conker 0x38, MTwo 0x39, Marth 0x3A | "Remix" |
| 4 Custom | copia de `u` por jogador (`id_table_p1`, `id_table_p2`, valores mutaveis); indice = `NUM_PRESETS(4)` | | | "Custom" |

Cada conjunto tem 3 tabelas: `id_table` (slot -> id), `portrait_offset_table` (slot -> offset no arquivo 0xA05, = `portrait_offsets.<NOME>` = `kRemixCssPortraitOffset[id]` do port, confere linha a linha) e `portrait_id_table` (id -> slot da copia do P2; para o P1 subtrai 4). Conjunto escolhido por jogador (`config.pX.character_set`, 0..4), trocavel antes de `status` sair de NOT_STARTED. `get_portrait_id_` e `get_valid_portrait_id_` (TCB:1129, 1221) fazem id -> slot (com busca linear no Custom, preferindo o slot atual do token em `CSS player struct +0xB4`).

## 2. Tudo que o CSS desenha (texto, numeros, imagens)

`Render.draw_string(room, group, string, routine, ulx, uly, color, scale, alignment)`: ulx = ancora X (LEFT=0 borda esquerda, RIGHT=1 borda direita, CENTER=2 centro), **uly = topo da caixa do glifo** (16 px x escala), fonte Kabel (`Render.load_font`), blur ligado (R:600-638, R:1649-1683). `draw_number` idem com `Render.update_live_string_` (relê a variavel todo quadro). Escala 0x3F600000 = 0.875 (14 px de caixa). Grupos: ALWAYS 8, NOT_STARTED 9, STARTED 10, COMPLETED 0xC; room 0x1E. Visibilidade decidida **uma vez** em `setup_` (TCB:5128-5137) e refeita por `handle_reset_` (TCB:2193-2204): STARTED visivel se `status >= 1`; NOT_STARTED visivel so se `status == 0`; COMPLETED so se `status == 2`.

Cores: P1 `0xB00000FF` (176,0,0), P2 `0x4040C0FF` (64,64,192), resto branco `0xFFFFFFFF`. (O `gmremixtwelvecbhud.c` atual usa tons mais claros de proposito; para fidelidade usar os do asm ou registrar o desvio.)

### 2.1 Texto fixo do CSS (`setup_`, TCB:5050-5217)

| # | Condicao | Conteudo | ulx, uly | Cor | Escala | Alinh. | Asm |
|---|---|---|---|---|---|---|---|
| 1 | sempre | "Stocks Remaining" | 160.0, 126.0 | branco | 0.875 | CENTER | TCB:5093 |
| 2 | sempre | pool P1 (`config.p1.stocks_remaining`, inteiro vivo) | 124.0, 140.0 | P1 | 0.875 | CENTER | :5094 |
| 3 | sempre | pool P2 | 192.0, 140.0 | P2 | 0.875 | CENTER | :5095 |
| 4 | `status==0` | "Character Set" | 160.0, 160.0 | branco | 0.875 | CENTER | :5098 |
| 5 | `status==0` | nome do conjunto P1 (ponteiro, 5 strings: Default/Japanese/Polygon/Remix/Custom) | 124.0, 173.5 | P1 | 0.875 | CENTER | :5099, :5252 |
| 6 | `status==0` | nome do conjunto P2 | 192.0, 173.5 | P2 | 0.875 | CENTER | :5100 |
| 7 | `status==0` | setas do conjunto, arquivo 0x11 (`sMNPlayersVSFiles[0]`): esq. `llMNPlayersCommonArrowLSprite` (0xECE8) em (91.5, 175.5) e (159.0, 175.5); dir. `ArrowRSprite` (0xEDC8) em (153.0, 175.5) e (221.0, 175.5). cor `0xFF0000FF`, paleta `0x303030FF`, escala 0.625 | | | | | :5101-5104 |
| 8 | `status>=1` | "Best Character" | 160.0, 160.0 | branco | 0.875 | CENTER | :5109 |
| 9 | `status>=1` | nome do melhor personagem P1 / P2 (ponteiro, `max width 72`: a string e comprimida na horizontal para caber em 72) | 124.0 / 192.0, 174.0 | P1 / P2 | 0.78125 | CENTER | :5110-5118, `R:1783` |
| 10 | `status>=1` | TKOs a favor (so o numero; **a string "TKOs" existe mas nao e desenhada em lugar nenhum**) | 124.0 / 192.0, 187.0 | P1 / P2 | 0.78125 | CENTER | :5120-5121 |
| 11 | `status>=1` | botao RESET: sprite offset **0x187A8 do arquivo 0x11**, cor `0xFFFFFFFF`, paleta `0x000000FF`, escala 1.0, em (136.0, 206.0). Clique => `handle_reset_` + FGM 0xA4 | | | | | :5124, :2122-2230, :2675-2693 |
| 12 | `status==2` | coroa do vencedor: `llMNVSResultsWinnerSprite` (arquivo 0x22, 0xE2A0) em (84.0, 136.0), escala 0.625, cor branca, paleta `0x000000FF`; se P1 sem stocks, x = 208.0 | | | | | :5142-5150 |
| 13 | `status==0`, P1 ou P2 segura token sobre portrait do Custom | legenda "Set All / Presets / Random / Copy" no painel do jogador (room 0x1C group 0x16): setas esq. (26,172) dir. (78,172) via 0xECE8/0xEDC8, botoes Z (26,154) R (71,156), L (25,216), D-pad (arquivo 0xA06 off 0x218, escala 0.75) em (93,186), (93,200), (93,214); textos escala 0.8125 LEFT: ": Set All" em (41,217) P1 / (247.5,217) P2, ": Presets" (105 P1 / 173 P2, 187), ": Random" (105/173, 201), ": Copy" (105/173, 215). Botoes do arquivo 0xC5 (Z 0xBD8, R 0xCF8, L 0x18C8) | | | | | :4615-5046 |
| 14 | tecla (Z/R seguro, L solto, D-baixo, D-cima tocado/segurado 60q) | Edicao do Custom: Z/R segurado cicla o personagem do retrato sob o token (delay inicial 30q, repeticao 8q), L solto = aplica o personagem do retrato a todos, D-baixo = aleatorio em todos, D-cima solto <60q = proximo preset, segurado 60q = copia o conjunto do outro jogador | | | | | :4056-4464 |

### 2.2 Indicador de stocks no painel (CS `update_stock_icon_and_count_indicators_`, CS:5499-5915)

Em 12CB aparece **sempre** (ignora o StockMode do painel), para o personagem exibido no painel, valor `stocks_by_portrait[portrait]+1` (portrait = `CSS struct +0xB4`). room 0x38, group 0x13, blur.

| Parte | Posicao | Detalhe |
|---|---|---|
| Numero | borda direita = (X do cartao + largura) + padding; padding -4 (sem setas), -10 (setas, 1 digito), -7 (setas, 2 digitos); uly **191.0** | branco, escala 0.90625 (0x3F68), RIGHT, +1 sobre o valor 0-based |
| Icone de stock | ulx = ulx do 1o digito -18; uly 191.0 | stock icon do personagem do painel, paleta da cor do traje |
| "x" | ulx do 1o digito -9; uly 192.0 | escala 0.875, tipo I8, `llIFCommonDigitsCrossSprite` (arquivo 0x24 off 0x828) |
| So no StockMode Manual, portrait ainda nao jogado e 12CB nao completa | com ficha selecionada: setas esq. (cartao.x+20, 193) e dir. (urx-5, 193), escala 0.625, pisca 10q on/10q off; sem ficha: D-pad (arquivo 0xA06 off 0x218, escala 0.5625) em (ulx do digito -27.5, 192) e 2 retangulos amarelos | CS:5700-5960; `check_manual_stock_arrow_press_` CS:6040; `update_remaining_stocks_` CS:6195 |

O painel do **P2 em x=229** faz o numero ficar em torno de x=229+largura-4. Largura do cartao = `Sprite` do cartao (`llMNPlayersCommonRedCardSprite`), ler em runtime.

### 2.3 Resultados e pausa (mesmo codigo)

| Tela | Conteudo | Asm |
|---|---|---|
| Resultados (0x18) cabecalho | mesmo troca de imagem para offset 0x2048 (arquivo 0x12 = `sMNVSResultsFiles[2]`), no gancho 0x80136788 = dentro de `mnVSResultsMakeLabel` (`mnvsresults.c:2520`, sprite em (32, 29)) | TCB:1904-1930 |
| Resultados "Game N" | `draw_string(room 0x1F, group 0x16, "Game   ")` com N em 2 digitos (`current_game+1`), em (**32.0, 46.0**), branco, 0.875, LEFT (logo abaixo da linha branca y 42..44 de `mnVSResultsLabelProcDisplay`) | TCB:1931-1947 |
| Pausa, pedido de reset (so 12CB, `reset_requested`) | room 0x18 group 0xE, escala 1.0, branco: "Game reset requested by  P." (digito '1'/'2' na posicao 0x18) CENTER (160,126); "If accepted, stock counts will be reset." CENTER (160,144); "Accept:" LEFT (40,164) + sprites do combo (nativa 0x80113D60, indices 1..7, deslocados -4 em x e -39 em y); "Decline:" LEFT (188,164); `llMNSoundTestStartButtonSprite` (arquivo 0xC4 off 0x1D50) em (244,164), cor `0xFF0000FF`, paleta `0x303030FF`. Destroi a legenda de pausa (groups 0xF e 0x10). O controle passa ao outro jogador (HMN) | TCB:3523-3638 |
| Pausa, legenda "R: Legend" | nao desenha durante o pedido (`Pause.asm:255`) | `f14_pause_legends.md` |
| Pausa, loop de portas | 12CB so olha 2 portas (`s4=2`) | TCB:3712-3718 |

## 3. Estado do port hoje (leitura)

- `mnPlayersVSInitVars` (MPV:5708) ja fecha os slots 2/3 (`pkind=Not`) em 12CB (MPV:5781-5799) mas **continua criando** `InitSlot(2)`/`InitSlot(3)` e os 4 paineis (screenshot: 3P/4P "N/A" com mao).
- A grade e `mnPlayersVSMakePortraitAll` (MPV:647), 30 slots Remix, 10 colunas de 24 (`remix_css_data.h`), escala 0.8125, sem slide-in.
- Cabecalho fixo `FreeForAll`/`TeamBattle` (MPV:1814, 2377); o texto "12-Char. Battle  Game N" e "P1 Stocks 48 ..." e overlay centralizado em y=236/227 (`gmremixtwelvecbhud.c`), nao corresponde ao asm.
- Rules (`gmremixtwelvecbrules.c`) ja tem pool, portrait, KO e melhor personagem; `gmRemixTwelveCBPortraitOf(fkind)` (placeholder: slots 0..11 = fkind) e a ponte que o CSS real substitui.
- Nao existe no port: indicador de stock no painel, bloqueio de stock apos o inicio, retangulo de derrotado, conjuntos de personagens, RESET, travar vencedor no mesmo retrato, "Game N" no lugar do asm.

## 4. Mudancas no MPV (funcoes verificadas com grep, linha atual)

### 4.1 Painel e P2 na posicao do P4 (M)

Criar `mnPlayersVSGetPanelIndex(player)` = `(player == 1 && gmRemixTwelveCBIsActive()) ? 3 : player` e trocar `player * 69` por `panel_index * 69` em todos os pontos: MakeTeamSelect `MPV:683`, UpdatePlayerKindSelect `:738`, MakeRemixNameAndEmblem `:770,788`, MakeNameAndEmblem `:835,853`, UpdateShutter `:865,868`, MakePlayerKindSelect `:1184`, MakePlayerKind `:1209,1214`, MakeGate `:1267`, CheckTeamSelectInRange `:2444`, CheckHandicapArrowRInRange/LInRange `:2566,2593`, CheckPlayerKindSelectInRange `:2620`, ArrowThreadUpdate `:3241,3263`, MakeHandicapLevel `:3307,3313,3324`, MakeHandicapLevelValue `:3370`. Alem disso: `proc_displays[player]` em `MakeGate:1237` (indice 3), `translate.x = (player*840)-1250` em `MakeFighter:2105`, spotlight `MakeSpotlight:4837` (i=1 em x=1270; i>=2 fora da tela como o asm), `pos[]` de `MakeCursor:4440` (indice 3) e `MakePuck` (o fkind==Null usa (51,161) para todos, nao muda).

### 4.2 Remover P3/P4 (S/M)

`mnPlayersVSInitSlotAll` (MPV:5829): em 12CB chamar so slots 0 e 1. `mnPlayersVSUpdateControllerOrders` (MPV:5041): ordens 2 e 3 = -1 em 12CB. Auditar nulos de `cursor`, `puck`, `panel`, `panel_doors`, `name_emblem_gobj`, `type_button` dos slots 2/3 em `mnPlayersVSUpdateGateAll` (:2300), `mnPlayersVSFuncRun` (:5371), `mnPlayersVSCheckPlayerKindSelectAllPlayer` (:3088), `mnPlayersVSCheckCursorPuckGrab` (:3563, itera 4 jogadores), `mnPlayersVSPuckAdjustPlaced` (:4632) e `mnPlayersVSUpdateCursorGrabPriorities` (:3442). Novo `mnPlayersVSDetectBackPorts34` (B segurado 45q em P3/P4 chama `mnPlayersVSBackToVSMode`, MPV:3871).

### 4.3 Grade 8x3, hit-test e portrait por jogador (L)

- Novas funcoes: `mnPlayersVSTwelveCBCellX/Y(slot)`, `mnPlayersVSMakeTwelveCBPortrait(slot)` (copia de `MakeRemixPortrait:468` com escala 1.0, offset de `kRemixCssPortraitOffset[id_table[slot]]`, X do slide), `mnPlayersVSTwelveCBPortraitProcUpdate` (slide-in; port ja tem `mnPlayersVSGetNextPortraitX:271` que usa tabela de 12, criar versao de 8 colunas), desvio em `mnPlayersVSMakePortraitAll:647`.
- Hit-test (`mnPlayersVSGetRemixPuckFighterKind:3607`, ramo 12CB): com `xpos = puck.x + 13`, `ypos = puck.y + 12` (a conversao do port) aplicar o asm: descarta `ypos >= 110`; `row = (ypos-25)/30`; P1: x em [19,138] e `col=(xpos-19)/30`; P2: x em [155,274] e `col=(xpos-155)/30`; `portrait = row*8 + col (+4 se P2)`; precisa `portrait < 24`; fkind = `id_table_set[portrait]`. **Convencao 0xA a calibrar:** o port usa `puck+13/+12` contra um canto 39,44 (grade normal) enquanto o asm da grade normal contra 29,35; para 12CB seguir o asm literal e conferir por screenshot com um token (TCB:1023-1118, CS:486-520).
- Portrait vira estado por jogador: novo `sMNPlayersVSSlots[player].portrait` (ou array estatico) = `CSS player struct +0xB4` do asm (-1 sem retrato). Muda: `mnPlayersVSGetPortrait` (MPV:505) so recebe fkind e nao basta (duplicata no Custom): usar `mnPlayersVSTwelveCBGetPortrait(player, fkind)` em `mnPlayersVSCenterPuckInPortrait:4197`, `mnPlayersVSMakePortraitFlash:3013`, `mnPlayersVSAnnounceFighter:3112`, e alimentar `gmRemixTwelveCBSetNextPortrait` em `gmremixtwelvecb.c` (`gmRemixTwelveCBPickPortraits`) com esse valor em vez de `gmRemixTwelveCBPortraitOf`.
- `mnPlayersVSRandFighterKind:4219`: em 12CB sorteia 0..23, ajusta para a metade do jogador, rejeita vazio/derrotado (exceto batalha completa) e, se o jogador venceu a ultima partida, forca o retrato anterior (CS:1155-1228).
- Selecionar (`mnPlayersVSSelectFighter:3421`, hook 0x801371EC): bloqueia personagem derrotado (stocks 0) e, se o jogador venceu a ultima partida, so deixa selecionar o mesmo retrato (TCB:2860-2929). Regra de CPU/Random/Custom em `setup_ports_and_characters_` (TCB:452-588): checar `gmRemixTwelveCBCheckSlot` ja existente.

### 4.4 Cabecalho, stock e botoes (S)

`mnPlayersVSMakeLabels:1806` e `mnPlayersVSUpdateGameMode:2369`: em modo nao-default usar o offset do modo (12CB 0x2048) em `sMNPlayersVSFiles[4]` (`lbRelocGetFileData(Sprite*, ..., 0x2048)`); `mnPlayersVSCheckGameModeInRange:2344` retorna FALSE. Stock: em `mnPlayersVSCursorProcUpdate:3973` (blocos de seta do STOCK, MPV:4001-4060) chamar novo `gmRemixTwelveCBOnStockChange(value)` (bloqueia com FGM ILLEGAL se `status != NOT_STARTED`, senao `gmRemixTwelveCBUpdateStocks`).

### 4.5 Texto e sprites do centro (M/L)

- Overlay: o `remix_hud_text.h` do working tree (alterado por outra sessao depois da leitura inicial) ja tem `remix_hud_text_put(slot, text, x, y_topo, rgba, scale, align)`, `REMIX_HUD_ALIGN_*`, `remix_hud_rect_put` (serve ao retangulo de derrotado `0x000000A0` se o overlay couber, mas ele desenha **sobre** o jogo, nao entre o retrato e as demais camadas) e 128 slots. Falta so a **largura maxima** do "Best Character" (72): estender `put` ou comprimir via `RemixFont::Measure`. `gmremixtwelvecbhud.c` ainda chama a assinatura antiga (centro + altura) e precisa migrar. Conferir se o novo `remix_hud_text.cpp` ja corta o overscan (grep por `overscan` nao achou nada na leitura).
- Sprites do centro vao pelo caminho de sprite do jogo, nao pelo overlay: setas (`llMNPlayersCommonArrowL/RSprite`, `sMNPlayersVSFiles[0]`), RESET (0x187A8, mesmo arquivo), coroa (arquivo 0x22 `llMNVSResultsWinnerSprite`, novo id em `dMNPlayersVSFileIDs`), "x" e icone (arquivo 0x24 `llIFCommonDigitsCrossSprite`, icone vindo do lutador do painel), D-pad e variantes (arquivo 0xA06 `REMIX_CSS_IMAGES_FILE_ID`, novo item em `dMNPlayersVSFileIDs`).
- `gmRemixTwelveCBHudDrawCSS` (`gmremixtwelvecbhud.c`): reescrever para a tabela 2.1 (remover os rotulos de y=236/227).

### 4.6 Dados: gerar ou ler em runtime

| Dado | Origem | Decisao |
|---|---|---|
| Layouts `u/j/pv/r` (24 ids cada), `NUM_COLUMNS`, `START_*`, `PORTRAIT_*` | constantes de montagem (`define slot_N`) em TCB:81-197 e 334-341 | **Gerar** com `tools/gen_remix_css.py` (ja tem `slot_ids`, `define_name("layout.slot_N", scope)`) para `kRemixTwelveCbLayout[4][24]` em `remix_css_data.h`; nao usa o RAM image |
| Offsets de retrato por id | `kRemixCssPortraitOffset[97]` ja gerado | Reusar |
| X final e velocidade por coluna | formulas e tabela de 8 floats (TCB:231-357) | Hardcode em C com comentario de origem |
| Strings, cores, coordenadas da secao 2 | literais do asm | Hardcode |
| `Character.variant_type.table` | porta ja tem `RemixRosterEntry.variant_type` | Reusar; nao precisa de `REMIX_SYM_*` |
| `config.*` (status, current_game, pools...) | estado mutavel | **Nao ler** do RAM image; `gmremixtwelvecbrules.c` e a fonte. Nao existe simbolo `TwelveCharBattle` em `remix_symbols.h` e nenhum e necessario |
| Largura/altura dos sprites Remix-only (0x2048, 0x187A8, 0x218, 0x558, 0x3B8, 0x13A8) | cabecalho `Sprite` no arquivo em runtime | Ler em runtime (`lbRelocGetFileData`), nao hardcodear |

## 5. Passos pequenos e verificaveis

Teste base: `SSB64_REMIX_MODE=12cb SSB64_START_SCENE=16 .claude/skills/remix-smoke-tests/scripts/css.sh <input> <nome> 120` e sheet; tela de referencia a 4.267/4.364 px por unidade (secao 0). Para testes de regra, `tests/remix` headless.

| # | Passo | Esforco | Teste / aceite |
|---|---|---|---|
| 1 | Overlay corta o overscan igual ao jogo (`GameRect`, `remix_hud_text.cpp:58`, ler CVar) | S | screenshot: texto de teste em u=(32,35) cai sobre o canto do 1o retrato |
| 2 | Largura maxima no overlay (a ancora/alinhamento ja existem no `.h` atual) e migrar `gmremixtwelvecbhud.c` para a assinatura nova | S | teste C do calculo de posicao + screenshot |
| 3 | `mnPlayersVSGetPanelIndex` + troca dos `* 69` e das demais tabelas (4.1) | M | `css.sh none` em 12cb: painel 2P em x=229 (px ~934), P1 em x=22 (px ~51) |
| 4 | Sem `InitSlot(2/3)`, controle 3/4 desligado, B 45q em P3/P4 volta ao menu (4.2) | S/M | screenshot sem 3P/4P; log sem crash por 600 quadros; input B segurado em porta 3 = tela de modos |
| 5 | Cabecalho 0x2048, sem toggle FFA/Team (4.4) | S | screenshot "12-Char. Battle" em (27,24); clique no cabecalho nao muda |
| 6 | Gerar layouts no `gen_remix_css.py` + teste: 4 conjuntos x 24 slots, 12 ids distintos x2, ids do TCB:81-197 | S | teste headless |
| 7 | Grade 8x3, escala 1.0, sem slide, posicoes da secao 1 (4.3) | M | screenshot: 24 retratos, X 32..258, Y 35..95; metades P1/P2 |
| 8 | Hit-test e portrait por jogador, `gmRemixTwelveCBSetNextPortrait` com o valor do CSS (4.3) | L | `css_pick_four.txt` adaptado: P1 em coluna 0 linha 0 = Luigi (slot 0), P2 no mesmo personagem = slot 4+; log `twelvecb: game 0 p1 ...` |
| 9 | Slide-in 8 colunas | S | sheet de quadros 0..40 |
| 10 | Textos do centro tabela 2.1 itens 1-6 (stocks remaining, conjunto, setas) | M | screenshot: "Stocks Remaining" (160,126), 48 / 48 em 124 e 192, nomes em y 173.5 |
| 11 | Troca de conjunto pelas setas (hit zones 123.5/91.5/191/159, y 172, h 14) e redesenho dos retratos | M | clique nas setas ciclando 5 conjuntos; sheet |
| 12 | Retangulo de derrotado (alpha 0xA0) e travar derrotado/vencedor | M | rodar 2 jogos, 2o CSS: retrato do perdedor escurecido |
| 13 | Indicador de stocks no painel (2.2, sem Manual) | M | screenshot: icone + x + N nos dois paineis, N=4 no inicio |
| 14 | `status>=1`: Best Character, TKOs, botao RESET, coroa, "Game N" nos resultados | M | log + screenshot do CSS apos 1 jogo; resultados com "Game 1" em (32,46) |
| 15 | Bloqueio de stock apos o inicio e `update_stock_fields_` ligado ao seletor (4.4) | S | stock 1..99, FGM ILLEGAL depois do jogo 1 |
| 16 | Variantes nos retratos (flag J/E, poligono, stock icon) | M | Japanese e Polygon mostram o icone; sheet |
| 17 | Aleatorio no CSS e travamento do vencedor (CS:1155-1228) | M | teste de regra + screenshot |
| 18 | Pausa: Gentlemen's Reset (2.3) | L | depende de T8.1 Pause |
| 19 | Custom set (Z/R/L/D-pad, legenda, copia) e StockMode Manual (arrows/dpad) | L | por ultimo; testes de regra + sheet |

## 6. O que NAO deu para determinar

1. **Tamanho/formato dos sprites Remix-only**: o `build/BattleShip-Remix.o2r` do repositorio so tem placeholders zerados (`reloc_menus/MNPlayersGameModes` 12672 bytes, zero em 0x2048/0x30F0). Nao sei largura/altura do titulo 0x2048, nem se e IA (tingido por `E3AC04`) ou RGBA (ignora a cor). Usar `SSB64_DUMP_FILE_ID=18` (arquivo 0x12) e `=17` (arquivo 0x11, RESET 0x187A8) com o o2r real do usuario.
2. **Relacao exata entre `puck.x/y` e `xpos/ypos` do asm** (CS:486-520, `f6/f10` sao os floats do token): a conversao `puck+13/+12` e inferencia do port; o asm da grade normal e o do port diferem em 10 px, entao a calibragem de 4.3 e obrigatoria.
3. **Extensao do hit-test do RESET e do "Set All"** (`CharacterSelect.check_image_press_`, `check_press_`): so a caixa do cartao de setas esta no asm (x 91.5/123.5/159/191, y 172, h 14, largura 32/33); RESET usa o tamanho do sprite.
4. **Posicoes exatas dos elementos do painel do P2 em start_x 207**: derivadas do start_x vanilla + offsets de MPV (tabela 4.1); nao verifiquei visualmente CP Level/Handicap com 207 (so os 19 hooks de `change_2p_panel_position_` indicam que todos usam 0xCF).
5. **Trechos nao lidos linha a linha**: `check_input_` (TCB:4286-4464, ramo `_update_all`/preset/copia), `get_random_char_id_` (TCB:4466), `portrait_change_` (TCB:4535), `prevent_invalid_cpu_` (TCB:4590) e o ramo de variantes/legenda do Custom (TCB:4900-5046).
6. **O mapeamento id -> sprite dos conjuntos Japanese/Polygon no port**: `kRemixCssPortraitOffset` cobre 0x00..0x60, mas nao conferi se os fkinds 0x24..0x32 (J/E) e 0xE..0x19 (polygon) estao registrados como lutadores jogaveis no `fighter_registry` (afeta o painel e o modelo do CSS).
7. **Anuncio de entrada** (`FGM.announcer.css.TWELVECB`, e "Player X Wins" + WINS no quadro 0x34, TCB:1746-1861) nao foi planejado aqui (fica no `mnPlayersVSFuncStart:5909-5921`).
8. **Colisao de largura/cor do texto do overlay com a fonte**: `max width 72` do "Best Character" exige comprimir glifos; o overlay atual so escala uniforme (`RemixFont::Draw`).
