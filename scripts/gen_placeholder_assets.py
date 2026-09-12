#!/usr/bin/env python3
"""Generate placeholder art for level 1, sized to the renderer's real needs.

Level 1 currently draws its obstacles, ground, and goal as SDL_FillRect
rectangles, and its backdrop is Niv1.png -- a 2048x341 image that
draw_level1_scene() blits a 1150x650 source rect out of, so the bottom 309
rows of the screen were never written (issue #14).

This produces correctly-sized stand-ins so the renderer can blit real art. The
output is deliberately flat and diagrammatic: it should read as intentional
placeholder, never be mistaken for finished art, and be replaceable 1:1 by
hand-drawn PNGs of the same dimensions. assets/MANIFEST.tsv records those
dimensions and scripts/verify_assets.py enforces them.

The backdrop reuses the existing Niv1.png skyline band rather than discarding
it, so the original art still shows through.

Deterministic: same input, same bytes out. Re-runnable. Requires Pillow.

    python3 scripts/gen_placeholder_assets.py [--force]
"""

import argparse
import os
import random
import sys

try:
    from PIL import Image, ImageDraw
except ImportError:
    sys.exit("Pillow is required: pip install Pillow")

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASSETS = os.path.join(ROOT, "assets", "img")

# Mirrors the constants in src/main_menu.c and include/perso.h. If those move,
# these must move with them -- verify_assets.py is what catches the drift.
SCREEN_W, SCREEN_H = 1150, 650
LEVEL1_W = 2300
GROUND_Y = 515                      # top of the drawn ground strip

# Parallax rates, matched to draw_level1_scene(). The far layer is the existing
# camera.x / 2; the near layer is new. Each layer must be wide enough that the
# widest source rect it is asked for still lands inside the image.
FAR_RATE = 0.5
NEAR_RATE = 0.75


def layer_width(rate):
    """Widest source x the camera can ask for at this rate, plus one screen."""
    return int((LEVEL1_W - SCREEN_W) * rate) + SCREEN_W


# Sampled from Niv1.png rather than invented, so generated pieces sit inside
# the existing art's palette instead of fighting it. Niv1.png is a finished
# volcanic backdrop: crimson sky, rock spires, lava shelves, dark ground band.
PALETTE = {
    "sky_high": (104, 18, 44),       # Niv1 top row, darkened for altitude
    "sky_low": (131, 25, 53),         # Niv1 top row as-is, meets the art
    "ground": (79, 19, 37),           # Niv1 bottom row: its own ground band
    "ground_dark": (52, 13, 26),
    "ground_light": (112, 33, 52),
    "crate": (226, 197, 143),         # the pale crates drawn into Niv1
    "crate_dark": (163, 128, 84),
    "crate_light": (245, 226, 186),
    "goal": (190, 170, 40),           # matches the existing SDL_FillRect
    "goal_bright": (247, 232, 120),
    "enemy": (150, 58, 58),
    "enemy_dark": (99, 36, 36),
    "enemy_eye": (242, 232, 226),
    "mark": (255, 0, 168),            # placeholder tell, see stamp()
}

# Niv1.png is 2048x341 and the screen is 650 tall, so 309 rows are missing.
# Rather than upscale 1.9x and lose crispness, the art is placed at native
# scale and its own edge colors are extended above and below. Placing its top
# at y=214 lands the art's ground band on the gameplay ground line at y=515.
NIV1_TOP_Y = 214

# Pixels trimmed from each side of Niv1.png before tiling, to drop its pale
# edge artifact. Small enough to cost no visible art.
EDGE_TRIM = 3


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def vertical_gradient(size, top, bottom):
    """A cheap vertical gradient; Pillow has no primitive for this."""
    w, h = size
    img = Image.new("RGB", size)
    draw = ImageDraw.Draw(img)
    for y in range(h):
        t = y / max(1, h - 1)
        draw.line(
            [(0, y), (w, y)],
            fill=tuple(round(a + (b - a) * t) for a, b in zip(top, bottom)))
    return img


