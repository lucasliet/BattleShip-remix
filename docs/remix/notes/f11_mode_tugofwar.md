# F11 modo Tug of War: nota de pesquisa para o port (M6)

Fonte principal: `.toolchain/pkgs/smashremix-src/src/TugOfWar.asm` (545 linhas, leitura integral). Contexto: `docs/remix/menus.md` 3.2, `docs/remix/notes/f10_menu_vsmodes.md`, `docs/remix/gameplay_m_z.md:1384-1395`, `docs/remix/file_table_and_loader.md:3526`. Sem build, sem execução, sem alteração de código.

Nota sobre ai-memory: o escopo do projeto não foi resolvido. `memory_briefing` com `workspace: "default"` e `project: "BattleShip"` retornou "not found", com `JRickey/BattleShip` e `JRickey`/`BattleShip` também sem resultado, e não há `.ai-memory.toml` na raiz. Foram lidas só as três páginas `_rules/` do escopo `_global`. As regras do projeto não foram carregadas.

## 1. O que é o modo

**Regras (asm).** Tug of War é uma partida de stocks de VS em que cada KO também dá um stock extra ao atacante: o autor do KO (KOer) ganha +1 no contador do fighter e +1 em `players[].stock_count` (TugOfWar.asm:318-328 e 399-406). O alvo perde o stock pela regra vanilla de morte. Quando o KOer estava eliminado e recebe o stock, ele é revivido (`ftCommonRebirthDownSetStatus`, TugOfWar.asm:333-338). Não há timer neste arquivo; o fim da partida segue a regra vanilla de stocks.

**Pontuação do resultado** (`TimedStock.asm`, `score_fix_`, ramo `_tug_of_war`, TimedStock.asm:97-111 e saída pelo `_add_damage_to_score`): `score = stocks_restantes << 24 + (KOs - mortes) << 12 - dano_final`. Os stocks restantes vêm de `Global.vs.px + 0x0B` (ending stocks, base 0) mais 1.

**Crédito de KO** (TugOfWar.asm:282-359, 364-428):
- Se o KOer está em time diferente do alvo, ele recebe o crédito.
- Se o KOer é do mesmo time, o crédito vai para o último atacante do alvo (ver risco R2 sobre o índice).
- Sem KOer (autodestruição, dano ambiental), o crédito vai para quem bateu por último no alvo. Se ninguém bateu, nada é creditado (TODO no asm).
- Em 1v1 (`is_1v1`), o último atacante não é limpo entre stocks.

**Seleção do modo:** `VsRemixMenu.vs_mode_flag == VsRemixMenu.mode.TUG_OF_WAR` (= 5, VsRemixMenu.asm:18). O valor é gravado pelo botão "Tug of War" da página 2 do menu VS (VsRemixMenu.asm:97, ver f10_menu_vsmodes.md).

**Flags e globais que o modo escreve:**
- `Global.vs.game_mode` = 3 (TugOfWar.asm:59-63, 193-195). O comentário do asm diz "TIMED STOCK", mas TimedStock.asm:65 diz "1 = time, 3 = stock". O valor 3 é stock. O comentário está errado.
- `Global.vs.stocks` = 3 (`TugOfWar.stocks`, TugOfWar.asm:13-14).
- `Global.vs.teams` = `TugOfWar.teams` (FALSE por padrão, TugOfWar.asm:18-19), lido do CSS.
- `Global.match_info`: não é referenciado em TugOfWar.asm.
- Flag de single player: não é usado. `is_1v1` é derivado da contagem de jogadores em `setup_` (TugOfWar.asm:223-271).

## 2. Fluxo

Fluxo: menu VS (página 2) -> CSS -> stage select -> partida -> resultados.

Ganchos do modo, no arquivo e nos arquivos de vanilla (todos os endereços de RAM vêm do asm):

