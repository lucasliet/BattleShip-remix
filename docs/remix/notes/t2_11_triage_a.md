# T2.11 triagem A: REMIX-TODO de ef/ e ft/

Triagem somente leitura dos 16 pontos `REMIX-TODO` listados. Referências de asm são arquivos em `.toolchain/pkgs/smashremix-src/src/` (abreviado como `src/`). Os endereços RAM entre parênteses vêm dos comentários do próprio asm.

Harness citado: `SSB64_VS_FKINDS`, `SSB64_VS_STAGE`, `SSB64_REMIX_TOGGLES`, `SSB64_REMIX_FORCE_ITEM`, `SSB64_REMIX_TRAINING`, `SSB64_REMIX_ITEM_LOG` (todos encontrados em `scripts/` ou `tools/`), mais scripts de input.

Resumo: READY 7, BLOCKED 6, NOT-OBSERVABLE 1 (hitbox, somente com toggles padrão), e 1 comentário obsoleto (golden gun) cuja remoção do TODO é READY.

---

## 1. `ef/efmanager.c:4315` (Pikachu DSP, ThunderAmp)

O asm escala o gfx do DSP connect de Pikachu pelo multiplicador do jogador: `src/Size.asm:1896-1915` (`pikachu.adjust_gfx_size_`, patch em 0x8010146C). Lê `Size.multiplier_table[port]` com `port` em `player+0x0D` (dentro de `Size.asm:1899-1915`) e escreve o mesmo valor em `0x1C` e `0x20` da estrutura de posição do gfx (x e y). Os callers da função C já têm o jogador: `ft/ftparam.c:2473` (dentro de `ftParamMakeEffect`, que recebe `fighter_gobj`, linha 2147) e `it/itremix/itremixlightning.c:217` (recebe `fp`). **READY, S.** Mudar a assinatura para receber o `port` (ou `FTStruct *fp`) e aplicar `ftRemixSizeGetMultiplier` nos campos de escala de `LBTransform` retornado por `lbParticleAddTransformForStruct`. Incerteza: o mapeamento de `0x1C/0x20` do gfx do asm para os campos de `LBTransform` não foi confirmado (UNKNOWN). Verificação: `SSB64_VS_FKINDS` com Pikachu e um multiplicador de tamanho via `SSB64_REMIX_TOGGLES`, capturar o efeito e comparar com o multiplicador 1.0.

## 2. `ef/efmanager.c:5102` (Ness PSI Magnet, DSP)

O asm troca o update do gfx por um wrapper: chama o update original (`0x8000DF34`) e depois escala a estrutura de posição a cada frame (`Size.asm:2014-2075`, `ness.dsp`, patch em 0x8010260C e update em 0xA9C1C+0x10). O multiplicador vem de `gfx -> player -> port`. Na port, `ep->fighter_gobj` já existe (`efmanager.c:5118`), e `dEFManagerNessPsychicMagnetEffectDesc` (`efmanager.c:1033`) usa `gcPlayAnimAll` como Proc Update. **READY, M.** Criar um wrapper de Proc Update para a descrição (com cópia ou variante da desc) que chama `gcPlayAnimAll` e depois aplica `DObj` scale por `ftRemixSizeGetMultiplier(fp->player)`. Incerteza: qual DObj corresponde à "segunda junta" do gfx do asm (UNKNOWN). Verificação: `SSB64_VS_FKINDS=Ness` com Ness usando PSI Magnet (input script) e comparar screenshot com multiplicador 1.0 (skill `remix-smoke-tests`).

## 3. `ef/efmanager.c:5260` (Ness PK Thunder Wave, USP)

Mesmo padrão do item 2: `Size.asm:1951-2010` (`ness.usp`), patch de update em 0xA9C94+0x10 e `_initialize` em 0x80102A3C (trecho próximo de `Size.asm:1951-1966`, linhas aproximadas). A parte de `_initialize` aplica escala também nos offsets (`Size.asm` do bloco `_scale`, mesmo trecho que o dsp). Só o início do bloco `usp` foi lido, não o trecho inteiro (parcial). **READY, M.** Mesmo wrapper de update usado no item 2, com um passo extra na criação para escalar offsets quando o gfx é inicializado. A função C é `efManagerNessPKThunderWaveMakeEffect` (`efmanager.c:5262`), chamada por `ft/ftchar/ftness/ftnessspecialhi.c:321`. Verificação: `SSB64_VS_FKINDS=Ness` com o USP (input script) e screenshot.

