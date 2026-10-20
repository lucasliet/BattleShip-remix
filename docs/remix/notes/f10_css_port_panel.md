# F10 - Painel de debug da seleção de personagem (CSS): 20 opções

Fonte (somente leitura): `.toolchain/pkgs/smashremix-src/src/`. Os caminhos abaixo são relativos a essa pasta. `css/X.asm` define o item de menu (label, faixa, tabela de strings, `onchange`, hooks de limpeza 1P) e `X.asm` (na raiz de `src/`) costuma conter o hook de partida que lê o valor por porta. Os números de linha foram conferidos no código.

Limitação: o briefing do ai-memory para o projeto não foi carregado, porque não existe `.ai-memory.toml` na raiz do BattleShip e o projeto não existe no workspace `default`. Só as regras globais (`_rules/`) foram lidas.

## Como o painel abre, fecha e ocupa a tela

O painel tem um botão próprio em cada CSS (sprite do arquivo de imagens do CSS, footer `0x1208`). O botão é criado em `CharacterSelectDebugMenu.asm` (aprox. linhas 128 a 290) e tratado em `handle_debug_button_press_` (linhas 1571 a 1616). Ele só responde se o toggle global "CSS Panel Menu" estiver ligado (`Toggles.asm:2381`) e se o flag de habilitação do botão (offset `0x50`) estiver ativo. Cada toque inverte o estado de exibição (offset `0x44`), chama `set_panel_display_` e toca o FGM `FGM.menu.TOGGLE`. Fecha-se com o mesmo botão.

Por modo: no VS há um painel por porta (4), e o botão começa desabilitado e só é liberado quando as portas do painel estão totalmente abertas (offset `0xA4` igual a 0, em `CharacterSelectDebugMenu.asm` perto das linhas 110 a 120). No treino há 4 painéis, com o índice de D-pad trocado quando a CPU é P1. No 1P e no Bônus há um único painel, cuja porta vem de `0x800A4AE3`. No All-Star usa-se o array de menu próprio (`menu_array_allstar`).

Layout: 3 itens por página (`CharacterSelectDebugMenu.asm:602` a `611`, divisão por 3 com paginação). Cada linha ocupa 24 px de altura (linha 699), e o texto começa 4 px à direita e 18 px abaixo da origem do painel (constantes `0x4080` e `0x4190`, linhas 802 e 807 de `CharacterSelectDebugMenu.asm`). A largura máxima do texto é a largura do painel menos 8 px (linhas 823 a 825), e a largura do painel vem da imagem do CSS em tempo de execução, não de uma constante do código. Na prática são 2 colunas por linha (rótulo à esquerda e valor entre setas), mais setas de paginação. A largura exata em pixels não é fixa no código.

## Modos de aplicação (bitmask APPLIES_TO)

Bits na ordem [vs][1p][training][bonus1][bonus2][allstar]. HMN/CPU indica a quem a opção se aplica.

| Opção | Bitmask | Modos | HMN/CPU |
|---|---|---|---|
| Shield | 111111 | todos | ambos |
| PlayerTag | 111111 | todos | ambos |
| Visibility | 111111 | todos | ambos |
| Skeleton | 111111 | todos | ambos |
| ModelDisplay | 111111 | todos | ambos |
| InputDisplay | 111111 | todos | ambos |
| Size | 111111 | todos | ambos |
| StockMode | 100000 | só VS | ambos |
| Knockback | 111111 | todos | ambos |
| InputDelay | 111111 | todos | ambos |
| Handicap | 101000 | VS e treino | só CPU |
| StartWith | 111111 | todos | ambos |
| TauntItem | 111111 | todos | ambos |
| TauntButton | 111111 | todos | só HMN |
| KirbyHat | 111111 | todos | ambos |
| DpadFunctions | 111111 | todos | só HMN |
| DpadControl | 111111 | todos | só HMN |
| Damage | 110110 | VS, 1P, Bônus 1 e 2 | ambos |
| PoisonDmg | 110111 | VS, 1P, Bônus 1 e 2, All-Star | ambos |
| Practice_1P | 010000 | só 1P | só HMN |

