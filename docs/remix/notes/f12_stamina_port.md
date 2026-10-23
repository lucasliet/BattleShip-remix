# F12 Stamina port: fighter guards (shield, armor, invisibility, egg, down, ledge)

Source: `.toolchain/pkgs/smashremix-src/src/Stamina.asm` (stamina_shield_prevent 686, down/ledge 1068-1208,
stamina_invis_prevent 1306, egg_crash_prevent 1429, stamina_armor_fix_ 1474), `Size.asm` passive_armor_ (around 128-150),
`Action.asm` and `Roy/Roy.asm` for the action ids. Plan step: `f11_mode_stamina.md` section 7, P11 (hooks 12, 24-27, 30-33).

## Step P11: shield, ledge, down wait/bounce, egg lay, armor

Rule used by every guard: `gmRemixStaminaIsActive()` (P1) and the fighter's damage percent above
`gmRemixStaminaGetTotalHP()` (the asm TOTAL_HP). The guards do not read `gmRemixStaminaIsDefeated`, because the
percent_port flag is only set by P7 (hook 28), which is not ported yet.

### Files written

- `decomp/src/ft/ftremix/ftremixstaminaguard.c` (new): `ftRemixStaminaIsOutOfHP`, `ftRemixStaminaArmorIsZeroed`,
  `ftRemixStaminaSkipsInvisible`, `ftRemixStaminaSkipsDeadWaitInit`, plus the static `sFTRemixStaminaIsInvisibleExcludedAction`.
- `decomp/src/ft/ftremix/ftremixstaminadown.c` (new): `ftRemixStaminaDownBlocked(GObj *)`, `ftRemixStaminaLedgeBlocked(FTStruct *)`.
- `decomp/src/ft/ftremix/ftremixhooks.h`: block appended at the end (declarations of the six functions above).

### Files edited (hook spots)

| asm (line) | RAM | vanilla function (decomp file) | port |
|---|---|---|---|
| Stamina.asm:686 `stamina_shield_prevent` | 0x80148CE0 | `ftCommonGuardOnCheckInterruptSuccess` (ftcommon/ftcommonguard1.c) | early `return FALSE` when out of HP. Include of the hooks header added (it was missing in that file). |
| Stamina.asm:1068 `stamina_down_wait_down` | 0x8014423C | `ftCommonDownWaitProcUpdate` (ftcommon/ftcommondownwaitbounce.c) | the stand call `ftCommonDownStandSetStatus` (0x80144580) is skipped when out of HP. `stand_wait--` still runs. |
| Stamina.asm:1104 `stamina_down_wait_input` | 0x8014425C | `ftCommonDownWaitProcInterrupt` (ftcommondownwaitbounce.c) | early `return` when out of HP (the asm jumps to the epilogue at 0x80144284). |
| Stamina.asm:1140 `stamina_down_bounce_input` | 0x80144360 | `ftCommonDownBounceProcUpdate` (ftcommondownwaitbounce.c) | out of HP and `anim_frame <= 0`: call `ftCommonDownWaitSetStatus` and return (the asm jumps to 0x80144380). |
| Stamina.asm:1176 `stamina_ledge` | 0x80144C34 | `ftCommonCliffCatchSetStatus` (ftcommon/ftcommoncliffcatchwait.c) | early `return` when out of HP (the asm jumps to 0x80144CE4). |
| Stamina.asm:1306 `stamina_invis_prevent` | 0x8013C150 | `ftCommonDeadInitStatusVars` (ftcommon/ftcommondead.c) | `fp->is_invisible = TRUE` is skipped in Stamina for the excluded actions. |
| Stamina.asm:1429 `egg_crash_prevent` | 0x8013C134 | `ftCommonDeadInitStatusVars` (ftcommondead.c) | `dead.wait = FTCOMMON_DEAD_WAIT` is skipped in Stamina during EggLay. |
| Size.asm (passive_armor_, calls `stamina_armor_fix_`) | 0x800E6324 | `ftRemixSizePassiveArmor` (ftremix/ftremixsizebody.c) | returns 0.0F when Stamina is active and `percent_damage + damage_queue > TOTAL_HP`. The old note saying "no port" is replaced. |

Note: `ftcommondead.c` is not in the lead's file list. The invis and egg hooks live in `ftCommonDeadInitStatusVars`, so
the edit was needed. The P9 hook 13 (`stamina_ko_prevent`, 0x8013C168) is in the same function. **Whoever ports P9 must
edit that function after these guards.**

### Deviations from the asm

