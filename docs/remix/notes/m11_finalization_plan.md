# Plano de finalização do Smash Remix no BattleShip

Criado em 2026-11-03. Fonte do estado: `docs/remix/ROADMAP.md` (CP10a, ~87%), o histórico de trabalho de outubro e novembro
e as triagens `docs/remix/notes/t2_11_triage_{a..d}.md`. O ROADMAP é o resumo vivo; este arquivo guarda os diagnósticos de cada fase.

Regra de execução: exploração e leitura por subagentes com `anthropic/claude-haiku-5-5` (economia de tokens); o lead implementa,
compila, roda e confere em tela. Uma task por vez, checkpoint por fase, commits sem trailers de atribuição. Ao sair do modo de
plano, este plano vira o milestone **M11 Finalização** no ROADMAP.

Critério de fim (decidido em 2026-11-03): "Remix jogável" com todas as features do Remix presentes. Adaptações ao port são
aceitas, desde que documentadas. As decisões estão no fim do arquivo.

---

## Fase R: recuperar o trabalho interrompido (antes de tudo)

### R1. Trabalho do lead, não commitado (feito)

Ponto de partida: fonte Kabel real decodificada do `.o2r` do usuário e teste do decodificador, já commitados. Depois disso, sem commit:

| Arquivo | O que mudou |
|---|---|
| `port/remix/remix_hud_text.{h,cpp}` | mais slots (antes 32), alinhamento LEFT/RIGHT/CENTER, função de largura do `draw_string_`, corte do overscan de 10 px lido do CVar como o `Gui.cpp` |
| `port/remix/remix_font.{h,cpp}` | `Draw` com escala e altura de glifo |
| `decomp/src/mn/mnvsmode/mnremixvsstatspages.c` | página de stats reescrita (rótulo à esquerda, células à direita, retângulos sem contorno), faixa de slots nova |
| `decomp/src/mn/mnvsmode/mnremixvsstatsview.c` | constantes do prompt "Press A" |
| `decomp/src/sc/sc1pmode/sc1premixhrcrules.{c,h}`, `sc1premixhrcrun.c`, `tests/remix/test_1p_hrc_rules.c` | layout do contador do HRC |
| `decomp/src/sc/sc1pmode/sc1ptrainingremix.c` | slot e posição do HUD do Training |
| `decomp/src/if/ifremixpauselegend.c` | argumentos de `Render.draw_string` do `Pause.asm` |
| `decomp/src/gm/gmremixtwelvecbhud.c` | textos reais do asm (Stocks Remaining etc.), slots 32+ |
| `decomp/src/gm/gmremixgameendrows.c`, `sc1premixmultiman.c` | parâmetros de texto |

Estado medido: `ssb64` linkou; `run_remix_mode_tests` com **928 checagens, 3 falhas** em
`tests/remix/test_1p_hrc_rules.c:160,163,182` (176 contra 177; 108 contra 109; 1 contra 2). Nada conferido em tela.

Revisão Haiku do diff (2026-11-03):
- **Conforme a spec:**
  - overlay com alinhamento, `put_fit`, `remix_hud_rect_put`, 128 rótulos, 16 barras, 48 retângulos;
  - corte de overscan feito (`OverscanMargin`/`GameRect` leem `.Overscan.*`, pixel-perfect desliga);
  - posição do glifo por altura `top + (14 - h) * scale`;
  - Match Stats (prompt CENTER 160,218 escala 0.8125; páginas LABEL_X 24, CELL_X 184, FIRST_Y 30);
  - Reset Count do Training (99.5/221, y 25);
  - Multi-Man (`%d` LEFT 50,18 escala 0.9);
  - posições do HRC (278,44 e 282,47);
  - coordenadas da pausa (226/203, 243/205, 72/140, 94/154, 94/170);
  - textos do 12CB (Stocks Remaining, pools, Best com fit 72, TKOs, Game NN).
