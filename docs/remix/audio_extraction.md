# Smash Remix: reconstrução dos bancos de áudio (ctl/tbl/sbk)

Prototipo: `tools/remix_dump_audio.py` (stdlib only, `python3`). Saída padrão: `build/remix-dump/audio/` (git-ignorado). Nenhum dado derivado da ROM foi escrito em caminho rastreado. Não há alteração no port, no `.o2r` nem no CMake.

Comando:

```
python3 tools/remix_dump_audio.py [--rom baserom.remix.z64] [--out build/remix-dump/audio]
```

Arquivos gerados: `B1_sounds1_ctl.bin`, `B1_sounds2_ctl.bin`, `B1_sounds1_tbl.bin`, `B1_sounds2_tbl.bin`, `S1_music_sbk.bin`, `manifest.json` (layout e números de validação).

## Resultado

- Os dois bancos parseiam limpos após a reconstrução: 0 ponteiros fora de faixa, 0 referências a lacunas não mapeadas, 0 violações de sanidade (envelope, keymap, book, wavetable).
- Música: 71 instrumentos no header, 70 parseados (índice 0 é NULL), 326 sons, 324 wavetables ADPCM.
- SFX: 1 instrumento, 1137 sons, 1137 wavetables ADPCM, 815 samples além do TBL vanilla (bate com os 815 SFX novos de `docs/remix/audio.md` seção 1.4).
- Tamanhos: `B1_sounds1_ctl` 8 097 072 bytes; `B1_sounds2_ctl` 8 070 672 bytes; `B1_sounds1_tbl` 17 183 478 bytes; `B1_sounds2_tbl` 24 680 728 bytes; `S1_music_sbk` 14 712 768 bytes.

## Método

1. **Mapeamento RAM para ROM.** A imagem de código do Remix fica em RAM `0x80400000` = ROM `0x2C00000`, com `ROM = RAM - 0x7D800000`. O limite superior usado é `0x3000000`, início de `MIDI_BANK`. Conferido nos bytes: o offset de bank `0x3C9EA0` somado à base RAM `0x800472D0` dá `0x80411170`, que vira ROM `0x2C11170`, onde está o header de 71 instrumentos.
2. **Ctl como imagem de runtime.** Cada ctl é DMAed para uma base RAM (`INST_CTL_TABLE_PC = 0x800472D0`, `CTL_TABLE_PC = 0x8004D9F0`), e todos os offsets são relativos a essa base. O blob reconstruído começa nessa base e contém três faixas:
   - `[0, len_vanilla)`: o blob vanilla da ROM (música em `0x252B560`, SFX em `0x26485F0`).
   - `[RAM 0x80400000 - base, + 0x400000)`: a imagem de código, copiada de ROM `0x2C00000..0x3000000`.
   - o resto: zeros, por não ser backed por ROM. Referência a essa faixa é contada como não resolvida.
3. **TBL.** Os offsets de wavetable são relativos à base do TBL (`INST_SAMPLE_DATA` e `RAW_SAMPLE_DATA`, lidos da tabela de ponteiros da ROM em `0x3D750..0x3D768`). O TBL reconstruído é uma fatia contígua da ROM a partir dessa base, até o último byte referenciado (passe 1 calcula o fim máximo). Samples novos ficam depois dos dados de música, em `0x3000000+`, por isso a fatia vai até `0x3594F76` (música) e `0x3DE1AA8` (SFX).
4. **SBK.** Fatia contígua a partir de `0x2504750` até o fim da última entrada. Os offsets das entradas são relativos ao início do SBK. A última entrada termina em `0x330C710`, conferindo com o relatório anterior.
5. **Validação.** `BankParser` é um port Python, com verificação de limites, de `port/bridge/audio_bridge.cpp`. Em vez de substituir silenciosamente ponteiros inválidos por NULL, ele conta cada caso. Também confere o book ADPCM (order e npredictors, limite 8).

## Layout final

| Blob | Base RAM | Origem na ROM | Tamanho (bytes) | Mapeamento |
|---|---|---|---|---|
| `B1_sounds1_ctl` | `0x800472D0` | vanilla `0x252B560` (len `0x6720`) + código `0x2C00000..0x3000000` | 8 097 072 (`0x7B8D30`) | vanilla em `[0, 0x6720)`, código em `[0x3B8D30, 0x7B8D30)`, zeros no meio |
| `B1_sounds2_ctl` | `0x8004D9F0` | vanilla `0x26485F0` (len `0xFBA0`) + código | 8 070 672 (`0x7B2610`) | vanilla em `[0, 0xFBA0)`, código em `[0x3B2610, 0x7B2610)`, extensão SFX em `0x3C5810` (= ROM `0x2C13200`) |
| `B1_sounds1_tbl` | n/a | ROM `0x2531C80` até `0x3594F76` | 17 183 478 (`0x10632F6`) | base de wavetable = offset relativo a `0x2531C80` |
| `B1_sounds2_tbl` | n/a | ROM `0x2658190` até `0x3DE1AA8` | 24 680 728 (`0x1789918`) | base de wavetable = offset relativo a `0x2658190` |
| `S1_music_sbk` | n/a | ROM `0x2504750` até `0x330C710` | 14 712 768 (`0xE07FC0`) | entradas `(offset, size)` relativas ao início do SBK |

