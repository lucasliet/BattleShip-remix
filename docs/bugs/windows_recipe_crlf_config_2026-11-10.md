# Windows builds stamped a recipe the Switch rejects (CRLF `config.yml`)

Issue: lucasliet/BattleShip-remix#3.

## Symptom

On the Switch, ports 7 to 10 refused a `BattleShip-Remix.o2r` and `.recipe` extracted by the Windows build of the same
release: the error screen said the recipe did not match. Pairs extracted on macOS or Linux worked. A player found the
two embedded hashes: the Windows port.10 exe held `abf46c921e20dfa1a6047726280ee2306b5a0b22`, the Switch `.nro` held
`a1b04b121a55192184ea1073bf8fbd119f8413e8`.

## Root cause

The recipe hash (`CMakeLists.txt`, `SSB64_ASSET_RECIPE_HASH`) is the SHA-1 of the torch revision, the SHA-1 of
`config.yml` and the SHA-1 of every `yamls/<region>/*.yml`. `file(SHA1)` hashes the bytes on disk. The GitHub Windows
runner checks out with `core.autocrlf=true`, and `.gitattributes` pinned LF on the yamls but not on `config.yml`, so the
Windows build hashed a CRLF `config.yml`. Recomputing the hash with only `config.yml` converted to CRLF gives exactly
`abf46c92...`; with LF everywhere it gives `a1b04b12...`.

## Fix

- `.gitattributes`: `config.yml` and `tools/torch_extract_rev.txt` are `text eol=lf`.
- `CMakeLists.txt`: `ssb64_sha1_lf` hashes each recipe input with CRLF turned into LF, and the torch revision is
  stripped, so the hash no longer depends on how git checked the files out (local Windows builds with `autocrlf` too).

The hash of LF checkouts did not change (`a1b04b12...` on macOS, Linux and Switch), so Switch users keep their files.
Windows builds now get the same hash, so a Windows install extracts its `.o2r` again once after the update.

## Verification

- macOS configure: `SSB64 asset recipe hash: a1b04b121a55192184ea1073bf8fbd119f8413e8` (unchanged).
- A copy of `config.yml`, `tools/torch_extract_rev.txt` and `yamls/remix/*.yml` converted to CRLF: the old CMake code
  gives `83df5083...`, the new one gives `a1b04b12...`.
