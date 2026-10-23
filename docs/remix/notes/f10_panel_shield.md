# F10 Shield (Remix CSS panel, ROADMAP T5.2)

## Alterações

- `decomp/src/ft/ftremix/ftremixshield.c` (novo): `ftRemixShieldGetEnvColor(port, is_damage_shield, vanilla)`. Lê
  `gmRemixPanel(SHIELD, port)`. Retorna a cor vanilla para o valor 0 (Default), 16 (Vanilla), 17 (Costume), para o flash de
  dano do escudo e para portas fora de 0..3. Para 1..15 retorna a cor da tabela.
- `decomp/src/ef/efmanager.c`, `efManagerShieldProcDisplay` (RAM `0x80101024`): a cor do env passa a vir de
  `env` (cópia de `dEFManagerShieldColors[id].env`), trocada sob `#ifdef REGION_REMIX` pela função acima. A cor prim
  continua vanilla. Sem REGION_REMIX o código é o mesmo de antes.
- `decomp/src/ft/ftremix/ftremixhooks.h`: declaração anexada ao final.

## Tabela de cores

Linhas 1..15 de `table_custom` em `.toolchain/pkgs/smashremix-src/src/Shield.asm` (linhas 219..233 do texto), copiadas
como constantes RGB literais. Nenhum byte de `Character.asm` foi copiado.

## Origem no asm

- `src/Shield.asm` `color_fix_` (ROM `0x0007C8E8`, RAM `0x801010E8`), dentro de `efManagerShieldProcDisplay`
  (`0x80101024`). Corresponde à cor do env do escudo.
- Cor de dano (`is_damage_shield`, id 4): mantida vanilla de propósito. O asm não separa o flash, mas perder o flash branco
  de dano é um efeito de feedback que não estava no pedido. Desvio consciente, registrado aqui.

## Pendente

- **Costume (17)**: não portado. A tabela `Character.costume_shield_color` tem as linhas vanilla como bytes literais
  (`db 0x01, 0x03, ...` em `src/Character.asm`, ex.: MARIO, FOX, DONKEY, SAMUS). Isso é dado de ROM e não foi copiado. Só as
  linhas Remix (`set_costume_shield_colors`, ex.: `PURPLE, RED, TURQUOISE...`) são símbolos. Falta decidir como portar a
  tabela sem copiar bytes derivados da ROM. Marcado como `REMIX-TODO(Shield)` em `ftremixshield.c`.
- **Ovo do Yoshi com cor escolhida**: não portado. `alter_yoshi_egg_color_` (`Shield.asm` linhas 188..) escolhe um índice de
  paleta via `Render.MODEL_PART_IMAGE_INIT_` e `v0+0x88`, e o port não tem esse caminho. Tingir o env do ovo com a cor
  escolhida foi descartado: no vanilla o env do ovo é o dano (cresce com o dano), então a cor não apareceria com vida cheia.
  `efManagerYoshiShieldProcDisplay` mantém o tom vanilla (já tem `ftRemixSizeScaleYoshiShield`, de outro escopo).
- **Default (0) não aplica a cor de time/porta do Remix**: o `color_fix_` do asm usa cor por porta/time para o valor 0. Pelo
  pedido, 0 precisa ser byte a byte vanilla, então essa lógica não foi portada. Se o usuário quiser o Remix de verdade, é
  outro passo.

## Verificação

- `scripts/remix/syntax-check.sh` OK para `ftremixshield.c` e `efmanager.c`.
- Build e jogo não rodados (responsabilidade do lead). Teste sugerido: opção 1..15 em uma porta, escudo com a cor
  escolhida; opção 0 e 16 iguais ao build anterior; flash de dano branco.
- O aviso `-Wcomment` no syntax-check vem de `port/remix/remix_player_panel.h` (`/*` dentro de comentário de bloco), que não
  é deste escopo.