- **Falhas do HRC, diagnóstico:**
  - linha 160: o teste está errado (o código dá 176.5);
  - linha 182: o teste está errado (o passo do código dá 2);
  - linha 163: o **código** está errado. Só o X usa `steps = tics + 1`; escala e y devem usar `tics`.
    Os valores certos são rótulo 1.075, contador 1.35, y 66.5.
- **Ainda texto em vez de sprite** (dependem do I2): rótulos Times/FGs dos resultados, glifos A/B/R/Z/setas da pausa e da
  Match Stats, nome do item do Training, ícone e "X" do Multi-Man.
- **Faltando nos chamadores migrados:**
  - legenda de música 0x0F da pausa, e a câmera começa escondida;
  - variante Dragon King do Multi-Man (Y 212);
  - itens §5.2–5.5 do Training (Press Z, L legend, D-pad, Hold A);
  - condição do Training aproximada por `game_status != Pause`;
  - zebra `stripe_on` das páginas não conferida;
  - sombra no fallback ImGui.
- **Não migrados:**
  - `gmremixkothhud.c` (barra em y 180 e X por slot; a spec quer y 188, moldura em 180, X = 40 + 70 × porta);
  - `port/remix/remix_music_title.cpp` (ainda `remix_overlay_show_text`; a spec quer (160,166) e (160,178) CENTER com retângulo
    0x34343494).
- **Slots:** sem colisão dentro da mesma cena. gameend 20–23/30 reusa os da pausa e do HRC em cenas diferentes; vale um
  mapa de slots documentado no header.
- **Passos do 12CB:**
  - feitos: 1 (overscan) e 2 (largura máxima e migração do HUD);
  - parciais: 10 (falta Character Set, nomes de conjunto, setas) e 14 (falta RESET e coroa);
  - não iniciados: 3–9, 11–13 e 15–19.

Passos:
1. Corrigir o off-by-one de escala e y no `sc1premixhrcrules.c` e as expectativas das linhas 160 e 182.
2. Build e testes verdes; conferir em captura cada tela migrada (resultados, Match Stats, pausa, HRC, Multi-Man, Training e o
   texto central do CSS 12CB).
3. Documentar o mapa de slots por cena em `remix_hud_text.h`.
4. Commit no `decomp`, bump do submódulo, commit no outer (com `f15_hud_text_params.md` e `f15_twelvecb_css_spec.md`).
   O resto (12CB passos 3–19, KOTH, banner de música, sprites) segue na Fase 2.

### R2. Subagente "Research Remix button-sprite drawing" (interrompido)

- Parou decodificando o botão A do `.o2r` (a leitura do zip em Python passou de 120 s, foi para background e foi morta).
- O arquivo alvo `docs/remix/notes/f15_button_sprites.md` **nunca foi escrito**.
- Refeito por Haiku (2026-11-03). Na Fase R, gravar o resultado abaixo nessa nota.
- Destrava: sprite do botão A da Match Stats, glifos de botão nas legendas da pausa e do Training, setas/D-pad do CSS, ícone
  de polígono do Multi-Man, moldura do KOTH (infra I2 da Fase 1).

Resultado:
- **Arquivos (todos presentes no `.o2r`):**
  - 0xC5: o Remix **não** sobrescreve o id; é `reloc_interface/IFCommonBattlePause` (`RelocFileTable.remix.cpp:210`).
  - 0xA06 = `reloc_misc/file_2566`.
  - 0x19 = `reloc_fighters_common/FTStocksZako`.
  - 0x14F3 = `reloc_misc/file_5363`.
- **Membro do zip:** os dados começam em `member[80:]`; o tamanho fica em `member[76]` (u32 LE, `len - 80`). Isso foi inferido
  dos dados, sem ler `RelocFileFactory.cpp`; confirmar.
