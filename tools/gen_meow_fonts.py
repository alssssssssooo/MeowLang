#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate the Meow-* symbol fonts used by MeowLang DOCX files.

Each generated font contains one visible glyph mapped to U+55B5 (喵).  The
glyph is composed from the ASCII outlines of a monospace source font, so a
run such as ``<w:rFonts w:ascii="Meow-return"/>喵</w:r>`` is displayed as
``return`` while the MeowLang interpreter still reads the original 喵 text.

The generated fonts are build artifacts but are committed to the repository
so users do not need fontTools to install them.

Usage:
    python3 tools/gen_meow_fonts.py
    python3 tools/gen_meow_fonts.py --check
    python3 tools/gen_meow_fonts.py --source /path/to/monospace.ttf

This script requires fontTools (pip install fonttools).  The generated fonts
are derived from Droid Sans Mono, which is licensed under Apache-2.0; see
fonts/NOTICE and fonts/LICENSE-APACHE-2.0.txt.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

try:
    from fontTools.fontBuilder import FontBuilder
    from fontTools.misc.transform import Transform
    from fontTools.pens.recordingPen import DecomposingRecordingPen
    from fontTools.pens.transformPen import TransformPen
    from fontTools.pens.ttGlyphPen import TTGlyphPen
    from fontTools.ttLib import TTFont
except ImportError as exc:  # pragma: no cover - only for developers
    raise SystemExit(
        "fontTools is required to regenerate the fonts. "
        "Install it with: python3 -m pip install fonttools"
    ) from exc

UPEM = 1000
ADVANCE = 500
CODEPOINT = 0x55B5
FONT_PREFIX = "Meow-"
VERSION = "1.0"

# Fallback list, kept in sync with native/meow.c.  The normal path parses
# native/meow.c so the fonts always match the runtime symbol table.
FALLBACK_NAMES = [
    "println", "add", "sub", "mul", "div", "mod", "pow", "neg", "abs",
    "min", "max", "sum", "eq", "ne", "lt", "le", "gt", "ge", "not",
    "contains", "list", "cons", "head", "tail", "len", "get", "set",
    "push", "pop", "concat", "slice", "range", "str", "parseInt",
    "textLen", "charAt", "substring", "join", "random", "srand", "clear",
    "if", "while", "for", "do", "return", "and", "or",
]

SOURCE_CANDIDATES = [
    "/system/fonts/DroidSansMono.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
]


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def builtin_names(meow_c: Path) -> list[str]:
    """Return the sorted unique Meow-* names referenced by native/meow.c."""
    if meow_c.is_file():
        text = meow_c.read_text(encoding="utf-8", errors="replace")
        names = sorted(set(re.findall(r'"Meow-([A-Za-z0-9_]+)"', text)))
        if names:
            return names
    return sorted(set(FALLBACK_NAMES))


def find_source_font(explicit: str | None) -> Path:
    if explicit:
        path = Path(explicit)
        if not path.is_file():
            raise SystemExit(f"source font not found: {path}")
        return path
    for candidate in SOURCE_CANDIDATES:
        path = Path(candidate)
        if path.is_file():
            return path
    raise SystemExit(
        "no monospace source font found; pass --source /path/to/font.ttf"
    )


def _notdef_glyph():
    pen = TTGlyphPen(None)
    pen.moveTo((50, 0))
    pen.lineTo((450, 0))
    pen.lineTo((450, 700))
    pen.lineTo((50, 700))
    pen.closePath()
    return pen.glyph()


