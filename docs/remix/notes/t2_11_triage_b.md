# T2.11 triagem B: REMIX-TODO de ft/gm/gr

Triagem somente leitura. Asm de referência: `.toolchain/pkgs/smashremix-src/src/`. Classificações: READY, BLOCKED, NOT-OBSERVABLE, UNKNOWN.

## ft/ftremix/ftremixzcancel.c:67 (1P final boss, instant KO)

**BLOCKED.** ZCancel.asm:150-180 (`_instant_ko`) só pula o KO quando a tela é 1P (`Global.current_screen` = 1), a fase é a 0x0D (`SinglePlayerModes.STAGE_FLAG`) e o byte "match off" em `0x80131580` é 0 (ZCancel.asm:165-167). Caso contrário segue para `_instant_ko_normal` (limpa overlays e chama `0x8013C1C4`). O port (`ftRemixZCancelInstantKO`) sempre faz o KO. Falta: um dono do byte `0x80131580` (escrito em runtime, "ligado após o fim da luta e ao pausar"; não está no RAM image nem em `remix.sym`) e acesso ao STAGE_FLAG. Desbloqueio: M. Verificação: partida 1P na fase final com pausa, smoke headless.

## ft/ftremix/ftremixzcancel.c:78 (arquivo de ovo do Yoshi 0x153)

**BLOCKED.** ZCancel.asm:325-345 (`setup_`) carrega o arquivo 0x153 em `0x80131000` com `Render.load_file` quando o modo Cruel Z-Cancel é EGG ou RANDOM. Falta: o port não tem o carregamento do arquivo 0x153 (a extração do asset do ROM Remix e o ponto de carga). Desbloqueio: M, se o arquivo estiver disponível no ROM Remix. Verificação: `SSB64_REMIX_*` do Cruel Z-Cancel em Egg, errar um z-cancel e observar o ovo. Não verifiquei se `0x80131000` já é preenchido pelo vanilla.

## ft/ftremix/goemon/ftremixgoemonusp.c:93 (render routine da nuvem)

**Provavelmente READY, não confirmado.** GoemonSpecial.asm:1857-1859 grava `Size.item.render_routine_` em `+0x2C` da nuvem criada por `Item.Cloud.SPAWN_ITEM`, para que o tamanho seja obedecido. O port cria a nuvem com `itRemixCloudMakeItem` (ftremixgoemonusp.c:94). Falta confirmar se existe um helper de render de Size no port para itens (não procurei). Esforço S, se o helper existir. Verificação: Goemon com Size alterado no USP, nuvem com o tamanho certo.

## ft/ftremix/marina/ftremixmarinansp.c:290 e :302 (rotação do throw_turn)

**BLOCKED.** MarinaSpecial.asm:652-700 (`throw_turn_`) decrementa o timer em `0x0B18` e grava a rotação `DObj.rotate.vec.f.y` (`0x34` do DObj, DObj em `gobj+0x74`) a partir de duas constantes por direção. As constantes são lidas com `lui 0x8019` + `lwc1 0xC230/0xC234/0xC238/0xC23C`, ou seja, `0x8019C230..0x8019C23C`. O comentário REMIX-TODO no port diz `0x8018C230`, o que contradiz o asm. Falta: confirmar se o RAM image contém esses floats (`remix_ram_image_read_word`), que hoje não foi verificado; a constante vem do código copiado do DK cargo. Se estiverem na imagem, desbloqueio S. Verificação: Marina em throw de chão e de ar, observar a virada.

## ft/ftremix/sheik/ftremixsheiknsp.c:352 (carga do spark)

**BLOCKED.** SheikSpecial.asm:1221-1225 (`charge_main_`) grava o nível de carga em `0x02A4` da struct do FTStruct do objeto spark (`lw a0,0x0B20(a3)` e `lw v1,0x0084(a0)`) a cada tick de carga, quando `0x0B20` não é zero. O port não tem objeto spark (o rotina de cor não carrega dados por lutador). Falta: um GObj spark com struct própria. Esforço M-L. Não verifiquei se a rotina visual lê `0x2A4`, então o efeito observável é UNKNOWN.

