# F15 notes: parametros exatos do texto de HUD do Remix (fonte Kabel) para cada chamador do port

Escopo: pesquisa somente leitura. Fonte autoritativa: `.toolchain/pkgs/smashremix-src/src` (caminhos abaixo relativos a ela). Os chamadores do port
ainda carregam valores da era ImGui. Nada foi editado, compilado ou executado. UNKNOWN marca o que nao deu para determinar.

## 0. Convencoes e regras do Render.draw_string_ (valem para todas as tabelas)

| Fato | Evidencia |
|---|---|
| Alinhamento: LEFT = 0, RIGHT = 1, CENTER = 2. X' = X - (largura)/alinhamento: RIGHT usa X - w, CENTER usa X - w/2, LEFT usa X. | Render.asm:117-121, 1652-1683 (`div.s f0, f0, f2` com f2 = 1 ou 2) |
| Largura w = soma de (largura_glifo + 1) * scale; espaco = 4 * scale; o `scale` e usado direto (default_font_size NAO multiplica). Bate com `RemixFont::Measure`. | Render.asm:1655-1675, 1711-1716, 1761-1766 |
| Numeros (draw_number) viram string via `String.itoa_` sem sinal: so digitos, sem "%" nem zeros a esquerda. Com `decimals = d` a string tem no minimo d+1 digitos (0 com 1 decimal vira "0.0", 5 vira "0.5"). O argumento de numero e SEMPRE o endereco do valor (mesmo no tipo NUMBER, nao so no *_POINTER). | Render.asm:1629-1646, 713-735; String.asm:29-150 |
| Conversao para o port: `height = 16 * scale`, `y_port = y_top + 8 * scale`, `x_center = X` (CENTER), `X + w/2` (LEFT), `X - w/2` (RIGHT) onde w = `RemixFont::Measure(texto, scale)`. Precisa de um parametro de alinhamento (LEFT/RIGHT/CENTER) em `remix_hud_text_put`; o overlay calcula a ancora com Measure. | pedido; Render.asm |
| ATENCAO (diferenca nao tratada pelo port): cada glifo e posto em `Y + (14 - altura_do_glifo) * scale`, onde altura_do_glifo e o byte 1 da entrada de 8 bytes de `character_offsets_*`. `RemixFont::Draw` desenha todo glifo como celula 16x16 em `position.y` e ignora esse ajuste (e usa 14, nao 16, como linha de base). Efeito visual UNKNOWN (depende de a sprite do glifo ser aparada ou nao). | Render.asm:1726-1738; remix_font.cpp `Draw` |
| `blur` (ultimo argumento): os macros de 9 argumentos usam blur = 1 (antialias, flags 0x0201). Com blur = 0 a flag cai para 0x0200 e a cor de mistura fixa vira 0x30303030. O port nao distingue; so afeta a suavizacao. | Render.asm:600-638, 1740-1748 |
| Fontes: `Render.load_font()` = Kabel (File.CUSTOM_FONT 0xA15, character_offsets_custom, default 1.0). `load_font_2()` = file 0xC3 (fonte dos creditos, default 0.625). TODOS os chamadores abaixo usam `load_font()` (Kabel). | Render.asm:560-583; File.asm:464 |
| Slots: `REMIX_HUD_TEXT_SLOTS = 32` NAO basta se as paginas de stats virarem uma string por rotulo/celula (ver secao 2: ate 73 strings numa pagina). Sugestao: subir para >= 128 e reservar faixas por chamador. | remix_hud_text.h:20 |
| O texto de HUD do Remix nunca tem sombra; a cor e sempre branca (0xFFFFFFFF) nos casos abaixo. | todas as chamadas abaixo |
| Os "glifos de botao" (A, B, R, L, Z, +, D-pad) NAO sao texto: sao texturas do file 0xC5 (offsets em `Render.file_c5_offsets`: A 0x0958, B 0x0A88, L 0x18C8, R 0x0CF8, Z 0x0BD8, PLUS 0x04D8, L_THIN 0x1B10) e do file CSS_IMAGES 0xA06 (D-pad 0x218). Os espacos dentro das strings ("Press     for ...") reservam o buraco da sprite. | Render.asm:136-144; File.asm:449 |

Constantes de escala usadas (hex float -> decimal): 0x3F800000 = 1.0, 0x3F700000 = 0.9375, 0x3F666666 = 0.9, 0x3F600000 = 0.875,
0x3F500000 = 0.8125, 0x3F400000 = 0.75, 0x3F200000 = 0.625.

---

## 1. mnremixvsstatsview.c (Press A, B, R) - VsStats.asm

Fonte: VsStats.asm:45,71,72 (strings), 917-967 (`setup_`), 675-868 (`check_menu_toggle_`).
`setup_` chama `Render.load_font()` (VsStats.asm:934) e carrega file 0xC5 em file_pointer_1 (935).

| Elemento | String exata | X | Y_top | scale | altura | cor | align | fonte | Evidencia |
|---|---|---|---|---|---|---|---|---|---|
| Prompt "Press A" | `"Press     for Match Stats"` (5 espacos entre "Press" e "for") | 160.0 (0x43200000) | 218.0 (0x435A0000) | 0.8125 | 13.0 | 0xFFFFFFFF | CENTER | Kabel | VsStats.asm:940 (blur = FALSE) |
| Sprite A do prompt (NAO e texto) | - | 126.0 (0x42FC0000) | 216.0 (0x43580000) | 1.0 | - | tint 0x50A8FFFF, palette 0x0010FFFF | - | file 0xC5 + 0x0958 | VsStats.asm:946 |
| Hint "B" | `": Back"` | 34.75 (0x420B0000) | 15.9375 (0x417F0000) | 0.8125 | 13.0 | 0xFFFFFFFF | LEFT | Kabel | VsStats.asm:958 |
| Sprite B (NAO e texto) | - | 22.5 (0x41B40000) | 14.0 (0x41600000) | 1.0 | - | tint 0x00D040FF, palette 0x003000FF | - | file 0xC5 + 0x0A88 | VsStats.asm:959 |
| Hint "R" | `": Next Page"` | 87.0 (0x42AE0000) | 15.9375 (0x417F0000) | 0.8125 | 13.0 | 0xFFFFFFFF | LEFT | Kabel | VsStats.asm:962 |
| Sprite R (NAO e texto) | - | 71.5 (0x428F0000) | 15.0 (0x41700000) | 0.9375 | - | tint 0x848484FF, palette 0x303030FF | - | file 0xC5 + 0x0CF8 | VsStats.asm:963 |

