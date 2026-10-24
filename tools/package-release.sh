#!/bin/sh
# Build a self-contained MeowLang release tarball.
#
# The tarball contains the native binary, the optional legacy Meow-* symbol
# fonts, the Python-style frontend (with the Meow cipher specification),
# examples, README images, and a top-level install.sh.  The installer installs
# fonts only when --fonts is passed.
#
# Usage:
#   tools/package-release.sh
#   tools/package-release.sh --no-build
#   MEOW_VERSION=1.2.0 tools/package-release.sh

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
VERSION=${MEOW_VERSION:-1.2.0}
BUILD=1

say() {
    printf '%s\n' "$*"
}

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

while [ $# -gt 0 ]; do
    case "$1" in
        --no-build) BUILD=0 ;;
        --help|-h)
            sed -n '2,12p' "$0"
            exit 0
            ;;
        *) die "unknown option: $1" ;;
    esac
    shift
done

if [ "$BUILD" -eq 1 ]; then
    say "building native binary"
    make -C "$ROOT/native"
fi

[ -x "$ROOT/native/meow" ] || die "native/meow not found; build it first"
[ -d "$ROOT/fonts" ] || die "fonts/ not found"
[ -f "$ROOT/fonts/install-fonts.sh" ] || die "fonts/install-fonts.sh not found"
[ -d "$ROOT/images" ] || die "images/ not found"

STAGE="$ROOT/dist/meow-$VERSION"
case "$STAGE" in
    "$ROOT/dist/"*) rm -rf "$STAGE" ;;
    *) die "refusing to remove unexpected path: $STAGE" ;;
esac

mkdir -p "$STAGE/bin" "$STAGE/native" "$STAGE/tools" "$STAGE/examples"

cp -f "$ROOT/native/meow" "$STAGE/bin/meow"
chmod 755 "$STAGE/bin/meow"

cp -a "$ROOT/fonts" "$STAGE/fonts"
cp -f "$ROOT/install.sh" "$STAGE/install.sh"
chmod 755 "$STAGE/install.sh"

cp -f "$ROOT/native/meow.c" "$ROOT/native/Makefile" \
      "$ROOT/native/CMakeLists.txt" "$ROOT/native/README.md" "$STAGE/native/"
cp -f "$ROOT/tools/meowc.py" "$ROOT/tools/meow_cipher.py" \
      "$ROOT/tools/gen_meow_fonts.py" "$ROOT/tools/README.md" "$STAGE/tools/"
cp -a "$ROOT/examples/." "$STAGE/examples/"
cp -a "$ROOT/images" "$STAGE/images"

for doc in README.md README.zh-TW.md README.ja.md README.ko.md README.zh-CN.md \
           CHANGELOG.md CREDITS.md; do
    [ -f "$ROOT/$doc" ] && cp -f "$ROOT/$doc" "$STAGE/$doc"
done

# Include the original demo when it is available; it is optional because
# .docx files are ignored by git.
if [ -f "$ROOT/demo/HelloWorld.docx" ]; then
    mkdir -p "$STAGE/demo"
    cp -f "$ROOT/demo/HelloWorld.docx" "$STAGE/demo/"
fi

TARBALL="$ROOT/dist/meow-$VERSION-linux-android.tar.gz"
tar -czf "$TARBALL" -C "$ROOT/dist" "meow-$VERSION"

if command -v sha256sum >/dev/null 2>&1; then
    (cd "$ROOT/dist" && sha256sum "$(basename "$TARBALL")" > "$(basename "$TARBALL").sha256")
fi

say "release: $TARBALL"
say "size:    $(du -h "$TARBALL" | cut -f1)"
[ -f "$TARBALL.sha256" ] && say "sha256:  $(cut -d' ' -f1 "$TARBALL.sha256")"
