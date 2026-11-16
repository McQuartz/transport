"""Renders the app icon resources/icon.png (1024x1024) from the logo geometry: a dark rounded tile,
two overlapping discs (light bottom left, grey top right) with the overlap in theme::accent.
Same shapes as the logo SVG (round 13, "3C1"); drawn at 4x and scaled down for clean edges.

    python3 tools/make_icon.py      (needs Pillow)
"""
import os
from PIL import Image, ImageChops, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

TILE = (28, 28, 28, 255)        # #1c1c1c
LIGHT = (242, 242, 242, 255)    # #f2f2f2
GREY = (85, 85, 85, 255)        # #555555
ACCENT = (255, 101, 38, 255)    # theme::accent #ff6526

# geometry in a 1024 box
TILE_BOX, TILE_RADIUS = (64, 64, 960, 960), 200
DISC_R = 220
DISC_A = (430.1, 569.4)   # light, bottom left
DISC_B = (593.9, 454.6)   # grey, top right


def disc_mask(size, centre, r, k):
    m = Image.new("L", (size, size), 0)
    cx, cy = centre
    ImageDraw.Draw(m).ellipse(((cx - r) * k, (cy - r) * k, (cx + r) * k, (cy + r) * k), fill=255)
    return m


def render(size=1024):
    k = size * 4 / 1024
    S = size * 4
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle(tuple(v * k for v in TILE_BOX), TILE_RADIUS * k, fill=TILE)
    a = disc_mask(S, DISC_A, DISC_R, k)
    b = disc_mask(S, DISC_B, DISC_R, k)
    img.paste(LIGHT, (0, 0), a)
    img.paste(GREY, (0, 0), b)
    img.paste(ACCENT, (0, 0), ImageChops.multiply(a, b))
    return img.resize((size, size), Image.LANCZOS)


if __name__ == "__main__":
    out = os.path.join(ROOT, "resources", "icon.png")
    render().save(out)
    print("wrote", out)