Argumentos novos para `remix_hud_text_put` (com parametro de alinhamento; `y_port = Y_top + 6.5`, `height = 13.0`):

| Texto | align | X ancora | y_port | height |
|---|---|---|---|---|
| "Press     for Match Stats" | CENTER | 160.0 | 224.5 | 13.0 |
| ": Back" | LEFT | 34.75 | 22.4375 | 13.0 |
| ": Next Page" | LEFT | 87.0 | 22.4375 | 13.0 |

Sem suporte a sprite de botao, fallback aproximado: desenhar `"A"` / `"B"` / `"R"` como texto nas posicoes das sprites (126/216, 22.5/14, 71.5/15). A largura real do glifo
de botao e UNKNOWN (as sprites C5 nao foram medidas), entao o ":" pode nao cair exatamente em X = 34.75 / 87.0.

Condicao e vida:
- Grupo 0x0D (prompt + sprite A) nasce escondido (`sw at, 0x007C(v0)` = 1 em VsStats.asm:943-944, 949-950). E revelado por `check_menu_toggle_` quando
  `tempo_atual + STATS_TIME_UNTIL_ACTIVE(105) >= tempo_ate_START_valer` (VsStats.asm:686-699; 0x80139B78 e 0x80139C18, `STATS_TIME_UNTIL_ACTIVE` em :668).
- Ao apertar A (buffer de 15 tics = `STATS_PRESS_BUFFER_PERIOD`, :669, :707-725) o grupo 0x0D e escondido, o grupo 0x16 (linhas normais dos resultados) tambem, e o
  grupo 0x0E (fundo + hints B/R + cabecalhos de porta) e mostrado junto com o grupo da pagina atual (0x1A + pagina) (:734-751).
- B esconde 0x0E e todas as paginas e mostra 0x0D e 0x16 de volta (:756-811). R avanca a pagina (2 paginas, `TOTAL_PAGES(2)`, :38) com FGM scroll (:814-862).
- Os hints B e R fazem parte do grupo 0x0E (somem com o fundo), nao ficam sempre visiveis.

---

## 2. mnremixvsstatspages.c - paginas de stats - VsStats.asm

Fonte: `draw_line_` (VsStats.asm:354-480), `draw_underline_` (288-307), `draw_port_headers_` (542-666), `setup_` paginas (970-1040).
TODAS as strings desta tela: scale 0.8125 (`lui s4, 0x3F50`), height 13.0, cor 0xFFFFFFFF, blur FALSE, Kabel.
`y_port = Y_top + 6.5`.

### 2.1 Elementos e coordenadas

| Elemento | Texto | X | Y_top | align | Evidencia |
|---|---|---|---|---|---|
| Fundo (retangulo, nao texto) | - | 10, w 300 | 10, h 220 | - | cor 0x000000B0 com alpha: VsStats.asm:955 (grupo 0x0E) |
| Cabecalho de coluna de porta k (k = indice entre as portas ATIVAS, 0..3) | `"1P"`..`"4P"` (a string da porta, nao do slot k) | 184 + 35k (= 158 + 35k + 26) | 16.0 (0x41800000) | RIGHT | draw_port_headers_: VsStats.asm:550 (158), 616-631 (+26 em :621, scale 0x3F50 em :627) |
| Icone de stock da porta (nao texto) | - | 158 + 35k | 16.0 | - | draw_texture_ do `stock icon` do personagem, scale 1.0: VsStats.asm:594-604 |
| Sublinhado do cabecalho de porta (retangulo) | - | 158 + 35k, w 26 | 26, h 1 | - | branco, sem alpha: VsStats.asm:633-643 |
| Rotulo de linha | ver 2.2 | 24 + indent (indent 0 ou 8) | y da linha (ver 2.3) | LEFT | draw_line_: VsStats.asm:409-419 (`lli t0, 24`, `addu t0, t0, t5`) |
| Celula numerica da porta k | inteiro decimal sem padding; ou `"-"` | 184 + 35k (inicio `lli t1, 184` :378, `addiu t0,t0,35` :440) | y da linha | RIGHT | VsStats.asm:431-457 |
| Faixa (zebra) da linha, retangulo | - | 23, w 266 | y da linha, h 10 | - | cor 0x40407040 com alpha: VsStats.asm:394-402 |
| Sublinhado de secao, retangulo | - | 24, w = 75 / 68 / 80 / 58 / 65 / 105 (dano/combo/zcancel/tech/ledge/special) | y_secao, h 1 | - | branco sem alpha: draw_underline_ VsStats.asm:288-307; larguras em :976,1000,1015,1023,1031,1037 |