| asm file:line | patched RAM/ROM address | o que faz | função decomp correspondente |
|---|---|---|---|
| TugOfWar.asm:283-286 | ROM 0xB6884, RAM 0x8013BE44 | `credit_stock_to_koer_`, crédito ao KOer | `ft/ftcommon/ftcommondead.c` `ftCommonDeadUpdateScore` (0x8013BD64, +0xE0) |
| TugOfWar.asm:365-368 | ROM 0xB68C4, RAM 0x8013BE84 | `credit_stock_when_no_koer_` | mesma função (+0x120) |
| TugOfWar.asm:434-437 | ROM 0xBBF74, RAM 0x80141534 | `update_last_hit_by_._damage` | no match found (gameplay_m_z.md:1394 marca como unmatched; não reverificado) |
| TugOfWar.asm:439-442 | ROM 0xC3FC4, RAM 0x80149584 | `update_last_hit_by_._shield` | `ft/ftcommon/ftcommonshieldbreakfly.c` `ftCommonShieldBreakFlyCommonSetStatus` (0x80149510, +0x74) |
| Render.asm:2303 | chamada, sem patch | `TugOfWar.setup_`, a cada início de partida | nenhuma função decomp identificada nesta leitura |
| VsRemixMenu.asm:447-449 | rotina | teams do Tug para sincronizar custom de time | `mn/mnvsmode/mnremixvsmode.c` (não ligado ainda) |
| VsRemixMenu.asm:540 | tabela `mode_setup_table` | `before_css_setup_` | não identificado |
| VsRemixMenu.asm:622 | tabela `mode_setup_returning_table` | `leave_css_setup_` | não identificado |
| VsRemixMenu.asm:679 | tabela `mode_setup_start_table` | `start_match_setup_` | não identificado |
| StockMode.asm:92 | patch `set_initial_stock_count_` ROM 0x10A3A4, RAM 0x8018D4B4 | Tug usa a contagem anterior mesmo se for maior que o padrão | `gm/gmremixstockmode.c` `gmRemixStockModeCount` (não trata o ramo Tug) |
| TimedStock.asm:72 e :97 | patch `score_fix_` ROM 0x155DA8, RAM 0x80136C08 | ramo de pontuação do Tug | não identificado |
| CharacterSelect.asm:5640-5643 | rotina do CSS | Tug permite contagem acima do `stocks` | não identificado |
| css/StockMode.asm:41 | `onchange_handler` | não reseta a opção Stock para Default no Tug | não identificado |
| FGM.asm:2017 | constante `TUG_OF_WAR(479)` | voz de anúncio do CSS | não identificado |
| TwelveCharBattle.asm:1774-1776, 1820-1822 | ramos de anúncio | "Tug of War" como voz | não lidos a fundo nesta passada |
| TwelveCharBattle.asm:1889, 1919, 1977, 2032 | comparações de modo | não lidos nesta passada | - |

Endereço de ponteiro de título: `TugOfWar.asm:77-80` escreve `0x30F0` em `0x8013B7C8` e `0x8013B7CC` (ffa e teams). O arquivo de imagem não é identificado no asm.

Ordem no CSS e no início da partida:
1. `before_css_setup_` (TugOfWar.asm:55-111): força `game_mode = 3`, `stocks = 3`, `teams`, título, e a tabela `stockmode_table` (todo zero). Lê `Global.previous_screen`. Só restaura `previous_stock_count_table` quando a tela anterior é `VS_GAME_MODE_MENU`.
2. `leave_css_setup_` (TugOfWar.asm:115-175): salva o estado do Tug e restaura os globais do menu VS (`VsRemixMenu.global_*`).
3. `start_match_setup_` (TugOfWar.asm:179-219): salva stocks e teams do CSS, força `game_mode = 3`, salva as tabelas.
4. `setup_` (TugOfWar.asm:223-277), chamado de Render.asm:2303, a cada início de partida: zera `last_hit_by_port` e `team_by_port`, zera `shield_player` em cada jogador, conta jogadores e grava `is_1v1` e as portas de 1v1.