- **Sprites:** campos big-endian; Bitmap em `sprite - 0x10`; o stride da linha é `width_img`, não `width`.

  | Sprite | Arquivo @ offset | Tamanho | Formato | Texel |
  |---|---|---|---|---|
  | A | 0xC5 @0x958 | 13x13 | IA8 (stride 16) | 0x878 |
  | B | 0xC5 @0xA88 | 13x13 | IA8 (stride 16) | 0x9A8 |
  | L | 0xC5 @0x18C8 | 16x12 | IA8 | ? |
  | R | 0xC5 @0xCF8 | 16x12 | IA8 | ? |
  | Z | 0xC5 @0xBD8 | 9x15 | IA8 | ? |
  | PLUS | 0xC5 @0x4D8 | 8x8 | I4 | ? |
  | L_THIN | 0xC5 @0x1B10 | 16x12 | IA8 | ? |
  | "Legend" | 0xC5 @0x19F8 | 42x9 | I4 | ? |
  | D-pad | 0xA06 @0x218 | 16x16 | RGBA16 | 0x8 |
  | setas | 0xA06 @0x688/0x1958/0x1A88/0x1BB8 | 13x13 | IA8 | 0x5A8/0x1878/0x19A8/0x1AD8 |
  | polígono | 0x19 @0x80 | 8x8 | CI4, LUT em 0x50 (não confirmado) | 0x8 |
  | moldura KOTH | 0x14F3 @0xAA8 | 64x16 | IA8 | 0x698 |

- **Cor:**
  - `TEXTURE_INIT_` (0x800CCFDC) = `lbCommonMakeSObjForGObj`; `TEXTURE_RENDER_` (0x800CCF00) = `lbCommonDrawSObjAttr`. O Remix
    usa o caminho SObj vanilla.
  - O argumento color vira a PRIM (`sprite.red..alpha`) e o palette vira a ENV (`sobj->envcolor`). É inferência: confirmar os
    offsets +0x28 e +0x60 na `struct SObj`.
  - Combiner de IA (`lbcommon.c:2907-2909`): `RGB = ENV + (PRIM - ENV) * I` e `A = A_tex * PRIM.a`.
  - Ressalva do lead: o I e o A do IA8 são nibbles de 4 bits, expandidos ×17. O relatório os dividiu por 255 sem expandir.
  - Falta conferir o combiner do I4 (PLUS, "Legend") e do CI4 em `lbCommonPrepSObjAttr`.
  - Exemplo do botão A da Match Stats: tint 0x50A8FFFF, palette 0x0010FFFF.
- **Decisão: SObj nativo, não atlas.**
  - Padrão a copiar: `decomp/src/if/ifremixcombometer.c`:
    - `lbRelocGetExternHeapFile` (l. 435), `lbRelocGetFileData` (281-282);
    - `lbCommonMakeSObjForGObj`, que já faz o fixup e desfaz o swizzle do TMEM;
    - `gcAddGObjDisplay(..., lbCommonDrawSObjAttr, link 0x18, ...)`.
    - Cor por `sobj->sprite.red..` e `sobj->envcolor`; ocultar com `SP_HIDDEN`.
  - Motivos: as coordenadas do asm já são de SObj; os 4 formatos, o LUT e o swizzle já estão tratados; a cor sai igual à do
    Remix.
  - Custo: os chamadores atuais desenham a cada frame (`remix_hud_text_put`). Com sprites, cada HUD precisa de um GObj vivo
    com ciclo de vida (criar, destruir ou ocultar), por exemplo na legenda da pausa.
- **I2 na Fase 1:** um módulo pequeno (ex. `ifremixsprite.c`) com:
  - carga do arquivo de extensão;
  - criação de SObj com tint e palette;
  - ocultar e mostrar.

  Teste de unidade da tabela de offsets e formatos, e o botão A da Match Stats como primeiro consumidor em tela.
- **Antes de implementar:**
  - O 0xC5 provavelmente já é carregado pela pausa vanilla. Reaproveitar o arquivo carregado em vez de alocar outra cópia
    (conferir em `ifcommon.c`).
  - O texto continua no overlay ImGui, por cima do jogo; os sprites vão para a DL do jogo. Conferir que nenhum texto Kabel
    precisa ficar sob um sprite.

### R3. Subagente "Build macOS and Linux packages" (interrompido)