Pontos finos confirmados no asm:
- As colunas k sao empacotadas so entre portas ativas (o incremento de X esta dentro do caminho ativo): VsStats.asm:426-441 e 645-660. O port ja faz isso.
- Linha com `ignore_port` de uma porta inativa e pulada inteira e NAO consome Y (`jr ra` cedo, VsStats.asm:355-366). O port ja faz isso.
- A celula `"-"` e desenhada quando `porta == port_to_skip` (VsStats.asm:445-452). O rotulo `"1P".."4P"` tem indent 8.
- "Success %" mostra so o numero (0..100), sem "%" (o rotulo e a unica parte com "%"): VsStats.asm:1016, 1024; calculo em :168-203.
- Em cada linha o rotulo e as celulas sao strings separadas de uma mesma altura Y_top. O port atual junta tudo em UMA string com espacos e centraliza em X = 160; isso nao alinha com as colunas RIGHT do asm.
- Zebra: `stripe_on` e um byte global (VsStats.asm:30-31) que so e invertido em `draw_line_` para linhas COM valor (a0 != 0) (:386-393) e NUNCA e zerado em `setup_`.
  Logo, a paginacao e o estado herdado: a primeira linha com valor NAO tem faixa, a seguinte tem, e assim por diante, atravessando a pagina 1 -> 2 e tambem de uma partida para outra
  (o estado persiste na RAM). O port zera `stripe_on` a cada `Draw`, entao a pagina 2 pode diferir quando a pagina 1 tem numero impar de linhas com valor
  (4 portas + combo: 4 + 3 + 4 + 2 = 13 linhas -> a pagina 2 comecaria com faixa). Se for fiel quiser o estado persistente: UNKNOWN se isso foi intencional no Remix.

### 2.2 Textos exatos (VsStats.asm:50-72)

Pagina 1 - Dano: `"Damage Stats"`, `"Dealt to"`, linhas por porta `"1P".."4P"` (indent 8), `"Total Dealt"`, `"Total Taken"`, `"Highest Taken"`.
Pagina 1 - Combo (so se `Toggles.entry_combo_meter` estiver ligado, VsStats.asm:993-995): `"Combo Stats"`, `"Longest Combo VS"`, linhas `"1P".."4P"` (indent 8), `"Max Hits Taken"`, `"Max Damage Taken"`.
Pagina 2: `"Z-Cancel Stats"`, `"Success %"` (indent 0), `"Successful"` (indent 8), `"Missed"` (indent 8), `"Tech Stats"`, `"Success %"`, `"Successful"`, `"Missed"`, `"Ledge Stats"`, `"Times grabbed"`,
`"Special Move Stats"`, `"Up Specials used"`, `"Neutral Specials used"`, `"Down Specials used"` (todos indent 0 exceto onde dito).

### 2.3 Y de cada linha (cursor)

Y inicial 30 (`lli a2, 30`, VsStats.asm:973 e 1012). Cada cabecalho ou linha exibida avanca +11 (`addiu a2, a2, 11`, :476). Sublinhado: o chamador faz `a2 -= 1`, o retangulo e desenhado em y = a2 e o cursor avanca +3 (:303). Espaco entre secoes: +5 (:997, 1020, 1028, 1034).
Exemplo 4 portas ativas, combo ligado (Y_top da string; y_port = Y_top + 6.5):

Pagina 1: "Damage Stats" 30; sublinhado y 40 (w 75); "Dealt to" 43; 1P/2P/3P/4P 54, 65, 76, 87; "Total Dealt" 98; "Total Taken" 109; "Highest Taken" 120;
"Combo Stats" 136; sublinhado y 146 (w 68); "Longest Combo VS" 149; 1P..4P 160, 171, 182, 193; "Max Hits Taken" 204; "Max Damage Taken" 215.

Pagina 2: "Z-Cancel Stats" 30; sublinhado y 40 (w 80); "Success %" 43; "Successful" 54; "Missed" 65; "Tech Stats" 81; sublinhado y 91 (w 58); "Success %" 94; "Successful" 105; "Missed" 116;
"Ledge Stats" 132; sublinhado y 142 (w 65); "Times grabbed" 145; "Special Move Stats" 161; sublinhado y 171 (w 105); "Up Specials used" 174; "Neutral Specials used" 185; "Down Specials used" 196.

Para menos portas ativas os Y de depois das linhas por porta sobem 11 por porta ausente (o cursor ja trata isso no port).

### 2.4 Argumentos novos

| Item | align | X ancora | y_port | height |
|---|---|---|---|---|
| Rotulo | LEFT | 24 + indent | Y_top + 6.5 | 13.0 |
| Celula / "-" da coluna k | RIGHT | 184 + 35k | Y_top + 6.5 | 13.0 |
| "1P".. no cabecalho | RIGHT | 184 + 35k | 22.5 | 13.0 |
| Barras/retangulos | - | ver tabela 2.1 | y direto (topo, como o port ja trata `remix_hud_bar_put`) | - |

Mudanca obrigatoria: um rotulo + ate 4 celulas por linha ficam em strings separadas; pagina 1 usa ate 4 + 17 rotulos + 52 celulas = 73 strings (4 cabecalhos, 17 rotulos, 13 linhas x 4 celulas); pagina 2 usa ate 4 + 14 + 40 = 58. Precisa de ~80 slots so para esta tela.

Grupos escondidos enquanto a pagina esta ativa (asm): 0x0D (prompt Press A + sprite A), 0x16 (linhas normais dos resultados). Mostrados: 0x0E (fundo, hints, cabecalhos) e 0x1A + pagina.
O `REMIX-TODO(VsStats)` em mnremixvsstatsview.c:133 descreve exatamente isso. Evidencia: VsStats.asm:734-751 e 784-811.

---

## 3. sc1premixmultiman.c - contador de KOs - SinglePlayerModes.asm

Fonte: `SinglePlayerModes.setup_` (SinglePlayerModes.asm:2287-2331); `KO_AMOUNT_POINTER = 0x801936A0` (:112). Roda para Multi-Man e Cruel Man (ids 2 e 3); `Bonus 3`, `Remix 1P` pulam o desenho, `All Star` pula a fonte (:2292-2300). `Render.load_font()` em :2302; file 0x19 em file_pointer_1 (:2303).

