# SSB64 PC Port — Agent Context

PC port of Super Smash Bros. 64 built from the complete decompilation at github.com/VetriTheRetri/ssb-decomp-re. Target integration: libultraship (LUS) + Torch asset pipeline.

## Documentation

Detailed reference material lives under `docs/`. Read the file that matches the task before touching code. When looking for a topic not listed here, run `ls docs/` and `ls docs/bugs/` to see what's available.

| Topic | File |
|-------|------|
| Project status, ROM info, dependencies, source tree layout | `docs/architecture.md` |
| C type system, decomp naming prefixes, code style, macros | `docs/c_conventions.md` |
| RDRAM / RSP / RDP / GBI / audio / threading / controller / endianness | `docs/n64_reference.md` |
| Build and package every platform (desktop, Switch, Android, PS5), ROMs and hashes, asset recipe, in-game updater | `docs/remix/build_and_roms.md` (generic prerequisites and Windows steps: `BUILDING.md`) |
| Reloc stub regen, runtime logs, LP64 compat notes, asset recipe fingerprint | `docs/build_and_tooling.md` |
| Switch, Android bring-up notes and the decisions behind them | `docs/switch_port_2026-10-12.md`, `docs/android_port_status_2026-05-01.md` |
| Native C mods (event catalog, detours, fighter overrides) | `docs/modding.md` |
| Netplay input staging and rollback boundary; per-frame state trace | `docs/netplay_architecture.md`, `docs/netcode_agent_rules.md`, `docs/state_trace.md` |
| GBI trace capture (port + M64P plugin) and `gbi_diff.py` usage | `docs/debug_gbi_trace.md` |
| IDO BE bitfield layout audit (compile + rabbitizer disasm to verify port struct bit positions) | `docs/debug_ido_bitfield_layout.md` |
| Resolved bugs (index + per-bug root cause / fix write-ups) | `docs/bugs/README.md` |
| **Smash Remix port: the only active plan (follow it task by task)** | `docs/remix/ROADMAP.md` |
| Smash Remix reference docs, open-item ledger (`patch_audit.md`) and index | `docs/remix/README.md` |

Ongoing investigations and handoff notes are loose `.md` files at the top level of `docs/` — check there before starting work on rendering, collision, or animation issues so you don't duplicate prior effort.

When you fix a new significant bug, add an entry under `docs/bugs/` using the slug pattern `<topic>_<YYYY-MM-DD>.md` and link it from `docs/bugs/README.md`.

---

## GitHub Issue Access

When asked to inspect GitHub issues, prefer the GitHub connector. If issue tools
are not visible yet, first run tool discovery for "GitHub issue fetch/view" so
the connector exposes `_fetch_issue` and `_fetch_issue_comments`, then fetch with
`repository_full_name: "lucasliet/BattleShip-remix"`. This fork's issues live there; `JRickey/BattleShip`
is the upstream BattleShip project and its issues are not this fork's.

If the human-formatted `gh issue view` output is blank or unreliable, use the JSON/template path instead:

```bash
gh issue view <number> -R lucasliet/BattleShip-remix --json number,title,state,author,body,url,comments,labels
```

New issues arrive through `.github/ISSUE_TEMPLATE/bug_report.yml` (platform, version, ROM, fighter/stage/mode, steps,
evidence, logs); blank issues are disabled.

---

## Self-contained toolchain — nothing on the system

Every dependency, toolchain and generated tool lives under `.toolchain/` (git-ignored, portable with the repo). Agents MUST NOT:

- run `brew install`, `port install`, `pip install` (system/user site), `npm -g`, `sudo`, or write to `/opt`, `/usr/local`, `~/.cache`, `~/.cmake`, shell profiles;
- point CMake at Homebrew/MacPorts prefixes. Configure with `-DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local"` and `-DCMAKE_PREFIX_PATH=$PWD/.toolchain/host/deps`;
- ask the user to install a package on their machine. If a dependency is missing, add it to a setup script instead.

How to get things:

- Switch (devkitA64, portlibs, cmake, ninja): `scripts/switch/setup-toolchain.sh`, then `source .toolchain/env.sh`.
- macOS desktop libs (libzip, nlohmann_json, tinyxml2, spdlog; SDL2/GLEW already cached): `scripts/macos/setup-host-deps.sh`. It pins every tarball by sha256 and installs into `.toolchain/host/deps`.
- Python: `uv` only, venv in `.toolchain/host/venv`.

A new dependency means: pinned URL + sha256 in the setup script, installed under `.toolchain/`, idempotent, no system side effects. Downloads need network, so run them outside the Bash sandbox only for that command.

## Parallel Sessions — Worktree Workflow

Multiple Claude windows working in the same checkout will clobber each other's source edits and build outputs. **Every parallel session works in its own git worktree.**

### Spinning up a new worktree

