# T2.11 triage C: REMIX-TODO em gr/grremix, if/ifcommon e mn/mnremixgallery

Triagem somente de leitura. Referências de asm são de `.toolchain/pkgs/smashremix-src/src/`. Classificação: READY (pode ser implementado com o que a port tem), BLOCKED (falta algo nomeado), NOT-OBSERVABLE (sem efeito visível). UNKNOWN quando não foi possível determinar.

## 1. gr/grremix/grremixdreamland.c:31 (screen id). READY (com ressalva)

O C usa `gSCManagerSceneData.scene_curr == nSCKind1PGame` (`sc/scdef.h:188`). O asm compara `Global.current_screen`, em que o id 0x01 (`Global.asm:60`) cobre title, game over 1P, batalha 1P e Remix 1P. A diferença só aparece se o Whispy Hyper rodar na tela de título ou no game over, o que não está verificado (UNKNOWN). Para fechar: trocar a função por um teste explícito de screen, ou confirmar que nesses estados o hazard não roda. Esforço S. Verificação headless: VS em Dream Land com Whispy em Hyper (toggle `whispy_mode`, pelo override SSB64_REMIX_* correspondente, nome a confirmar em `gm/gmremixtoggle`) e comparar com 1P.

## 2. gr/grremix/grremixjapes.c:24 (f20 no rushing_water). BLOCKED

`Hazards.asm:1950` faz `sub.s f2, f0, f20`. Dentro de `rushing_water` (1870..1955) f20 não é escrito, então é o valor deixado pelo chamador. O valor real é UNKNOWN; a port usa 0.0 como suposição. Para desbloquear é preciso achar o chamador do loop de hazards ou medir f20 em runtime, o que não é barato.

## 3. gr/grremix/grremixmetallicbeelaser.c:173 (FTStruct, palavra 0x18C). BLOCKED

`Hazards.asm` em `robot_bee_laser_collision_` (começa em 5135) lê a palavra inteira de `0x18C` e compara com `0x04000010` (reflect) e `0x08800110` (psi magnet). A port testa só os bitfields `is_reflect` e `is_absorb` (`ft/fttypes.h:1516` e `1518`). A diferença aparece quando outros bits da palavra estão ligados. O mapeamento de bits não fechou: `ftremixmarinadsp.c:91` diz que reflect é o bit 5, mas `0x04000010` tem o bit 4 (0x10). Desbloqueio barato: confirmar a posição de cada bitfield de `fttypes.h` contra a palavra e decidir se a comparação exata é necessária.

## 4. gr/grremix/grremixpiratecannonball.c:80 (velocidade de lançamento). BLOCKED

O asm grava o hitbox do cannonball em `Hazards.asm:3734-3737` e cria o projétil com `cannonball_projectile_struct` (`cannon_stage_setting_`, 4149..4182). O asm não escreve velocidade direta: ela vem das propriedades do projétil (speed 100, ângulo 270, gravidade 0, citadas no comentário). A port não tem caminho de velocidade por ângulo em `wpManagerMakeWeapon`, então o cannonball nasce parado. Falta a convenção de ângulo e facing do sistema de projéteis. Esforço M.

## 5. gr/grremix/grremixpiratecannonball.c:122 (cópia da bomba do Samus). BLOCKED

`Hazards.asm` em `cannonball_explosion_` (4268 em diante) copia 0x38 bytes de ROM em 0xE396C e 0x28 em 0xE39A8 e depois zera `0x0290(v0)`. A port tem a lógica equivalente do 0x80168F2C em `wp/wpsamus/wpsamusbomb.c:61` (`wpSamusBombExplodeInitVars`). O conteúdo exato dos bytes copiados não foi desmontado, então é UNKNOWN. Para desbloquear: desmontar `baserom` nesses dois endereços e comparar com a função da port. Esforço S a M.

## 6. gr/grremix/grremixrainbowchomp.c:43 (monitor de hitbox e som). Dividido: som READY, hitbox BLOCKED

