# F10 painel: Taunt Button (TauntButton do CSS)

Referência do asm: `.toolchain/pkgs/smashremix-src/src/Joypad.asm` (`set_taunt_mask_` linha 389, `prevent_taunt_jump_` linha 428,
`taunt_mask_per_port` linha 384, `mask_table` após set_taunt_mask_). Valores: 0 L (vanilla), 1 C-Up, 2 C-Down, 3 C-Left,
4 C-Right, 5 D-Up, 6 D-Down, 7 D-Left, 8 D-Right.

## Código tocado

- `decomp/src/ft/ftremix/ftremixtauntbutton.c` (novo, dono do arquivo):
  - `ftRemixTauntButtonMaskFor`: máscara do taunt por lutador. Só `nFTPlayerKindMan`; com `single_button_mode` ligado
    mantém a máscara vanilla (igual ao `set_taunt_mask_`); valor do painel 0 mantém a vanilla; fora de 0..8 loga com
    `port_log` e usa a vanilla.
  - `ftRemixTauntButtonAllowsJump`: equivale ao `prevent_taunt_jump_`. Pega os bits de C pressionados e os compara com a
    máscara do taunt: o pulo é negado só quando os C pressionados são exatamente o botão de taunt. Sem C pressionado retorna
    FALSE; máscara de D-pad ou L retorna TRUE (pulo normal, como no asm).
- `decomp/src/ft/ftmanager.c`, `ftManagerMakeFighter`, atribuição de `fp->input.button_mask_l`: `#ifdef REGION_REMIX` chama
  `ftRemixTauntButtonMaskFor`; sem a flag, linha vanilla.
- `decomp/src/ft/ftcommon/ftcommonkneebend.c`, `ftCommonKneeBendCheckButtonTap` (0x8013F450): asm ROM 0xB9E94 / hook em
  0x8013F454 (`andi` do asm, usa `button_tap`). Com REGION_REMIX retorna `ftRemixTauntButtonAllowsJump(fp, button_tap)`.
- `decomp/src/ft/ftcommon/ftcommonjumpaerial.c`, `ftCommonJumpAerialMultiCheckJumpButtonHold` (0x8014012C): asm ROM 0xBAB70
  (Kirby/Puff 3º pulo em diante) / hook em 0x80140130 (usa `button_hold`). Mesmo tratamento.
- `decomp/src/ft/ftremix/ftremixhooks.h`: declarações novas no fim, antes do `#endif`.

Endereços conferidos com `build/remixasm/refs/vanilla_funcs.txt` (0x8013F450 e 0x8014012C).

## Decisões

- A regra do asm depende da tabela `taunt_mask_per_port`. O port usa `fp->input.button_mask_l` como fonte única: o
  `ftRemixTauntButtonAllowsJump` lê essa máscara. Assim o pulo e o taunt sempre usam o mesmo botão.
- Premissa: a máscara vanilla de `desc->button_mask_l` é o L trigger (comentário em `fttypes.h:1571`), sem bits de C. Com
  essa premissa o padrão 0 é byte a byte igual ao vanilla. Não consegui confirmar o valor numérico nos dados de descrição
  (vêm de dados carregados, não de código); o lead deve confirmar em jogo.
- Single button mode: o asm desliga o remap; o port usa `gmRemixToggle(single_button_mode)` do mesmo jeito.

## O que falta / não coberto

- `css/DpadControl.asm` (`set_shield_mask_`) lê `taunt_mask_per_port` para não colidir com o escudo no Z. Não portado
  (opção DpadControl fora deste escopo). Pode haver colisão entre taunt em D-pad e DpadControl.
- TauntItem (painel próprio) não está ligado ao taunt: fora de escopo.
- Limpeza 1P (`css/TauntButton.asm:45`): não há código; `gmRemixPanel` já devolve 0 para não humanos nos modos Bonus e 1P.
- Nenhum `REMIX-TODO` de taunt existia, então nada foi removido.

## Para o lead testar (não rodei build)

- Painel 1 (C-Up): taunt com C-Up, sem pulo; C-Up junto de outro C dá pulo.
- Painel 5 (D-Up): taunt com D-Up, pulo com C normal.
- Painel 0 e single button mode: comportamento vanilla.
- CPU: taunt vanilla, sem remap.
