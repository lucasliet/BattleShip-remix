# Remix 1P: tamanho forçado das CPUs sobrescrito pelo painel do CSS (CP6f)

## Sintoma
`sc1PRemix1PSetupEnemy` gravava `FTREMIX_SIZE_STATE_GIANT`/`TINY` com `ftRemixSizeSetMatchState`, mas a CPU da fase gigante e as do Tiny Team
entravam em partida com tamanho normal.

## Causa
`ftRemixSizeInitFighter` (chamada em `ftManagerInitFighter`) regrava o estado com `gmRemixPanel(SIZE, player)`. Nos modos 1P e Bonus
o painel das portas que não são humanas vale 0 (`gmRemixPanelGet`, igual ao `clear_settings_for_1p_` do asm), então o estado voltava a normal.

## Correção
`ftremixsize.c` guarda um estado forçado por porta (`ftRemixSizeForceMatchState`, `FTREMIX_SIZE_STATE_UNFORCED` = -1 devolve a porta ao painel).
`ftRemixSizeInitFighter` usa o forçado quando existe. `sc1PRemix1PSetupEnemy` força Giant na linha 6 (Donkey) e Tiny na linha 9 (Samus),
`sc1PRemix1PRandomize` e as demais fases limpam os estados forçados.

## Verificação
- `SSB64_SPGAME_STAGE=6`, Remix 1P: log com `player 3 fkind 29 state 1 multiplier 2.25` (log temporário, removido).
- `SSB64_SPGAME_STAGE=9`: as três CPUs Dedede ficam menores que o Mario na captura.
- `run_remix_mode_tests`: 187 checagens, 0 falhas; `ssb64` compila.
- O valor de `SSB64_SPGAME_STAGE` é o índice da linha da tabela; o caminho muda a cada execução, então conferir o log da mesma rodada.