---

## 1. Damage ("Initial Damage")

1. **O que faz:** define o dano inicial (porcentagem) de cada jogador ao carregar o personagem. Em modo stamina, o valor é limitado ao HP total.
2. **Faixa e padrão:** 0 a 999, padrão 0. Tabela `state_table` (`css/Damage.asm:16`), tipo numérico.
3. **Leituras de partida:** `css/Damage.asm:83` (`char_init_`, hook no load do personagem, em `0x800D7A14`, ignora respawn); `css/Damage.asm:110` (`restrict_damage_for_stamina_`, chamado por `src/CharacterSelect.asm:4577` e `src/Stamina.asm:2055`); `css/Damage.asm:155` (`onchange_handler`, que limita e cicla o valor em stamina).
4. **Complexidade no port C:** trivial (aplicar o valor inicial ao estado do jogador na criação; a regra de stamina é simples).

## 2. DpadControl ("Dpad ctrl")

1. **O que faz:** troca o D-pad pelo analógico (Stick Swap), remove o analógico (Stickless), ou as variantes "J" em que o D-pad Up vira pulo. Só para humanos.
2. **Faixa e padrão:** 0 a 4, padrão 0. Valores: 0 Off, 1 Stick Swap, 2 Stickless, 3 Stick Swap J, 4 Stickless J (comentário logo após a tabela de strings em `css/DpadControl.asm`). Tabela `dpad_controls_table` (`css/DpadControl.asm:680`).
3. **Leituras de partida:** `css/DpadFunctions.asm:49` (`dpad_macro_check_`, hook do Joypad em `0x800E1330`, patch `0x5CB30`); `css/DpadControl.asm:60` (`set_shield_mask_`, patch `0x800D84F0`); `css/DpadControl.asm:105` (`set_R_macro`, patch `0x800E12FC`); `css/DpadControl.asm:141` (`css_cursor_and_dpad_controls_`); `css/DpadControl.asm:471` (`force_settings_for_training_`); `css/DpadControl.asm:517` e `556` (`check_jump_type_idle_` e `check_jump_type_running_`); `css/DpadControl.asm:648` (`dash_attack_check_`). Também `src/SingleButtonMode.asm:112` pula para `_sbm_checked`.
4. **Complexidade no port C:** needs a fighter hook (camada de entrada: mapeia Joypad, pulo, dash e escudo por porta).

## 3. DpadFunctions ("Dpad map")

1. **O que faz:** converte o D-pad em golpes de Smash, Tilt ou Special (macro). Só para humanos.
2. **Faixa e padrão:** 0 a 3, padrão 0. Valores: 0 Off, 1 Smash, 2 Tilt, 3 Special. Tabela `dpad_macro_table` (`css/DpadFunctions.asm:643`). O mapeamento exato de direção para golpe não foi lido linha a linha (a verificar em `dpad_macro_check_`).
3. **Leituras de partida:** `css/DpadFunctions.asm:84` (`dpad_macro_check_`, mesmo hook de DpadControl); `css/DpadFunctions.asm:584` (`dpad_macro_check_held_`); `css/DpadFunctions.asm:628` (`clear_settings_for_training_`). `src/Render.asm:2416` e `2418` têm chamadas comentadas a essas rotinas.
4. **Complexidade no port C:** needs a fighter hook (camada de entrada, igual a DpadControl).

## 4. Handicap

