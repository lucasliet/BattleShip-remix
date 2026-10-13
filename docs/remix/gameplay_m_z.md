# Smash Remix: gameplay e sistemas (arquivos M a Z)

Pesquisa somente leitura sobre `.toolchain/pkgs/smashremix-src/src/*.asm` (Smash Remix 2.0.1). Gerada por varredura automática de `OS.patch_start(ROM, RAM)` e `table_patch_start`, com cruzamento de endereços RAM contra comentários `// 0x80XXXXXX` em `decomp/src/**/*.c`.

## Escopo

- Arquivos analisados: 68 (com ao menos um hook ativo ou origin de dados).
- Hooks de código (`OS.patch_start`): 801. Patches de tabela (`table_patch_start`): 65.
- Hooks de código com correspondência no decomp: 747 de 801 (exato: 89, por faixa de até 0x400 após o comentário anterior: 658; sem correspondência: 34; endereço por expressão: 20).
- Excluídos por regra: Menu*, VsRemixMenu, Settings, Toggles, Stages, String, Transitions, Credits, SinglePlayerMenus. `Walljump.asm` não tem hook ativo (os 4 `OS.patch_start` estão comentados).
- Incluídos fora da faixa alfabética, conforme pedido: `command.asm`, `fireball.asm`, `moveset.asm` e os `items/*.asm` com hooks.

## Limitações

- Coluna Comportamento: derivada do nome da rotina (scope) do Remix; a descrição por arquivo vem do nome das rotinas, não de leitura linha a linha de cada hook.
- Gating: detectado apenas se `Toggles.read`/`Toggles.guard` aparecer nas linhas seguintes ao `patch_start` (até a próxima rotina). Hooks marcados como não detectados podem ser condicionados por flag de modo, de SRAM ou por outra rotina; conferir o código.
- Decomp: o match por faixa usa o comentário de endereço anterior mais próximo (até 0x400 bytes). Não há tabela de tamanhos de função no decomp, então o match por faixa indica uma função provável, não confirmada.
- Dificuldade: heurística por arquivo (contagem de linhas `db/dw/dd` e de patches de tabela), não avaliação por hook.

## Resumo por arquivo

| Arquivo | Hooks código | Tabela | Dificuldade (heur.) | Feature |
|---|---|---|---|---|
| command.asm | 9 | 0 | tabela (dado de ROM/tabela) | Ajustes de comando/movimento: velocidades aéreas e terrestres, criação/fim de hitbox, FGM e correção de textura. |
| fireball.asm | 5 | 2 | tabela (dado de ROM/tabela) | Rotinas de projétil tipo fireball e cápsula: rebote/destruição, flag de cápsula, correção Kirby. |
| items/Basketball.asm | 11 | 0 | novo estado (flags/variáveis) | Item Basketball: hitlag e hitbox da bola, rebote, FGM de rebote e entrada em barril. |
| items/CloakingDevice.asm | 2 | 0 | novo estado (flags/variáveis) | Dispositivo de camuflagem: impede dano e sombra enquanto ativo. |
| items/Cloud.asm | 1 | 0 | novo estado (flags/variáveis) | Item Cloud: habilita o flash. |
| items/FranklinBadge.asm | 3 | 0 | novo estado (flags/variáveis) | Franklin Badge: imunidade a itens e projéteis e manutenção da ação ao trocar de ação. |
| items/GoldenGun.asm | 6 | 0 | novo estado (flags/variáveis) | Golden Gun: correções de ID de item no tiro aéreo/terrestre, FGM de tiro e estrutura do projétil. |
| items/ScuttleMinion.asm | 1 | 0 | novo estado (flags/variáveis) | Scuttle Minion: remove o FGM de caminhada do bob-omb. |
| items/Stopwatch.asm | 4 | 0 | novo estado (flags/variáveis) | Stopwatch: mapa de física e correção de crash de sombra no despawn de plataforma. |
| items/WaddleDee.asm | 1 | 0 | novo estado (flags/variáveis) | Waddle Dee: remove o FGM de caminhada do bob-omb. |
| items/WaddleDoo.asm | 1 | 0 | novo estado (flags/variáveis) | Waddle Doo: corrige a direção de knockback do feixe. |
| MagnifyingGlass.asm | 1 | 0 | edição C trivial | Zoom customizado da lupa (item) aplicado ao viewport da câmera. |
| marioshared.asm | 2 | 17 | tabela (dado de ROM/tabela) | Helpers compartilhados do Mario: entrada e saída de cano (pipe turn). |
| midi.asm | 5 | 0 | tabela (dado de ROM/tabela) | Sistema de música/MIDI: tabela de músicas, bloco MIDI, mapa de bancos na DMA, instrumentos e volume master. |
| moveset.asm | 1 | 0 | novo estado (flags/variáveis) | Lookup de endereço de comando para movesets customizados. |
| nessshared.asm | 36 | 5 | tabela (dado de ROM/tabela) | Helpers compartilhados do Ness e correções de PK Thunder/PK Fire e uso de USP pela CPU. |
| OnHit.asm | 1 | 0 | edição C trivial | Hook de acerto (on-hit): chama a rotina Remix quando o jogador é atingido. |
| os.asm | 0 | 0 | edição C trivial | Helpers de OS (patches de inicialização/leitura), sem rotinas de gameplay próprias. |
| PAL.asm | 1 | 0 | tabela (dado de ROM/tabela) | Detecção/atualização do tipo de TV (PAL/NTSC) no boot. |
| Parry.asm | 4 | 0 | edição C trivial | Parry: detecção de hit parry por personagem, item e projétil, bolha de parry e hitlag extra no atacante. |
| Pause.asm | 5 | 0 | novo estado (flags/variáveis) | Menu de pausa: controles extras de câmera, ocultar HUD, legenda de controles e hold-to-pause. |
| PerfectShield.asm | 3 | 0 | novo estado (flags/variáveis) | Perfect Shield: reflete projéteis e itens e pula a troca de ação de stun do escudo. |
| pikashared.asm | 13 | 6 | tabela (dado de ROM/tabela) | Helpers compartilhados do Pikachu: Thunder/Thunder Jolt, correções de smash forward e structs de animação. |
| PlayerTag.asm | 7 | 0 | edição C trivial | Tag de jogador customizada acima do personagem (criação, posição, imagem e exibição sempre). |
| Poison.asm | 2 | 0 | novo estado (flags/variáveis) | Veneno: hook de dano por veneno e correção de star. |
| PokemonAnnouncer.asm | 29 | 0 | novo estado (flags/variáveis) | Anunciador de voz tipo Pokémon: falas de hit, carga, especiais, clang e outras. |
| Practice.asm | 2 | 0 | novo estado (flags/variáveis) | Modo prática: flash no Z-cancel e cor de overlay. |
| Practice_1P.asm | 12 | 0 | novo estado (flags/variáveis) | Prática 1P: segurar L para tentar de novo, retries de bônus e prevenção de softlock do Master Hand. |
| Rage.asm | 5 | 0 | edição C trivial | Rage: multiplicador de knockback, ataque direto/projétil, arremesso e gráficos. |
| Reflect.asm | 13 | 10 | tabela (dado de ROM/tabela) | Reflect: input de reflexão, extensão de reflexão para itens/projéteis e correção de direção. |
| Render.asm | 1 | 0 | tabela (dado de ROM/tabela) | Renderização: setup de fonte e tabelas de largura de caracteres (origin de dados). |
| resultsscreen.asm | 19 | 7 | tabela (dado de ROM/tabela) | Tela de resultados: logo do vencedor, animações de vitória, BGM/FGM do vencedor e registro de vitórias. |
| samusshared.asm | 9 | 0 | novo estado (flags/variáveis) | Helpers compartilhados da Samus: carga de tiro, bomba, grapple e poder Kirby. |
| SFXReplace.asm | 2 | 0 | novo estado (flags/variáveis) | Substituição de SFX (incluindo bounce para baixo). |
| Shield.asm | 7 | 0 | novo estado (flags/variáveis) | Escudo: correção de cor e alteração da cor do ovo do Yoshi. |
| SingleButtonMode.asm | 4 | 0 | edição C trivial | Modo de um botão: tratamento de input da CPU. |
| SinglePlayer.asm | 54 | 0 | novo estado (flags/variáveis) | Modo 1P: tally, high score 1P, mapeamento de ambiente e prévia de figurino em dupla. |
| SinglePlayerEnemy.asm | 10 | 0 | edição C trivial | Controle dos inimigos no 1P: AFK, botões, respawn e escudo. |
| SinglePlayerModes.asm | 127 | 0 | novo estado (flags/variáveis) | Modos 1P: estágios, final All-Star, crash na morte e tela de fim de partida. |
| Size.asm | 73 | 0 | novo estado (flags/variáveis) | Modo tamanho: câmera (centro e zoom), ECB, escala do gfx, altura de pulo e posição do fogo PK. |
| Skeleton.asm | 2 | 0 | edição C trivial | Modo esqueleto: dois hooks de modo. |
| Smashketball.asm | 13 | 0 | novo estado (flags/variáveis) | Modo Smashketball: IA da bola, overtime, chant, seletor de gol, despawn e estatísticas. |
| Spawn.asm | 2 | 0 | novo estado (flags/variáveis) | Carregamento de ponto de spawn/respawn. |
| SpecialZoom.asm | 4 | 0 | novo estado (flags/variáveis) | Special Zoom (BETA): câmera de zoom para especiais, knockback final e offset. |
| Speed.asm | 2 | 0 | novo estado (flags/variáveis) | Velocidade do jogo: taxa de update e processamento. |
| SpotDodge.asm | 1 | 0 | edição C trivial | Spot Dodge: checagem de input. |
| SRAM.asm | 2 | 0 | novo estado (flags/variáveis) | Leitura/escrita de SRAM (dados salvos) em duas rotinas. |
| StaleMoves.asm | 3 | 0 | novo estado (flags/variáveis) | Stale moves: penalização de repetição de golpe. |
| Stamina.asm | 59 | 0 | novo estado (flags/variáveis) | Modo Stamina: exibição de stock/tempo, prevenção de crash, input de down-bounce e atualização de porta de percentual. |
| Stereo.asm | 1 | 0 | edição C trivial | Correção de pan estéreo de SFX. |
| StockMode.asm | 1 | 0 | novo estado (flags/variáveis) | Modo stock: define a contagem inicial de stocks. |
| Surface.asm | 8 | 0 | novo estado (flags/variáveis) | Superfícies: fricção, conveyor para fighter e item, correção do ácido de Zebes e override de FGM. |
| SwordTrail.asm | 7 | 0 | novo estado (flags/variáveis) | Rastro de espada: setup de eixos, desenho e checagens. |
| TagTeam.asm | 25 | 0 | novo estado (flags/variáveis) | Modo Tag Team: troca de slot, roubo de stock, ícones, pickup de token e carregamento de arquivos. |
| Teams.asm | 32 | 0 | novo estado (flags/variáveis) | Modo Teams: setup de time, extensões da tela de resultados e correções de figurino clássico. |
| TimedStock.asm | 8 | 0 | edição C trivial | Modo Timed Stock: pontuação, prevenção de morte súbita e resultados completos. |
| timeouts.asm | 11 | 0 | edição C trivial | Timeouts: hooks de branch em rotinas de tempo limite. |
| Training.asm | 32 | 0 | novo estado (flags/variáveis) | Modo treino: menus de entrada/saída, reset, desabilitar spawn de itens, correções de crash Pokémon e setor Z. |
| Tripping.asm | 6 | 0 | edição C trivial | Tripping: retenção de hitstun ao virar e checagem de tropeço. |
| TugOfWar.asm | 4 | 0 | novo estado (flags/variáveis) | Tug of War: crédito de stock para o Koer e última vez que foi atingido. |
| TwelveCharBattle.asm | 46 | 0 | novo estado (flags/variáveis) | 12 Char Battle: mudança de stock, FFA, portas 3/4, portrait e prevenção de seleção de personagem derrotado. |
| VanillaLv10Attacks.asm | 0 | 12 | tabela (dado de ROM/tabela) | Ataques Lv10 vanilla: patches de tabela de comportamento de IA por personagem (dado, não código). |
| VsDemo.asm | 5 | 0 | edição C trivial | Vs Demo: habilita personagens e estágios Remix e correção de offset. |
| VsStats.asm | 12 | 0 | novo estado (flags/variáveis) | Estatísticas de Vs: rastreador de especiais e prevenção de flash de linha. |
| WallTeching.asm | 3 | 0 | edição C trivial | Wall Teching: rastreio de roll e tech bem-sucedidos. |
| Widescreen.asm | 3 | 0 | edição C trivial | Widescreen: habilita o modo e corrige aspect ratio do menu. |
| yoshishared.asm | 15 | 6 | tabela (dado de ROM/tabela) | Helpers compartilhados do Yoshi: correções de shield, grab, DJ, cpu e structs de especiais. |
| ZCancel.asm | 2 | 0 | novo estado (flags/variáveis) | Z-Cancel: opções de modo (Cruel Z-Cancel). |

## Hooks por arquivo

### command.asm

Ajustes de comando/movimento: velocidades aéreas e terrestres, criação/fim de hitbox, FGM e correção de textura.

Hooks de código: 9; patches de tabela: 0; origins de dados (não contados como hook): 5. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 1 | `.toolchain/pkgs/smashremix-src/src/command.asm:150` | `end_hitbox_` | 0x63D2C | `0x800E852C` | não detectado na janela | `j       end_hitbox_` | `decomp/src/ft/ftparam.c:541` `ftParamClearAttackCollAll` (+0x14) | end hitbox |
| 2 | `.toolchain/pkgs/smashremix-src/src/command.asm:190` | `create_hitbox_` | 0x5A9F0 | `0x800DF1F0` | não detectado na janela | `j       create_hitbox_` | `decomp/src/ft/ftmain.c:159` `ftMainParseMotionEvent` (+0x100) | create hitbox |
| 3 | `.toolchain/pkgs/smashremix-src/src/command.asm:232` | `texture_form_fix_` | 0x64E2C | `0x800E962C` | não detectado na janela | `j       texture_form_fix_` | `decomp/src/ft/ftparam.c:1261` `ftParamSetTexturePartID` (exato) | texture form fix |
| 4 | `.toolchain/pkgs/smashremix-src/src/command.asm:612` | `apply_ground_z_` | 0x5442C | `0x800D8C2C` | não detectado na janela | `j       apply_ground_z_` | `decomp/src/ft/ftphysics.c:167` `ftPhysicsApplyGroundVelTransN` (+0x18) | apply ground z |
| 5 | `.toolchain/pkgs/smashremix-src/src/command.asm:664` | `apply_ground_x_` | 0x54468 | `0x800D8C68` | não detectado na janela | `j       apply_ground_x_` | `decomp/src/ft/ftphysics.c:167` `ftPhysicsApplyGroundVelTransN` (+0x54) | apply ground x |
| 6 | `.toolchain/pkgs/smashremix-src/src/command.asm:716` | `apply_air_z_` | 0x54A94 | `0x800D9294` | não detectado na janela | `j       apply_air_z_` | `decomp/src/ft/ftphysics.c:407` `ftPhysicsGetAirVelTransN` (+0x34) | apply air z |
| 7 | `.toolchain/pkgs/smashremix-src/src/command.asm:768` | `apply_air_y_` | 0x54AB0 | `0x800D92B0` | não detectado na janela | `j       apply_air_y_` | `decomp/src/ft/ftphysics.c:407` `ftPhysicsGetAirVelTransN` (+0x50) | apply air y |
| 8 | `.toolchain/pkgs/smashremix-src/src/command.asm:821` | `apply_air_x_` | 0x54B64 | `0x800D9364` | não detectado na janela | `j       apply_air_x_` | `decomp/src/ft/ftphysics.c:407` `ftPhysicsGetAirVelTransN` (+0x104) | apply air x |
| 9 | `.toolchain/pkgs/smashremix-src/src/command.asm:921` | `apply_fgm_` | 0x5E4A8 | `0x800E2CA8` | não detectado na janela | `j       apply_fgm_` | `decomp/src/ft/ftmain.c:2243` `ftMainPlayHitSFX` (+0x84) | apply fgm |

### fireball.asm

Rotinas de projétil tipo fireball e cápsula: rebote/destruição, flag de cápsula, correção Kirby.