- Out of HP is `percent_damage > TOTAL_HP` for every guard, as the brief asked. The asm shield hook checks stock `-1`
  instead; the port uses HP, so a fighter out of HP cannot shield even before the KO sets the stock. The down and ledge hooks
  already compare HP in the asm. `stamina_armor_fix_` uses percent + incoming damage (0x7F0), and the armor guard keeps that sum.
- Stamina.asm stamina_invis_prevent: the asm keeps the flag unset for the excluded actions only. The port keeps the same meaning.
  The Roy strong ends (0xF2 and 0xF6, Roy/Roy.asm) apply only to `fkind == REMIX_ID_ROY` (= 0x4A).
- Bounce hook: the asm runs the forced transition after `anim_frame <= 0`, the port keeps that order. The asm order of the
  anim check against the first call could not be proven from the source; it is assumed that the anim check comes first.
- `ftRemixStaminaLedgeBlocked` is an alias of `ftRemixStaminaIsOutOfHP` kept in the down file as the lead asked.

### Assumptions (not confirmed in game)

- `0xB18` in egg_crash_prevent and action_hit_crash_prevent is `status_vars.common.dead.wait`. The asm comment in
  action_hit_crash_prevent says the word is "used in coming actions if player is dead already", and the store value is
  the dead wait. Confirm with the egg lay test.
- Hook 33 `stamina_armor_fix_` returns 0 from the whole `passive_armor_` (the stub restores `ra` from the caller frame), so
  the port returns 0.0F from `ftRemixSizePassiveArmor`.
- `0x7F0` (incoming damage this frame) is `fp->damage_queue`, as in `ftParamGetCommonKnockback` callers.
- `0x80144CE4` (ledge) and `0x80144284` (down wait input) are the function epilogues in vanilla.

### Left open

- Nothing in P11 is marked REMIX-TODO. P7 (`percent_port` flag and the KO), P8 (forced KO action), P9 (hook 13, 14, respawn)
  and P10 are not ported. The guards here do not depend on them, but a fighter gets no KO from the guards alone; the KO comes
  from P7.
- The guards are inert unless `gmRemixStaminaIsActive()` is TRUE. That needs `gGMRemixStaminaRule` set by the VS menu Rule
  button (P14) and `game_rules == SCBATTLE_GAMERULE_TIMED_STOCK`. The `SSB64_REMIX_MODE` values documented in `modes_base.md`
  do not include Stamina, so the lead needs the menu to see this.

### Verification

- `scripts/remix/syntax-check.sh` OK on: ftremixstaminaguard.c, ftremixstaminadown.c, ftcommonguard1.c,
  ftcommondownwaitbounce.c, ftcommoncliffcatchwait.c, ftcommondead.c, ftremixsizebody.c.
- One pre-existing warning (`'&&' within '||'`, ftcommondownwaitbounce.c line 114, vanilla code) was left alone.
- Not built and not run (the brief says the lead builds).

### What the lead must see on screen

Stamina rule on (menu), 200 HP, then damage one fighter until its percent passes TOTAL_HP (199), then:

1. Press Z (shield) while out of HP: no shield bubble.
2. Knocked down out of HP: no stand-up input and no bounce-to-stand; the fighter stays in down wait.
3. Ledge in reach out of HP: no ledge grab.
4. Passive armor (a size-1 fighter has none; use a giant size for the test) out of HP: no damage reduction.
5. KO during EggLay (Yoshi) and during a capture or Falcon dive pull: no crash and no invisible flag left on the fighter.

## Step P14 e P15: botao Rule e Time/Stock do menu VS, valor de HP e scroller do CSS

Arquivos editados:
- `decomp/src/mn/mndef.h`: enum `MNVSModeRule` ganha `nMNVSModeRuleStamina` (2) e `nMNVSModeRuleStaminaTeam` (5) sob `PORT && REGION_REMIX`, na ordem Time, Stock, Stamina, Time Team, Stock Team, Stamina Team. Macros `nMNVSModeRuleLast` e `nMNVSModeRuleFreeForAllLast` apontam para o ultimo valor de cada build (vanilla: StockTeam e Stock, mesmos valores de antes).
- `decomp/src/mn/mnvsmode/mnremixvsstamina.c` e `.h` (novos): helpers sem statics do vanilla. Sprites lidos de `sMNVSModeFiles[1]` por offset (0x6D28 imagem Stamina, 0x6B28 "Stamina.", 0x6E28 "H.P.") e `llMNVSModeTeamTextSprite`. Passo do HP e escrita em `gGMRemixStaminaHP`.
- `decomp/src/mn/mnvsmode/mnvsmode.c`: hooks pequenos sob `#if PORT && REGION_REMIX`.
- `decomp/src/mn/mnplayers/mnplayersvs.c`: hooks no CSS (includes, `mnPlayersVSMakeStockSetting`, duas setas de Time/Stock).

