# F10 - Painel CSS: Stock (VS) e Practice 1P

Fontes (somente leitura): `.toolchain/pkgs/smashremix-src/src/`. Linhas conferidas no código.

## StockMode (VS) e nova opção StockCount

Asm lido: `StockMode.asm` (`stockmode_table` linha 11, `stock_count_table` linha 17), `set_initial_stock_count_` (linhas 49 a 110), `css/StockMode.asm`, `CharacterSelect.asm` 6157 (`update_remaining_stocks_`).

Semântica por porta, no início da partida (t8 = contagem de stocks das regras da partida):
- 0 Default: usa t8.
- 1 Last: usa o último valor guardado em `previous_stock_count_table`, pelo menos `min(prev, t8)`. Esse valor vem do contador de stocks do CSS (`CharacterSelect.asm` 5627) e de `GameEnd.asm` 138. O port não tem o contador do CSS, então não foi portado (ver REMIX-TODO abaixo).
- 2 Manual: usa `stock_count_table[porta]` somente se for menor que t8 (`sltu`). Sem valor definido (-1, lido como 0xFF) vale t8.

Contagem manual: o contador do CSS guarda a contagem 0-based (0..contagem da regra). `ifcommon.c` 1095 exibe `stock_count + 1`. Por isso o padrão 3 corresponde a 4 stocks, como no ruleset competitivo (`scmanager.c` 1333). A faixa usada é 0..99 (não 1..99 como sugeria o brief), porque o asm aceita 0 e o valor acima da regra não tem efeito.

Port:
- `port/remix/remix_player_panel.h` e `.cpp`: `REMIX_PANEL_STOCK_COUNT` acrescentado depois de `REMIX_PANEL_PRACTICE_1P`, chave `StockCount`, título "Stock Count (Manual)", faixa 0..99, padrão 3, sem rótulos.
- `decomp/src/gm/gmremixstockmode.c` e `.h` (novos): `gmRemixStockModeCount(player, match_stocks)`. Retorna `match_stocks` quando o modo não é Manual, senão `min(manual, match_stocks)`.
- `decomp/src/sc/sccommon/scvsbattle.c`: no loop que cria os lutadores VS, `desc.stock_count` usa `gmRemixStockModeCount` sob `#ifdef REGION_REMIX` (vanilla inalterado sem a flag). `ftmanager.c` 1033 copia `desc.stock_count` para `fp->stock_count` e `players[].stock_count`, então a HUD e a regra de morte acompanham.

REMIX-TODO aberto: `REMIX-TODO(StockMode)` em `gmremixstockmode.c`. O modo Last (1) cai no Default. Para fechar, é preciso guardar a contagem final de cada porta no fim da partida (gmremixgameend.c) e aplicar a regra do asm.

Não tratado: Tag Team, Tug of War, 12 Character Battle, King of the Hill e Smashketball (o asm altera a tabela nesses modos). O hook está só em `scvsbattle.c`; confirmar se esses modos passam por ele no port.

## Practice_1P

Asm lido: `Practice_1P.asm` `_1p_practice` (linhas 41 a 110, mapeamento de fase), `hold_l_to_retry_` (172), `hold_l_to_retry_stage_clear_` (235), `stage_table` (339), `STAGES` = 11.

Mapeamento (valor do painel para fase): 1..3 para as fases 0..2 do asm; 4..6 para 4..6; 7..9 para 8..10; 10 para 12; 11 (Final Stage) para 13. As fases 3, 7 e 11 são bônus e ficam de fora. Confere com `nSC1PGameStage` em `decomp/src/sc/scdef.h` 293 a 315 (Link=0, ..., Zako=12, Boss=13).

Port:
- `decomp/src/gm/gmremixpractice.c` e `.h` (novos): `gmRemixPracticeStartStage(player, stage)`. Lê `remix_panel_get(REMIX_PANEL_PRACTICE_1P, player)` diretamente, não por `gmRemixPanel`, porque o início da partida 1P roda antes de `gSCManagerBattleState->players` estar preenchido e a guarda de porta humana poderia zerar o valor.
- `decomp/src/mn/mnplayers/mnplayers1pgame.c`, `mnPlayers1PGameSetSceneData`: depois do bloco `SSB64_SPGAME_STAGE`, sob `#ifdef REGION_REMIX`, troca `spgame_stage` pela fase do painel. O painel vence o override de ambiente quando está ligado.

REMIX-TODO aberto: `hold_l_to_retry_` e `hold_l_to_retry_stage_clear_` (segurar L para reiniciar a fase na tela GAME SET e STAGE CLEAR). O port não tem reinício sem continue: o único caminho de nova tentativa é o continue (`sc1pmanager.c` 586 a 594), que cobra continue e vai para a tela de continue. Falta um caminho de reinício sem custo, que é a parte grande do asm (`reset_flag`, `Global.current_screen`, `screen_interrupt`). Não foi portado.

## Não compilado

Nada foi compilado nem rodado. `scripts/remix/syntax-check.sh` passou para `gmremixstockmode.c`, `gmremixpractice.c`, `mnplayers1pgame.c` e `scvsbattle.c`. `remix_player_panel.cpp` não foi checado (o build do lead cobre). Não há teste unitário novo: o brief pede que o lead valide no jogo.

Verificação sugerida ao lead: no VS, pôr StockMode=Manual e StockCount=1 para a porta 2 e conferir se a HUD mostra 2 stocks; com StockMode=Default, a partida igual à vanilla. No 1P, Practice=Stage 4 deve começar em Mario (fase 4), e Practice=Final Stage em Boss.
