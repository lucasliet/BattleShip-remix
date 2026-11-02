# Builds e ROMs: BattleShip original (US) e BattleShip-Remix

Nada de ROM nem de `.o2r` entra no git (`*.z64`, `build/`, `build-*/` e `dist/` estão no `.gitignore`). Quem clona o
repositório precisa fornecer as ROMs abaixo e gerar o `.o2r` no desktop.

## ROMs necessárias

Ficam na **raiz do repositório**. O CMake aceita `.z64`, `.n64` e `.v64` (o Torch normaliza a ordem dos bytes).

| Arquivo | Para quê | Tamanho | SHA-1 | MD5 |
|---|---|---|---|---|
| `baserom.remix.z64` | Build Remix: fonte do `BattleShip-Remix.o2r` (`torch o2r`) e das tabelas de lutadores, áudio e arquivos | 65 536 000 | `13716381ed5e29be2606e0f4724b18fd00789d04` | `2b2d6b295106c54216b7fc7a2f14346e` |
| `baserom.us.z64` | Build vanilla, e também no build Remix: três imagens de stage do CSS (`derive_stage_assets.py` só entende o layout US) e `scripts/remix/assemble-remix.sh` | 16 777 216 | `e2929e10fccc0aa84e5776227e798abc07cedabf` | `f7c52568a31aadf26e14dc2b6416b2ed` |

SHA-256 das duas, medidos nas cópias desta máquina:

- `baserom.remix.z64`: `7efec9e0983656bb0219a23c511cd1505a5f84d524e50ad4284dc1c7eb4d1403`
- `baserom.us.z64`: `15592e79d3c5295cef4371d4992f0bd25bec2102fc29644c93e682f7ea99ef3d`

CRCs do cabeçalho N64: Remix `34EE4749 1833C64C`; US `916B8B5B 780B85A4`. Os dois têm o código de jogo `NALE`; o nome interno
da ROM do Remix é `SMASH REMIX`.

A ROM JP (`baserom.jp.z64`) não é usada no Remix.

Sem `baserom.us.z64` o build Remix ainda compila (o alvo `DeriveStageAssets` vira um stub), mas as imagens de stage do CSS
ficam faltando.

## Qual versão do Remix

**Smash Remix 2.0.1**, a ROM de release (a que o projeto distribui na aba Releases do repositório
`JSsixtyfour/smashremix`, aplicada sobre uma ROM US).

Evidências de que a nossa `baserom.remix.z64` é a 2.0.1:

- A string `Smash Remix v2.0.1` está dentro da ROM.
- O fonte do Remix que usamos (`.toolchain/pkgs/smashremix-src`) está pinado no commit
  `5e04fe7fcd023cd43c71f25f89bb6e810d254d55` (tag `2.0.1`, 2026-03-08). Montado por `scripts/remix/assemble-remix.sh`
  sobre `baserom.us.z64`, a imagem de código (ROM `0x2C00000`, 4 MB) bate com a da nossa ROM exceto em 1 038 de 1 048 576
  palavras, que são os dados lidos dos arquivos injetados.

O que não está verificado: os hashes acima são os da cópia usada no desenvolvimento. Eu não confirmei que toda ROM de
release 2.0.1 tenha exatamente o mesmo SHA-1; se outra cópia divergir, compare o código de jogo, a string de versão e a
imagem de código antes de concluir que ela está errada.

O fonte do Remix traz o `original.xdelta` (SHA-256 `e892db9f57135ae2ba380e6db98516dd849dcabb4be5c553ea48158ec7dece1d`),
que gera a ROM-base para a montagem do ASM a partir de uma US. A ROM vanilla não serve como `baserom.remix.z64`, porque
muitas edições do Remix estão dentro de arquivos comprimidos da ROM. Não rodamos o xdelta nesta máquina.

## Dois produtos, dois diretórios de build

| Produto | `SSB64_VERSION` | ROM | Binário | Arquivo de assets | Diretório de build sugerido |
|---|---|---|---|---|---|
| BattleShip (SSB64 original, US) | `us` | `baserom.us.z64` | `BattleShip` | `BattleShip.o2r` | `build-us` |
| BattleShip-Remix | `remix` (padrão) | `baserom.remix.z64` (+ `baserom.us.z64`) | `BattleShip-Remix` | `BattleShip-Remix.o2r` | `build` |

O código do jogo é compilado por região (`REGION_US` ou `REGION_REMIX`), então cada versão é um build próprio, com seu
diretório. O nome do binário e do `.o2r` vem de `SSB64_APP_NAME` (`CMakeLists.txt:661-670`). Os dois podem existir lado a
lado.

## Build desktop do BattleShip original (macOS, toolchain contida)

Mesmos comandos do Remix, trocando a versão e o diretório. Precisa só de `baserom.us.z64`.

```bash
PATH=$PWD/.toolchain/host/ninja:$PATH
.toolchain/host/cmake/CMake.app/Contents/bin/cmake -S $PWD -B $PWD/build-us -GNinja -DCMAKE_BUILD_TYPE=Release \
  -DSSB64_VERSION=us -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local" \
  -DCMAKE_PREFIX_PATH=$PWD/.toolchain/host/deps
.toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build-us --target ssb64 -j 4
(cd build-us && ./BattleShip)
```