Hooks (asm -> decomp):
- Stamina.asm:1564-1576 `vs_menu` (limite do botao Rule 4 -> 6) -> `nMNVSModeRuleLast` em `mnVSModeMakeRuleButton` vizinhos e no scroll de `mnVSModeFuncRun`.
- :1576-1698 `add_to_rule_button_` -> `mnVSModeMakeRuleValue` (novo caso Stamina / Stamina Team chama `mnRemixVSStaminaMakeRuleValue`), `mnVSModeAnimateRuleArrows` (seta direita some em `nMNVSModeRuleLast`), `mnVSModeFuncStartVars` (rule restaurada de `gGMRemixStaminaRule` + flag de time), `mnVSModeSaveSettings` (grava flag e `game_rules = SCBATTLE_GAMERULE_TIMED_STOCK` nos dois Stamina).
- :1698-1778 `add_to_stock_time_button_` -> `mnVSModeMakeTimeStockButton` (rotulo "Stamina." na posicao do Stock e "H.P." na do MIN).
- :1778-1968 `add_to_stock_time_value_` -> `mnVSModeMakeTimeStockValue` (3 digitos, x 197/205/209 pelo asm), `mnVSModeGetShade` (Stamina conta como FFA), `mnVSModeSetCostumesAndShades` (casos Stamina e Stamina Team), input L/R do botao Time/Stock (passo do HP, `mnRemixVSStaminaScrollHP`), input L/R do botao Rule (limite `nMNVSModeRuleLast`, costume na troca FFA/time usa `nMNVSModeRuleFreeForAllLast`).
- :1968 init do Rule (`lbu t8 ... srl`): coberto por `mnVSModeFuncStartVars` (mapeamento direto do enum, sem o shift do asm).
- :1984 `vs_css` (3 digitos) -> `mnPlayersVSMakeStockSetting` (remix: `mnRemixVSStaminaMakeCSSValue`, 3 digitos, x 210/214).
- :1990 `stamina_scrolling_` (setas do CSS) -> `mnPlayersVSCheckTimeArrowRInRange` / `...LInRange`: `else if (gGMRemixStaminaRule)` chama `mnRemixVSStaminaScrollHP` e redesenha com `mnPlayersVSMakeStockSelect`.
- :2064 `stamina_hp_display_` (fundo "H.P." do CSS em 0x1ADE0 e valor) NAO portado nesta etapa: o fundo do CSS e um sprite de outro arquivo; o valor ja aparece porque o seletor de stock usa o HP.

Passo do HP (decisao, display 1..999; asm guarda HP-1):
- Subir: 1 ate 9 de 1 em 1 (2..10), 10 em 10 ate 990 (20..990), depois 1 em 1 (991..999). Descer espelhado: 999..991 de 1 em 1, 990..20 de 10 em 10, 10..1 de 1 em 1.
- Limite: MIN 1, MAX 999. Resolvido o 998/999: o asm guarda 0x3E6 = 998 como teto (scroll_left/right e CSS), que exibido e 999. O comentario "1000" no `_stamina` e antigo. Usei 999 (`GMREMIX_STAMINA_HP_MAX`), consistente com a flag do lead.

Desvios do asm (intencionais):
- Os passos literais do asm nao caem em 1 e 10 de forma consistente (os comentarios misturam valor guardado e exibido). Usei o passo descrito na nota (1 abaixo de 10, 10 ate 990, 1 acima), o mesmo nos dois lugares (menu e CSS). O CSS do asm usava +/-10 com ajustes.
- Ignorei o teste `Joypad.L|CL|DL HELD` do menu (o asm nao move o valor com o gatilho mantido). O ritmo de repeticao fica no controle da tela vanilla (ChangeWait).
- O asm chama `CharacterSelectDebugMenu.Damage.restrict_damage_for_stamina_` no CSS; nao portei (fora de P15).
- Sem subtitulo de regra para Stamina (o switch de `mnVSModeMakeSubtitle` nao tem caso; a tela fica sem texto abaixo do botao). Pendente de texto.
- Posicoes do rotulo: "Stamina." na posicao do Stock (106,114), "H.P." em x 212 y 120 (x do asm, `_min_position`). O X 84.33 do asm e sobrescrito depois, por isso nao usei.