Hooks de código: 5; patches de tabela: 2; origins de dados (não contados como hook): 3. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 10 | `.toolchain/pkgs/smashremix-src/src/fireball.asm:60` | `id_patch_` | - | `tabela fireball[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | id patch |
| 11 | `.toolchain/pkgs/smashremix-src/src/fireball.asm:73` | `id_patch_kirby_` | - | `tabela kirby_fireball[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | id patch kirby |
| 12 | `.toolchain/pkgs/smashremix-src/src/fireball.asm:223` | `bounce_destroy_check` | 0xE30A4 | `0x80168664` | não detectado na janela | `j       bounce_destroy_check` | `decomp/src/wp/wpmario/wpmariofireball.c:118` `wpMarioFireballProcMap` (+0x74) | bounce destroy check |
| 13 | `.toolchain/pkgs/smashremix-src/src/fireball.asm:345` | `kirby_fix_` | 0xD1448 | `0x80156A08` | não detectado na janela | `j       kirby_fix_` | `decomp/src/ft/ftchar/ftkirby/ftkirbycopymariospecialn.c:24` `ftKirbyCopyMarioSpecialNProcAccessory` (+0x34) | kirby fix |
| 14 | `.toolchain/pkgs/smashremix-src/src/fireball.asm:452` | `load_capsule_subroutine_` | 0xD0A40 | `0x80156000` | não detectado na janela | `j       load_capsule_subroutine_` | `decomp/src/ft/ftchar/ftmario/ftmariospecialn.c:93` `ftMarioSpecialNInitStatusVars` (+0x4) | load capsule subroutine |
| 15 | `.toolchain/pkgs/smashremix-src/src/fireball.asm:457` | `load_capsule_subroutine_` | 0xD15FC | `0x80156BBC` | não detectado na janela | `j       load_capsule_subroutine_` | `decomp/src/ft/ftchar/ftkirby/ftkirbycopymariospecialn.c:102` `ftKirbyCopyMarioSpecialNInitStatusVars` (+0x4) | load capsule subroutine |
| 16 | `.toolchain/pkgs/smashremix-src/src/fireball.asm:511` | `capsule_flag` | 0xE324C | `0x8016880C` | não detectado na janela | `j       capsule_flag` | `decomp/src/wp/wpmario/wpmariofireball.c:174` `wpMarioFireballMakeWeapon` (+0x6C) | capsule flag |

### items/Basketball.asm

Item Basketball: hitlag e hitbox da bola, rebote, FGM de rebote e entrada em barril.

Hooks de código: 11; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 17 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1161` | `prevent_fighter_enter_barrel_` | 0x858E0 | `0x8010A0E0` | não detectado na janela | `j       prevent_fighter_enter_barrel_` | `decomp/src/gr/grcommon/grjungle.c:161` `grJungleTaruCannCheckGetDamageKind` (+0x108) | prevent fighter enter barrel |
| 18 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1206` | `bounce_patch_` | 0xEE2B4 | `0x80173874` | não detectado na janela | `jal     bounce_patch_._lwall` | `decomp/src/it/itmap.c:158` `itMapCheckCollideAllRebound` (+0x88) | bounce patch |
| 19 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1211` | `bounce_patch_` | 0xEE32C | `0x801738EC` | não detectado na janela | `jal     bounce_patch_._rwall` | `decomp/src/it/itmap.c:158` `itMapCheckCollideAllRebound` (+0x100) | bounce patch |
| 20 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1216` | `bounce_patch_` | 0xEE39C | `0x8017395C` | não detectado na janela | `jal     bounce_patch_._ceiling` | `decomp/src/it/itmap.c:158` `itMapCheckCollideAllRebound` (+0x170) | bounce patch |
| 21 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1221` | `bounce_patch_` | 0xEE3FC | `0x801739BC` | não detectado na janela | `jal     bounce_patch_._floor` | `decomp/src/it/itmap.c:158` `itMapCheckCollideAllRebound` (+0x1D0) | bounce patch |
| 22 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1332` | `prevent_bounce_fgm_` | 0xEE2E4 | `0x801738A4` | não detectado na janela | `jal     prevent_bounce_fgm_` | `decomp/src/it/itmap.c:158` `itMapCheckCollideAllRebound` (+0xB8) | prevent bounce fgm |
| 23 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1337` | `prevent_bounce_fgm_` | 0xEE354 | `0x80173914` | não detectado na janela | `jal     prevent_bounce_fgm_` | `decomp/src/it/itmap.c:158` `itMapCheckCollideAllRebound` (+0x128) | prevent bounce fgm |
| 24 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1342` | `prevent_bounce_fgm_` | 0xEE41C | `0x801739DC` | não detectado na janela | `jal     prevent_bounce_fgm_` | `decomp/src/it/itmap.c:158` `itMapCheckCollideAllRebound` (+0x1F0) | prevent bounce fgm |
| 25 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1371` | `multiply_player_hitlag_` | 0x5DEC8 | `0x800E26C8` | não detectado na janela | `j       multiply_player_hitlag_` | `decomp/src/ft/ftmain.c:2076` `ftMainSetHitInteractStats` (+0xC) | multiply player hitlag |
| 26 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1403` | `multiply_ball_hitlag_hurtbox_` | 0xEBDA8 | `0x80171368` | não detectado na janela | `j       multiply_ball_hitlag_hurtbox_` | `decomp/src/it/itprocess.c:1014` `itProcessProcHitCollisions` (+0x2A4) | multiply ball hitlag hurtbox |
| 27 | `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm:1427` | `add_ball_hitlag_hitbox_` | 0x5F1D0 | `0x800E39D0` | não detectado na janela | `jal     add_ball_hitlag_hitbox_` | `decomp/src/ft/ftmain.c:2640` `ftMainUpdateDamageStatItem` (+0x20) | add ball hitlag hitbox |

### items/CloakingDevice.asm

Dispositivo de camuflagem: impede dano e sombra enquanto ativo.

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 28 | `.toolchain/pkgs/smashremix-src/src/items/CloakingDevice.asm:223` | `prevent_damage_when_cloaked_` | 0x65A68 | `0x800EA268` | não detectado na janela | `j       prevent_damage_when_cloaked_` | `decomp/src/ft/ftparam.c:1689` `ftParamUpdateDamage` (+0x20) | prevent damage when cloaked |
| 29 | `.toolchain/pkgs/smashremix-src/src/items/CloakingDevice.asm:251` | `prevent_shadow_when_cloaked_` | 0xB5A14 | `0x8013AFD4` | não detectado na janela | `jal     prevent_shadow_when_cloaked_` | `decomp/src/mn/mnplayers/mnplayersvs.c:5406` `mnPlayersVSInitSlot` (+0x14) | prevent shadow when cloaked |

### items/Cloud.asm

Item Cloud: habilita o flash.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 30 | `.toolchain/pkgs/smashremix-src/src/items/Cloud.asm:289` | `enable_flash` | 0xE9FF4 | `0x8016F5B4` | não detectado na janela | `j       enable_flash` | `decomp/src/it/itprocess.c:92` `itProcessProcItemMain` (+0x80) | enable flash |

### items/FranklinBadge.asm

Franklin Badge: imunidade a itens e projéteis e manutenção da ação ao trocar de ação.

Hooks de código: 3; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 31 | `.toolchain/pkgs/smashremix-src/src/items/FranklinBadge.asm:285` | `keep_on_action_change_` | 0x628EC | `0x800E70EC` | não detectado na janela | `j       keep_on_action_change_` | `decomp/src/ft/ftmain.c:4578` `ftMainSetStatus` (+0x1C8) | keep on action change |
| 32 | `.toolchain/pkgs/smashremix-src/src/items/FranklinBadge.asm:327` | `grant_projectile_immunity_` | 0x60CF4 | `0x800E54F4` | não detectado na janela | `j   grant_projectile_immunity_` | unmatched | grant projectile immunity |
| 33 | `.toolchain/pkgs/smashremix-src/src/items/FranklinBadge.asm:362` | `grant_item_immunity_` | 0x61348 | `0x800E5B48` | não detectado na janela | `j       grant_item_immunity_` | unmatched | grant item immunity |

### items/GoldenGun.asm

Golden Gun: correções de ID de item no tiro aéreo/terrestre, FGM de tiro e estrutura do projétil.

Hooks de código: 6; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 34 | `.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm:193` | `projectile_struct_fix` | 0xF04AC | `0x80175A6C` | não detectado na janela | `j       projectile_struct_fix` | `decomp/src/it/itcommon/itlgun.c:350` `itLGunWeaponAmmoMakeWeapon` (+0xC) | projectile struct fix |
| 35 | `.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm:218` | `projectile_shoot_fgm_fix` | 0xC1B74 | `0x80147134` | não detectado na janela | `j       projectile_shoot_fgm_fix` | `decomp/src/ft/ftcommon/ftcommonitemshoot.c:20` `ftCommonLGunShootProcAccessory` (+0x160) | projectile shoot fgm fix |
| 36 | `.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm:241` | `projectile_properties_fix` | 0xF04CC | `0x80175A8C` | não detectado na janela | `j       projectile_properties_fix` | `decomp/src/it/itcommon/itlgun.c:350` `itLGunWeaponAmmoMakeWeapon` (+0x2C) | projectile properties fix |
| 37 | `.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm:280` | `projectile_no_ammo_fgm_fix` | 0xC1BE0 | `0x801471A0` | não detectado na janela | `j       projectile_no_ammo_fgm_fix` | `decomp/src/ft/ftcommon/ftcommonitemshoot.c:20` `ftCommonLGunShootProcAccessory` (+0x1CC) | projectile no ammo fgm fix |
| 38 | `.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm:307` | `extend_grounded_shoot_item_id_check` | 0xC22C8 | `0x80147888` | não detectado na janela | `j       extend_grounded_shoot_item_id_check` | `decomp/src/ft/ftcommon/ftcommonitemshoot.c:296` `ftCommonItemShootSetStatus` (+0x44) | extend grounded shoot item id check |
| 39 | `.toolchain/pkgs/smashremix-src/src/items/GoldenGun.asm:330` | `extend_aerial_shoot_item_id_check` | 0xC2370 | `0x80147930` | não detectado na janela | `j       extend_aerial_shoot_item_id_check` | `decomp/src/ft/ftcommon/ftcommonitemshoot.c:339` `ftCommonItemShootAirSetStatus` (+0x44) | extend aerial shoot item id check |

### items/ScuttleMinion.asm

Scuttle Minion: remove o FGM de caminhada do bob-omb.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 40 | `.toolchain/pkgs/smashremix-src/src/items/ScuttleMinion.asm:128` | `remove_bob_bomb_walk_fgm_` | 0xF23CC | `0x8017798C` | não detectado na janela | `j		remove_bob_bomb_walk_fgm_` | `decomp/src/it/itcommon/itbombhei.c:513` `itBombHeiWalkInitVars` (+0x144) | remove bob bomb walk fgm |

### items/Stopwatch.asm

Stopwatch: mapa de física e correção de crash de sombra no despawn de plataforma.

Hooks de código: 4; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 41 | `.toolchain/pkgs/smashremix-src/src/items/Stopwatch.asm:959` | `physics_map` | 0x5D888 | `0x800e2088` | não detectado na janela | `j       hitlag_check_1` | `decomp/src/ft/ftmain.c:1875` `ftMainProcPhysicsMap` (+0x40) | physics map |
| 42 | `.toolchain/pkgs/smashremix-src/src/items/Stopwatch.asm:965` | `physics_map` | 0x5DC0C | `0x800E240C` | não detectado na janela | `j       hitlag_check_2` | `decomp/src/ft/ftmain.c:1875` `ftMainProcPhysicsMap` (+0x3C4) | physics map |
| 43 | `.toolchain/pkgs/smashremix-src/src/items/Stopwatch.asm:971` | `physics_map` | 0x5DC20 | `0x800E2420` | não detectado na janela | `j       hitlag_check_3` | `decomp/src/ft/ftmain.c:1875` `ftMainProcPhysicsMap` (+0x3D8) | physics map |
| 44 | `.toolchain/pkgs/smashremix-src/src/items/Stopwatch.asm:1193` | `platform_despawn_shadow_crash_fix` | 0xB5A54 | `0x8013B014` | não detectado na janela | `j       platform_despawn_shadow_crash_fix` | `decomp/src/mn/mnplayers/mnplayersvs.c:5406` `mnPlayersVSInitSlot` (+0x54) | platform despawn shadow crash fix |

### items/WaddleDee.asm

Waddle Dee: remove o FGM de caminhada do bob-omb.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 45 | `.toolchain/pkgs/smashremix-src/src/items/WaddleDee.asm:131` | `remove_bob_bomb_walk_fgm_` | 0xF23CC | `0x8017798C` | não detectado na janela | `j		remove_bob_bomb_walk_fgm_` | `decomp/src/it/itcommon/itbombhei.c:513` `itBombHeiWalkInitVars` (+0x144) | remove bob bomb walk fgm |

### items/WaddleDoo.asm

Waddle Doo: corrige a direção de knockback do feixe.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 46 | `.toolchain/pkgs/smashremix-src/src/items/WaddleDoo.asm:410` | `beam_knockback_direction_fix` | 0x5FCEC | `0x800E44EC` | não detectado na janela | `j       beam_knockback_direction_fix` | unmatched | beam knockback direction fix |

### MagnifyingGlass.asm

Zoom customizado da lupa (item) aplicado ao viewport da câmera.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 47 | `.toolchain/pkgs/smashremix-src/src/MagnifyingGlass.asm:22` | `apply_custom_zoom_` | 0x8C680 | `0x80110E80` | não detectado na janela | `jal     apply_custom_zoom_` | `decomp/src/if/ifcommon.c:1632` `ifCommonPlayerMagnifyUpdateViewport` (+0xAC) | apply custom zoom |

### marioshared.asm

Helpers compartilhados do Mario: entrada e saída de cano (pipe turn).

Hooks de código: 2; patches de tabela: 17; origins de dados (não contados como hook): 0. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 48 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:145` | `ledge_check` | - | `tabela recovery_logic[Character.id.MARIO] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 49 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:147` | `ledge_check` | - | `tabela recovery_logic[Character.id.JMARIO] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 50 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:149` | `ledge_check` | - | `tabela recovery_logic[Character.id.LUIGI] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 51 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:151` | `ledge_check` | - | `tabela recovery_logic[Character.id.JLUIGI] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 52 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:153` | `ledge_check` | - | `tabela recovery_logic[Character.id.DRM] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 53 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:155` | `ledge_check` | - | `tabela recovery_logic[Character.id.DRL] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 54 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:159` | `ledge_check` | - | `tabela pipe_turn[Character.id.NDRM] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 55 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:163` | `ledge_check` | - | `tabela pipe_turn[Character.id.NWARIO] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 56 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:167` | `ledge_check` | - | `tabela pipe_turn[Character.id.NCONKER] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 57 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:171` | `ledge_check` | - | `tabela pipe_turn[Character.id.JMARIO] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 58 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:175` | `ledge_check` | - | `tabela pipe_turn[Character.id.JLUIGI] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 59 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:179` | `ledge_check` | - | `tabela pipe_turn[Character.id.MLUIGI] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 60 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:183` | `ledge_check` | - | `tabela pipe_turn[Character.id.WARIO] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 61 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:187` | `ledge_check` | - | `tabela pipe_turn[Character.id.GOEMON] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 62 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:191` | `ledge_check` | - | `tabela pipe_turn[Character.id.EBI] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 63 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:195` | `ledge_check` | - | `tabela pipe_turn[Character.id.DRM] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 64 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:199` | `ledge_check` | - | `tabela pipe_turn[Character.id.CONKER] +0x1` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 65 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:205` | `pipe_turn_enter` | 0xBCC40 | `0x80142200` | não detectado na janela | `j       pipe_turn_enter` | `decomp/src/ft/ftcommon/ftcommondokan.c:72` `ftCommonDokanStartSetStatus` (+0x9C) | pipe turn enter |
| 66 | `.toolchain/pkgs/smashremix-src/src/marioshared.asm:231` | `pipe_turn_exit` | 0xBD19C | `0x8014275C` | não detectado na janela | `j       pipe_turn_exit` | `decomp/src/ft/ftcommon/ftcommondokan.c:275` `ftCommonDokanEndSetStatus` (+0x8C) | pipe turn exit |

### midi.asm

Sistema de música/MIDI: tabela de músicas, bloco MIDI, mapa de bancos na DMA, instrumentos e volume master.

Hooks de código: 5; patches de tabela: 0; origins de dados (não contados como hook): 21. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 67 | `.toolchain/pkgs/smashremix-src/src/midi.asm:705` | `replace_midi_block_` | 0x20304 | `0x8001F704` | não detectado na janela | - | `decomp/src/sys/audio.c:820` `syAudioInit` (+0x2C0) | replace midi block |
| 68 | `.toolchain/pkgs/smashremix-src/src/midi.asm:716` | `augment_sound_test_music_` | 0x188530 | `0x80132160` | não detectado na janela | `j       augment_sound_test_music_` | `decomp/src/mv/mvopening/mvopeningsector.c:301` `mvOpeningSectorCockpitProcUpdate` (+0x4) | augment sound test music |
| 69 | `.toolchain/pkgs/smashremix-src/src/midi.asm:1253` | `dmaCopy_moved_bank_map_` | 0x20148 | `0x8001F548` | não detectado na janela | `jal     dmaCopy_moved_bank_map_` | `decomp/src/sys/audio.c:820` `syAudioInit` (+0x104) | dmaCopy moved bank map |
| 70 | `.toolchain/pkgs/smashremix-src/src/midi.asm:1363` | `override_instrument_parameters_` | 0x2C820 | `0x8002BC20` | não detectado na janela | `jal     override_instrument_parameters_` | unmatched | override instrument parameters |
| 71 | `.toolchain/pkgs/smashremix-src/src/midi.asm:1426` | `override_master_volume_` | 0x302C0 | `0x8002F6C0` | não detectado na janela | `j       override_master_volume_` | `decomp/src/libultra/n_audio/n_seqpgetvol.c:4` `n_alSeqpGetVol` (exato) | override master volume |

### moveset.asm

Lookup de endereço de comando para movesets customizados.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 1. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 72 | `.toolchain/pkgs/smashremix-src/src/moveset.asm:12` | `get_cmd_address_` | 0x63140 | `0x800E7940` | não detectado na janela | `j       get_cmd_address_` | unmatched | get cmd address |

### nessshared.asm

Helpers compartilhados do Ness e correções de PK Thunder/PK Fire e uso de USP pela CPU.

Hooks de código: 36; patches de tabela: 5; origins de dados (não contados como hook): 0. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 73 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:10` | `pk_thunder_reflect_crash_fix_1_` | 0xE6280 | `0x8016B840` | Toggle: PK Thunder Reflect Crash Fix | `j       pk_thunder_reflect_crash_fix_1_` | `decomp/src/wp/wpness/wpnesspkthunder.c:556` `wpNessPKReflectHeadProcReflector` (exato) | pk thunder reflect crash fix 1 |
| 74 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:36` | `pk_thunder_reflect_crash_fix_2_` | 0xE6640 | `0x8016BC00` | Toggle: PK Thunder Reflect Crash Fix | `j       pk_thunder_reflect_crash_fix_2_` | `decomp/src/wp/wpness/wpnesspkthunder.c:663` `wpNessPKReflectTrailMakeWeapon` (+0x94) | pk thunder reflect crash fix 2 |
| 75 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:75` | `get_pkfire_pointer_` | 0xE56D0 | `0x8016AC90` | não detectado na janela | `j       get_pkfire_pointer_` | `decomp/src/wp/wpness/wpnesspkfire.c:131` `wpNessPKFireMakeWeapon` (+0x18) | get pkfire pointer |
| 76 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:122` | `get_pkfire_collision_pointer_` | 0x100288 | `0x80185848` | não detectado na janela | `j       get_pkfire_collision_pointer_` | `decomp/src/it/itfighter/itnesspkfire.c:244` `itNessPKFireMakeItem` (+0x24) | get pkfire collision pointer |
| 77 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:188` | `get_pkfire_collision_pointer_` | - | `tabela ness_jump[Character.id.NESS] +0x1` | não detectado na janela | - | n/a (patch de dado) | get pkfire collision pointer |
| 78 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:191` | `get_pkfire_collision_pointer_` | - | `tabela ness_jump[Character.id.NNESS] +0x1` | não detectado na janela | - | n/a (patch de dado) | get pkfire collision pointer |
| 79 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:196` | `ness_jump1_` | 0xBA848 | `0x8013FE08` | não detectado na janela | `j       ness_jump1_` | `decomp/src/ft/ftcommon/ftcommonjumpaerial.c:128` `ftCommonJumpAerialSetStatus` (+0x94) | ness jump1 |
| 80 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:223` | `ness_jump2_` | 0xBA8C0 | `0x8013FE80` | não detectado na janela | `j       ness_jump2_` | `decomp/src/ft/ftcommon/ftcommonjumpaerial.c:128` `ftCommonJumpAerialSetStatus` (+0x10C) | ness jump2 |
| 81 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:253` | `up_special_landing_fsm_` | 0xCEE54 | `0x80154414` | não detectado na janela | `j       up_special_landing_fsm_` | `decomp/src/ft/ftchar/ftness/ftnessspecialhi.c:341` `ftNessSpecialAirHiEndProcUpdate` (+0x30) | up special landing fsm |
| 82 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:288` | `up_special_ending_delay_` | 0xCEA18 | `0x80153FD8` | não detectado na janela | `j       up_special_ending_delay_` | `decomp/src/ft/ftchar/ftness/ftnessspecialhi.c:179` `ftNessSpecialHiInitStatusVars` (+0xC) | up special ending delay |
| 83 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:308` | `pkfire_ground_speed_1_` | 0xCE49C | `0x80153A5C` | não detectado na janela | `jal     pkfire_ground_speed_1_` | `decomp/src/ft/ftchar/ftness/ftnessspecialn.c:17` `ftNessSpecialNProcAccessory` (+0x10C) | pkfire ground speed 1 |
| 84 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:313` | `pkfire_ground_speed_1_` | 0xD0688 | `0x80155C48` | não detectado na janela | `jal     pkfire_ground_speed_1_` | `decomp/src/ft/ftchar/ftkirby/ftkirbycopynessspecialn.c:17` `ftKirbyCopyNessSpecialNProcAccessory` (+0x108) | pkfire ground speed 1 |
| 85 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:353` | `pkfire_ground_speed_2_` | 0xCE4C8 | `0x80153A88` | não detectado na janela | `jal     pkfire_ground_speed_2_` | `decomp/src/ft/ftchar/ftness/ftnessspecialn.c:17` `ftNessSpecialNProcAccessory` (+0x138) | pkfire ground speed 2 |
| 86 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:358` | `pkfire_ground_speed_2_` | 0xD06B4 | `0x80155C74` | não detectado na janela | `jal     pkfire_ground_speed_2_` | `decomp/src/ft/ftchar/ftkirby/ftkirbycopynessspecialn.c:17` `ftKirbyCopyNessSpecialNProcAccessory` (+0x134) | pkfire ground speed 2 |
| 87 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:399` | `pkfire_air_speed_1_` | 0xCE43C | `0x801539FC` | não detectado na janela | `jal     pkfire_air_speed_1_` | `decomp/src/ft/ftchar/ftness/ftnessspecialn.c:17` `ftNessSpecialNProcAccessory` (+0xAC) | pkfire air speed 1 |
| 88 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:404` | `pkfire_air_speed_1_` | 0xD0628 | `0x80155BE8` | não detectado na janela | `jal     pkfire_air_speed_1_` | `decomp/src/ft/ftchar/ftkirby/ftkirbycopynessspecialn.c:17` `ftKirbyCopyNessSpecialNProcAccessory` (+0xA8) | pkfire air speed 1 |
| 89 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:444` | `pkfire_air_speed_2_` | 0xCE468 | `0x80153A28` | não detectado na janela | `jal     pkfire_air_speed_2_` | `decomp/src/ft/ftchar/ftness/ftnessspecialn.c:17` `ftNessSpecialNProcAccessory` (+0xD8) | pkfire air speed 2 |
| 90 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:449` | `pkfire_air_speed_2_` | 0xD0654 | `0x80155C14` | não detectado na janela | `jal     pkfire_air_speed_2_` | `decomp/src/ft/ftchar/ftkirby/ftkirbycopynessspecialn.c:17` `ftKirbyCopyNessSpecialNProcAccessory` (+0xD4) | pkfire air speed 2 |
| 91 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:489` | `get_pillar_playerstruct_` | 0x1002B4 | `0x80185874` | não detectado na janela | `j       get_pillar_playerstruct_` | `decomp/src/it/itfighter/itnesspkfire.c:244` `itNessPKFireMakeItem` (+0x50) | get pillar playerstruct |
| 92 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:506` | `pkfire_pillar_gravity_` | 0x10007C | `0x8018563C` | não detectado na janela | `jal     pkfire_pillar_gravity_` | `decomp/src/it/itfighter/itnesspkfire.c:157` `itNessPKFireFallProcUpdate` (+0x28) | pkfire pillar gravity |
| 93 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:539` | `get_pkthunder_anim_struct_` | 0x7E208 | `0x80102A08` | não detectado na janela | `j       get_pkthunder_anim_struct_` | `decomp/src/ef/efmanager.c:5194` `efManagerNessPKThunderWaveMakeEffect` (+0x10) | get pkthunder anim struct |
| 94 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:572` | `get_pkthunder_anim_struct2_` | 0x7E178 | `0x80102978` | não detectado na janela | `j       get_pkthunder_anim_struct2_` | `decomp/src/ef/efmanager.c:5163` `efManagerNessPKReflectTrailMakeEffect` (+0x10) | get pkthunder anim struct2 |
| 95 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:611` | `get_pkthunder_anim_struct3_` | 0x7E058 | `0x80102858` | não detectado na janela | `j       get_pkthunder_anim_struct3_` | `decomp/src/ef/efmanager.c:5112` `efManagerNessPKThunderTrailMakeEffect` (+0x10) | get pkthunder anim struct3 |
| 96 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:645` | `get_pkthunder_special_struct1_` | 0xE5D1C | `0x8016B2DC` | não detectado na janela | `j       get_pkthunder_special_struct1_` | `decomp/src/wp/wpness/wpnesspkthunder.c:332` `wpNessPKThunderHeadMakeWeapon` (+0x18) | get pkthunder special struct1 |
| 97 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:682` | `get_pkthunder_special_struct2_` | 0xE5FD8 | `0x8016B598` | não detectado na janela | `j       get_pkthunder_special_struct2_` | `decomp/src/wp/wpness/wpnesspkthunder.c:431` `wpNessPKThunderTrailMakeWeapon` (+0x18) | get pkthunder special struct2 |
| 98 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:717` | `get_pkthunder_reflect_struct1_` | 0xE62FC | `0x8016B8BC` | não detectado na janela | `j       get_pkthunder_reflect_struct1_` | `decomp/src/wp/wpness/wpnesspkthunder.c:578` `wpNessPKReflectHeadMakeWeapon` (+0x24) | get pkthunder reflect struct1 |
| 99 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:759` | `get_pkthunder_reflect_struct2_` | 0xE65CC | `0x8016BB8C` | não detectado na janela | `j       get_pkthunder_reflect_struct2_` | `decomp/src/wp/wpness/wpnesspkthunder.c:663` `wpNessPKReflectTrailMakeWeapon` (+0x20) | get pkthunder reflect struct2 |
| 100 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:796` | `cpu_usp_usage_fix_` | 0xAF170 | `0x80134730` | não detectado na janela | `j       cpu_usp_usage_fix_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1586` `mnPlayers1PBonusResetPlayer` (+0xC) | cpu usp usage fix |
| 101 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:823` | `cpu_usp_control_fix_` | 0xB1BA0 | `0x80137160` | não detectado na janela | `j       cpu_usp_control_fix_` | `decomp/src/mn/mnplayers/mnplayersvs.c:3173` `mnPlayersVSCheckHandicap` (+0x18) | cpu usp control fix |
| 102 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:990` | `get_pkthunder_playerstruct1_` | 0xE597C | `0x8016AF3C` | não detectado na janela | `j       get_pkthunder_playerstruct1_` | `decomp/src/wp/wpness/wpnesspkthunder.c:190` `wpNessPKThunderHeadMakeTrail` (+0xD8) | get pkthunder playerstruct1 |
| 103 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1008` | `get_pkthunder_playerstruct2_` | 0xE5E10 | `0x8016B3D0` | não detectado na janela | `j       get_pkthunder_playerstruct2_` | `decomp/src/wp/wpness/wpnesspkthunder.c:366` `wpNessPKThunderTrailProcUpdate` (+0x38) | get pkthunder playerstruct2 |
| 104 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1023` | `get_pkthunder_playerstruct3_` | 0xCE724 | `0x80153CE4` | não detectado na janela | `j       get_pkthunder_playerstruct3_` | `decomp/src/ft/ftchar/ftness/ftnessspecialhi.c:37` `ftNessSpecialHiMakePKThunder` (+0x5C) | get pkthunder playerstruct3 |
| 105 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1042` | `reflect_fix_1_` | 0xCAB60 | `0x80150120` | não detectado na janela | `j       reflect_fix_1_` | `decomp/src/ft/ftcommon/ftcommonattacks4.c:70` `ftCommonAttackS4SetStatus` (+0x140) | reflect fix 1 |
| 106 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1073` | `reflect_fix_2_` | 0xCABBC | `0x8015017C` | não detectado na janela | `j       reflect_fix_2_` | `decomp/src/ft/ftcommon/ftcommonattacks4.c:70` `ftCommonAttackS4SetStatus` (+0x19C) | reflect fix 2 |
| 107 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1104` | `reflect_fix_3_` | 0xCA8A0 | `0x8014FE60` | não detectado na janela | `j       reflect_fix_3_` | `decomp/src/ft/ftcommon/ftcommonattacks4.c:11` `ftCommonAttackS4ProcUpdate` (+0x20) | reflect fix 3 |
| 108 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1134` | `cpu_pk_thunder_attack_` | 0xB344C | `0x80138A0C` | não detectado na janela | `j		cpu_pk_thunder_attack_` | `decomp/src/mn/mnplayers/mnplayersvs.c:3926` `mnPlayersVSPuckProcUpdate` (+0x114) | cpu pk thunder attack |
| 109 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1160` | `improved_pk_thunder_` | 0xB1C3C | `0x801371FC` | não detectado na janela | `j    improved_pk_thunder_` | `decomp/src/mn/mnvsmode/mnvsresults.c:2782` `mnVSResultsSetTeamPlaceAll` (+0x44) | improved pk thunder |
| 110 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1246` | `save_cpu_recovery_location_` | 0xCE6E8 | `0x80153CA8` | não detectado na janela | `j 		save_cpu_recovery_location_` | `decomp/src/ft/ftchar/ftness/ftnessspecialhi.c:37` `ftNessSpecialHiMakePKThunder` (+0x20) | save cpu recovery location |
| 111 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1422` | `ledge_check` | - | `tabela recovery_logic[Character.id.NESS] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 112 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1424` | `ledge_check` | - | `tabela recovery_logic[Character.id.JNESS] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 113 | `.toolchain/pkgs/smashremix-src/src/nessshared.asm:1426` | `ledge_check` | - | `tabela recovery_logic[Character.id.LUCAS] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |

### OnHit.asm

Hook de acerto (on-hit): chama a rotina Remix quando o jogador é atingido.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 114 | `.toolchain/pkgs/smashremix-src/src/OnHit.asm:7` | `on_hit` | 0x63A44 | `0x800E8244` | não detectado na janela | `j       on_hit` | `decomp/src/ft/ftparam.c:402` `ftParamStopVoiceRunProcDamage` (+0x8) | on hit |

### os.asm

Helpers de OS (patches de inicialização/leitura), sem rotinas de gameplay próprias.

Hooks de código: 0; patches de tabela: 0; origins de dados (não contados como hook): 2. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|

### PAL.asm

Detecção/atualização do tipo de TV (PAL/NTSC) no boot.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 5. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 115 | `.toolchain/pkgs/smashremix-src/src/PAL.asm:22` | `update_tv_type_` | 0x1000 | `0x80000400` | não detectado na janela | - | unmatched | update tv type |

### Parry.asm

Parry: detecção de hit parry por personagem, item e projétil, bolha de parry e hitlag extra no atacante.

Hooks de código: 4; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 116 | `.toolchain/pkgs/smashremix-src/src/Parry.asm:17` | `direct_hit_parry_bubble_check` | 0x60538 | `0x800E4D38` | Toggle: Parrying (BETA) | `j       direct_hit_parry_bubble_check` | unmatched | direct hit parry bubble check |
| 117 | `.toolchain/pkgs/smashremix-src/src/Parry.asm:230` | `projectile_hit_parry_bubble_check` | 0x60AC0 | `0x800E52C0` | Toggle: Parrying (BETA) | `j       projectile_hit_parry_bubble_check` | `decomp/src/ft/ftmain.c:3372` `ftMainSearchHitWeapon` (+0x3EC) | projectile hit parry bubble check |
| 118 | `.toolchain/pkgs/smashremix-src/src/Parry.asm:430` | `item_hit_parry_bubble_check` | 0x611A0 | `0x800E59A0` | Toggle: Parrying (BETA) | `j       item_hit_parry_bubble_check` | `decomp/src/ft/ftmain.c:3569` `ftMainSearchHitItem` (+0x3C4) | item hit parry bubble check |
| 119 | `.toolchain/pkgs/smashremix-src/src/Parry.asm:625` | `apply_attacker_extra_hitlag` | 0x61E90 | `0x800E6690` | Toggle: Parrying (BETA) | `j       apply_attacker_extra_hitlag` | unmatched | apply attacker extra hitlag |

### Pause.asm

Menu de pausa: controles extras de câmera, ocultar HUD, legenda de controles e hold-to-pause.

Hooks de código: 5; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 120 | `.toolchain/pkgs/smashremix-src/src/Pause.asm:22` | `hold_` | 0x0008F88C | `0x8011408C` | Toggle: Hold To Pause | `j       Pause.hold_` | `decomp/src/if/ifcommon.c:3127` `ifCommonBattleGoUpdateInterface` (+0x50) | hold |
| 121 | `.toolchain/pkgs/smashremix-src/src/Pause.asm:76` | `disable_hud_` | 0x0008F738 | `0x80113F38` | não detectado na janela | `jal     Pause.disable_hud_` | `decomp/src/if/ifcommon.c:3052` `ifCommonBattlePauseMakeInterface` (+0x84) | disable hud |
| 122 | `.toolchain/pkgs/smashremix-src/src/Pause.asm:109` | `extra_controls_` | 0x0008FB20 | `0x80114320` | não detectado na janela | `jal     extra_controls_` | `decomp/src/if/ifcommon.c:3211` `ifCommonBattlePauseUpdateInterface` (+0x34) | extra controls |
| 123 | `.toolchain/pkgs/smashremix-src/src/Pause.asm:165` | `show_legend_helper_` | 0x8F6A0 | `0x80113EA0` | não detectado na janela | `j       show_legend_helper_` | `decomp/src/if/ifcommon.c:3030` `ifCommonBattlePauseMakeSObjsAll` (+0x9C) | show legend helper |
| 124 | `.toolchain/pkgs/smashremix-src/src/Pause.asm:410` | `extra_camera_controls_` | 0x0008FB28 | `0x80114328` | não detectado na janela | `jal     extra_camera_controls_` | `decomp/src/if/ifcommon.c:3211` `ifCommonBattlePauseUpdateInterface` (+0x3C) | extra camera controls |

### PerfectShield.asm

Perfect Shield: reflete projéteis e itens e pula a troca de ação de stun do escudo.

Hooks de código: 3; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 125 | `.toolchain/pkgs/smashremix-src/src/PerfectShield.asm:24` | `shield_stun_action_change_skip` | 0x61CD8 | `0x800E64D8` | Toggle: Perfect Shielding | `j       shield_stun_action_change_skip` | `decomp/src/ft/ftmain.c:3961` `ftMainProcParams` (+0x2EC) | shield stun action change skip |
| 126 | `.toolchain/pkgs/smashremix-src/src/PerfectShield.asm:58` | `shield_reflect_projectiles` | 0xE16BC | `0x80166C7C` | Toggle: Perfect Shielding | `j       shield_reflect_projectiles` | `decomp/src/wp/wpprocess.c:462` `wpProcessProcHitCollisions` (+0x98) | shield reflect projectiles |
| 127 | `.toolchain/pkgs/smashremix-src/src/PerfectShield.asm:145` | `shield_reflect_items` | 0xEBBFC | `0x801711BC` | Toggle: Perfect Shielding | `j       shield_reflect_items` | `decomp/src/it/itprocess.c:1014` `itProcessProcHitCollisions` (+0xF8) | shield reflect items |

### pikashared.asm

Helpers compartilhados do Pikachu: Thunder/Thunder Jolt, correções de smash forward e structs de animação.

Hooks de código: 13; patches de tabela: 6; origins de dados (não contados como hook): 0. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 128 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:137` | `_usp_zip1` | - | `tabela recovery_logic[Character.id.PIKACHU] +0x4` | não detectado na janela | - | n/a (patch de dado) | usp zip1 |
| 129 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:139` | `_usp_zip1` | - | `tabela recovery_logic[Character.id.EPIKA] +0x4` | não detectado na janela | - | n/a (patch de dado) | usp zip1 |
| 130 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:141` | `_usp_zip1` | - | `tabela recovery_logic[Character.id.JPIKA] +0x4` | não detectado na janela | - | n/a (patch de dado) | usp zip1 |
| 131 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:190` | `cpu_post_process` | - | `tabela cpu_post_process[Character.id.PIKACHU] +0x4` | não detectado na janela | - | n/a (patch de dado) | cpu post process |
| 132 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:192` | `cpu_post_process` | - | `tabela cpu_post_process[Character.id.EPIKA] +0x4` | não detectado na janela | - | n/a (patch de dado) | cpu post process |
| 133 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:194` | `cpu_post_process` | - | `tabela cpu_post_process[Character.id.JPIKA] +0x4` | não detectado na janela | - | n/a (patch de dado) | cpu post process |
| 134 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:199` | `rapid_jab_fix_1` | 0xC93B4 | `0x8014E974` | não detectado na janela | `j       rapid_jab_fix_1` | `decomp/src/ft/ftcommon/ftcommonattack1.c:92` `ftCommonAttack11ProcInterrupt` (+0x58) | rapid jab fix 1 |
| 135 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:221` | `rapid_jab_fix_2` | 0xC921C | `0x8014E7DC` | não detectado na janela | `j       rapid_jab_fix_2` | `decomp/src/ft/ftcommon/ftcommonattack1.c:48` `ftCommonAttack11ProcUpdate` (+0x2C) | rapid jab fix 2 |
| 136 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:243` | `forward_smash_fix_1` | 0xCA898 | `0x8014FE58` | não detectado na janela | `j       forward_smash_fix_1` | `decomp/src/ft/ftcommon/ftcommonattacks4.c:11` `ftCommonAttackS4ProcUpdate` (+0x18) | forward smash fix 1 |
| 137 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:265` | `forward_smash_fix_2` | 0xCABB0 | `0x80150170` | não detectado na janela | `j       forward_smash_fix_2` | `decomp/src/ft/ftcommon/ftcommonattacks4.c:70` `ftCommonAttackS4SetStatus` (+0x190) | forward smash fix 2 |
| 138 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:287` | `forward_smash_fix_3` | 0xCAB54 | `0x80150114` | não detectado na janela | `j       forward_smash_fix_3` | `decomp/src/ft/ftcommon/ftcommonattacks4.c:70` `ftCommonAttackS4SetStatus` (+0x134) | forward smash fix 3 |
| 139 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:310` | `get_thunder_playerstruct1_` | 0xE00C4 | `0x80165684` | não detectado na janela | `j       get_thunder_playerstruct1_` | `decomp/src/wp/wpmanager.c:93` `wpManagerMakeWeapon` (+0xBC) | get thunder playerstruct1 |
| 140 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:340` | `get_thunder_playerstruct2_` | 0xE015C | `0x8016571C` | não detectado na janela | `j       get_thunder_playerstruct2_` | `decomp/src/wp/wpmanager.c:93` `wpManagerMakeWeapon` (+0x154) | get thunder playerstruct2 |
| 141 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:358` | `get_thunder_anim_struct_` | 0x7D3A0 | `0x80101BA0` | não detectado na janela | `j       get_thunder_anim_struct_` | `decomp/src/ef/efmanager.c:4592` `efManagerPikachuThunderTrailMakeEffect` (+0x18) | get thunder anim struct |
| 142 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:397` | `get_thunder_jolt_anim_struct_` | 0x7D448 | `0x80101C48` | não detectado na janela | `j       get_thunder_jolt_anim_struct_` | `decomp/src/ef/efmanager.c:4622` `efManagerPikachuThunderJoltMakeEffect` (+0x14) | get thunder jolt anim struct |
| 143 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:431` | `get_thunder_jolt_special_struct_1` | 0xE4038 | `0x801695F8` | não detectado na janela | `j       get_thunder_jolt_special_struct_1` | `decomp/src/wp/wppikachu/wppikachuthunderjolt.c:168` `wpPikachuThunderJoltAirMakeWeapon` (+0x14) | get thunder jolt special struct 1 |
| 144 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:469` | `get_thunder_jolt_special_struct_2` | 0xE4E94 | `0x8016A454` | não detectado na janela | `j       get_thunder_jolt_special_struct_2` | `decomp/src/wp/wppikachu/wppikachuthunderjolt.c:700` `wpPikachuThunderJoltGroundMakeWeapon` (+0x28) | get thunder jolt special struct 2 |
| 145 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:500` | `get_thunder_special_struct_1` | 0xE5260 | `0x8016A820` | não detectado na janela | `j       get_thunder_special_struct_1` | `decomp/src/wp/wppikachu/wppikachuthunder.c:156` `wpPikachuThunderHeadMakeWeapon` (+0x14) | get thunder special struct 1 |
| 146 | `.toolchain/pkgs/smashremix-src/src/pikashared.asm:535` | `get_thunder_special_struct_2` | 0xE53D0 | `0x8016A990` | não detectado na janela | `j       get_thunder_special_struct_2` | `decomp/src/wp/wppikachu/wppikachuthunder.c:219` `wpPikachuThunderTrailMakeWeapon` (+0x10) | get thunder special struct 2 |

### PlayerTag.asm

Tag de jogador customizada acima do personagem (criação, posição, imagem e exibição sempre).

Hooks de código: 7; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 147 | `.toolchain/pkgs/smashremix-src/src/PlayerTag.asm:20` | `use_custom_tag_image_` | 0x8D49C | `0x80111C9C` | não detectado na janela | `jal     use_custom_tag_image_` | `decomp/src/if/ifcommon.c:2034` `ifCommonPlayerTagMakeInterface` (+0xB8) | use custom tag image |
| 148 | `.toolchain/pkgs/smashremix-src/src/PlayerTag.asm:26` | `use_custom_tag_image_` | 0x152EF0 | `0x80133D50` | não detectado na janela | `jal     use_custom_tag_image_._results_human` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1330` `mnPlayers1PTrainingCheckPuckInRange` (+0xC) | use custom tag image |
| 149 | `.toolchain/pkgs/smashremix-src/src/PlayerTag.asm:32` | `use_custom_tag_image_` | 0x152F78 | `0x80133DD8` | não detectado na janela | `jal     use_custom_tag_image_._results_cpu` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1219` `mnPlayers1PGameMakeLevel` (+0x3C) | use custom tag image |
| 150 | `.toolchain/pkgs/smashremix-src/src/PlayerTag.asm:72` | `create_custom_tag_` | 0x8D51C | `0x80111D1C` | não detectado na janela | `jal     create_custom_tag_` | `decomp/src/if/ifcommon.c:2034` `ifCommonPlayerTagMakeInterface` (+0x138) | create custom tag |
| 151 | `.toolchain/pkgs/smashremix-src/src/PlayerTag.asm:78` | `create_custom_tag_` | 0x153000 | `0x80133E60` | não detectado na janela | `jal     create_custom_tag_._results` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1354` `mnPlayers1PTrainingUpdateFighter` (+0x30) | create custom tag |
| 152 | `.toolchain/pkgs/smashremix-src/src/PlayerTag.asm:275` | `position_custom_tag_` | 0x8D3CC | `0x80111BCC` | não detectado na janela | `j       position_custom_tag_` | `decomp/src/if/ifcommon.c:1999` `ifCommonPlayerTagProcDisplay` (+0x190) | position custom tag |
| 153 | `.toolchain/pkgs/smashremix-src/src/PlayerTag.asm:312` | `always_display_custom_tag_` | 0x8D28C | `0x80111A8C` | não detectado na janela | `jal     always_display_custom_tag_` | `decomp/src/if/ifcommon.c:1999` `ifCommonPlayerTagProcDisplay` (+0x50) | always display custom tag |

### Poison.asm

Veneno: hook de dano por veneno e correção de star.

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 154 | `.toolchain/pkgs/smashremix-src/src/Poison.asm:17` | `star_fix` | 0x5D008 | `0x800E1808` | não detectado na janela | `j       star_fix` | unmatched | star fix |
| 155 | `.toolchain/pkgs/smashremix-src/src/Poison.asm:36` | `poison_hook` | 0x5D020 | `0x800E1820` | não detectado na janela | `j       poison_hook` | unmatched | poison hook |

### PokemonAnnouncer.asm

Anunciador de voz tipo Pokémon: falas de hit, carga, especiais, clang e outras.

Hooks de código: 29; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 156 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:19` | `there_goes_` | 0x90530 | `0x80114D30` | não detectado na janela | `j       there_goes_` | `decomp/src/if/ifcommon.c:3554` `ifCommonAnnounceEndMessage` (+0xB0) | there goes |
| 157 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:47` | `delay_` | 0x90538 | `0x80114D38` | não detectado na janela | `j       delay_` | `decomp/src/if/ifcommon.c:3554` `ifCommonAnnounceEndMessage` (+0xB8) | delay |
| 158 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:79` | `pokemon_name_` | 0xE9C80 | `0x8016F240` | não detectado na janela | `j       pokemon_name_` | `decomp/src/it/itmanager.c:751` `itManagerMakeItemKind` (+0x8) | pokemon name |
| 159 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:262` | `sleep_announcement_` | 0xC43EC | `0x801499Ac` | não detectado na janela | `j       sleep_announcement_` | `decomp/src/ft/ftcommon/ftcommonfurasleep.c:29` `ftCommonFuraSleepSetStatus` (+0x8) | sleep announcement |
| 160 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:351` | `hit_announcement_` | 0xBBFE8 | `0x801415A8` | não detectado na janela | `j       hit_announcement_` | `decomp/src/ft/ftcommon/ftcommondamage.c:664` `ftCommonDamageGotoDamageStatus` (+0x48) | hit announcement |
| 161 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:487` | `throw_hit_announcement_` | 0xC5BE4 | `0x8014B1A4` | não detectado na janela | `j       throw_hit_announcement_` | `decomp/src/ft/ftcommon/ftcommonthrown2.c:119` `ftCommonThrownReleaseThrownUpdateStats` (+0x1D4) | throw hit announcement |
| 162 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:591` | `thunder_g_announcement_` | 0xCCA04 | `0x80151FC4` | não detectado na janela | `jal     thunder_announcement_` | `decomp/src/ft/ftchar/ftpikachu/ftpikachuspeciallw.c:107` `ftPikachuSpecialLwStartSetStatus` (+0x8) | thunder g announcement |
| 163 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:598` | `thunder_a_announcement_` | 0xCCA44 | `0x80152004` | não detectado na janela | `jal     thunder_announcement_` | `decomp/src/ft/ftchar/ftpikachu/ftpikachuspeciallw.c:115` `ftPikachuSpecialAirLwStartSetStatus` (+0x8) | thunder a announcement |
| 164 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:646` | `quick_g_announcement_` | 0xCD3F0 | `0x801529B0` | não detectado na janela | `jal       quick_announcement_` | `decomp/src/ft/ftchar/ftpikachu/ftpikachuspecialhi.c:104` `ftPikachuSpecialHiStartSetStatus` (+0x10) | quick g announcement |
| 165 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:653` | `quick_a_announcement_` | 0xCD43C | `0x801529FC` | não detectado na janela | `jal     quick_announcement_` | `decomp/src/ft/ftchar/ftpikachu/ftpikachuspecialhi.c:113` `ftPikachuSpecialAirHiStartSetStatus` (+0x10) | quick a announcement |
| 166 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:698` | `charging_announcement_` | 0xE3848 | `0x80168E08` | não detectado na janela | `j       charging_announcement_` | `decomp/src/wp/wpsamus/wpsamuschargeshot.c:273` `wpSamusChargeShotMakeWeapon` (+0x2C) | charging announcement |
| 167 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:749` | `sing_announcement_` | 0x7D900 | `0x80102100` | não detectado na janela | `j       sing_announcement_` | `decomp/src/ef/efmanager.c:4821` `efManagerPurinSingMakeEffect` (+0xC) | sing announcement |
| 168 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:948` | `refresh_burrow_binary_` | 0x529F0 | `0x800D71F0` | não detectado na janela | `j       refresh_burrow_binary_` | `decomp/src/ft/ftmanager.c:149` `ftManagerAllocFighter` (+0x5C) | refresh burrow binary |
| 169 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1149` | `fireflower_` | 0xF0998 | `0x80175F58` | não detectado na janela | `jal       fireflower_` | `decomp/src/it/itcommon/itfflower.c:323` `itFFlowerWeaponFlameMakeWeapon` (+0x10) | fireflower |
| 170 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1164` | `charizard_` | 0xFA688 | `0x8017FC48` | não detectado na janela | `j       charizard_` | `decomp/src/it/itmonster/itlizardon.c:428` `itLizardonWeaponFlameMakeWeapon` (+0x10) | charizard |
| 171 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1260` | `rest_g_announcement_` | 0xCC22C | `0x801517EC` | não detectado na janela | `jal       rest_announcement_` | `decomp/src/ft/ftchar/ftpurin/ftpurinspeciallw.c:45` `ftPurinSpecialLwSetStatus` (+0x8) | rest g announcement |
| 172 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1267` | `rest_a_announcement_` | 0xCC264 | `0x80151824` | não detectado na janela | `jal       rest_announcement_` | `decomp/src/ft/ftchar/ftpurin/ftpurinspeciallw.c:52` `ftPurinSpecialAirLwSetStatus` (+0x8) | rest a announcement |
| 173 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1315` | `pound_g_announcement_` | 0xCBF4C | `0x8015150C` | não detectado na janela | `jal       pound_announcement_` | `decomp/src/ft/ftchar/ftpurin/ftpurinspecialn.c:124` `ftPurinSpecialAirNSetStatus` (+0x8) | pound g announcement |
| 174 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1322` | `pound_a_announcement_` | 0xCBF0C | `0x801514CC` | não detectado na janela | `jal       pound_announcement_` | `decomp/src/ft/ftchar/ftpurin/ftpurinspecialn.c:116` `ftPurinSpecialNSetStatus` (+0x8) | pound a announcement |
| 175 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1431` | `hung_tough_announcement_` | 0xDF6C0 | `0x80164C80` | não detectado na janela | `j       hung_tough_announcement_` | `decomp/src/ft/ftpublic.c:139` `ftPublicPlayCommon` (+0x3C) | hung tough announcement |
| 176 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1491` | `recovery_announcement_` | 0x65BD8 | `0x800EA3D8` | não detectado na janela | `j       recovery_announcement_` | `decomp/src/ft/ftparam.c:1719` `ftParamSetHealDamage` (+0x4) | recovery announcement |
| 177 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1540` | `recovery_announcement_magnet_` | 0x5EBDC | `0x800E33DC` | não detectado na janela | `j       recovery_announcement_magnet_` | `decomp/src/ft/ftmain.c:2458` `ftMainUpdateAbsorbStatWeapon` (+0xD4) | recovery announcement magnet |
| 178 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1592` | `taunt_announcement_` | 0xC9130 | `0x8014E6F0` | não detectado na janela | `j       taunt_announcement_` | `decomp/src/ft/ftcommon/ftcommonappeal.c:20` `ftCommonAppealSetStatus` (+0x10) | taunt announcement |
| 179 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1641` | `clang_announcement_` | 0x5DED8 | `0x800E26D8` | não detectado na janela | `j       clang_announcement_` | `decomp/src/ft/ftmain.c:2076` `ftMainSetHitInteractStats` (+0x1C) | clang announcement |
| 180 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1696` | `stun_announcement_` | 0xC42E8 | `0x801498A8` | não detectado na janela | `j       stun_announcement_` | `decomp/src/ft/ftcommon/ftcommonfurafura.c:31` `ftCommonFuraFuraSetStatus` (+0x4) | stun announcement |
| 181 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1741` | `time_up_announcement_` | 0x8EFC4 | `0x801137C4` | não detectado na janela | `j       time_up_announcement_` | `decomp/src/if/ifcommon.c:2870` `ifCommonBattleEndPlaySoundQueue` (+0x28) | time up announcement |
| 182 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1780` | `team_ko_announcement_` | 0xB814C | `0x8013D70C` | não detectado na janela | `j       team_ko_announcement_` | `decomp/src/ft/ftcommon/ftcommonsleep.c:53` `ftCommonSleepProcUpdate` (+0x3C) | team ko announcement |
| 183 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1857` | `air_specials_announcement_` | 0xCB978 | `0x80150F38` | não detectado na janela | `j       air_specials_announcement_` | `decomp/src/ft/ftcommon/ftcommonspecialair.c:175` `ftCommonSpecialAirCheckInterruptCommon` (+0x30) | air specials announcement |
| 184 | `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm:1930` | `ground_specials_announcement_` | 0xCBB00 | `0x801510C0` | não detectado na janela | `j       ground_specials_announcement_` | `decomp/src/ft/ftcommon/ftcommonspecialn.c:119` `ftCommonSpecialNCheckInterruptCommon` (+0x28) | ground specials announcement |

### Practice.asm

Modo prática: flash no Z-cancel e cor de overlay.

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 185 | `.toolchain/pkgs/smashremix-src/src/Practice.asm:26` | `overlay_colour_` | 0x0005DE0C | `0x800E260C` | Toggle: Color Overlays | `j       overlay_colour_` | `decomp/src/ft/ftmain.c:2054` `ftMainProcPhysicsMapDefault` (+0x8) | overlay colour |
| 186 | `.toolchain/pkgs/smashremix-src/src/Practice.asm:200` | `flash_on_z_cancel_` | 0x000CB528 | `0x80150AE8` | Toggle: Flash On Z-Cancel | `jal     flash_on_z_cancel_` | `decomp/src/ft/ftcommon/ftcommonattackair.c:55` `ftCommonAttackAirProcMap` (+0xE0) | flash on z cancel |

### Practice_1P.asm

Prática 1P: segurar L para tentar de novo, retries de bônus e prevenção de softlock do Master Hand.

Hooks de código: 12; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 187 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:22` | `_1p_practice` | 0x52128 | `0x800D6928` | não detectado na janela | `jal     _1p_practice._check_mode` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0x14C) | 1p practice |
| 188 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:90` | `prevent_masterhand_softlock_` | 0x110F60 | `0x80192700` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pgameboss.c:1035` `sc1PGameBossSetBossPlayer` (+0x8) | prevent masterhand softlock |
| 189 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:94` | `prevent_masterhand_softlock_` | 0x110F68 | `0x80192708` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pgameboss.c:1035` `sc1PGameBossSetBossPlayer` (+0x10) | prevent masterhand softlock |
| 190 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:98` | `prevent_masterhand_softlock_` | 0x110F80 | `0x80192720` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pgameboss.c:1035` `sc1PGameBossSetBossPlayer` (+0x28) | prevent masterhand softlock |
| 191 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:102` | `prevent_masterhand_softlock_` | 0x110F8C | `0x8019272C` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pgameboss.c:1035` `sc1PGameBossSetBossPlayer` (+0x34) | prevent masterhand softlock |
| 192 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:106` | `prevent_masterhand_softlock_` | 0x110F94 | `0x80192734` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pgameboss.c:1035` `sc1PGameBossSetBossPlayer` (+0x3C) | prevent masterhand softlock |
| 193 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:110` | `prevent_masterhand_softlock_` | 0x110FA0 | `0x80192740` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pgameboss.c:1035` `sc1PGameBossSetBossPlayer` (+0x48) | prevent masterhand softlock |
| 194 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:114` | `prevent_masterhand_softlock_` | 0x110FA8 | `0x80192748` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pgameboss.c:1035` `sc1PGameBossSetBossPlayer` (+0x50) | prevent masterhand softlock |
| 195 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:123` | `hold_l_to_retry_` | 0x8FF24 | `0x80114724` | não detectado na janela | `j       hold_l_to_retry_` | `decomp/src/if/ifcommon.c:3347` `ifCommonBattleBossDefeatUpdateInterface` (exato) | hold l to retry |
| 196 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:220` | `hold_l_to_retry_stage_clear_` | 0x17DCE0 | `0x80134A90` | não detectado na janela | `jal     hold_l_to_retry_stage_clear_` | `decomp/src/mn/mnplayers/mnplayersvs.c:1870` `mnPlayersVSMakeFighter` (+0x4) | hold l to retry stage clear |
| 197 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:292` | `hold_l_to_retry_bonus_1` | 0x113358 | `0x8018EC18` | não detectado na janela | `j      hold_l_to_retry_bonus_1` | `decomp/src/sc/sc1pmode/sc1pgame.c:1908` `sc1PGameTeamStockDisplayProcDisplay` (+0x60) | hold l to retry bonus 1 |
| 198 | `.toolchain/pkgs/smashremix-src/src/Practice_1P.asm:314` | `hold_l_to_retry_bonus_2` | 0x11345C | `0x8018ED1C` | não detectado na janela | `j      hold_l_to_retry_bonus_2` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1256` `sc1PTrainingModeInitMenuOptionSpriteAttrs` (+0x78) | hold l to retry bonus 2 |

### Rage.asm

Rage: multiplicador de knockback, ataque direto/projétil, arremesso e gráficos.

Hooks de código: 5; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 199 | `.toolchain/pkgs/smashremix-src/src/Rage.asm:17` | `direct_attack` | 0x5F770 | `0x800E3F70` | Toggle: Rage | `j       direct_attack` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0xB4) | direct attack |
| 200 | `.toolchain/pkgs/smashremix-src/src/Rage.asm:45` | `projectile_attack` | 0x5F8FC | `0x800E40FC` | Toggle: Rage | `j       projectile_attack` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0x240) | projectile attack |
| 201 | `.toolchain/pkgs/smashremix-src/src/Rage.asm:83` | `throw` | 0xC5AA0 | `0x8014B060` | Toggle: Rage | `j       throw` | `decomp/src/ft/ftcommon/ftcommonthrown2.c:119` `ftCommonThrownReleaseThrownUpdateStats` (+0x90) | throw |
| 202 | `.toolchain/pkgs/smashremix-src/src/Rage.asm:110` | `knockback_multiply` | 0x65760 | `0x800E9F60` | Toggle: Rage | `j       knockback_multiply` | `decomp/src/ft/ftparam.c:1606` `ftParamGetCommonKnockback` (+0x1E8) | knockback multiply |
| 203 | `.toolchain/pkgs/smashremix-src/src/Rage.asm:227` | `gen_gfx` | 0x5DBD0 | `0x800E23D0` | Toggle: Rage | `j       gen_gfx` | `decomp/src/ft/ftmain.c:1875` `ftMainProcPhysicsMap` (+0x388) | gen gfx |

### Reflect.asm

Reflect: input de reflexão, extensão de reflexão para itens/projéteis e correção de direção.

Hooks de código: 13; patches de tabela: 10; origins de dados (não contados como hook): 0. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 204 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:27` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.WOLF] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 205 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:29` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.FALCO] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 206 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:31` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.JFOX] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 207 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:33` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.JNESS] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 208 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:35` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.LUCAS] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 209 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:37` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.NLUCAS] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 210 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:39` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.MARINA] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 211 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:41` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.DEDEDE] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 212 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:43` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.PIANO] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 213 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:45` | `custom_reflect_type` | - | `tabela fighter_reflect[Character.id.SLIPPY] +0x1` | não detectado na janela | - | n/a (patch de dado) | custom reflect type |
| 214 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:55` | `extend_reflect_absorb_character_check_` | 0xAE290 | `0x80133850` | não detectado na janela | `j		extend_reflect_absorb_character_check_` | `decomp/src/mn/mnvsmode/mnvsmode.c:1175` `mnVSModeGetShade` (exato) | extend reflect absorb character check |
| 215 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:93` | `set_projectile_detection_radius_` | 0xB0680 | `0x80135C40` | não detectado na janela | `j		set_projectile_detection_radius_` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:2263` `mnPlayers1PTrainingUpdatePuck` (+0x28) | set projectile detection radius |
| 216 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:131` | `set_item_detection_radius_` | 0xB0914 | `0x80135ED4` | não detectado na janela | `j		set_item_detection_radius_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2397` `mnPlayers1PBonusMakePuckCamera` (+0x8C) | set item detection radius |
| 217 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:165` | `extend_projectile_reflect_initial_` | 0xB0B74 | `0x80135E90` | não detectado na janela | `j     extend_projectile_reflect_initial_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2397` `mnPlayers1PBonusMakePuckCamera` (+0x48) | extend projectile reflect initial |
| 218 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:251` | `extend_item_reflect_initial_` | 0xB08D0 | `0x80136134` | não detectado na janela | `j     extend_item_reflect_initial_` | `decomp/src/mn/mnplayers/mnplayersvs.c:2622` `mnPlayersVSUpdateFighter` (+0xC) | extend item reflect initial |
| 219 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:327` | `maintain_reflect_input_` | 0xB226C | `0x8013782C` | não detectado na janela | `j     maintain_reflect_input_` | `decomp/src/mn/mnplayers/mnplayersvs.c:3369` `mnPlayersVSGetPuckFighterKind` (exato) | maintain reflect input |
| 220 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:421` | `apply_reflect_input_` | 0xB2A98 | `0x80138058` | não detectado na janela | `j     apply_reflect_input_` | `decomp/src/mn/mnplayers/mnplayersvs.c:3567` `mnPlayersVSRecallPuck` (+0x60) | apply reflect input |
| 221 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:507` | `override_reflectability_` | 0x60AD0 | `0x800E52D0` | não detectado na janela | `j       override_reflectability_` | `decomp/src/ft/ftmain.c:3372` `ftMainSearchHitWeapon` (+0x3FC) | override reflectability |
| 222 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:562` | `extend_reflect_types` | 0x61E3C | `0x800E663C` | não detectado na janela | `j       extend_reflect_types` | unmatched | extend reflect types |
| 223 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:597` | `override_projectile_reflect_routine_` | 0xE1794 | `0x80166D54` | não detectado na janela | `j       override_projectile_reflect_routine_` | `decomp/src/wp/wpprocess.c:462` `wpProcessProcHitCollisions` (+0x170) | override projectile reflect routine |
| 224 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:676` | `override_item_reflect_routine_` | 0xEBCDC | `0x8017129C` | não detectado na janela | `j       override_item_reflect_routine_` | `decomp/src/it/itprocess.c:1014` `itProcessProcHitCollisions` (+0x1D8) | override item reflect routine |
| 225 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:767` | `projectile_direction_fix_custom_` | 0xE2B2C | `0x801680EC` | não detectado na janela | `j      projectile_direction_fix_custom_` | `decomp/src/wp/wpmain.c:102` `wpMainReflectorSetLR` (exato) | projectile direction fix custom |
| 226 | `.toolchain/pkgs/smashremix-src/src/Reflect.asm:818` | `item_direction_fix_custom_` | 0xEDE74 | `0x80173434` | não detectado na janela | `j      item_direction_fix_custom_` | `decomp/src/it/itmain.c:839` `itMainCommonProcReflector` (exato) | item direction fix custom |

