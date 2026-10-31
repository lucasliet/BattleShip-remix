# F14: auditoria das linhas `todo`/`partial` de parity.md (menus_modes, audio, ai)

Escopo: linhas de `docs/remix/parity.md` com área `menus_modes`, `audio` ou `ai` e status `todo` ou `partial` (56 linhas). Os `<Fighter>/AI/Attacks.asm` ficam numa linha resumo.

Critérios (proposta, não aplicada ao `STATUS` do `tools/gen_remix_parity.py`):
- **ported**: há equivalente no port e nenhum `REMIX-TODO` nem sobra do ROADMAP (T8.1/T8.2 ou nota de T6/T7) ligado ao asm.
- **partial**: há port, mas resta `REMIX-TODO` ou sobra do ROADMAP.
- **todo**: nada encontrado.
- **unknown**: sem evidência suficiente para classificar.

Limites do método: a evidência cita nomes de arquivo encontrados em `decomp/src` ou `port` (busca por nome e por conteúdo). Não foi conferido função a função contra o asm, então `ported` é provisório onde indicado com "(prov.)". O código do Torch (fora deste repositório) não foi verificado.

| asm | proposto | evidência | leftover |
|---|---|---|---|
| `AA.asm` | ported (prov.) | `remix_display_settings.cpp` (MSAA); ROADMAP linha 237 (M4) | nenhum; `disable_aa_` sem evidência no port |
| `AerialAttackFastFall.asm` | unknown | nenhum arquivo com o nome; `grep fast_fall` retorna 61 arquivos genéricos | nenhum no ROADMAP |
| `Bonus.asm` | partial | `sc1premixbonusexit.c`, `sc1premixmultiman.c`; T6.2 (ROADMAP linha 376) | T8.2: recordes e título do CSS do Bonus 2, lutador aleatório, Bonus 3 |
| `CharacterDataScreen.asm` | partial | `mnremixchardata.c` (T5.3, linha 317) | `REMIX-TODO(chardata-emblem)` em `mncharacters.c:1698`; `REMIX-TODO(chardata-motion-names)` em `mncharacters.c:2371`; T8.2: emblema e golpes dos 18 extras |
| `CharacterSelect.asm` | partial | CSS port: `mnremixcpulevel.c`, `mnremixgoalpicker.c`, `mnplayersvs.c`; T5.1 | T8.2: sobras do CSS VS; `REMIX-TODO(koth-goal-label)` em `mnremixgoalpicker.c:6` |
| `CharacterSelectDebugMenu.asm` | todo | nenhuma correspondência por nome ou conteúdo | nenhum no ROADMAP |
| `Cheats.asm` | todo | só `UnlockCheats.cpp`, sem relação comprovada com o asm | T8.1 cita Cheats |
| `ComboMeter.asm` | ported (prov.) | `ifremixcombometer.c`, `ifremixcombometer.h`; M4 (ROADMAP linha 235) | nenhum |
| `Costumes.asm` | partial | `mnremixcostumes.c`, `mnremixcostumes.h`; T5.1 | T8.2: costumes extras além de `num_costumes`, lutador de 0 costumes |
| `Crash.asm` | unknown | sem arquivo com o nome; `Crash` aparece só em código vanilla | nenhum |
| `Credits.asm` | partial | `scstaffroll.c`, `tools/gen_remix_credits.py`; ROADMAP linha 315 | `REMIX-TODO(credits-glyphs)` em `scstaffroll.c:20`; `REMIX-TODO(credits-speed-assist)` em `scstaffroll.c:1475`; T8.2: glifos e speed assist |
| `DKMode.asm` | todo | nenhuma correspondência por nome ou conteúdo | nenhum no ROADMAP |
| `Gallery.asm` | partial | `mnremixgallery.c`, `mnremixgallery.h`; T5.3 (linha 318) | `REMIX-TODO(gallery-remix-fighters)` em `mnremixgallery.c:12`; T8.2: botão Gallery e 28 entradas |
| `GameEnd.asm` | partial | `gmremixgameend.c`, `gmremixgameend.h`; ROADMAP linha 241 | `REMIX-TODO(results-mode-rows)` em `gmremixgameend.c:150` |
| `HRC.asm` | partial | `sc1premixhrc*.c` (Home-Run Contest); T6.2 (linha 376) | ROADMAP linha 378 ("unidade do HRC"); sem REMIX-TODO |
| `Handicap.asm` | partial | `ftremixhandicap.c` | `REMIX-TODO(AI)` em `ftremixhandicap.c:14` e em `mnremixcpulevel.c:17` |
| `InputDelay.asm` | ported (prov.) | `ftremixinputdelay.c`; T5.2 (linha 301) | nenhum |
| `InputDisplay.asm` | partial | `ftremixinputdisplay.c`; overlay ImGui (linha 314) | ROADMAP linha 246: "Input Display" na lista de sobras |
| `KingOfTheHill.asm` | partial | `gmremixkoth*.c`, `mnremixgoalpicker.c`; T6.1c (linha 371) | T6.1c: faltam faixa em polilinhas, título e anúncio do CSS; linha 325: linhas de modo nos resultados |
| `Menu.asm` | unknown | `Menu.cpp` e `PortMenu.cpp` são genéricos; sem correspondência comprovada | nenhum |
| `Pause.asm` | partial | `ftremixpause.c`, `gmremixcamerapause.c`; M4 | T8.1 cita Pause; T8.2: legends da pausa |
| `PlayerTag.asm` | partial | `ifremixplayertag.c`; T5.2 (linha 299) | T8.2: nomes das tags |
| `Practice.asm` | unknown | `gmremixpractice.c` cobre Practice_1P; `flash_on_z_cancel_` e `overlay_colour_` sem evidência | nenhum |
| `Practice_1P.asm` | partial | `gmremixpractice.c`, `remix_player_panel.cpp`; T6.2 (linha 377) | T8.2: Practice_1P L-retry |
| `resultsscreen.asm` | partial | `mnremixwinbgm.c`, `mnvsresults.c`, `SkipResultsScreen.cpp`; T6.x | T8.2: telas de resultados; `REMIX-TODO(results-mode-rows)` |
| `Settings.asm` | unknown | `remix_toggles.cpp` (T4.1, linha 220); botão Settings citado no linha 225 | sem evidência para `set_vs_settings_` |
| `Shield.asm` | partial | `ftremixshield.c` | `REMIX-TODO(Shield)` em `ftremixshield.c:7`; T8.2: Shield Costume e ovo do Yoshi |
| `SinglePlayer.asm` | partial | `sc1premixallstar.c`, `sc1premix1p.h`; T6.2 (linha 376) | T8.2: sobras do 1P |
| `SinglePlayerEnemy.asm` | unknown | sem arquivo com o nome; `enemy_routine_` sem busca | nenhum |
| `SinglePlayerMenus.asm` | unknown | sem evidência específica para `header_graphic_remix_1p` nem `draw_hrc_high_score_decimal_` | nenhum |
| `SinglePlayerModes.asm` | partial | `sc1premixintro.*`, `sc1premixallstar.*`, `sc1premixmultiman.c`; T6.2 R4 (linha 393) | T8.2: sobras do 1P |
| `Skeleton.asm` | todo | nenhum arquivo; `REMIX-TODO(Skeleton)` citado no ROADMAP (linha 302) não existe no código | T8.1 e T8.2 citam Skeleton |
| `Smashketball.asm` | partial | `gmremixsmashketball*.c` (6 arquivos); T6.1e (linha 373) | T6.1e: sobras no T8.2 (título e anúncio do CSS, cores, câmera) |
| `SRAM.asm` | unknown | `port_save.cpp`, `lbbackup.c` citados sem vínculo comprovado com o asm | ROADMAP T4.1 citado no parity, sem texto próprio |
| `Stamina.asm` | partial | `ftremixstamina*.c` (6 arquivos); T6.1a (linha 364) | `REMIX-TODO(stamina-gigabowser)` em `ftremixstaminaaction.c:148`; T8.2: Tug of War e Stamina (`REMIX-TODO(stamina)` do T6.D5) |
| `StockMode.asm` | partial | `gmremixstockmode.c`, `gmremixstockmode.h`; T6.x (linha 363) | `REMIX-TODO(StockMode)` em `gmremixstockmode.c:14`; T8.2: StockMode Last |
| `TagTeam.asm` | partial | `gmremixtagteam*.c` (6 arquivos); T6.1d (linha 372) | T6.1d e T8.2: título e anúncio do CSS, variantes, Sonic classic, heap |
| `Teams.asm` | partial | mudanças de Teams feitas no Stamina (linha 364); `scmanager.c` | ROADMAP linha 369: "P16 Teams (T6.2)" aberto |
| `TimedStock.asm` | ported (prov.) | `gmremixtimedstock.c`, `gmremixtimedstockrules.c`; T6.1a (linha 364) | nenhum |
| `Toggles.asm` | partial | `remix_toggles.cpp`, `gmremixtoggle.c`, `PortMenu.cpp`; T4.2 (linha 227) | `REMIX-TODO(toggles)` em `grremixscuttle.c:83` (acessor do menu de Training ausente) |
| `Training.asm` | partial | `sc1ptrainingremix.h`, `sc1ptrainingmode.c`; T6.3 | T8.2: Start With no Treino; sobras do Training (T6.3) |
| `Transitions.asm` | partial | `lbtransition.c`; `REMIX-TODO(results-transition-remix-logo)` | `REMIX-TODO(results-transition-remix-logo)` em `lbtransition.c:192`; T8.2: logos de transição |
| `TugOfWar.asm` | partial | `gmremixtugofwar*.c` (4 arquivos); T6.1b (linha 370) | T6.1b e T8.2: P7 a P8, título do CSS |
| `TwelveCharBattle.asm` | partial | `gmremixtwelvecb*.c` (6 arquivos); T6.1f (linha 375) | `REMIX-TODO(twelvecb-css)` em `gmremixtwelvecbrules.h:74`; T8.2: CSS de 24 retratos, portas 3 e 4 |
| `VsDemo.asm` | todo | nenhuma correspondência | T8.1 cita VsDemo |
| `VsRemixMenu.asm` | unknown | T5.3 cita "Remix Modes" (linha 348); sem arquivo com o nome | nenhum |
| `VsStats.asm` | partial | `mnremixvsstats.c`, `mnremixvsstats.h`; T5.3 (linha 319) | `REMIX-TODO(results-text)` e `REMIX-TODO(results-stats-trackers)` em `mnremixvsstats.c:5,7`; T8.2: página de estatísticas |
| `Walljump.asm` | todo | nenhuma correspondência | nenhum no ROADMAP |
| `timeouts.asm` | todo | nenhuma correspondência | nenhum no ROADMAP |
| `BGM.asm` | partial | `grRemixGetStageBGM` (`grremixstage.c:220`); `mnremixwinbgm.c` | `REMIX-TODO(bgm)` em `grremixtimetwister.c:74` (troca de faixa) |
| `FGM.asm` | ported (prov.) | Torch `RemixAudioFactory` (fora do repositório, não verificado); T3.2 (linha 204) | nenhum |
| `midi.asm` | ported (prov.) | `port/audio/audio_playback.cpp`; `audio_bridge` | nenhum |
| `SFXReplace.asm` | unknown | `sfx_replace` sem correspondência; T3.2 diz que os SFX tocam | nenhum |
| `Stereo.asm` | ported | `lbcommon.c:775` ("Stereo.asm fix_sfx_pan_"); ROADMAP linha 207 | nenhum |
| `AI.asm` | partial | `ftremixai*.c`, `ftremixaiprevent.*`, `ftremixailv10.*`, `ftcomputer.c`; T7.1 (linha 463) | 20 `REMIX-TODO(AI)`; T8.2: sobras do M7 (`cpu_di_`, `_improve_recovery_drift`, `cpu_post_process`, `auto_jab_grab_`, AERIAL_CHASE, PUFF_DSP) |
| `VanillaLv10Attacks.asm` | todo | nenhuma correspondência; só `tools/gen_remix_parity.py` | T8.2: rotinas PREVENT vanilla (MARIO, YOSHI_FALCON, KIRBY), precisam da ROM |
| `<Fighter>/AI/Attacks.asm` (32 linhas) | resumo | parity marca todo; `ftremixai*.c` e T7.1 (linha 463) cobrem as tabelas dos 30 lutadores Remix | T7.1 (linha 475): Banjo P06, Sheik, Peppy, Bowser e GBowser P11 |

## Contagem por status proposto (56 linhas)

- ported: 7 (AA, ComboMeter, InputDelay, TimedStock, FGM, midi, Stereo). Sendo que 6 são provisórios (marcados com "prov.").
- partial: 31
- todo: 8
- unknown: 10 (AerialAttackFastFall, Crash, Menu, Practice, Settings, SinglePlayerEnemy, SinglePlayerMenus, SRAM, VsRemixMenu, SFXReplace)