def stamp(img, label):
    """Mark every generated file so placeholder art is obvious on screen.

    A magenta corner tick is the same tell load_image_safe() uses for a failed
    load, which keeps "this is not real art" a single visual vocabulary.
    """
    draw = ImageDraw.Draw(img)
    w, h = img.size
    size = max(3, min(w, h) // 24)
    draw.polygon([(w - size, h), (w, h), (w, h - size)], fill=PALETTE["mark"])
    return img


def make_backdrop(name, rate, with_alpha, seed):
    """Build one parallax layer at the width its rate actually requires."""
    width = layer_width(rate)

    if not with_alpha:
        img = Image.new("RGB", (width, SCREEN_H), PALETTE["sky_low"])
        draw = ImageDraw.Draw(img)
        niv1_path = os.path.join(ASSETS, "Niv1.png")
        if not os.path.exists(niv1_path):
            sys.exit(f"{niv1_path} is required to build the backdrop")

        band = Image.open(niv1_path).convert("RGBA")
        # Niv1.png carries a 2px near-white column on its left edge (an export
        # artifact; Niv2.png has one on the right). Untrimmed it tiles into a
        # white seam every 2048px and shows as a stripe at screen x=0. Trim a
        # small inset from every side rather than special-casing one edge.
        band = band.crop((EDGE_TRIM, EDGE_TRIM,
                          band.width - EDGE_TRIM, band.height - EDGE_TRIM))
        # Tile horizontally: the source is 2048 wide and the layer needs more
        # than that at some parallax rates. The art's spires are irregular
        # enough that a seam does not read as a repeat.
        tiled = Image.new("RGBA", (width, band.height))
        for x in range(0, width, band.width):
            tiled.paste(band, (x, 0))

        # Sky above the art, graded from its own top row so the join is invisible.
        sky = vertical_gradient((width, NIV1_TOP_Y + 2),
                                PALETTE["sky_high"], PALETTE["sky_low"])
        img.paste(sky, (0, 0))
        img.paste(tiled, (0, NIV1_TOP_Y), tiled)

        # Ground below the art, in the art's own ground colour.
        below = NIV1_TOP_Y + band.height
        draw.rectangle([0, below - 2, width, SCREEN_H], fill=PALETTE["ground"])
        out = img
    else:
        # Near layer: low foreground rock silhouettes in the art's darkest
        # tone, confined to the ground line. Deliberately sparse -- this adds
        # depth on movement and must never compete with the backdrop.
        out = Image.new("RGBA", (width, SCREEN_H), (0, 0, 0, 0))
        odraw = ImageDraw.Draw(out)
        rng = random.Random(seed)
        for i in range(width // 260 + 1):
            x = i * 260 + rng.randint(-40, 40)
            h = rng.randint(26, 58)
            w = rng.randint(70, 150)
            odraw.polygon([(x, GROUND_Y),
                           (x + w // 5, GROUND_Y - h),
                           (x + w // 2, GROUND_Y - h + rng.randint(-8, 8)),
                           (x + w, GROUND_Y)],
                          fill=PALETTE["ground_dark"] + (205,))
        odraw.rectangle([0, GROUND_Y, width, SCREEN_H],
                        fill=PALETTE["ground_dark"] + (150,))

    out = out.convert("RGBA")
    stamp(out, name)
    path = os.path.join(ASSETS, "levels", name)
    ensure_dir(os.path.dirname(path))
    out.save(path)
    return path, out.size


def make_crate(name, size):
    """A blocky obstacle, sized to the hardcoded level1_obstacles[] rects."""
    w, h = size
    img = Image.new("RGBA", size, PALETTE["crate"] + (255,))
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, w - 1, h - 1], outline=PALETTE["crate_dark"], width=3)
    # Plank seams, so the top face reads as standable once collision lands.
    draw.rectangle([0, 0, w - 1, 8], fill=PALETTE["crate_light"])
    for y in range(18, h, 26):
        draw.line([(3, y), (w - 4, y)], fill=PALETTE["crate_dark"], width=2)
    draw.line([(3, 3), (w - 4, h - 4)], fill=PALETTE["crate_dark"], width=2)
    draw.line([(w - 4, 3), (3, h - 4)], fill=PALETTE["crate_dark"], width=2)
    stamp(img, name)
    path = os.path.join(ASSETS, "level1", name)
    ensure_dir(os.path.dirname(path))
    img.save(path)
    return path, img.size


def make_goal(name, size):
    """The exit beacon. Both players must stand in it to finish the mission."""
    w, h = size
    img = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.polygon([(w // 2, 0), (w - 1, h - 1), (0, h - 1)],
                 fill=PALETTE["goal"] + (210,))
    for i in range(4):
        y = h - 1 - i * (h // 5)
        draw.line([(w // 2, y), (w // 2, y - h // 10)],
                  fill=PALETTE["goal_bright"], width=5)
    draw.ellipse([w // 2 - 14, 6, w // 2 + 14, 34],
                 fill=PALETTE["goal_bright"])
    stamp(img, name)
    path = os.path.join(ASSETS, "level1", name)
    ensure_dir(os.path.dirname(path))
    img.save(path)
    return path, img.size


def make_ground_tile(name, size):
    """Horizontally tileable ground strip for the y=515..650 band."""
    w, h = size
    img = Image.new("RGBA", size, PALETTE["ground"] + (255,))
    draw = ImageDraw.Draw(img)
    # Lit crust on top, so the standable surface is legible once vertical
    # collision lands; body darkens with depth.
    draw.rectangle([0, 0, w - 1, 4], fill=PALETTE["ground_light"])
    draw.rectangle([0, h // 2, w - 1, h - 1], fill=PALETTE["ground_dark"])
    rng = random.Random(7)
    for _ in range(12):
        x, y = rng.randint(0, w - 6), rng.randint(8, h - 8)
        draw.ellipse([x, y, x + 4, y + 3], fill=PALETTE["ground_light"])
    stamp(img, name)
    path = os.path.join(ASSETS, "level1", name)
    ensure_dir(os.path.dirname(path))
    img.save(path)
    return path, img.size


def make_enemy_frames():
    """Six walk frames per direction, matching the perso image[dir][frame] shape."""
    made = []
    size = 96
    for direction in (0, 1):
        for frame in range(6):
            img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
            draw = ImageDraw.Draw(img)
            bob = (0, -2, -3, -2, 0, 1)[frame]
            body_top = 30 + bob
            draw.rounded_rectangle([18, body_top, 78, 84], radius=12,
                                   fill=PALETTE["enemy"],
                                   outline=PALETTE["enemy_dark"], width=3)
            # Legs alternate so the patrol reads as walking, not sliding.
            swing = (-8, -3, 4, 8, 3, -4)[frame]
            draw.line([(38, 84), (38 + swing, 94)],
                      fill=PALETTE["enemy_dark"], width=7)
            draw.line([(58, 84), (58 - swing, 94)],
                      fill=PALETTE["enemy_dark"], width=7)
            eye_x = 60 if direction == 0 else 28
            draw.ellipse([eye_x - 8, body_top + 12, eye_x + 8, body_top + 28],
                         fill=PALETTE["enemy_eye"])
            pupil = eye_x + (4 if direction == 0 else -4)
            draw.ellipse([pupil - 3, body_top + 18, pupil + 3, body_top + 24],
                         fill=PALETTE["enemy_dark"])
            name = f"walk{direction}-{frame}.png"
            stamp(img, name)
            path = os.path.join(ASSETS, "enemy", name)
            ensure_dir(os.path.dirname(path))
            img.save(path)
            made.append((path, img.size))
    return made


ROLES = {
    "levels/niv1_far.png": "level1 backdrop, far parallax layer (camera.x * 0.5)",
    "levels/niv1_near.png": "level1 backdrop, near parallax layer (camera.x * 0.75)",
    "level1/crate.png": "obstacle, matches level1_obstacles[0] and [2]",
    "level1/crate_tall.png": "obstacle, matches level1_obstacles[1]",
    "level1/goal_beacon.png": "mission exit, matches level1_goal",
    "level1/ground_tile.png": "tileable ground strip for y=515..650",
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--force", action="store_true",
                        help="overwrite existing generated files")
    args = parser.parse_args()

    target = os.path.join(ASSETS, "levels", "niv1_far.png")
    if os.path.exists(target) and not args.force:
        print("Generated art already present; pass --force to regenerate.")
        return 0

    made = []
    made.append(make_backdrop("niv1_far.png", FAR_RATE, False, seed=1))
    made.append(make_backdrop("niv1_near.png", NEAR_RATE, True, seed=2))
    made.append(make_crate("crate.png", (80, 128)))
    made.append(make_crate("crate_tall.png", (90, 130)))
    made.append(make_goal("goal_beacon.png", (125, 180)))
    made.append(make_ground_tile("ground_tile.png", (64, 135)))
    enemies = make_enemy_frames()

    # Rewrite the manifest so declared dimensions can never drift from reality.
    lines = [
        "# Generated by scripts/gen_placeholder_assets.py -- do not hand-edit.",
        "# Enforced by scripts/verify_assets.py (make verify).",
        "# path\twidth\theight\trole",
    ]
    for path, size in made:
        rel = os.path.relpath(path, ROOT)
        key = rel.replace("assets/img/", "")
        lines.append(f"{rel}\t{size[0]}\t{size[1]}\t{ROLES.get(key, 'level1 art')}")
    for path, size in enemies:
        rel = os.path.relpath(path, ROOT)
        lines.append(f"{rel}\t{size[0]}\t{size[1]}\tpatrolling enemy walk frame")

    manifest = os.path.join(ROOT, "assets", "MANIFEST.tsv")
    with open(manifest, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")

    for path, size in made + enemies:
        print(f"  {size[0]:>5} x {size[1]:<5}  {os.path.relpath(path, ROOT)}")
    print(f"\nWrote {len(made) + len(enemies)} files and "
          f"{os.path.relpath(manifest, ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
