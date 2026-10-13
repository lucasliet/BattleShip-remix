# BattleShip — Nintendo Switch (homebrew) port notes

**Status:** em andimento (2026-10-12)
**Branches:** `ssb64-nx` no submódulo `libultraship/` (fork JRickey), `main` aqui.

## Arquitetura

```
host (macOS/CI desktop)                     Nintendo Switch (homebrew NRO)
─────────────────────────                   ──────────────────────────────
baserom.us.z64 (do usuário)                 BattleShip.nro  (ssb64 + LUS ssb64-nx)
  └─ ExtractAssets (Torch host)             f3d.o2r         (shaders)
       └─ BattleShip.o2r + .recipe          BattleShip.o2r  (assets da ROM do usuário)
            └─ copiados ao SD:              sdmc:/switch/BattleShip/
               scripts/switch/pack.sh
```

- A lógica do jogo é C decompilado compilado nativamente para aarch64 (o
  mesmo caminho do Android — `coroutine_aarch64.S` reusado).
- Render: Fast3D → OpenGL 2.1 core via `switch-mesa` (Nouveau/GM20B) +
  `switch-glad`, janela/áudio/input via `switch-sdl2`.
- Sem JIT: `DISABLE_SCRIPTING=ON` forçado (precedentes Android/UWP) — o
  sistema de mods (TinyCC/funchook) não existe no Switch.
- Toolchain 100% contida em `.toolchain/` (devkitPro `.pkg.tar.*`
  extraídos, pinados por SHA256 em `scripts/switch/packages.lock`);
  gcc x86_64 macOS sob Rosetta. Nada é instalado no sistema.

## Camadas de port

| Camada | O que foi feito |
|---|---|
| `libultraship` branch `ssb64-nx` | Suporte Switch portado de Net64DD/main-nx (mesma geração estrutural; delta nx = 27 arquivos): SwitchImpl/SwitchController, switch.cmake (deps via FetchContent), patch imgui-switch na cadeia, gfx_sdl2 (GL 2.1 core + glad + GetDisplaySize), Context/Gui/os.cpp/ResourceManager wiring |
| `port/` (app) | Coroutine `__SWITCH__` (aligned_alloc, sem guard page), first-run substituído por erro de o2r ausente, sinais crash-handler no default do libnx, exclusões CMake, NRO packaging (`nx_generate_nacp`/`nx_create_nro`) |
| Toolchain | `scripts/switch/{setup-toolchain,build,pack,relocate-toolchain}.sh` — bootstrap idempotente, verificação sha256 contra o lock, relocação de prefixos, bundle offline |

## Build

Ver `BUILDING.md` § "Nintendo Switch". Resumo:

```bash
scripts/switch/setup-toolchain.sh   # uma vez
scripts/switch/build.sh us          # build-switch/BattleShip.nro
scripts/switch/pack.sh              # dist/switch/SD layout
```

CI: `.github/workflows/build-switch.yml` (container devkitpro/devkita64
com tag datada — runner efêmero, não aplica a regra de isolamento local).

## Decisões e porquês

- **LUS: port do delta nx, não rebase.** Net64DD/main-nx e o fork ssb64
  compartem o merge-base upstream `f5c3843f`; o delta nx isolado é
  pequeno (+805/−19). Rebase do ssb64 (197 commits) sobre main-nx
  reescreveria 197 patches à toa.
- **Stack GL = desktop GL 2.1 via Mesa Nouveau, não GLES.** Os shaders do
  Fast3D no caminho GL core emitem `#version 110/130` — o mesmo perfil
  validado no SoH Switch. `USE_OPENGLES` continua exclusivo do Android.
- **first_run desligado no Switch:** sem `std::system()` nem dialogs em
  libnx. O o2r vem do SD; ausência = mensagem clara no log + exit.
- **Sem crash-handler próprio:** newlib não tem execinfo; deixar o
  reporter de crashes do HOS fazer o trabalho.

## Checklist de validação (hardware)

- [ ] Boot em full-memory mode (segurar R); applet mode deve falhar com
      mensagem clara (detectado pelo SwitchImpl).
- [ ] nxlink log aparece (`Switch::Init(PreInitPhase)`).
- [ ] Título → CSS → partida VS 4P a 60fps docked/handheld.
- [ ] Joy-Con/Pro Controller: input, gyro, rumble.
- [ ] Dock↔handheld mid-session sem crash (resize do viewport do LUS é
      caminho historicamente sensível — ver `port/port.cpp` widescreen latch).
- [ ] Áudio volta após sleep (workaround do applet hook).
- [ ] Widescreen 16:9 ativo por padrão (CVar `gEnhancements.Widescreen=1`).

## Questões abertas

1. `SDL_GetPrefPath` do switch-sdl2: confirmar para onde aponta
   (`sdmc:/switch/<app>`?) e se o o2r no app-dir é encontrado — senão,
   override explícito de paths no boot.
2. Patch `imgui-switch-config.patch` aplica sobre o `imgui-fixes-and-config.patch`
   atual do ssb64? (configure do LUS valida — `git-patch.cmake` falha alto.)
3.Perfil de clock default: `SwitchProfiles::MAXIMUM` deve ser aplicado no
   boot ( ApplyOverclock no Gui init) — validar consumo/temperatura.
4. Link do `glad`: se o link do NRO reclamar de símbolos glad, adicionar
   `glad`/`SDL2` explicitamente ao target do exe.