### Render.asm

Renderização: setup de fonte e tabelas de largura de caracteres (origin de dados).

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 58. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 227 | `.toolchain/pkgs/smashremix-src/src/Render.asm:2116` | `setup_` | 0x000073B4 | `0x800067B4` | não detectado na janela | `jal     setup_` | `decomp/src/sys/taskman.c:1251` `syTaskmanLoadScene` (+0x26C) | setup |

### resultsscreen.asm

Tela de resultados: logo do vencedor, animações de vitória, BGM/FGM do vencedor e registro de vitórias.

Hooks de código: 19; patches de tabela: 7; origins de dados (não contados como hook): 3. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 228 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:12` | `load_character_files_` | 0x157DF8 | `0x80138C58` | não detectado na janela | `j       load_character_files_` | `decomp/src/mn/mnplayers/mnplayersvs.c:4024` `mnPlayersVSMakePuckCamera` (+0x4C) | load character files |
| 229 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:43` | `vs_record_fix_` | 0x150DD4 | `0x80131C34` | não detectado na janela | `jal     _character` | `decomp/src/mv/mvopening/mvopeningcliff.c:140` `mvOpeningCliffMakeHills` (exato) | vs record fix |
| 230 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:49` | `vs_record_fix_` | 0x150F08 | `0x80131D68` | não detectado na janela | `jal     _opponent` | `decomp/src/mn/mncommon/mnmodeselect.c:344` `mnModeSelectMakeOption` (exato) | vs record fix |
| 231 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:103` | `fix_port_index_` | 0x152B68 | `0x801339C8` | não detectado na janela | `jal     fix_port_index_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1133` `mnPlayers1PBonusMakeBestTaskCount` (exato) | fix port index |
| 232 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:117` | `get_winner_fgm_` | 0x00151164 | `0x80131FC4` | não detectado na janela | `j       get_winner_fgm_` | `decomp/src/mn/mnoption/mnoption.c:351` `mnOptionMakeScreenAdjust` (exato) | get winner fgm |
| 233 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:136` | `winner_logo_fix_` | 0x151E18 | `0x80132C78` | não detectado na janela | `jal     winner_logo_fix_` | `decomp/src/mn/mnplayers/mnplayersvs.c:688` `mnPlayersVSShutterProcUpdate` (+0xC) | winner logo fix |
| 234 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:168` | `winner_logo_zoom_fix_` | 0x151E64 | `0x80132CC4` | não detectado na janela | `jal     winner_logo_zoom_fix_` | `decomp/src/mn/mnoption/mnoption.c:855` `mnOptionMakeDecalsCamera` (+0x4) | winner logo zoom fix |
| 235 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:200` | `winner_logo_color_fix_` | 0x151E88 | `0x80132CE8` | não detectado na janela | `jal     winner_logo_color_fix_` | `decomp/src/mn/mndata/mnvsrecord.c:768` `mnVSRecordMakeColumnArrows` (+0x20) | winner logo color fix |
| 236 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:236` | `label_height_fix_` | 0x152D00 | `0x80133B60` | não detectado na janela | `jal     label_height_fix_` | `decomp/src/mv/mvopening/mvopeningroom.c:1073` `mvOpeningRoomTransitionOverlayProcDisplay` (+0x8) | label height fix |
| 237 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:241` | `label_height_fix_` | 0x152D58 | `0x80133BB8` | não detectado na janela | `jal     label_height_fix_` | `decomp/src/mn/mnplayers/mnplayersvs.c:1218` `mnPlayersVSMakeGameRuleNumber` (+0x8) | label height fix |
| 238 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:246` | `label_height_fix_` | 0x152DB0 | `0x80133C10` | não detectado na janela | `jal     label_height_fix_` | `decomp/src/sc/sc1pmode/sc1pstageclear.c:1633` `sc1PStageClearMakeWallpaperCamera` (+0x28) | label height fix |
| 239 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:268` | `get_str_wins_lx_` | 0x1534DC | `0x8013433C` | não detectado na janela | `j       get_str_wins_lx_` | `decomp/src/mv/mvopening/mvopeningroom.c:1239` `mvOpeningRoomCheckSetFramebuffer` (+0x24) | get str wins lx |
| 240 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:306` | `get_str_winner_info_` | 0x1535E0 | `0x80134440` | não detectado na janela | `j       get_str_winner_info_` | `decomp/src/sc/sc1pmode/sc1pintro.c:1871` `sc1PIntroMakeDecalsCamera` (+0x14) | get str winner info |
| 241 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:334` | `add_ampersand_` | 0x1530C8 | `0x80133F28` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pstageclear.c:1789` `sc1PStageClearCheckNoTimer` (exato) | add ampersand |
| 242 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:355` | `add_ampersand_` | 0x1532F4 | `0x80134154` | não detectado na janela | `jal     add_ampersand_._texture` | `decomp/src/mn/mncommon/mntitlefiles.c:35` `mnTitleLoadFiles` (+0x14) | add ampersand |
| 243 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:361` | `add_ampersand_` | 0x15337C | `0x801341DC` | não detectado na janela | `jal     add_ampersand_._width` | `decomp/src/mv/mvopening/mvopeningroom.c:1191` `mvOpeningRoomGetDroppedFighterKind` (+0x28) | add ampersand |
| 244 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:388` | `get_victory_bgm_` | 0x1578CC | `0x8013872C` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsresults.c:3287` `mnVSResultsPlayWinBGM` (+0x18) | get victory bgm |
| 245 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:419` | `victory_animations_` | 0x152638 | `0x80133498` | não detectado na janela | `j       pick_victory_animation_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:989` `mnPlayers1PBonusGetMins` (+0x10) | victory animations |
| 246 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:424` | `victory_animations_` | 0x152650 | `0x801334B0` | não detectado na janela | `j       pick_victory_animation_` | `decomp/src/mn/mnvsmode/mnvsoptions.c:1189` `mnVSOptionsMakeDecal` (exato) | victory animations |
| 247 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:675` | `series_logo_color` | - | `tabela winner_fgm[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | series logo color |
| 248 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:699` | `series_logo_color` | - | `tabela label_height[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | series logo color |
| 249 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:704` | `series_logo_color` | - | `tabela str_wins_lx[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | series logo color |
| 250 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:714` | `series_logo_color` | - | `tabela str_winner_ptr[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | series logo color |
| 251 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:717` | `series_logo_color` | - | `tabela str_winner_lx[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | series logo color |
| 252 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:720` | `series_logo_color` | - | `tabela str_winner_scale[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | series logo color |
| 253 | `.toolchain/pkgs/smashremix-src/src/resultsscreen.asm:729` | `series_logo_color` | - | `tabela winner_bgm[{id}] +0x4` | não detectado na janela | - | n/a (patch de dado) | series logo color |

### samusshared.asm

Helpers compartilhados da Samus: carga de tiro, bomba, grapple e poder Kirby.

Hooks de código: 9; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 254 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:13` | `get_bombinstructions_struct_` | 0xE3D74 | `0x80169334` | não detectado na janela | `j       get_bombinstructions_struct_` | `decomp/src/wp/wpsamus/wpsamusbomb.c:214` `wpSamusBombMakeWeapon` (+0xC) | get bombinstructions struct |
| 255 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:50` | `get_chargeinstructions_struct_` | 0xE3854 | `0x80168E14` | não detectado na janela | `j       get_chargeinstructions_struct_` | `decomp/src/wp/wpsamus/wpsamuschargeshot.c:273` `wpSamusChargeShotMakeWeapon` (+0x38) | get chargeinstructions struct |
| 256 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:99` | `fully_charged_check_` | 0x66418 | `0x800EAC18` | não detectado na janela | `jal     fully_charged_check_` | `decomp/src/ft/ftparam.c:1974` `ftParamMakeEffect` (+0x3C) | fully charged check |
| 257 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:128` | `kirby_power_check_flash_` | 0x651C4 | `0x800E99C4` | não detectado na janela | `jal     kirby_power_check_flash_` | `decomp/src/ft/ftparam.c:1398` `ftParamResetStatUpdateColAnim` (+0xF0) | kirby power check flash |
| 258 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:209` | `kirby_power_change_` | 0xDC904 | `0x80161EC4` | não detectado na janela | `j       kirby_power_change_` | `decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:85` `ftKirbySpecialNInitPassiveVars` (+0x10) | kirby power change |
| 259 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:242` | `throw_ball_graphic` | 0xC4654 | `0x80149C14` | não detectado na janela | `jal       throw_ball_graphic` | `decomp/src/ft/ftcommon/ftcommoncatch1.c:96` `ftCommonCatchSetStatus` (+0x6C) | throw ball graphic |
| 260 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:268` | `throw_ball_graphic_2` | 0xC4D1C | `0x8014A2DC` | não detectado na janela | `j       throw_ball_graphic_2` | `decomp/src/ft/ftcommon/ftcommonthrow.c:72` `ftCommonThrowSetStatus` (+0xF4) | throw ball graphic 2 |
| 261 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:291` | `get_entry_anim_gfx_struct` | 0x7EC7C | `0x8010347C` | não detectado na janela | `j       get_entry_anim_gfx_struct` | `decomp/src/ef/efmanager.c:5689` `efManagerSamusEntryPointMakeEffect` (+0x8) | get entry anim gfx struct |
| 262 | `.toolchain/pkgs/smashremix-src/src/samusshared.asm:322` | `get_grapple_anim_gfx_struct` | 0x7D688 | `0x80101E88` | não detectado na janela | `j       get_grapple_anim_gfx_struct` | `decomp/src/ef/efmanager.c:4708` `efManagerSamusGrappleBeamGlowMakeEffect` (+0x8) | get grapple anim gfx struct |

### SFXReplace.asm

Substituição de SFX (incluindo bounce para baixo).

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 263 | `.toolchain/pkgs/smashremix-src/src/SFXReplace.asm:66` | `sfx_replace` | 0x5B064 | `0x800DF864` | não detectado na janela | `jal     sfx_replace` | unmatched | sfx replace |
| 264 | `.toolchain/pkgs/smashremix-src/src/SFXReplace.asm:176` | `sfx_replace_down_bounce` | 0xBEEB0 | `0x80144470` | não detectado na janela | `jal     sfx_replace_down_bounce` | `decomp/src/ft/ftcommon/ftcommondownwaitbounce.c:100` `ftCommonDownBounceUpdateEffects` (+0x48) | sfx replace down bounce |

### Shield.asm

Escudo: correção de cor e alteração da cor do ovo do Yoshi.

Hooks de código: 7; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 265 | `.toolchain/pkgs/smashremix-src/src/Shield.asm:48` | `color_fix_` | 0x0007C8E8 | `0x801010E8` | não detectado na janela | `j       color_fix_` | `decomp/src/ef/efmanager.c:4192` `efManagerShieldProcDisplay` (+0xC4) | color fix |
| 266 | `.toolchain/pkgs/smashremix-src/src/Shield.asm:190` | `alter_yoshi_egg_color_` | 0x7CBA8 | `0x801013A8` | não detectado na janela | `jal     alter_yoshi_egg_color_._shield` | `decomp/src/ef/efmanager.c:4258` `efManagerYoshiShieldMakeEffect` (+0x34) | alter yoshi egg color |
| 267 | `.toolchain/pkgs/smashremix-src/src/Shield.asm:195` | `alter_yoshi_egg_color_` | 0x7E98C | `0x8010318C` | não detectado na janela | `jal     alter_yoshi_egg_color_._shield_roll` | `decomp/src/ef/efmanager.c:5531` `efManagerYoshiEggEscapeMakeEffect` (+0x3C) | alter yoshi egg color |
| 268 | `.toolchain/pkgs/smashremix-src/src/Shield.asm:200` | `alter_yoshi_egg_color_` | 0xD9534 | `0x8015EAF4` | não detectado na janela | `j       alter_yoshi_egg_color_._eggthrow` | `decomp/src/ft/ftchar/ftyoshi/ftyoshispecialhi.c:46` `ftYoshiSpecialHiUpdateEggVars` (+0x98) | alter yoshi egg color |
| 269 | `.toolchain/pkgs/smashremix-src/src/Shield.asm:206` | `alter_yoshi_egg_color_` | 0xC7AC0 | `0x8014D080` | não detectado na janela | `jal     alter_yoshi_egg_color_._egglay_setup` | `decomp/src/ft/ftcommon/ftcommoncaptureyoshi.c:355` `ftCommonYoshiEggSetStatus` (+0x160) | alter yoshi egg color |
| 270 | `.toolchain/pkgs/smashremix-src/src/Shield.asm:211` | `alter_yoshi_egg_color_` | 0xC73C4 | `0x8014C984` | não detectado na janela | `jal     alter_yoshi_egg_color_._egglay` | `decomp/src/ft/ftcommon/ftcommoncaptureyoshi.c:129` `ftCommonYoshiEggMakeEffect` (+0x2C) | alter yoshi egg color |
| 271 | `.toolchain/pkgs/smashremix-src/src/Shield.asm:216` | `alter_yoshi_egg_color_` | 0x7E75C | `0x80102F5C` | não detectado na janela | `jal     alter_yoshi_egg_color_._entry` | `decomp/src/ef/efmanager.c:5443` `efManagerYoshiEntryEggMakeEffect` (+0x28) | alter yoshi egg color |

### SingleButtonMode.asm

Modo de um botão: tratamento de input da CPU.

Hooks de código: 4; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 272 | `.toolchain/pkgs/smashremix-src/src/SingleButtonMode.asm:119` | `handle_cpu_inputs_` | 0xAC73C | `0x80131CFC` | não detectado na janela | `j       handle_cpu_inputs_._a_b` | `decomp/src/mn/mncommon/mntitle.c:349` `mnTitleInitVars` (+0x8) | handle cpu inputs |
| 273 | `.toolchain/pkgs/smashremix-src/src/SingleButtonMode.asm:124` | `handle_cpu_inputs_` | 0xAC764 | `0x80131D24` | não detectado na janela | `j       handle_cpu_inputs_._a_b` | `decomp/src/mv/mvopening/mvopeningroom.c:237` `mvOpeningRoomMakeDesk` (+0x1C) | handle cpu inputs |
| 274 | `.toolchain/pkgs/smashremix-src/src/SingleButtonMode.asm:129` | `handle_cpu_inputs_` | 0xAC784 | `0x80131D44` | não detectado na janela | `j       handle_cpu_inputs_._z_l` | `decomp/src/mn/mndata/mncharacters.c:1463` `mnCharactersMakeDecals` (exato) | handle cpu inputs |
| 275 | `.toolchain/pkgs/smashremix-src/src/SingleButtonMode.asm:134` | `handle_cpu_inputs_` | 0xAC7A4 | `0x80131D64` | não detectado na janela | `j       handle_cpu_inputs_._z_l` | `decomp/src/mn/mndata/mndata.c:241` `mnDataMakeCharacters` (+0x8) | handle cpu inputs |

### SinglePlayer.asm

Modo 1P: tally, high score 1P, mapeamento de ambiente e prévia de figurino em dupla.

Hooks de código: 54; patches de tabela: 0; origins de dados (não contados como hook): 3. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 276 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:22` | `extend_high_score_btx_count_check_` | 0x149BAC | `0x80133B7C` | não detectado na janela | `j       extend_high_score_btx_count_check_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1179` `mnPlayers1PBonusCheckBonusComplete` (exato) | extend high score btx count check |
| 277 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:103` | `extend_high_score_btx_count_` | 0x1499C0 | `0x80133990` | não detectado na janela | `j       extend_high_score_btx_count_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1123` `mnPlayers1PBonusGetBestTaskCount` (exato) | extend high score btx count |
| 278 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:175` | `extend_high_score_btx_time_` | 0x149440 | `0x80133410` | não detectado na janela | `j       extend_high_score_btx_time_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:964` `mnPlayers1PBonusGetBestTime` (exato) | extend high score btx time |
| 279 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:249` | `extend_high_score_btt_count_write_` | 0x1130E4 | `0x8018E9A4` | não detectado na janela | `j       extend_high_score_btt_count_write_` | `decomp/src/sc/sc1pmode/sc1pgame.c:1765` `sc1PGameWaitStageBossUpdate` (exato) | extend high score btt count write |
| 280 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:340` | `extend_high_score_btt_time_write_` | 0x11310C | `0x8018E9CC` | não detectado na janela | `j       extend_high_score_btt_time_write_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1151` `sc1PTrainingModeItemDisplayProcDisplay` (+0x20) | extend high score btt time write |
| 281 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:459` | `extend_high_score_btt_new_record_check_` | 0x111C9C | `0x8018D55C` | não detectado na janela | `j       extend_high_score_btt_new_record_check_` | `decomp/src/sc/sccommon/scautodemo.c:369` `scAutoDemoResetFocusPlayerAll` (+0x34) | extend high score btt new record check |
| 282 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:550` | `extend_high_score_btp_count_write_` | 0x113158 | `0x8018EA18` | não detectado na janela | `j       extend_high_score_btp_count_write_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1151` `sc1PTrainingModeItemDisplayProcDisplay` (+0x6C) | extend high score btp count write |
| 283 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:642` | `extend_high_score_btp_time_write_` | 0x113180 | `0x8018EA40` | não detectado na janela | `j       extend_high_score_btp_time_write_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1151` `sc1PTrainingModeItemDisplayProcDisplay` (+0x94) | extend high score btp time write |
| 284 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:762` | `extend_high_score_btp_new_record_check_` | 0x112100 | `0x8018D9C0` | não detectado na janela | `j       extend_high_score_btp_new_record_check_` | `decomp/src/mv/mvopening/mvopeningyoshi.c:325` `mvOpeningYoshiPosedFighterProcUpdate` (exato) | extend high score btp new record check |
| 285 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:854` | `extend_btx_tally_` | 0x14CCE8 | `0x80136CB8` | não detectado na janela | `j       extend_btx_tally_._check_counts` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:2749` `mnPlayers1PGameBackTo1PMode` (exato) | extend btx tally |
| 286 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:859` | `extend_btx_tally_` | 0x1496C8 | `0x80133698` | não detectado na janela | `j       extend_btx_tally_._ms` | `decomp/src/mn/mnvsmode/mnvsoptions.c:1265` `mnVSOptionsMakeLabelUnderlineCamera` (exato) | extend btx tally |
| 287 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:864` | `extend_btx_tally_` | 0x149698 | `0x80133668` | não detectado na janela | `j     extend_btx_tally_._ms_exclude_check` | `decomp/src/sc/sc1pmode/sc1pstageclear.c:1454` `sc1PStageClearCheckHaveBonusStatID` (exato) | extend btx tally |
| 288 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:869` | `extend_btx_tally_` | 0x14965C | `0x8013362C` | não detectado na janela | `j       extend_btx_tally_._s` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1074` `mnPlayers1PTrainingGetStatusSelected` (+0x10) | extend btx tally |
| 289 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:874` | `extend_btx_tally_` | 0x14962C | `0x801335FC` | não detectado na janela | `j       extend_btx_tally_._s_exclude_check` | `decomp/src/mn/mndata/mnvsrecord.c:975` `mnVSRecordMakeColumnIcons` (exato) | extend btx tally |
| 290 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:879` | `extend_btx_tally_` | 0x1495F0 | `0x801335C0` | não detectado na janela | `j       extend_btx_tally_._m` | `decomp/src/mn/mnoption/mnoption.c:1210` `mnOptionStartScene` (exato) | extend btx tally |
| 291 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:884` | `extend_btx_tally_` | 0x1495C0 | `0x80133590` | não detectado na janela | `j       extend_btx_tally_._m_exclude_check` | `decomp/src/mv/mvopening/mvopeningroom.c:881` `mvOpeningRoomInitScene3Cameras` (exato) | extend btx tally |
| 292 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1035` | `set_btt_stage_id_` | 0x111950 | `0x8018D210` | não detectado na janela | `j       set_btt_stage_id_` | `decomp/src/sc/sc1pmode/sc1pgame.c:832` `sc1PGameFuncUpdate` (+0x10) | set btt stage id |
| 293 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1086` | `set_btp_stage_id_` | 0x111964 | `0x8018D224` | não detectado na janela | `j       set_btp_stage_id_` | `decomp/src/sc/sccommon/scautodemo.c:277` `scAutoDemoSetCameraPlayerZoom` (+0x4) | set btp stage id |
| 294 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1139` | `extend_high_score_1p_` | 0x13C958 | `0x80134758` | não detectado na janela | `j       extend_high_score_1p_` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1499` `mnPlayers1PGameGetHiScore` (exato) | extend high score 1p |
| 295 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1194` | `extend_high_score_1p_bonus_count_` | 0x13CB68 | `0x80134968` | não detectado na janela | `j       extend_high_score_1p_bonus_count_` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1555` `mnPlayers1PGameGetBonusCount` (exato) | extend high score 1p bonus count |
| 296 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1243` | `extend_high_score_1p_difficulty_` | 0x13CAD8 | `0x801348D8` | não detectado na janela | `j       extend_high_score_1p_difficulty_` | `decomp/src/mn/mndata/mnvsrecord.c:1455` `mnVSRecordGetAvg` (+0x24) | extend high score 1p difficulty |
| 297 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1300` | `extend_high_score_1p_write_` | 0x51F44 | `0x800D6744` | não detectado na janela | `j       extend_high_score_1p_write_` | `decomp/src/sc/sc1pmode/sc1pmanager.c:259` `sc1PManagerTrySaveBackup` (+0xC) | extend high score 1p write |
| 298 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1372` | `extend_high_score_1p_write_difficulty` | 0x51F54 | `0x800D6754` | não detectado na janela | `j       extend_high_score_1p_write_difficulty` | `decomp/src/sc/sc1pmode/sc1pmanager.c:259` `sc1PManagerTrySaveBackup` (+0x1C) | extend high score 1p write difficulty |
| 299 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1389` | `extend_1p_tally_` | 0x13CD80 | `0x80134B80` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1609` `mnPlayers1PGameGetTotalHiScore` (+0x1C) | extend 1p tally |
| 300 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1393` | `extend_1p_tally_` | 0x13CED4 | `0x80134CD4` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1842` `mnPlayers1PTrainingSetCursorGrab` (+0xC) | extend 1p tally |
| 301 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1401` | `set_btt_stage_id_1p_` | 0x1118E8 | `0x8018D1A8` | não detectado na janela | `j       set_btt_stage_id_1p_` | `decomp/src/sc/sc1pmode/sc1pgame.c:817` `sc1PGameSetGameEnd` (exato) | set btt stage id 1p |
| 302 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1450` | `set_btp_stage_id_1p_` | 0x111910 | `0x8018D1D0` | não detectado na janela | `j       set_btp_stage_id_1p_` | `decomp/src/sc/sc1pmode/sc1pgame.c:817` `sc1PGameSetGameEnd` (+0x28) | set btp stage id 1p |
| 303 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1497` | `write_extended_high_score_table_` | 0x00050014 | `0x800D4634` | não detectado na janela | `j       write_extended_high_score_table_` | `decomp/src/lb/lbbackup.c:39` `lbBackupWrite` (+0x40) | write extended high score table |
| 304 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1536` | `load_extended_high_score_table_` | 0x000500C4 | `0x800D46E4` | não detectado na janela | `j       load_extended_high_score_table_` | `decomp/src/lb/lbbackup.c:47` `lbBackupIsSramValid` (+0xA0) | load extended high score table |
| 305 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1770` | `get_name_texture_` | 0x12BA4C | `0x8013270C` | não detectado na janela | `jal     get_name_texture_` | `decomp/src/sc/sccommon/scstaffroll.c:844` `scStaffrollHighlightThreadUpdate` (exato) | get name texture |
| 306 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1776` | `get_name_texture_` | 0x12BA5C | `0x8013271C` | não detectado na janela | `jal     get_name_texture_._marina` | `decomp/src/mn/mnplayers/mnplayersvs.c:525` `mnPlayersVSMakeTeamSelect` (exato) | get name texture |
| 307 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1835` | `get_fgm_` | 0x12DA2C | `0x801346EC` | não detectado na janela | `jal     get_fgm_` | `decomp/src/mn/mndata/mnvsrecord.c:1430` `mnVSRecordRankingHighlightProcDisplay` (+0x14) | get fgm |
| 308 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1961` | `get_name_delay_` | 0x12D9DC | `0x8013469C` | não detectado na janela | `jal     get_name_delay_` | `decomp/src/mn/mnvsmode/mnvsresults.c:1578` `mnVSResultsGetHundredsDigit` (+0x14) | get name delay |
| 309 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:1980` | `singleplayer_vs_preview_fix_` | 0x12D67C | `0x8013433C` | não detectado na janela | `j       singleplayer_vs_preview_fix_` | `decomp/src/mv/mvopening/mvopeningroom.c:1239` `mvOpeningRoomCheckSetFramebuffer` (+0x24) | singleplayer vs preview fix |
| 310 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2003` | `singleplayer_vs_preview_with_allies_fix_` | 0x12D620 | `0x801342E0` | não detectado na janela | `jal     singleplayer_vs_preview_with_allies_fix_` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1546` `mnPlayers1PTrainingDestroyHandicapLevel` (+0x28) | singleplayer vs preview with allies fix |
| 311 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2011` | `singleplayer_vs_preview_with_allies_fix_` | 0x12D580 | `0x80134240` | não detectado na janela | `jal     singleplayer_vs_preview_with_allies_fix_` | `decomp/src/mn/mnvsmode/mnvsresults.c:1330` `mnVSResultsMakeWinnerText` (+0x4) | singleplayer vs preview with allies fix |
| 312 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2054` | `singleplayer_gdk_polygon_team_costume_preview_fix_` | 0x12C738 | `0x801333F8` | não detectado na janela | `j       singleplayer_gdk_polygon_team_costume_preview_fix_` | `decomp/src/mn/mndata/mnvsrecord.c:902` `mnVSRecordMakeStatsGrid` (+0xC) | singleplayer gdk polygon team costume preview fix |
| 313 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2134` | `singleplayer_mm_costume_preview_fix_` | 0x12D174 | `0x80133E34` | não detectado na janela | `j       singleplayer_mm_costume_preview_fix_` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1354` `mnPlayers1PTrainingUpdateFighter` (+0x4) | singleplayer mm costume preview fix |
| 314 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2180` | `singleplayer_polygon_team_costume_fix_` | 0x10BDF4 | `0x8018D594` | não detectado na janela | `j       singleplayer_polygon_team_costume_fix_` | `decomp/src/sc/sccommon/scautodemo.c:369` `scAutoDemoResetFocusPlayerAll` (+0x6C) | singleplayer polygon team costume fix |
| 315 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2242` | `duo_team_costume_preview_fix_` | 0x12D15C | `0x80133E1C` | não detectado na janela | `jal     duo_team_costume_preview_fix_` | `decomp/src/mn/mncommon/mntitle.c:1482` `mnTitleMakeLogoFire` (+0x40) | duo team costume preview fix |
| 316 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2247` | `duo_team_costume_preview_fix_` | 0x12D170 | `0x80133E30` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1354` `mnPlayers1PTrainingUpdateFighter` (exato) | duo team costume preview fix |
| 317 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2252` | `duo_team_costume_preview_fix_` | 0x12D1C8 | `0x80133E88` | não detectado na janela | `jal     duo_team_costume_preview_fix_` | `decomp/src/mn/mnvsmode/mnvsresults.c:1199` `mnVSResultsMakePlayerTagCamera` (+0xC) | duo team costume preview fix |
| 318 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2257` | `duo_team_costume_preview_fix_` | 0x12D1DC | `0x80133E9C` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsresults.c:1199` `mnVSResultsMakePlayerTagCamera` (+0x20) | duo team costume preview fix |
| 319 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2384` | `replace_victory_image_` | 0x17E824 | `0x80131E14` | não detectado na janela | `jal     replace_victory_image_` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:217` `mnPlayers1PTrainingSetPortraitWallpaperPosition` (+0x8) | replace victory image |
| 320 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2391` | `replace_victory_image_` | 0x17E850 | `0x80131E40` | não detectado na janela | `jal     replace_victory_image_` | `decomp/src/mn/mndata/mnvsrecord.c:232` `mnVSRecordSetSpriteColors` (exato) | replace victory image |
| 321 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2398` | `replace_victory_image_` | 0x17E870 | `0x80131E60` | não detectado na janela | `jal     replace_victory_image_._2` | `decomp/src/mv/mvopening/mvopeningportraits.c:155` `mvOpeningPortraitsBlockRow2` (exato) | replace victory image |
| 322 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2406` | `replace_victory_image_` | 0x17E8B4 | `0x80131EA4` | não detectado na janela | `jal     replace_victory_image_._3` | `decomp/src/mv/mvopening/mvopeningportraits.c:161` `mvOpeningPortraitsBlockRow3` (+0x14) | replace victory image |
| 323 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2413` | `replace_victory_image_` | 0x17E8E0 | `0x80131ED0` | não detectado na janela | `jal     replace_victory_image_._3` | `decomp/src/mn/mncommon/mnstartup.c:263` `mnStartupFuncLights` (+0x4) | replace victory image |
| 324 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2420` | `replace_victory_image_` | 0x17E900 | `0x80131EF0` | não detectado na janela | `jal     replace_victory_image_._4` | `decomp/src/mn/mncommon/mnstartup.c:269` `mnStartupStartScene` (exato) | replace victory image |
| 325 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2551` | `env_mapping_fix_` | 0x17558 | `0x80016958` | não detectado na janela | `j       env_mapping_fix_` | `decomp/src/sys/objdisplay.c:3263` `func_8001663C` (+0x31C) | env mapping fix |
| 326 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2635` | `fix_port_index_` | 0x12C08C | `0x80132D4C` | não detectado na janela | `jal     fix_port_index_` | `decomp/src/mv/mvopening/mvopeningroom.c:627` `mvOpeningRoomMakeCloseUpOverlayCamera` (+0x4) | fix port index |
| 327 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2640` | `fix_port_index_` | 0x12C764 | `0x80133424` | não detectado na janela | `jal     fix_port_index_._opponent` | `decomp/src/sc/sccommon/scstaffroll.c:1353` `scStaffrollGetPauseStatusHighlight` (+0x8) | fix port index |
| 328 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2645` | `fix_port_index_` | 0x178BBC | `0x8013215C` | não detectado na janela | `jal     fix_port_index_._gameover_and_room_drop` | `decomp/src/mv/mvopening/mvopeningsector.c:301` `mvOpeningSectorCockpitProcUpdate` (exato) | fix port index |
| 329 | `.toolchain/pkgs/smashremix-src/src/SinglePlayer.asm:2650` | `fix_port_index_` | 0x177944 | `0x80131FA4` | não detectado na janela | `jal     fix_port_index_._gameover_and_room_drop` | `decomp/src/mn/mnmaps/mnmaps.c:659` `mnMapsMakeWallpaper` (exato) | fix port index |

### SinglePlayerEnemy.asm

Controle dos inimigos no 1P: AFK, botões, respawn e escudo.

Hooks de código: 10; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 330 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:28` | `onep_control_port` | 0x10E350 | `0x8018FAF0` | não detectado na janela | `j        onep_control_port` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1733` `func_ovl7_8018FA54` (+0x9C) | onep control port |
| 331 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:78` | `_afk_1p` | 0x17DC68 | `0x80134A18` | não detectado na janela | `j       _handle_stage_clear` | `decomp/src/sc/sc1pmode/sc1pintro.c:2127` `sc1PIntroSetupFighterFiles` (+0x20) | afk 1p |
| 332 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:82` | `_afk_1p` | 0x12DC6C | `0x8013492C` | não detectado na janela | `j       _handle_preview` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1705` `mnPlayers1PBonusMakePortraitFlash` (+0x10) | afk 1p |
| 333 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:86` | `_afk_1p` | 0x17A588 | `0x80133B28` | não detectado na janela | `j       _handle_continue` | `decomp/src/mn/mnplayers/mnplayersvs.c:1200` `mnPlayersVSGetNumberDigitCount` (+0x24) | afk 1p |
| 334 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:90` | `_afk_1p` | 0x1129E8 | `0x8018E2A8` | não detectado na janela | `j       _handle_bonus` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:1022` `sc1PBonusStageTimeUpProcUpdate` (exato) | afk 1p |
| 335 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:94` | `_afk_1p` | 0x86F4C | `0x8010B74C` | não detectado na janela | `j       _handle_rttf` | `decomp/src/gr/grbonus/grbonus3.c:99` `grBonus3FinishProcUpdate` (+0x4C) | afk 1p |
| 336 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:203` | `onep_control_respawn` | 0x10CD88 | `0x8018E528` | não detectado na janela | `j        onep_control_respawn` | `decomp/src/sc/sccommon/scexplain.c:839` `scExplainFuncLights` (+0xC) | onep control respawn |
| 337 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:235` | `onep_control_shield` | 0x7C964 | `0x80101164` | não detectado na janela | `j        onep_control_shield` | `decomp/src/ef/efmanager.c:4205` `efManagerShieldMakeEffect` (+0x5C) | onep control shield |
| 338 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:442` | `z_button` | 0x14029C | `0x8013809C` | não detectado na janela | `j        z_button` | `decomp/src/mn/mnplayers/mnplayersvs.c:3567` `mnPlayersVSRecallPuck` (+0xA4) | z button |
| 339 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm:490` | `b_button` | 0x13EF28 | `0x80136D28` | não detectado na janela | `j        b_button` | `decomp/src/mn/mnvsmode/mnvsresults.c:2703` `mnVSResultsSetRoyalPlace` (exato) | b button |