## gm/gmremixcamerapause.c:36 (lupa ao sair da pausa com Special Zoom)

**READY.** Camera.asm:886-888 (`unpause_camera_`) lê `Zoom.zoom_type` e, se for diferente de zero, põe a lupa em FALSE (`bnezl`, `lli v0, FALSE`). O `Zoom.zoom_type` está em `build/remixasm/remix.sym` (`80513b94`). O port guarda o tipo em `sFTRemixSpecialZoomType` (gmremixspecialzoom.c), que é estático e sem getter. Mudança: expor em gmremixspecialzoom.c um getter do tipo (ou `!= FTREMIX_SPECIAL_ZOOM_NONE`) e usá-lo em `gmRemixCameraRestorePauseHud`, retornando FALSE para a lupa. Esforço S. Verificação: smoke com Special Zoom ligado (`SSB64_REMIX_*`), zoom ativo e pausa por script de input, lupa oculta.

## gm/gmremixgameend.c:155 (linhas de resultados KOTH e Smashketball)

**BLOCKED.** Os valores existem (`gmremixgameendrows.h`, `gmremixgameendrowsrules.h`), mas `mnvsresults.c` não desenha as linhas extras. O asm é VsRemixMenu.asm:1007 (`add_row_to_results_`), com hook em `0x1554D8` para `0x80136338` (VsRemixMenu.asm:1009-1010). Falta: o desenho das linhas extras em `mnvsresults.c`. Esforço M. Verificação: KOTH ou Smashketball com full results ligado, conferir as linhas.

## gm/gmremixmenumusic.c:115 (IDs de BGM 17 e 18)

**BLOCKED.** `gmRemixMenuMusicTrackOf` cai no `default_bgm` para os valores 17 e 18 porque `sMenuMusicTrackBGM` não os mapeia. BGM.asm:626 e :629 usam `t1 = 17 (BOMBERMAN)` e `t1 = 18 (SONIC)`. Falta: o ID de BGM de cada faixa no port (a tabela de faixas do asm não foi lida). Desbloqueio S-M, se o ID estiver na tabela de BGM. UNKNOWN para o ID exato. Verificação: opção Menu Music 17 e 18, ouvir a faixa.

## gm/gmremixspecialzoom.c:127 e :160 (grupos de stage no zoom)

**BLOCKED.** SpecialZoom.asm:1591-1600 (`temp_toggle_group_display_`) usa `Render.GROUP_TABLE` para ocultar e mostrar os grupos 1 e 2 do stage. As chamadas estão em SpecialZoom.asm:917, :922, :1125 e :1130. O port não tem a tabela de grupos de display. O port faz o FGM de ocultar stage (gmremixspecialzoom.c:162-163), mas não o toggle. Falta: um acesso aos grupos de display de Render. Esforço M-L. Verificação: KO zoom em partida, comparar screenshots com o stage oculto.

## gm/gmremixspecialzoomwallpaper.c:124 (rotação Y do wallpaper)

**UNKNOWN / BLOCKED.** `create_zoom_camera` (SpecialZoom.asm:236 em diante) deveria definir `dobj->rotate.vec.f.y` a partir do float em `0x80132704`. Não encontrei `132704` em nenhum arquivo do asm lido nem em `remix.sym`, então o valor não tem dono nomeado. Falta: identificar quem escreve o float e se ele está no RAM image. Esforço M se estiver no image. Efeito observável: rotação do fundo "Intro Fighters Clash" (Zoom.zoom_background_file, arquivo 0x04B).

## gm/gmremixtwelvecbrules.h:74 (CSS de 24 slots)