```bash
./scripts/new-worktree.sh <slug>           # configure only (fast)
./scripts/new-worktree.sh <slug> --build   # configure + full Debug compile
./scripts/new-worktree.sh <slug> --base some-branch --release
```

Output lands at `.claude/worktrees/<slug>` on branch `agent/<slug>`. The script:
1. Creates the worktree and branch.
2. Symlinks `baserom.us.z64` (gitignored, too large to duplicate).
3. **Independently clones `libultraship`, `torch`, and `decomp`** from the main tree's local submodule checkouts (picks up pinned SHAs that may not be pushed to the forks yet), then resets each submodule's `origin` to whatever URL the main tree's submodule uses — usually SSH so pushes work.
4. Regenerates gitignored codegen (`reloc_data.h`, `yamls/us/reloc_*.yml`, credits encodings).
5. Runs `cmake -B build` inside the worktree (and compiles if `--build` given).

### What this gives you

- **Full edit authority everywhere** — any file under `decomp/src/`, `decomp/include/`, `port/`, `libultraship/`, `torch/` is fair game. Submodule checkouts are real independent clones, not symlinks.
- **Zero collision** with other windows on source, build artifacts, or submodule state.
- **Normal git flow for submodule changes**:
  1. Edit and commit inside `<worktree>/decomp/`, `<worktree>/libultraship/`, or `<worktree>/torch/`.
  2. Push to the fork: `git -C <worktree>/<sm> push origin <branch>`.
     - `decomp` → `feature/remix-port` on `lucasliet/ssb-decomp-re-remix` (upstream `JRickey/ssb-decomp-re`)
     - `libultraship` → `feature/remix-port` on `lucasliet/libultraship-remix` (upstream `JRickey/libultraship`)
     - `torch` → `feature/remix-port` on `lucasliet/Torch-remix` (upstream `JRickey/Torch`)
  3. In the outer worktree, bump the submodule pointer: `git add <sm> && git commit -m "Bump <sm>: <summary>"`.
  4. When the outer branch lands on main, the pointer update goes with it.

### Merging back to main

The outer worktree is a normal branch (`agent/<slug>`). Merge or PR it into `main` like any other branch. Submodule pointer bumps ride along in the commits.

### Cleanup

```bash
git worktree remove .claude/worktrees/<slug>
git branch -D agent/<slug>
```

Stale worktrees under `.claude/worktrees/` from past sessions are fine to remove — check `git worktree list` and prune anything you don't recognize.

### Gotchas