| Elemento | Conteudo | X | Y_top | scale | altura | cor | align | fonte | Evidencia |
|---|---|---|---|---|---|---|---|---|---|
| Numero de KOs | so o numero inteiro (valor de 0x801936A0), SEM prefixo "KOs ", sem padding | 50.0 (0x42480000) | 18.0 (0x41900000) | 0.9 (0x3F666666) | 14.4 | 0xFFFFFFFF | LEFT | Kabel, blur 1 | SinglePlayerModes.asm:2314; routine `update_live_string_` (redesenha quando o valor muda) |
| Icone do poligono (NAO texto) | - | 30.0 (0x41F00000) | 20.0 (0x41A00000) | 1.0 | - | tint 0x848484FF, palette 0x303030FF | - | file 0x19 + 0x80 | :2312 |
| "X" (NAO texto, sprite) | - | 39.0 (0x421C0000) | 18.0 (0x41900000) | 1.0 | - | tint 0x848484FF, palette 0x303030FF | - | ponteiro de arquivo em 0x80130D50 + 0x828 (id do arquivo UNKNOWN) | :2313 |

Argumentos novos: `remix_hud_text_put(slot, "<kos>", 50.0, 25.2, 0xFFFFFFFF, 14.4)` com align LEFT (x_center = 50 + Measure/2). y_port = 18 + 7.2 = 25.2.
Hoje o port usa `"KOs %d"` em (48, 22) altura 16: o prefixo "KOs " nao existe no Remix; no lugar vem o icone + "x".

Condicao e vida: objetos no room 0x17 grupo 0x0B criados em `setup_` (inicio da partida), vivem a partida inteira. O contador para de subir depois que o jogador e nocauteado (`multiman_ko_count_stop`, SinglePlayerModes.asm:2224-2268).
Variante Dragon King HUD: se `Toggles.entry_dragon_king_hud == 1` (sempre ligado), o numero e o "X" vao para Y_top = 212.0 (0x43540000, :2320-2326) e o icone do poligono para Y = 214.0 (0x43560000, :2329-2331); y_port do numero = 212 + 7.2 = 219.2. A opcao Auto (0) NAO move nada aqui (so `bne t7, t6` com t6 = 1, :2318-2319).

---

## 4. sc1premixhrcrun.c - distancia do HRC - SinglePlayerModes.asm + HRC.asm

Fonte: SinglePlayerModes.asm:2333-2368 (`_hrc`), HRC.asm:30-47 (variaveis), 88-107 (distancia), 255-345 (`_complete`), String.asm:29-150.

| Elemento | Conteudo | X | Y_top | scale | altura | cor | align | fonte | Evidencia |
|---|---|---|---|---|---|---|---|---|---|
| Contador | `distance` com 1 decimal: `"%d.%d"` com `distance/10` e `distance%10` (itoa com 1 decimal: 0 -> "0.0", 7 -> "0.7", 123 -> "12.3"). `distance` = pes * 10 truncado (`trunc.w.s`, HRC.asm:105-107) | 278.0 (0x438B0000) | 44.0 (0x42300000) | 0.9 | 14.4 | 0xFFFFFFFF | RIGHT | Kabel, blur 1, 1 decimal | SinglePlayerModes.asm:2361 |
| Rotulo | `"FT."` | 282.0 (0x438D0000) | 47.0 (0x423C0000) | 0.625 (0x3F200000) | 10.0 | 0xFFFFFFFF | LEFT | Kabel, blur 1 | SinglePlayerModes.asm:2365, 2378 |

Sao DOIS textos, nao `"%d.%d FT."` centrado. Argumentos novos:
- contador: `put(slot_a, "12.3", 278.0, 51.2, 0xFFFFFFFF, 14.4)` com align RIGHT (x_center = 278 - Measure/2). y_port = 44 + 7.2 = 51.2.
- rotulo: `put(slot_b, "FT.", 282.0, 52.0, 0xFFFFFFFF, 10.0)` com align LEFT (x_center = 282 + Measure/2). y_port = 47 + 5.0 = 52.0.

Vida: criados em `setup_` na partida, vivem a partida inteira (mostram "0.0" ate o sandbag andar).

Animacao de "COMPLETE" (`_complete`, HRC.asm:255-345), durante `COMPLETE_TIMER_START = 45` tics (HRC.asm via SinglePlayerModes: `lli t0, HRC.COMPLETE_TIMER_START`, :2338):
- por tic, os DOIS textos (contador e rotulo): X -= dX, Y += 0.5, scale += 0.01 (0.9 -> 1.35 e 0.625 -> 1.075), blur desligado (HRC.asm:312-330).
- dX = (X_atual_do_contador - X_final) / (tics restantes + 1), calculado uma vez; `X_final = 160 + 0.725 * L - 12.5` onde `L` = largura em pixels do numero ate o fim do contador (`urx - ulx` do contador) e `12.5` = 0x41480000 (HRC.asm:287-306). Isso e uma conta aproximada do asm; a posicao final visual depende de L medido na escala inicial.
- `sc1PRemixHrcCounterAt` do port move 246 -> 160 e altura 16 -> 32; nao bate com 278 / 14.4 -> 21.6 (0.9 + 0.45 = 1.35) nem com Y += 0.5 por tic (soma 22.5 em 45 tics). Evidencia: SC1PREMIX_HRC_COUNTER_* em sc1premixhrcrules.h:16-21.

---

## 5. sc1ptrainingremix.c - Reset Count, item, legendas - Training.asm

Fonte: Training.asm `setup_` (1569-1665), `run_` (1795-2077), strings (2837-2868). `Render.load_font()` em :1573; file C5 em file_pointer_1, CSS_IMAGES em file_pointer_2 (:1574-1575).
Estados de `toggle_menu` (byte do jogo em 0x80190979): 1 = BOTH_DOWN (jogando), 2 = SSB_UP (menu de pausa do SSB), 3 = CUSTOM_UP (menu custom) (Training.asm:23-26).

### 5.1 Reset Count (grupo 0x15) - o port mostra isso DURANTE o jogo, o Remix so na pausa

