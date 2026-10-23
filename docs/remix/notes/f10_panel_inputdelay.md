# F10 painel: InputDelay (Input Delay, 0 a 12 frames)

Referência do asm: `.toolchain/pkgs/smashremix-src/src/InputDelay.asm` (`apply_input_delay_` em linha 37, `MAX_FRAMES` 12 na linha 14, `delay_table` por porta). O original escreve as amostras do controle no buffer da porta (hook em 0x4BF8) e lê o frame `delay` atrás. Valor 0 mantém o comportamento vanilla.

## Port

Arquivos:
- `decomp/src/ft/ftremix/ftremixinputdelay.c` (novo): ring de `SYController` por porta, `MAX_FRAMES + 1` slots. `ftRemixInputDelayController` grava a amostra atual e devolve a que foi gravada `gmRemixPanel(INPUT_DELAY, player)` frames antes. Com delay 0 devolve o ponteiro original. `ftRemixInputDelayReset` zera o ring da porta.
- `decomp/src/ft/ftmain.c` em `ftMainProcUpdateInterrupt` (0x800E1260), caso `nFTPlayerKindMan`: `controller = ftRemixInputDelayController(this_fp, controller);` logo depois de `controller = this_fp->input.controller;`, antes de ler `button_hold` e `stick_range`. O resto da função (SBM filter, R_TRIG, taps, releases) não muda.
- `decomp/src/ft/ftmanager.c` em `ftManagerInitFighter` (0x800D79F0), dentro do bloco `REGION_REMIX` após `ftRemixKnockbackRandomizeAngles(fp)`: `ftRemixInputDelayReset(fp);`. Ponto de reset por partida: cada lutador é criado no início da partida.
- `decomp/src/ft/ftremix/ftremixhooks.h`: bloco anexado com as duas declarações.

Só lutadores humanos (`nFTPlayerKindMan`) passam pelo caminho. CPU, demo e teclas não são afetados. Em 1P e Bônus, `gmRemixPanel` já devolve 0 para portas não humanas.

## Decisões

- O buffer é por porta, não por lutador: `fp->player` (0..3), como no asm.
- O ring avança uma vez por chamada de `ftMainProcUpdateInterrupt`. Essa função só roda quando `is_control_disable` é 0, então o atraso conta frames com controle habilitado, não frames de relógio. Se isso divergir do asm em situações de desabilitação de controle (ex.: lag, cutscenes de morte), é um ponto a validar no jogo.
- Antes do primeiro frame atrasado o ring está zerado: entradas neutras, como no asm, que zera o buffer.
- Faixa: o painel já limita 0..12 (`port/remix/remix_player_panel.cpp`, linha 65). Não há clamp no código C.

## Verificação

- `scripts/remix/syntax-check.sh` em `ftremixinputdelay.c`, `ftmain.c` e `ftmanager.c`: `syntax-check OK`. Os avisos de `/*` dentro de comentário vêm de `port/remix/remix_player_panel.h` e `gmremixpanel.h`, já existentes.
- Build e jogo ficam com o lead, conforme o brief.

## Pendente

- Teste em partida: delay 0 deve ser idêntico ao vanilla; delay 6 ou 12 deve atrasar stick e botões de forma visível no lutador humano, sem afetar CPU.
- Confirmar o comportamento com `is_control_disable` (ver Decisões).
- Sem `REMIX-TODO` para InputDelay no código: nenhum existia.