Observação sobre o `B1_sounds1_ctl`: os offsets dos bancos novos de música (`0x3BEDA8..0x3C9E78`) caem na faixa de código, e os 42 instrumentos vanilla caem no blob vanilla.

## Números de validação

Fonte: `manifest.json`, seção `validation`.

| Métrica | Música (`B1_sounds1_ctl`) | SFX (`B1_sounds2_ctl`) |
|---|---|---|
| Instrumentos no header | 71 | 1 |
| Instrumentos parseados (não NULL) | 70 (+ percussion parseado) | 1 |
| Sons referenciados e parseados | 326 | 1137 |
| Envelopes / keymaps / books / wavetables | 215 / 326 / 324 / 324 | 1 / 816 / 1137 / 1137 |
| Loops ADPCM | 138 | 26 |
| Ponteiros NULL (offset 0) | 187 | 1112 |
| Registros na faixa vanilla / na imagem de código | sons 119 / 207, wavetables 117 / 207 | sons 322 / 815, wavetables 318 / 819 |
| Wavetables com base além do TBL vanilla | 207 | 815 |
| Fora de faixa (ponteiros) | 0 | 0 |
| Não resolvidos (caem em lacuna) | 0 | 0 |
| Violações de sanidade (envelope, keymap, book, tipo de wavetable, contagem negativa) | 0 | 0 |
| Book máximo (order / npredictors) | 2 / 4 | 2 / 4 |
| Amostras referenciadas com todos os bytes zero | 0 de 324 | 0 de 1137 |

SBK: header `S1`, 375 entradas, 375 dentro dos limites, 0 fora. O bloco vanilla de sequências (entradas 1..46) continua na ROM original.

Checagem pontual feita fora do script (não está no `manifest.json`): os tamanhos de sample batem com o fim do TBL (`base + len` máximo é igual ao tamanho do blob em ambos os casos).

## Pendências e riscos

- **Não há verificação em runtime.** A validação é estática. Não rodei o jogo nem comparei áudio. A suposição de que a imagem de código é DMAed sem modificação vem da coerência dos headers, não de um dump de RAM.
- **Tamanhos vanilla não verificados diretamente.** `0x6720` (ctl música), `0xFBA0` (ctl SFX), `0x116970` e `0x2DC1E0` (TBL) vêm de `docs/remix/audio.md`. O header e o bank pointer foram conferidos, mas não o tamanho da região.
- **Extensão SFX em `0x2C13200`.** O relatório anterior descreve o parameters map como 815 entradas de 16 bytes. Não validei o formato de cada bloco de extensão além do fato de que os ponteiros resolvem e parseiam.
- **Lacuna de RAM vs. aliasing.** A RAM entre o blob vanilla de música e a imagem de código é o ctl SFX em runtime. O blob reconstruído a trata como zeros. Nenhuma referência de música cai nessa faixa, então isso não afeta o resultado, mas um offset futuro que caia ali leria dados de SFX no hardware, não zeros.
- **Blobs grandes.** Cada ctl tem ~8 MB, quase todos zeros de preenchimento, porque os offsets de ctl são absolutos desde a base. Um formato compacto (lista de faixas) seria um passo de integração, não necessário para a validação.
- **Port atual.** `BankParser::fits()` em `port/bridge/audio_bridge.cpp` usa o tamanho do blob vanilla (`0x6720` para música), por isso rejeita o bank novo (`0x3C9EA0`). O blob reconstruído resolveria isso, mas a integração (substituir os assets do `.o2r`) não foi feita e não deve ser commitada, porque é derivada da ROM.
- **Memória do projeto.** `memory_briefing` com `JRickey/BattleShip` não foi encontrado (o servidor exige workspace e project separados: `JRickey` e `BattleShip`, e `JRickey` também não existe). As regras globais de `_global` foram lidas. Não há `.ai-memory.toml` no repositório.
- **Arquivo de outro agente.** `tools/remix_dump_fighters.py` está não rastreado e não foi tocado.