Ordem de resultados: `score_fix_` (TimedStock.asm) acima.

## 3. Estado e dados

Globais do modo (todos em `TugOfWar` no asm):

| nome | tamanho | quem escreve | observação |
|---|---|---|---|
| `stocks` | dw (4 bytes) | `before_css_setup_` lê; `leave_css_setup_` e `start_match_setup_` salvam do CSS | padrão 3 |
| `teams` | dw | igual a `stocks` | padrão FALSE |
| `stockmode_table` | 4 x dw | salvo e restaurado a partir de `StockMode.stockmode_table` | padrão 0 |
| `stock_count_table` | 4 x db | salvo a partir de `StockMode.stock_count_table` | padrão -1 |
| `previous_stock_count_table` | 4 x db | salvo a partir de `StockMode.previous_stock_count_table` | padrão -1 |
| `last_hit_by_port` | 4 x db | `setup_`, `update_last_hit_by_` (damage e shield), `credit_*` (limpa) | -1 = ninguém |
| `team_by_port` | 4 x db | `setup_` | time por porta |
| `is_1v1` | dw | `setup_` | FALSE se mais de 2 jogadores |

Globais vanilla lidos ou escritos (endereços do asm):
- `0x8013BD80` (stocks, CSS), `0x8013BDA8` (teams, CSS), `0x8013BDAC` (game_mode), `0x8013B7C8` (ponteiros de título).
- `0x801317F4` = `sIFCommonBattlePlace` (TugOfWar.asm:534). A decomp declara `sIFCommonBattlePlace` em `decomp/src/if/ifcommon.c:473`. A variável bate pelo nome e pelo endereço comentado.
- `0x80136C08` e `0x8018D4B4`: não são globais, são os endereços dos patches (ver seção 2).
- `0x800466FC`: ponteiro para o primeiro objeto de jogador (TugOfWar.asm:237-238, 510-511).

Tabelas e strings: nenhuma tabela de texto é criada aqui. O texto do botão do menu fica no arquivo VS (`VsRemixMenu.asm:97`, offset `0x93B8`). O título usa o offset `0x30F0` do arquivo de imagem (não identificado).

Arquivos Remix-only ou ids de arquivo: o asm não carrega arquivos próprios. A voz de anúncio é `FGM 479` (FGM.asm:2017). O arquivo de título é desconhecido.

Offsets de struct usados pelo asm (ver risco R3 sobre a conferência):
- FTStruct (fighter): `0x04` gobj, `0x0C` team, `0x0D` porta, `0x14` stock_count. Conferido contra `decomp/src/ft/fttypes.h:1419-1437`.
- Entrada de jogador em `a2 + porta * 0x74`, com `0x2B` como stocks restantes. Não conferido: `SCPlayerData` em `decomp/src/sc/sctypes.h:375-400` tem `stock_count` em `0x0B` e tamanho diferente.

## 4. Lógica em partida

Ordem por evento:

1. **Início da partida** (`setup_`, Render.asm:2303): zera `last_hit_by_port` e `team_by_port` (valores -1 e time). Zera `shield_player` de cada jogador (correção de bug vanilla). Conta jogadores. Se são 2, marca `is_1v1 = TRUE` e cruza as portas: cada um tem o outro como último a acertá-lo, para que o primeiro KO já conte.
2. **Dano** (`update_last_hit_by_._damage`, TugOfWar.asm:444-464): se Tug, não é 1v1, e o atacante é de outro time, grava a porta do atacante em `last_hit_by_port[porta do alvo]`. Porta inválida (>= 4) é ignorada.
3. **Quebra de escudo** (`update_last_hit_by_._shield`, TugOfWar.asm:473-496): se Tug, não é 1v1, e o `shield_player` é porta válida, grava a porta do atacante. Ver bug R1 sobre a comparação de time.
4. **Morte** (`ftCommonDeadUpdateScore` vanilla, decomp ftcommondead.c:53-102): o alvo perde o stock. Depois, o hook tenta creditar o KOer (TugOfWar.asm:282-359 ou 364-428).
5. **Crédito ao KOer:** +1 em `FTStruct.stock_count` e +1 no contador de stocks do jogador. Se o contador passa a 0 (o jogador estava eliminado), chama `ftCommonRebirthDownSetStatus` no `gobj` do KOer e depois `update_sIFCommonBattlePlace_` (TugOfWar.asm:330-338). Se não é 1v1, limpa `last_hit_by_port[alvo]` (TugOfWar.asm:340-347).
6. **Sem KOer:** o mesmo crédito, sem o caso de time. Em não-1v1, limpa `last_hit_by_port[alvo]` antes do crédito (TugOfWar.asm:377-381).
7. **Revive e lugar** (`update_sIFCommonBattlePlace_`, TugOfWar.asm:505-542): em modo de times, se ainda há um companheiro com stock, não mexe no lugar. Se o time foi eliminado, incrementa `sIFCommonBattlePlace`.
8. **Câmera, itens, stage**: não há lógica nesta arquivo. Não há hook de câmera nem de item. O mesmo vale para troca de stage.

Verificação de crédito: no caminho de time igual, o código indexa `last_hit_by_port` por `t2` (time do alvo) e não pela porta (TugOfWar.asm:296-303). Ver R2.

## 5. HUD e texto

- Contador de stocks no CSS: `CharacterSelect.asm:5640-5643` permite contagem acima do `stocks` no Tug. O título do CSS aparece com o `0x30F0`.
- Contador de stocks em partida: o asm não altera o HUD. O HUD vanilla mostra `players[].stock_count`. Não sei se o HUD vanilla tolera valores acima de 4 ícones ou acima de 3 em `stocks`. Verificar no jogo.
- Texto do menu VS: "Tug of War" na página 2 (VsRemixMenu.asm:97). A nota f10 registra que a porta lê o texto como `Sprite`, não como textura Remix. Risco já registrado.
- Anúncio de voz: `FGM 479` no CSS (FGM.asm:2017) e nos ramos de TwelveCharBattle.asm:1774-1776 e 1820-1822. Não é texto. A porta precisa tocar o FGM.
- Texto de placar: não há texto novo. A ordem de resultados passa pelo `score_fix_` (seção 1).

O port desenha texto só pelo overlay ImGui (`port/remix/remix_overlay.h`) e por sprites vanilla. Nenhum elemento de HUD deste modo precisa de fonte nova, salvo confirmação do título do CSS.

## 6. Dependências

- **StockMode** (`StockMode.asm`, `gm/gmremixstockmode.c`): o `set_initial_stock_count_` (StockMode.asm:88-121) usa a contagem anterior no Tug mesmo quando ela é maior que o padrão. O port `gmRemixStockModeCount` cobre apenas o caminho manual e o `Default`. O caminho Last ainda não existe no port (REMIX-TODO no próprio arquivo). Quando o Last for portado, o Tug precisa da exceção.
- **TimedStock** (`TimedStock.asm`): o ramo de pontuação do Tug é independente da pontuação de tempo. Os modos KOTH e Smashketball usam o mesmo `score_fix_`. Não há decomp equivalente identificado.
- **Stamina**: não usado pelo Tug.
- **Neutral Spawns**: não usado.
- **Resultados** (`mn/mnvsmode/mnvsresults.c:2856`): compara `game_rules == SCBATTLE_GAMERULE_STOCK`. O Tug ajusta o game_rules pelo modo do VS. Não verificado se o resultado do Tug usa o ramo de stock.
- **Outros modos**: o Tug compartilha `VsRemixMenu.global_*`, o `StockMode` e o `TimedStock`. O 12CB (`TwelveCharBattle.asm`) também cita o Tug nos anúncios. Mexer em `TwelveCharBattle.asm` afeta o Tug.
- **Menu VS** (`mn/mnvsmode/mnremixvsmode.c`): o estado `gMNVSModeRemixMode` (5 = Tug) é gravado, mas nada lê ainda. Esse é o ponto de entrada de todo o M6.