## 4. `ft/ftcommon/ftcommonitemshoot.c:64` (Golden Gun)

O comentário está **obsoleto**. O som do Golden Gun (0x04C2) já está no código (`ftcommonitemshoot.c:64`, ramo `ITREMIX_GOLDENGUN_ID`). O bloqueio citado (bala de ouro não portada e patches `projectile_struct_fix` 0x80175A6C e `projectile_properties_fix` 0x80175A8C) já foi portado: `decomp/src/it/itcommon/itlgun.c:357` cita os dois patches, `decomp/src/it/itremix/itremixgoldengun.h:20` declara o `golden_bullet_projectile_struct` como WPDesc e `itremixgoldengun.h:23` declara `projectile_properties_fix`. Referência asm: `src/items/GoldenGun.asm:192-250`. **READY, S.** Só atualizar ou remover o comentário depois de confirmar que a cadeia de chamada de `itlgun.c:357` está ativa (não lido por completo; UNKNOWN). Verificação: `SSB64_REMIX_FORCE_ITEM` com Golden Gun, atirar e checar dano e som.

## 5. `ft/ftcommon/ftcommonthrow.c:107` (PokemonAnnouncer, forward throw Kirby/Bowser)

Parte do TODO já está portada: o anúncio de Seismic Toss para J.Kirby existe em `ftcommonthrow.c:118-123` (`ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallSeismicToss)`). O que falta é o asm para BOWSER e GBOWSER: `src/jigglypuffkirbyshared.asm:272-300` (`kirby_fthrow_1_fix`, patch em 0x8014A238). Esse trecho seta `t3 = 0xE5` (`:283`) e faz `beq` para `_kirbyfthrow_1` quando o fkind é Bowser ou GBowser. `_kirbyfthrow_1` toca o anúncio e salta para a rotina vanilla `0x8014A24C`. **BLOCKED.** Não está claro (UNKNOWN) como `t3` é consumido pela rotina `0x8014A24C`, nem qual é o status/ação de Bowser na port. Desbloqueio barato: ler a rotina `0x8014A24C` e confirmar o uso de `t3`; depois portar o ramo de Bowser (S-M). Verificação: forward throw de Bowser em VS com input script.

## 6. `ft/ftremix/banjo/ftremixbanjousp.c:140` (Banjo USP wall splat, wall bounce)

O asm chama `atan2` (`0x8001863C`) com `f12 = -v0.x` e `f14 = v0.y` (`src/Banjo/BanjoSpecial.asm:1195-1206`), e depois `Size.ground_gfx.save_player_struct_._wall_bounce` (`:1206`). Esse hook está em `src/Size.asm:1424-1460` e usa `li at, 0x800FFD58`, que é `efManagerImpactWaveMakeEffect(pos, index, rotate)` (`ef/efmanager.c:3427`). **READY com ressalva, M.** Chamar `efManagerImpactWaveMakeEffect` em `ftRemixBanjoUSPWallSplatSetStatus` com o ângulo de `atan2` e escalar o efeito pelo multiplicador. Incertezas: de onde vem `v0` (`0x003C(sp)`, UNKNOWN), e a escala exata do ramo `_assign` (UNKNOWN). Verificação: `SSB64_VS_FKINDS=Banjo` com USP contra parede (input script).

## 7. `ft/ftremix/crash/ftremixcrashnsp.c:202` (Crash NSP, item spin e fling)

A função C `ftRemixCrashNSPSpinItem` está vazia. O asm tem `spin_detect_item_` (`src/Crash/CrashSpecial.asm:264-329`), chamado em `:365` e `:415`, e `item_fling_` (`:891-907`, chamado em `:388` e `:438`), com `flung_item_state_table` em `:1045`. A detecção usa a lista de itens na RAM (`lui 0x8004 / lw 0x6700`, ou seja, 0x80046700) e o bit de pickup em `item+0x2CE` (`:280-300`). Não há nenhuma função de "fling" na port fora deste arquivo. **BLOCKED.** Falta o `item_fling_` e a tabela `flung_item_state_table` (asm `:1045`, não lida). Desbloqueio: M-L, depende de como a port expõe a lista de itens. Verificação: `SSB64_REMIX_FORCE_ITEM` com Crash e spin no item.