### SinglePlayerModes.asm

Modos 1P: estágios, final All-Star, crash na morte e tela de fim de partida.

Hooks de código: 127; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 340 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:131` | `bonus2_css_2_1p` | 0x14CAF8 | `0x80136AC8` | não detectado na janela | `j       bonus2_css_2_1p` | `decomp/src/mn/mnvsmode/mnvsresults.c:2630` `mnVSResultsSetKOs` (+0x2C) | bonus2 css 2 1p |
| 341 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:278` | `opponent_type` | 0x10C0C4 | `0x8018D864` | não detectado na janela | `j       opponent_type` | `decomp/src/mv/mvopening/mvopeningpikachu.c:286` `mvOpeningPikachuPosedWallpaperProcDisplay` (exato) | opponent type |
| 342 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:342` | `battle_style` | 0x10E18C | `0x8018F92C` | não detectado na janela | `j        battle_style` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1679` `func_ovl7_8018F8FC` (+0x30) | battle style |
| 343 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:376` | `_1p_stage` | 0x10BEC4 | `0x8018D664` | não detectado na janela | `j        _1p_stage` | `decomp/src/mv/mvopening/mvopeningpikachu.c:227` `mvOpeningPikachuMakeMotionWindow` (+0x30) | 1p stage |
| 344 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:431` | `_team_attack` | 0x10BEF0 | `0x8018D690` | não detectado na janela | `j        _team_attack` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:502` `sc1PTrainingModeUpdateSpeedOption` (+0xC) | team attack |
| 345 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:472` | `_item_spawn` | 0x10BF64 | `0x8018D704` | não detectado na janela | `j        _item_spawn` | `decomp/src/sc/sccommon/scautodemo.c:442` `scAutoDemoMakeFocusInterface` (exato) | item spawn |
| 346 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:501` | `_hmn_knockback` | 0x5209C | `0x800D689C` | não detectado na janela | `j        _hmn_knockback` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0xC0) | hmn knockback |
| 347 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:523` | `_opponent_knockback` | 0x10BDAC | `0x8018D54C` | não detectado na janela | `j        _opponent_knockback` | `decomp/src/sc/sccommon/scautodemo.c:369` `scAutoDemoResetFocusPlayerAll` (+0x24) | opponent knockback |
| 348 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:560` | `_opponent_number_1` | 0x10BF74 | `0x8018D714` | não detectado na janela | `j        _opponent_number_1` | `decomp/src/sc/sccommon/scautodemo.c:442` `scAutoDemoMakeFocusInterface` (+0x10) | opponent number 1 |
| 349 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:598` | `_opponent_number_2` | 0x10C234 | `0x8018D9D4` | não detectado na janela | `j        _opponent_number_2` | `decomp/src/mv/mvopening/mvopeningyoshi.c:325` `mvOpeningYoshiPosedFighterProcUpdate` (+0x14) | opponent number 2 |
| 350 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:638` | `_opponent_2` | 0x10BDC0 | `0x8018D560` | não detectado na janela | `j        _opponent_2` | `decomp/src/sc/sccommon/scautodemo.c:369` `scAutoDemoResetFocusPlayerAll` (+0x38) | opponent 2 |
| 351 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:672` | `_time_limit` | 0x10BF04 | `0x8018D6A4` | não detectado na janela | `j        _time_limit` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:502` `sc1PTrainingModeUpdateSpeedOption` (+0x20) | time limit |
| 352 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:735` | `_percent_fix` | 0x10D930 | `0x8018F0D0` | não detectado na janela | `j        _percent_fix` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1373` `sc1PTrainingModeUpdateItemOptionSprite` (+0x90) | percent fix |
| 353 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:765` | `_spawning_style` | 0x10D360 | `0x8018EB00` | não detectado na janela | `j       _spawning_style` | `decomp/src/sc/sc1pmode/sc1pgame.c:1852` `sc1PGameWaitThreadUpdate` (+0x24) | spawning style |
| 354 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:846` | `spawn_human_left_remix_rttf_` | 0x10E388 | `0x8018FB28` | não detectado na janela | `jal     spawn_human_left_remix_rttf_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1733` `func_ovl7_8018FA54` (+0xD4) | spawn human left remix rttf |
| 355 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:865` | `_initial_screen_set` | 0x12DC90 | `0x80134950` | não detectado na janela | `j       _initial_screen_set` | `decomp/src/mn/mnplayers/mnplayersvs.c:1828` `mnPlayersVSFighterProcUpdate` (+0x4) | initial screen set |
| 356 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:892` | `_reset_button` | 0x8F668 | `0x80113E68` | não detectado na janela | `j       _reset_button` | `decomp/src/if/ifcommon.c:3030` `ifCommonBattlePauseMakeSObjsAll` (+0x64) | reset button |
| 357 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:931` | `_reset` | 0x8FD28 | `0x80114528` | não detectado na janela | `j        _reset` | `decomp/src/sys/objdisplay.c:2172` `gcDrawDObjDLLinksForGObj` (+0x12C) | reset |
| 358 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:990` | `_load_timer` | 0x10E558 | `0x8018FCF8` | não detectado na janela | `j       _load_timer` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1835` `sc1PTrainingModeUnderlineProcDisplay` (+0x18) | load timer |
| 359 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1173` | `_end_match_screen` | 0x523D4 | `0x800D6BD4` | não detectado na janela | `j       _end_match_screen` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0x3F8) | end match screen |
| 360 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1211` | `_ko_save` | 0x10D8A4 | `0x8018F044` | não detectado na janela | `j        _ko_save` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1373` `sc1PTrainingModeUpdateItemOptionSprite` (+0x4) | ko save |
| 361 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1301` | `_time_save` | 0x90564 | `0x80114D64` | não detectado na janela | `j        _time_save` | `decomp/src/if/ifcommon.c:3574` `ifCommonAnnounceCompleteInitInterface` (+0xC) | time save |
| 362 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1399` | `_new_record` | 0x8F038 | `0x80113838` | não detectado na janela | `j        _new_record` | `decomp/src/if/ifcommon.c:2881` `ifCommonBattleEndAddSoundQueueID` (+0x34) | new record |
| 363 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1443` | `_opponent_behavior` | 0x10BE54 | `0x8018D5F4` | não detectado na janela | `j        _opponent_behavior` | `decomp/src/sc/sccommon/scautodemo.c:394` `scAutoDemoExit` (+0x4) | opponent behavior |
| 364 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1490` | `_infinite_spawn_1` | 0x10CBEC | `0x8018E38C` | não detectado na janela | `j        _infinite_spawn_1` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:1057` `sc1PBonusStageMakeTimer` (+0x48) | infinite spawn 1 |
| 365 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1519` | `_death_crash` | 0x10D8B4 | `0x8018F054` | não detectado na janela | `j        _death_crash` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1373` `sc1PTrainingModeUpdateItemOptionSprite` (+0x14) | death crash |
| 366 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1548` | `_infinite_spawn_2` | 0x10CA2C | `0x8018E1CC` | não detectado na janela | `j        _infinite_spawn_2` | `decomp/src/sc/sccommon/scexplain.c:724` `scExplainFuncStart` (+0x2C) | infinite spawn 2 |
| 367 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1576` | `_infinite_ko` | 0x10D910 | `0x8018F0B0` | não detectado na janela | `j        _infinite_ko` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1373` `sc1PTrainingModeUpdateItemOptionSprite` (+0x70) | infinite ko |
| 368 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1606` | `_preview_skip` | 0x12DC64 | `0x80134924` | não detectado na janela | `j        _preview_skip` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1705` `mnPlayers1PBonusMakePortraitFlash` (+0x8) | preview skip |
| 369 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1655` | `_preview_song_skip` | 0x12E0B0 | `0x80134D70` | não detectado na janela | `j        _preview_song_skip` | `decomp/src/mn/mnplayers/mnplayersvs.c:1979` `mnPlayersVSUpdateCursor` (+0x1C) | preview song skip |
| 370 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1694` | `skip_cflag_music_function` | 0x10E514 | `0x8018FCB4` | não detectado na janela | `j        skip_cflag_music_function` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1811` `sc1PTrainingModeSetVScrollOptionSObjs` (+0x38) | skip cflag music function |
| 371 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1731` | `set_bgm` | 0x10E548 | `0x8018FCE8` | não detectado na janela | `j        set_bgm` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1835` `sc1PTrainingModeUnderlineProcDisplay` (+0x8) | set bgm |
| 372 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1877` | `skip_cflag_music_function_2` | 0x10B9C0 | `0x8018D160` | não detectado na janela | `j       skip_cflag_music_function_2` | `decomp/src/sc/sc1pmode/sc1pgame.c:806` `sc1PGameSetGameStart` (exato) | skip cflag music function 2 |
| 373 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1914` | `load_player_stocks` | 0x520C4 | `0x800D68C4` | não detectado na janela | `j        load_player_stocks` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0xE8) | load player stocks |
| 374 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:1960` | `load_1p_difficulty_1` | 0x10BD54 | `0x8018D4F4` | não detectado na janela | `j        load_1p_difficulty_1` | `decomp/src/sc/sccommon/scautodemo.c:360` `SCAutoDemoProcFocusPlayer2` (+0x4) | load 1p difficulty 1 |
| 375 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2001` | `load_1p_difficulty_2` | 0x10BD9C | `0x8018D53C` | não detectado na janela | `j        load_1p_difficulty_2` | `decomp/src/sc/sccommon/scautodemo.c:369` `scAutoDemoResetFocusPlayerAll` (+0x14) | load 1p difficulty 2 |
| 376 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2033` | `_opponent_cpu` | 0x10BD64 | `0x8018D504` | não detectado na janela | `j        _opponent_cpu` | `decomp/src/sc/sccommon/scexplain.c:296` `scExplainMakeControlStickCamera` (+0x4) | opponent cpu |
| 377 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2087` | `crash_prevent` | 0x10D938 | `0x8018F0D8` | não detectado na janela | `j       crash_prevent` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1373` `sc1PTrainingModeUpdateItemOptionSprite` (+0x98) | crash prevent |
| 378 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2123` | `stage_flag` | 0x52324 | `0x800D6B24` | não detectado na janela | `j       stage_flag` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0x348) | stage flag |
| 379 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2156` | `icon_removal` | 0x10D4D0 | `0x8018EC70` | não detectado na janela | `j       icon_removal` | `decomp/src/sc/sc1pmode/sc1pgame.c:1908` `sc1PGameTeamStockDisplayProcDisplay` (+0xB8) | icon removal |
| 380 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2187` | `multiman_random` | 0x10CB6C | `0x8018E30C` | não detectado na janela | `j       multiman_random` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:959` `sc1PTrainingModeInitSpriteEnvColors` (+0xC) | multiman random |
| 381 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2225` | `multiman_ko_count_stop` | 0x10D97C | `0x8018F11C` | não detectado na janela | `j       multiman_ko_count_stop` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1388` `sc1PTrainingModeInitItemOptionSpriteColors` (+0x20) | multiman ko count stop |
| 382 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2273` | `player_ko_refresh` | 0x10C914 | `0x8018E0B4` | não detectado na janela | `j       player_ko_refresh` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:956` `sc1PBonusStageMakeTaskSprites` (+0x1C) | player ko refresh |
| 383 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2405` | `master_hand_` | 0x61B8C | `0x800E638C` | não detectado na janela | `j       master_hand_` | `decomp/src/ft/ftmain.c:3961` `ftMainProcParams` (+0x1A0) | master hand |
| 384 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2443` | `master_hand_music_fix_` | 0x10D31C | `0x8018EABC` | não detectado na janela | `j       master_hand_music_fix_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1182` `sc1PTrainingModeMakeItemDisplay` (+0x34) | master hand music fix |
| 385 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2468` | `giga_fix_` | 0xD3170 | `0x80158730` | não detectado na janela | `j       giga_fix_` | `decomp/src/ft/ftchar/ftboss/ftbosscommon.c:218` `ftBossCommonUpdateDamageStats` (+0x90) | giga fix |
| 386 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2493` | `giga_stamina_check_` | 0xD310C | `0x801586CC` | não detectado na janela | `j       giga_stamina_check_` | `decomp/src/ft/ftchar/ftboss/ftbosscommon.c:218` `ftBossCommonUpdateDamageStats` (+0x2C) | giga stamina check |
| 387 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2530` | `master_hand_HP_display_` | 0x8A20C | `0x8010EA0C` | não detectado na janela | `j           master_hand_HP_display_` | `decomp/src/if/ifcommon.c:594` `ifCommonPlayerDamageUpdateDigits` (+0x118) | master hand HP display |
| 388 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2565` | `giga_change_action_` | 0xD3150 | `0x80158710` | não detectado na janela | `j       giga_change_action_` | `decomp/src/ft/ftchar/ftboss/ftbosscommon.c:218` `ftBossCommonUpdateDamageStats` (+0x70) | giga change action |
| 389 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2626` | `giga_change_action_2_` | 0xD3140 | `0x80158700` | não detectado na janela | `j       giga_change_action_2_` | `decomp/src/ft/ftchar/ftboss/ftbosscommon.c:218` `ftBossCommonUpdateDamageStats` (+0x60) | giga change action 2 |
| 390 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:2688` | `no_laugh_` | 0x10DF08 | `0x8018F6A8` | não detectado na janela | `j       no_laugh_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1584` `sc1PTrainingModeUpdateOptionArrows` (+0x78) | no laugh |
| 391 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:3581` | `remix_1p_randomization` | 0x1402E8 | `0x801380E8` | não detectado na janela | `j       remix_1p_randomization` | `decomp/src/mn/mnplayers/mnplayersvs.c:3567` `mnPlayersVSRecallPuck` (+0xF0) | remix 1p randomization |
| 392 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4036` | `polygon_selection_match` | 0x10C510 | `0x8018DCB0` | não detectado na janela | `j       polygon_selection_match` | `decomp/src/sc/sccommon/scexplain.c:530` `scExplainSetPhaseSObjs` (+0x2C) | polygon selection match |
| 393 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4084` | `polygon_selection_match_rttf` | 0x10C794 | `0x8018DF34` | não detectado na janela | `j       polygon_selection_match_rttf` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:854` `sc1PTrainingModeMakeStatDisplayText` (+0x4) | polygon selection match rttf |
| 394 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4134` | `polygon_file_loading` | 0x10E21C | `0x8018F9BC` | não detectado na janela | `j       polygon_file_loading` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1696` `func_ovl7_8018F984` (+0x38) | polygon file loading |
| 395 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4214` | `polygon_allocation` | 0x10E234 | `0x8018F9D4` | não detectado na janela | `j       polygon_allocation` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1696` `func_ovl7_8018F984` (+0x50) | polygon allocation |
| 396 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4299` | `title_card_text_name` | 0x12B834 | `0x801324F4` | não detectado na janela | `j       title_card_text_name` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:481` `mnPlayers1PBonusMakePortraitShadow` (+0x4) | title card text name |
| 397 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4322` | `title_card_text_routine` | 0x12B904 | `0x801325C4` | não detectado na janela | `j       title_card_text_routine` | `decomp/src/mn/mnvsmode/mnvsitemswitch.c:626` `mnVSItemSwitchMakeDecalCamera` (+0x14) | title card text routine |
| 398 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4487` | `title_card_text_position` | 0x12BC54 | `0x80132914` | não detectado na janela | `j       title_card_text_position` | `decomp/src/mn/mn1pmode/mn1pmode.c:687` `mn1PModeInitVars` (+0x4) | title card text position |
| 399 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4507` | `title_card_text_location_routine` | 0x12BE18 | `0x80132AD8` | não detectado na janela | `j       title_card_text_location_routine` | `decomp/src/mn/mnvsmode/mnvsoptions.c:730` `mnVSOptionsMakeHandicapOption` (+0x10) | title card text location routine |
| 400 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4575` | `title_card_giant_announce` | 0x12DAC0 | `0x80134780` | não detectado na janela | `j       title_card_giant_announce` | `decomp/src/mn/mnvsmode/mnvsresults.c:1608` `mnVSResultsSetNumberColor` (+0x10) | title card giant announce |
| 401 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4689` | `title_card_yoshi_team_skip` | 0x12DAAC | `0x8013476C` | não detectado na janela | `j       title_card_yoshi_team_skip` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1600` `mnPlayers1PBonusUpdateFighter` (exato) | title card yoshi team skip |
| 402 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4723` | `title_card_flag_refresh` | 0x12DE60 | `0x80134B20` | não detectado na janela | `j       title_card_flag_refresh` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1770` `func_ovl29_80134B1C` (+0x4) | title card flag refresh |
| 403 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4763` | `title_card_voice_name` | 0x12D950 | `0x80134610` | não detectado na janela | `j       title_card_voice_name` | `decomp/src/mn/mndata/mnvsrecord.c:1395` `mnVSRecordMakeBattleScoreTableHeaders` (exato) | title card voice name |
| 404 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4787` | `title_card_model_scale` | 0x12CAB8 | `0x80133778` | não detectado na janela | `j       title_card_model_scale` | `decomp/src/mn/mncommon/mntitle.c:1349` `mnTitleFireCameraProcUpdate` (+0x8) | title card model scale |
| 405 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4810` | `title_card_1` | 0x12CE1C | `0x80133ADC` | não detectado na janela | `j       title_card_1` | `decomp/src/sc/sc1pmode/sc1pintro.c:1449` `sc1PIntroInitVSFighters` (+0x14) | title card 1 |
| 406 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4833` | `title_card_2` | 0x12DD4C | `0x80134A0C` | não detectado na janela | `j       title_card_2` | `decomp/src/sc/sc1pmode/sc1pintro.c:2127` `sc1PIntroSetupFighterFiles` (+0x14) | title card 2 |
| 407 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4856` | `remix_1p_pointer_part1` | 0x10BE90 | `0x8018D630` | não detectado na janela | `j       remix_1p_pointer_part1` | `decomp/src/mv/mvopening/mvopeningkirby.c:235` `mvOpeningKirbyMakeMotionWindow` (+0x4) | remix 1p pointer part1 |
| 408 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4887` | `remix_1p_pointer_part2` | 0x10BEE0 | `0x8018D680` | não detectado na janela | `j       remix_1p_pointer_part2` | `decomp/src/sc/sccommon/scautodemo.c:421` `scAutoDemoUpdateFocus` (+0xC) | remix 1p pointer part2 |
| 409 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4917` | `remix_icon_pointer` | 0x12B518 | `0x801321D8` | não detectado na janela | `j       remix_icon_pointer` | `decomp/src/mv/mvopening/mvopeningstandoff.c:365` `mvOpeningStandoffLightningFlashProcDisplay` (exato) | remix icon pointer |
| 410 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:4954` | `metal_bros_magic` | 0x12DB58 | `0x80134818` | não detectado na janela | `j       metal_bros_magic` | `decomp/src/sc/sc1pmode/sc1pintro.c:2056` `sc1PIntroInitVars` (+0x8) | metal bros magic |
| 411 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5007` | `metal_bros_bug_fix` | 0x12E050 | `0x80134D10` | não detectado na janela | `j       metal_bros_bug_fix` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1649` `mnPlayers1PGameMakeTotalBonusCount` (+0x8) | metal bros bug fix |
| 412 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5045` | `metal_bros_text` | 0x12BE58 | `0x80132B18` | não detectado na janela | `j       metal_bros_text` | `decomp/src/sc/sc1pmode/sc1pintro.c:849` `sc1PIntroMakeNameAll` (+0x8) | metal bros text |
| 413 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5078` | `metal_bros_stage_text` | 0x12E028 | `0x80134CE8` | não detectado na janela | `j       metal_bros_stage_text` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1842` `mnPlayers1PTrainingSetCursorGrab` (+0x20) | metal bros stage text |
| 414 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5115` | `remix_1p_giant` | 0x10C2A8 | `0x8018DA48` | não detectado na janela | `j       remix_1p_giant` | `decomp/src/mv/mvopening/mvopeninglink.c:347` `mvOpeningLinkMakePosedFighter` (+0x8) | remix 1p giant |
| 415 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5304` | `remix_yoshi_card` | 0x12CEB8 | `0x80133B78` | não detectado na janela | `j       remix_yoshi_card` | `decomp/src/mn/mnmaps/mnmaps.c:2370` `mnMapsSaveSceneData` (exato) | remix yoshi card |
| 416 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5327` | `remix_yoshi_card_shade_skip` | 0x12CF14 | `0x80133BD4` | não detectado na janela | `j       remix_yoshi_card_shade_skip` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1197` `mnPlayers1PBonusMakeHiScore` (+0x8) | remix yoshi card shade skip |
| 417 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5353` | `remix_yoshi_card_costumes` | 0x12CF3C | `0x80133BFC` | não detectado na janela | `j       remix_yoshi_card_costumes` | `decomp/src/sc/sc1pmode/sc1pstageclear.c:1633` `sc1PStageClearMakeWallpaperCamera` (+0x14) | remix yoshi card costumes |
| 418 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5401` | `remix_team_shade_start` | 0x10C440 | `0x8018DBE0` | não detectado na janela | `j       remix_team_shade_start` | `decomp/src/mv/mvopening/mvopeninglink.c:394` `mvOpeningLinkMakePosedFighterCamera` (exato) | remix team shade start |
| 419 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5447` | `remix_team_shade_ko` | 0x10CAD8 | `0x8018E278` | não detectado na janela | `j       remix_team_shade_ko` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:933` `sc1PTrainingModeInitStatDisplayCharacterSprites` (+0x5C) | remix team shade ko |
| 420 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5492` | `tiny_team_screen_card` | 0x12DF64 | `0x80134C24` | não detectado na janela | `jal     tiny_team_screen_card._set_num_chars` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1805` `mnPlayers1PBonusUpdateCursorPlacementPriorities` (+0x8) | tiny team screen card |
| 421 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5497` | `tiny_team_screen_card` | 0x12D04C | `0x80133D0C` | não detectado na janela | `j       tiny_team_screen_card._load` | `decomp/src/mn/mncommon/mntitle.c:1468` `mnTitleLogoFireProcDisplay` (+0x10) | tiny team screen card |
| 422 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5644` | `doubles_card_1` | 0x12D148 | `0x80133E08` | não detectado na janela | `j       doubles_card_1` | `decomp/src/mn/mncommon/mntitle.c:1482` `mnTitleMakeLogoFire` (+0x2C) | doubles card 1 |
| 423 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5674` | `doubles_card_2` | 0x12D1B4 | `0x80133E74` | não detectado na janela | `j       doubles_card_2` | `decomp/src/mn/mncommon/mntitle.c:1493` `mnTitleMakeLogoFireParticles` (+0xC) | doubles card 2 |
| 424 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5707` | `starfox_costume` | 0x521D4 | `0x800D69D4` | não detectado na janela | `j       starfox_costume` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0x1F8) | starfox costume |
| 425 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5736` | `doubles_file_loading` | 0x12DE00 | `0x80134AC0` | não detectado na janela | `j       doubles_file_loading` | `decomp/src/mn/mndata/mnvsrecord.c:1508` `mnVSRecordMakeRankingTableValues` (+0x18) | doubles file loading |
| 426 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5772` | `giant_file_loading` | 0x12DDE8 | `0x80134AA8` | não detectado na janela | `j       giant_file_loading` | `decomp/src/mn/mndata/mnvsrecord.c:1508` `mnVSRecordMakeRankingTableValues` (exato) | giant file loading |
| 427 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5801` | `giant_card` | 0x12D114 | `0x80133DD4` | não detectado na janela | `j       giant_card` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1219` `mnPlayers1PGameMakeLevel` (+0x38) | giant card |
| 428 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5824` | `remix_kirby_card` | 0x12C3CC | `0x8013308C` | não detectado na janela | `j       remix_kirby_card` | `decomp/src/sc/sc1pmode/sc1pintro.c:1069` `sc1PIntroSetKirbyTeamModelPartIDs` (+0xC) | remix kirby card |
| 429 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5848` | `remix_kirby_selection` | 0x10CBD0 | `0x8018E370` | não detectado na janela | `j       remix_kirby_selection` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:1057` `sc1PBonusStageMakeTimer` (+0x2C) | remix kirby selection |
| 430 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5873` | `remix_kirby_skip` | 0x10CBA4 | `0x8018E344` | não detectado na janela | `j       remix_kirby_skip` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:1057` `sc1PBonusStageMakeTimer` (exato) | remix kirby skip |
| 431 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5899` | `remix_kirby_selection_initial` | 0x10C6F4 | `0x8018DE94` | não detectado na janela | `j       remix_kirby_selection_initial` | `decomp/src/mv/mvopening/mvopeningyoshi.c:503` `mvOpeningYoshiFuncStart` (+0xC) | remix kirby selection initial |
| 432 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5921` | `remix_1p_kirby_load` | 0x10E1D0 | `0x8018F970` | não detectado na janela | `j       remix_1p_kirby_load` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1679` `func_ovl7_8018F8FC` (+0x74) | remix 1p kirby load |
| 433 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:5942` | `_final_stage_spawn` | 0x10C08C | `0x8018D82C` | não detectado na janela | `j        _final_stage_spawn` | `decomp/src/mv/mvopening/mvopeninglink.c:284` `mvOpeningLinkPosedWallpaperProcDisplay` (+0x8) | final stage spawn |
| 434 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:6022` | `_giga_usp_prevent` | 0xCBBC8 | `0x80151188` | não detectado na janela | `j       _giga_usp_prevent` | `decomp/src/ft/ftcommon/ftcommonspecialhi.c:51` `ftCommonSpecialHiCheckInterruptCommon` (+0x28) | giga usp prevent |
| 435 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:6081` | `game_over_end` | 0x17A3F0 | `0x80133990` | não detectado na janela | `j       game_over_end` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1123` `mnPlayers1PBonusGetBestTaskCount` (exato) | game over end |
| 436 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:6955` | `player_percent` | 0x5320C | `0x800D7A0C` | não detectado na janela | `j       player_percent` | `decomp/src/ft/ftmanager.c:477` `ftManagerInitFighter` (+0x1C) | player percent |
| 437 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7003` | `clear_percent` | 0xB6990 | `0x8013BF50` | não detectado na janela | `j       clear_percent` | `decomp/src/ft/ftcommon/ftcommondead.c:48` `ftCommonDeadUpdateScore` (+0x1EC) | clear percent |
| 438 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7030` | `clear_percent_continue` | 0x17A6A8 | `0x80133C48` | não detectado na janela | `j       clear_percent_continue` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1213` `mnPlayers1PBonusMakeTotalTime` (+0x2C) | clear percent continue |
| 439 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7081` | `rest_area_and_hrc_spawning` | 0x10BE4C | `0x8018D5EC` | não detectado na janela | `j       rest_area_and_hrc_spawning` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:604` `sc1PBonusStageBonus2LoadFile` (+0x4) | rest area and hrc spawning |
| 440 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7111` | `rest_area_and_hrc_spawning_fix` | 0x10CE20 | `0x8018e5c0` | não detectado na janela | `j       rest_area_and_hrc_spawning_fix` | `decomp/src/sc/sccommon/scexplain.c:861` `scExplainStartScene` (exato) | rest area and hrc spawning fix |
| 441 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7154` | `instant_spawn` | 0x10C084 | `0x8018D824` | não detectado na janela | `j       instant_spawn` | `decomp/src/mv/mvopening/mvopeninglink.c:284` `mvOpeningLinkPosedWallpaperProcDisplay` (exato) | instant spawn |
| 442 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7186` | `limbo_clear` | 0x5324C | `0x800D7A4C` | não detectado na janela | `j       limbo_clear` | `decomp/src/ft/ftmanager.c:477` `ftManagerInitFighter` (+0x5C) | limbo clear |
| 443 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7221` | `skip_go` | 0x10D1C4 | `0x8018E964` | não detectado na janela | `j       skip_go` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:1253` `sc1PBonusStageWriteBackup` (+0x8) | skip go |
| 444 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7815` | `heart_persist` | 0xE9FFC | `0x8016F5BC` | não detectado na janela | `j       heart_persist` | `decomp/src/it/itprocess.c:92` `itProcessProcItemMain` (+0x88) | heart persist |
| 445 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7850` | `progress_advancement` | 0x52530 | `0x800D6D30` | não detectado na janela | `j       progress_advancement` | unmatched | progress advancement |
| 446 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7891` | `screen_set_end` | 0x523E8 | `0x800D6BE8` | não detectado na janela | `j       screen_set_end` | unmatched | screen set end |
| 447 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7918` | `heart_counter` | 0xC06E4 | `0x80145CA4` | não detectado na janela | `j       heart_counter` | `decomp/src/ft/ftcommon/ftcommonget.c:80` `ftCommonLightGetProcDamage` (+0xC0) | heart counter |
| 448 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7950` | `advance_allstar` | 0x17DF18 | `0x80134CC8` | não detectado na janela | `j       advance_allstar` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1842` `mnPlayers1PTrainingSetCursorGrab` (exato) | advance allstar |
| 449 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:7989` | `pause_exit` | 0x8FAC4 | `0x801142C4` | não detectado na janela | `j       pause_exit` | `decomp/src/if/ifcommon.c:3201` `ifCommonBattleInterfaceProcSet` (+0x10) | pause exit |
| 450 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8030` | `display_percent` | 0x10D158 | `0x8018E8f8` | não detectado na janela | `j       display_percent` | `decomp/src/sc/sc1pmode/sc1pgame.c:1744` `sc1PGameWaitStageBonus3Update` (exato) | display percent |
| 451 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8058` | `record_percent` | 0x10EC20 | `0x801903C0` | não detectado na janela | `j       record_percent` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1958` `sc1PTrainingModeFuncStart` (+0x160) | record percent |
| 452 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8085` | `end_allstar` | 0x10F768 | `0x80190F08` | não detectado na janela | `j       end_allstar` | `decomp/src/sc/sc1pmode/sc1pgame.c:3262` `sc1PGameInitBonusStats` (+0xB0) | end allstar |
| 453 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8110` | `prevent_erase` | 0x520CC | `0x800D68CC` | não detectado na janela | `j       prevent_erase` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0xF0) | prevent erase |
| 454 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8129` | `stock_prevent` | 0x520d8 | `0x800D68D8` | não detectado na janela | `j       stock_prevent` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0xFC) | stock prevent |
| 455 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8154` | `damage_prevent` | 0x520E8 | `0x800D68E8` | não detectado na janela | `j       damage_prevent` | `decomp/src/sc/sc1pmode/sc1pmanager.c:290` `sc1PManagerUpdateScene` (+0x10C) | damage prevent |
| 456 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8180` | `costume_fix` | 0x10C350 | `0x8018DAf0` | não detectado na janela | `j       costume_fix` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:779` `sc1PBonusStageMakeBonus2Ground` (+0x10) | costume fix |
| 457 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8217` | `no_miss` | 0x10EC88 | `0x80190428` | não detectado na janela | `j       no_miss` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1958` `sc1PTrainingModeFuncStart` (+0x1C8) | no miss |
| 458 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8246` | `no_miss_2` | 0x10ECB4 | `0x80190454` | não detectado na janela | `j       no_miss_2` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1958` `sc1PTrainingModeFuncStart` (+0x1F4) | no miss 2 |
| 459 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8274` | `no_calamity` | 0x10F448 | `0x80190BE8` | não detectado na janela | `j       no_calamity` | unmatched | no calamity |
| 460 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8454` | `_SC1PGameEnemyTeamSprites_expansion` | 0x10D754 | `0x8018EEF4` | não detectado na janela | `j       _SC1PGameEnemyTeamSprites_expansion` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1328` `sc1PTrainingModeUpdateCPOptionSprite` (+0xC) | SC1PGameEnemyTeamSprites expansion |
| 461 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8517` | `_stock_loading_allstar` | 0x10D534 | `0x8018ECD4` | não detectado na janela | `j       _stock_loading_allstar` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1256` `sc1PTrainingModeInitMenuOptionSpriteAttrs` (+0x30) | stock loading allstar |
| 462 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8569` | `_SC1PGameEnemyTeamSprites_allstar_yoshi_path` | 0x10D6C0 | `0x8018EE60` | não detectado na janela | `j       _SC1PGameEnemyTeamSprites_allstar_yoshi_path` | `decomp/src/it/itground/ittarget.c:45` `itTargetMakeItem` (+0x4) | SC1PGameEnemyTeamSprites allstar yoshi path |
| 463 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8601` | `_allstar_final_costume_address_fix` | 0x10D598 | `0x8018ED38` | não detectado na janela | `j       _allstar_final_costume_address_fix` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1282` `sc1PTrainingModeMenuProcDisplay` (+0xC) | allstar final costume address fix |
| 464 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8630` | `_allstar_final_character_address_fix` | 0x10CB64 | `0x8018E304` | não detectado na janela | `j       _allstar_final_character_address_fix` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:959` `sc1PTrainingModeInitSpriteEnvColors` (+0x4) | allstar final character address fix |
| 465 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8663` | `_allstar_final_costume_set` | 0x10CB74 | `0x8018E314` | não detectado na janela | `j       _allstar_final_costume_set` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:967` `sc1PTrainingModeMakeDamageDisplay` (+0x4) | allstar final costume set |
| 466 | `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm:8714` | `_allstar_final_series_stock_set` | 0x10CD98 | `0x8018E538` | não detectado na janela | `j       _allstar_final_series_stock_set` | `decomp/src/sc/sccommon/scexplain.c:839` `scExplainFuncLights` (+0x1C) | allstar final series stock set |

### Size.asm

Modo tamanho: câmera (centro e zoom), ECB, escala do gfx, altura de pulo e posição do fogo PK.

Hooks de código: 73; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 467 | `.toolchain/pkgs/smashremix-src/src/Size.asm:62` | `adjust_respawn_platform` | 0x7F878 | `0x80104078` | não detectado na janela | `j       adjust_respawn_platform` | `decomp/src/ef/efmanager.c:6120` `efManagerRebirthHaloMakeEffect` (+0x10) | adjust respawn platform |
| 468 | `.toolchain/pkgs/smashremix-src/src/Size.asm:88` | `adjust_player_label_` | 0x8D304 | `0x80111B04` | não detectado na janela | `jal     adjust_player_label_` | `decomp/src/if/ifcommon.c:1999` `ifCommonPlayerTagProcDisplay` (+0xC8) | adjust player label |
| 469 | `.toolchain/pkgs/smashremix-src/src/Size.asm:107` | `adjust_player_shadow_` | 0xB5B84 | `0x8013B144` | não detectado na janela | `jal     adjust_player_shadow_` | `decomp/src/mn/mnplayers/mnplayersvs.c:5438` `mnPlayersVSFuncStart` (+0x7C) | adjust player shadow |
| 470 | `.toolchain/pkgs/smashremix-src/src/Size.asm:129` | `passive_armor_` | 0x61B24 | `0x800E6324` | não detectado na janela | `jal     passive_armor_` | `decomp/src/ft/ftmain.c:3961` `ftMainProcParams` (+0x138) | passive armor |
| 471 | `.toolchain/pkgs/smashremix-src/src/Size.asm:166` | `adjust_ecb_display_` | 0x6E804 | `0x800F3004` | não detectado na janela | `jal     adjust_ecb_display_._lower_y` | unmatched | adjust ecb display |
| 472 | `.toolchain/pkgs/smashremix-src/src/Size.asm:171` | `adjust_ecb_display_` | 0x6E858 | `0x800F3058` | não detectado na janela | `jal     adjust_ecb_display_._center_y_and_width` | unmatched | adjust ecb display |
| 473 | `.toolchain/pkgs/smashremix-src/src/Size.asm:176` | `adjust_ecb_display_` | 0x6E8F0 | `0x800F30F0` | não detectado na janela | `jal     adjust_ecb_display_._center_y` | unmatched | adjust ecb display |
| 474 | `.toolchain/pkgs/smashremix-src/src/Size.asm:181` | `adjust_ecb_display_` | 0x6E948 | `0x800F3148` | não detectado na janela | `jal     adjust_ecb_display_._center_y_upper_y_and_width` | unmatched | adjust ecb display |
| 475 | `.toolchain/pkgs/smashremix-src/src/Size.asm:241` | `adjust_top_joint_and_ecb_` | 0x5D868 | `0x800E2068` | não detectado na janela | `jal     adjust_top_joint_and_ecb_` | `decomp/src/ft/ftmain.c:1875` `ftMainProcPhysicsMap` (+0x20) | adjust top joint and ecb |
| 476 | `.toolchain/pkgs/smashremix-src/src/Size.asm:294` | `kirby_ausp_` | 0xDB974 | `0x80160F34` | não detectado na janela | `jal     kirby_ausp_._first` | `decomp/src/ft/ftchar/ftkirby/ftkirbyspecialhi.c:165` `ftKirbySpecialAirHiProcPhysics` (+0x24) | kirby ausp |
| 477 | `.toolchain/pkgs/smashremix-src/src/Size.asm:298` | `kirby_ausp_` | 0xDB9A0 | `0x80160F60` | não detectado na janela | `jal     kirby_ausp_._second` | `decomp/src/ft/ftchar/ftkirby/ftkirbyspecialhi.c:165` `ftKirbySpecialAirHiProcPhysics` (+0x50) | kirby ausp |
| 478 | `.toolchain/pkgs/smashremix-src/src/Size.asm:343` | `initialize_base_multiplier_table_` | 0x534B4 | `0x800D7CB4` | não detectado na janela | `jal     initialize_base_multiplier_table_` | `decomp/src/ft/ftmanager.c:477` `ftManagerInitFighter` (+0x2C4) | initialize base multiplier table |
| 479 | `.toolchain/pkgs/smashremix-src/src/Size.asm:444` | `fix_size_` | 0x5AB94 | `0x800DF394` | não detectado na janela | `jal     fix_size_._create_hitbox` | `decomp/src/ft/ftmain.c:159` `ftMainParseMotionEvent` (+0x2A4) | fix size |
| 480 | `.toolchain/pkgs/smashremix-src/src/Size.asm:449` | `fix_size_` | 0x5AEAC | `0x800DF6AC` | não detectado na janela | `jal     fix_size_._change_hitbox_size` | unmatched | fix size |
| 481 | `.toolchain/pkgs/smashremix-src/src/Size.asm:471` | `fix_damage_` | 0x5AB4C | `0x800DF34C` | Toggle: Charge Smashes | `j       fix_damage_` | `decomp/src/ft/ftmain.c:159` `ftMainParseMotionEvent` (+0x25C) | fix damage |
| 482 | `.toolchain/pkgs/smashremix-src/src/Size.asm:564` | `adjust_player_walking_speed_` | 0x54274 | `0x800D8A74` | não detectado na janela | `j       adjust_player_walking_speed_` | `decomp/src/ft/ftphysics.c:92` `ftPhysicsSetGroundVelAbsStickRange` (+0x4) | adjust player walking speed |
| 483 | `.toolchain/pkgs/smashremix-src/src/Size.asm:600` | `adjust_player_initial_dash_speed_` | 0xB9780 | `0x8013ED40` | não detectado na janela | `jal     adjust_player_initial_dash_speed_` | `decomp/src/ft/ftcommon/ftcommondash.c:111` `ftCommonDashSetStatus` (+0x40) | adjust player initial dash speed |
| 484 | `.toolchain/pkgs/smashremix-src/src/Size.asm:627` | `adjust_player_dash_deceleration_` | 0x5417C | `0x800D897C` | não detectado na janela | `j       adjust_player_dash_deceleration_` | `decomp/src/ft/ftphysics.c:51` `ftPhysicsSetGroundVelFriction` (+0x4) | adjust player dash deceleration |
| 485 | `.toolchain/pkgs/smashremix-src/src/Size.asm:655` | `adjust_player_running_speed_` | 0xB9954 | `0x8013EF14` | não detectado na janela | `jal     adjust_player_running_speed_` | `decomp/src/ft/ftcommon/ftcommonrun.c:27` `ftCommonRunSetStatus` (+0x2C) | adjust player running speed |
| 486 | `.toolchain/pkgs/smashremix-src/src/Size.asm:682` | `adjust_player_jump_squat_` | 0xB9D48 | `0x8013F308` | não detectado na janela | `j      adjust_player_jump_squat_` | `decomp/src/ft/ftcommon/ftcommonkneebend.c:13` `ftCommonKneeBendProcUpdate` (+0x68) | adjust player jump squat |
| 487 | `.toolchain/pkgs/smashremix-src/src/Size.asm:708` | `adjust_jumping_height_multiplier_1` | 0xBA38C | `0x8013F94C` | não detectado na janela | `jal     adjust_jumping_height_multiplier_1` | `decomp/src/ft/ftcommon/ftcommonjump.c:64` `ftCommonJumpSetStatus` (+0xCC) | adjust jumping height multiplier 1 |
| 488 | `.toolchain/pkgs/smashremix-src/src/Size.asm:741` | `adjust_jumping_height_multiplier_2` | 0xBA8A4 | `0x8013FE64` | não detectado na janela | `jal     adjust_jumping_height_multiplier_2` | `decomp/src/ft/ftcommon/ftcommonjumpaerial.c:128` `ftCommonJumpAerialSetStatus` (+0xF0) | adjust jumping height multiplier 2 |
| 489 | `.toolchain/pkgs/smashremix-src/src/Size.asm:800` | `adjust_jumping_height_multiplier_kirby` | 0xBAA54 | `0x80140014` | não detectado na janela | `jal     adjust_jumping_height_multiplier_kirby` | `decomp/src/ft/ftcommon/ftcommonjumpaerial.c:185` `ftCommonJumpAerialMultiSetStatus` (+0xDC) | adjust jumping height multiplier kirby |
| 490 | `.toolchain/pkgs/smashremix-src/src/Size.asm:844` | `adjust_player_weight_1` | 0x5F788 | `0x800E3F88` | não detectado na janela | `j       adjust_player_weight_1` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0xCC) | adjust player weight 1 |
| 491 | `.toolchain/pkgs/smashremix-src/src/Size.asm:887` | `adjust_player_weight_2` | 0x5F920 | `0x800E4120` | não detectado na janela | `j       adjust_player_weight_2` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0x264) | adjust player weight 2 |
| 492 | `.toolchain/pkgs/smashremix-src/src/Size.asm:930` | `adjust_player_weight_3` | 0x5FA44 | `0x800E4244` | não detectado na janela | `j       adjust_player_weight_3` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0x388) | adjust player weight 3 |
| 493 | `.toolchain/pkgs/smashremix-src/src/Size.asm:973` | `adjust_player_weight_4` | 0xBE770 | `0x80143D30` | não detectado na janela | `j       adjust_player_weight_4` | `decomp/src/ft/ftcommon/ftcommontwister.c:91` `ftCommonTwisterShootFighter` (+0x6C) | adjust player weight 4 |
| 494 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1017` | `adjust_player_weight_5` | 0xC5AC4 | `0x8014B084` | não detectado na janela | `j       adjust_player_weight_5` | `decomp/src/ft/ftcommon/ftcommonthrown2.c:119` `ftCommonThrownReleaseThrownUpdateStats` (+0xB4) | adjust player weight 5 |
| 495 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1060` | `adjust_player_weight_6` | 0xC5E30 | `0x8014B3F0` | não detectado na janela | `j       adjust_player_weight_6` | `decomp/src/ft/ftcommon/ftcommonthrown2.c:224` `ftCommonThrownSetStatusDamageRelease` (+0xC0) | adjust player weight 6 |
| 496 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1103` | `adjust_player_weight_7` | 0xC904C | `0x8014E60C` | não detectado na janela | `j       adjust_player_weight_7` | `decomp/src/ft/ftcommon/ftcommoncapture.c:145` `ftCommonCaptureShoulderedSetStatus` (+0xB4) | adjust player weight 7 |
| 497 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1147` | `adjust_player_weight_8` | 0x845E8 | `0x80108DE8` | não detectado na janela | `j       adjust_player_weight_8` | `decomp/src/gr/grcommon/grinishie.c:92` `grInishieScaleGetPressure` (+0x98) | adjust player weight 8 |
| 498 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1190` | `adjust_camera_zoom_` | 0x8739C | `0x8010BB9C` | não detectado na janela | `j       adjust_camera_zoom_` | `decomp/src/gm/gmcamera.c:210` `gmCameraCalcFighterZoomRange` (+0x4) | adjust camera zoom |
| 499 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1219` | `adjust_camera_zoom_multiplier_` | 0x881C4 | `0x8010C9C4` | não detectado na janela | `jal     adjust_camera_zoom_multiplier_` | `decomp/src/gm/gmcamera.c:703` `gmCameraUpdatePlayerZoom` (+0x64) | adjust camera zoom multiplier |
| 500 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1240` | `adjust_camera_center_point_` | 0x87578 | `0x8010BD78` | não detectado na janela | `jal     adjust_camera_center_point_._1` | `decomp/src/gm/gmcamera.c:237` `gmCameraUpdateInterests` (+0x124) | adjust camera center point |
| 501 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1245` | `adjust_camera_center_point_` | 0x6E25C | `0x800F2A5C` | não detectado na janela | `jal     adjust_camera_center_point_._2` | `decomp/src/ft/ftdisplaymain.c:1089` `ftDisplayMainProcDisplay` (+0x120) | adjust camera center point |
| 502 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1250` | `adjust_camera_center_point_` | 0x6E280 | `0x800F2A80` | não detectado na janela | `jal     adjust_camera_center_point_._3` | `decomp/src/ft/ftdisplaymain.c:1089` `ftDisplayMainProcDisplay` (+0x144) | adjust camera center point |
| 503 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1255` | `adjust_camera_center_point_` | 0x6E29C | `0x800F2A9C` | não detectado na janela | `jal     adjust_camera_center_point_._4` | `decomp/src/ft/ftdisplaymain.c:1089` `ftDisplayMainProcDisplay` (+0x160) | adjust camera center point |
| 504 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1262` | `adjust_camera_center_point_` | 0x881AC | `0x8010C9AC` | não detectado na janela | `jal     adjust_camera_center_point_._5` | `decomp/src/gm/gmcamera.c:703` `gmCameraUpdatePlayerZoom` (+0x4C) | adjust camera center point |
| 505 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1334` | `adjust_size_egg_shield_` | 0x7C9A4 | `0x801011A4` | não detectado na janela | `jal     adjust_size_egg_shield_` | `decomp/src/ef/efmanager.c:4234` `efManagerYoshiShieldProcDisplay` (+0x24) | adjust size egg shield |
| 506 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1362` | `adjust_size_egg_lay_` | 0xC73D4 | `0x8014C994` | não detectado na janela | `j       adjust_size_egg_lay_` | `decomp/src/ft/ftcommon/ftcommoncaptureyoshi.c:129` `ftCommonYoshiEggMakeEffect` (+0x3C) | adjust size egg lay |
| 507 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1398` | `adjust_size_egg_entry_` | 0x7E764 | `0x80102F64` | não detectado na janela | `jal     adjust_size_egg_entry_` | `decomp/src/ef/efmanager.c:5443` `efManagerYoshiEntryEggMakeEffect` (+0x30) | adjust size egg entry |
| 508 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1429` | `save_player_struct_` | 0x668E8 | `0x800EB0E8` | não detectado na janela | `jal     save_player_struct_._grounded` | unmatched | save player struct |
| 509 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1434` | `save_player_struct_` | 0x668F8 | `0x800EB0F8` | não detectado na janela | `jal     save_player_struct_._aerial` | unmatched | save player struct |
| 510 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1439` | `save_player_struct_` | 0xBC580 | `0x80141B40` | não detectado na janela | `jal     save_player_struct_._wall_bounce` | `decomp/src/ft/ftcommon/ftcommonwalldamage.c:25` `ftCommonWallDamageSetStatus` (+0x38) | save player struct |
| 511 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1444` | `save_player_struct_` | 0xCFBFC | `0x801551BC` | não detectado na janela | `jal     save_player_struct_._wall_bounce` | `decomp/src/ft/ftchar/ftness/ftnessspecialhi.c:824` `ftNessSpecialAirHiJibakuBoundSetStatus` (+0xA8) | save player struct |
| 512 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1449` | `save_player_struct_` | 0xC6E4C | `0x8014C40C` | não detectado na janela | `jal     save_player_struct_._kirby_star_clipping_collide` | `decomp/src/ft/ftcommon/ftcommoncapturekirby.c:387` `ftCommonThrownCommonStarProcMap` (+0x18C) | save player struct |
| 513 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1496` | `save_player_struct_` | 0xA97C4 + 0x10 | `0x8012DFC4 + 0x10` | não detectado na janela | - | n/a (endereço por expressão) | save player struct |
| 514 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1541` | `samus_gun_blast` | 0xA98B4 + 0x14 | `0x8012E0B4 + 0x14` | não detectado na janela | - | n/a (endereço por expressão) | samus gun blast |
| 515 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1584` | `adjust_suck_gfx_` | 0x7FA9C | `0x8010429C` | não detectado na janela | `j       adjust_suck_gfx_` | `decomp/src/ef/efmanager.c:6228` `efManagerKirbyInhaleWindProcUpdate` (+0x5C) | adjust suck gfx |
| 516 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1611` | `usp` | 0xA9B7C + 0x14 | `0x8012E37C + 0x14` | não detectado na janela | - | n/a (endereço por expressão) | usp |
| 517 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1615` | `usp` | 0xA9BA4 + 0x14 | `0x8012E3A4 + 0x14` | não detectado na janela | - | n/a (endereço por expressão) | usp |
| 518 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1619` | `usp` | 0xA9BF4 + 0x14 | `0x8012E3F4 + 0x14` | não detectado na janela | - | n/a (endereço por expressão) | usp |
| 519 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1795` | `adjust_usp_projectile_size_` | 0xDE860 | `0x80163E20` | não detectado na janela | `jal     adjust_usp_projectile_size_` | `decomp/src/ft/ftchar/ftlink/ftlinkspecialhi.c:150` `ftLinkSpecialHiMakeWeapon` (+0x8C) | adjust usp projectile size |
| 520 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1843` | `adjust_usp_gfx_size_` | 0xDE818 | `0x80163DD8` | não detectado na janela | `jal     adjust_usp_gfx_size_` | `decomp/src/ft/ftchar/ftlink/ftlinkspecialhi.c:150` `ftLinkSpecialHiMakeWeapon` (+0x44) | adjust usp gfx size |
| 521 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1900` | `adjust_gfx_size_` | 0x7CC6C | `0x8010146C` | não detectado na janela | `jal     adjust_gfx_size_` | `decomp/src/ef/efmanager.c:4288` `efManagerThunderAmpMakeEffect` (+0x64) | adjust gfx size |
| 522 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1928` | `adjust_pkfire_position_` | 0xCE3F4 | `0x801539B4` | não detectado na janela | `jal     adjust_pkfire_position_` | `decomp/src/ft/ftchar/ftness/ftnessspecialn.c:17` `ftNessSpecialNProcAccessory` (+0x64) | adjust pkfire position |
| 523 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1954` | `usp` | 0xA9C94 + 0x10 | `0x8012E494 + 0x10` | não detectado na janela | - | n/a (endereço por expressão) | usp |
| 524 | `.toolchain/pkgs/smashremix-src/src/Size.asm:1959` | `usp` | 0x7E23C | `0x80102A3C` | não detectado na janela | `jal     update_routine_._initialize` | `decomp/src/ef/efmanager.c:5194` `efManagerNessPKThunderWaveMakeEffect` (+0x44) | usp |
| 525 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2017` | `dsp` | 0xA9C1C + 0x10 | `0x8012E41C + 0x10` | não detectado na janela | - | n/a (endereço por expressão) | dsp |
| 526 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2022` | `dsp` | 0x7DE0C | `0x8010260C` | não detectado na janela | `jal     update_routine_._initialize` | `decomp/src/ef/efmanager.c:5038` `efManagerNessPsychicMagnetMakeEffect` (+0x34) | dsp |
| 527 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2092` | `jigglypuff` | 0xA9B14 + 0x10 | `0x8012E314 + 0x10` | não detectado na janela | - | n/a (endereço por expressão) | jigglypuff |
| 528 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2148` | `nsp` | 0xA99D4 + 0x14 | `0x8012E1D4 + 0x14` | não detectado na janela | - | n/a (endereço por expressão) | nsp |
| 529 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2245` | `arwing` | 0xA9EF4 + 0x10 | `0x8012E6F4 + 0x10` | não detectado na janela | - | n/a (endereço por expressão) | arwing |
| 530 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2250` | `arwing` | 0x7F0F0 | `0x801038F0` | não detectado na janela | `jal     update_routine_._initialize` | `decomp/src/ef/efmanager.c:5819` `efManagerFoxEntryArwingMakeEffect` (+0x104) | arwing |
| 531 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2310` | `animation_` | 0xB856C | `0x8013DB2C` | não detectado na janela | `j       animation_` | `decomp/src/ft/ftcommon/ftcommonentry.c:147` `ftCommonAppearProcPhysics` (exato) | animation |
| 532 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2362` | `gfx_` | 0x7EF68 | `0x80103768` | não detectado na janela | `jal     gfx_._top_joint` | `decomp/src/ef/efmanager.c:5775` `efManagerMarioEntryDokanMakeEffect` (+0x7C) | gfx |
| 533 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2368` | `gfx_` | 0x7EC5C | `0x8010345C` | não detectado na janela | `jal     gfx_._top_joint` | `decomp/src/ef/efmanager.c:5670` `efManagerDonkeyEntryTaruMakeEffect` (+0x44) | gfx |
| 534 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2374` | `gfx_` | 0x7ECB8 | `0x801034B8` | não detectado na janela | `jal     gfx_._top_joint` | `decomp/src/ef/efmanager.c:5689` `efManagerSamusEntryPointMakeEffect` (+0x44) | gfx |
| 535 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2380` | `gfx_` | 0x7E328 | `0x80102B28` | não detectado na janela | `jal     gfx_._link_ground_gfx` | `decomp/src/ef/efmanager.c:5238` `efManagerLinkEntryWaveMakeEffect` (+0x44) | gfx |
| 536 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2386` | `gfx_` | 0x7E384 | `0x80102B84` | não detectado na janela | `jal     gfx_._entry_shaft` | `decomp/src/ef/efmanager.c:5257` `efManagerLinkEntryBeamMakeEffect` (+0x44) | gfx |
| 537 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2392` | `gfx_` | 0x7E410 | `0x80102C10` | não detectado na janela | `jal     gfx_._2nd_joint` | `decomp/src/ef/efmanager.c:5276` `efManagerKirbyEntryStarMakeEffect` (+0x74) | gfx |
| 538 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2398` | `gfx_` | 0x7EE70 | `0x80103670` | não detectado na janela | `jal     gfx_._blue_falcon` | `decomp/src/ef/efmanager.c:5727` `efManagerCaptainEntryCarMakeEffect` (+0x104) | gfx |
| 539 | `.toolchain/pkgs/smashremix-src/src/Size.asm:2620` | `apply_size_to_animation_` | 0x3EA5C | `0x8003DE5C` | não detectado na janela | - | unmatched | apply size to animation |

### Skeleton.asm

Modo esqueleto: dois hooks de modo.

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 540 | `.toolchain/pkgs/smashremix-src/src/Skeleton.asm:45` | `skeleton_mode_1_` | 0x6DCB0 | `0x800F24B0` | não detectado na janela | `jal     skeleton_mode_1_` | `decomp/src/ft/ftdisplaymain.c:947` `ftDisplayMainDrawAll` (+0x10) | skeleton mode 1 |
| 541 | `.toolchain/pkgs/smashremix-src/src/Skeleton.asm:59` | `skeleton_mode_2_` | 0x6DA28 | `0x800F2228` | não detectado na janela | `jal     skeleton_mode_2_` | `decomp/src/ft/ftdisplaymain.c:859` `ftDisplayMainDrawSkeleton` (+0x74) | skeleton mode 2 |

### Smashketball.asm

Modo Smashketball: IA da bola, overtime, chant, seletor de gol, despawn e estatísticas.

Hooks de código: 13; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 542 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:477` | `prevent_despawn` | 0xEA004 | `0x8016F5C4` | não detectado na janela | `jal     prevent_despawn` | `decomp/src/it/itprocess.c:92` `itProcessProcItemMain` (+0x90) | prevent despawn |
| 543 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:502` | `camera_follow` | 0x87914 | `0x8010C114` | não detectado na janela | `jal     camera_follow` | unmatched | camera follow |
| 544 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1109` | `play_end_buzzer_` | 0x8F040 | `0x80113840` | não detectado na janela | `j       play_end_buzzer_` | `decomp/src/if/ifcommon.c:2881` `ifCommonBattleEndAddSoundQueueID` (+0x3C) | play end buzzer |
| 545 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1131` | `force_chant_` | 0xDF520 | `0x80164AE0` | não detectado na janela | `jal     force_chant_` | `decomp/src/ft/ftpublic.c:79` `ftPublicTryStartCall` (+0x30) | force chant |
| 546 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1153` | `ai_ball_behavior_` | 0xAD584 | `0x80132B44` | não detectado na janela | `jal     ai_ball_behavior_` | `decomp/src/sc/sc1pmode/sc1pintro.c:857` `sc1PIntroMakeStageInfo` (+0x4) | ai ball behavior |
| 547 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1211` | `ai_ball_attack_range` | 0xADB84 | `0x80133144` | não detectado na janela | `j       ai_ball_attack_range` | `decomp/src/mn/mndata/mncharacters.c:2316` `mnCharactersMakeEmblemCamera` (+0x4) | ai ball attack range |
| 548 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1268` | `allow_overtime_` | 0x10B18C | `0x8018E29C` | não detectado na janela | `jal     allow_overtime_` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:1016` `sc1PBonusStageSetTimeUp` (+0x4) | allow overtime |
| 549 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1286` | `no_initial_damage_` | 0x10AED0 | `0x8018DFE0` | não detectado na janela | `jal     no_initial_damage_` | `decomp/src/mv/mvopening/mvopeningfox.c:534` `mvOpeningFoxFuncLights` (exato) | no initial damage |
| 550 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1304` | `prevent_match_stats_reset_` | 0x63468 | `0x800E7C68` | não detectado na janela | `j       prevent_match_stats_reset_` | `decomp/src/ft/ftparam.c:182` `ftParamInitPlayerBattleStats` (+0x1C) | prevent match stats reset |
| 551 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1339` | `enable_toggling_mode_` | 0x1335FC | `0x8013537C` | não detectado na janela | `jal     enable_toggling_mode_` | `decomp/src/mn/mnplayers/mnplayersvs.c:2166` `mnPlayersVSUpdateGameMode` (+0x48) | enable toggling mode |
| 552 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1344` | `enable_toggling_mode_` | 0x133680 | `0x80135400` | não detectado na janela | `jal     enable_toggling_mode_._get_offset` | `decomp/src/mn/mnvsmode/mnvsresults.c:1949` `mnVSResultsGetNumberColorID` (+0xC) | enable toggling mode |
| 553 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1349` | `enable_toggling_mode_` | 0x133724 | `0x801354A4` | não detectado na janela | `jal     enable_toggling_mode_._skip_to_end` | `decomp/src/mn/mnvsmode/mnvsresults.c:1974` `mnVSResultsMakeHeader` (+0x8) | enable toggling mode |
| 554 | `.toolchain/pkgs/smashremix-src/src/Smashketball.asm:1400` | `use_goal_yellow_picker_` | 0x132258 | `0x80133FD8` | não detectado na janela | `jal     use_goal_yellow_picker_` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:1452` `mnPlayers1PTrainingPortraitFlashThreadUpdate` (+0x24) | use goal yellow picker |

### Spawn.asm

Carregamento de ponto de spawn/respawn.

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 555 | `.toolchain/pkgs/smashremix-src/src/Spawn.asm:19` | `load_respawn_point_` | 0x000780B0 | `0x800FC8B0` | não detectado na janela | `j       load_respawn_point_` | `decomp/src/mp/mpcollision.c:4432` `mpCollisionGetMapObjPositionID` (+0x1C) | load respawn point |
| 556 | `.toolchain/pkgs/smashremix-src/src/Spawn.asm:103` | `load_spawn_` | 0x00076764 | `0x800FAF64` | não detectado na janela | `j       Spawn.load_spawn_` | `decomp/src/mp/mpcollision.c:3515` `mpCollisionGetPlayerMapObjPosition` (exato) | load spawn |

### SpecialZoom.asm

Special Zoom (BETA): câmera de zoom para especiais, knockback final e offset.

Hooks de código: 4; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 557 | `.toolchain/pkgs/smashremix-src/src/SpecialZoom.asm:112` | `final_knockback_was_set` | 0xBBF24 | `0x801414E4` | Toggle: Special Zoom (BETA) | `j       final_knockback_was_set` | unmatched | final knockback was set |
| 558 | `.toolchain/pkgs/smashremix-src/src/SpecialZoom.asm:237` | `create_zoom_camera` | 0x89410 | `0x8010DC10` | Toggle: Special Zoom (BETA) | `j       create_zoom_camera` | `decomp/src/gm/gmcamera.c:1251` `gmCameraMakeWallpaperCamera` (+0xBC) | create zoom camera |
| 559 | `.toolchain/pkgs/smashremix-src/src/SpecialZoom.asm:361` | `zoomed_in_camera_offset` | 0x881BC | `0x8010C9BC` | Toggle: Special Zoom (BETA) | `j       zoomed_in_camera_offset` | `decomp/src/gm/gmcamera.c:703` `gmCameraUpdatePlayerZoom` (+0x5C) | zoomed in camera offset |
| 560 | `.toolchain/pkgs/smashremix-src/src/SpecialZoom.asm:420` | `objects_update` | 0xB1EC | `0x8000A5EC` | Toggle: Special Zoom (BETA) | `j       objects_update` | `decomp/src/sys/objman.c:2334` `gcRunAll` (+0x8) | objects update |

### Speed.asm

Velocidade do jogo: taxa de update e processamento.

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 561 | `.toolchain/pkgs/smashremix-src/src/Speed.asm:14` | `process_update_` | 0xB118 | `0x8000A518` | não detectado na janela | `j       process_update_` | `decomp/src/sys/objman.c:2272` `gcRunGObjProcess` (+0x7C) | process update |
| 562 | `.toolchain/pkgs/smashremix-src/src/Speed.asm:58` | `handle_rate_` | 0x8FA7C | `0x8011427C` | não detectado na janela | `j       handle_rate_` | `decomp/src/if/ifcommon.c:3127` `ifCommonBattleGoUpdateInterface` (+0x240) | handle rate |

### SpotDodge.asm

Spot Dodge: checagem de input.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 563 | `.toolchain/pkgs/smashremix-src/src/SpotDodge.asm:9` | `check_input` | 0xBC9DC | `0x80141F9C` | Toggle: Spot Dodging | `j       check_input` | `decomp/src/ft/ftcommon/ftcommonpass.c:96` `ftCommonGuardPassCheckInterruptGuard` (+0x10) | check input |

### SRAM.asm

Leitura/escrita de SRAM (dados salvos) em duas rotinas.

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 564 | `.toolchain/pkgs/smashremix-src/src/SRAM.asm:52` | `SRAM` | 0x0005000C | `0x800D462C` | não detectado na janela | - | `decomp/src/lb/lbbackup.c:39` `lbBackupWrite` (+0x38) | SRAM |
| 565 | `.toolchain/pkgs/smashremix-src/src/SRAM.asm:58` | `SRAM` | 0x00050050 | `0x800D4670` | não detectado na janela | `b       0x800D468C` | `decomp/src/lb/lbbackup.c:47` `lbBackupIsSramValid` (+0x2C) | SRAM |

### StaleMoves.asm

Stale moves: penalização de repetição de golpe.

Hooks de código: 3; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 566 | `.toolchain/pkgs/smashremix-src/src/StaleMoves.asm:40` | `stale_moves` | 0x65CDC | `0x800EA4DC` | não detectado na janela | `j       stale_moves.check_toggle` | `decomp/src/ft/ftparam.c:1737` `ftParamGetStale` (+0x6C) | stale moves |
| 567 | `.toolchain/pkgs/smashremix-src/src/StaleMoves.asm:178` | `stale_moves` | 0x65CF8 | `0x800EA4F8` | não detectado na janela | - | `decomp/src/ft/ftparam.c:1737` `ftParamGetStale` (+0x88) | stale moves |
| 568 | `.toolchain/pkgs/smashremix-src/src/StaleMoves.asm:181` | `stale_moves` | 0x65D04 | `0x800EA504` | não detectado na janela | - | `decomp/src/ft/ftparam.c:1737` `ftParamGetStale` (+0x94) | stale moves |

### Stamina.asm

Modo Stamina: exibição de stock/tempo, prevenção de crash, input de down-bounce e atualização de porta de percentual.

Hooks de código: 59; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 569 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:21` | `stamina_display_` | 0x8A1FC | `0x8010E9FC` | não detectado na janela | `j   stamina_display_` | `decomp/src/if/ifcommon.c:594` `ifCommonPlayerDamageUpdateDigits` (+0x108) | stamina display |
| 570 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:66` | `stamina_dark_` | 0x8A428 | `0x8010EC28` | não detectado na janela | `j   stamina_dark_` | `decomp/src/if/ifcommon.c:594` `ifCommonPlayerDamageUpdateDigits` (+0x334) | stamina dark |
| 571 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:142` | `stamina_dark_fix_2` | 0x8A15C | `0x8010E95C` | não detectado na janela | `j   stamina_dark_fix_2` | `decomp/src/if/ifcommon.c:594` `ifCommonPlayerDamageUpdateDigits` (+0x68) | stamina dark fix 2 |
| 572 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:187` | `stamina_knockback_` | 0x5F764 | `0x800E3F64` | não detectado na janela | `j       stamina_knockback_` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0xA8) | stamina knockback |
| 573 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:216` | `stamina_knockback_item_` | 0x5FA24 | `0x800E4224` | não detectado na janela | `j       stamina_knockback_item_` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0x368) | stamina knockback item |
| 574 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:274` | `stamina_knockback_throw_` | 0x65670 | `0x800E9E70` | não detectado na janela | `j       stamina_knockback_throw_` | `decomp/src/ft/ftparam.c:1606` `ftParamGetCommonKnockback` (+0xF8) | stamina knockback throw |
| 575 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:303` | `stamina_knockback_projectile_` | 0x5F900 | `0x800E4100` | não detectado na janela | `j       stamina_knockback_projectile_` | `decomp/src/ft/ftmain.c:2801` `ftMainProcessHitCollisionStatsMain` (+0x244) | stamina knockback projectile |
| 576 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:383` | `stamina_player_control_` | 0x8F260 | `0x80113A60` | não detectado na janela | `j       stamina_player_control_` | `decomp/src/if/ifcommon.c:2900` `ifCommonBattleUpdateScoreStocks` (+0x1D4) | stamina player control |
| 577 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:434` | `stamina_main_` | 0x65A70 | `0x800EA270` | não detectado na janela | `j           stamina_main_` | `decomp/src/ft/ftparam.c:1689` `ftParamUpdateDamage` (+0x28) | stamina main |
| 578 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:515` | `stamina_ko_action` | 0x6273C | `0x800E6F3C` | não detectado na janela | `j           stamina_ko_action` | `decomp/src/ft/ftmain.c:4578` `ftMainSetStatus` (+0x18) | stamina ko action |
| 579 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:687` | `stamina_shield_prevent` | 0xC3720 | `0x80148CE0` | não detectado na janela | `j           stamina_shield_prevent` | `decomp/src/ft/ftcommon/ftcommonguard1.c:481` `ftCommonGuardOnCheckInterruptSuccess` (+0x24) | stamina shield prevent |
| 580 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:747` | `stamina_ko_prevent` | 0xB6BA8 | `0x8013C168` | não detectado na janela | `j           stamina_ko_prevent` | `decomp/src/ft/ftcommon/ftcommondead.c:212` `ftCommonDeadInitStatusVars` (+0x48) | stamina ko prevent |
| 581 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:787` | `stamina_ko_prevent_front` | 0xB73F8 | `0x8013C9B8` | não detectado na janela | `j           stamina_ko_prevent_front` | `decomp/src/ft/ftcommon/ftcommondead.c:457` `ftCommonDeadUpFallProcUpdate` (+0x1AC) | stamina ko prevent front |
| 582 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:825` | `stamina_stock` | 0x53834 | `0x800D8034` | não detectado na janela | `j           stamina_stock` | `decomp/src/ft/ftmanager.c:821` `ftManagerMakeFighter` (+0xF8) | stamina stock |
| 583 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:853` | `stamina_stock_2` | 0x53814 | `0x800D8014` | não detectado na janela | `j           stamina_stock_2` | `decomp/src/ft/ftmanager.c:821` `ftManagerMakeFighter` (+0xD8) | stamina stock 2 |
| 584 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:889` | `stamina_fix_1` | 0xDFBE0 | `0x801651A0` | não detectado na janela | `j           stamina_fix_1` | `decomp/src/ft/ftpublic.c:307` `ftPublicProcUpdate` (+0x6C) | stamina fix 1 |
| 585 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:908` | `stamina_fix_2` | 0xB68F8 | `0x8013BEB8` | não detectado na janela | `j           stamina_fix_2` | `decomp/src/ft/ftcommon/ftcommondead.c:48` `ftCommonDeadUpdateScore` (+0x154) | stamina fix 2 |
| 586 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:957` | `stamina_fix_3` | 0x8E970 | `0x80113170` | não detectado na janela | `j           stamina_fix_3` | `decomp/src/if/ifcommon.c:2685` `ifCommonTimerFuncRun` (+0x6C) | stamina fix 3 |
| 587 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:977` | `stamina_fix_4` | 0xB6748 | `0x8013BD08` | não detectado na janela | `j           stamina_fix_4` | `decomp/src/ft/ftcommon/ftcommondead.c:21` `ftCommonDeadUpdateRumble` (+0x7C) | stamina fix 4 |
| 588 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:997` | `stamina_fix_5` | 0xB694C | `0x8013BF0C` | não detectado na janela | `j           stamina_fix_5` | `decomp/src/ft/ftcommon/ftcommondead.c:48` `ftCommonDeadUpdateScore` (+0x1A8) | stamina fix 5 |
| 589 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1017` | `stamina_fix_6` | 0xB69E4 | `0x8013BFA4` | não detectado na janela | `j           stamina_fix_6` | `decomp/src/ft/ftcommon/ftcommondead.c:126` `ftCommonDeadCheckRebirth` (+0x10) | stamina fix 6 |
| 590 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1037` | `stamina_fix_7` | 0xB69F8 | `0x8013BFB8` | não detectado na janela | `j           stamina_fix_7` | `decomp/src/ft/ftcommon/ftcommondead.c:126` `ftCommonDeadCheckRebirth` (+0x24) | stamina fix 7 |
| 591 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1069` | `stamina_down_wait_down` | 0xBEC7C | `0x8014423C` | não detectado na janela | `j           stamina_down_wait_down` | `decomp/src/ft/ftcommon/ftcommondownwaitbounce.c:21` `ftCommonDownWaitProcUpdate` (+0x1C) | stamina down wait down |
| 592 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1105` | `stamina_down_wait_input` | 0xBEC9C | `0x8014425C` | não detectado na janela | `j           stamina_down_wait_input` | `decomp/src/ft/ftcommon/ftcommondownwaitbounce.c:34` `ftCommonDownWaitProcInterrupt` (+0x8) | stamina down wait input |
| 593 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1141` | `stamina_down_bounce_input` | 0xBEDA0 | `0x80144360` | não detectado na janela | `j           stamina_down_bounce_input` | `decomp/src/ft/ftcommon/ftcommondownwaitbounce.c:64` `ftCommonDownBounceProcUpdate` (+0x58) | stamina down bounce input |
| 594 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1177` | `stamina_ledge` | 0xBF674 | `0x80144C34` | não detectado na janela | `j           stamina_ledge` | `decomp/src/ft/ftcommon/ftcommoncliffcatchwait.c:38` `ftCommonCliffCatchSetStatus` (+0x10) | stamina ledge |
| 595 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1222` | `stamina_percent_prevent` | 0x65AB4 | `0x800EA2B4` | não detectado na janela | `j           stamina_percent_prevent` | `decomp/src/ft/ftparam.c:1689` `ftParamUpdateDamage` (+0x6C) | stamina percent prevent |
| 596 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1287` | `percent_port_refresh` | 0x10A37C | `0x8018D48C` | não detectado na janela | `j           percent_port_refresh` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:422` `sc1PTrainingModeUpdateCPOption` (+0x14) | percent port refresh |
| 597 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1307` | `stamina_invis_prevent` | 0xB6B90 | `0x8013C150` | não detectado na janela | `j           stamina_invis_prevent` | `decomp/src/ft/ftcommon/ftcommondead.c:212` `ftCommonDeadInitStatusVars` (+0x30) | stamina invis prevent |
| 598 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1365` | `action_hit_crash_prevent` | 0x63A5C | `0x800E825C` | não detectado na janela | `j           action_hit_crash_prevent` | `decomp/src/ft/ftparam.c:402` `ftParamStopVoiceRunProcDamage` (+0x20) | action hit crash prevent |
| 599 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1430` | `egg_crash_prevent` | 0xB6B74 | `0x8013C134` | não detectado na janela | `j           egg_crash_prevent` | `decomp/src/ft/ftcommon/ftcommondead.c:212` `ftCommonDeadInitStatusVars` (+0x14) | egg crash prevent |
| 600 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1516` | `overkill_prevent` | 0x8F224 | `0x80113A24` | não detectado na janela | `j           overkill_prevent` | `decomp/src/if/ifcommon.c:2900` `ifCommonBattleUpdateScoreStocks` (+0x198) | overkill prevent |
| 601 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1566` | `vs_menu` | 0x124BBC | `0x8013420C` | não detectado na janela | - | `decomp/src/mv/mvopening/mvopeningroom.c:1191` `mvOpeningRoomGetDroppedFighterKind` (+0x58) | vs menu |
| 602 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1570` | `vs_menu` | 0x123080 | `0x801326D0` | não detectado na janela | - | `decomp/src/mn/mnoption/mnscreenadjust.c:435` `mnScreenAdjustFuncStart` (+0x4) | vs menu |
| 603 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1578` | `add_to_rule_button_` | 0x122C6C | `0x801322BC` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:395` `mnPlayers1PBonusCheckFighterLocked` (exato) | add to rule button |
| 604 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1581` | `add_to_rule_button_` | 0x122C78 | `0x801322C8` | não detectado na janela | - | `decomp/src/mv/mvopening/mvopeningroom.c:362` `func_ovl34_801322C8` (exato) | add to rule button |
| 605 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1586` | `add_to_rule_button_` | 0x001241A4 | `0x801337F4` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsoptions.c:1315` `mnVSOptionsMakeDecalCamera` (+0x1C) | add to rule button |
| 606 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1589` | `add_to_rule_button_` | 0x001241AC | `0x801337FC` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsoptions.c:1315` `mnVSOptionsMakeDecalCamera` (+0x24) | add to rule button |
| 607 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1594` | `add_to_rule_button_` | 0x001241B8 | `0x80133808` | não detectado na janela | `j       add_to_rule_button_._set_mode` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1046` `mnPlayers1PGameMakeTimeSelect` (+0x4) | add to rule button |
| 608 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1600` | `add_to_rule_button_` | 0x122C84 | `0x801322D4` | não detectado na janela | `j       add_to_rule_button_` | `decomp/src/mn/mncommon/mnmessage.c:353` `mnMessageFuncStart` (exato) | add to rule button |
| 609 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1605` | `add_to_rule_button_` | 0x122EC4 | `0x80132514` | não detectado na janela | `j       add_to_rule_button_._position` | `decomp/src/mn/mncommon/mnmodeselect.c:759` `mnModeSelectEjectOptions` (+0x4) | add to rule button |
| 610 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1611` | `add_to_rule_button_` | 0x124ACC | `0x8013411C` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:1365` `mnPlayers1PBonusMakeFighter` (+0x14) | add to rule button |
| 611 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1614` | `add_to_rule_button_` | 0x124BE4 | `0x80134234` | não detectado na janela | - | `decomp/src/mv/mvopening/mvopeningroom.c:1191` `mvOpeningRoomGetDroppedFighterKind` (+0x80) | add to rule button |
| 612 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1617` | `add_to_rule_button_` | 0x124458 | `0x80133AA8` | não detectado na janela | `b       0x80133B30` | `decomp/src/mn/mncommon/mntitle.c:1407` `mnTitleMakeCameras` (+0x14) | add to rule button |
| 613 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1700` | `add_to_stock_time_button_` | 0x123744 | `0x80132D94` | não detectado na janela | - | `decomp/src/mn/mndata/mnsoundtest.c:1396` `mnSoundTestMakeAFunctionSObj` (+0x4) | add to stock time button |
| 614 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1705` | `add_to_stock_time_button_` | 0x123804 | `0x80132E54` | não detectado na janela | `jal     add_to_stock_time_button_` | `decomp/src/mn/mnoption/mnoption.c:917` `mnOptionFuncRun` (+0x8) | add to stock time button |
| 615 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1711` | `add_to_stock_time_button_` | 0x1237B0 | `0x80132E00` | não detectado na janela | `jal     add_to_stock_time_button_._min` | `decomp/src/mn/mncommon/mntitle.c:1031` `mnTitleLogoProcDisplay` (+0x4) | add to stock time button |
| 616 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1716` | `add_to_stock_time_button_` | 0x1237C4 | `0x80132E14` | não detectado na janela | `jal     add_to_stock_time_button_._min_position` | `decomp/src/mn/mnoption/mnoption.c:908` `mnOptionWriteBackup` (+0x4) | add to stock time button |
| 617 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1772` | `add_to_stock_time_button_` | 0x1234F0 | `0x80132B40` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pintro.c:857` `sc1PIntroMakeStageInfo` (exato) | add to stock time button |
| 618 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1780` | `add_to_stock_time_value_` | 0x12353C | `0x80132B8C` | não detectado na janela | `j       add_to_stock_time_value_` | `decomp/src/mn/mnmaps/mnmaps.c:1466` `mnMapsLoadMapFile` (+0x8) | add to stock time value |
| 619 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1787` | `add_to_stock_time_value_` | 0x123674 | `0x80132CC4` | não detectado na janela | `jal     add_to_stock_time_value_._position` | `decomp/src/mn/mnoption/mnoption.c:855` `mnOptionMakeDecalsCamera` (+0x4) | add to stock time value |
| 620 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1794` | `add_to_stock_time_value_` | 0x124D98 | `0x801343E8` | não detectado na janela | `jal     add_to_stock_time_value_._scroll_left` | `decomp/src/mn/mndata/mnvsrecord.c:1353` `mnVSRecordMakeBattleScoreTableValues` (+0x8) | add to stock time value |
| 621 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1803` | `add_to_stock_time_value_` | 0x124F00 | `0x80134550` | não detectado na janela | `jal     add_to_stock_time_value_._scroll_right` | `decomp/src/mn/mnvsmode/mnvsresults.c:1527` `mnVSResultMakeNoContestText` (+0x10) | add to stock time value |
| 622 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1968` | `add_to_stock_time_value_` | 0x124098 | `0x801336E8` | não detectado na janela | `b       0x80133750` | `decomp/src/mn/mndata/mnsoundtest.c:1686` `mnSoundTestMakeAllSObjs` (+0x10) | add to stock time value |
| 623 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1984` | `vs_css` | 0x1323E8 | `0x80134168` | não detectado na janela | - | `decomp/src/mn/mncommon/mntitlefiles.c:35` `mnTitleLoadFiles` (+0x28) | vs css |
| 624 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1992` | `stamina_scrolling_` | 0x13660C | `0x8013838C` | não detectado na janela | `jal     stamina_scrolling_._right` | `decomp/src/ft/ftcomputer.c:6752` `func_ovl3_8013837C` (+0x10) | stamina scrolling |
| 625 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:1997` | `stamina_scrolling_` | 0x136690 | `0x80138410` | não detectado na janela | `jal     stamina_scrolling_._left` | `decomp/src/ft/ftcomputer.c:6752` `func_ovl3_8013837C` (+0x94) | stamina scrolling |
| 626 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:2066` | `stamina_hp_display_` | 0x132444 | `0x801341C4` | não detectado na janela | `jal     stamina_hp_display_._bg` | `decomp/src/mv/mvopening/mvopeningroom.c:1191` `mvOpeningRoomGetDroppedFighterKind` (+0x10) | stamina hp display |
| 627 | `.toolchain/pkgs/smashremix-src/src/Stamina.asm:2072` | `stamina_hp_display_` | 0x1324E4 | `0x80134264` | não detectado na janela | `jal     stamina_hp_display_._value` | `decomp/src/mn/mnvsmode/mnvsresults.c:1330` `mnVSResultsMakeWinnerText` (+0x28) | stamina hp display |