## 7. Plano de port

Cada passo é para um worker Haiku em uma sessão: um arquivo ou um grupo de hooks. "Aceite" é o que o lead vê no jogo ou no teste.

Os arquivos seguem o padrão de `decomp/src/gm/gmremix*.c`: `#if defined(PORT) && defined(REGION_REMIX)`, um `.h` com `extern`, e chamada a partir do código vanilla.

1. **P1: lógica pura de crédito.** Criar `decomp/src/gm/gmremixtugofwar.h` e `decomp/src/gm/gmremixtugofwar.c`. Funções: escolher o alvo do crédito (KOer, último atacante ou nenhum) a partir de `(time do alvo, time do KOer, porta do último atacante, is_1v1)`; decidir se o stock deve revive. Sem RAM. Aceite: compila e tem teste offline se o projeto tiver harness (verificar); nada visível no jogo.
2. **P2: entrada do modo.** Ler `gMNVSModeRemixMode == 5` em `decomp/src/sc/sccommon/scvsbattle.c` e no caminho do CSS. Gravar `game_rules = SCBATTLE_GAMERULE_STOCK`, `stocks = 3` (ou o valor do CSS) e o estado do modo. Depende de T6.1 (passo de estado VS compartilhado com 12CB, KOTH, Smashketball). Aceite: smoke test do menu até o CSS (f10_menu_vsmodes.md tem o roteiro) mostra o título "Tug of War" e o contador em 3.
3. **P3: rastreio do último atacante.** Hook de dano (ponto a localizar, RAM 0x80141534 sem match) e hook de quebra de escudo em `ftcommonshieldbreakfly.c`. Usar P1. Depende de P1. Aceite: sem visível direto; a verificação é o crédito do P4.
4. **P4: crédito de KO e revive.** Em `decomp/src/ft/ftcommon/ftcommondead.c`, após o bloco de stocks (linhas 98-102) e antes do bloco de 1P, chamar a lógica de P1. Revive com `ftCommonRebirthDownSetStatus` (`ftcommonrebirth.c:27`). Depende de P1 e P3. Aceite: com dois jogadores, o KOer ganha stock visível no HUD, e um jogador eliminado volta ao receber stock.
5. **P5: lugar em time.** Em `decomp/src/if/ifcommon.c` `ifCommonBattleUpdateScoreStocks` (linha 2956), incrementar o lugar quando um time é eliminado no Tug (ver R4). Depende de P4. Aceite: resultado com lugar coerente em partida de times.
6. **P6: pontuação do resultado.** Localizar o dono da pontuação de TimedStock no decomp (não identificado nesta passada) e acrescentar o ramo Tug. Depende de P2. Aceite: placar final ordena por stocks e depois por KOs menos mortes.
7. **P7: CSS de contagem.** Permitir contagem acima de `stocks` no CSS e não resetar a opção Stock no Tug (`css/StockMode.asm:41`). Depende de P2. Aceite: no CSS, a contagem passa de 3 no Tug e volta a ser limitada em outros modos.
8. **P8: anúncio de voz.** Tocar FGM 479 no CSS (TwelveCharBattle.asm:1774-1776 e 1820-1822 como referência). Depende de P2. Aceite: ouvir "Tug of War" ao entrar no CSS.
9. **P9: correções.** Aplicar R1 e R2 com `#ifdef PORT` ou em código Remix, e registrar a decisão em `docs/bugs/`. Depende de P4. Aceite: teste de regressão do lado de escudo e de time.

## 8. Riscos e incertezas

