#!/usr/bin/env python3
"""Generate the Switch homebrew icons from assets/icon.png.

The original BattleShip icon ("BS" over a black starburst on a blue tile) is the base. The lettering is
erased and redrawn as "SR" (Smash Remix) in the same style: a yellow-to-orange fill, a red outline and a
black offset shadow. The starburst is rebuilt under the lettering from the vertices below.

Outputs:
  assets/icon-remix.png            1024x1024 RGBA, rounded tile like assets/icon.png
  assets/switch/icon.jpg           256x256 JPEG for the original BattleShip .nro
  assets/switch/icon-remix.jpg     256x256 JPEG for the BattleShip-Remix .nro

Needs Pillow only (run it with the repo venv: .toolchain/host/venv/bin/python -I tools/gen_switch_icon.py).
"""

import random
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont, ImageOps

REPO_ROOT = Path(__file__).resolve().parent.parent
ASSETS = REPO_ROOT / "assets"
SOURCE_PNG = ASSETS / "icon.png"
REMIX_PNG = ASSETS / "icon-remix.png"
SWITCH_DIR = ASSETS / "switch"
FONT_PATH = "/System/Library/Fonts/Supplemental/Arial Black.ttf"

SIZE = 1024
LETTER_BOX = (150, 225, 925, 830)
STAR_POINTS = [
    (490, 82), (580, 283), (693, 125), (732, 272), (917, 187), (825, 345), (998, 486), (845, 572),
    (965, 800), (822, 775), (790, 917), (700, 782), (610, 943), (478, 797), (387, 937), (362, 822),
    (243, 909), (200, 780), (62, 820), (216, 610), (25, 492), (214, 418), (77, 188), (278, 268),
    (290, 117), (395, 268),
]
LETTERS = "SR"
LETTER_SIZE = 440
LETTER_STRETCH_Y = 1.38
LETTER_ADVANCE = 322
LETTER_ORIGIN = (225, 312)
OUTLINE_GROW = 27
SHADOW_OFFSET = (22, 26)
FILL_TOP = (255, 238, 0)
FILL_MID = (255, 176, 0)
FILL_BOTTOM = (255, 118, 0)
GRUNGE_COARSE_CELLS = 36
GRUNGE_FINE_CELLS = 130
GRUNGE_FINE_WEIGHT = 0.5
GRUNGE_TOP_BIAS = 225
GRUNGE_BOTTOM_BIAS = 40
OUTLINE_COLOR = (255, 0, 0)
SUPERSAMPLE = 3
SHADOW_REACH = 13
RNG_SEED = 64


def is_black(pixel):
    return pixel[0] < 45 and pixel[1] < 45 and pixel[2] < 45


def is_warm(pixel):
    red, green, blue = pixel
    return red > 190 and blue < 90 and (green < 80 or green > 90)