### Stereo.asm

Correção de pan estéreo de SFX.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 628 | `.toolchain/pkgs/smashremix-src/src/Stereo.asm:16` | `fix_sfx_pan_` | 0x440A4 | `0x800C86C4` | não detectado na janela | `j      fix_sfx_pan_` | `decomp/src/lb/lbcommon.c:738` `lbCommonMakePositionFGM` (+0x70) | fix sfx pan |

### StockMode.asm

Modo stock: define a contagem inicial de stocks.

Hooks de código: 1; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 629 | `.toolchain/pkgs/smashremix-src/src/StockMode.asm:49` | `set_initial_stock_count_` | 0x10A3A4 | `0x8018D4B4` | não detectado na janela | `jal     set_initial_stock_count_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:422` `sc1PTrainingModeUpdateCPOption` (+0x3C) | set initial stock count |

### Surface.asm

Superfícies: fricção, conveyor para fighter e item, correção do ácido de Zebes e override de FGM.

Hooks de código: 8; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 630 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:197` | `get_friction_` | 0x543DC | `0x800D8BDC` | não detectado na janela | - | `decomp/src/ft/ftphysics.c:157` `ftPhysicsApplyGroundVelFriction` (+0x28) | get friction |
| 631 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:209` | `get_friction_` | 0x5D9DC | `0x800E21DC` | não detectado na janela | - | `decomp/src/ft/ftmain.c:1875` `ftMainProcPhysicsMap` (+0x194) | get friction |
| 632 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:221` | `get_friction_` | 0xDC2BC | `0x8016187C` | não detectado na janela | - | `decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:490` `ftKirbySpecialNTurnProcMap` (+0x30) | get friction |
| 633 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:236` | `get_struct_` | 0x61474 | `0x800E5C74` | não detectado na janela | `j       _bounce` | `decomp/src/ft/ftmain.c:3750` `ftMainGetGroundHitObstacle` (+0x44) | get struct |
| 634 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:348` | `fgm_override_` | 0x5F538 | `0x800E3D38` | não detectado na janela | `j       fgm_override_` | `decomp/src/ft/ftmain.c:2728` `ftMainUpdateDamageStatGround` (+0x8C) | fgm override |
| 635 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:382` | `zebes_acid_fix_` | 0x83A08 | `0x80108208` | não detectado na janela | - | `decomp/src/gr/grcommon/grzebes.c:70` `grZebesMakeAcid` (+0x11C) | zebes acid fix |
| 636 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:394` | `fighter_apply_conveyor_surface` | 0x5DBD8 | `0x800E23D8` | não detectado na janela | `j       fighter_apply_conveyor_surface` | `decomp/src/ft/ftmain.c:1875` `ftMainProcPhysicsMap` (+0x390) | fighter apply conveyor surface |
| 637 | `.toolchain/pkgs/smashremix-src/src/Surface.asm:454` | `item_apply_conveyor_surface` | 0xEA324 | `0x8016F8E4` | não detectado na janela | `j       item_apply_conveyor_surface` | `decomp/src/it/itprocess.c:92` `itProcessProcItemMain` (+0x3B0) | item apply conveyor surface |

### SwordTrail.asm

Rastro de espada: setup de eixos, desenho e checagens.

Hooks de código: 7; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 638 | `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm:137` | `initial_setup_` | 0x61F40 | `0x800E6740` | não detectado na janela | `j       initial_setup_` | unmatched | initial setup |
| 639 | `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm:189` | `axis_setup_` | 0x61FF4 | `0x800E67F4` | não detectado na janela | `j       axis_setup_` | unmatched | axis setup |
| 640 | `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm:214` | `axis_setup_` | 0x61FFC | `0x800E67FC` | não detectado na janela | - | unmatched | axis setup |
| 641 | `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm:217` | `axis_setup_` | 0x62014 | `0x800E6814` | não detectado na janela | - | unmatched | axis setup |
| 642 | `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm:220` | `axis_setup_` | 0x6202C | `0x800E682C` | não detectado na janela | - | unmatched | axis setup |
| 643 | `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm:229` | `second_check_` | 0x6DD3C | `0x800F253C` | não detectado na janela | `j       second_check_` | `decomp/src/ft/ftdisplaymain.c:947` `ftDisplayMainDrawAll` (+0x9C) | second check |
| 644 | `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm:258` | `draw_trail_` | 0x6C860 | `0x800F1060` | não detectado na janela | `j       draw_trail_` | `decomp/src/ft/ftdisplaymain.c:403` `ftDisplayMainDrawAfterImage` (+0x40) | draw trail |

### TagTeam.asm

Modo Tag Team: troca de slot, roubo de stock, ícones, pickup de token e carregamento de arquivos.

Hooks de código: 25; patches de tabela: 0; origins de dados (não contados como hook): 1. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 645 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:694` | `use_character_queues_on_load_` | 0x10A2FC | `0x8018D40C` | não detectado na janela | `jal     use_character_queues_on_load_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:400` `sc1PTrainingModeCheckUpdateOptionID` (exato) | use character queues on load |
| 646 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:700` | `use_character_queues_on_load_` | 0x10A314 | `0x8018D424` | não detectado na janela | `jal     use_character_queues_on_load_._reset_alt_heap` | `decomp/src/mv/mvopening/mvopeningkirby.c:194` `mvOpeningKirbyMakeMotionCamera` (+0x8) | use character queues on load |
| 647 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:706` | `use_character_queues_on_load_` | 0x10A3E4 | `0x8018D4F4` | não detectado na janela | `j       use_character_queues_on_load_._reset_alt_heap_safeguard` | `decomp/src/sc/sccommon/scautodemo.c:360` `SCAutoDemoProcFocusPlayer2` (+0x4) | use character queues on load |
| 648 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:988` | `load_next_char_` | 0xB79A0 | `0x8013CF60` | não detectado na janela | `jal     load_next_char_` | `decomp/src/ft/ftcommon/ftcommonrebirth.c:23` `ftCommonRebirthDownSetStatus` (exato) | load next char |
| 649 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1505` | `load_all_files_` | 0x4903C | `0x800CD65C` | não detectado na janela | `j       load_all_files_` | `decomp/src/lb/lbcommon.c:3335` `lbCommonCross3D` (+0xB0) | load all files |
| 650 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1548` | `load_particle_bank_stuff_in_normal_heap` | 0x9126C | `0x80115A6C` | não detectado na janela | `jal     load_particle_bank_stuff_in_normal_heap._before_malloc` | `decomp/src/ef/efparticle.c:82` `efParticleGetLoadBankID` (+0x74) | load particle bank stuff in normal heap |
| 651 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1553` | `load_particle_bank_stuff_in_normal_heap` | 0x91298 | `0x80115A98` | não detectado na janela | `jal     load_particle_bank_stuff_in_normal_heap._after_malloc` | `decomp/src/ef/efparticle.c:82` `efParticleGetLoadBankID` (+0xA0) | load particle bank stuff in normal heap |
| 652 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1589` | `use_largest_anim_heap_` | 0x10A3B4 | `0x8018D4C4` | não detectado na janela | `j       use_largest_anim_heap_` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:555` `sc1PBonusStageUpdateTargetInterface` (exato) | use largest anim heap |
| 653 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1618` | `show_all_icons_` | 0x8B0B0 | `0x8010F8B0` | não detectado na janela | `jal     show_all_icons_._get_falls` | `decomp/src/if/ifcommon.c:1061` `ifCommonPlayerStockMultiProcDisplay` (+0x38) | show all icons |
| 654 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1624` | `show_all_icons_` | 0x8B190 | `0x8010F990` | não detectado na janela | `jal     show_all_icons_._set_icon` | `decomp/src/if/ifcommon.c:1061` `ifCommonPlayerStockMultiProcDisplay` (+0x118) | show all icons |
| 655 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1747` | `handle_stock_stealing_` | 0x8BBC0 | `0x801103C0` | não detectado na janela | `jal     handle_stock_stealing_` | `decomp/src/if/ifcommon.c:1364` `ifCommonPlayerStockStealMakeInterface` (+0x110) | handle stock stealing |
| 656 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1819` | `update_queue_` | 0x130030 | `0x80131DB0` | não detectado na janela | `j       update_queue_._select_with_token` | `decomp/src/mn/mndata/mnvsrecord.c:212` `mnVSRecordGetPowerOf` (+0x10) | update queue |
| 657 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1827` | `update_queue_` | 0x00136208 | `0x80137F88` | não detectado na janela | `j       update_queue_._costume_change` | `decomp/src/ft/ftcomputer.c:6576` `ftComputerFollowObjectiveRecover` (+0x64) | update queue |
| 658 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1834` | `update_queue_` | 0x1343E0 | `0x80136160` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayersvs.c:2622` `mnPlayersVSUpdateFighter` (+0x38) | update queue |
| 659 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1837` | `update_queue_` | 0x00134468 | `0x801361E8` | não detectado na janela | `j       update_queue_._fighter_sync` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:2420` `mnPlayers1PGameAdjustCursor` (+0x20) | update queue |
| 660 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1844` | `update_queue_` | 0x1333FC | `0x8013517C` | não detectado na janela | `jal     update_queue_._toggle_to_ffa_check` | `decomp/src/sc/sccommon/scstaffroll.c:2237` `scStaffrollFuncDraw` (+0x64) | update queue |
| 661 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1848` | `update_queue_` | 0x133438 | `0x801351B8` | não detectado na janela | `j       update_queue_._toggle_to_ffa` | `decomp/src/sc/sccommon/scstaffroll.c:2327` `scStaffrollStartScene` (exato) | update queue |
| 662 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1854` | `update_queue_` | 0x133494 | `0x80135214` | não detectado na janela | `jal     update_queue_._toggle_to_teams_check` | `decomp/src/mn/mnvsmode/mnvsresults.c:1896` `mnVSResultsMakeWallpaperTint2` (+0x10) | update queue |
| 663 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1858` | `update_queue_` | 0x1334B0 | `0x80135230` | não detectado na janela | `jal     0x800E9248` | `decomp/src/mn/mnvsmode/mnvsresults.c:1896` `mnVSResultsMakeWallpaperTint2` (+0x2C) | update queue |
| 664 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1869` | `update_queue_` | 0x1339B4 | `0x80135734` | não detectado na janela | `jal     update_queue_._toggle_team_check` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:2021` `mnPlayers1PGameCheckPuckInRange` (+0x10) | update queue |
| 665 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:1873` | `update_queue_` | 0x1339DC | `0x8013575C` | não detectado na janela | `j       update_queue_._toggle_team` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2123` `mnPlayers1PBonusBackTo1PMode` (+0x1C) | update queue |
| 666 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:2365` | `move_to_next_slot_on_pickup` | 0x1358BC | `0x8013763C` | não detectado na janela | `jal     move_to_next_slot_on_pickup` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:3041` `mnPlayers1PTrainingPauseSlotProcesses` (+0x4) | move to next slot on pickup |
| 667 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:2400` | `clear_slot_and_go_to_previous_slot_` | 0x136920 | `0x801386A0` | não detectado na janela | `j       clear_slot_and_go_to_previous_slot_` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:3720` `mnPlayers1PGameStartScene` (+0x148) | clear slot and go to previous slot |
| 668 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:2512` | `adjust_current_slot_on_stock_count_change_` | 0x132418 | `0x80134198` | não detectado na janela | `j       adjust_current_slot_on_stock_count_change_` | `decomp/src/mn/mnplayers/mnplayersvs.c:1347` `mnPlayersVSMakeStockSelect` (exato) | adjust current slot on stock count change |
| 669 | `.toolchain/pkgs/smashremix-src/src/TagTeam.asm:2713` | `speed_up_token_pickup_` | 0x135488 | `0x80137208` | não detectado na janela | `jal     speed_up_token_pickup_` | `decomp/src/mn/mnvsmode/mnvsresults.c:2782` `mnVSResultsSetTeamPlaceAll` (+0x50) | speed up token pickup |

