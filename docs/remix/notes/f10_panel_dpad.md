# F10 painel: D-pad Control (DpadControl) e D-pad Map (DpadFunctions)

Referência do asm: `.toolchain/pkgs/smashremix-src/src/css/DpadControl.asm` (`css_cursor_and_dpad_controls_` linha 135,
`set_R_macro` linha 98, `check_jump_type_idle_` 505, `check_jump_type_running_` 545, `dash_attack_check_` 638) e
`css/DpadFunctions.asm` (`dpad_macro_check_` linha 35, `dpad_macro_check_held_` 551). Valores: DpadControl 0 Off, 1 Stick
Swap, 2 Stickless, 3 Stick Swap J, 4 Stickless J. DpadFunctions 0 Off, 1 Smash, 2 Tilt, 3 Special.

## Feito: DpadControl (transformação do controle)

- `decomp/src/ft/ftremix/ftremixdpad.c` (novo, dono do arquivo): `ftRemixDpadControlController(fp, controller)`. Só lutadores
  humanos (o caminho `nFTPlayerKindMan`). Valor 0 devolve o ponteiro original. Usa um `SYController` sintético por porta.
  - R macro (`set_R_macro`, modos 1..4): R vira A+L em vez de A+Z. O bit de R é removido porque `ftMainProcUpdateInterrupt`
    expande R para A+Z.
  - Stick Swap (1, 3): o direcional vira bits de D-pad (deadzone X 40, Y 30, como o asm). O D-pad físico vira stick em valor
    cheio (+-80), e o stick original é descartado.
  - Stickless (2, 4): o stick original é zerado. O D-pad físico vira stick em valor cheio, e os bits de D-pad são mantidos.
- `decomp/src/ft/ftmain.c`, `ftMainProcUpdateInterrupt`, caso `nFTPlayerKindMan`: uma linha logo após
  `ftRemixInputDelayController`, dentro do bloco `REGION_REMIX`. A ordem é a do asm: o atraso (hook em 0x4BF8) vem antes da
  troca (0x4C00). O filtro de Single Button Mode continua depois, sobre o `button_hold` já transformado.
- `decomp/src/ft/ftremix/ftremixhooks.h`: declaração anexada ao final.

## Interpretação do asm (confirmar em jogo)

- A intenção descrita no próprio asm ("Dpad e Stick são trocados") é a implementada: o stick físico vira D-pad e o D-pad
  físico vira stick. Li o caminho do macro (`_dpad_macro_move_setup` lê o stick cru do struct do controle) e não consegui
  confirmar com certeza de onde vem o stick de saída no modo Stick Swap. Se o jogo mostrar outro comportamento, o ponto a
  revisar é a origem do stick em `ftRemixDpadControlController`.
- Os modos J (3, 4) usam a mesma transformação. A diferença de J está só nos pulos e no dash attack (abaixo).

## REMIX-TODO(DpadControl): não portado, com motivo

- Valor parcial do D-pad (+-40 em um toque único, +-80 em toque duplo ou D-pad segurado) e o timer `dpad_move_timer` do
  macro (`DpadFunctions.asm` `_dpad_move_*`). O port usa sempre valor cheio, o que faz o andar por toque ficar mais rápido que
  no asm. Precisa do estado do timer por porta e de ids de ação do lutador (Walk2, Walk3, Turn, Action.FOX.*, etc.), que não
  estão expostos no decomp desta escopo. Também não há o acréscimo de +40 no ar (`_dpad_move_check_up`).
- CSS (`css_cursor_and_dpad_controls_` nas telas de seleção, D-pad como cursor): fora do escopo de partida.
- Pausa de treino (`_training_pause_control`): não portado.

## Feito (T8.2, M11 Fase 4): D-pad Map e ações do DpadControl

- D-pad Map (`DpadFunctions.asm` `dpad_macro_check_` e `dpad_macro_check_held_`): `ftremixdpadmaprules.*` (regras puras, 74
  checagens em `tests/remix/test_dpad_map_rules.c`) e `ftremixdpadmap.c` (cola). Dois ganchos em `ftMainProcUpdateInterrupt`
  (caso `nFTPlayerKindMan`): `ftRemixDpadMapApplyPress` depois da máscara de toque (gancho 0x800E1330: troca o toque, pode
  escrever o stick e põe A no segurado) e `ftRemixDpadMapHeldButtons` antes de guardar o segurado (gancho 0x800E1378: Special
  segura B, Smash segura A com Charged Smashes ligado). Desligado com Stickless, Single Button Mode (só no toque) e no Training
  com "D-pad Controls" diferente de OFF.
- Os modos J não têm código próprio: no asm, `check_jump_type_idle_`, `check_jump_type_running_` e `dash_attack_check_` só agem
  nos valores 1 e 2 (`slti at, at, 3`), os J (3 e 4) seguem o vanilla. A nota antiga dizia o contrário. Portado em
  `ftremixdpad.c` (`ftRemixDpadControlDisablesStickJump`, `ftRemixDpadControlHandlesRunStickUp`,
  `ftRemixDpadControlSkipsDashAttackOf`) com ganchos em `ftcommonkneebend.c`, `ftcommonrun.c` e `ftcommonattackdash.c`.
- `set_shield_mask_`: `ftRemixDpadControlSetButtonMasks` (chamado em `ftManagerMakeFighter` depois da máscara de taunt): escudo
  em L, taunt padrão em Z; com Taunt Button personalizado o escudo responde a L e Z.

## Verificação

- `scripts/remix/syntax-check.sh` em `ftremixdpad.c`: `syntax-check OK`. Em `ftmain.c`: `syntax-check OK` (os avisos são
  pré-existentes, de linhas fora do hook).
- Build e jogo ficam com o lead, conforme o brief.

## Para o lead testar

- Stick Swap (1): stick para a direita vira D-pad direita (o botao, para ataques e movimentos). D-pad para a direita vira stick
  cheio para a direita (movimento).
- Stickless (2): stick sem efeito; D-pad anda.
- R segurado com DpadControl ligado: A+L, sem Z.
- Painel 0: byte a byte vanilla (o ponteiro original volta).
- CPU e modos 1P/Bônus: nenhum efeito (o caminho é só humano, e `gmRemixPanel` devolve 0 nesses modos).