- **macOS (feito no commit `fb50770`):** árvore `build-macos-release` (Release, `NON_PORTABLE=ON`, GLEW estático), script
  **não versionado** `scripts/macos/package-remix-contained.sh`, gerou `.app`, `.dmg` (71 MB) e `.zip` (68 MB); smoke da `.app`
  extraída do zip com rc=0.
- **Linux:** imagem podman `ssb64-linux-jammy-amd64` (Ubuntu 22.04 amd64 emulado), binário em
  `build-linux-release/BattleShip-Remix` (log termina em `BUILD_DONE`, também do `fb50770`). O empacotamento AppImage
  (`package-in-container.sh`) foi escrito mas **nunca rodou**. Os scripts estão no scratchpad em `/private/tmp/...`, que é volátil.

Auditoria Haiku (2026-11-03):
- **macOS:**
  - O `.dmg` e o `.zip` não existem em lugar nenhum (`dist/`, `build-*`, Desktop, Downloads). Sobrou só a `.app` de 21:32.
  - A `.app` está completa: binário arm64 de 22,8 MB, `.o2r` de 59,9 MB com o recipe, `f3d.o2r`, ícone, gamecontrollerdb,
    licenças, fontes, assinatura ad-hoc.
- **Linux:**
  - `build-linux-release/BattleShip-Remix` é ELF x86-64 PIE, versão `2.0.1-port.fb50770`, log terminando em `BUILD_DONE`.
  - O AppImage nunca foi gerado.
  - O Dockerfile baixa `appimagetool` e `linuxdeploy` **sem sha256 fixado**, o que viola a regra do toolchain.
  - `DIST_VERSION` não tem default.
- **Política do `.o2r`:**
  - `build_and_roms.md` só proíbe ROM e `.o2r` no git.
  - Os scripts oficiais `scripts/package-macos.sh` (l. 22-27) e `scripts/package-linux.sh` (l. 17-22) **excluem o `.o2r` de
    propósito**, porque ele é derivado da ROM. O script do Remix e o `package-in-container.sh` o embutem.
  - O próprio cabeçalho do script do Remix diz "teste privado, não redistribuição".
  - Conclusão: pacote com `.o2r` só para uso pessoal. Para distribuir, a extração tem de acontecer no primeiro uso, como no US.
- **Script do macOS:** usa `mktemp -d` sem `TMPDIR` (falhou no sandbox) e não tem `trap` de limpeza do iconset.
  `build/BattleShip-Remix.o2r` é de 16/10: conferir se o recipe bate com o Torch atual antes de empacotar.
- **CI:** `.github/workflows/release.yml` usa a matriz `[us, jp]`, sem `remix`. Os dois scripts oficiais já aceitam
  `SSB64_VERSION=remix`.
- **Defasagem:** os pacotes são do `fb50770`, 2 commits atrás do HEAD `11d791d`, e não têm o HUD Kabel nem o diff do R1.
- Comando reconstruído do AppImage (não executado):
  `podman run --rm -v $REPO:/src:ro -v $REPO/build-linux-release:/out -v ssb64-linux-work:/work -v <scripts>:/scripts:ro
  -e DIST_VERSION=... -e ARCH_TAG=x86_64 localhost/ssb64-linux-jammy-amd64 bash /scripts/package-in-container.sh`

Passos:
1. Copiar o Dockerfile e os scripts do scratchpad para `scripts/linux/` antes que o `/private/tmp` seja limpo. Fixar o sha256
   de `appimagetool` e `linuxdeploy`, e dar default ao `DIST_VERSION`.
2. Versionar `scripts/macos/package-remix-contained.sh` com `TMPDIR` controlado e `trap`, como pacote de teste de uso pessoal
   (decisão 6). O fluxo sem `.o2r` fica para a Fase 7.
3. Documentar o fluxo em `docs/remix/build_and_roms.md`.
4. Os pacotes finais saem só na Fase 7, de um commit limpo, com o `.o2r` regenerado ou conferido.

### R4. Outros subagentes da sessão

- Terminaram com relatório (não versionado):
  - análise de slow frames do Switch;
  - freeze no Training;
  - dois crashes (Status Buffer, já corrigido para 1024);
  - tamanho do painel das variantes J;
  - auditoria de UI provisória (virou o T8.3).