| Elemento | Texto | X | Y_top | scale | altura | cor | align | Evidencia |
|---|---|---|---|---|---|---|---|---|
| Rotulo | `"Reset Count:"` (Training.asm:2848) | 99.5 (0x42C70000) | 25.0 (0x41C80000) | 1.0 | 16.0 | 0xFFFFFFFF | LEFT | Training.asm:1617 (blur FALSE) |
| Valor | inteiro `reset_counter` (Training.asm:1438), sem padding | 221.0 (0x435D0000) | 25.0 | 1.0 | 16.0 | 0xFFFFFFFF | RIGHT | Training.asm:1618 (NOOP: criado uma vez por carga do cenario) |

Argumentos novos: `put(s, "Reset Count:", 99.5, 33.0, 0xFFFFFFFF, 16.0)` LEFT e `put(s, "<n>", 221.0, 33.0, 0xFFFFFFFF, 16.0)` RIGHT (y_port = 25 + 8 = 33).
Condicao: grupo 0x15 comeca escondido (:1751-1753); `run_` o mostra em `_ssb_up` (:1920-1925) e o esconde em `_both_down` (:2053-2057). Fica visivel com SSB_UP e CUSTOM_UP; some quando `toggle_menu == BOTH_DOWN`.
Hoje o port o mostra com `game_status == Go` (jogando) em (72, 64) altura 12: errado em posicao, tamanho, texto (um unico "Reset Count: %d") e condicao.

### 5.2 Press Z for Custom Menu (grupo 0x0E) - FALTA no port

| Elemento | Texto | X | Y_top | scale | altura | cor | align | Evidencia |
|---|---|---|---|---|---|---|---|---|
| Prompt | `"Press    for Custom Menu"` (4 espacos, Training.asm:2852) | 160.0 (0x43200000) | 50.0 (0x42480000) | 1.0 | 16.0 | 0xFFFFFFFF | CENTER | Training.asm:1583 (blur FALSE) |
| Sprite Z (NAO texto) | - | 117.5 (0x42EB0000) | 49.0 (0x42440000) | 1.0 | - | tint 0x848484FF, palette 0x303030FF | - | file C5 + 0x0BD8, :1584 |

Argumentos: `put(s, "Press    for Custom Menu", 160.0, 58.0, 0xFFFFFFFF, 16.0)` CENTER.
Condicao: grupo 0x0E escondido em `setup_` (:1747-1749); mostrado quando o menu SSB esta de pe e escondido com o menu custom aberto. A reabertura esta em `_close_custom_menu` (:1904-1918, liga 0x0E) e a ocultacao em :1946-1949 (liga o menu custom). Quem liga 0x0E na PRIMEIRA pausa nao esta em Training.asm: a inferencia e o hook `Pause.disable_hud_` (Pause.asm:81-98), que a cada pausa seta o display dos grupos 0x0E, 0x0F e 0x10 para `Toggles.entry_disable_hud > 0` (visivel quando Disable HUD = OFF) - ver docs/remix/notes/f10_menu_pausegroups.md:11-19. INFERIDO, nao verificado em jogo.

### 5.3 Legenda do botao L (grupo 0x15) e legenda de paginacao (grupo 0x16) - FALTAM no port

| Elemento | Texto | X | Y_top | scale | altura | cor | align | Evidencia |
|---|---|---|---|---|---|---|---|---|
| Sprite L (NAO texto) | - | 20.0 (0x41A00000) | 16.0 (0x41800000) | 0.875 | - | tint 0x848484FF, palette 0x303030FF | - | file C5 + 0x18C8, :1635 |
| `"Show"` | Training.asm:2862 | 20.0 | 27.25 (0x41DA0000) | 0.875 | 14.0 | 0xFFFFFFFF | LEFT | :1636 |
| `"Action"` | Training.asm:2863 | 20.0 | 38.5 (0x421A0000) | 0.875 | 14.0 | 0xFFFFFFFF | LEFT | :1637 |
| Sprite R (NAO texto) | - | 272.0 (0x43880000) | 16.0 | 0.875 | - | tint 0x848484FF, palette 0x303030FF | - | file C5 + 0x0CF8, :1640 |
| `"/"` (= `pagination_legend_1 + 4`) | - | 288.0 (0x43900000) | 16.0 | 0.875 | 14.0 | 0xFFFFFFFF | LEFT | :1641 |
| Sprite Z (NAO texto) | - | 294.0 (0x43930000) | 15.0 (0x41700000) | 0.875 | - | tint 0x848484FF, palette 0x303030FF | - | file C5 + 0x0BD8, :1642 |
| `"Next/"` | Training.asm:2867 | 272.0 | 27.25 | 0.875 | 14.0 | 0xFFFFFFFF | LEFT | :1643 |
| `"Prev"` | Training.asm:2868 | 272.0 | 38.5 | 0.875 | 14.0 | 0xFFFFFFFF | LEFT | :1644 |

y_port (altura 14 -> meia altura 7): "Show" 34.25, "Action" 45.5, "/" 23.0, "Next/" 34.25, "Prev" 45.5.
Condicao: "Show Action" (grupo 0x15) = visivel na pausa (ver 5.1); apertar L alterna o grupo 0x17 (as strings de acao/frame por porta), nao este legend (Training.asm:1972-1985).
Paginacao (grupo 0x16): visivel so com `CUSTOM_UP` (liga em :1943-1945, desliga em :1904-1908). O fundo translucido do menu custom (retangulo 66, 45, 189 x 154, 0x0064FF64 com alpha) e o quadro `draw_modal_frame_` sao do grupo 0x16 e nao sao texto (:1647-1648).

### 5.4 Legenda do D-pad (grupos 0x19 e 0x1A) - FALTA no port

Todas com scale 1.0 (altura 16), cor 0xFFFFFFFF, LEFT, blur FALSE. Texturas D-pad: file CSS_IMAGES (0xA06) + 0x218, tint 0x848484FF, palette 0x303030FF, scale 1.0 (:1621-1624). Quadradinhos amarelos 2x2 `Color.high.YELLOW` = 0xFFFF00FF (Color.asm:41).