- **R1 (bug provável, `TugOfWar.asm:491-492`).** `li t1, last_hit_by_port` sobrescreve `t1`, que tinha o time do jogador (`lbu t1, 0x000C(s0)` em 483). Em seguida, `beq t0, t1, _end` compara o time do atacante com o endereço de `last_hit_by_port`. A comparação nunca é igual, então a checagem de time igual na quebra de escudo nunca bloqueia. Fica por confirmar no jogo.
- **R2 (bug provável, `TugOfWar.asm:296-303`).** No caminho de time igual, `t9 = last_hit_by_port + t2`, com `t2` = time do alvo (`lbu t2, 0x000C(s0)`, linha 299). O restante do arquivo indexa `last_hit_by_port` pela porta (`0x000D`). Com time igual a porta, o caso funciona por coincidência. O comentário do asm diz "player's last hit by", o que sugere que deveria ser a porta (`0x000D(s0)`).
- **R3 (offsets não conferidos).** `FTStruct` confere (`fttypes.h:1419`). A entrada `a2 + porta * 0x74` com `0x2B` não confere com `SCPlayerData` (`sctypes.h:375`, `stock_count` em `0x0B`). O asm pode estar lendo um layout Remix. Antes do port, decidir se o port usa `gSCManagerBattleState->players[porta].stock_count` do decomp.
- **R4 (`update_sIFCommonBattlePlace_` com `0x24(sp)`).** Em `credit_stock_to_koer_`, o `a0` do `update_sIFCommonBattlePlace_` vem de `0x24(sp)` depois de `OS.save_registers()`. O asm chama de "KOer player struct", mas a função espera o jogador revivido, que é o próprio KOer. A posição do `0x24(sp)` depois de `OS.save_registers()` não foi verificada. Pode estar lendo um slot salvo e não a struct. Ler `OS.save_registers` antes do port.
- **R5 (registrador `t0` assumido).** `credit_stock_to_koer_` usa `t0` como 0x74 (TugOfWar.asm:308-309) sem carregar o valor. `credit_stock_when_no_koer_` carrega (`lli t0, 0x0074`, linha 386). O caminho com KOer depende do valor que o hook vanilla deixa em `t0`. Não verificado. Igual para `a2` em `credit_stock_when_no_koer_` (linha 389), que não é recarregado.
- **R6 (teams em FFA).** O crédito compara times. Em FFA (`teams` FALSE), a checagem de time igual depende de cada jogador ter time distinto. Não confirmei como o VS atribui `team` em FFA (ver `mnplayersvs.c:2457`, que cicla times no CSS). Se todos tiverem time 0, o Tug nunca credita. Verificar.
- **R7 (comentário de modo).** `game_mode = 3` está comentado como "TIMED STOCK" no asm. Pelo `TimedStock.asm:65`, 3 é stock. O port deve usar stock e não tempo.
- **R8 (cap de 0xFF).** `slti t8, t9, 0xFF` (TugOfWar.asm:316) compara um byte com sinal (`lb`) com 255. A condição é sempre verdadeira, então o limite nunca atua. Sem efeito prático se os stocks ficarem baixos. Registrar se o port usar o mesmo teste.
- **R9 (sem SD sem hit).** O caso de autodestruição sem ninguém ter batido não dá crédito (TODO no asm, linhas 353-358 e 422-427). Decisão de design: manter ou corrigir. Não há nota de origem.
- **R10 (HUD acima do padrão).** Não sei se o HUD vanilla mostra corretamente mais de 3 ou 4 stocks. Verificar no jogo antes de aceitar P7.
- **R11 (arquivo do título).** O `0x30F0` aponta para uma imagem em um arquivo que não foi identificado. Sem isso não dá para fechar o título do CSS.
- **R12 (hook de dano sem match).** `0x80141534` não tem função correspondente no decomp (gameplay_m_z.md:1394). Precisa ser localizado por assinatura antes do P3.
- **R13 (ai-memory).** Regras do projeto não carregadas (ver nota no início).