- Recomendações ainda abertas vão para a Fase 7:
  - `fflush` do `ssb64.log` antes do `abort()` em `debug.c:1023`;
  - resumo de hitch por cena;
  - log de `lbReloc` lento;
  - log de escala do painel J.
- **Varredura Haiku** :
  - 77 de 79 subagentes de outubro terminaram com relatório; só faltavam os dois conhecidos (R2 e R3).
  - Os 40 notes citados existem, exceto `f15_button_sprites.md`.
  - Única task de background morta: `blloq55yx`, a leitura do `.o2r` do R2.
  - O lead não deixou lista de tarefas; o último trabalho foi o HUD do 12CB e as linhas de resultado.
- **Três subagentes interrompidos em outubro.** Conferir se o trabalho foi refeito depois:
  1. "Port leftover Kirby-copy routines A" (limite semanal, 13/10): faltavam 4 rotinas da Sheik copiada pelo
     Kirby (3 colisões de chão e `air_shoot_collision_`). Conferir `ftremixkirbycopy`/sheik e o T2.6.
  2. "Reconstruct Remix FGM tables for Torch" (interrompido pelo usuário, 12/10). Provavelmente substituído: o
     T3.2 diz que `fgm_tbl`/`fgm_ucd` já são autossuficientes. Confirmar e descartar.
  3. "Port Remix Crash DSP" (interrompido pelo usuário, 13/10): faltava a nota do Crash DSP (11 rotinas de `dig`).
     O código existe (Crash Dig no T2.11 onda 2); conferir se a nota existe.
  - `docs/remix/notes/f7_GFX.md` foi citado e não existe. Conferir se foi renomeado (o doc do T2.1 está em `docs/bugs/`).
- **Pendências `REMIX-TODO(AI)` dos relatórios de outubro:** B2 sem P10, B1 com P02 e P06 parcial, A4c com P14, A4b com P05 e
  Banjo P06. Já estão na Fase 6.

### R5. Fechamento da Fase R

- `tools/gen_remix_parity.py` com o STATUS atualizado (o `parity.md` é de 31/10 e ainda lista `DKMode`, `VsDemo`, `SFXReplace`
  e `timeouts` como todo).
- O handoff do ai-memory está velho (14/10): atualizar a memória `project_remix_port_checkpoint.md`.
- **Aceite:** árvore limpa, testes verdes, `parity.md` atual, nota `f15_button_sprites.md` gravada, scripts de pacote versionados.

---

## Fase 1: infraestrutura que destrava vários itens

| # | Peça | Destrava |
|---|---|---|
| I1 | Mostrar e ocultar grupos de display (`Render.toggle_group_display_`, `GROUP_TABLE`) | estágio oculto do SpecialZoom; grupos 0x16 e 0x0D da Match Stats; legendas da pausa |
| I2 | Sprites dos arquivos de extensão do Remix (0xC5, 0xA06, 0xFE, 0x12+0x2048, 0x153, 0x19, 0x14F3) por SObj nativo (decidido no R2) | botão A; glifos de botão; ícones do handicap; bandeiras J/E e D-pad do CSS; nome do item no Training; título do 12CB; ovo do Cruel Z-Cancel; moldura do KOTH |
| I3 | Texto Kabel completo (regra do `draw_string_`, altura do glifo, `apply_max_width_`, 128 ou mais slots) | todos os chamadores do T8.3 |
| I4 | Estado de BGM em runtime (atual, padrão, vanilla) e troca com fade | BGM do Time Twister; música alternativa da pausa; troca ao vivo no Training; Menu Music 17 e 18 |
| I5 | Ferramenta endereço N64 para símbolo (`remix.sym` mais mapa US), com teste | 0x80131580 (KO do chefe do 1P); 0x80131AD8 (risada do BBH); 0x80132704 (rotação do zoom); 0x800F9C8C/0x800F9CC4 (lag do DragonKing); 0x8014A24C (fthrow do Bowser); floats 0x8019C230 da Marina |
| I6 | Grupo 2 do estágio (wallpaper e objeto de fundo) e troca de rotina de câmera (`zoom_to_max_`) | T1.5b do Time Twister e do Scuttle Town |
| I7 | Troca de imagem ou paleta por parte de modelo (`MODEL_PART_IMAGE_INIT_`) | costumes além de `num_costumes`; cor do ovo do Yoshi no escudo; Sonic clássico |
| I8 | Emulador de referência (decisão 2), detalhado abaixo | comparação visual do T8.3 e da Fase 8; valores que hoje só saem da leitura do asm |