| Texto | String | X | Y_top | y_port | Grupo | Evidencia |
|---|---|---|---|---|---|---|
| Toggle Pause | `"Toggle Pause"` (:2837) | 63.25 (0x427D0000) | 202.0 (0x434A0000) | 210.0 | 0x19 | :1629 |
| Quick Reset | `"Quick Reset"` (:2840) | 63.25 | 217.0 (0x43590000) | 225.0 | 0x1A | :1630 |
| Frame Advance | `"Frame Advance"` (:2838) | 178.0 (0x43320000) | 202.0 | 210.0 | 0x19 | :1631 |
| Model Display | `"Model Display"` (:2839) | 178.0 | 217.0 | 225.0 | 0x19 | :1632 |
| Sprites D-pad (nao texto) | - | (45.25, 200.0)[0x19], (45.25, 215.0)[0x1A], (160.0, 200.0)[0x19], (160.0, 215.0)[0x19] | | | | :1621-1624 |
| Quadrados amarelos 2x2 (nao texto) | - | (52, 203)[0x19], (48, 222)[0x1A], (171, 207)[0x19], (167, 226)[0x19] | | | | :1625-1628 |

Condicao (o valor e `entry_dpad_menu` = "D-pad Controls", 0 = ligado, 1 = so reset, 2 = desligado; Training.asm:3908, 2988-2991): grupo 0x19 visivel so se valor == 0; grupo 0x1A visivel se valor for 0 ou 1 (Training.asm:1956-1966). Os dois so aparecem com SSB_UP/CUSTOM_UP e sao escondidos em `_both_down` (:2053-2063).

### 5.5 Hold A para sair (grupo 0x0F) - FALTA no port

`"(Hold --)"` (Training.asm:2844), LEFT, X 169.0 (0x43290000), Y_top 166.0 (0x43260000), scale 0.875, altura 14.0, y_port 173.0 (:1597). Sprite A em (202.0, 165.0) scale 0.9375, tint 0x50A8FFFF, palette 0x303030FF, file C5 + 0x0958 (:1598). Barra de progresso: retangulo (73, 179), altura 2, cor 0x0088FFFF, largura cresce 2 por tic ate 64 + 2 (:1603-1613, 2020-2027). So existe se `Toggles.entry_hold_to_exit_training` estiver ligado (:1592-1595) e so e visivel com o cursor do menu em EXIT (indice 5) e SSB_UP (:1995-2012).

### 5.6 Nome do item segurado/selecionado - NAO e texto no Remix

O nome do item de treino customizado e uma IMAGEM (nao o Kabel): menu de item = tabela de imagens em file 0xFE + 0x02A0 (`TRAINING_MENU_IMAGE_TABLE`), HUD do item segurado = file 0xFE + 0x0330 (`TRAINING_HUD_IMAGE_TABLE`), imagens anexadas ao file 0x1D (Item.asm:22-23, 262-365, comentarios em :298-301 e :357-360). O conteudo exato de cada imagem e UNKNOWN (vem de arquivos binarios, nao do `src`). Posicao no menu: a sprite do option do menu de item (vanilla usa pos.x = 191 - w/2; o decomp usa y 85, sc1ptrainingmode.c:1469-1473). Portanto a tupla `(191, 90, 12.0)` do port e so um substituto; nao ha equivalente Kabel. Se mantido como texto: UNKNOWN (sem parametros do Remix).

---

## 6. gmremixgameendrows.c - linhas Goals/Time - VsRemixMenu.asm (NAO e GameEnd.asm)

O GameEnd.asm nao tem nada disso. A linha extra e `add_row_to_results_` em VsRemixMenu.asm:1007-1300. Importante: NAO e texto Kabel.

| Elemento | Conteudo | X | Y | Evidencia |
|---|---|---|---|---|
| Rotulo KOTH | imagem `"Times"` (offset 0xE648) | 26.0 (vanilla: `pos.x = 26`, mnvsresults.c:2069) | 124 | VsRemixMenu.asm:1106-1122 (`_draw_alt_texture`); linha criada por `jal 0x8013569C` com `a0 = 124` (:1074-1075, `_delay_place` em :1056) |
| Rotulo Smashketball | imagem `"FGs"` (offset 0xE9E0) | 26.0 | 124 | VsRemixMenu.asm:1112-1119 |
| Valores KOTH | tempo `M:SS`: segundos em colX, minutos em colX - 20.0 (0x41A00000), dois pontos (sprite file 0x24 + 0x08D8) em colX - 20 + 31 = colX + 11; zero das dezenas forcado | colX da porta (`mnResultsGetColumnX`) | 124 | VsRemixMenu.asm:1125-1213 (`_draw_times`), `_show_seconds_tens_digit_` logo apos `_draw_goals` |
| Valores Smashketball | gols (limite 999, `slti 0x03E8`) em colX | colX | 124 | VsRemixMenu.asm:1217-1232 (`_draw_goals`) |

Os numeros usam os sprites numericos do proprio resultado (`mnResultsDrawNumber` com a cor da porta, `mnResultsGetNumberColorIndex`), nao a fonte Kabel e nao branco fixo.
O "Place" desce de 124 para 140 (`addiu a0, a0, 16`, `_move_place`, :1091-1103). Atraso de 20 tics (`addiu at, at, 20`, :1066). Sem contest: linha em y 96 no tic 100 (docs/remix/notes/f14_results_rows.md).
Portanto: o texto `"Time"` / `"Goals"` e os valores do port nao tem equivalente Kabel. Para fidelidade usar o sprite de label (offsets 0xE648 e 0xE9E0 do mesmo arquivo do "KOs", que o port ja carrega em `sMNVSResultsFiles[0]`, arquivo exato UNKNOWN) e `mnVSResultsMakeNumber`. Se ficar texto (aproximacao): nao ha parametros do Remix; qualquer tupla seria UNKNOWN.

---

## 7. ifremixpauselegend.c - legendas de pausa - Pause.asm