## 8. `ft/ftremix/dragonking/ftremixdragonkingnsp.c:281` (lag procs do Kirby)

O asm grava dois ponteiros de função em `player+0x0A04` e `player+0x0A08` (`src/DragonKing/DragonKingSpecial.asm:885-886` e `943-944`) com `lui 0x800F` mais `addiu 0x9C8C` e `0x9CC4`. A port guarda esses campos como `proc_lagstart` e `proc_lagend`, e os preenche em `ef/efmanager.c:5725-5726` com `ftParamProcPauseEffect` e `ftParamProcResumeEffect` (`ft/ftparam.c:1664, 1676`). Os endereços 0x800F9C8C e 0x800F9CC4 não aparecem em `build/remixasm/remix.sym` (não existe esse arquivo; o símbolo é desconhecido). **BLOCKED.** Falta mapear os dois endereços para funções conhecidas (UNKNOWN). Desbloqueio: procurar no mapa vanilla ou na tabela de símbolos do Remix. Se os endereços forem `ftParamProcPauseEffect`/`ResumeEffect`, a correção é S. Verificação: Kirby no special do DragonKing com pausa de lag de hit (input script).

## 9. `ft/ftremix/ftremixdpad.c:8` e `:137` (D-pad, valores parciais e timer)

O asm usa valores parciais de stick de +-40 (0x28 e 0xD8) para um toque simples, e cheio de +-80 (0x50) para toque duplo ou D-pad segurado (`src/css/DpadFunctions.asm:163, 202-211`). Também usa `dpad_move_timer` por port (`:270-285`) e checagens de ação por personagem (`:296-340`, com Walk3, Turn e ações de Fox, Pikachu e Mystical Ninjas). A port sempre usa o valor cheio. **BLOCKED (parcial).** O estado do timer (por port) pode ser feito em C (sem RAM). Faltam as ações específicas por personagem: o asm `:240-440` não foi lido inteiro (UNKNOWN). Desbloqueio: M, após ler o trecho completo. Verificação: input script com toques de D-pad e comparar a velocidade de andar com o valor de stick esperado.

## 10. `ft/ftremix/ftremixentry.c:455` (Size, gfx do entry)

Os efeitos de entrada não são escalados. O asm tem hooks para Mario (pipe, 0x80103768), Donkey (barrel, 0x8010345C), Samus (portal, 0x801034B8), Link (ground 0x80102B28 e shaft 0x80102B84), Kirby (estrela, 0x80102C10) e Blue Falcon/Clown Car (0x80103670) em `src/Size.asm:2305-2410`. As rotinas `_top_joint`, `_top_joint_no_y`, `_2nd_joint` e `_common` estão em `Size.asm:2410-2470` e escalam x, y, z por `Size.multiplier_table[port]`. Na port, `ftRemixEntryMakeEffect` chama as funções `efManager*MakeEffect` em `ftremixentry.c:350-448` e descarta o retorno (exceto o ovo de Yoshi, que já usa `ftRemixSizeScaleEntryEgg` em `:421`). **READY, M.** Capturar o `GObj*` retornado e aplicar a mesma escala uniforme de `DObj` que o ovo de Yoshi. Incertezas: mapear a "junta de topo" do asm para o `DObj` (provado só para o ovo), e os ramos especiais de Link/Banjo/Goemon (`_render_override`, `_2nd_joint`), UNKNOWN. Verificação: `SSB64_VS_FKINDS` com Mario, DK, Samus, Kirby e Captain, screenshot da entrada.

## 11. `ft/ftremix/ftremixhitbox.c:4` e `:20` (display de hitbox)

Os patches de `Hitbox.asm` não estão portados. A parte de display do fighter e o Model Display estão em `ftremixhitboxdisplay.c`. O resto (`projectile_`, `grab_immunity_and_armor_frames_`, `make_hurtboxes_transparent_`, `render_hitbox_plus_*`, `fix_hitbox_position_`) é código de depuração. Com os toggles padrão (Special Model Display e Advanced Hurtbox Display, ambos FALSE), a maior parte é no-op, exceto as escritas de estado de display. **NOT-OBSERVABLE** com os toggles padrão. Com Advanced Hurtbox ligado, fica **BLOCKED**: falta a fonte de cor do hurtbox vanilla (`t5`), e os construtores de display list não estão portados (ver `docs/remix/notes/f14_hitbox_display.md` citado no próprio arquivo). Verificação dos toggles ligados: `SSB64_REMIX_TOGGLES` com Advanced Hurtbox.