def build_font(name: str, out_path: Path, source_path: Path) -> None:
    src = TTFont(source_path)
    src_upem = src["head"].unitsPerEm
    src_glyphs = src.getGlyphSet()
    cmap = src.getBestCmap()
    if not cmap:
        raise SystemExit(f"source font has no cmap: {source_path}")

    # Use the advance of a representative ASCII glyph so the composed name
    # has a uniform, compact rhythm in Word.
    probe = cmap.get(ord("n")) or cmap.get(ord("m")) or cmap.get(ord("0"))
    if not probe:
        raise SystemExit(f"source font has no ASCII glyphs: {source_path}")
    source_advance = src["hmtx"][probe][0]
    scale = ADVANCE / float(source_advance)

    pen = TTGlyphPen(None)
    for index, char in enumerate(name):
        glyph_name = cmap.get(ord(char))
        if not glyph_name:
            raise SystemExit(
                f"source font has no glyph for {char!r} (needed by {name})"
            )
        recording = DecomposingRecordingPen(src_glyphs)
        src_glyphs[glyph_name].draw(recording)
        transform = Transform(scale, 0, 0, scale, index * ADVANCE, 0)
        recording.replay(TransformPen(pen, transform))

    glyphs = {
        ".notdef": _notdef_glyph(),
        "uni55B5": pen.glyph(),
    }
    advance = ADVANCE * max(1, len(name))

    builder = FontBuilder(UPEM, isTTF=True)
    builder.setupGlyphOrder([".notdef", "uni55B5"])
    builder.setupCharacterMap({CODEPOINT: "uni55B5"})
    builder.setupGlyf(glyphs)
    builder.setupHorizontalMetrics({
        ".notdef": (ADVANCE, 0),
        "uni55B5": (advance, 0),
    })
    builder.setupHorizontalHeader(ascent=800, descent=-200, lineGap=0)
    builder.setupNameTable({
        "familyName": f"{FONT_PREFIX}{name}",
        "styleName": "Regular",
        "uniqueFontIdentifier": f"{FONT_PREFIX}{name} Regular {VERSION}",
        "fullName": f"{FONT_PREFIX}{name}",
        "psName": f"{FONT_PREFIX}{name}",
        "version": f"Version {VERSION}",
        "copyright": (
            "Generated by MeowLang (DeepSeek V4.1 Flash). "
            "Glyph outlines derived from Droid Sans Mono, "
            "Copyright (C) 2008 The Android Open Source Project, "
            "licensed under Apache License 2.0."
        ),
        "description": (
            "MeowLang symbol font. Maps U+55B5 to a readable glyph "
            f"for the built-in symbol {name}."
        ),
    })
    builder.setupOS2(
        sTypoAscender=800,
        sTypoDescender=-200,
        sTypoLineGap=0,
        usWinAscent=800,
        usWinDescent=200,
        sxHeight=500,
        sCapHeight=700,
        usWeightClass=400,
        usWidthClass=5,
        fsType=0,
    )
    builder.setupPost(isFixedPitch=0)
    builder.setupDummyDSIG()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    builder.save(out_path)

    # Re-open and validate the essential properties.  This catches malformed
    # name/cmap tables before the font is installed.
    check = TTFont(out_path)
    family = check["name"].getDebugName(1)
    full = check["name"].getDebugName(4)
    ps = check["name"].getDebugName(6)
    cmap_out = check.getBestCmap() or {}
    if family != f"{FONT_PREFIX}{name}" or full != f"{FONT_PREFIX}{name}":
        raise SystemExit(f"bad name table in {out_path}: {family!r}/{full!r}")
    if ps != f"{FONT_PREFIX}{name}":
        raise SystemExit(f"bad PostScript name in {out_path}: {ps!r}")
    if cmap_out.get(CODEPOINT) != "uni55B5":
        raise SystemExit(f"bad cmap in {out_path}: {cmap_out!r}")
    if check["hmtx"]["uni55B5"][0] != advance:
        raise SystemExit(f"bad advance in {out_path}")


def check_fonts(font_dir: Path, names: list[str]) -> int:
    missing = []
    bad = []
    for name in names:
        path = font_dir / f"{FONT_PREFIX}{name}.ttf"
        if not path.is_file():
            missing.append(path.name)
            continue
        try:
            font = TTFont(path)
            if font["name"].getDebugName(1) != f"{FONT_PREFIX}{name}":
                bad.append(f"{path.name}: family mismatch")
            if (font.getBestCmap() or {}).get(CODEPOINT) != "uni55B5":
                bad.append(f"{path.name}: cmap mismatch")
        except Exception as exc:  # noqa: BLE001 - report and continue
            bad.append(f"{path.name}: {exc}")
    if missing:
        print("missing fonts:", file=sys.stderr)
        for item in missing:
            print(f"  {item}", file=sys.stderr)
    if bad:
        print("invalid fonts:", file=sys.stderr)
        for item in bad:
            print(f"  {item}", file=sys.stderr)
    if missing or bad:
        return 1
    print(f"OK: {len(names)} Meow-* fonts in {font_dir}")
    return 0


def main() -> int:
    root = repo_root()
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--meow-c", default=str(root / "native" / "meow.c"))
    parser.add_argument("--source", default=None)
    parser.add_argument("--output-dir", default=str(root / "fonts"))
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    names = builtin_names(Path(args.meow_c))
    if not names:
        print("no Meow-* names found", file=sys.stderr)
        return 1

    font_dir = Path(args.output_dir)
    if args.check:
        return check_fonts(font_dir, names)

    source = find_source_font(args.source)
    print(f"source font: {source}")
    print(f"output dir:  {font_dir}")
    print(f"built-ins:   {len(names)}")

    for index, name in enumerate(names, 1):
        out = font_dir / f"{FONT_PREFIX}{name}.ttf"
        if out.exists() and not args.force:
            # Regeneration is deterministic; skip existing files to make
            # repeated runs fast, but still validate them.
            check = TTFont(out)
            if check["name"].getDebugName(1) == f"{FONT_PREFIX}{name}":
                print(f"[{index:02d}/{len(names)}] keep {out.name}")
                continue
        build_font(name, out, source)
        print(f"[{index:02d}/{len(names)}] write {out.name}")

    return check_fonts(font_dir, names)


if __name__ == "__main__":
    raise SystemExit(main())