**BLOCKED.** O CSS Remix de 24 slots, em que o jogador escolhe os personagens, não existe no port. Falta: a tela do CSS de 24 slots, um recurso de engine. Esforço L. Não aprofundei o asm do CharacterSelect.asm, então os detalhes são UNKNOWN.

## gr/grcommon/grzebes.c:268 (cópia da câmera Y no hazard de ácido)

**NOT-OBSERVABLE (provável).** Hazards.asm:172-175 copia `0x8013EB1C` para `0x801313FC`. Busquei `13FC` em todo o asm: a única ocorrência que referencia `0x13FC` como leitura ou escrita é a própria escrita em Hazards.asm:175, sem leitor. Como a grep cobriu o texto literal, não há consumidor conhecido. Não exclui leitura calculada por outro caminho.

## gr/grremix/grremixbbh.c:43 (contador de fundo do laugh do Boo)

**BLOCKED.** Hazards.asm:8429-8435 (`boo_sound_`) lê o flag em `0x80131AD8` e só toca a FGM 0x0527 quando o valor é zero (`bnez t5, _end`). O port devolve FALSE sempre, então nunca toca. O stages/bbh.asm:35 diz que o contador "countdown" fica em `80131AD8`, mas o asm lido não tem um escritor do valor. Falta: o dono do contador no fundo da BBH (rotina de animação de bg não identificada). Efeito observável: o laugh do Boo está ausente. Desbloqueio M, após achar o escritor.

## gr/grremix/grremixcorneria.c:60 (argumento pai do laser esquerdo)

**UNKNOWN.** O comentário diz que o asm passa um registrador indefinido como pai do tiro esquerdo. Vi o disparo do Great Fox em Hazards.asm:2026-2100 (`great_fox_`), mas não tracei o `a0` do tiro esquerdo até o fim. Não é possível classificar sem essa leitura.

## gr/grremix/grremixcorneria.c:61 (atributo do header 0x130 do laser)

**BLOCKED.** Hazards.asm:4598 lê `0x0130(t1)` do header do stage, o ponteiro para o arquivo de hitbox do laser. O header vem de `0x80131300` (Hazards.asm:4590-4594). O comentário de Hazards.asm (perto de 2060) diz que o header de Sector Z ganhou essa informação de hitbox, e o port usa `dGRSectorArwingWeaponLaser2DWeaponDesc`. Falta: o id do arquivo de hitbox no header Remix de Sector Z (`stages/sector_z_remix.asm`) e a carga desse arquivo no port. Esforço M. Verificação: Corneria, comparar a hitbox do laser do Great Fox com o asm.

## Resumo

| Local | Classificação | Esforço |
|---|---|---|
| zcancel.c:67 | BLOCKED | M |
| zcancel.c:78 | BLOCKED | M |
| goemonusp.c:93 | READY provável (não confirmado) | S |
| marinansp.c:290/302 | BLOCKED (confirmar floats na imagem) | S após confirmação |
| sheiknsp.c:352 | BLOCKED | M-L |
| gmremixcamerapause.c:36 | READY | S |
| gmremixgameend.c:155 | BLOCKED | M |
| gmremixmenumusic.c:115 | BLOCKED (ID de BGM) | S-M |
| gmremixspecialzoom.c:127/160 | BLOCKED | M-L |
| gmremixspecialzoomwallpaper.c:124 | UNKNOWN/BLOCKED | M |
| gmremixtwelvecbrules.h:74 | BLOCKED | L |
| grzebes.c:268 | NOT-OBSERVABLE (provável) | - |
| grremixbbh.c:43 | BLOCKED | M |
| grremixcorneria.c:60 | UNKNOWN | - |
| grremixcorneria.c:61 | BLOCKED | M |

Observação: o briefing do ai-memory para o projeto BattleShip falhou ("project 'BattleShip' not found"), e não há `.ai-memory.toml` no repositório. As regras globais foram lidas. As regras do projeto não foram carregadas.
