#!/usr/bin/env python3
"""Validate that every asset the game loads actually exists and is readable.

Three checks, each one corresponding to a bug that reached main:

  1. Path case. Every "assets/..." literal in src/*.c must resolve on a
     case-sensitive filesystem. src/minimap.c shipped seven literals spelling
     "ressources" while the directory is "Ressources", which made init_temps()
     fail at boot with the font never loading.

  2. PNG integrity. Chunk CRCs and a terminating IEND. The README long claimed
     assets were corrupt and that libpng aborted on them; this check is what
     turns that question into a build result instead of a rumour.

  3. Declared dimensions, against assets/MANIFEST.tsv. draw_level1_scene()
     blits a 1150x650 source rect out of a 2048x341 Niv1.png, so the bottom
     309px of the screen was never written. A backdrop too small to cover the
     screen should fail the build, not render garbage.

Exits non-zero on any failure. Run via `make verify`.
"""

import glob
import os
import re
import struct
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC_GLOB = os.path.join(ROOT, "src", "*.c")
MANIFEST = os.path.join(ROOT, "assets", "MANIFEST.tsv")

# Matches a double-quoted literal that looks like an asset path. printf-style
# conversions are kept so they can be expanded to a glob below.
ASSET_LITERAL = re.compile(r'"(assets/[^"\n]*)"')
# Only %d/%i/%s appear in this codebase's sprintf asset paths.
CONVERSION = re.compile(r"%[0-9.]*[dis]")

errors = []
warnings = []


def strip_comments(text):
    """Remove /*...*/ and //... so commented-out loads are not treated as live."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def collect_literals():
    """Map each asset path literal to the source locations that reference it."""
    found = {}
    for path in sorted(glob.glob(SRC_GLOB)):
        raw = open(path, encoding="utf-8", errors="replace").read()
        live = strip_comments(raw)
        rel = os.path.relpath(path, ROOT)
        # Line numbers come from the raw text so they match the real file.
        lines = raw.splitlines()
        for literal in ASSET_LITERAL.findall(live):
            where = found.setdefault(literal, [])
            for n, line in enumerate(lines, 1):
                if '"' + literal + '"' in line:
                    where.append(f"{rel}:{n}")
    return found


def check_paths(found):
    """Every literal must resolve case-sensitively; formats must match >=1 file."""
    for literal, where in sorted(found.items()):
        sites = ", ".join(where) if where else "unknown"
        if CONVERSION.search(literal):
            # "image%d-%d.png" -> "image*-*.png"; the glob is case-sensitive,
            # so a wrong-case directory yields zero matches.
            pattern = CONVERSION.sub("*", literal)
            if not glob.glob(os.path.join(ROOT, pattern)):
                errors.append(
                    f"{sites}: pattern '{literal}' matches no file on disk"
                )
            continue

        full = os.path.join(ROOT, literal)
        if not os.path.exists(full):
            # Distinguish a wrong-case path from a genuinely absent one, since
            # the fix differs and the symptom at runtime is identical.
            hint = case_hint(literal)
            errors.append(f"{sites}: '{literal}' does not exist{hint}")
        elif not os.path.isfile(full):
            errors.append(f"{sites}: '{literal}' is not a regular file")


def case_hint(literal):
    """If only the case is wrong, say so and name the path that would work."""
    parts = literal.split("/")
    resolved = ROOT
    for i, part in enumerate(parts):
        candidate = os.path.join(resolved, part)
        if os.path.exists(candidate):
            resolved = candidate
            continue
        try:
            siblings = os.listdir(resolved)
        except OSError:
            return ""
        match = next((s for s in siblings if s.lower() == part.lower()), None)
        if match is None:
            return ""
        fixed = "/".join(parts[:i] + [match] + parts[i + 1:])
        return f" (case mismatch: did you mean '{fixed}'?)"
    return ""


def png_dimensions(path):
    """Validate every chunk CRC, require IEND, and return (width, height)."""
    data = open(path, "rb").read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG (bad signature)")

    offset, seen, size = 8, [], None
    while offset + 8 <= len(data):
        length = struct.unpack(">I", data[offset:offset + 4])[0]
        ctype = data[offset + 4:offset + 8].decode("latin1")
        body_end = offset + 8 + length
        crc_bytes = data[body_end:body_end + 4]
        if len(crc_bytes) < 4:
            raise ValueError(f"chunk '{ctype}' is truncated")
        want = struct.unpack(">I", crc_bytes)[0]
        got = zlib.crc32(data[offset + 4:body_end]) & 0xFFFFFFFF
        if want != got:
            raise ValueError(f"chunk '{ctype}' has a bad CRC")
        if ctype == "IHDR":
            size = struct.unpack(">II", data[offset + 8:offset + 16])
        seen.append(ctype)
        offset = body_end + 4

    if "IHDR" not in seen:
        raise ValueError("no IHDR chunk")
    if "IEND" not in seen:
        raise ValueError("no IEND chunk (file is truncated)")
    if offset != len(data):
        raise ValueError(f"{len(data) - offset} trailing bytes after IEND")
    return size


def check_pngs():
    """Integrity-check every PNG in assets/, not just the referenced ones."""
    pngs = sorted(glob.glob(os.path.join(ROOT, "assets", "**", "*.png"),
                            recursive=True))
    if not pngs:
        errors.append("assets/: no PNG files found at all")
    sizes = {}
    for path in pngs:
        rel = os.path.relpath(path, ROOT)
        try:
            sizes[rel] = png_dimensions(path)
        except (ValueError, OSError) as exc:
            errors.append(f"{rel}: {exc}")
    return sizes


def check_manifest(sizes):
    """Enforce assets/MANIFEST.tsv: path, width, height, role."""
    if not os.path.exists(MANIFEST):
        warnings.append(
            "assets/MANIFEST.tsv is absent; skipping dimension checks")
        return
    for n, line in enumerate(open(MANIFEST, encoding="utf-8"), 1):
        line = line.rstrip("\n")
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        fields = line.split("\t")
        if len(fields) < 3:
            errors.append(
                f"MANIFEST.tsv:{n}: expected path<TAB>w<TAB>h<TAB>role")
            continue
        rel, want_w, want_h = fields[0], fields[1], fields[2]
        if not os.path.exists(os.path.join(ROOT, rel)):
            errors.append(f"MANIFEST.tsv:{n}: '{rel}' does not exist")
            continue
        if rel not in sizes:
            # Non-PNG entries (fonts, audio) only need to exist.
            continue
        got_w, got_h = sizes[rel]
        if (str(got_w), str(got_h)) != (want_w, want_h):
            errors.append(
                f"MANIFEST.tsv:{n}: '{rel}' is {got_w}x{got_h}, "
                f"manifest declares {want_w}x{want_h}")


def main():
    found = collect_literals()
    if not found:
        errors.append("no asset path literals found in src/*.c (parser broken?)")
    check_paths(found)
    sizes = check_pngs()
    check_manifest(sizes)

    for warning in warnings:
        print(f"warning: {warning}")

    if errors:
        print(f"\nFAIL: {len(errors)} asset problem(s):\n", file=sys.stderr)
        for err in errors:
            print(f"  {err}", file=sys.stderr)
        return 1

    print(f"OK: {len(found)} asset references, {len(sizes)} PNGs validated")
    return 0


if __name__ == "__main__":
    sys.exit(main())