### Teams.asm

Modo Teams: setup de time, extensões da tela de resultados e correções de figurino clássico.

Hooks de código: 32; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 670 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:118` | `Teams` | 0xB615C | `0x8013B71C` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayersvs.c:5569` `mnPlayersVSStartScene` (+0x3E0) | Teams |
| 671 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:128` | `Teams` | TEAM_BUTTON_TABLE_ORIGIN + (current_team_index * 4) | `team_button_table + (current_team_index * 4` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 672 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:132` | `Teams` | TEAM_PANEL_TABLE_ORIGIN + (current_team_index * 4) | `team_panel_table + (current_team_index * 4` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 673 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:136` | `Teams` | TEAM_PORT_COLOR_TABLE_ORIGIN + current_team_index | `team_port_color_table + current_team_index` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 674 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:143` | `Teams` | TEAM_NAME_TABLE_ORIGIN + (current_team_index * 4) | `team_name_table + (current_team_index * 4` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 675 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:147` | `Teams` | TEAM_NAME_X_POSITION_TABLE_ORIGIN + (current_team_index * 4) | `team_name_x_position_table + (current_team_index * 4` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 676 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:151` | `Teams` | TEAM_NAME_SCALE_TABLE_ORIGIN + (current_team_index * 4) | `team_name_scale_table + (current_team_index * 4` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 677 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:155` | `Teams` | TEAM_NAME_PALETTE_INDEX_TABLE_ORIGIN + current_team_index | `team_name_palette_index_table + current_team_index` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 678 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:159` | `Teams` | BIG_TEXT_PALETTE_TABLE_ORIGIN + ((current_team_index + 2) * 6) | `big_text_palette_table + ((current_team_index + 2` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 679 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:168` | `Teams` | TEAM_NAME_FGM_TABLE_ORIGIN + (current_team_index * 4) | `team_name_fgm_table + (current_team_index * 4` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 680 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:185` | `Teams` | Teams.NEW_COSTUME_TABLE_ORIGIN + (Teams.NUM_NEW_TEAMS * Character.id.{char}) + {t} | `Teams.new_costume_table + (Teams.NUM_NEW_TEAMS * Character.id.{char}` | não detectado na janela | - | n/a (endereço por expressão) | Teams |
| 681 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:247` | `use_extended_team_table_` | 0x130A38 | `0x801327B8` | não detectado na janela | `jal     use_extended_team_table_` | `decomp/src/mn/mndata/mnvsrecord.c:568` `mnVSRecordMakeLabels` (exato) | use extended team table |
| 682 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:262` | `use_extended_panel_table_` | 0x133990 | `0x80135710` | não detectado na janela | `jal     use_extended_panel_table_` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:2112` `mnPlayers1PTrainingRecallPuck` (+0x40) | use extended panel table |
| 683 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:270` | `use_extended_panel_table_` | 0x131AB4 | `0x80133834` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsresults.c:1023` `mnVSResultsGetSpot` (+0x24) | use extended panel table |
| 684 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:278` | `use_extended_panel_table_` | 0x13346C | `0x801351EC` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1837` `mnPlayers1PGameMakeFighterCamera` (+0x20) | use extended panel table |
| 685 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:285` | `use_extended_panel_table_` | 0x1334C8 | `0x80135248` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsresults.c:1896` `mnVSResultsMakeWallpaperTint2` (+0x44) | use extended panel table |
| 686 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:290` | `use_extended_panel_table_` | 0x134158 | `0x80135ED8` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2397` `mnPlayers1PBonusMakePuckCamera` (+0x90) | use extended panel table |
| 687 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:298` | `use_extended_panel_table_` | 0x134038 | `0x80135DB8` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsresults.c:2184` `mnVSResultsGetPoints` (exato) | use extended panel table |
| 688 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:313` | `extend_button_click_` | 0x133948 | `0x801356C8` | não detectado na janela | `jal     extend_button_click_` | `decomp/src/mn/mnvsmode/mnvsresults.c:2032` `mnVSResultsMakeKOs` (+0x2C) | extend button click |
| 689 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:328` | `get_team_costume_` | 0x67904 | `0x800EC104` | não detectado na janela | `j       get_team_costume_` | `decomp/src/ft/ftparam.c:2903` `ftParamGetCostumeTeamID` (exato) | get team costume |
| 690 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:362` | `classic_sonic_costume_fix_` | 0x1334A4 | `0x80135224` | não detectado na janela | `j       classic_sonic_costume_fix_._team_battle_toggle` | `decomp/src/mn/mnvsmode/mnvsresults.c:1896` `mnVSResultsMakeWallpaperTint2` (+0x20) | classic sonic costume fix |
| 691 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:370` | `classic_sonic_costume_fix_` | 0x1339C4 | `0x80135744` | não detectado na janela | `j       classic_sonic_costume_fix_._team_button_toggle` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2123` `mnPlayers1PBonusBackTo1PMode` (+0x4) | classic sonic costume fix |
| 692 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:378` | `classic_sonic_costume_fix_` | 0x132B54 | `0x801348D4` | não detectado na janela | `j       classic_sonic_costume_fix_._cpu_select` | `decomp/src/mn/mndata/mnvsrecord.c:1455` `mnVSRecordGetAvg` (+0x20) | classic sonic costume fix |
| 693 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:462` | `classic_sonic_costume_fix_` | 0x138920 | `0x8013A6A0` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayersvs.c:4865` `mnPlayersVSSetSceneData` (+0x3C) | classic sonic costume fix |
| 694 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:468` | `classic_sonic_costume_fix_` | 0x153624 | `0x80134484` | não detectado na janela | `jal     0x80132A2C` | `decomp/src/mn/mnvsmode/mnvsresults.c:1497` `mnVSResultMakeTeamName` (+0x4) | classic sonic costume fix |
| 695 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:489` | `extend_results_screen_series_logo_color_index_` | 0x151DE4 | `0x80132C44` | não detectado na janela | `jal     extend_results_screen_series_logo_color_index_` | `decomp/src/sc/sc1pmode/sc1pintro.c:902` `sc1PIntroGetFighterPositionZ` (exato) | extend results screen series logo color index |
| 696 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:503` | `extend_results_screen_background_palette_index_` | 0x1521FC | `0x8013305C` | não detectado na janela | `jal     extend_results_screen_background_palette_index_` | `decomp/src/sc/sc1pmode/sc1pstageclear.c:1267` `sc1PStageClearCheckHaveBonusStats` (exato) | extend results screen background palette index |
| 697 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:518` | `extend_results_team_name_palette_` | 0x1536B8 | `0x80134518` | não detectado na janela | `jal     extend_results_team_name_palette_._index` | `decomp/src/mn/mnvsmode/mnvsoptions.c:1706` `mnVSOptionsFuncStart` (+0x14) | extend results team name palette |
| 698 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:523` | `extend_results_team_name_palette_` | 0x153300 | `0x80134160` | não detectado na janela | `jal     extend_results_team_name_palette_._palette` | `decomp/src/mn/mncommon/mntitlefiles.c:35` `mnTitleLoadFiles` (+0x20) | extend results team name palette |
| 699 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:543` | `extend_results_screen_column_color_index_` | 0x1545F4 | `0x80135454` | não detectado na janela | `j       extend_results_screen_column_color_index_` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:2038` `mnPlayers1PTrainingUpdateCursorNoRecall` (+0x24) | extend results screen column color index |
| 700 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:557` | `extend_results_screen_announcer_` | 0x1511BC | `0x8013201C` | não detectado na janela | `jal     extend_results_screen_announcer_` | `decomp/src/mn/mncommon/mncongra.c:438` `mnCongraStartScene` (+0x10) | extend results screen announcer |
| 701 | `.toolchain/pkgs/smashremix-src/src/Teams.asm:572` | `extend_results_screen_announcer_` | 0x1563E0 | `0x80137240` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayersvs.c:3204` `mnPlayersVSUpdateCursorGrabPriorities` (+0xC) | extend results screen announcer |

### TimedStock.asm

Modo Timed Stock: pontuação, prevenção de morte súbita e resultados completos.

Hooks de código: 8; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 702 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:20` | `TimedStock` | 0x001241A0 | `0x801337F0` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsoptions.c:1315` `mnVSOptionsMakeDecalCamera` (+0x18) | TimedStock |
| 703 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:26` | `TimedStock` | 0x001241B4 | `0x80133804` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1046` `mnPlayers1PGameMakeTimeSelect` (exato) | TimedStock |
| 704 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:33` | `TimedStock` | 0x001389F8 | `0x8013A778` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayersvs.c:4865` `mnPlayersVSSetSceneData` (+0x114) | TimedStock |
| 705 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:41` | `TimedStock` | 0x00156560 | `0x801373C0` | não detectado na janela | - | `decomp/src/mn/mnvsmode/mnvsresults.c:2842` `mnVSResultsInitRankings` (+0xC) | TimedStock |
| 706 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:59` | `score_fix_` | 0x155DA8 | `0x80136C08` | não detectado na janela | `j       score_fix_` | `decomp/src/mn/mnvsmode/mnvsresults.c:2669` `mnVSResultsGetPointsDirect` (exato) | score fix |
| 707 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:151` | `pick_scoring_` | 0x156564 | `0x801373C4` | não detectado na janela | `j   pick_scoring_` | `decomp/src/mn/mnvsmode/mnvsresults.c:2842` `mnVSResultsInitRankings` (+0x10) | pick scoring |
| 708 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:174` | `prevent_sudden_death_` | 0x10A4F4 | `0x8018D604` | não detectado na janela | `j       prevent_sudden_death_` | `decomp/src/mv/mvopening/mvopeningdonkey.c:265` `mvOpeningDonkeyMakeMotionWindow` (exato) | prevent sudden death |
| 709 | `.toolchain/pkgs/smashremix-src/src/TimedStock.asm:215` | `show_full_results_` | 0x156884 | `0x801376E4` | não detectado na janela | `j       show_full_results_` | `decomp/src/mn/mnplayers/mnplayersvs.c:3325` `mnPlayersVSCheckCursorPuckGrab` (+0x14) | show full results |

### timeouts.asm

Timeouts: hooks de branch em rotinas de tempo limite.

Hooks de código: 11; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 710 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:17` | `Timeouts` | 0x0011D5B0 | `0x80132620` | não detectado na janela | `b 0x80132640` | `decomp/src/mn/mnoption/mnoption.c:653` `func_ovl60_80132618` (+0x8) | Timeouts |
| 711 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:22` | `Timeouts` | 0x0011EB1C | `0x80132A0C` | não detectado na janela | `b 0x80132A2C` | `decomp/src/mn/mncommon/mnmodeselect.c:963` `mnModeSelectFuncStart` (exato) | Timeouts |
| 712 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:27` | `Timeouts` | 0x00120658 | `0x80132EA8` | não detectado na janela | `b 0x80132ED0` | `decomp/src/mn/mnvsmode/mnvsresults.c:752` `mnVSResultsMakeWallpaper` (exato) | Timeouts |
| 713 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:32` | `Timeouts` | 0x00121D20 | `0x801328D0` | não detectado na janela | `b 0x801328F0` | `decomp/src/mv/mvending/mvending.c:564` `mvEndingFuncStart` (exato) | Timeouts |
| 714 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:37` | `Timeouts` | 0x001245A0 | `0x80133BF0` | não detectado na janela | `b 0x80133C18` | `decomp/src/sc/sc1pmode/sc1pstageclear.c:1633` `sc1PStageClearMakeWallpaperCamera` (+0x8) | Timeouts |
| 715 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:42` | `Timeouts` | 0x00127284 | `0x80133AA4` | não detectado na janela | `b 0x80133AD4` | `decomp/src/mn/mncommon/mntitle.c:1407` `mnTitleMakeCameras` (+0x10) | Timeouts |
| 716 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:47` | `Timeouts` | 0x00138BDC | `0x8013A95C` | não detectado na janela | `b 0x8013A984` | `decomp/src/mn/mnplayers/mnplayersvs.c:5013` `mnPlayersVSFuncRun` (+0x3C) | Timeouts |
| 717 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:52` | `Timeouts` | 0x001401F4 | `0x80137FF4` | não detectado na janela | `b 0x80138024` | `decomp/src/ft/ftcomputer.c:6621` `ftComputerFollowObjectiveCounterAttack` (+0x20) | Timeouts |
| 718 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:57` | `Timeouts` | 0x00146D08 | `0x80137728` | não detectado na janela | `b 0x80137758` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:3058` `mnPlayers1PTrainingFuncRun` (+0x28) | Timeouts |
| 719 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:62` | `Timeouts` | 0x0014CA28 | `0x801369F8` | não detectado na janela | `b 0x80136A28` | `decomp/src/mn/mnplayers/mnplayersvs.c:2984` `mnPlayersVSArrowThreadUpdate` (+0x14) | Timeouts |
| 720 | `.toolchain/pkgs/smashremix-src/src/timeouts.asm:67` | `Timeouts` | 0x0014F928 | `0x80133DB8` | não detectado na janela | `b 0x80133DE0` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1219` `mnPlayers1PGameMakeLevel` (+0x1C) | Timeouts |

### Training.asm

Modo treino: menus de entrada/saída, reset, desabilitar spawn de itens, correções de crash Pokémon e setor Z.

Hooks de código: 32; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 721 | `.toolchain/pkgs/smashremix-src/src/Training.asm:142` | `load_character_` | 0x00116AA0 | `0x80190280` | não detectado na janela | `jal load_character_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1958` `sc1PTrainingModeFuncStart` (+0x20) | load character |
| 722 | `.toolchain/pkgs/smashremix-src/src/Training.asm:400` | `init_character_` | 0x0005321C | `0x800D7A1C` | não detectado na janela | `j       init_character_` | `decomp/src/ft/ftmanager.c:477` `ftManagerInitFighter` (+0x2C) | init character |
| 723 | `.toolchain/pkgs/smashremix-src/src/Training.asm:468` | `set_action_` | 0x53F34 | `0x800D8734` | não detectado na janela | `j       set_action_` | unmatched | set action |
| 724 | `.toolchain/pkgs/smashremix-src/src/Training.asm:507` | `obey_cp_state_` | 0x116A14 | `0x801901F4` | não detectado na janela | `j       obey_cp_state_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1946` `sc1PTrainingModeUpdateDummyBehavior` (exato) | obey cp state |
| 725 | `.toolchain/pkgs/smashremix-src/src/Training.asm:550` | `disable_during_pause_` | 0x11398C | `0x8018D16C` | não detectado na janela | `jal     0x800E7F14` | `decomp/src/mv/mvopening/mvopeningjungle.c:222` `mvOpeningJungleMakeGroundViewport` (+0x4) | disable during pause |
| 726 | `.toolchain/pkgs/smashremix-src/src/Training.asm:581` | `enable_during_unpause_` | 0x113A7C | `0x8018D25C` | não detectado na janela | `jal     0x800E7F68` | `decomp/src/sc/sccommon/scexplain.c:215` `scExplainMakeCamera` (+0x14) | enable during unpause |
| 727 | `.toolchain/pkgs/smashremix-src/src/Training.asm:627` | `check_enter_menu_` | 0x113930 | `0x8018D110` | não detectado na janela | `j       check_enter_menu_` | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:365` `sc1PBonusStageInitVars` (+0x20) | check enter menu |
| 728 | `.toolchain/pkgs/smashremix-src/src/Training.asm:674` | `check_leave_menu_` | 0x113A38 | `0x8018D218` | não detectado na janela | `j       check_leave_menu_` | `decomp/src/sc/sc1pmode/sc1pgame.c:832` `sc1PGameFuncUpdate` (+0x18) | check leave menu |
| 729 | `.toolchain/pkgs/smashremix-src/src/Training.asm:720` | `check_disabled_item_spawn_` | 0x113DCC | `0x8018D5AC` | não detectado na janela | `j       check_disabled_item_spawn_` | `decomp/src/sc/sccommon/scautodemo.c:369` `scAutoDemoResetFocusPlayerAll` (+0x84) | check disabled item spawn |
| 730 | `.toolchain/pkgs/smashremix-src/src/Training.asm:755` | `hold_to_exit_` | 0x114078 | `0x8018D858` | não detectado na janela | `jal     hold_to_exit_` | `decomp/src/mv/mvopening/mvopeningfox.c:291` `mvOpeningFoxPosedWallpaperProcDisplay` (+0xC) | hold to exit |
| 731 | `.toolchain/pkgs/smashremix-src/src/Training.asm:760` | `hold_to_exit_` | 0x11414C | `0x8018D92C` | não detectado na janela | `jal     hold_to_exit_._reset` | `decomp/src/mv/mvopening/mvopeninglink.c:297` `mvOpeningLinkMakePosedWallpaper` (+0x8) | hold to exit |
| 732 | `.toolchain/pkgs/smashremix-src/src/Training.asm:814` | `load_from_sss_` | 0x00116E20 | `0x80190600` | não detectado na janela | `j   load_from_sss_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:2051` `sc1PTrainingModeStartScene` (+0xC) | load from sss |
| 733 | `.toolchain/pkgs/smashremix-src/src/Training.asm:866` | `load_from_reset_` | 0x00116E88 | `0x80190668` | não detectado na janela | `j   load_from_reset_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:2051` `sc1PTrainingModeStartScene` (+0x74) | load from reset |
| 734 | `.toolchain/pkgs/smashremix-src/src/Training.asm:951` | `disable_hud_` | 0x116D70 | `0x80190550` | não detectado na janela | `j       disable_hud_._init` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1958` `sc1PTrainingModeFuncStart` (+0x2F0) | disable hud |
| 735 | `.toolchain/pkgs/smashremix-src/src/Training.asm:957` | `disable_hud_` | 0x113A4C | `0x8018D22C` | não detectado na janela | `j       disable_hud_._leave_menu` | `decomp/src/sc/sccommon/scvsbattle.c:253` `scVSBattleStartBattle` (+0x4) | disable hud |
| 736 | `.toolchain/pkgs/smashremix-src/src/Training.asm:1044` | `advance_frame_` | 0x00114260 | `0x8018DA40` | não detectado na janela | `jal     advance_frame_` | `decomp/src/mv/mvopening/mvopeninglink.c:347` `mvOpeningLinkMakePosedFighter` (exato) | advance frame |
| 737 | `.toolchain/pkgs/smashremix-src/src/Training.asm:1340` | `run_after_object_routines_` | 0x114270 | `0x8018DA50` | não detectado na janela | `j   run_after_object_routines_` | `decomp/src/mv/mvopening/mvopeningdonkey.c:387` `mvOpeningDonkeyMakePosedFighter` (exato) | run after object routines |
| 738 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2282` | `play_custom_music_` | 0x116994 | `0x80190174` | não detectado na janela | `jal     play_custom_music_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1925` `sc1PTrainingModeSetPlayDefaultBGM` (+0x10) | play custom music |
| 739 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2314` | `shield_break_mode_` | 0x5CC2C | `0x800E142C` | não detectado na janela | `jal     shield_break_mode_` | `decomp/src/ft/ftmain.c:1350` `ftMainProcUpdateInterrupt` (+0x1CC) | shield break mode |
| 740 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2497` | `spam_practice_` | 0xB4E10 | `0x8013A3D0` | não detectado na janela | `j       spam_practice_` | `decomp/src/mn/mnplayers/mnplayersvs.c:4740` `mnPlayersVSSetPlayerNot` (+0x24) | spam practice |
| 741 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2646` | `fix_sector_z_crashes_` | 0x82FFC | `0x801077FC` | não detectado na janela | `j       fix_sector_z_crashes_` | `decomp/src/gr/grcommon/grsector.c:847` `grSectorArwingWeaponLaser3DMakeWeapon` (+0x114) | fix sector z crashes |
| 742 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2668` | `fix_pokemon_crashes_` | 0xFD3B0 | `0x80182970` | não detectado na janela | `jal     fix_pokemon_crashes_._hitmonlee_init` | `decomp/src/it/itmonster/itsawamura.c:217` `itSawamuraAttackInitVars` (+0x18) | fix pokemon crashes |
| 743 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2672` | `fix_pokemon_crashes_` | 0xFD2A0 | `0x80182860` | não detectado na janela | `jal     fix_pokemon_crashes_._hitmonlee` | `decomp/src/it/itmonster/itsawamura.c:187` `itSawamuraAttackSetFollowPlayerLR` (+0x4) | fix pokemon crashes |
| 744 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2679` | `fix_pokemon_crashes_` | 0xFCB38 | `0x801820F8` | não detectado na janela | `jal     fix_pokemon_crashes_._starmie` | `decomp/src/it/itmonster/itstarmie.c:266` `itStarmieNFollowInitVars` (+0x2C) | fix pokemon crashes |
| 745 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2685` | `fix_pokemon_crashes_` | 0xFB614 | `0x80180BD4` | não detectado na janela | `jal     fix_pokemon_crashes_._blastoise` | `decomp/src/it/itmonster/itkamex.c:355` `itKamexCommonFindTargetsSetLR` (+0x28) | fix pokemon crashes |
| 746 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2732` | `skip_crowd_cheer_` | 0x116D68 | `0x80190548` | não detectado na janela | `jal     skip_crowd_cheer_._training` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1958` `sc1PTrainingModeFuncStart` (+0x2E8) | skip crowd cheer |
| 747 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2738` | `skip_crowd_cheer_` | 0x112FA8 | `0x8018E868` | não detectado na janela | `jal     skip_crowd_cheer_._bonus` | `decomp/src/sc/sc1pmode/sc1pgame.c:1699` `sc1PGameWaitStageTeamUpdate` (+0x44) | skip crowd cheer |
| 748 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2744` | `skip_crowd_cheer_` | 0x10E550 | `0x8018FCF0` | não detectado na janela | `jal     skip_crowd_cheer_._bonus` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:1835` `sc1PTrainingModeUnderlineProcDisplay` (+0x10) | skip crowd cheer |
| 749 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2804` | `handle_reset_menu_selections_` | 0x1144D4 | `0x8018DCB4` | não detectado na janela | `j       handle_reset_menu_selections_` | `decomp/src/sc/sccommon/scexplain.c:530` `scExplainSetPhaseSObjs` (+0x30) | handle reset menu selections |
| 750 | `.toolchain/pkgs/smashremix-src/src/Training.asm:2811` | `handle_reset_menu_selections_` | 0x1144E4 | `0x8018DCC4` | não detectado na janela | - | `decomp/src/sc/sc1pmode/sc1pbonusstage.c:870` `sc1PBonusStageMakeInterface` (exato) | handle reset menu selections |
| 751 | `.toolchain/pkgs/smashremix-src/src/Training.asm:3863` | `show_stage_bg_` | 0x803F4 | `0x80104BF4` | não detectado na janela | `j       show_stage_bg_` | `decomp/src/gr/grwallpaper.c:360` `grWallpaperMakeDecideKind` (+0x18) | show stage bg |
| 752 | `.toolchain/pkgs/smashremix-src/src/Training.asm:3869` | `show_stage_bg_` | 0x1146C4 | `0x8018DEA4` | não detectado na janela | `jal     show_stage_bg_._magnifying_glass` | `decomp/src/sc/sccommon/scexplain.c:599` `scExplainHideTapSpark` (+0x4) | show stage bg |

### Tripping.asm

Tripping: retenção de hitstun ao virar e checagem de tropeço.

Hooks de código: 6; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 753 | `.toolchain/pkgs/smashremix-src/src/Tripping.asm:14` | `tripping` | 0xB9358 | `0x8013E918` | não detectado na janela | `jal     tripping` | `decomp/src/ft/ftcommon/ftcommonturn.c:109` `ftCommonTurnSetStatus` (+0x10) | tripping |
| 754 | `.toolchain/pkgs/smashremix-src/src/Tripping.asm:19` | `tripping` | 0xB9758 | `0x8013ED18` | não detectado na janela | `jal     tripping` | `decomp/src/ft/ftcommon/ftcommondash.c:111` `ftCommonDashSetStatus` (+0x18) | tripping |
| 755 | `.toolchain/pkgs/smashremix-src/src/Tripping.asm:24` | `tripping` | 0xB9C58 | `0x8013F218` | não detectado na janela | `jal     tripping` | `decomp/src/ft/ftcommon/ftcommonturnrun.c:35` `ftCommonTurnRunSetStatus` (+0x10) | tripping |
| 756 | `.toolchain/pkgs/smashremix-src/src/Tripping.asm:112` | `trip_turn_hitstun_retention_1` | 0xB9330 | `0x8013E8F0` | não detectado na janela | `jal     trip_turn_hitstun_retention_1` | `decomp/src/ft/ftcommon/ftcommonturn.c:9` `ftCommonTurnProcUpdate` (+0x260) | trip turn hitstun retention 1 |
| 757 | `.toolchain/pkgs/smashremix-src/src/Tripping.asm:138` | `trip_turn_hitstun_retention_2` | 0xB9388 | `0x8013E948` | não detectado na janela | `jal     trip_turn_hitstun_retention_2` | `decomp/src/ft/ftcommon/ftcommonturn.c:109` `ftCommonTurnSetStatus` (+0x40) | trip turn hitstun retention 2 |
| 758 | `.toolchain/pkgs/smashremix-src/src/Tripping.asm:164` | `damaged_check_trip` | 0xBAF28 | `0x801404E8` | não detectado na janela | `jal     damaged_check_trip` | `decomp/src/ft/ftcommon/ftcommondamage.c:103` `ftCommonDamageCommonProcUpdate` (+0x8) | damaged check trip |

### TugOfWar.asm

Tug of War: crédito de stock para o Koer e última vez que foi atingido.

Hooks de código: 4; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 759 | `.toolchain/pkgs/smashremix-src/src/TugOfWar.asm:283` | `credit_stock_to_koer_` | 0xB6884 | `0x8013BE44` | não detectado na janela | `jal     credit_stock_to_koer_` | `decomp/src/ft/ftcommon/ftcommondead.c:48` `ftCommonDeadUpdateScore` (+0xE0) | credit stock to koer |
| 760 | `.toolchain/pkgs/smashremix-src/src/TugOfWar.asm:365` | `credit_stock_when_no_koer_` | 0xB68C4 | `0x8013BE84` | não detectado na janela | `jal     credit_stock_when_no_koer_` | `decomp/src/ft/ftcommon/ftcommondead.c:48` `ftCommonDeadUpdateScore` (+0x120) | credit stock when no koer |
| 761 | `.toolchain/pkgs/smashremix-src/src/TugOfWar.asm:434` | `update_last_hit_by_` | 0xBBF74 | `0x80141534` | não detectado na janela | `jal     update_last_hit_by_._damage` | unmatched | update last hit by |
| 762 | `.toolchain/pkgs/smashremix-src/src/TugOfWar.asm:439` | `update_last_hit_by_` | 0xC3FC4 | `0x80149584` | não detectado na janela | `jal     update_last_hit_by_._shield` | `decomp/src/ft/ftcommon/ftcommonshieldbreakfly.c:42` `ftCommonShieldBreakFlyCommonSetStatus` (+0x74) | update last hit by |

### TwelveCharBattle.asm

12 Char Battle: mudança de stock, FFA, portas 3/4, portrait e prevenção de seleção de personagem derrotado.

Hooks de código: 46; patches de tabela: 0; origins de dados (não contados como hook): 2. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 763 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:453` | `setup_ports_and_characters_` | 0x138F3C | `0x8013ACBC` | não detectado na janela | `j       setup_ports_and_characters_` | `decomp/src/mn/mnplayers/mnplayersvs.c:5219` `mnPlayersVSInitPlayer` (+0x40) | setup ports and characters |
| 764 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:593` | `ignore_ports_3_and_4_` | 0x138560 | `0x8013A2E0` | não detectado na janela | `j       ignore_ports_3_and_4_` | `decomp/src/mn/mnplayers/mnplayersvs.c:4701` `mnPlayersVSUpdateControllerOrders` (+0x3C) | ignore ports 3 and 4 |
| 765 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:620` | `skip_rendering_panels_3_and_4_` | 0x139328 | `0x8013B0A8` | não detectado na janela | `jal     skip_rendering_panels_3_and_4_._panel` | `decomp/src/mn/mnplayers/mnplayersvs.c:5429` `mnPlayersVSInitSlotAll` (+0x18) | skip rendering panels 3 and 4 |
| 766 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:627` | `skip_rendering_panels_3_and_4_` | 0x134A34 | `0x801367B4` | não detectado na janela | `jal     skip_rendering_panels_3_and_4_._button_interaction` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:2563` `mnPlayers1PGameCheckLevelArrowLInRange` (+0x2C) | skip rendering panels 3 and 4 |
| 767 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:632` | `skip_rendering_panels_3_and_4_` | 0x137938 | `0x801396B8` | não detectado na janela | `jal     skip_rendering_panels_3_and_4_._token_check` | `decomp/src/mn/mnplayers/mnplayersvs.c:4295` `mnPlayersVSPuckAdjustPlaced` (+0x9C) | skip rendering panels 3 and 4 |
| 768 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:696` | `change_2p_panel_position_` | 0x131A14 | `0x80133794` | não detectado na janela | `jal     change_2p_panel_position_._panel` | `decomp/src/mn/mncommon/mntitle.c:1349` `mnTitleFireCameraProcUpdate` (+0x24) | change 2p panel position |
| 769 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:701` | `change_2p_panel_position_` | 0x131880 | `0x80133600` | não detectado na janela | `jal     change_2p_panel_position_._2p_image` | `decomp/src/mn/mndata/mnvsrecord.c:975` `mnVSRecordMakeColumnIcons` (+0x4) | change 2p panel position |
| 770 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:706` | `change_2p_panel_position_` | 0x131838 | `0x801335B8` | não detectado na janela | `jal     change_2p_panel_position_._cp_image` | `decomp/src/mn/mndata/mnvsrecord.c:954` `mnVSRecordMakeLockedIcon` (+0x18) | change 2p panel position |
| 771 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:711` | `change_2p_panel_position_` | 0x13504C | `0x80136DCC` | não detectado na janela | `jal     change_2p_panel_position_._cp_level` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:2770` `mnPlayers1PGameCursorProcUpdate` (+0x74) | change 2p panel position |
| 772 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:716` | `change_2p_panel_position_` | 0x135238 | `0x80136FB8` | não detectado na janela | `jal     change_2p_panel_position_._cp_handicap_value` | `decomp/src/mn/mnvsmode/mnvsresults.c:2737` `mnVSResultsGetTeamTotalPoints` (exato) | change 2p panel position |
| 773 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:721` | `change_2p_panel_position_` | 0x134D68 | `0x80136AE8` | não detectado na janela | `jal     change_2p_panel_position_._cp_handicap_left` | `decomp/src/mn/mnvsmode/mnvsresults.c:2630` `mnVSResultsSetKOs` (+0x4C) | change 2p panel position |
| 774 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:726` | `change_2p_panel_position_` | 0x134DF8 | `0x80136B78` | não detectado na janela | `jal     change_2p_panel_position_._cp_handicap_right` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2887` `mnPlayers1PBonusInitVars` (+0x24) | change 2p panel position |
| 775 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:731` | `change_2p_panel_position_` | 0x135000 | `0x80136D80` | não detectado na janela | `jal     change_2p_panel_position_._handicap` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:2770` `mnPlayers1PGameCursorProcUpdate` (+0x28) | change 2p panel position |
| 776 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:736` | `change_2p_panel_position_` | 0x1316D0 | `0x80133450` | não detectado na janela | `jal     change_2p_panel_position_._hmn_button` | `decomp/src/mn/mndata/mnvsrecord.c:909` `mnVSRecordSetIconPositionForColumn` (+0x18) | change 2p panel position |
| 777 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:741` | `change_2p_panel_position_` | 0x130C58 | `0x801329D8` | não detectado na janela | `jal     change_2p_panel_position_._hmn_button_pressed` | `decomp/src/mn/mn1pmode/mn1pcontinue.c:579` `mnPlayers1PGameContinueMakeOptions` (+0x8) | change 2p panel position |
| 778 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:746` | `change_2p_panel_position_` | 0x130EA4 | `0x80132C24` | não detectado na janela | `j       change_2p_panel_position_._sliding_doors` | `decomp/src/mn/mnplayers/mnplayers1ptraining.c:698` `mnPlayers1PTrainingSetGateLUT` (exato) | change 2p panel position |
| 779 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:752` | `change_2p_panel_position_` | 0x13194C | `0x801336CC` | não detectado na janela | `jal     change_2p_panel_position_._sliding_doors_render` | `decomp/src/sc/sc1pmode/sc1pintro.c:1262` `sc1PIntroGetStageCObjDesc` (exato) | change 2p panel position |
| 780 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:757` | `change_2p_panel_position_` | 0x1370C0 | `0x80138E40` | não detectado na janela | `jal     change_2p_panel_position_._cursor` | `decomp/src/mn/mnplayers/mnplayersvs.c:4099` `mnPlayersVSMakeCursor` (+0x54) | change 2p panel position |
| 781 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:762` | `change_2p_panel_position_` | 0x132E2C | `0x80134BAC` | não detectado na janela | `jal     change_2p_panel_position_._character` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:1609` `mnPlayers1PGameGetTotalHiScore` (+0x48) | change 2p panel position |
| 782 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:767` | `change_2p_panel_position_` | 0x130D98 | `0x80132B18` | não detectado na janela | `jal     change_2p_panel_position_._name_and_logo` | `decomp/src/sc/sc1pmode/sc1pintro.c:849` `sc1PIntroMakeNameAll` (+0x8) | change 2p panel position |
| 783 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:772` | `change_2p_panel_position_` | 0x138004 | `0x80139D84` | não detectado na janela | `jal     change_2p_panel_position_._white_circle` | `decomp/src/ft/ftcomputer.c:7462` `ftComputerFollowObjectivePatrol` (+0x18) | change 2p panel position |
| 784 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:777` | `change_2p_panel_position_` | 0x133D44 | `0x80135AC4` | não detectado na janela | `j       change_2p_panel_position_._hmn_button_press` | `decomp/src/mn/mnplayers/mnplayersvs.c:2403` `mnPlayersVSCheckPlayerKindSelectInRange` (+0xC) | change 2p panel position |
| 785 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:783` | `change_2p_panel_position_` | 0x133B84 | `0x80135904` | não detectado na janela | `j       change_2p_panel_position_._right_arrow_press` | `decomp/src/mn/mnplayers/mnplayersvs.c:2349` `mnPlayersVSCheckHandicapArrowRInRange` (+0xC) | change 2p panel position |
| 786 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:789` | `change_2p_panel_position_` | 0x133C64 | `0x801359E4` | não detectado na janela | `j       change_2p_panel_position_._left_arrow_press` | `decomp/src/mn/mnplayers/mnplayersvs.c:2376` `mnPlayersVSCheckHandicapArrowLInRange` (+0xC) | change 2p panel position |
| 787 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:1665` | `force_ffa_and_stock_` | 0x1391A0 | `0x8013AF20` | não detectado na janela | `j       force_ffa_and_stock_` | `decomp/src/mn/mnplayers/mnplayersvs.c:5327` `mnPlayersVSInitVars` (+0x58) | force ffa and stock |
| 788 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:1748` | `update_announcer_on_entry_` | 0x139588 | `0x8013B308` | não detectado na janela | `jal     update_announcer_on_entry_` | `decomp/src/mn/mnplayers/mnplayersvs.c:5438` `mnPlayersVSFuncStart` (+0x240) | update announcer on entry |
| 789 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:1759` | `update_announcer_on_entry_` | 0x138BD0 | `0x8013A950` | não detectado na janela | `jal     update_announcer_on_entry_._wins` | `decomp/src/mn/mnplayers/mnplayersvs.c:5013` `mnPlayersVSFuncRun` (+0x30) | update announcer on entry |
| 790 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:1867` | `update_css_header_` | 0x1326B0 | `0x80134430` | não detectado na janela | `jal     update_css_header_._vs` | `decomp/src/sc/sc1pmode/sc1pintro.c:1871` `sc1PIntroMakeDecalsCamera` (+0x4) | update css header |
| 791 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:1872` | `update_css_header_` | 0x155928 | `0x80136788` | não detectado na janela | `jal     update_css_header_._results` | `decomp/src/mn/mnplayers/mnplayers1pgame.c:2563` `mnPlayers1PGameCheckLevelArrowLInRange` (exato) | update css header |
| 792 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:1958` | `handle_stock_count_change_` | 0x1366D0 | `0x80138450` | não detectado na janela | `jal     handle_stock_count_change_._decrement` | `decomp/src/ft/ftcomputer.c:6752` `func_ovl3_8013837C` (+0xD4) | handle stock count change |
| 793 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:1963` | `handle_stock_count_change_` | 0x136648 | `0x801383C8` | não detectado na janela | `jal     handle_stock_count_change_._increment` | `decomp/src/ft/ftcomputer.c:6752` `func_ovl3_8013837C` (+0x4C) | handle stock count change |
| 794 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:2090` | `handle_stock_change_fgm_` | 0x1366F8 | `0x80138478` | não detectado na janela | `jal     handle_stock_change_fgm_` | `decomp/src/ft/ftcomputer.c:6752` `func_ovl3_8013837C` (+0xFC) | handle stock change fgm |
| 795 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:2095` | `handle_stock_change_fgm_` | 0x136670 | `0x801383F0` | não detectado na janela | `jal     handle_stock_change_fgm_` | `decomp/src/ft/ftcomputer.c:6752` `func_ovl3_8013837C` (+0x74) | handle stock change fgm |
| 796 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:2654` | `handle_custom_presses_` | 0x136708 | `0x80138488` | não detectado na janela | `jal     handle_custom_presses_` | `decomp/src/ft/ftcomputer.c:6752` `func_ovl3_8013837C` (+0x10C) | handle custom presses |
| 797 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:2861` | `prevent_defeated_char_select_` | 0x13546C | `0x801371EC` | não detectado na janela | `jal     prevent_defeated_char_select_` | `decomp/src/mn/mnvsmode/mnvsresults.c:2782` `mnVSResultsSetTeamPlaceAll` (+0x34) | prevent defeated char select |
| 798 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:2936` | `defeated_action_override_` | 0x62D58 | `0x800E7558` | não detectado na janela | `jal     defeated_action_override_` | unmatched | defeated action override |
| 799 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:3303` | `prevent_token_pickup_` | 0x13588C | `0x8013760C` | não detectado na janela | `j       prevent_token_pickup_` | `decomp/src/mn/mnplayers/mnplayersvs.c:3300` `mnPlayersVSSetCursorGrab` (exato) | prevent token pickup |
| 800 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:3381` | `set_initial_stock_count_` | 0x10A39C | `0x8018D4AC` | não detectado na janela | `jal     set_initial_stock_count_` | `decomp/src/sc/sc1pmode/sc1ptrainingmode.c:422` `sc1PTrainingModeUpdateCPOption` (+0x34) | set initial stock count |
| 801 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:3425` | `treat_selected_defeated_chars_as_unselected_` | 0x1385B8 | `0x8013A338` | não detectado na janela | `j       treat_selected_defeated_chars_as_unselected_._p1` | `decomp/src/mn/mnplayers/mnplayersvs.c:4724` `mnPlayersVSGetReadyPlayerCount` (+0x2C) | treat selected defeated chars as unselected |
| 802 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:3431` | `treat_selected_defeated_chars_as_unselected_` | 0x1385E0 | `0x8013A360` | não detectado na janela | `j       treat_selected_defeated_chars_as_unselected_._p2` | `decomp/src/mn/mnplayers/mnplayersvs.c:4724` `mnPlayersVSGetReadyPlayerCount` (+0x54) | treat selected defeated chars as unselected |
| 803 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:3525` | `gentlemens_reset_` | 0x8FCD0 | `0x801144D0` | não detectado na janela | `jal     gentlemens_reset_` | `decomp/src/sys/objdisplay.c:2172` `gcDrawDObjDLLinksForGObj` (+0xD4) | gentlemens reset |
| 804 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:3530` | `gentlemens_reset_` | 0x8FC90 | `0x80114490` | não detectado na janela | `jal     gentlemens_reset_._unpause` | `decomp/src/sys/objdisplay.c:2172` `gcDrawDObjDLLinksForGObj` (+0x94) | gentlemens reset |
| 805 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:3535` | `gentlemens_reset_` | 0x8F87C | `0x8011407C` | não detectado na janela | `jal     gentlemens_reset_._pause_press_check` | `decomp/src/if/ifcommon.c:3127` `ifCommonBattleGoUpdateInterface` (+0x40) | gentlemens reset |
| 806 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:4536` | `portrait_change_` | 0x136DC0 | `0x80138B40` | não detectado na janela | `j       portrait_change_` | `decomp/src/ft/ftcomputer.c:6926` `func_ovl3_80138AA8` (+0x98) | portrait change |
| 807 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:5350` | `update_stocks_remaining_` | 0xB6930 | `0x8013BEF0` | não detectado na janela | `jal     update_stocks_remaining_` | `decomp/src/ft/ftcommon/ftcommondead.c:48` `ftCommonDeadUpdateScore` (+0x18C) | update stocks remaining |
| 808 | `.toolchain/pkgs/smashremix-src/src/TwelveCharBattle.asm:5354` | `update_stocks_remaining_` | 0xB6988 | `0x8013BF48` | não detectado na janela | `jal     update_stocks_remaining_` | `decomp/src/ft/ftcommon/ftcommondead.c:48` `ftCommonDeadUpdateScore` (+0x1E4) | update stocks remaining |

### VanillaLv10Attacks.asm

Ataques Lv10 vanilla: patches de tabela de comportamento de IA por personagem (dado, não código).

Hooks de código: 0; patches de tabela: 12; origins de dados (não contados como hook): 0. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 809 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:45` | `Mario` | - | `tabela lv10_ai_behaviour[Character.id.MARIO] +0x4` | não detectado na janela | - | n/a (patch de dado) | Mario |
| 810 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:89` | `Link` | - | `tabela lv10_ai_behaviour[Character.id.LINK] +0x4` | não detectado na janela | - | n/a (patch de dado) | Link |
| 811 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:134` | `Dk` | - | `tabela lv10_ai_behaviour[Character.id.DK] +0x4` | não detectado na janela | - | n/a (patch de dado) | Dk |
| 812 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:181` | `Fox` | - | `tabela lv10_ai_behaviour[Character.id.FOX] +0x4` | não detectado na janela | - | n/a (patch de dado) | Fox |
| 813 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:235` | `Captain` | - | `tabela lv10_ai_behaviour[Character.id.FALCON] +0x4` | não detectado na janela | - | n/a (patch de dado) | Captain |
| 814 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:283` | `Pikachu` | - | `tabela lv10_ai_behaviour[Character.id.PIKACHU] +0x4` | não detectado na janela | - | n/a (patch de dado) | Pikachu |
| 815 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:331` | `Samus` | - | `tabela lv10_ai_behaviour[Character.id.SAMUS] +0x4` | não detectado na janela | - | n/a (patch de dado) | Samus |
| 816 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:375` | `Jigglypuff` | - | `tabela lv10_ai_behaviour[Character.id.JIGGLYPUFF] +0x4` | não detectado na janela | - | n/a (patch de dado) | Jigglypuff |
| 817 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:419` | `Luigi` | - | `tabela lv10_ai_behaviour[Character.id.LUIGI] +0x4` | não detectado na janela | - | n/a (patch de dado) | Luigi |
| 818 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:460` | `Yoshi` | - | `tabela lv10_ai_behaviour[Character.id.YOSHI] +0x4` | não detectado na janela | - | n/a (patch de dado) | Yoshi |
| 819 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:500` | `Ness` | - | `tabela lv10_ai_behaviour[Character.id.NESS] +0x4` | não detectado na janela | - | n/a (patch de dado) | Ness |
| 820 | `.toolchain/pkgs/smashremix-src/src/VanillaLv10Attacks.asm:541` | `Kirby` | - | `tabela lv10_ai_behaviour[Character.id.KIRBY] +0x4` | não detectado na janela | - | n/a (patch de dado) | Kirby |

### VsDemo.asm

Vs Demo: habilita personagens e estágios Remix e correção de offset.

Hooks de código: 5; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 821 | `.toolchain/pkgs/smashremix-src/src/VsDemo.asm:13` | `VsDemo` | 0x18C914 | `0x8018DBD4` | não detectado na janela | - | `decomp/src/sc/sccommon/scexplain.c:500` `scExplainSetInterfaceGObjs` (+0x4) | VsDemo |
| 822 | `.toolchain/pkgs/smashremix-src/src/VsDemo.asm:21` | `fix_offset_` | 0x18C948 | `0x8018DC08` | não detectado na janela | `jal     fix_offset_` | `decomp/src/mv/mvopening/mvopeningfox.c:401` `mvOpeningFoxMakePosedFighterCamera` (exato) | fix offset |
| 823 | `.toolchain/pkgs/smashremix-src/src/VsDemo.asm:34` | `enable_focused_remix_chars_` | 0x11735C | `0x80131CDC` | não detectado na janela | `jal     enable_focused_remix_chars_` | `decomp/src/mn/mn1pmode/mn1pcontinue.c:226` `mnPlayers1PGameContinueScoreDigitGetSprite` (exato) | enable focused remix chars |
| 824 | `.toolchain/pkgs/smashremix-src/src/VsDemo.asm:74` | `enable_nonfocused_remix_chars_` | 0x18C668 | `0x8018D928` | não detectado na janela | `jal     enable_nonfocused_remix_chars_` | `decomp/src/mv/mvopening/mvopeninglink.c:297` `mvOpeningLinkMakePosedWallpaper` (+0x4) | enable nonfocused remix chars |
| 825 | `.toolchain/pkgs/smashremix-src/src/VsDemo.asm:108` | `enable_remix_stages_` | 0x18C760 | `0x8018DA20` | não detectado na janela | `jal     enable_remix_stages_` | `decomp/src/sc/sccommon/scexplain.c:443` `scExplainMakeTapSpark` (+0x1C) | enable remix stages |

### VsStats.asm

Estatísticas de Vs: rastreador de especiais e prevenção de flash de linha.

Hooks de código: 12; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 826 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:79` | `VsStats` | 0x1588E8 | `0x80139748` | não detectado na janela | - | `decomp/src/mn/mnplayers/mnplayersvs.c:4295` `mnPlayersVSPuckAdjustPlaced` (+0x12C) | VsStats |
| 827 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:216` | `increment_special_tracker` | 0xCB9C8 | `0x80150F88` | não detectado na janela | `jal increment_special_tracker` | `decomp/src/ft/ftcommon/ftcommonspecialair.c:175` `ftCommonSpecialAirCheckInterruptCommon` (+0x80) | increment special tracker |
| 828 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:222` | `increment_special_tracker` | 0xCBA78 | `0x80151038` | não detectado na janela | `jal increment_special_tracker` | `decomp/src/ft/ftcommon/ftcommonspecialair.c:175` `ftCommonSpecialAirCheckInterruptCommon` (+0x130) | increment special tracker |
| 829 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:228` | `increment_special_tracker` | 0xCBA0C | `0x80150FCC` | não detectado na janela | `jal increment_special_tracker` | `decomp/src/ft/ftcommon/ftcommonspecialair.c:175` `ftCommonSpecialAirCheckInterruptCommon` (+0xC4) | increment special tracker |
| 830 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:234` | `increment_special_tracker` | 0xCBBF4 | `0x801511B4` | não detectado na janela | `jal increment_special_tracker` | `decomp/src/ft/ftcommon/ftcommonspecialhi.c:51` `ftCommonSpecialHiCheckInterruptCommon` (+0x54) | increment special tracker |
| 831 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:240` | `increment_special_tracker` | 0xCBB74 | `0x80151134` | não detectado na janela | `jal increment_special_tracker` | `decomp/src/ft/ftcommon/ftcommonspecialn.c:119` `ftCommonSpecialNCheckInterruptCommon` (+0x9C) | increment special tracker |
| 832 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:246` | `increment_special_tracker` | 0xCBC78 | `0x80151238` | não detectado na janela | `jal increment_special_tracker` | `decomp/src/ft/ftcommon/ftcommonspeciallw.c:51` `ftCommonSpecialLwCheckInterruptCommon` (+0x58) | increment special tracker |
| 833 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:875` | `prevent_timed_row_flash_` | 0x1554EC | `0x8013634C` | não detectado na janela | `j       prevent_timed_row_flash_` | `decomp/src/ft/ftcomputer.c:5589` `ftComputerCheckFindItem` (+0x3C) | prevent timed row flash |
| 834 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:880` | `prevent_timed_row_flash_` | 0x15557C | `0x801363DC` | não detectado na janela | `j       prevent_timed_row_flash_` | `decomp/src/mn/mnplayers/mnplayersvs.c:2776` `mnPlayersVSPortraitFlashThreadUpdate` (exato) | prevent timed row flash |
| 835 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:885` | `prevent_timed_row_flash_` | 0x155644 | `0x801364A4` | não detectado na janela | `j       prevent_timed_row_flash_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2607` `mnPlayers1PBonusMakePuckAdjust` (+0x8) | prevent timed row flash |
| 836 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:890` | `prevent_timed_row_flash_` | 0x1556D4 | `0x80136534` | não detectado na janela | `j       prevent_timed_row_flash_` | `decomp/src/mn/mnplayers/mnplayers1pbonus.c:2613` `mnPlayers1PBonusSpotlightProcUpdate` (+0x54) | prevent timed row flash |
| 837 | `.toolchain/pkgs/smashremix-src/src/VsStats.asm:895` | `prevent_timed_row_flash_` | 0x155748 | `0x801365A8` | não detectado na janela | `j       prevent_timed_row_flash_` | `decomp/src/ft/ftcomputer.c:5657` `ftComputerCheckTargetItemInRange` (+0x58) | prevent timed row flash |

### WallTeching.asm

Wall Teching: rastreio de roll e tech bem-sucedidos.

Hooks de código: 3; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 838 | `.toolchain/pkgs/smashremix-src/src/WallTeching.asm:8` | `patch` | 0xBC644 | `0x80141C04` | Toggle: Wall Teching | `j       patch` | `decomp/src/ft/ftcommon/ftcommonwalldamage.c:25` `ftCommonWallDamageSetStatus` (+0xFC) | patch |
| 839 | `.toolchain/pkgs/smashremix-src/src/WallTeching.asm:66` | `track_successful_tech` | 0xBF0B0 | `0x80144670` | não detectado na janela | `jal     track_successful_tech` | `decomp/src/ft/ftcommon/ftcommonpassive.c:9` `ftCommonPassiveSetStatus` (+0x10) | track successful tech |
| 840 | `.toolchain/pkgs/smashremix-src/src/WallTeching.asm:88` | `track_successful_roll` | 0xBF154 | `0x80144714` | não detectado na janela | `jal     track_successful_roll` | `decomp/src/ft/ftcommon/ftcommonpassivestand.c:9` `ftCommonPassiveStandSetStatus` (+0x14) | track successful roll |

### Widescreen.asm

Widescreen: habilita o modo e corrige aspect ratio do menu.

Hooks de código: 3; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): edição C trivial.

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 841 | `.toolchain/pkgs/smashremix-src/src/Widescreen.asm:19` | `enable_widescreen_` | 0x000890D0 | `0x8010D8D0` | não detectado na janela | `jal    enable_widescreen_._match_start_1` | `decomp/src/gm/gmcamera.c:1115` `gmCameraMakeDefaultCamera` (+0xE8) | enable widescreen |
| 842 | `.toolchain/pkgs/smashremix-src/src/Widescreen.asm:24` | `enable_widescreen_` | 0x000891B4 | `0x8010D9B4` | não detectado na janela | `jal     enable_widescreen_._match_start_2` | `decomp/src/gm/gmcamera.c:1115` `gmCameraMakeDefaultCamera` (+0x1CC) | enable widescreen |
| 843 | `.toolchain/pkgs/smashremix-src/src/Widescreen.asm:53` | `menu_fix_aspect_ratio_` | 0x1B20 | `0x80000F20` | não detectado na janela | `j      menu_fix_aspect_ratio_` | `decomp/src/sys/scheduler.c:325` `sySchedulerAddPausedQueue` (+0x1DC) | menu fix aspect ratio |

### yoshishared.asm

Helpers compartilhados do Yoshi: correções de shield, grab, DJ, cpu e structs de especiais.

Hooks de código: 15; patches de tabela: 6; origins de dados (não contados como hook): 0. Dificuldade (heurística): tabela (dado de ROM/tabela).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 844 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:9` | `yoshi_dj_fix_1` | 0xBA818 | `0x8013FDD8` | não detectado na janela | `j       yoshi_dj_fix_1` | `decomp/src/ft/ftcommon/ftcommonjumpaerial.c:128` `ftCommonJumpAerialSetStatus` (+0x64) | yoshi dj fix 1 |
| 845 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:32` | `yoshi_shield_fix_1` | 0xC3688 | `0x80148C48` | não detectado na janela | `j       yoshi_shield_fix_1` | `decomp/src/ft/ftcommon/ftcommonguard1.c:450` `ftCommonGuardOnSetStatus` (+0x4C) | yoshi shield fix 1 |
| 846 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:54` | `yoshi_shield_fix_2` | 0xC3560 | `0x80148B20` | não detectado na janela | `j       yoshi_shield_fix_2` | `decomp/src/ft/ftcommon/ftcommonguard1.c:397` `ftCommonGuardOnProcUpdate` (+0x98) | yoshi shield fix 2 |
| 847 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:76` | `yoshi_shield_fix_3` | 0xC2E54 | `0x80148414` | não detectado na janela | `j       yoshi_shield_fix_3` | `decomp/src/ft/ftcommon/ftcommonguard1.c:127` `ftCommonGuardUpdateShieldCollision` (+0xC) | yoshi shield fix 3 |
| 848 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:98` | `yoshi_shield_fix_4` | 0xC2DC4 | `0x80148384` | não detectado na janela | `j       yoshi_shield_fix_4` | `decomp/src/ft/ftcommon/ftcommonguard1.c:74` `ftCommonGuardUpdateShieldVars` (+0x80) | yoshi shield fix 4 |
| 849 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:121` | `yoshi_shield_fix_5` | 0x66B60 | `0x800EB360` | não detectado na janela | `j       yoshi_shield_fix_5` | unmatched | yoshi shield fix 5 |
| 850 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:143` | `yoshi_shield_fix_6` | 0xC38B8 | `0x80148E78` | não detectado na janela | `j       yoshi_shield_fix_6` | `decomp/src/ft/ftcommon/ftcommonguard2.c:13` `ftCommonGuardSetStatusFromEscape` (+0x48) | yoshi shield fix 6 |
| 851 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:165` | `yoshi_shield_fix_7` | 0xC3C6C | `0x8014922C` | não detectado na janela | `j       yoshi_shield_fix_7` | `decomp/src/ft/ftcommon/ftcommonescape.c:9` `ftCommonEscapeProcUpdate` (+0x5C) | yoshi shield fix 7 |
| 852 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:187` | `yoshi_shield_fix_8` | 0xC3538 | `0x80148AF8` | não detectado na janela | `j       yoshi_shield_fix_8` | `decomp/src/ft/ftcommon/ftcommonguard1.c:397` `ftCommonGuardOnProcUpdate` (+0x70) | yoshi shield fix 8 |
| 853 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:209` | `yoshi_grab_fix_1` | 0xC4ACC | `0x8014A08C` | não detectado na janela | `j       yoshi_grab_fix_1` | `decomp/src/ft/ftcommon/ftcommoncatch2.c:68` `ftCommonCatchWaitSetStatus` (+0x8C) | yoshi grab fix 1 |
| 854 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:231` | `yoshi_grab_fix_2` | 0xC54E0 | `0x8014AAA0` | não detectado na janela | `j       yoshi_grab_fix_2` | `decomp/src/ft/ftcommon/ftcommoncapturewait.c:45` `ftCommonCaptureWaitSetStatus` (+0x48) | yoshi grab fix 2 |
| 855 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:254` | `yoshi_throw_fix_1` | 0xC5748 | `0x8014AD08` | não detectado na janela | `j       yoshi_throw_fix_1` | `decomp/src/ft/ftcommon/ftcommonthrown1.c:83` `ftCommonThrownSetStatusImmediate` (+0x54) | yoshi throw fix 1 |
| 856 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:279` | `yoshi_cpu_fix_1` | 0xAFFCC | `0x8013558C` | não detectado na janela | `j       yoshi_cpu_fix_1` | `decomp/src/mn/mnplayers/mnplayersvs.c:2231` `mnPlayersVSCheckTeamSelectInRange` (+0x38) | yoshi cpu fix 1 |
| 857 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:310` | `up_special_struct_fix` | 0xE6EE8 | `0x8016C4A8` | não detectado na janela | `j       up_special_struct_fix` | `decomp/src/wp/wpyoshi/wpyoshieggthrow.c:261` `wpYoshiEggThrowMakeWeapon` (+0x10) | up special struct fix |
| 858 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:341` | `down_special_struct_fix` | 0xE72A4 | `0x8016C864` | não detectado na janela | `j       down_special_struct_fix` | `decomp/src/wp/wpyoshi/wpyoshistar.c:140` `wpYoshiStarProcReflector` (+0xB4) | down special struct fix |
| 859 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:519` | `ledge_check` | - | `tabela recovery_logic[Character.id.YOSHI] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 860 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:521` | `ledge_check` | - | `tabela recovery_logic[Character.id.JYOSHI] +0x4` | não detectado na janela | - | n/a (patch de dado) | ledge check |
| 861 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:578` | `nsp_check` | - | `tabela cpu_post_process[Character.id.YOSHI] +0x4` | não detectado na janela | - | n/a (patch de dado) | nsp check |
| 862 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:580` | `nsp_check` | - | `tabela cpu_post_process[Character.id.JYOSHI] +0x4` | não detectado na janela | - | n/a (patch de dado) | nsp check |
| 863 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:623` | `cpu_attack_weight` | - | `tabela cpu_attack_weight[Character.id.YOSHI] +0x4` | não detectado na janela | - | n/a (patch de dado) | cpu attack weight |
| 864 | `.toolchain/pkgs/smashremix-src/src/yoshishared.asm:625` | `cpu_attack_weight` | - | `tabela cpu_attack_weight[Character.id.JYOSHI] +0x4` | não detectado na janela | - | n/a (patch de dado) | cpu attack weight |

### ZCancel.asm

Z-Cancel: opções de modo (Cruel Z-Cancel).

Hooks de código: 2; patches de tabela: 0; origins de dados (não contados como hook): 0. Dificuldade (heurística): novo estado (flags/variáveis).

| # | Cite | Rotina (scope) | ROM | RAM / alvo | Gating | Entrada Remix | Decomp | Comportamento |
|---|---|---|---|---|---|---|---|---|
| 865 | `.toolchain/pkgs/smashremix-src/src/ZCancel.asm:34` | `CRUEL_Z_CANCEL_MODE` | 0xCB488 | `0x80150A48` | não detectado na janela | `j       _cruel_z_cancel` | `decomp/src/ft/ftcommon/ftcommonattackair.c:55` `ftCommonAttackAirProcMap` (+0x40) | CRUEL Z CANCEL MODE |
| 866 | `.toolchain/pkgs/smashremix-src/src/ZCancel.asm:235` | `_z_cancel_opts` | 0xCB480 | `0x80150A40` | não detectado na janela | `j       _z_cancel_opts` | `decomp/src/ft/ftcommon/ftcommonattackair.c:55` `ftCommonAttackAirProcMap` (+0x38) | z cancel opts |
