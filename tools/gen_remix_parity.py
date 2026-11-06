#!/usr/bin/env python3
"""Writes docs/remix/parity.md: one row per Smash Remix .asm with its port status.

Status rules:
- items/<Name>.asm: `ported` when decomp/src/it/itremix/itremix<name>.c exists, `partial` when that file still has
  a REMIX-TODO.
- <Fighter>/<file>.asm: `ported` when the fighter has a port directory (decomp/src/ft/ftremix/<dir>) or is a data-only
  clone (no *Special.asm), `partial` when its port directory has a REMIX-TODO. <Fighter>/AI/*.asm belong to the AI area.
- Every other file takes its area, port location and fallback status from STATUS below; a file missing there is `todo`.
- The ledger (docs/remix/patch_audit.md) then decides: a file with an `open` item is `partial`; a file whose items are
  all `done` or `n/a` is `ported` (`adapted` when STATUS says so), unless its code still has a REMIX-TODO. A file with no
  ledger item keeps STATUS, except that `partial`/`todo` becomes `ported` when the scope map
  (docs/remix/patch_audit_scopes.md) audited it: the audit gave every GAP or STUB scope a ledger item.

Progress per area = (ported + adapted + partial / 2) / (rows that are not n/a).
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / ".toolchain/pkgs/smashremix-src/src"
FT_REMIX = ROOT / "decomp/src/ft/ftremix"
IT_REMIX = ROOT / "decomp/src/it/itremix"
OUTPUT = ROOT / "docs/remix/parity.md"
FEATURES = ROOT / "tools/remix_asm_features.tsv"
LEDGER = ROOT / "docs/remix/patch_audit.md"
SCOPES = ROOT / "docs/remix/patch_audit_scopes.md"

ASM_REF = re.compile(r"([A-Za-z0-9_/]+\.asm)\b(?::(\d+))?")
ASM_INCLUDE = re.compile(r'^\s*include\s+"([^"]+\.asm)"', re.MULTILINE)
TABLE_PATCH = re.compile(r"^\s*Character\.table_patch_start\(\s*(\w+)", re.MULTILINE)
TABLE_NAME = re.compile(r"`(\w+)`")
FILE_HEADING = re.compile(r"^#####\s+([A-Za-z0-9_/]+\.asm)")
LEDGER_ROW = re.compile(r"^\| ([A-Z]{2}-\d+) \| ([^|]+?) \| ([^|]*) \| ([^|]*) \|")
LEDGER_STATES = ("open", "done", "n/a")

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

# file -> (area, status, port location / note). Status: ported | adapted | partial | todo | n/a.
# adapted = the feature exists for the player through a documented port deviation; it counts as done.
STATUS = {
    "Character.asm": ("fighters", "partial", "port/remix/remix_roster.cpp, ftremix*"),
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
    "DragonKingHUD.asm": ("stages", "todo", "ifremixdragonkinghud.c"),
    "KirbyHats.asm": ("fighters", "partial", "ftremixkirbycopy.c, ftremixkirbyhats.c, ftremixkirbypower.c"),
    "Size.asm": ("fighters", "partial", "ftremixsize*.c"),
    "GFX.asm": ("fighters", "ported", "lbremixgfx.c, ftparam.c"),
    "GFXRoutine.asm": ("fighters", "ported", "gmremixcolscripts.c"),
    "CharEnvColor.asm": ("fighters", "partial", "ftremixenvcolor.c"),
    "Reflect.asm": ("fighters", "partial", "ftremixreflect.c"),
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
    "Speed.asm": ("fighters", "partial", "ftremixspeed.c"),
    "SingleButtonMode.asm": ("fighters", "partial", "ftremixsinglebutton.c"),
    "Japan.asm": ("fighters", "partial", "ftremixhitlag.c, shieldstun, jstunsleep, japanesesounds"),
    "SpecialZoom.asm": ("fighters", "partial", "gmremixspecialzoom*.c"),
    "Combo.asm": ("fighters", "ported", "ftremixcombo.c"),
    "Accessibility.asm": ("fighters", "ported", "ifscreenflash.c, efremixscreenshake.c, gmremixaccessibility.c"),
    "Pause.asm": ("menus_modes", "partial", "ftremixpause.c"),
    "Knockback.asm": ("fighters", "partial", "ftremixknockback.c"),
    "Hitbox.asm": ("fighters", "partial", "ftremixhitbox.c"),
    "Damage.asm": ("fighters", "partial", "ftremixdamage.c"),
    "Poison.asm": ("fighters", "ported", "ftremixpoison.c"),
    "Costumes.asm": ("menus_modes", "partial", "CSS port (costume tables)"),
    "Item.asm": ("items", "partial", "itremixspawn.c, itmanager.c, mnvsitemswitch.c"),
    "MagnifyingGlass.asm": ("items", "partial", "gmremixmagnify.c"),
    "Stages.asm": ("stages", "partial", "grremixstage.c, mnmaps.c"),
    "Hazards.asm": ("stages", "partial", "grremixhazards_*.c, grremixgating.c"),
    "FD.asm": ("audio", "partial", "scvsbattle.c (FD music)"),
    "Surface.asm": ("stages", "partial", "grremixsurface.c"),
    "Blastzone.asm": ("stages", "partial", "grremixblastzone.c"),
    "Spawn.asm": ("stages", "partial", "grremixspawn.c"),
    "Camera.asm": ("stages", "partial", "grremixcamera.c"),
    "BGM.asm": ("audio", "partial", "grRemixGetStageBGM"),
    "SFXReplace.asm": ("audio", "ported", "ftremixsfxreplace.c"),
    "Global.asm": ("infra", "n/a", "addresses and constants"),
    "os.asm": ("infra", "n/a", "assembler helpers"),
    "String.asm": ("infra", "n/a", "assembler helpers"),
    "Render.asm": ("infra", "n/a", "replaced by the port's GObj/SObj calls"),
    "File.asm": ("infra", "ported", "Torch yamls/remix, RelocFactory"),
    "Color.asm": ("infra", "n/a", "constants"),
    "Joypad.asm": ("infra", "ported", "ftremixtauntbutton.c (Taunt Button mask and jump guard)"),
    "PAL.asm": ("infra", "n/a", "N64 TV type"),
    "Boot.asm": ("menus_modes", "partial", "title logo in mntitle.c, mnremixsplash.c"),
    "SRAM.asm": ("menus_modes", "partial", "lbBackup records (port_save.cpp); toggles and panels as CVars"),
    "Widescreen.asm": ("infra", "n/a", "out of scope (4:3)"),
    "FPS.asm": ("infra", "adapted", "FPS Display toggle, port/remix/remix_display_settings.cpp"),
    "CharacterSelect.asm": ("menus_modes", "partial", "CSS port (port/css_*, mnplayers*)"),
    "AI.asm": ("ai", "partial", "ftremixai*.c, ftcomputer.c hooks"),
    "VanillaLv10Attacks.asm": ("ai", "partial", "ftremixailv10.c"),
    "AA.asm": ("menus_modes", "ported", "remix_display_settings.cpp (MSAA)"),
    "ComboMeter.asm": ("menus_modes", "ported", "ifremixcombometer.c, ifremixcombohits.c"),
    "InputDelay.asm": ("menus_modes", "ported", "ftremixinputdelay.c"),
    "TimedStock.asm": ("menus_modes", "partial", "gmremixtimedstock*.c"),
    "Stereo.asm": ("audio", "ported", "lbcommon.c fix_sfx_pan_"),
    "FGM.asm": ("audio", "ported", "Torch RemixAudioFactory"),
    "midi.asm": ("audio", "ported", "Torch RemixAudioFactory, audio_bridge.cpp"),
    "Stamina.asm": ("menus_modes", "partial", "ftremixstamina*.c, gmremixstamina*.c"),
    "TugOfWar.asm": ("menus_modes", "partial", "gmremixtugofwar*.c"),
    "KingOfTheHill.asm": ("menus_modes", "partial", "gmremixkoth*.c, mnremixgoalpicker.c"),
    "TagTeam.asm": ("menus_modes", "partial", "gmremixtagteam*.c"),
    "Smashketball.asm": ("menus_modes", "partial", "gmremixsmashketball*.c, itremixbasketball.c"),
    "TwelveCharBattle.asm": ("menus_modes", "partial", "gmremixtwelvecb*.c"),
    "StockMode.asm": ("menus_modes", "partial", "gmremixstockmode.c"),
    "Teams.asm": ("menus_modes", "partial", "scmanager.c, ftremixstamina*.c"),
    "Training.asm": ("menus_modes", "partial", "sc1ptrainingmode.c, remix_training.* (Z menu as port menu)"),
    "Practice_1P.asm": ("menus_modes", "partial", "gmremixpractice.c"),
    "SinglePlayer.asm": ("menus_modes", "partial", "sc1premix*.c"),
    "SinglePlayerModes.asm": ("menus_modes", "partial", "sc1premixintro/allstar/multiman*.c"),
    "Bonus.asm": ("menus_modes", "partial", "sc1premixbonusexit.c, sc1premixmultiman.c"),
    "HRC.asm": ("menus_modes", "partial", "sc1premixhrc*.c"),
    "CharacterDataScreen.asm": ("menus_modes", "partial", "mnremixchardata.c, mncharacters.c"),
    "Credits.asm": ("menus_modes", "partial", "scstaffroll.c"),
    "Gallery.asm": ("menus_modes", "partial", "mnremixgallery.c"),
    "GameEnd.asm": ("menus_modes", "partial", "gmremixgameend.c"),
    "Handicap.asm": ("menus_modes", "partial", "ftremixhandicap.c"),
    "InputDisplay.asm": ("menus_modes", "partial", "ftremixinputdisplay.c"),
    "PlayerTag.asm": ("menus_modes", "partial", "ifremixplayertag.c"),
    "resultsscreen.asm": ("menus_modes", "partial", "mnremixwinbgm.c, mnvsresults.c"),
    "Shield.asm": ("menus_modes", "partial", "ftremixshield.c"),
    "Transitions.asm": ("menus_modes", "partial", "lbtransition.c"),
    "VsStats.asm": ("menus_modes", "partial", "mnremixvsstats.c"),
    "PokemonAnnouncer.asm": ("fighters", "ported", "ifremixannouncer.c, ifremixhitannouncer.c"),
    "OnHit.asm": ("fighters", "ported", "ftremixchartables.c ftRemixRunOnHit"),
    "SinglePlayerMenus.asm": ("menus_modes", "partial", "mnremix1pmode.c"),
    "VsRemixMenu.asm": ("menus_modes", "ported", "mnremixvsmode.c"),
    "Menu.asm": ("infra", "n/a", "replaced by the port menu (ROADMAP T4.1 deviation)"),
    "Walljump.asm": ("infra", "n/a", "all patches are commented out in the Remix 2.0.1 build"),
    "Practice.asm": ("menus_modes", "ported", "ftremixzcancel.c (Flash On Z-Cancel), ftremixoverlay.c (Spawned Fear overlay n/a, M11)"),
    "CharacterSelectDebugMenu.asm": ("menus_modes", "partial", "remix_player_panel.cpp, mnremixcsspanelmenu*.c"),
    "SwordTrail.asm": ("fighters", "ported", "ftremixswordtrail.c"),
    "Skeleton.asm": ("fighters", "ported", "ftremixskeleton.c (per-port force)"),
    "AerialAttackFastFall.asm": ("fighters", "ported", "ftremixaerialfastfall.c"),
    "DKMode.asm": ("fighters", "ported", "ftremixdkmode.c (port variable: the RAM image is read-only)"),
    "Cheats.asm": ("menus_modes", "ported", "only the unlock word is live in the asm (Cheats.asm:19-37 is commented out); unlock all is a port toggle"),
    "Crash.asm": ("infra", "adapted", "N64 crash debugger; the port crash handler logs the backtrace without the button sequence"),
    "VsDemo.asm": ("menus_modes", "partial", "scremixvsdemo.c"),
    "timeouts.asm": ("menus_modes", "ported", "mnremixtimeouts.c"),
    "Settings.asm": ("menus_modes", "ported", "scremixtournament.c (tournament rules)"),
    "SinglePlayerEnemy.asm": ("menus_modes", "ported", "sc1premixenemy*.c (all seven groups)"),
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
        return ("ai", "ported", "ftremixaitables.c reads the attack table from the Remix image")
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
            code_open = False
        elif parts[0] == "items":
            area, status, where = item_row(asm)
            code_open = status == "partial"
        else:
            area, status, where = fighter_row(parts[0], asm)
            code_open = status == "partial"
        rows.append((asm.relative_to(SRC).as_posix(), area, status, where, code_open))
    if not rows:
        sys.exit(f"no .asm files under {SRC}")
    return rows


def includer_map(paths: list, sources: dict) -> dict:
    """Maps a skipped include (e.g. css/DpadControl.asm, lowercased) to the row path that includes it."""
    includers = {}
    for path in paths:
        for included in ASM_INCLUDE.findall(sources[path]):
            if any(part in SKIP_DIRS for part in Path(included).parts[:-1]):
                includers.setdefault(included.lower(), path)
    return includers


def table_patchers(paths: list, sources: dict) -> dict:
    """Maps a Character table name to the row paths that patch an entry of it (plus Character.asm, which declares it)."""
    patchers = {}
    for path in paths:
        for table in TABLE_PATCH.findall(sources[path]):
            patchers.setdefault(table, {"Character.asm"}).add(path)
    return patchers


class AsmResolver:
    """Maps the .asm references the audit docs write (any case, with or without folder, maybe with a line) to rows."""

    def __init__(self, paths: list):
        sources = {path: (SRC / path).read_text(errors="ignore") for path in paths}
        self.line_counts = {path: text.count("\n") + 1 for path, text in sources.items()}
        self.by_path = {path.lower(): path for path in paths}
        self.by_name = {}
        for path in paths:
            self.by_name.setdefault(Path(path).name.lower(), []).append(path)
        self.includers = includer_map(paths, sources)
        self.tables = table_patchers(paths, sources)

    def files(self, name: str, line: str = "") -> list:
        """Rows for one reference; a cited line past the end of a same-name match picks the file that has it."""
        key = name.lower()
        if key in self.includers:
            return [self.includers[key]]
        candidates = [self.by_path[key]] if key in self.by_path else self.by_name.get(Path(key).name, [])
        if line and len(candidates) <= 1:
            longer = [path for path in self.by_name.get(Path(key).name, []) if self.line_counts[path] >= int(line)]
            if candidates and self.line_counts[candidates[0]] < int(line) and longer:
                return longer
        return candidates

    def cell_files(self, cell: str) -> list:
        """Rows for every .asm reference of a ledger or scope-map cell."""
        return [path for name, line in ASM_REF.findall(cell) for path in self.files(name, line)]

    def table_files(self, cell: str) -> list:
        """Rows that patch the Character tables a CT cell names in backticks."""
        return [path for table in TABLE_NAME.findall(cell) for path in sorted(self.tables.get(table, ()))]


def ledger_item_files(item_id: str, cells: tuple, heading, resolver: AsmResolver) -> list:
    """Row paths of an item: the .asm of its asm cell, else of its scope cell, else the files patching the tables it
    names (Character.asm, which declares them, when every row is commented out), else the section file."""
    asm_cell, scope_cell = cells
    paths = resolver.cell_files(asm_cell) or resolver.cell_files(scope_cell)
    if not paths and item_id.startswith("CT-"):
        paths = resolver.table_files(asm_cell + " " + scope_cell) or resolver.files("Character.asm")
    if not paths and heading:
        paths = resolver.files(heading)
    return sorted(set(paths))


def load_ledger(resolver: AsmResolver) -> dict:
    """Maps each row path to its ledger items, as (id, state) with state one of LEDGER_STATES."""
    items = {}
    unplaced = []
    heading = None
    for number, line in enumerate(LEDGER.read_text().splitlines(), 1):
        if line.startswith("#"):
            match = FILE_HEADING.match(line)
            heading = match.group(1) if match else None
            continue
        match = LEDGER_ROW.match(line)
        if not match:
            continue
        item_id, state_cell, asm_cell, scope_cell = match.groups()
        state = state_cell.split()[0]
        if state not in LEDGER_STATES:
            sys.exit(f"{LEDGER.relative_to(ROOT)}:{number}: {item_id} state {state!r}, expected one of {LEDGER_STATES}")
        paths = ledger_item_files(item_id, (asm_cell, scope_cell), heading, resolver)
        if not paths:
            unplaced.append(item_id)
        for path in paths:
            items.setdefault(path, []).append((item_id, state))
    if unplaced:
        sys.exit(f"ledger items with no .asm row (name the file in the asm column): {', '.join(unplaced)}")
    return items


def load_audited(resolver: AsmResolver) -> set:
    """Row paths with at least one line in the per-scope map (a line names its file, or sits under its heading)."""
    audited = set()
    heading = None
    for line in SCOPES.read_text().splitlines():
        if line.startswith("#"):
            match = FILE_HEADING.match(line)
            heading = match.group(1) if match else None
            continue
        if not line.startswith("- "):
            continue
        paths = resolver.cell_files(line)
        if not paths and heading:
            paths = resolver.files(heading)
        audited.update(paths)
    return audited


def ledger_status(fallback: str, code_open: bool, items: list, audited: bool) -> str:
    """Status of one row from its fallback (STATUS or the code check) and its ledger items."""
    if code_open or any(state == "open" for _, state in items):
        return "partial"
    if items:
        return "adapted" if fallback == "adapted" else "ported"
    if fallback in ("partial", "todo") and audited:
        return "ported"
    return fallback


def ledger_note(items: list) -> str:
    """Short ledger summary for the Port column: counts per state and the open IDs."""
    if not items:
        return ""
    counts = {state: sum(1 for _, s in items if s == state) for state in LEDGER_STATES}
    note = f"ledger: {counts['done']} done, {counts['n/a']} n/a, {counts['open']} open"
    open_ids = sorted({item_id for item_id, state in items if state == "open"},
                      key=lambda item_id: (item_id.split("-")[0], int(item_id.split("-")[1])))
    return note + (f" ({', '.join(open_ids)})" if open_ids else "")


def apply_ledger(rows: list) -> list:
    """Rows with the ledger status and the ledger note added to the port location."""
    resolver = AsmResolver([row[0] for row in rows])
    items = load_ledger(resolver)
    audited = load_audited(resolver)
    result = []
    for path, area, status, where, code_open in rows:
        own = items.get(path, [])
        final = ledger_status(status, code_open, own, path in audited)
        note = ledger_note(own)
        result.append((path, area, final, "; ".join(part for part in (where, note) if part)))
    return result


def summary_lines(rows: list) -> list:
    lines = ["| Area | ported | adapted | partial | todo | n/a | progress |", "|---|---|---|---|---|---|---|"]
    for area in AREAS:
        counts = {key: sum(1 for row in rows if row[1] == area and row[2] == key)
                  for key in ("ported", "adapted", "partial", "todo", "n/a")}
        done = counts["ported"] + counts["adapted"]
        scored = done + counts["partial"] + counts["todo"]
        progress = (done + counts["partial"] / 2) / scored * 100 if scored else 100.0
        lines.append(f"| {area} | {counts['ported']} | {counts['adapted']} | {counts['partial']} | {counts['todo']} | {counts['n/a']} | "
                     f"{progress:.0f}% |")
    return lines


def load_features() -> dict:
    """One-line feature summary per Remix .asm path (FEATURES, tab separated), so each parity row says what the file adds."""
    features = {}
    for number, line in enumerate(FEATURES.read_text().splitlines(), 1):
        if not line.strip():
            continue
        path, sep, summary = line.partition("\t")
        if not sep:
            sys.exit(f"{FEATURES.relative_to(ROOT)}:{number}: expected '<asm path><TAB><summary>', got {line!r}")
        features[path] = summary.strip()
    return features


def main() -> None:
    rows = apply_ledger(collect_rows())
    features = load_features()
    out = ["# Smash Remix 2.0.1 parity (generated by tools/gen_remix_parity.py; do not edit by hand)", "",
           "One row per Remix `.asm`. The status comes from the item states of `patch_audit.md` (a file with an open item",
           "is `partial`); rerun the script after a ledger state changes. The Port column lists the open IDs of each file.", ""]
    out += summary_lines(rows)
    out += ["", "| File | Feature | Area | Status | Port |", "|---|---|---|---|---|"]
    out += [f"| `{path}` | {features.get(path, '')} | {area} | {status} | {where} |" for path, area, status, where in rows]
    OUTPUT.write_text("\n".join(out) + "\n")
    print(f"{len(rows)} rows -> {OUTPUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
