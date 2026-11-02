# T2.11 triagem D: REMIX-TODO de CSS, VS, Training e Credits

Triagem somente de leitura. Nenhum arquivo do repositório foi alterado além desta nota. Os caminhos de asm são relativos a `.toolchain/pkgs/smashremix-src/src/`.

Classificações: READY (pode ser implementado com o que a porta tem), BLOCKED (falta algo nomeado), NOT-OBSERVABLE (sem efeito visível). UNKNOWN quando não foi possível determinar.

## 1. `decomp/src/mn/mndata/mncharacters.c:1698` (chardata-emblem)

NOT-OBSERVABLE. O TODO diz que o asm não adiciona modelos de emblema para os fighters estendidos, e a porta deixa o gobj vazio para eles, então a tela fica sem emblema nos dois lados. Não localizei a rotina de emblema no asm por busca direta; a afirmação vem de `docs/remix/notes/f10_menu_chardata.md:21` e `:29`. Falta confirmar a linha exata do asm (UNKNOWN). Nenhuma mudança necessária além de manter o retorno antecipado.

## 2. `decomp/src/mn/mndata/mncharacters.c:2371` (chardata-motion-names)

NOT-OBSERVABLE, pelo mesmo critério. A porta pula o sprite de nome do golpe especial para os fighters estendidos, e a nota `docs/remix/notes/f10_menu_chardata.md:23` e `:30` diz que o asm também não cria esses sprites. Não encontrei `motion_names` nem a rotina correspondente no asm por busca textual (UNKNOWN na linha do asm). Os botões especiais desses fighters usam descritores comuns (`f10_menu_chardata.md:31`), o que é um ponto de paridade separado e ainda não verificado visualmente.

## 3. `decomp/src/mn/mnplayers/mnremixgoalpicker.c:6` (koth-goal-label)

BLOCKED. O asm lê `CharacterSelect.showing_time` em `KingOfTheHill.asm:239` e `:273`. A alternância Time/Goal é feita pelo atalho L em `Toggles.asm:3291-3292` e pelo ponto que inverte a flag em `CharacterSelect.asm:6357-6377`. A porta sempre mostra os valores de Goal, o que coincide com o asm quando `showing_time` está desligado, mas não há como mostrar Time nem o rótulo "Goal". Faltam: (a) um símbolo REMIX_SYM para `CharacterSelect.showing_time`, que não existe em `port/remix/remix_symbols.h` (cheap: incluir em `EXTRA_SYMBOLS` de `tools/gen_remix_actions.py` e regenerar); (b) o caminho de desenho do texto "Goal" no CSS, que não localizei no asm (UNKNOWN). Esforço S para o símbolo, M para o rótulo. Verificação: script de entrada no CSS com a opção de tempo do VS KOTH, via a skill `remix-smoke-tests` (nomes exatos dos scripts UNKNOWN).

## 4. `decomp/src/mn/mnvsmode/mnremixvsstatsview.c:133` (VsStats)

BLOCKED. `VsStats.asm:735-739` esconde o grupo 0x0D (instruções do menu) e o grupo 0x16 (linhas normais de resultado), e mostra o grupo 0x0E e a página atual. Isso usa `Render.toggle_group_display_` (`Render.asm:2087`), que percorre uma lista ligada a partir de `GROUP_TABLE` e grava um flag em `0x7C` de cada objeto. Não há nenhum símbolo `Render__*` ou `GROUP_TABLE` em `remix_symbols.h`, e a porta lê a imagem da RAM como somente leitura, então a escrita não é possível. Faltam: um mecanismo de ocultar grupos de display na porta (engine feature) e o símbolo de `GROUP_TABLE`. Alternativa mais barata: a porta ocultar por conta própria as linhas normais quando a página de estatísticas está visível, mas isso depende de qual código desenha essas linhas (UNKNOWN, ver `docs/remix/notes/f14_results_rows.md`). Esforço M.

## 5. `decomp/src/sc/sc1pmode/sc1ptrainingremix.h:68` (Training HUD, item na mão, passo 8)

BLOCKED. `Item.asm:311-365` (`extend_training_mode_hud_`) mostra o nome do item na mão. Os hooks são em `0x8018EA0C` (`:314`, checagem de ID) e `0x8018E930` (`:321`, imagem customizada). Para os tipos customizados, o índice é `kind - 0x2A` (`:358`), e a tabela de imagens fica no endereço do arquivo 0xFE somado a `TRAINING_HUD_IMAGE_TABLE = 0x0330` (`:23`, `:360`). Faltam: (a) um símbolo REMIX_SYM para o ponteiro do arquivo 0xFE, que a nota `docs/remix/notes/f14_training_left.md:12` também aponta como ausente; (b) a rotina de desenho do HUD vanilla que o hook em `0x8018E930` substitui, que a porta ainda não tem como gancho. Esforço M a L.

## 6. `decomp/src/sc/sc1pmode/sc1ptrainingremix.h:74` (Z menu, passos 10 a 13)

