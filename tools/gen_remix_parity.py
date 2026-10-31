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
    "Toggles.asm": ("menus_modes", "partial", "remix_toggles.cpp, PortMenu.cpp (Remix section)"),
    "Hitstun.asm": ("fighters", "ported", "ftparam.c"),
    "DI.asm": ("fighters", "ported", "ftremixdi.c"),
    "AirDodge.asm": ("fighters", "ported", "ftremixairdodge.c"),
    "Tripping.asm": ("fighters", "ported", "ftremixtripping.c"),
    "ZCancel.asm": ("fighters", "partial", "ftremixzcancel.c"),
    "StaleMoves.asm": ("fighters", "ported", "ftremixstalemoves.c"),
    "JabLock.asm": ("fighters", "ported", "ftremixjablock.c"),
    "FootStool.asm": ("fighters", "ported", "ftremixfootstool.c"),
    "LedgeJump.asm": ("fighters", "ported", "ftremixledgejump.c"),
    "LedgeTrump.asm": ("fighters", "partial", "ftremixledgetrump.c"),
    "WallTeching.asm": ("fighters", "partial", "ftremixwalltech.c"),
    "SpotDodge.asm": ("fighters", "ported", "ftremixspotdodge.c"),
    "PerfectShield.asm": ("fighters", "ported", "ftremixperfectshield.c"),
    "Parry.asm": ("fighters", "ported", "ftremixparry.c"),
    "Rage.asm": ("fighters", "ported", "ftremixrage.c"),
    "ChargeSmashAttacks.asm": ("fighters", "ported", "ftremixchargesmash.c"),
    "Speed.asm": ("fighters", "partial", "ftremixspeed.c (training wiring open)"),
    "SingleButtonMode.asm": ("fighters", "partial", "ftremixsinglebutton.c (entry wiring open)"),
    "Japan.asm": ("fighters", "partial", "ftremixhitlag.c, shieldstun, jstunsleep, japanesesounds"),
    "SpecialZoom.asm": ("fighters", "partial", "gmremixspecialzoom*.c (stage hide open)"),
    "Combo.asm": ("fighters", "ported", "ftremixcombo.c"),
    "Accessibility.asm": ("fighters", "partial", "ifscreenflash.c, efremixscreenshake.c"),
    "Pause.asm": ("menus_modes", "partial", "ftremixpause.c (Disable HUD, music, legend open)"),
    "Knockback.asm": ("fighters", "partial", "ftremixknockback.c"),
    "Hitbox.asm": ("fighters", "partial", "ftremixhitbox.c (display open)"),
    "Damage.asm": ("fighters", "partial", "ftremixdamage.c"),
    "Poison.asm": ("fighters", "ported", "ftremixpoison.c"),
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
    "BGM.asm": ("audio", "partial", "grRemixGetStageBGM"),
    "SFXReplace.asm": ("audio", "todo", "ROADMAP T3.2"),
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
    "AI.asm": ("ai", "partial", "ftremixai*.c, ftcomputer.c hooks (ROADMAP T7.1, leftovers in T8.2)"),
    "VanillaLv10Attacks.asm": ("ai", "partial", "ftremixailv10.c (PREVENT routines of vanilla fighters in T8.2)"),
    "AA.asm": ("menus_modes", "ported", "remix_display_settings.cpp (MSAA)"),
    "ComboMeter.asm": ("menus_modes", "ported", "ifremixcombometer.c, ifremixcombohits.c"),
    "InputDelay.asm": ("menus_modes", "ported", "ftremixinputdelay.c"),
    "TimedStock.asm": ("menus_modes", "ported", "gmremixtimedstock*.c"),
    "Stereo.asm": ("audio", "ported", "lbcommon.c fix_sfx_pan_"),
    "FGM.asm": ("audio", "ported", "Torch RemixAudioFactory"),
    "midi.asm": ("audio", "ported", "Torch RemixAudioFactory, audio_bridge.cpp"),
    "Stamina.asm": ("menus_modes", "partial", "ftremixstamina*.c, gmremixstamina*.c (T6.1a, leftovers in T8.2)"),
    "TugOfWar.asm": ("menus_modes", "partial", "gmremixtugofwar*.c (T6.1b, leftovers in T8.2)"),
    "KingOfTheHill.asm": ("menus_modes", "partial", "gmremixkoth*.c, mnremixgoalpicker.c (T6.1c)"),
    "TagTeam.asm": ("menus_modes", "partial", "gmremixtagteam*.c (T6.1d)"),
    "Smashketball.asm": ("menus_modes", "partial", "gmremixsmashketball*.c, itremixbasketball.c (T6.1e)"),
    "TwelveCharBattle.asm": ("menus_modes", "partial", "gmremixtwelvecb*.c (T6.1f, CSS of 24 portraits in T8.2)"),
    "StockMode.asm": ("menus_modes", "partial", "gmremixstockmode.c (Last mode open)"),
    "Teams.asm": ("menus_modes", "partial", "scmanager.c, ftremixstamina*.c"),
    "Training.asm": ("menus_modes", "partial", "sc1ptrainingmode.c, remix_training.* (Z menu as port menu; per-port chain, Music and Stage BG open)"),
    "Practice_1P.asm": ("menus_modes", "partial", "gmremixpractice.c"),
    "SinglePlayer.asm": ("menus_modes", "partial", "sc1premix*.c (T6.2, leftovers in T8.2)"),
    "SinglePlayerModes.asm": ("menus_modes", "partial", "sc1premixintro/allstar/multiman*.c"),
    "Bonus.asm": ("menus_modes", "partial", "sc1premixbonusexit.c, sc1premixmultiman.c (Bonus 2 records in T8.2)"),
    "HRC.asm": ("menus_modes", "partial", "sc1premixhrc*.c"),
    "CharacterDataScreen.asm": ("menus_modes", "partial", "mnremixchardata.c, mncharacters.c"),
    "Credits.asm": ("menus_modes", "partial", "scstaffroll.c (glyphs and speed assist open)"),
    "Gallery.asm": ("menus_modes", "partial", "mnremixgallery.c"),
    "GameEnd.asm": ("menus_modes", "partial", "gmremixgameend.c (mode rows open)"),
    "Handicap.asm": ("menus_modes", "partial", "ftremixhandicap.c"),
    "InputDisplay.asm": ("menus_modes", "partial", "ftremixinputdisplay.c"),
    "PlayerTag.asm": ("menus_modes", "partial", "ifremixplayertag.c"),
    "resultsscreen.asm": ("menus_modes", "partial", "mnremixwinbgm.c, mnvsresults.c"),
    "Shield.asm": ("menus_modes", "partial", "ftremixshield.c (Costume and egg open)"),
    "Transitions.asm": ("menus_modes", "partial", "lbtransition.c (logo open)"),
    "VsStats.asm": ("menus_modes", "partial", "mnremixvsstats.c (page and trackers open)"),
    "PokemonAnnouncer.asm": ("fighters", "ported", "ifremixannouncer.c, ifremixhitannouncer.c (throw announcement open in ftcommonthrow.c)"),
    "OnHit.asm": ("fighters", "ported", "ftremixchartables.c ftRemixRunOnHit"),
    "SinglePlayerMenus.asm": ("menus_modes", "ported", "mnremix1pmode.c"),
    "VsRemixMenu.asm": ("menus_modes", "ported", "mnremixvsmode.c"),
    "Menu.asm": ("infra", "n/a", "replaced by the port menu (ROADMAP T4.1 deviation)"),
    "Walljump.asm": ("infra", "n/a", "all patches are commented out in the Remix 2.0.1 build"),
    "Practice.asm": ("menus_modes", "partial", "ftremixzcancel.c (Flash On Z-Cancel); Color Overlays open"),
    "CharacterSelectDebugMenu.asm": ("menus_modes", "partial", "remix_player_panel.cpp (the CSS panel menu is the debug menu open)"),
    "SwordTrail.asm": ("fighters", "partial", "afterimage flag of Link only"),
    "Skeleton.asm": ("fighters", "partial", "ftdisplaymain.c (vanilla skeleton path, per-port force open)"),
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
        return ("ai", "ported", "ftremixaitables.c reads the attack table from the Remix image (T7.1)")
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