- **Never use relative `build` paths in Bash tool calls** — Claude Code resets cwd between `Bash` calls. `cmake --build build` from the project root builds the main tree, not the worktree. Always use absolute paths: `cmake --build <worktree>/build ...`.
- **Cap build parallelism to what the machine's memory holds.** Bare `-j` lets the build spawn one compiler per logical core; libultraship's Debug C++ TUs hold 1–2 GB resident each, so on a 16 GB machine 8 in parallel push it into swap for the whole compile (`-j 4` there, `-j 6` when nothing else heavy is open). Size `-j` from the cores (`nproc` or `sysctl -n hw.ncpu`) and about 2 GB of free memory per job, and pass it explicitly: `cmake --build <worktree>/build --target ssb64 -j <n>`. Worktree first-builds are full from-scratch (no shared cache with main tree), so this matters most the first time.
- The binary loads `BattleShip-Remix.o2r` (`BattleShip.o2r` for the vanilla build; ROM-derived, user-extracted) and `f3d.o2r` (shaders) from its CWD at launch; without them it exits with `archive ... does not exist`. `new-worktree.sh` symlinks both from the main tree's `build/` into the worktree's `build/`. If the main tree has never been extracted, run `cmake --build <main-tree>/build --target ExtractAssets` there first so the symlinks resolve. (`ssb64.o2r` was an early-development port-asset archive; it is no longer produced or loaded — the build never contains ROM-derived data beyond the user's own first-run `.o2r`.)

---

## Releases

A release is a `v*` tag pushed on `feature/remix-port`; `.github/workflows/release-remix.yml` builds every platform
and publishes a draft whose body is `.github/release-notes.md`. Every new tag ships updated notes: before tagging,
rewrite the `## Highlights` section of `.github/release-notes.md` for what this release changes (and the Install /
Files sections when they change) and commit it, so the draft never repeats the previous release's text.

---

## Nothing private or temporary in the repository

The repository and its submodules are public. Never commit, and check before every push (`git diff origin/<branch>..HEAD`):

- private data of the owner or a tester: LAN or public IP addresses (consoles, machines), host names, user names,
  e-mails other than the git identity, absolute home paths (`/home/<user>`, `/Users/<user>`), tokens or keys;
- ROM-derived data: ROMs, `.o2r`, extracted assets, dumps, saves (`ssb64_save.bin`), `BattleShip.cfg.json` taken from a
  device;
- temporary or session material: scratchpad and `/tmp` paths, session ids, test logs and screenshots, one-off scripts or
  inputs made for a single investigation, debug logging left in code.

Docs name a device path by its role (`/data/homebrew/PPSA99264/`, "the console's FTP") and keep addresses in the
owner's private notes. When something private already sits in unpushed commits, rewrite those commits before pushing;
once pushed, tell the owner instead of rewriting public history.

Smoke-test inputs and scripts (`.claude/skills/remix-smoke-tests/`) follow the same rule without being asked:

- a new file goes into `inputs/` or `scripts/` only when it drives a flow that other fighters, stages or tasks will
  reuse (a move, a menu path, a mode), with a header naming the scene, env and what to check;
- an input made to reproduce one bug, one audit item or one question stays in the session scratchpad and is never
  committed;
- after a task is validated and its evidence recorded, delete its runs under `build/agent-shots/` and
  `build/debug_traces/`; before ending a session, leave both empty unless an open task still needs a run.

Temporary files outside the repository's history get the same cleanup, wherever the session created them, without
being asked: redirected build logs (`configure*.log`,
`compile*.log`), `git bundle`s and other transfer copies, test uploads to a device (a renamed `eboot.bin`), backups of a
device's save or cfg once the test passed, `__pycache__`, build trees with no CMake cache or no script that uses them,
and personal test packages that embed the `.o2r`. Keep the incremental build trees (`build*/` with a `CMakeCache.txt`,
`android/app/.cxx`) and `.toolchain/`. When unsure whether something is still useful, list it with its size and ask.

---

## Asset extraction fingerprint

The `.o2r` is re-extracted when its `.recipe` hash differs from the binary's; on the Switch that means copying a new `.o2r` from a desktop by hand. The hash covers `tools/torch_extract_rev.txt`, `config.yml` and `yamls/<region>/`, not the checked-out torch commit. When a change in `torch/` (or in `config.yml` / the yamls) alters what gets extracted, set `tools/torch_extract_rev.txt` to the new torch commit (`git -C torch rev-parse HEAD`) in the same change; for a torch change that leaves the output unchanged (PS5, logging, listing) leave the file alone. See `docs/build_and_tooling.md`.

---

## Logs

Every log file the port or libultraship creates holds only the last run: open it with truncate (or `rotate_on_open` with no backups), never append across launches, and never keep rotated copies. A new log writer follows the same rule.

---

## Agent Directives

### Pre-Work

1. **THE "STEP 0" RULE**: Before any structural refactor on a file >300 LOC, first remove dead code, unused exports, unused imports, and debug logs. Commit cleanup separately.

2. **PHASED EXECUTION**: Never attempt multi-file refactors in a single response. Break work into phases. Complete Phase 1, run verification, wait for approval before Phase 2. Max 5 files per phase.

### Code Quality

3. **THE SENIOR DEV OVERRIDE**: If architecture is flawed, state is duplicated, or patterns are inconsistent — propose and implement structural fixes. Ask: "What would a senior, experienced, perfectionist dev reject in code review?" Fix all of it.

4. **FORCED VERIFICATION**: Do not report a task complete until you have run the build and fixed all errors. If no build is configured yet, state that explicitly.

5. **DECOMP PRESERVATION — preserve behavior, not byte-matching**: The decomp describes the *game*, not the build. Keep IDO idioms (goto, odd casts, temp variables) that encode original N64 semantics — those are load-bearing and must not be "modernized." But don't preserve **compiler compat shims** (warning suppressions, permissive flags, header shortcuts) that hurt port stability just to avoid touching decomp source. If a suppressed diagnostic is masking real bugs on modern LP64 toolchains (e.g., `-Wno-implicit-function-declaration` silently truncating 64-bit pointer returns to `int`), fix the root cause — add the missing include, wrap a port fix in `#ifdef PORT`, or adjust the decomp file itself — rather than keeping the suppression. **Accuracy to game behavior > accuracy to ROM bytes.** When choosing between stability and ROM-matching, choose stability and document the deviation in `docs/bugs/`.

### Context Management

6. **SUB-AGENT SWARMING**: For tasks touching >5 independent files, launch parallel sub-agents. Each agent gets its own context window.

7. **CONTEXT DECAY AWARENESS**: After 10+ messages, re-read any file before editing. Do not trust memory of file contents.

8. **FILE READ BUDGET**: For files over 500 LOC, use offset and limit parameters to read in chunks.

9. **EDIT INTEGRITY**: Before every edit, re-read the file. After editing, verify the change applied correctly. Never batch >3 edits to the same file without a verification read.

10. **NO SEMANTIC SEARCH**: When renaming or changing any function/type/variable, search separately for: direct calls, type references, string literals, dynamic references, re-exports, and tests.