Estado atual da porta: já existem no menu de porta as entradas Percent e Reset Sets Percent por porta (`port/remix/remix_training.cpp:79-86`), Costume (`:95-98`) e Spawn (`:99-102`). O commit `cf13c42` também cobre Type e Character. A nota `docs/remix/notes/f14_training_menu.md:28` está defasada e ainda lista esses itens como não feitos.

Ainda faltam:
- Set Custom Spawn: no asm, `Training.asm:3670` (`entry_set_custom_spawn_pN`) chama `spawn_func_N_` (`:3617-3620`), que usa a macro `set_custom_spawn` (`:3592-3615`) e chama `set_custom_spawn_` em `Training.asm:1380`. A porta não tem entrada nem editor de posição de spawn. BLOCKED: falta uma função de editor de spawn na cena de training e o armazenamento da posição por porta (engine feature). Esforço M a L.
- Comportamento de dummy por porta: a porta resolve a porta do dummy em `sc1PTrainingRemixSetupPorts` (`sc1ptrainingremix.h:36-41`). Mas `sc1PTrainingModeUpdateDummyBehavior` (`decomp/src/sc/sc1pmode/sc1ptrainingmode.c`, por volta da linha 2040) ainda aplica o comportamento ao índice único `sSC1PTrainingModeMenu.dummy`. Não verifiquei se o `cf13c42` já alimenta esse índice a partir de `sc1PTrainingRemixSetupPorts` (UNKNOWN). Se não alimenta, é READY, esforço S: trocar a origem do índice. Verificação: script de training com CPU em outra porta, checando a aplicação de `dSC1PTrainingModeDummyBehaviors`. As opções globais de CPU (CPU Teching, CPU DI, Spam Practice) estão em `Training.asm:3899-3913` e são globais no asm, não por porta.

## 7. `decomp/src/sc/sccommon/scstaffroll.c:20` (credits-glyphs)

BLOCKED. `Credits.asm:260-289` (`add_new_character`) define 18 glifos, de `0x50` a `0x61`: 10 dígitos, `_`, `!`, `-`, `/`, `*`, `(`, `)`, `@` e `ã` (`~`). Há uma ambiguidade no asm: a entrada 4 (linha 276) está rotulada como `_`, e não existe entrada para o dígito 4. Isso precisa ser resolvido antes de codificar (asm detail UNKNOWN). Os offsets `0x7958` a `0x88B8` são dados por `add_new_character`, e a textura está em `File.REMIX_CREDITS`, arquivo 0xDF1 (`f14_credits_left.md`), que é um id de asset, não um símbolo. Falta a base de offset da textura no arquivo carregado (BLOCKED, conforme o próprio TODO). Além disso, `tools/gen_remix_credits.py` (`encode_scroll_char`) mapeia esses glifos para espaço. Esforço M depois de fixar a base.

## 8. `decomp/src/sc/sccommon/scstaffroll.c:1475` (credits-speed-assist)

BLOCKED. `Credits.asm:1310` (`remix_credits_speed_assist_`, gancho em `0x180CC4`) simula Start quando o segundo tag do jogador é "afk" (`0x61666B00`), o id do nome é `0x55` (endereço `0x8013A8B8`) e a velocidade atual difere da normal (`0x8013A7B0` contra `0x8013A8BC`). A porta não guarda nomes de tag customizados (`src/if/ifremixplayertag.c`). O tag de boot 2 vem vazio no `remix.z64` (`f14_credits_left.md:43`). Não existe símbolo REMIX_SYM para `CharacterSelectDebugMenu.PlayerTag` em `remix_symbols.h` (busca retornou zero ocorrências), então o caminho de leitura do tag também está ausente. Falta uma fonte de tag customizado, que é um subsistema à parte (M). A lógica de comparação e de simulação de Start pode ser portada depois disso. Verificação: não há variável de ambiente documentada para isso (UNKNOWN).

## Resumo

| Local | Classificação | Pendência principal |
|---|---|---|
| mncharacters.c:1698 | NOT-OBSERVABLE | sem efeito visível; linha do asm UNKNOWN |
| mncharacters.c:2371 | NOT-OBSERVABLE | sem efeito visível; linha do asm UNKNOWN |
| mnremixgoalpicker.c:6 | BLOCKED | símbolo `CharacterSelect.showing_time` (S) e rótulo "Goal" (M) |
| mnremixvsstatsview.c:133 | BLOCKED | sem toggle de grupos de display na porta; `GROUP_TABLE` ausente |
| sc1ptrainingremix.h:68 | BLOCKED | símbolo do arquivo 0xFE e rotina de HUD vanilla (M a L) |
| sc1ptrainingremix.h:74 | BLOCKED (spawn), READY ou BLOCKED (dummy, UNKNOWN) | editor de Set Custom Spawn; origem do índice de dummy |
| scstaffroll.c:20 | BLOCKED | base de offset da textura; ambiguidade da entrada 4 |
| scstaffroll.c:1475 | BLOCKED | fonte de tag customizado "afk" (M) |

Limitação: a consulta ao ai-memory não encontrou o projeto (`JRickey/BattleShip` não existe e `JRickey` não é workspace). Só as regras de `default/_global` foram lidas.