O que o lead deve ver na tela (com o build):
1. Menu VS, pagina 1, botao Rule: setas trocam a ordem Time, Stock, Stamina, Time Team, Stock Team, Stamina Team. Stamina aparece como imagem (x 176). Stamina Team: duas imagens a 0.875 de escala (x 165 e 224).
2. Botao Time/Stock em Stamina: "Stamina." no lugar de "Stock.", "H.P." no lugar de "MIN", valor com 3 digitos (ex.: 200). Seta esquerda e direita mudam o valor: 1, 2..10, 20..990, 991..999.
3. Salvar o menu (VS Start) e voltar: o Rule continua em Stamina e o valor de HP continua.
4. CSS VS: o seletor mostra o HP (200) com 3 digitos; as setas mudam o HP com os mesmos passos. Em Stock e Time nada muda.
5. Partida: o HP de inicio do CSS deve ser o mesmo do menu (um unico valor, `gGMRemixStaminaHP`).

Estado: `syntax-check OK` nos 4 arquivos tocados (mndef.h, mnvsmode.c, mnremixvsstamina.c, mnplayersvs.c). Nao compilei nem rodei (regra do brief). Pendente: build e teste em jogo; subtitulo de Stamina; fundo "H.P." do CSS (asm 0x1ADE0/0x5270); restrict_damage_for_stamina_.

## Step P9: Respawn and single death (stamina_ko_prevent, stamina_ko_prevent_front, stamina_fix_6, stamina_fix_7)

Source: `.toolchain/pkgs/smashremix-src/src/Stamina.asm` lines 746-860 (stamina_ko_prevent, stamina_ko_prevent_front,
stamina_stock and stamina_stock_2 read for context) and 1016-1066 (stamina_fix_6, stamina_fix_7), read in full. Decomp:
`decomp/src/ft/ftcommon/ftcommondead.c` (ftCommonDeadCheckRebirth, ftCommonDeadInitStatusVars, ftCommonDeadUpStarProcUpdate,
ftCommonDeadUpFallProcUpdate) and the P7 section above.

Files:
- `decomp/src/ft/ftremix/ftremixstaminadead.c` (new): `ftRemixStaminaIsOutOfStock` (stock -1 while Stamina is active),
  `ftRemixStaminaIsBlastRedeath` (out of stock and motion flag1 0, the first frame of a death proc),
  `ftRemixStaminaRebirthSleeps` (stores a star KO -2 as -1 in FTStruct and in players[], TRUE when out of stock).
- `decomp/src/ft/ftremix/ftremixhooks.h`: declarations of the three functions, after the ledge guards.
- `decomp/src/ft/ftcommon/ftcommondead.c`, four small REGION_REMIX blocks, all with `ftCommonSleepSetStatus` and `return`:
  1. top of `ftCommonDeadCheckRebirth`: `ftRemixStaminaRebirthSleeps(fp)`.
  2. top of `ftCommonDeadInitStatusVars`: `ftRemixStaminaIsOutOfStock(fp)`, before the dead wait, the invisible flag and the score.
  3. top of `ftCommonDeadUpStarProcUpdate` (star KO death proc): `ftRemixStaminaIsBlastRedeath(fp)`.
  4. top of `ftCommonDeadUpFallProcUpdate` (top blast death proc): `ftRemixStaminaIsBlastRedeath(fp)`.

Both files pass `scripts/remix/syntax-check.sh` (syntax-check OK).

Hook map (asm -> decomp):
- stamina_ko_prevent (0x8013C168, hook 13) -> `ftCommonDeadInitStatusVars` entry (hook 2). The asm only skips the rumble call
  (0x8013BC8C) and jumps to 0x8013C1B0; that target is not identified in the decomp. The port skips the whole death (no rumble,
  no score, no SFX), which is what the lead asked for.
- stamina_ko_prevent_front (0x8013C9B8, hook 14) -> `ftCommonDeadUpFallProcUpdate` first frame (hook 4). The asm skips the
  rumble call inside case 1 and jumps to 0x8013C9EC; not identified. The port skips the whole case 1 (rumble, score, SFX).
- stamina_fix_6 (0x8013BFA4, hook 22) -> NOT mapped exactly. The asm reads byte +3 of a table entry (v1) into a1, and in Stamina
  sets a1 = 2 on the rebirth path. The decomp counterpart of that byte is unknown. The respawn loop is covered instead by
  hook 1: in Stamina a fighter with stock -1 always goes to sleep, whatever the game_rules bits say.