1. **O que faz:** define um handicap de CPU por porta, aplicado só a CPUs (VS e treino). A lista mistura níveis de dificuldade ("V. Easy" a "V. Hard") de Yoshi, Kirby, Polygon, G. DK, M. Mario e MH, e a entrada "Samus" (0x28). O significado exato de cada valor não foi confirmado no código (a verificar em `src/Handicap.asm`).
2. **Faixa e padrão:** 0 a 0x28 (40), padrão 0 ("Default"). Tabela `override_table` (`src/Handicap.asm:13`), com 41 strings listadas em `css/Handicap.asm:18` a `103`.
3. **Leitura de partida:** `src/Handicap.asm:35` (`use_handicap_`, só quando o jogador é CPU; lê `override_table` pela porta).
4. **Complexidade no port C:** needs a fighter hook (ajuste do comportamento da CPU).

## 5. InputDelay ("Delay")

1. **O que faz:** atrasa as entradas do controle em N frames, com buffer por porta.
2. **Faixa e padrão:** 0 a 12 frames (`MAX_FRAMES` = 12 em `src/InputDelay.asm:14`), padrão 0 ("None"). Tabela `delay_table` (`src/InputDelay.asm:118`).
3. **Leitura de partida:** `src/InputDelay.asm:37` (`apply_input_delay_`, lê `delay_table` por porta). Limpeza 1P em `src/InputDelay.asm:44` (`clear_settings_for_1p_`).
4. **Complexidade no port C:** needs a fighter hook (camada de entrada, buffer de frames por porta).

## 6. InputDisplay ("Input HUD")

1. **O que faz:** mostra a exibição de inputs do jogador na HUD, em posição superior ou inferior.
2. **Faixa e padrão:** 0 a 2, padrão 0 ("Disabled", reaproveitado de `Skeleton.string_disabled`). 1 = Top, 2 = Bottom. Tabela `state_table` (`src/InputDisplay.asm:15`).
3. **Leituras de partida:** `src/InputDisplay.asm:30` (`setup_`); `src/InputDisplay.asm:959` e `971` (`adjust_timer_position_`, ajuste de posição). Limpeza 1P em `src/InputDisplay.asm:32`.
4. **Complexidade no port C:** needs UI only.

## 7. KirbyHat ("Kirby Hat")

1. **O que faz:** define qual chapéu o Kirby (e o J. Kirby) recebe ao copiar. Inclui "???" (chapéu mágico) e "Random".
2. **Faixa e padrão:** 0 a 0x2A (42), padrão 0 ("None"). Índice 41 = "???" (`MAX_VALUE - 1`), índice 42 = "Random". Tabela `spawn_with_hat` (`src/KirbyHats.asm:510`), com lista de personagens em `css/KirbyHat.asm:19` a `61`.
3. **Leituras de partida:** `src/KirbyHats.asm:582` (`kirby_hat_select_`); `src/KirbyHats.asm:658` (`kirby_hat_files_training_`); `src/KirbyHats.asm:765` (`kirby_hat_files_1p_`); `src/Size.asm:364` (`initialize_base_multiplier_table_` lê o slot do chapéu para o flag do chapéu mágico). A troca de parte do modelo fica em `css/KirbyHat.asm:76` (`onchange_handler`, chamadas `0x800E8EAC` e `0x800E8ECC`).
4. **Complexidade no port C:** needs a fighter hook (troca do chapéu no modelo do Kirby).

## 8. Knockback

1. **O que faz:** usa ângulo de knockback padrão ou aleatório, aplicado a hitbox e a arremessos.
2. **Faixa e padrão:** 0 a 1, padrão 0 ("Default"); 1 = "Random". Tabela `state_table` (`src/Knockback.asm:112`).
3. **Leituras de partida:** `src/Knockback.asm:151` (`get_angle_hitbox_`); `src/Knockback.asm:191` (`get_angle_throw_`). Limpeza 1P em `css/Knockback.asm:31`.
4. **Complexidade no port C:** needs a fighter hook (cálculo de ângulo do golpe).

## 9. ModelDisplay ("Model")