**Aceite:** regra pura testada quando houver, e um consumidor visto em tela por peça.

### I8. Emulador de referência

- **Base:** o setup que já funcionou em `docs/intro_divergence_triage_2026-07-30.md`, montado na época em outra máquina:
  - `mupen64plus-core` com `NO_ASM=1` (interpretador puro) e `mupen64plus-ui-console`;
  - `mupen64plus-rsp-cxd4` (LLE) e `angrylion-rdp-plus` (RDP por software, exato);
  - captura sem janela com `--testshots`.
- **Setup:** script novo `scripts/macos/setup-ref-emu.sh`:
  - fontes em commits fixos com sha256, compilação em `.toolchain/ref-emu/`;
  - nada fora do `.toolchain/` e nada no git (é GPL; o doc já avisa que não pode entrar no repo);
  - dependências (SDL2 já em cache, libpng, zlib) com URL e sha256 fixos, como em `setup-host-deps.sh`.
- **Captura:** script `scripts/ref-emu/capture.sh <rom> <frames> <saida>` que roda o `remix.z64` com Expansion Pak.
  - Entradas roteirizadas para chegar às telas. Antes de escrever o roteiro, conferir se o ui-console aceita entrada gravada
    ou se é preciso um plugin de input simples.
- **Cuidado do doc:** o índice de frame do emulador conta só os frames apresentados, e o do port conta todo VI. Alinhar pelo
  conteúdo, nunca pelo índice.
- **Aceite:** uma captura da Match Stats e uma da pausa do Remix ao lado das do port, registradas em `docs/remix/notes/`.
- **Risco:** o angrylion é lento. Se ficar inviável para chegar às telas, usar um RDP mais rápido só para navegar e o angrylion
  no frame final. Se nenhum compilar no arm64, registrar e voltar à leitura do asm.

## Fase 2: T8.3, visual dos modos (bloqueia "Remix jogável")

1. **CSS do 12CB** pelos 19 passos de `f15_twelvecb_css_spec.md`:
   - grade 3x8 com slide-in, retrato derrotado e indicador de variante;
   - título 0x2048, dois painéis, "Stocks Remaining", contador de 0 a 99 com trava;
   - Character Set, Best Character, coroa;
   - P13 (reset de pausa), P15 (heap em 10 jogos), P16 (dois jogos seguidos pelo CSS).
2. **Match Stats:** sprite do botão A em (160,218), página em Kabel, grupos ocultos (I1).
3. **Restantes:** Multi-Man, HRC, Training ("Reset Count", "Press Z", legendas), linhas Times/FGs dos resultados (sprites
   0xE648 e 0xE9E0 em y=124), legenda da pausa (sprite R e 0xC5+0x19F8), medidor do KOTH (moldura 0x14F3+0xAA8, x = 40 + 70 × porta),
   banner de música (retângulo 0x34343494, faixa e jogo).

**Aceite:** nenhum chamador de `remix_hud_text_*` nos modos, uma captura por tela.

## Fase 3: estágios e gameplay (T1.5b, T2.9b, T2.11)

- **Prontos, pequenos:**
  - Size no Pikachu DSP, Ness DSP/USP e efeitos de entrada;
  - onda do Banjo USP, render da nuvem do Goemon;
  - tabela de hazard das nuvens do Yoshi, som do chomp do Rainbow Road;
  - id de tela do Dream Land, palavra inteira do laser do Metallic Madness, cópia da bomba no canhão do Pirate Land;
  - TODO obsoleto do Golden Gun.
