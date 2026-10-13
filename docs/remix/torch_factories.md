# Smash Remix: fábricas do Torch (`SSB64:REMIXFIGHTERS`, `SSB64:REMIXAUDIO`)

Implementa o contrato de `docs/remix/implementation_plan.md` (F1): o Torch gera, a partir de `baserom.remix.z64`,
a tabela de lutadores (`remix/fighters`) e os cinco bancos de áudio reconstruídos (`audio/*`). Os dados derivados
da ROM ficam só no `.o2r` e em `build/`.

## O que foi feito

Código em `torch/src/factories/ssb64/` (registro em `torch/src/Companion.cpp`, dentro de `SSB64_SUPPORT`):

| Arquivo | Responsabilidade |
|---|---|
| `RemixRom.{h,cpp}` | Detecção da ROM Remix (country `E`, tamanho > 0x1000000), leitura big-endian com limites, tradução RAM→ROM |
| `RemixFighterTable.{h,cpp}` | Leitura da struct table FTData (ROM `0x92610`) e dos arrays de motion e menu |
| `RemixFightersFactory.{h,cpp}` | `SSB64:REMIXFIGHTERS`: serialização do formato `remix/fighters` |
| `RemixAudioCtl.{h,cpp}` | Imagem de runtime de um ctl (vanilla + imagem de código + zeros) e varredura de alcance para o tamanho do TBL |
| `RemixAudioFactory.{h,cpp}` | `SSB64:REMIXAUDIO` (parâmetro `bank`): despacho dos cinco bancos, slices de TBL e SBK |

As fábricas herdam de `BlobFactory` e reaproveitam `BlobHeaderExporter`, `BlobBinaryExporter` e `BlobCodeExporter`.
Assim cada saída é um recurso BLOB padrão, com o cabeçalho Torch de 0x40 bytes, um u32 de tamanho e o payload.

Yamls em `yamls/remix/`:

- `audio.yml`: as cinco entradas `BLOB` de `B1_sounds1_ctl/tbl`, `B1_sounds2_ctl/tbl` e `S1_music_sbk` viraram
  `SSB64:REMIXAUDIO` com `bank`. Os segmentos `fgm_*` e `particles` não foram alterados.
- `remix.yml` (novo): entrada `fighters` do tipo `SSB64:REMIXFIGHTERS`. O nome do recurso é
  `<caminho do yaml sem extensão>/<chave>`, então o o2r recebe `remix/fighters`.

Teste: `tools/test_remix_blobs.py`.

## Formatos

**`remix/fighters`** (little-endian): header de 16 bytes (`'RMXF'`, versão 1, count 97, reservado 0), 97 registros de
0x80 bytes e depois os arrays de motion (12 bytes cada: anim_file_id, command_offset, flags).
`mainmotion_off` e `menu_off` são offsets em bytes a partir do início da região de motion (logo após os registros).
Quando um array está vazio, o offset registrado é o fim atual da região de motion, e isso vale nos dois lados
(Torch e teste de referência).

**`audio/*`**: imagem de runtime do ctl (base RAM `0x800472D0` para música, `0x8004D9F0` para SFX), TBL como fatia
contígua da ROM até o último sample alcançável, e SBK como fatia da ROM a partir de `0x2504750`.

## Verificação

Comandos (caminhos absolutos no `cmake --build`):

```
cmake -S torch -B build-torch                                   # necessário: GLOB de fontes, sem rede
cmake --build /Users/lucas/Projetos/Pessoal/BattleShip/build-torch -j 4
./build-torch/torch o2r baserom.remix.z64 -s /Users/lucas/Projetos/Pessoal/BattleShip \
    -d /Users/lucas/Projetos/Pessoal/BattleShip/build/remix-test/run1
python3 -I tools/test_remix_blobs.py --o2r build/remix-test/run1/BattleShip.o2r
```

Resultado da execução:

| Recurso | Payload (bytes) | Comparação |
|---|---|---|
| `audio/B1_sounds1_ctl` | 8 097 072 | idêntico a `remix_dump_audio.py` |
| `audio/B1_sounds1_tbl` | 17 183 478 | idêntico |
| `audio/B1_sounds2_ctl` | 8 070 672 | idêntico |
| `audio/B1_sounds2_tbl` | 24 680 728 | idêntico |
| `audio/S1_music_sbk` | 14 712 768 | idêntico |
| `remix/fighters` | 286 488 | idêntico a uma referência em Python montada a partir do JSON de `remix_dump_fighters.py` |

- Os seis payloads são byte a byte iguais às referências Python. A referência de áudio é regenerada a cada execução
  do teste (`build/remix-dump/audio`). A referência de fighters segue o layout acima.
- Controle negativo: um byte trocado em `remix/fighters` no o2r faz o teste falhar na posição 0x100, com exit 1.
- Tamanhos do blob de áudio conferem com o relatório Python (payload + 68 bytes de cabeçalho Torch no zip).
- Log do Torch: `REMIXAUDIO` e `REMIXFIGHTERS` processam sem erro. Os 42 erros
  "Segment data missing from game config" vêm das entradas `fgm_*` e `particles`, que não foram alteradas.

## Tempo e tamanho

- Extração completa (`torch o2r`, todos os yamls de `yamls/remix`): **~7,3 s** de parede (Torch reporta 6,95 s).
- `BattleShip.o2r`: **59 378 845 bytes** (~56,6 MiB), com 5 483 entradas.
- Teste de referência: ~0,3 s.

## Decisões e pendências

- **Tamanho do TBL**: `FindRemixSampleExtent` percorre o ctl a partir do bank file, com as mesmas regras de limite do
  `BankParser` (`port/bridge/audio_bridge.cpp`). Só o alcance dos wavetables entra no cálculo, então
  envelopes, keymaps, books e loops não são copiados para essa conta. Como o resultado bate byte a byte com o Python,
  o alcance coincide com o do Python.
- **Guarda de ROM**: `RequireRemixRom` recusa qualquer ROM que não seja country `E` com tamanho acima de 0x1000000.
  Também verifica o ponteiro de banco do ctl de música e a contagem 0x177 do SBK. Na prática esses yamls só são
  processados com a ROM Remix (o `config.yml` seleciona `yamls/remix` pelo hash). O caminho de recusa não foi exercitado
  com ROM vanilla.
- **Port ainda não lê `remix/fighters`** (fase F2). Não há verificação em runtime. A validação é estática e por bytes.
- **Action arrays** não entram no `remix/fighters`. O plano não define esse formato, e a contagem ainda não foi
  validada (ver `docs/remix/fighter_table_extraction.md`, ressalva 1).
- **`audio_bridge.cpp`** ainda usa o tamanho vanilla de `BankParser::fits()`. Essa integração é da fase F2 e não foi
  alterada aqui.