1. **O que faz:** escolhe o modelo do personagem em alta ou baixa poligonagem, e recarrega o modelo quando o personagem já foi selecionado.
2. **Faixa e padrão:** 0 a 2, padrão 0 ("Default", que usa alta). 1 = Hi Poly, 2 = Lo Poly. Tabela `model_display_for_port` (`src/Costumes.asm:1188`).
3. **Leituras de partida:** `src/Costumes.asm:1226` e `1260` (`force_hi_lo_poly_`). O `onchange_handler` em `css/ModelDisplay.asm:36` chama `0x800E9198` para recarregar o modelo.
4. **Complexidade no port C:** needs a fighter hook (troca e recarga do modelo).

## 10. PlayerTag ("Player Tag")

1. **O que faz:** troca a etiqueta (imagem) do jogador. "Default" mantém a original, as demais são as tags de `Toggles.entry_player_tags`.
2. **Faixa e padrão:** 0 a 20, padrão 0 ("Default"). Lista de tags em `css/PlayerTag.asm:23` (entradas `Toggles.entry_player_tags_{n} + 0x28`). Tabela `enable_for_port` (`src/PlayerTag.asm:13`).
3. **Leituras de partida:** `src/PlayerTag.asm:40` (`use_custom_tag_image_`); `src/PlayerTag.asm:318` (`always_display_custom_tag_`); `src/Toggles.asm:2816` (lê `PlayerTag.enable_for_port`). Limpeza 1P em `css/PlayerTag.asm:35`.
4. **Complexidade no port C:** needs UI only (troca de textura da etiqueta, sem mudar a lógica do lutador).

## 11. PoisonDmg ("Poison Dmg")

1. **O que faz:** aplica dano periódico de veneno ao jogador (a cada 2 s de jogo), com ajuste para velocidade de jogo e stamina. Pula se o jogador estiver com o Cloaking Device ativo ou com Super Star. O valor "Heal" cura.
2. **Faixa e padrão:** 0 a 4, padrão 0 ("None"). Valores: 1 Low (+1%), 2 Medium (+2%), 3 High (+4%), 4 Heal (+2 HP). Tabela `state_table` (`css/PoisonDmg.asm:16`).
3. **Leitura de partida:** `css/PoisonDmg.asm:113` (`apply_damage`, lê `PoisonDmg.state_table` por porta). É chamado por `src/Poison.asm:51`. Limpeza 1P em `css/PoisonDmg.asm:45`.
4. **Complexidade no port C:** needs a fighter hook (dano por tempo do jogador, no mesmo ponto do hazard de veneno).

## 12. Practice_1P ("Practice")

1. **O que faz:** modo de prática 1P. Escolhe OFF ou uma fase específica (1 a 10 ou Final Stage), e segurar L reinicia a fase.
2. **Faixa e padrão:** 0 a 11 (`STAGES` = 11, em `src/Practice_1P.asm:335`), padrão 0 ("OFF"). Índices 1 a 10 = "Stage N", 11 = "Final Stage". Tabela `stage_table` (`src/Practice_1P.asm:339`).
3. **Leituras de partida:** `src/Practice_1P.asm:41` (`_1p_practice`); `src/Practice_1P.asm:172` (`hold_l_to_retry_`); `src/Practice_1P.asm:235` (`hold_l_to_retry_stage_clear_`).
4. **Complexidade no port C:** needs a fighter hook no sentido amplo (é fluxo de partida 1P e troca de fase, não de lutador).

## 13. Shield

1. **O que faz:** muda a cor do escudo do jogador, incluindo a cor do ovo do Yoshi, e pode usar um costume.
2. **Faixa e padrão:** 0 a 17 (18 valores), padrão 0 ("Default"). 1 Red, 2 Orange, 3 Yellow, 4 Lime, 5 Green, 6 Turquoise, 7 Cyan, 8 Azure, 9 Blue, 10 Purple, 11 Magenta, 12 Pink, 13 Brown, 14 Black, 15 White, 16 Vanilla, 17 Costume. Tabela `state_table` (`src/Shield.asm:180`).
3. **Leituras de partida:** `src/Shield.asm:70` (`color_fix_`); `src/Shield.asm:245`, `307`, `367`, `461`, `524` (`alter_yoshi_egg_color_`). Limpeza 1P em `css/Shield.asm:63`.
4. **Complexidade no port C:** needs a fighter hook (cor do escudo e do ovo no render).