## 12. `ft/ftremix/ftremixshield.c:7` (Costume 17 e cor do ovo de Yoshi)

A tabela `table_custom` com 16 cores (Default, Red até White) está em `src/Shield.asm:159-180`. `color_fix_` (`Shield.asm:41-138`) usa `Character.costume_shield_color.table` para o modo Costume (17) e um ajuste de Sonic clássico (`:86-93`). Essa tabela é gerada por `set_costume_shield_colors` em `src/Character.asm:1115+` (`:342-343`). A cor do ovo de Yoshi (`alter_yoshi_egg_color_`, `Shield.asm:187-212`) escolhe um índice de paleta do ovo por `Render.MODEL_PART_IMAGE_INIT_`. **BLOCKED.** Costume: falta copiar a tabela de bytes por personagem para a C (S-M, dado literal). Ovo: falta a rotina `Render.MODEL_PART_IMAGE_INIT_` na port (feature de engine ausente, M-L, confirmado no comentário; não verifiquei a ausência por busca). Verificação: `SSB64_VS_FKINDS` com cor de shield em modo Costume.

## 13. `ft/ftremix/ftremixsizefx.c:8` (Size, GFX restantes)

Lista de pendências do próprio arquivo, cada uma com hook diferente: ovo de Yoshi ao sair (`src/Size.asm:1355-1400`, patch em 0xC73D4, `adjust_size_egg_lay_`); tiro de Samus (`Size.asm:1536-1550`, render wrapper em 0xA98B4+0x14); render da espada do USP de Kirby (`Size.asm:1605-1625`); update do chute de Falcon (`captainshared.asm`); e a tabela base do USP de Kirby (`kirby_ausp_`, `Size.asm:293-336`). **BLOCKED** (multi-item). O item mais barato é o ovo de Yoshi ao sair (M): o asm roda a cada frame sobre o jogador capturado, então precisa de um hook por frame na port que não existe hoje (UNKNOWN o ponto de chamada exato). Verificação: `SSB64_VS_FKINDS` com Yoshi e egg lay.

## 14. `ft/ftremix/ftremixsizemove.c:76` (Pwing, 0.8 no pulo aéreo)

O asm multiplica a altura do segundo pulo por 0.8 enquanto `Item.Pwing.pwing_jump_flag[port]` está ligado (`src/Size.asm:740-790`, `_check_pwing` em `:782`, constante 0x3F4CCCCD). A flag é byte por port em `src/items/Pwing.asm:447`, lida em `:189`, `:259` e `:266`. O local exato onde a flag é escrita em 1 não foi encontrado (UNKNOWN). A função C `ftRemixSizeAerialJumpHeight` tem caller real (`ft/ftcommon/ftcommonjumpaerial.c:178` e `:255`) e hoje só aplica o multiplicador de tamanho, sem o Pwing. A port tem o estado equivalente em `it/itremix/itremixpwing.c:75` (`jump_state`, com `nITRemixPwingJumpJumping` em `:44`), mas não há acessor exportado. **READY, S.** Expor em `itremixpwing.h` uma função do tipo `itRemixPwingIsJumping(port)` e, em `ftRemixSizeAerialJumpHeight`, multiplicar por 0.8F quando ela for TRUE. Incerteza: se o índice do slot de `itremixpwing.c` é o mesmo que o port (UNKNOWN; conferir antes). Verificação: `SSB64_REMIX_FORCE_ITEM` com Pwing e segundo pulo com input script; comparar altura com e sem o item.

---

## Pendências desta triagem

- Não foi possível acessar a memória do projeto BattleShip no ai-memory (`workspace` "JRickey" e `project` "BattleShip" não existem; o remote `origin` é `JRickey/BattleShip`). As regras globais (`_global`) foram lidas.
- Nenhuma função foi compilada nem o jogo foi executado. Nada foi alterado no repositório, exceto este arquivo.