Fonte: Pause.asm:164-191 (`show_legend_helper_`), 235-362 (`check_legend_toggle_press_`), 597-602 (strings), Toggles.asm:1127 (`"/"`). `Render.load_font()` em :180; room 0x18.

### 7.1 Legenda "R" (grupo 0x0E) - NAO e texto

| Elemento | X | Y | scale | cor | Evidencia |
|---|---|---|---|---|---|
| Sprite R | 226.0 (0x43620000) | 203.0 (0x434B0000) | 1.0 | tint 0x808080FF, palette 0x212121FF; file C5 + 0x0CF8 (arquivo em 0x80130D54) | Pause.asm:181 |
| Imagem da palavra "Legend" | 243.0 (0x43730000) | 205.0 (0x434D0000) | 1.0 | tint 0xFFFFFFFF, palette 0; file C5 + 0x19F8 | Pause.asm:182 |

Nao ha string no Remix: `"R: Legend"` do port e um substituto. Se mantido como texto: UNKNOWN (sem parametros Kabel). Visivel quando `entry_disable_hud == 0` (Pause.asm:87-98) e nao escondido com D-pad Left (:195-231).

### 7.2 Legenda de camera (grupo 0x10) - texto Kabel, scale 1.0 (FONTSIZE_DEFAULT), altura 16, cor 0xFFFFFFFF, LEFT, blur 1

| Linha | String | X | Y_top | y_port | Sprites que a acompanham (nao texto) | Evidencia |
|---|---|---|---|---|---|---|
| Zoom | `": Zoom Camera"` (:600) | 72.0 (0x42900000) | 140.0 (0x430C0000) | 148.0 | A (40.0, 140.0) tint 0x50A8FFFF/pal 0x303030FF scale 0.9375; `"/"` (52.0, 140.0); B (58.0, 140.0) tint 0x00D040FF/pal 0x003000FF scale 0.9375 | :321-324 |
| Pan | `": Pan Camera"` (:601) | 94.0 (0x42BC0000) | 154.0 (0x431A0000) | 162.0 | quatro setas CSS_IMAGES (cor 0xC0CC00FF/pal 0x000000FF, scale 0.75) em y 156.0: 0x1958 em x 40, 0x0688 em x 54, 0x1A88 em x 68, 0x1BB8 em x 82; `"/"` em x 50.0, 64.0, 78.0 (Y_top 154.0, y_port 162.0) | :326-333 |
| Field of View | `": Field of View"` (:602) | 94.0 | 170.0 (0x432A0000) | 178.0 | Z (41.0, 170.0) scale 1.0 tint 0x848484FF/pal 0x303030FF; "+" C5 PLUS 0x04D8 (53.0, 173.0) scale 0.9375 tint 0xFFFFFFFF; A (62.0, 170.0) scale 0.9375; `"/"` (74.0, 170.0); B (80.0, 170.0) scale 0.9375 | :335-340 |

Textos: o `"A/B: Zoom Camera"`, `"D-Pad: Pan Camera"`, `"Z+A/B: Field of View"` do port juntam o glifo e o texto; no Remix as strings Kabel sao apenas `": Zoom Camera"` etc. e `"/"`. Altura 8 do port deve ser 16.

### 7.3 Legenda de musica (grupo 0x0F, "nao portada" no cabecalho do arquivo)

Criada na PRIMEIRA R solo (Pause.asm:269-343): so ai nascem 0x0F e 0x10, ambos mostrados (:342-343, e :350-357 esconde so a camera se o byte 0x80131828 != 1).
- Nota musical: string `0x80` (glifo da nota, :597) LEFT em (40.0, 74.0), scale 1.0 (:279).
- Titulo da faixa: LEFT em (52.0, 74.0), scale 1.0, largura maxima 228 (0x00E4; se mais larga, `apply_max_width_` achata so o eixo X, Render.asm:1783-1839), atualizado por `render_live_music_string_` (:295-301, 370-405).
- Titulo do jogo: LEFT em (52.0, 90.0), scale 0.75 (0x3F400000), largura maxima 228 (:305-312).
- `": Next Music Track"` LEFT (60.0, 106.0) scale 1.0 (:316); `": Random Music Track"` LEFT (60.0, 122.0) scale 1.0 (:319). Sprites D-pad CSS_IMAGES + 0x218 em (40.0, 105.0) e (40.0, 121.0), quadrados amarelos 2x2 em (51, 113) e (47, 132) (:314-318).
y_port: nota 82.0, titulo 82.0, jogo 96.0 (altura 12), Next 114.0, Random 130.0.

Comportamento que difere do port: no Remix a primeira R solo mostra musica E camera juntas (0x0F e 0x10 criados visiveis); o port comeca a camera escondida (`sIFRemixPauseLegendCameraShown = FALSE`) e so alterna a camera. Alem disso a R legend fica escondida e o 0x10 sempre escondido se o byte 0x80131828 != 1 (camera nao permitida) (Pause.asm:350-357).

---

## 8. gmremixkothhud.c - barras de KOTH - KingOfTheHill.asm 1585-1700

NAO ha texto Kabel em KingOfTheHill.asm (nenhum `draw_string` / `draw_number`). Os medidores sao sprites + retangulos (room 0x17, grupo 0x0B). Divergencias do port:

| Elemento | Valor | Evidencia |
|---|---|---|
| Moldura da barra (textura, NAO texto) | file KING_OF_THE_HILL_GFX (0x14F3, File.asm:3244, em `Render.file_pointer_4`, KingOfTheHill.asm:1533) + 0x0AA8; ulx = 40 + 70 * PORTA - 3 (NAO por slot de barra), uly = 180.0 (0x43340000); scale 1.0; tint = cor da barra, palette 0x000000FF ("shadow BLACK") | KingOfTheHill.asm:1560-1561, 1606-1635 |
| Barra de progresso (retangulo) | ulx = 40 + 70 * PORTA, uly (inteiro) = 188, altura 5, largura 0..40 = `trunc(score/alvo * 40)` por `update_indicators_`, cor da barra com alpha (`OS.TRUE`) | KingOfTheHill.asm:1566-1568, 1645-1668, 1434-1456 |
| Flash (retangulo branco) | mesmo x, uly 188, largura 40, altura 5, cor 0xFFFFFF00 com alpha; vira ativo quando a largura chega a 40; alpha vai de 0 a 100 em passos de 5 (`ALPHA_DELTA = 5`, `ALPHA_DELTA_MAX = 100`) e volta, por tic | KingOfTheHill.asm:13-14, 1672-1685, 1458-1480 |
| Variante Dragon King HUD | Y da moldura = 8.0 (0x41000000), Y da barra/flash = 16: com `entry_dragon_king_hud == 1` (sempre) ou em stage Dragon King / Dragon King Remix com o toggle em Auto; com o toggle = 2 (desligado) usa 180/188 | KingOfTheHill.asm:1560-1581 |
| Cores | FFA `colors_bar` = C96A6AFF, 7E7EC9FF, B2B244FF, 4EAD4EFF, cinza 888888FF (CPU); times `team_colors_bar` = C96A6AFF, 7E7EC9FF, 4EAD4EFF, B2B244FF; indice = TIME do jogador (`lbu 0x000C(at)`); em FFA o CPU usa a cor 4 (`use_cpu_color`) | KingOfTheHill.asm:112-126, 1595-1610 (port ja usa essas tabelas) |

Diferencas do port: (1) `y` da barra e 180 (o asm desenha barra e flash em 188, a moldura em 180); (2) X e `40 + 70 * slot`, mas o asm usa `40 + 70 * porta` (porta 3 ativa sozinha cai em x = 110, nao 40); (3) falta a moldura (textura) e o contorno branco/fundo preto do `DrawBar` nao existem no Remix; (4) o flash do port soma `flash` sobre toda a barra incluindo a moldura, no asm so cobre os 40x5 do retangulo (nao a moldura). Nada disto exige `remix_hud_text_put`.

---

## 9. Banner do titulo da musica - BGM.asm:1001-1160 (port: port/remix/remix_music_title.cpp)

Hoje o port usa `remix_overlay_show_text("<faixa> - <jogo>", 4s)` (toast de GameOverlay), uma linha sem nota.
Remix: dois textos Kabel + retangulo, no room 0x17 grupo 0x0B (`setup_` carrega `Render.load_font()` so se `Toggles.entry_show_music_title` != 0, BGM.asm:981-995).

| Elemento | Conteudo | X | Y_top | scale | altura | cor | align | Evidencia |
|---|---|---|---|---|---|---|---|---|
| Fundo (retangulo, nao texto) | - | 1, w 320 | 156, h 28 | - | - | 0x34343494, alpha | - | BGM.asm:1032 (`Render.draw_rectangle(0x17, 0xB, 1, 156, 320, 28, 0x34343494, OS.TRUE)`) |
| Linha da faixa | `"\x80 " + <faixa> + " \x80"` (bytes 0x80, 0x20, titulo, 0x20, 0x80; 0x80 e o glifo da nota musical, suportado: `REMIX_FONT_LAST_CHARACTER 0x80`) | 160.0 (0x43200000) | 158.0 (0x431E0000) | 1.0 | 16.0 | 0xFFFFFFFF | CENTER | BGM.asm:1036-1065, 1068-1080 (blur 1) |
| Linha do jogo | `<jogo>` (ponteiro 0 da string_table) | 160.0 | 172.0 (0x432C0000) | 0.75 (0x3F400000) | 12.0 | 0xFFFFFFFF | CENTER | BGM.asm:1086-1103 (blur 1) |

y_port: faixa 158 + 8 = 166.0 (altura 16); jogo 172 + 6 = 178.0 (altura 12). Argumentos: `put(s1, "\x80 <faixa> \x80", 160.0, 166.0, 0xFFFFFFFF, 16.0)` CENTER e `put(s2, "<jogo>", 160.0, 178.0, 0xFFFFFFFF, 12.0)` CENTER. Isso exige mover o banner de `remix_overlay_show_text` para `remix_hud_text_put` (e o retangulo para `remix_hud_bar_put`, que hoje faz fundo preto 160 + contorno branco; nao ha como pedir a cor 0x34343494 sem borda).
Condicao: criado no hook em 0x8011280C (criacao das luzes de GO, `BGM.asm:1004-1006`), destruido no hook em 0x80112100 (criacao do texto "GO!", `_destroy`, :1009-1011, 1127-1152); exige `show_music_title` e `play_music` ligados. A duracao exata em tics UNKNOWN (depende da contagem regressiva do jogo). O port nao tem o hook de destruicao (nota em f10_menu_musictitle.md) e usa 4 s fixos.
IDs sem titulo (menus, MIDIs extras) mostram nada, como no asm (string_table entry 0).

---

## 10. Resumo de lacunas no port (por prioridade de fidelidade)

1. `remix_hud_text_put` precisa de parametro de alinhamento (LEFT/RIGHT/CENTER) e de mais slots (>= 128).
2. Varios elementos "de texto" do port sao imagens ou sprites no Remix: `R: Legend`, nome do item de treino, rotulos "Time"/"Goals" dos resultados, glifos de botao (A, B, R, L, Z, +, D-pad), icone/"X" do multiman, moldura da barra de KOTH, icones de stock dos cabecalhos de stats.
3. Condicoes de vida erradas no port: Reset Count aparece jogando (no Remix so na pausa); camera da pausa comeca escondida (no Remix musica + camera aparecem juntas na primeira R).
4. Textos faltando: "Press Z for Custom Menu", legendas D-pad e L/R do treino, "(Hold --)", legenda de musica da pausa.
5. HRC e Multiman: o texto do port junta o que o Remix separa (numero + "FT."; so o numero sem "KOs").