## 14. Size

1. **O que faz:** força o personagem para tamanho Giant ou Tiny durante a partida, com multiplicador de escala. Desabilita high scores quando diferente do padrão.
2. **Faixa e padrão:** 0 a 2, padrão 0 ("Default"). 1 = Giant, 2 = Tiny. Tabela `state_table` (`src/Size.asm:23`), copiada em `match_state_table` (`src/Size.asm:29`). Multiplicadores em `multiplier_table` (`src/Size.asm:13`).
3. **Leituras de partida:** `src/Size.asm:384` (`initialize_base_multiplier_table_`, lê `match_state_table[index]` e aplica o multiplicador); `src/SinglePlayerModes.asm:5126` e `5148` (`remix_1p_giant`, lê `match_state_table`); `src/Hazards.asm:4766` (`shrink_timer_`). Sincronização do CSS em `src/CharacterSelect.asm:4581` (`sync_match_state_table_`). `multiplier_table` também é lido em muitos pontos (`src/Size.asm`, `src/Hazards.asm:4765`, `src/command.asm:632`).
4. **Complexidade no port C:** needs a fighter hook (escala do personagem; a multiplicação precisa chegar à física e ao render).

## 15. Skeleton

1. **O que faz:** ativa o modo esqueleto para o personagem da porta.
2. **Faixa e padrão:** 0 a 1, padrão 0 ("Disabled"); 1 = "Enabled". Tabela `enable_for_port` (`src/Skeleton.asm:15`).
3. **Leitura de partida:** `src/Skeleton.asm:25` e `26` (rotina do escopo `Skeleton`, offset por porta). Limpeza 1P em `css/Skeleton.asm:31`. O corpo completo da rotina não foi lido (a verificar).
4. **Complexidade no port C:** needs a fighter hook (troca de modelo ou de ossos).

## 16. StartWith ("Start With")

1. **O que faz:** item que o jogador começa com (no início da partida, e também no respawn). Inclui "None" e "Random".
2. **Faixa e padrão:** 0 a 19, padrão 0 ("None"). `MAX_VALUE` = `Item.start_with_random_entry` = 0x13 (`src/Item.asm:1720`). Índice 19 = "Random". Tabela `start_with_item` (`src/Item.asm:1722`). Entradas comentadas (Tomato, Heart, Star, Cloaking Device, Super Shroom, Poison Shroom) ficam fora da lista.
3. **Leituras de partida:** `src/Item.asm:1744` (`match_begin_with_item_`); `src/Item.asm:1802` (`start_with_item_`); `src/Item.asm:1867` (`respawn_with_item_`). `src/Render.asm:2410` chama `Item.start_with_item_`. Limpeza 1P em `css/StartWith.asm:79`.
4. **Complexidade no port C:** needs a fighter hook (spawn de item no início e no respawn).

## 17. StockMode ("Stock")

1. **O que faz:** define a quantidade de stocks por porta no VS: Default (regra do modo), Last (último valor usado) ou Manual (contador do menu). No Tag Team e no Tug of War a tabela também é usada.
2. **Faixa e padrão:** 0 a 2, padrão 0 ("Default"). 1 = Last, 2 = Manual. Tabela `stockmode_table` (`src/StockMode.asm:11`) e contador `stock_count_table` (`src/StockMode.asm:17`).
3. **Leituras de partida:** `src/CharacterSelect.asm:5552` e `5961` (`update_stock_icon_and_count_indicators_`); `src/TagTeam.asm:404`, `506`, `530`, `574`, `728`, `1034`, `1651`, `2310`; `src/TugOfWar.asm:82`, `129`, `153`, `197`; `src/TwelveCharBattle.asm:404`, `436`, `4109`; `src/KingOfTheHill.asm:220`; `src/Smashketball.asm:164`; `src/VsRemixMenu.asm:419`, `498`, `572`, `647`.
4. **Complexidade no port C:** needs a fighter hook no sentido amplo (é regra de partida, contagem de stocks e HUD; não é do lutador).