- **Depois do I5:** Seismic Toss do Bowser e do Giga Bowser, lag do DragonKing, KO do chefe do 1P, risada do BBH, virada da Marina.
- **Depois do I6 e do I4:** T1.5b completo.
- **Médios:**
  - fling de item no spin do Crash;
  - DpadControl (toque parcial, timer, ações por personagem);
  - escudo modo Costume e ovo do Yoshi (I7);
  - Size restante (ovo ao sair, tiro da Samus, espada do Kirby, chute do Falcon, `kirby_ausp_`);
  - spark da Sheik, hitbox 0x130 da Corneria, hitboxes do chomp, f20 do Japes.
- **T2.9b:** causa do guard de TransN dos polígonos.

**Aceite:** `REMIX-TODO` fora de IA em 0. Quando a forma exata do Remix não couber no port, a feature entra adaptada, com o
desvio documentado em `docs/bugs/` ou na nota da área (decisão 1). Feature ausente não conta como sobra aceitável.

## Fase 4: CSS, stage select, painel, telas (M5 e T8.1)

- **CSS:**
  - costumes extras (I7), C no team battle;
  - D-pad e bandeiras J/E (I2), bookend de bônus, `recent_randoms`;
  - logos VS dos novos, lutador com 0 costumes;
  - ícones do handicap, seletor de CPU até 10 visto;
  - `scale.vec.f.y` repetido.
- **Stage select:** texto do tipo de variante, roulette do RANDOM, layout tournament.
- **Painel por porta:** D-pad Map, DpadControl J/escudo, Practice_1P com L-retry, Start With no Training, Poison Heal, trava de
  stamina, nomes das tags.
- **Telas:**
  - "Smash Remix v2.0.1" no título;
  - glifos 0x50 a 0x61 dos créditos (`REMIX-TODO(credits-glyphs)`, `scstaffroll.c:20`);
  - speed assist dos créditos: **n/a** (decisão 5). A aceleração por Start (0.11) já está portada em `scstaffroll.c:1476`.
    Trocar o `REMIX-TODO(credits-speed-assist)` de `scstaffroll.c:1475` por uma nota de n/a (o assist depende da tag "afk",
    que o port não guarda) e registrar em `docs/remix/menus.md`.
  - Gallery com 40 entradas;
  - RANDOM ALL, legenda do item switch;
  - logos de transição, timeouts e regras de torneio vistos em tela.
- **Outros:** Skip Start Cheer, sons JP do Link, bytes restantes do Cheats, 7 grupos do SinglePlayerEnemy, gate do
  SingleButtonMode no Training.

## Fase 5: modos (M6)

- **Tag Team:** título e anúncio do CSS, variantes e Sonic clássico, heap com troca repetida, Stock Mode LAST.
- **Tug of War e Stamina:** `REMIX-TODO(stamina)` do T6.D5.
- **KOTH:** polilinhas, cores de equipe, rótulo "Goal".
- **Smashketball:** tipo no CSS, exigir dois times, câmera com a bola, torcida, 3 e 4 jogadores.
- **1P:**
  - recordes, título e Random do CSS do Bonus 2; Bonus 3;
  - fim do Giga Bowser;
  - cards do Remix Team e do Piano, textura do Metal Mario Bros;
  - Rest Area e ícones de série do All-Star;
  - `costume_fix`, `match_begin_flag`, `pause_exit`, final próprio.
- **Training:** Custom Spawn, música ao vivo (I4), dummy por porta, nome do item na mão (I2), guardas de um lutador.

## Fase 6: IA (M7)

- `cpu_post_process` por lutador, `auto_jab_grab_`, AERIAL_CHASE, PUFF_DSP.
- PREVENT vanilla, Banjo P06, Kirby P14, P11 de Sheik/Peppy/Bowser/GBowser.
- As 30 rotinas `cpu_*`.
- `_improve_recovery_drift`.
- Refresh do Default CPU LVL, handicap 40, conflito do `CpuLevel9` com o nível 10.

**Aceite:** `REMIX-TODO(AI)` em 0, sem regressão da recuperação (base 181/222 no nível 9).

