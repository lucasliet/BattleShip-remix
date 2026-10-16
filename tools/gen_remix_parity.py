#!/usr/bin/env python3
"""Writes docs/remix/parity.md: one row per Smash Remix .asm with its port status.

Status rules:
- items/<Name>.asm: `ported` when decomp/src/it/itremix/itremix<name>.c exists, `partial` when that file still has
  a REMIX-TODO.
- <Fighter>/<file>.asm: `ported` when the fighter has a port directory (decomp/src/ft/ftremix/<dir>) or is a data-only
  clone (no *Special.asm), `partial` when its port directory has a REMIX-TODO. <Fighter>/AI/*.asm belong to the AI area.
- Every other file takes its status from STATUS below; a file missing there is `todo`. Update STATUS when a task of
  docs/remix/ROADMAP.md lands, then rerun this script.

Progress per area = (ported + partial / 2) / (rows that are not n/a).
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / ".toolchain/pkgs/smashremix-src/src"
FT_REMIX = ROOT / "decomp/src/ft/ftremix"
IT_REMIX = ROOT / "decomp/src/it/itremix"
OUTPUT = ROOT / "docs/remix/parity.md"

SKIP_DIRS = {"Moveset", "costumes", "css", "Req", "stages", "Random", "gfx", "music", "sounds", "Sounds", "1p"}

# Fighter folder -> port directory under decomp/src/ft/ftremix (None = data-only clone of a vanilla or Remix fighter).
FIGHTER_DIRS = {
    "Banjo": "banjo", "Bowser": "bowser", "Conker": "conker", "Crash": "crash", "DSamus": "dsamus",
    "Dedede": "dedede", "DragonKing": "dragonking", "Ebi": "ebi", "Falco": "falco", "Goemon": "goemon",
    "JKirby": "jkirby", "JPika": "jpika", "Kirby": "kirby", "Lanky": "lanky", "Lucas": "lucas", "Marina": "marina",
    "Marth": "marth", "Mewtwo": "mewtwo", "Peach": "peach", "Peppy": "peppy", "Piano": "piano", "Roy": "roy",
    "Sheik": "sheik", "Slippy": "slippy", "Sonic": "sonic", "SSonic": "ssonic", "Wario": "wario", "Wolf": "wolf",
    "YoungLink": "younglink",
}

AREAS = ["fighters", "items", "stages", "audio", "menus_modes", "ai", "infra"]

# file -> (area, status, port location / note). Status: ported | partial | todo | n/a.
STATUS = {
    "Character.asm": ("fighters", "ported", "port/remix/remix_roster.cpp, ftremix*"),
    "Action.asm": ("fighters", "ported", "ftremixvanillaprocs.c, remix_action_layout.h"),
    "moveset.asm": ("fighters", "ported", "ftremixcommand.c"),
    "command.asm": ("fighters", "partial", "ftremixcommand.c"),
    "Projectile.asm": ("fighters", "ported", "decomp/src/wp/*remix*"),
    "fireball.asm": ("fighters", "partial", "wpmarioremixfireball.c, wpmariofireball.c"),
    "captainshared.asm": ("fighters", "partial", "ftremixdkcaptainshared.c, ftremixentry.c"),
    "dkshared.asm": ("fighters", "partial", "ftremixdkcaptainshared.c, ftremixentry.c"),
    "foxshared.asm": ("fighters", "partial", "ftremix/falco, wolf, peppy, slippy"),
    "jigglypuffkirbyshared.asm": ("fighters", "partial", "ftremixpuffkirbyshared.c"),
    "linkshared.asm": ("fighters", "partial", "ftremixlinkshared.c"),
    "marioshared.asm": ("fighters", "partial", "ftremixentry.c, wpmario"),
    "nessshared.asm": ("fighters", "partial", "ftremix/lucas"),
    "pikashared.asm": ("fighters", "partial", "ftremix/jpika"),
    "samusshared.asm": ("fighters", "partial", "ftremixsamusshared.c, ftremixsamusshared2.c"),
    "yoshishared.asm": ("fighters", "partial", "ftremixyoshishared.c"),
    "CustomGrabAction.asm": ("fighters", "partial", "ftremixpianoshared.c, fighter_registry custom_capture_*"),
    "DragonKingHUD.asm": ("fighters", "partial", "ftremixdragonkingdrainflash.c"),
    "KirbyHats.asm": ("fighters", "partial", "ftremixkirbycopy.c, ftremixkirbyhats.c, ftremixkirbypower.c"),
    "Size.asm": ("fighters", "partial", "ftremixsize*.c (phases a, b, c partial)"),
    "GFX.asm": ("fighters", "ported", "lbremixgfx.c, ftparam.c"),
    "GFXRoutine.asm": ("fighters", "ported", "gmremixcolscripts.c"),
    "CharEnvColor.asm": ("fighters", "partial", "ftremixenvcolor.c (display list fixes open)"),
    "Reflect.asm": ("fighters", "partial", "ftremixreflect.c (CPU hooks in M7)"),
    "PokemonAnnouncer.asm": ("fighters", "todo", "ROADMAP T2.5"),
    "Knockback.asm": ("fighters", "partial", "ftremixknockback.c"),
    "Hitbox.asm": ("fighters", "partial", "ftremixhitbox.c (display open)"),
    "Damage.asm": ("fighters", "partial", "ftremixdamage.c"),
    "Poison.asm": ("fighters", "ported", "ftremixpoison.c"),
    "OnHit.asm": ("fighters", "todo", "ROADMAP T2.7"),
    "SwordTrail.asm": ("fighters", "todo", "ROADMAP T8.1"),
    "Costumes.asm": ("menus_modes", "partial", "CSS port (costume tables)"),
    "Item.asm": ("items", "partial", "itremixspawn.c, itmanager.c, mnvsitemswitch.c"),
    "MagnifyingGlass.asm": ("items", "todo", ""),
    "Stages.asm": ("stages", "partial", "grremixstage.c, mnmaps.c"),
    "Hazards.asm": ("stages", "partial", "grremixhazards_*.c, grremixgating.c"),
    "FD.asm": ("stages", "ported", "scvsbattle.c (FD music), port stage page"),
    "Surface.asm": ("stages", "partial", "grremixsurface.c"),
    "Blastzone.asm": ("stages", "partial", "grremixblastzone.c"),
    "Spawn.asm": ("stages", "partial", "grremixspawn.c"),
    "Camera.asm": ("stages", "partial", "grremixcamera.c"),
    "Smashketball.asm": ("menus_modes", "todo", "ROADMAP T6.1"),
    "midi.asm": ("audio", "partial", "Torch RemixAudioFactory, audio_bridge.cpp"),
    "FGM.asm": ("audio", "partial", "Torch RemixAudioFactory"),
    "BGM.asm": ("audio", "partial", "grRemixGetStageBGM"),
    "SFXReplace.asm": ("audio", "todo", "ROADMAP T3.2"),
    "Stereo.asm": ("audio", "todo", "ROADMAP T3.2"),
    "AI.asm": ("ai", "todo", "ROADMAP T7.1"),
    "VanillaLv10Attacks.asm": ("ai", "todo", "ROADMAP T7.1"),
    "Global.asm": ("infra", "n/a", "addresses and constants"),
    "os.asm": ("infra", "n/a", "assembler helpers"),
    "String.asm": ("infra", "n/a", "assembler helpers"),
    "Render.asm": ("infra", "n/a", "replaced by the port's GObj/SObj calls"),
    "File.asm": ("infra", "ported", "Torch yamls/remix, RelocFactory"),
    "Color.asm": ("infra", "n/a", "constants"),
    "Joypad.asm": ("infra", "n/a", "replaced by the port's controller code"),
    "PAL.asm": ("infra", "n/a", "N64 TV type"),
    "Boot.asm": ("infra", "n/a", "N64 boot"),
    "SRAM.asm": ("menus_modes", "todo", "ROADMAP T4.1"),
    "Widescreen.asm": ("infra", "n/a", "out of scope (4:3)"),
    "FPS.asm": ("infra", "n/a", "N64 frame counter"),
    "CharacterSelect.asm": ("menus_modes", "partial", "CSS port (port/css_*, mnplayers*)"),
}


def area_of_top_level(name: str) -> str:
    return STATUS.get(name, ("menus_modes", "todo", ""))[0]


def has_todo(paths) -> bool:
    for path in paths:
        if path.is_file() and "REMIX-TODO" in path.read_text(errors="ignore"):
            return True
    return False


def fighter_row(folder: str, asm: Path) -> tuple:
    rel = asm.relative_to(SRC).as_posix()
    if "/AI/" in rel:
        return ("ai", "todo", "ROADMAP T7.1")
    port_dir = FIGHTER_DIRS.get(folder)
    if port_dir is None:
        return ("fighters", "ported", "data-only clone (roster)")
    directory = FT_REMIX / port_dir
    if not directory.is_dir():
        return ("fighters", "todo", f"no {directory.relative_to(ROOT)}")
    status = "partial" if has_todo(directory.glob("*.c")) else "ported"
    return ("fighters", status, directory.relative_to(ROOT).as_posix())


def item_row(asm: Path) -> tuple:
    c_file = IT_REMIX / f"itremix{asm.stem.lower()}.c"
    if not c_file.is_file():
        return ("items", "todo", "")
    status = "partial" if has_todo([c_file]) else "ported"
    return ("items", status, c_file.relative_to(ROOT).as_posix())


def collect_rows() -> list:
    rows = []
    for asm in sorted(SRC.rglob("*.asm")):
        parts = asm.relative_to(SRC).parts
        if any(part in SKIP_DIRS for part in parts[:-1]):
            continue
        if len(parts) == 1:
            area, status, where = STATUS.get(parts[0], (area_of_top_level(parts[0]), "todo", ""))
        elif parts[0] == "items":
            area, status, where = item_row(asm)
        else:
            area, status, where = fighter_row(parts[0], asm)
        rows.append((asm.relative_to(SRC).as_posix(), area, status, where))
    if not rows:
        sys.exit(f"no .asm files under {SRC}")
    return rows


def summary_lines(rows: list) -> list:
    lines = ["| Área | ported | partial | todo | n/a | progresso |", "|---|---|---|---|---|---|"]
    for area in AREAS:
        counts = {key: sum(1 for row in rows if row[1] == area and row[2] == key)
                  for key in ("ported", "partial", "todo", "n/a")}
        scored = counts["ported"] + counts["partial"] + counts["todo"]
        progress = (counts["ported"] + counts["partial"] / 2) / scored * 100 if scored else 100.0
        lines.append(f"| {area} | {counts['ported']} | {counts['partial']} | {counts['todo']} | {counts['n/a']} | "
                     f"{progress:.0f}% |")
    return lines


def main() -> None:
    rows = collect_rows()
    out = ["# Paridade do Smash Remix 2.0.1 (gerado por tools/gen_remix_parity.py; não editar à mão)", "",
           "Uma linha por `.asm` do Remix. Atualize `STATUS` no script quando uma task do `ROADMAP.md` fechar.", ""]
    out += summary_lines(rows)
    out += ["", "| Arquivo | Área | Status | Port |", "|---|---|---|---|"]
    out += [f"| `{path}` | {area} | {status} | {where} |" for path, area, status, where in rows]
    OUTPUT.write_text("\n".join(out) + "\n")
    print(f"{len(rows)} rows -> {OUTPUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