## 18. TauntButton ("Taunt Btn.")

1. **O que faz:** muda o botão de taunt (L, C-Up, C-Down, C-Left, C-Right, D-Up, D-Down, D-Left, D-Right). Só para humanos.
2. **Faixa e padrão:** 0 a 8, padrão 0 ("L"). Tabela `taunt_mask_per_port` (`src/Joypad.asm:384`).
3. **Leituras de partida:** `src/Joypad.asm:399` (`set_taunt_mask_`); `src/Joypad.asm:452` (`prevent_taunt_jump_`); `css/DpadControl.asm` (`set_shield_mask_`) também lê `Joypad.taunt_mask_per_port` para não conflitar com o escudo no Z. Limpeza 1P em `css/TauntButton.asm:45`.
4. **Complexidade no port C:** trivial (é um mapeamento de botão por porta; cuidar só da interação com o escudo).

## 19. TauntItem ("Taunt Itm.")

1. **O que faz:** item que aparece ao usar taunt. Usa a mesma lista de StartWith, com "None" e "Random".
2. **Faixa e padrão:** 0 a 19, padrão 0 ("None"). `MAX_VALUE` = `Item.taunt_item_random_entry` = 0x13 (`src/Item.asm:1932`). Tabela `taunt_spawn_item` (`src/Item.asm:1934`). Usa as strings de `StartWith`.
3. **Leitura de partida:** `src/Item.asm:1950` (`taunt_spawn_item_`). Limpeza 1P em `css/TauntItem.asm:50`.
4. **Complexidade no port C:** needs a fighter hook (spawn de item quando o taunt é usado).

## 20. Visibility

1. **O que faz:** altera a cor de ambiente do personagem, em forma de override por porta, com a aparência por valor (Dark escurece). "Cloaked" não está implementado: o código tem o comentário "I don't know how to do that yet" e grava um valor fixo.
2. **Faixa e padrão:** 0 a 3, padrão 0 ("Default"). 1 Cloaked (não implementado), 2 None (override `0xFFFFFF00`, provavelmente invisível, a verificar), 3 Dark (override `0x000000FF`). Tabelas `state_table` (`src/CharEnvColor.asm:26`) e `override_table` (`src/CharEnvColor.asm:16`).
3. **Leituras de partida:** `src/CharEnvColor.asm:976` (`override_env_color_`); `src/CharEnvColor.asm:1669` (`update_player_indicator_alpha_`); `src/CharEnvColor.asm:1700` (`initialize_override_table_`). Limpeza 1P em `css/Visibility.asm:65`.
4. **Complexidade no port C:** needs a fighter hook (cor de ambiente e transparência do personagem no render).

---

## Resumo de complexidade no port C

- **Trivial:** Damage, TauntButton.
- **Needs UI only:** PlayerTag, InputDisplay.
- **Needs a fighter hook:** Shield, Visibility, Skeleton, ModelDisplay, Size, Knockback, PoisonDmg, KirbyHat, Handicap, StartWith, TauntItem.
- **Hook de entrada ou fluxo (não é lutador puro):** InputDelay, DpadControl, DpadFunctions (camada de input), StockMode e Practice_1P (regras e fluxo de partida).

## Observações

- Tabelas que a `css/X.asm` lê (`state_table`, `enable_for_port`, etc.) são definidas em `src/X.asm`, e não no arquivo `css/`. Por isso uma busca só em `css/` não acha as leituras.
- O menu principal registra 20 opções em `CharacterSelectDebugMenu.asm:2720` a `2740` (Practice_1P é a última). Os 20 batem com a lista pedida.