O binário `BattleShip` carrega `BattleShip.o2r` e `f3d.o2r` do diretório onde roda (o `ExtractAssets` os gera a partir de
`baserom.us.z64`). Em Linux e Windows os comandos genéricos estão em `BUILDING.md`.

## Build desktop do BattleShip-Remix (macOS, toolchain contida)

Tudo vem de `.toolchain/` (veja `AGENTS.md`). `SSB64_VERSION` tem como padrão `remix`. O diretório `build/` desta
máquina foi configurado assim (Ninja, `Release`):

```bash
PATH=$PWD/.toolchain/host/ninja:$PATH
.toolchain/host/cmake/CMake.app/Contents/bin/cmake -S $PWD -B $PWD/build -GNinja -DCMAKE_BUILD_TYPE=Release \
  -DSSB64_VERSION=remix -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local" \
  -DCMAKE_PREFIX_PATH=$PWD/.toolchain/host/deps
.toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build --target ssb64 -j 4
```

Resultado em `build/`: `BattleShip-Remix` (o alvo é `ssb64`), `BattleShip-Remix.o2r`, `BattleShip-Remix.o2r.recipe`,
`f3d.o2r`. Dependências do desktop: `scripts/macos/setup-host-deps.sh`. Um arquivo novo em `decomp/src` pede o
`cmake -S ... -B ...` antes do build.

O `ssb64` depende do `ExtractAssets`: com `baserom.remix.z64` presente, o build roda
`torch o2r baserom.remix.z64 -s <repo> -d build/extracted` e copia o resultado. Depois ele só refaz o `.o2r` quando o Torch,
o `config.yml`, os `yamls/remix/*.yml` ou a ROM mudam. O `.recipe` (41 bytes) é o SHA-1 desses insumos
(`<sha do torch>:<sha do config.yml>:<sha de cada yaml>`); se ele mudar, o port reextrai o `.o2r` sozinho.

Para refazer só os assets: `cmake --build build --target ExtractAssets`.

## Build Switch (BattleShip e BattleShip-Remix)

O console nunca lê ROM: o `.nro` é compilado com o devkitA64 e o `.o2r` vem do build desktop da mesma versão.

```bash
scripts/switch/setup-toolchain.sh        # uma vez; rede só nessa etapa

# Remix (padrão)
scripts/switch/build.sh remix            # build-switch/BattleShip-Remix.nro
VERSION=remix scripts/switch/pack.sh     # dist/switch/SD/switch/BattleShip-Remix/

# BattleShip original (US)
scripts/switch/build.sh us               # build-switch/BattleShip.nro
VERSION=us scripts/switch/pack.sh        # dist/switch/SD/switch/BattleShip/
```

O `pack.sh` procura `<app>.o2r`, `<app>.o2r.recipe` e `f3d.o2r` em `build/` e depois na raiz do repositório, nunca em
`build-us/`. Para o original, copie esses três arquivos de `build-us/` para a raiz (ou para `build/`) antes do pack.

O `pack.sh` escolhe a versão só pela variável `VERSION` (padrão `remix`), não pelo argumento; sem `VERSION=us` ele empacota o
Remix. O `build-switch/` é compartilhado: rodar `build.sh us` e depois `build.sh remix` reconfigura o mesmo diretório, então
empacote cada versão logo depois do seu build.

`pack.sh` só copia: o `.nro` de `build-switch/`, e `BattleShip-Remix.o2r`, `BattleShip-Remix.o2r.recipe` e `f3d.o2r` de
`build/`. Se algum não existir, ele só imprime `AVISO` e segue, e o pacote sai incompleto. Por isso:

1. Rode o build desktop antes (gera os `.o2r` a partir de `baserom.remix.z64`).
2. Rode o build Switch e o `pack.sh`.
3. Copie a pasta `dist/switch/SD/switch/BattleShip-Remix/` inteira para o SD, em `/switch/`.
4. Abra pelo Homebrew Menu em modo de memória completa (segurando R ao abrir um jogo instalado).

O ícone do Home menu é um JPEG 256x256 embutido no `.nro` pelo CMake (`assets/switch/icon.jpg` no original,
`assets/switch/icon-remix.jpg` no Remix, com "SR"). Os dois saem de `assets/icon.png` por
`.toolchain/host/venv/bin/python -I tools/gen_switch_icon.py` (só Pillow); o script também grava `assets/icon-remix.png`.

Se o `.o2r` ou o `Torch` mudarem (novo `.recipe`), refaça o pacote: um `.o2r` antigo no SD com um `.nro` novo causa tempestade
de tokens de reloc inválidos. O jogo grava `BattleShip-Remix.log` ao lado do `.nro`, no SD.

## Checklist para um clone novo

1. `git submodule update --init --recursive`.
2. Colocar `baserom.remix.z64` e `baserom.us.z64` na raiz e conferir o SHA-1 com a tabela acima (`shasum -a 1 baserom.*.z64`).
3. `scripts/macos/setup-host-deps.sh` (desktop) e `scripts/switch/setup-toolchain.sh` (Switch).
4. Build desktop (acima), depois o build e o pack do Switch.
