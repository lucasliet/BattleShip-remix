#!/usr/bin/env python3
"""Contact sheet of the PNG files in a directory: sheet.py <dir> [columns] [cell-width]. Writes <dir>/sheet.png."""
import sys
from pathlib import Path
from PIL import Image, ImageDraw

directory = Path(sys.argv[1])
columns = int(sys.argv[2]) if len(sys.argv) > 2 else 3
cell_width = int(sys.argv[3]) if len(sys.argv) > 3 else 320
cell_height = cell_width * 3 // 4
files = sorted(path for path in directory.glob("*.png") if path.name != "sheet.png")
rows = (len(files) + columns - 1) // columns
sheet = Image.new("RGB", (cell_width * columns, cell_height * max(rows, 1)))
for index, path in enumerate(files):
    cell = Image.open(path).convert("RGB").resize((cell_width, cell_height))
    ImageDraw.Draw(cell).text((4, 2), path.stem, fill=(255, 255, 0))
    sheet.paste(cell, ((index % columns) * cell_width, (index // columns) * cell_height))
sheet.save(directory / "sheet.png")
print(directory / "sheet.png")
