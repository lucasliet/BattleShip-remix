# Base comum dos modos Remix (T6.0)

Referência do que os modos de M6 reaproveitam. Plano e progresso ficam no [`ROADMAP.md`](ROADMAP.md).

## Modo da partida

- A escolha da página Remix Modes do menu VS fica em `gMNVSModeRemixMode` (`MNREMIX_VSMODE_*`, `mn/mnvsmode/mnremixvsmode.h`). O
  código de partida e de resultados lê `gmRemixModeGet()` / `gmRemixModeIs()` (`decomp/src/gm/gmremixmode.h`).
- `SSB64_REMIX_MODE=default|12cb|tagteam|koth|smashketball|tugofwar` força o modo da execução inteira (testes sem o menu).
- `scVSBattleStartBattle` grava uma linha `remix mode: mode=N rules=N time_limit=N stocks=N team=N` no log no início de cada partida.
- `gmRemixModeSceneClear()` (chamado em `scManagerRunLoop` a cada troca de cena) apaga os rótulos de HUD.

## Timed Stock (`TimedStock.asm`)

- A regra Stock do menu VS grava `game_rules = SCBATTLE_GAMERULE_TIMED_STOCK` (tempo | stock, valor 3, `mnVSModeSaveSettings`),
  para FFA e Team. O limite de tempo é o do estado (`time_limit`, escolhido na regra Time); `SCBATTLE_TIMELIMIT_INFINITE` (100)
  não liga o timer. As regras puras de stock do port (competitivo, Classic Co-op) continuam com `game_rules == 2`.
- O timer do HUD e o fim por tempo já eram por bit (`& TIME`); o stock já era por bit (`& STOCK`). Os pontos que comparavam
  `== TIME` foram ajustados: ícone único de stock (`mnPlayersVSSetSceneData`, `netreplay.c`) e os resultados.
- Resultados: `mnVSResultsInitRankings` usa os lugares de stock quando o stock acabou antes do timer e o placar de tempo quando o
  timer zerou (`gmRemixTimedStockUsesTimeScore`, `pick_scoring_`). `mnVSResultsGetPointsDirect` devolve
  `((stocks iniciais - mortes + 1) << 12) - dano final` (`gmRemixTimedStockScore`, `score_fix_`); sem stock restante, 0.
- Sem morte súbita em Timed Stock (`gmRemixTimedStockAllowsSuddenDeath`, `prevent_sudden_death_`); só a regra Time a tem.
  KOTH e Smashketball (empate) mudam isso nos seus passos.
- Teste: `SSB64_VS_STOCKS=3 SSB64_VS_TIME=1 vs.sh 0,8,6 none.txt nome 5800` (1 minuto, 3 stocks; `SSB64_VS_STOCKS` agora grava a
  regra 3 sob `REGION_REMIX`, `SSB64_VS_TIME` grava o limite em minutos). Visto: timer 01:00 -> TIME UP -> resultados em que
  quem levou dano fica atrás.

## Texto posicionado do HUD (`port/remix/remix_hud_text.h`)

O Remix desenha placares e contadores com fonte própria. O port usa o overlay ImGui:
`remix_hud_text_put(slot, texto, x, y, 0xRRGGBBAA, altura)` por quadro e por rótulo, em coordenadas da tela N64 (320x240), no
centro do texto; o rótulo some sozinho se o gancho parar de chamar (vive 2 quadros). 32 slots; cada modo reserva os seus.
Sprites vanilla continuam sendo preferidos onde o modo já usa sprite (HP do Stamina, contadores de stock).