## Fase 7: bugs abertos e distribuição

- **Bugs:**
  - Poly Young Link ausente no CSS até o primeiro reset;
  - Metal Mario minúsculo no Training;
  - stutter do Switch (`walked past registered DL range`, token 0x57EE stale);
  - OOB do `PEACH_TURNIP_GFX`, zoom 1,70 do Giga Bowser;
  - flake rc 139 do Taltal.
- **Instrumentação do R4:** `fflush` antes do `abort()`, resumo de hitch, `lbReloc` lento.
- **Pacotes de teste (decisão 6, agora):** o script do macOS e o AppImage com o `.o2r` embutido, marcados como uso pessoal no
  nome do arquivo e no cabeçalho dos scripts.
- **Pacotes finais (decisão 6, publicáveis):**
  - macOS `.app`/`.dmg`/`.zip`, AppImage x86_64, `.nro` do Switch, **sem** `.o2r`;
  - primeiro, confirmar que a extração na primeira execução funciona para o Remix (o jogo pede a ROM do Remix do usuário e gera
    o `BattleShip-Remix.o2r`), como no US. Se faltar algo, implementar e testar;
  - `remix` na matriz do `.github/workflows/release.yml` (decisão 7), usando `scripts/package-macos.sh` e
    `scripts/package-linux.sh`, que já aceitam `SSB64_VERSION=remix`;
  - doc de instalação: qual ROM é preciso ter (vanilla mais o patch do Remix 2.0.1) e como o jogo faz a extração.

## Fase 8: verificação final e encerramento (M9 e M10)

1. **Regressão:** 175 estágios, matriz de especiais 95 × 3, sweep de itens, 4 jogadores em cada modo pelo CSS e Training,
   build vanilla, testes de unidade.
2. **Usuário:** T9.1b (escuta e visual), T10.2 (Switch).
3. **Comparação com a referência:** capturas do emulador (I8) contra o port em cada tela de modo, CSS, stage select, pausa e
   resultados. Cada diferença é corrigida ou documentada como adaptação.
4. **Fecho:**
   - `parity.md` só com `ported`, `adapted` (desvio documentado) ou os quatro `n/a` aprovados;
   - T9.3 (relatório "Remix jogável" com a lista de adaptações);
   - checkpoint final.

---

## Fechados como n/a (OK do usuário em 2026-11-03)

- Emblema e golpes dos 18 extras no Character Data (o asm do Remix também não os tem).
- Overlay de Practice do Spawned Fear (sem efeito observável).
- Advanced Hurtbox e patches de debug do `Hitbox.asm` (só com toggle de debug).
- Speed assist dos créditos (`Credits.asm:1310`): só liga com a 2ª tag de jogador "afk", que o port não guarda. A aceleração por
  Start continua portada.

## Decisões do usuário (2026-11-03)

1. **Critério de fim:** "Remix jogável" com **todas as features do Remix presentes**.
   - Adaptações ao port são aceitas, desde que a feature exista para o jogador e o desvio fique documentado.
   - `n/a` só para os quatro itens acima.
   - No `parity.md`, todo arquivo do Remix termina como `ported` ou `adapted` (com o desvio documentado), nunca `partial`.
2. **Emulador de referência:** aprovado, em `.toolchain/`, fora do git (ver I8 na Fase 1).
3. **Execução:** subagentes Haiku para leitura e specs; o lead implementa, compila e confere em tela. Worktrees só para frentes
   realmente independentes (ex. estágios).
4. **M11:** aprovado. É a primeira edição ao entrar no agente de build.
5. **n/a:** aprovado para os quatro itens, incluindo o speed assist dos créditos (reconfirmado pelo usuário).
6. **Pacotes:** com o `.o2r` embutido (uso pessoal) para testes agora; sem o `.o2r`, extraído da ROM do usuário no primeiro
   uso, para os pacotes finais publicáveis.
7. **CI:** incluir `remix` na matriz do `.github/workflows/release.yml`, com os pacotes do modelo sem `.o2r`.