def lettering_mask(source):
    """Mask of the old lettering: warm colours, plus the black shadow close to them."""
    warm = Image.new("L", (SIZE, SIZE), 0)
    black = Image.new("L", (SIZE, SIZE), 0)
    warm_pixels = warm.load()
    black_pixels = black.load()
    source_pixels = source.load()
    left, top, right, bottom = LETTER_BOX
    for y in range(top, bottom):
        for x in range(left, right):
            pixel = source_pixels[x, y][:3]
            if is_warm(pixel):
                warm_pixels[x, y] = 255
            elif is_black(pixel):
                black_pixels[x, y] = 255
    near_warm = warm.resize((SIZE // 4, SIZE // 4), Image.BOX).point(lambda value: 255 if value else 0)
    near_warm = near_warm.filter(ImageFilter.MaxFilter(SHADOW_REACH)).resize((SIZE, SIZE), Image.NEAREST)
    shadow = ImageChops.multiply(black, near_warm)
    return ImageChops.lighter(warm.filter(ImageFilter.MaxFilter(9)), shadow.filter(ImageFilter.MaxFilter(5)))


def starburst_mask():
    scale = SUPERSAMPLE
    big = Image.new("L", (SIZE * scale, SIZE * scale), 0)
    ImageDraw.Draw(big).polygon([(x * scale, y * scale) for x, y in STAR_POINTS], fill=255)
    return big.resize((SIZE, SIZE), Image.LANCZOS)


def nearest_blue(source_pixels, replace_pixels, x, y):
    for distance in range(1, 260):
        for step in (-distance, distance):
            nx = x + step
            if 0 <= nx < SIZE and replace_pixels[nx, y] == 0 and source_pixels[nx, y][3] == 255:
                return source_pixels[nx, y]
    return (118, 149, 199, 255)


def clean_tile(source):
    """The source tile with the lettering erased: black inside the starburst, blue elsewhere."""
    replace = lettering_mask(source)
    star = starburst_mask()
    source_pixels = source.load()
    replace_pixels = replace.load()
    star_pixels = star.load()
    tile = source.copy()
    tile_pixels = tile.load()
    left, top, right, bottom = LETTER_BOX
    for y in range(top - 6, bottom + 6):
        for x in range(left - 6, right + 6):
            if replace_pixels[x, y] == 0:
                continue
            blue = nearest_blue(source_pixels, replace_pixels, x, y)
            coverage = star_pixels[x, y] / 255
            tile_pixels[x, y] = tuple(
                round(blue[channel] * (1 - coverage)) for channel in range(3)
            ) + (255,)
    return tile


def glyph_layer(character):
    font = ImageFont.truetype(FONT_PATH, LETTER_SIZE)
    layer = Image.new("L", (SIZE, SIZE), 0)
    glyph = Image.new("L", (SIZE, SIZE), 0)
    ImageDraw.Draw(glyph).text((0, 0), character, font=font, fill=255)
    box = glyph.getbbox()
    cropped = glyph.crop(box)
    stretched = cropped.resize((cropped.width, round(cropped.height * LETTER_STRETCH_Y)), Image.LANCZOS)
    return layer, stretched


def lettering_shapes():
    """The filled glyph shape of the whole word, letters overlapping like the BS lettering."""
    shape = Image.new("L", (SIZE, SIZE), 0)
    for index, character in enumerate(LETTERS):
        layer, glyph = glyph_layer(character)
        layer.paste(glyph, (LETTER_ORIGIN[0] + index * LETTER_ADVANCE, LETTER_ORIGIN[1]))
        shape = ImageChops.lighter(shape, layer)
    rounded = shape.filter(ImageFilter.GaussianBlur(7)).point(lambda value: 255 if value > 128 else 0)
    return rounded


def grow(shape, radius):
    return shape.filter(ImageFilter.MaxFilter(radius * 2 + 1)).filter(ImageFilter.GaussianBlur(2)).point(
        lambda value: 255 if value > 110 else 0
    )


def offset(shape, dx, dy):
    return ImageChops.offset(shape, dx, dy)


def noise_field(cells, sigma, seed):
    """Smooth random field, 0..255, whose blobs are about SIZE / cells pixels wide."""
    random.seed(seed)
    field = Image.effect_noise((cells, cells), sigma).resize((SIZE, SIZE), Image.BICUBIC)
    return ImageOps.autocontrast(field, cutoff=1)


def vertical_ramp(box, top_value, bottom_value):
    ramp = Image.new("L", (SIZE, SIZE), top_value)
    draw = ImageDraw.Draw(ramp)
    height = box[3] - box[1]
    for y in range(box[1], box[3]):
        ratio = (y - box[1]) / height
        draw.line([(0, y), (SIZE, y)], fill=round(top_value + (bottom_value - top_value) * ratio))
    return ramp


def distressed_patches(box, seed):
    """Mask of ragged yellow patches: dense at the top of the letters, gone near the bottom."""
    coarse = noise_field(GRUNGE_COARSE_CELLS, 90, seed)
    fine = noise_field(GRUNGE_FINE_CELLS, 90, seed + 1)
    field = ImageChops.add(Image.blend(coarse, fine, GRUNGE_FINE_WEIGHT), vertical_ramp(box, GRUNGE_TOP_BIAS, GRUNGE_BOTTOM_BIAS), 1, -128)
    return field.point(lambda value: 255 if value > 128 else 0)


def gradient_fill(shape):
    box = shape.getbbox()
    fill = Image.new("RGB", (SIZE, SIZE), FILL_TOP)
    pixels = fill.load()
    height = box[3] - box[1]
    for y in range(box[1], box[3]):
        ratio = min(1.0, max(0.0, (y - box[1]) / height * 1.1))
        base = tuple(FILL_MID[channel] + (FILL_BOTTOM[channel] - FILL_MID[channel]) * ratio for channel in range(3))
        for x in range(box[0], box[2]):
            pixels[x, y] = tuple(round(min(255, max(0, value))) for value in base)
    yellow = Image.new("RGB", (SIZE, SIZE), FILL_TOP)
    fill.paste(yellow, mask=distressed_patches(box, RNG_SEED))
    return fill


def compose_lettering(tile):
    shape = lettering_shapes()
    outline = grow(shape, OUTLINE_GROW)
    shadow = offset(outline, *SHADOW_OFFSET)
    black = Image.new("RGB", (SIZE, SIZE), (0, 0, 0))
    red = Image.new("RGB", (SIZE, SIZE), OUTLINE_COLOR)
    rgb = tile.convert("RGB")
    rgb.paste(black, mask=shadow.filter(ImageFilter.GaussianBlur(1)))
    rgb.paste(red, mask=outline.filter(ImageFilter.GaussianBlur(1)))
    rgb.paste(gradient_fill(shape), mask=shape.filter(ImageFilter.GaussianBlur(1)))
    result = rgb.convert("RGBA")
    result.putalpha(tile.split()[3])
    return result


def write_switch_jpeg(tile, path):
    flat = Image.new("RGB", tile.size, (0, 0, 0))
    flat.paste(tile, mask=tile.split()[3])
    path.parent.mkdir(parents=True, exist_ok=True)
    flat.resize((256, 256), Image.LANCZOS).save(path, "JPEG", quality=92, optimize=True)


def main():
    source = Image.open(SOURCE_PNG).convert("RGBA")
    remix = compose_lettering(clean_tile(source))
    remix.save(REMIX_PNG)
    write_switch_jpeg(source, SWITCH_DIR / "icon.jpg")
    write_switch_jpeg(remix, SWITCH_DIR / "icon-remix.jpg")
    print(f"wrote {REMIX_PNG}, {SWITCH_DIR / 'icon.jpg'}, {SWITCH_DIR / 'icon-remix.jpg'}")


if __name__ == "__main__":
    main()