`Hazards.asm` em `chainchomp_hitbox_main_` (6046..6112):
- Som: se o contador de frames em `a0+0x0E` for menor que 15 e `Global.get_random_int_(80)` der 2, toca FGM 0x401 (`6087..6098`). A port já tem `GRREMIX_RAINBOW_FGM_CHOMP_SPAWN 0x0401` em `grremixrainbow.c:25`, então o som é READY (S). A sequência aleatória só bate se o RNG da port for o mesmo.
- Hitbox: decrementa três contadores em `a2+0x29C` e limpa os slots em `a2+0x214` (`6058..6078`). A port não mapeia esse layout de hitbox do projétil, então é BLOCKED (M).

Verificação: sons de chomp em Rainbow Road (SSB64 smoke, stage Rainbow Road, cenas longas).

## 7. gr/grremix/grremixscuttle.c:70 (paleta do estágio). BLOCKED

`grRemixScuttleStagePaletteIndex` retorna 0, então a transição de fundo nunca começa. O asm percorre a cabeça do grupo 2 em RAM `0x800466F8`, depois o joint do topo, o primeiro joint (+0x10), a struct da imagem da parte (+0x80) e lê o índice da paleta (+0x88). Isso exige ler a árvore de objetos viva. Não é um dado de imagem estática, então `remix_ram_image` não resolve. Falta mapear `0x800466F8` para as estruturas de gobj/joint da port. Esforço M a L.

## 8. gr/grremix/grremixscuttle.c:198 (objeto de imagem do fundo). BLOCKED

`_check_max` (Scuttle) grava o endereço do fundo no objeto de imagem em RAM `0x80046800 +0x74 +0x44`. A port não tem esse objeto de fundo. Falta o objeto de imagem de fundo na port e as referências de arquivo do estágio. Mesmo bloqueio do item 7 para a parte da paleta.

## 9. gr/grremix/grremixspawnedfear.c:55 (overlay Practice). NOT-OBSERVABLE (provável)

`Hazards.asm` em `spawned_fear_acid_main_` (8512..8560) zera `0x0A20`, `0x0A24`, `0x0A30` e `0x0A88` do player antes do KO. Nenhum código C da port lê esses offsets (a busca em `decomp/src` achou apenas o comentário). O overlay da port fica em estado estático por jogador em `ft/ftremix/ftremixoverlay.c` (`sFTRemixOverlayIsSet`, `sFTRemixOverlayWasAir`), e `ResetLastFrame` já zera a flag de cor a cada frame. Por isso o efeito é provavelmente nulo. Não foi testado. Verificação: KO em doom acid com overlay de cor ligado, comparando a cor no frame seguinte ao KO.

## 10. gr/grremix/grremixtimetwister.c:62 (T1.5b, caminhada do warp ball). BLOCKED

`grRemixTimeTwisterFindWarpBgIndex` retorna -1, então a transição nunca inicia. A caminhada (primeiro objeto do grupo 2, quatro irmãos adiante, textura do material) depende do mesmo grupo 2 do item 7. Também depende do objeto de fundo do item 8, porque a transição sem a troca de fundo só faria a tela piscar. Esforço M a L.

## 11. gr/grremix/grremixtimetwister.c:73-74 (câmera e BGM). BLOCKED

- Câmera: `Hazards.asm:7888` chama `Camera.zoom_to_max_` (`Camera.asm:1447`), que troca a rotina da câmera (`gGMCameraStruct`, campo 0xC). A port não tem essa troca de rotina demonstrada (UNKNOWN sobre o campo). Esforço M.
- BGM: o asm faz fade do som do estágio e inicia a próxima faixa no frame 300. Falta o estado de faixa (`BGM.current_track` e `BGM.default_track`), que é RAM de runtime e não está na imagem somente leitura. Também falta uma função de troca de música na port. Esforço M a L.

## 12. gr/grremix/grremixtimetwister.c:103 (stage header e fundo). BLOCKED (depende do item 10)

