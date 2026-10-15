"""Contact sheet of the screenshots of one run.

Usage: .toolchain/host/venv/bin/python -I .claude/skills/remix-smoke-tests/scripts/sheet.py <name> [--step N] [--max N]
       [--crop x0,y0,x1,y1]
Reads build/agent-shots/vs/<name>/frame_*.png in frame order, keeps every Nth (default 1, at most --max, default 20),
optionally crops a fraction of the frame (0..1 each, e.g. 0.45,0.35,0.85,0.75), labels each tile with its frame and
writes build/agent-shots/vs/<name>/sheet.png (5 columns).
"""
import argparse
import glob
import sys
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[4]
SHOTS = ROOT / "build/agent-shots/vs"
TILE = (320, 240)
COLUMNS = 5


def frame_number(path: str) -> int:
    return int(Path(path).stem.split("_")[-1])


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("name")
    parser.add_argument("--step", type=int, default=1)
    parser.add_argument("--max", type=int, default=20)
    parser.add_argument("--crop", default=None)
    args = parser.parse_args()

    shots = sorted(glob.glob(str(SHOTS / args.name / "frame_*.png")), key=frame_number)[::args.step][:args.max]
    if not shots:
        sys.exit(f"no frame_*.png in {SHOTS / args.name}")
    crop = [float(v) for v in args.crop.split(",")] if args.crop else None

    rows = (len(shots) + COLUMNS - 1) // COLUMNS
    sheet = Image.new("RGB", (COLUMNS * TILE[0], rows * TILE[1]))
    for index, path in enumerate(shots):
        image = Image.open(path).convert("RGB")
        if crop:
            width, height = image.size
            image = image.crop((int(width * crop[0]), int(height * crop[1]), int(width * crop[2]), int(height * crop[3])))
        image = image.resize(TILE)
        ImageDraw.Draw(image).text((3, 3), str(frame_number(path)), fill=(255, 255, 0))
        sheet.paste(image, ((index % COLUMNS) * TILE[0], (index // COLUMNS) * TILE[1]))
    out = SHOTS / args.name / "sheet.png"
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
