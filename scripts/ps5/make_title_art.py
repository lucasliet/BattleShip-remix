"""Builds the PS5 title background (sce_sys/pic0.dds) from the repo artwork.

The console wants a 3840x2160 BC7 DDS (DX10 header). The image is the blue wave texture of
assets/macos_dmg_banner.png with assets/SmashRemix.png on top, encoded with BC7 mode 6 only.

Usage: .toolchain/host/venv/bin/python -I scripts/ps5/make_title_art.py <repo root> <out.dds>
(needs numpy and Pillow in the venv).
"""

import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

WIDTH = 3840
HEIGHT = 2160
DXGI_FORMAT_BC7_UNORM = 98
DDS_DIMENSION_TEXTURE2D = 3
BANNER_TEXTURE_TOP = 460
LOGO_WIDTH = 2600
LOGO_TOP = 260


def compose_background(root: Path) -> Image.Image:
    """Scales the logo-free part of the banner to cover 4K and pastes the Remix logo on it."""
    banner = Image.open(root / "assets/macos_dmg_banner.png").convert("RGB")
    texture = banner.crop((0, BANNER_TEXTURE_TOP, banner.width, banner.height))
    scale = max(WIDTH / texture.width, HEIGHT / texture.height)
    texture = texture.resize((round(texture.width * scale), round(texture.height * scale)), Image.LANCZOS)
    left = (texture.width - WIDTH) // 2
    top = (texture.height - HEIGHT) // 2
    canvas = texture.crop((left, top, left + WIDTH, top + HEIGHT)).convert("RGBA")

    logo = Image.open(root / "assets/SmashRemix.png").convert("RGBA")
    logo = logo.resize((LOGO_WIDTH, round(logo.height * LOGO_WIDTH / logo.width)), Image.LANCZOS)
    canvas.alpha_composite(logo, ((WIDTH - LOGO_WIDTH) // 2, LOGO_TOP))
    return canvas


def to_blocks(image: Image.Image) -> np.ndarray:
    """Splits an RGBA image into 4x4 blocks in raster order: (blocks, 16, 4) uint8."""
    pixels = np.asarray(image, dtype=np.uint8)
    rows, cols = HEIGHT // 4, WIDTH // 4
    return pixels.reshape(rows, 4, cols, 4, 4).transpose(0, 2, 1, 3, 4).reshape(rows * cols, 16, 4)


def bits_lsb_first(values: np.ndarray, count: int) -> np.ndarray:
    """Expands each value to `count` bits, least significant first: (blocks, count) uint8."""
    shifts = np.arange(count, dtype=np.uint32)
    return ((values.astype(np.uint32)[:, None] >> shifts) & 1).astype(np.uint8)


def encode_bc7_mode6(blocks: np.ndarray) -> bytes:
    """Encodes blocks with BC7 mode 6: one RGBA line per block, 7-bit endpoints, p-bits fixed at 1."""
    quantized_low = blocks.min(axis=1) >> 1
    quantized_high = blocks.max(axis=1) >> 1
    low = (quantized_low.astype(np.float32) * 2 + 1)
    high = (quantized_high.astype(np.float32) * 2 + 1)

    direction = high - low
    length_squared = np.maximum((direction * direction).sum(axis=1, keepdims=True), 1e-6)
    projection = ((blocks.astype(np.float32) - low[:, None, :]) * direction[:, None, :]).sum(axis=2)
    indices = np.clip(np.rint(projection / length_squared * 15), 0, 15).astype(np.uint8)

    # The anchor (first) index is stored with 3 bits, so its top bit must be 0.
    swap = indices[:, 0] >= 8
    indices[swap] = 15 - indices[swap]
    quantized_low[swap], quantized_high[swap] = quantized_high[swap].copy(), quantized_low[swap].copy()

    fields = [bits_lsb_first(np.full(len(blocks), 1 << 6), 7)]
    for channel in range(4):
        fields.append(bits_lsb_first(quantized_low[:, channel], 7))
        fields.append(bits_lsb_first(quantized_high[:, channel], 7))
    fields.append(np.ones((len(blocks), 2), dtype=np.uint8))
    fields.append(bits_lsb_first(indices[:, 0], 3))
    for pixel in range(1, 16):
        fields.append(bits_lsb_first(indices[:, pixel], 4))
    bits = np.concatenate(fields, axis=1)
    if bits.shape[1] != 128:
        raise ValueError(f"BC7 block has {bits.shape[1]} bits, expected 128")
    return np.packbits(bits, axis=1, bitorder="little").tobytes()


def dds_header() -> bytes:
    """DDS + DX10 header for a single-mip 3840x2160 BC7 texture (148 bytes)."""
    linear_size = (WIDTH // 4) * (HEIGHT // 4) * 16
    header = struct.pack("<4sIIIIIII", b"DDS ", 124, 0x000A1007, HEIGHT, WIDTH, linear_size, 0, 1)
    header += b"\0" * 44
    header += struct.pack("<II4sIIIII", 32, 0x4, b"DX10", 0, 0, 0, 0, 0)
    header += struct.pack("<IIIII", 0x1000, 0, 0, 0, 0)
    header += struct.pack("<IIIII", DXGI_FORMAT_BC7_UNORM, DDS_DIMENSION_TEXTURE2D, 0, 1, 0)
    return header


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit("usage: make_title_art.py <repo root> <out.dds>")
    root, output = Path(sys.argv[1]), Path(sys.argv[2])
    image = compose_background(root)
    image.convert("RGB").save(output.with_suffix(".preview.png"))
    output.write_bytes(dds_header() + encode_bc7_mode6(to_blocks(image)))


if __name__ == "__main__":
    main()