O write da cor do vidro em stage header `+0x4C` já existe na port: `grremixscuttle.c:188` (`grRemixScuttleFadeWriteGlassColor`). Pode ser reutilizado (S). Ponto a checar: o comentário de Time Twister diz que o vidro muda no alpha máximo, enquanto o de Scuttle diz que muda no alpha do meio (`_in_transition`). A diferença precisa ser conferida no asm `_bg_fade` antes de reutilizar. Como a transição nunca inicia (item 10), o código não roda hoje. A parte do fundo depende do item 8.

## 13. if/ifcommon.c:3390 (BGM do pause). BLOCKED

`Pause.asm:433` (`set_alternate_music_`) lê `Toggles.entry_play_music` (símbolo presente em `port/remix/remix_symbols.h:4100`), `BGM.vanilla_current_track` e o timer de ciclo do dpad. Depois troca a faixa. A port não lê o estado de faixa nem tem a troca de música. O estado de faixa é RAM de runtime, então não serve o valor da imagem. Esforço M a L.

## 14. gr/grremix/grremixyoshiclouds.c:70 (tabela de hazard dos estágios). READY (S)

A port usa uma lista fixa em `grRemixStageMovesAsHazard` (Onett, Tal Tal, Tal Tal Remix). O asm (`Stages.asm:1667-1682`) lê `stage_hazard_table[stage_id]` e testa `Hazards.type.MOVEMENT` (0x02, `Hazards.asm:19`). A tabela existe em `REMIX_SYM_Stages__stage_hazard_table` (0x8045B4A8, `remix_symbols.h:4038`) e tem um byte por stage id (`Stages.asm:4586`). Dá para ler com `remix_ram_image_read_byte`. Ponto a confirmar: o mapeamento de `gkind` da port para o `stage_id` do asm (`Global.match_info +1`). O flag `dont_freeze_stage` é RAM de runtime (`Stages.asm:5490`), então não deve ser lido da imagem. Verificação: `SSB64_VS_STAGE` num estágio sem bit MOVEMENT e comparar a plataforma com movimento ligado e desligado.

## 15. mn/mncommon/mnremixgallery.c:12 (fighter kinds da galeria). BLOCKED

`Gallery.asm:445-492` tem 40 entradas `Character.id`. A port mapeia 12 para `nFTKind*` vanilla, que indexam `dMNCongraPictures` (`mncongra.c:27`). Faltam os fighter kinds dos 28 Remix na port (não há `nFTKindSynth`/Remix no `decomp/include`) e as imagens de congratulações desses personagens (`mncongra.c:284-310` tem um caminho de CE). Desbloqueio: enum de fighter kinds Remix e os assets de imagem. Esforço L.

## Resumo

| Local | Classificação | Esforço |
|---|---|---|
| grremixdreamland.c:31 | READY (com ressalva) | S |
| grremixjapes.c:24 | BLOCKED | caro (runtime ou chamador) |
| grremixmetallicbeelaser.c:173 | BLOCKED | S a M |
| grremixpiratecannonball.c:80 | BLOCKED | M |
| grremixpiratecannonball.c:122 | BLOCKED | S a M |
| grremixrainbowchomp.c:43 | som READY (S), hitbox BLOCKED (M) | S e M |
| grremixscuttle.c:70 | BLOCKED | M a L |
| grremixscuttle.c:198 | BLOCKED | depende de objeto de fundo |
| grremixspawnedfear.c:55 | NOT-OBSERVABLE (provável) | nenhum |
| grremixtimetwister.c:62 | BLOCKED | M a L |
| grremixtimetwister.c:73-74 | BLOCKED | M a L |
| grremixtimetwister.c:103 | BLOCKED (depende do 62) | S para o vidro |
| grremixyoshiclouds.c:70 | READY | S |
| if/ifcommon.c:3390 | BLOCKED | M a L |
| mnremixgallery.c:12 | BLOCKED | L |

Pontos prontos para implementar primeiro: yoshiclouds:70 (S), dreamland:31 (S, após confirmar a tela) e a parte do som do rainbowchomp:43 (S).