- stamina_fix_7 (0x8013BFB8, hook 23) -> `ftCommonDeadCheckRebirth` entry (hook 1). The asm compares the stock byte at +0x14 with
  -2 (star KO on the last stock), stores -1 and jumps to the ScreenKOWait action (ftCommonSleepSetStatus). The port reads
  `fp->stock_count` (the lead's check), clamps -2 to -1 in FTStruct and in players[], and sleeps.
- stamina_invis_prevent (0x8013C150, hook 30): already in place (guard worker, `ftRemixStaminaSkipsInvisible` in
  `ftCommonDeadInitStatusVars`). Not changed here.
- stamina_stock (0x800D8034) and stamina_stock_2 (0x800D8014) are not part of P9: P5 handles stock 1 (`gmRemixStockModeCount`
  returns 0 in Stamina, so the first death goes from 0 to -1).

Deviations from the asm:
- The asm lets a re-death run the score (stock -1 -> -2) and fixes it at rebirth. The port stops the re-death before the score,
  so the stock never reaches -2 and no falls/KO score is counted twice. The rebirth clamp is kept for the star KO case.
- The star KO death proc (`ftCommonDeadUpStarProcUpdate`) has no asm hook. Its guard is a port addition, required by the brief.
- Re-death is sent to sleep (`ftCommonSleepSetStatus`) instead of returning silently, so the fighter cannot stay in a dead status
  with a stale dead.wait.
- Guard for the proc deaths depends on motion flag1 == 0 on the first frame. Checked in ftCommonDeadUpStarProcUpdate setter
  (flag1 = 0). NOT checked for the UpFall setter: it is the risk to watch in the screen check.

Risks for the lead:
- `ftCommonSleepSetStatus` is called from inside `ftCommonDeadInitStatusVars` (which the KO path calls from `ftRemixStaminaKO`
  during a hit) and from the death procs. CheckRebirth already does this from a proc, but the init call is new. Test a hit that
  KOs a fighter at stock 0 and check the fighter does not reappear.
- The first death must still run: stock 0 at entry, score -> -1, dead wait, rebirth -> sleep.

Screen check (for the lead, not run here): Stamina FFA, one stock per fighter.
- Blast zone death at stock 0: one death, stock -1, no second score. Then no respawn.
- Blast zone death on a fighter already at -1 (for example, a second blast after the KO): nothing happens; the fighter stays asleep.
- Top blast (UpFall) death at stock 0 then at -1: second one does not score; the fighter sleeps.
- Star KO on the last stock: falls +1 once; stock -1 (HUD shows 0 lives); the fighter sleeps and does not loop.

## Step P8: Forced KO action (stamina_ko_action)

Fonte: `.toolchain/pkgs/smashremix-src/src/Stamina.asm` linhas 514-685, lidas por completo. Ids de `Action.asm`.

Arquivos editados:
- `decomp/src/ft/ftremix/ftremixstaminaaction.c` (novo): `ftRemixStaminaKOStatus(fp, status_id)`. Retorna `status_id` sem mudar quando a regra está desligada, o lutador está dentro do HP, ou o status é protegido. Senão retorna `nFTCommonStatusDownBounceD` (chão, `fp->ga == nMPKineticsGround`) ou `nFTCommonStatusDamageFall` (ar, Tumble).
- `decomp/src/ft/ftremix/ftremixhooks.h`: declaração de `ftRemixStaminaKOStatus`, no bloco de Stamina, depois de `ftRemixStaminaStoredDamage`.
- `decomp/src/ft/ftmain.c` (`ftMainSetStatus`): uma linha dentro do `#ifdef REGION_REMIX` que já tinha os hooks de Dedede e Kirby clone: `status_id = ftRemixStaminaKOStatus(fp, status_id);`, por último, para que a checagem veja o status final. O hook não chama `ftMainSetStatus`, então não há recursão.

Condição de KO: regra Stamina ativa e (`gmRemixStaminaIsDefeated(fp->player)` ou `ftRemixStaminaIsOutOfHP(fp)`, que é porcentagem acima de `gmRemixStaminaGetTotalHP()`). Igual ao `_hp` do asm (`TOTAL_HP < percent`).

Mapa de status protegidos (asm -> decomp). Os ids do Remix batem com o valor do enum decomp `FTCommonStatus` para os nomes comuns (checado por script em 68 nomes; a exceção é `Sleep`, que no Remix é 0xA5 e no decomp é 4, ver abaixo).

| asm (Action) | id | Decomp | Nota |
|---|---|---|---|
| DeadD..ScreenKOWait | 0x000-0x004 | nFTCommonStatusDeadDown..nFTCommonStatusSleep | 0x004 é ScreenKOWait; o decomp chama esse valor de Sleep |
| DamageHi1..WallBounce | 0x025-0x038 | nFTCommonStatusDamageStart..nFTCommonStatusDamageEnd | Tumble (0x039) não é protegido |
| Tornado | 0x03C | nFTCommonStatusTwister | |
| Barrel | 0x03D | nFTCommonStatusTaruCann | |
| DownBounceD..DownWaitU | 0x043-0x046 | nFTCommonStatusDownBounceD..nFTCommonStatusDownWaitU | |
| CliffCatch..CliffEscapeSlow2 | 0x054-0x063 | nFTCommonStatusCliffCatch..nFTCommonStatusCliffEscapeSlow2 | ver desvio 1 |
| Grab..ThrownFoxB | 0x0A6-0x0BC | nFTCommonStatusCatch..nFTCommonStatusThrownFoxB | inclui ThrowF/ThrowB e capturas |
| DK Cargo..CargoThrowAir | 0x0EB-0x0F5 (DK e JDK) | nFTDonkeyStatusThrowFWait..nFTDonkeyStatusThrowAirFF | fkind REMIX_ID_DONKEY e REMIX_ID_JDK; valores locais no arquivo |
| Marina Cargo..CargoShake | 0x0EB-0x0FB (só Marina) | sem nome no decomp abaixo de 0xFA | fkind REMIX_ID_MARINA; valores locais no arquivo |

Não mapeados por nome: os ids 0xEB-0xF9 de Marina. O decomp só nomeia CargoJump (0xFA) e CargoShake (0xFB) em `ftremixmarinansp.c`; o intervalo usa os números do asm.

Desvios do asm:
1. Cliff: o asm checa 0x054-0x062 (o laço para antes de 0x063). O port inclui CliffEscapeSlow2 (0x063), como o próprio asm descreve ("CliffCatch..CliffEscapeSlow2"). Para o comportamento literal, tirar `nFTCommonStatusCliffEscapeSlow2` do intervalo.
2. Damage: a nota da f11 diz 0x25-0x39. O asm checa 0x25-0x38 e o Tumble (0x039) fica de fora. O port segue o asm, porque o próprio KO usa Tumble e ele precisa poder ser setado.
3. Heavy throws genéricos (0x07A-0x07D) NÃO são protegidos pelo asm. O lead listou "heavy item throws" como protegidos, mas o asm só protege o intervalo de DK até `DK.HeavyItemThrowF` (0x0F6, exclusivo). Os 0x0F6-0x0F9 de DK também ficam de fora. O port segue o asm. Proteger heavy throws é mudança de comportamento e precisa de decisão do lead.
4. Marina: o asm checa `Character.id.MARINA` (0x3F = REMIX_ID_MARINA). Polygon Marina (REMIX_ID_NMARINA, 0x54) não entra. Segue o asm.
5. JDK usa o mesmo intervalo de DK, como no asm.
6. Dead (0x000-0x004) inclui o 0x004 (ScreenKOWait), que o asm trata com teste separado. Mesmo resultado.

Não portado (REMIX-TODO(stamina-gigabowser), T6.2):
- Ramo de Giga Bowser (1P Remix), `stamina_ko_action` linhas 644-668 do asm: limite de HP 200/250/300 pela dificuldade (`0x800A493A`). Só aplica em 1P, então o port não o altera. O comentário está em `ftremixstaminaaction.c`.
- Checagem de tela (`Global.current_screen == 0x16`): o port não checa tela, pelo mesmo motivo de P6 e P13.

Verificação:
- `scripts/remix/syntax-check.sh` OK em `decomp/src/ft/ftremix/ftremixstaminaaction.c` e `decomp/src/ft/ftmain.c`.
- Nada foi compilado nem rodado. Build e jogo ficam com o lead.
- Include: `ft/ftchar/ftdonkey/ftdonkey.h` e `ft/ftchar.h` não compilam dentro de `ftremix` (`FTStatusDesc` incompleto), por isso os valores de DK e Marina são locais, como em `ftremixmarinansp.c`.
- ai-memory: regras globais (`_rules/`, `default/_global`) lidas. O escopo do projeto não resolveu: não há `.ai-memory.toml` em BattleShip e `JRickey/BattleShip` / `JRickey` não existem no servidor. As regras do projeto não foram lidas.

O que o lead deve ver na tela (partida Stamina, regra Timed Stock, HP baixo, ex.: 50):
- Lutador em HP zerado que leva um golpe que passa do HP: cai em DownBounceD no chão e em Tumble no ar.
- Agarre, captura e arremesso: a ação protegida termina antes do KO.
- Heavy throw genérico (desvio 3): deve virar KO, não protegido.

Restante:
- P9 já consta na seção acima. Giga Bowser: T6.2.

## Step P12: Poison and SpecialZoom Stamina TODOs

Files edited: `decomp/src/ft/ftremix/ftremixpoison.c`, `decomp/src/gm/gmremixspecialzoom.c`. No new files. The three
`REMIX-TODO(Stamina)` lines are gone (`grep REMIX-TODO(Stamina) decomp/src` is empty). Both files pass `scripts/remix/syntax-check.sh`.

Hooks (asm line -> decomp function):
- `Poison.asm:132-160` (`_poison_continue_2`, hazard poison) -> `ftRemixPoisonDamage` (clamp) and `ftRemixPoisonCanDamage` (skip).
- `css/PoisonDmg.asm:122-160` (`apply_damage`, CSS Poison Dmg panel) -> `ftRemixPoisonPanelDamage` (clamp at 999, then Stamina cap) and `ftRemixPoisonPanelTryApply` (skip).
- `SpecialZoom.asm:141-143` (`check_gamemode_2`, Stamina goes to the original function) -> `ftRemixSpecialZoomFinalKnockback`, early return before the decisive stock and offset update.

New helpers in `ftremixpoison.c` (static, no new files):
- `ftRemixPoisonIsStaminaDefeated(fp)`: `gmRemixStaminaIsActive() && gmRemixStaminaIsDefeated(fp->player)`.
- `ftRemixPoisonStaminaCap(percent)`: caps at `gmRemixStaminaGetTotalHP()` only when Stamina is active. Used by both damage paths.

Behaviour kept:
- Vanilla and non-Stamina battles: the cap returns the percent unchanged and the skip is FALSE, so nothing changes.
- The Stamina clamp matches the asm (`TOTAL_HP < new percent` -> `TOTAL_HP`). The hazard path has no 999 cap in the asm, and none was added.
- The CSS panel path skips a defeated fighter before the heal branch, as `apply_damage` does (the defeated check sits before `_check_type`). The skip returns FALSE, so the hazard poison runs, as `_return` jumps to `Poison.poison_hook._poison_check`.
- The hazard path skip sits after the respawn branch and does not reset the poison duration, as the asm `_exit_1` does.

Deviations from the asm:
- The asm checks the stock byte (`match_info` +0x2B < 0) for "defeated". The port uses `gmRemixStaminaIsDefeated`, as the brief asked. That flag is `percent_port` in the note (set in `ftremixstaminako.c:58` by the KO path, cleared by `gmRemixStaminaResetDefeated` in `scvsbattle.c:391`). Until P7/P9 the KO path is the only writer, so the skip is inert before those steps. Both should agree once the KO path sets the stock to -1 at the same point; check that in P9.
- SpecialZoom: the KO check file `gmremixspecialzoomko.c` needed no change, because the early return covers it.

Not verified: no build or game run (the lead does that). The Stamina skip needs a Stamina match with a poisoned fighter (acid hazard or CSS Poison Dmg) and a KO to see the defeated flag set. Input: a Stamina rule VS match with Spawned Fear acid or the CSS Poison Dmg option on, then check the HP does not go below 0 and the zoom does not show on KO.

## Step P10: end of match by the stock path (overkill_prevent, stamina_fix_1..5)

Source: `.toolchain/pkgs/smashremix-src/src/Stamina.asm` lines 880-1015 (stamina_fix_1..5) and 1515-1563 (overkill_prevent),
both read in full. Decomp: `if/ifcommon.c` (ifCommonBattleInitPlacement 2838, ifCommonBattleUpdateScoreStocks 2956,
ifCommonAnnounceEndMessage 3671, ifCommonBattleSetInterface 3628), `ft/ftcommon/ftcommondead.c` (ftCommonDeadUpdateRumble,
ftCommonDeadUpdateScore 53, ftCommonDeadInitStatusVars 223), `ifCommonTimerFuncRun` (ifcommon.c 2737). The brief's
`remixStaminaTryKO` does not exist; the KO is `ftRemixStaminaStoredDamage` (ft/ftremix/ftremixstaminako.c), called from
ftParamUpdateDamage (ftparam.c:1835).

Status: **no code changed in this step.** The stock path is already wired, and the reading below does not show why the
match did not end in the lead's run. Not built, not run.

### Where the stock goes to -1 (decided)

- Start: `ftmanager.c:1037` copies `desc->stock_count`, which is `gmRemixStockModeCount` = 0 in Stamina (gmremixstockmode.c).
  So every fighter starts at 0 and the first KO takes it to -1. `players[].stock_count` gets the same value.
- The -1 is taken at the KO moment, not at the end of the dead action: `ftRemixStaminaStoredDamage` -> `ftRemixStaminaKO` ->
  `ftCommonDeadInitStatusVars` -> `ftCommonDeadUpdateScore`. In that function the `SCBATTLE_GAMERULE_STOCK` block does
  `this_fp->stock_count--`, `players[].stock_count--`, then `ifCommonBattleUpdateScoreStocks(this_fp)`.
- TIMED_STOCK has the STOCK bit, so the block runs in Stamina.

### Overkill guard (asm overkill_prevent, 0x80113A24)

The asm decrements the place counter only when the fighter was not already marked in `percent_port`, otherwise it adds
one back and skips the decrement. The port gets the same result without a new hook, because a second death cannot reach the
counter:

1. `gmRemixStaminaSetDefeated` runs before the KO (ftremixstaminako.c), and `gmRemixStaminaIsDefeated` makes later hits add
   0 percent.
2. The KO leaves stock at -1. Any later death enters `ftCommonDeadInitStatusVars` through the guard at its top
   (`ftRemixStaminaIsOutOfStock`, ftcommondead.c), which sleeps the fighter before `ftCommonDeadUpdateScore`.
3. Blast-zone death procs use `ftRemixStaminaIsBlastRedeath` (P9) for the same reason.

So one stock is lost per defeated fighter, and no second score is counted. Verified by reading only.

### Why the match should end on paper

- `ifCommonBattleInitPlacement` (called from `ftParamInitGame`, ftparam.c:3145) sets `sIFCommonBattlePlace = teams - 1`.
  With 3 FFA fighters that is 2.
- In `ifCommonBattleUpdateScoreStocks`, each elimination (`teammates_remain == 0`) decrements the place. At 0,
  `ifCommonAnnounceEndMessage` runs and `ifCommonBattleSetInterface` sets `game_status = nSCBattleGameStatusEnd`.
- So two KOs in a 3 FFA match reach 0. The lead's run (Kirby alone, timer still at 2:09) contradicts this, so something in
  the chain differs in practice.

### Candidates checked by reading and ruled out

- FFA team mismatch: `desc.team = players[player].player` (scvsbattle.c:347, :638). Index based, so
  `current_team == fp->team` works for FFA.
- `stock_count > 0` checks: only in the team stock steal of `ft/ftcommon/ftcommonsleep.c` (34, 95). Not the match end.
- Vanilla placement init: runs in `ftParamInitGame` for every mode, so it cannot be Stamina only.

### Not confirmed (what the lead should check)

1. Whether the second KO reaches `ftCommonDeadUpdateScore`. A temporary log (remove after) in `ftRemixStaminaKO` and at the
   top of `ifCommonBattleUpdateScoreStocks` with `i`, `stock_count` per player, `teammates_remain` and
   `sIFCommonBattlePlace` would show it in one run.
2. Whether `ifCommonBattleInitPlacement` ran after the 3 fighters got their `pkind`. If it ran before, the place starts wrong.
   Vanilla would show the same, so check a vanilla 3 FFA stock match too.
3. Whether `game_status` reaches End and the match waits in `ifCommonBattleEndUpdateInterface`. This is less likely, since the
   timer should not run in that state, but it is cheap to log.

### What the lead must see on screen (env SSB64_VS_STAMINA=8, 3 fighters)

- Two fighters past HP: each leaves the HUD once. With the place log above, `sIFCommonBattlePlace` should be 0 after the second
  KO and the end announcement should play. If it does not, the log names the failing item (1 or 2).
- Kirby alone must not lose stock again on any later hit (no second score).

### Left open

- The stall cause is not identified. P10 needs runtime data from items 1 to 3 above before a fix is written.
- ai-memory: `_global` rules read (no conflict with this step). No project scope resolved for BattleShip, so project rules
  were not read.
