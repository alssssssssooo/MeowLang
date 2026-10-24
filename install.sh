#!/bin/sh
# MeowLang release installer.
#
# Installs the native `meow` binary for the current user.  This script is
# included at the root of release tarballs produced by tools/package-release.sh.
#
# The legacy Meow-* symbol fonts are optional: the Meow cipher made them
# unnecessary for reading compiled DOCX files, so fonts are not installed
# by default.  Pass --fonts to install them for the current user, or
# --android-fonts (root) for the system-wide module.
#
# Usage:
#   ./install.sh
#   ./install.sh --prefix "$HOME/.local"
#   ./install.sh --fonts
#   sudo ./install.sh --android-fonts

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
INSTALL_FONTS=0
ANDROID_FONTS=0
BIN_PREFIX=

say() {
    printf '%s\n' "$*"
}

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

usage() {
    cat <<EOF
Usage: $0 [OPTIONS]

Options:
  --prefix DIR       Install the binary under DIR/bin (default: \$PREFIX on
                     Termux, otherwise \$HOME/.local)
  --fonts            Also install the legacy Meow-* symbol fonts (not
                     needed since the Meow cipher; kept for old DOCX)
  --android-fonts    Also install the Android system font module (root only)
  --help             Show this help

The user font installer writes to ~/.fonts on Termux and to
\${XDG_DATA_HOME:-~/.local/share}/fonts elsewhere.  Existing same-name fonts
are backed up first.
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix)
            [ $# -ge 2 ] || die "--prefix needs a directory"
            BIN_PREFIX=$2
            shift
            ;;
        --fonts) INSTALL_FONTS=1 ;;
        --android-fonts) ANDROID_FONTS=1 ;;
        --help|-h) usage; exit 0 ;;
        *) die "unknown option: $1 (use --help)" ;;
    esac
    shift
done

if [ -z "$BIN_PREFIX" ]; then
    case "${PREFIX:-}" in
        *com.termux*) BIN_PREFIX=$PREFIX ;;
        *) BIN_PREFIX=${HOME}/.local ;;
    esac
fi

BINDIR=$BIN_PREFIX/bin

if [ -x "$ROOT/bin/meow" ]; then
    BINARY=$ROOT/bin/meow
elif [ -x "$ROOT/native/meow" ]; then
    BINARY=$ROOT/native/meow
else
    die "meow binary not found; build it with: make -C native"
fi

mkdir -p "$BINDIR"

if [ -f "$BINDIR/meow" ]; then
    backup="$BINDIR/.meow-backup-$(date +%Y%m%d-%H%M%S)"
    mkdir -p "$backup"
    cp -p "$BINDIR/meow" "$backup/meow"
    say "backup: $backup/meow"
fi

cp -f "$BINARY" "$BINDIR/meow"
chmod 755 "$BINDIR/meow"
say "installed binary: $BINDIR/meow"

if [ "$INSTALL_FONTS" -eq 1 ]; then
    [ -x "$ROOT/fonts/install-fonts.sh" ] || die "fonts/install-fonts.sh not found"
    sh "$ROOT/fonts/install-fonts.sh" --user
fi

if [ "$ANDROID_FONTS" -eq 1 ]; then
    [ "$(id -u)" -eq 0 ] || die "--android-fonts requires root; run with sudo or su"
    sh "$ROOT/fonts/install-fonts.sh" --android-module --yes
    say "reboot the device to activate the system font module"
fi

case ":$PATH:" in
    *":$BINDIR:"*) ;;
    *) say "note: add $BINDIR to PATH to run 'meow' directly" ;;
esac

say "done"
