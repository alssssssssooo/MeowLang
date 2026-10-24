#!/bin/sh
# MeowLang symbol font installer.
#
# Usage:
#   ./install-fonts.sh --user
#   ./install-fonts.sh --prefix /path/to/fonts
#   sudo ./install-fonts.sh --android-module --yes
#   sudo ./install-fonts.sh --uninstall --yes
#   ./install-fonts.sh --check
#
# The default mode is --user.  System-wide Android installation is opt-in and
# requires root plus --yes.  It creates a KernelSU/Magisk module and requires a
# reboot before Android's font service can see the new families.

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
MODULE_ID=meow_fonts
MODULE_ROOT=/data/adb/modules/$MODULE_ID
FONT_VERSION=1.0

say() {
    printf '%s\n' "$*"
}

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

font_files() {
    # Print one path per line; the filenames contain no spaces.
    find "$SCRIPT_DIR" -maxdepth 1 -type f -name 'Meow-*.ttf' | sort
}

require_fonts() {
    first=$(font_files | head -n 1)
    [ -n "$first" ] || die "no Meow-*.ttf files found next to this script"
}

user_font_dir() {
    case "${PREFIX:-}" in
        *com.termux*)
            printf '%s\n' "$HOME/.fonts"
            ;;
        *)
            printf '%s\n' "${XDG_DATA_HOME:-$HOME/.local/share}/fonts"
            ;;
    esac
}

backup_existing() {
    dir=$1
    existing=$(find "$dir" -maxdepth 1 -type f -name 'Meow-*.ttf' 2>/dev/null | head -n 1 || true)
    [ -n "$existing" ] || return 0
    backup="$dir/.meow-fonts-backup-$(date +%Y%m%d-%H%M%S)"
    mkdir -p "$backup"
    find "$dir" -maxdepth 1 -type f -name 'Meow-*.ttf' -exec cp -p {} "$backup"/ \;
    say "backup: $backup"
}

install_to_dir() {
    dir=$1
    mkdir -p "$dir"
    backup_existing "$dir"
    count=0
    for font in $(font_files); do
        cp -f "$font" "$dir"/
        chmod 644 "$dir/$(basename "$font")"
        count=$((count + 1))
    done
    say "installed $count fonts to $dir"
}

install_user() {
    require_fonts
    dir=$(user_font_dir)
    install_to_dir "$dir"
    if command -v fc-cache >/dev/null 2>&1; then
        fc-cache -f "$dir" >/dev/null 2>&1 || true
        say "font cache refreshed"
    else
        say "fc-cache not found; restart the application to pick up the fonts"
    fi
    say "Meow-* fonts are available to fontconfig-aware applications"
    say "For Word/Android system-wide use, see --android-module."
}

check_user() {
    dir=$(user_font_dir)
    say "user font dir: $dir"
    found=0
    for font in $(font_files); do
        base=$(basename "$font")
        if [ -f "$dir/$base" ]; then
            found=$((found + 1))
        fi
    done
    say "installed: $found / $(font_files | wc -l)"
    if [ "$found" -eq 0 ]; then
        return 1
    fi
    return 0
}

require_root_yes() {
    [ "$(id -u)" -eq 0 ] || die "this mode requires root; run it with su or sudo"
    [ "${YES:-0}" -eq 1 ] || die "system changes require --yes"
}

backup_module() {
    if [ -d "$MODULE_ROOT" ]; then
        backup="/data/adb/meow-fonts-backup-$(date +%Y%m%d-%H%M%S)"
        cp -a "$MODULE_ROOT" "$backup"
        say "backup: $backup"
    fi
}

write_module_prop() {
    cat > "$MODULE_ROOT/module.prop" <<EOF
id=$MODULE_ID
name=MeowLang Symbol Fonts
version=$FONT_VERSION
versionCode=1
author=DeepSeek V4.1 Flash
description=Meow-* symbol fonts for MeowLang DOCX files. Reboot after installation.
EOF
}

write_module_scripts() {
    cat > "$MODULE_ROOT/customize.sh" <<'EOF'
#!/system/bin/sh
# MeowLang fonts are provided as a system overlay.  No extra setup is needed.
EOF
    cat > "$MODULE_ROOT/uninstall.sh" <<'EOF'
#!/system/bin/sh
# Removing the module removes the /system/fonts and /system/etc/fonts.xml
# overlay on the next reboot.
EOF
    chmod 755 "$MODULE_ROOT/customize.sh" "$MODULE_ROOT/uninstall.sh"
}

patch_fonts_xml() {
    original=/system/etc/fonts.xml
    [ -f "$original" ] || die "$original not found"
    tmp_base=${TMPDIR:-/data/local/tmp}
    [ -d "$tmp_base" ] || tmp_base=/data/local/tmp
    families=$(mktemp "$tmp_base/meow-families.XXXXXX")
    trap 'rm -f "$families"' EXIT HUP INT TERM
    for font in $(font_files); do
        base=$(basename "$font" .ttf)
        cat >> "$families" <<EOF
  <family name="$base">
    <font weight="400" style="normal">$base.ttf</font>
  </family>
EOF
    done
    awk -v insert="$families" '
        BEGIN { done = 0 }
        /<\/familyset>/ && !done {
            while ((getline line < insert) > 0) print line
            close(insert)
            done = 1
        }
        { print }
    ' "$original" > "$MODULE_ROOT/system/etc/fonts.xml"
}

install_android_module() {
    require_root_yes
    require_fonts
    [ -d /data/adb/modules ] || die "/data/adb/modules not found; is KernelSU/Magisk installed?"
    backup_module
    rm -rf "$MODULE_ROOT"
    mkdir -p "$MODULE_ROOT/system/fonts" "$MODULE_ROOT/system/etc"
    for font in $(font_files); do
        cp -f "$font" "$MODULE_ROOT/system/fonts"/
        chmod 644 "$MODULE_ROOT/system/fonts/$(basename "$font")"
    done
    patch_fonts_xml
    write_module_prop
    write_module_scripts
    chmod 755 "$MODULE_ROOT" "$MODULE_ROOT/system" "$MODULE_ROOT/system/fonts" "$MODULE_ROOT/system/etc"
    chmod 644 "$MODULE_ROOT/module.prop" "$MODULE_ROOT/system/etc/fonts.xml"
    say "installed Android font module: $MODULE_ROOT"
    say "reboot is required before Word and other apps can use the fonts"
}

uninstall_user() {
    dir=$(user_font_dir)
    if [ -d "$dir" ]; then
        find "$dir" -maxdepth 1 -type f -name 'Meow-*.ttf' -exec rm -f {} \;
        say "removed user fonts from $dir"
    else
        say "user font dir does not exist: $dir"
    fi
    if command -v fc-cache >/dev/null 2>&1; then
        fc-cache -f "$dir" >/dev/null 2>&1 || true
    fi
}

uninstall_android_module() {
    require_root_yes
    if [ -d "$MODULE_ROOT" ]; then
        backup_module
        rm -rf "$MODULE_ROOT"
        say "removed Android font module; reboot to finish"
    else
        say "Android font module is not installed"
    fi
}

usage() {
    cat <<EOF
Usage: $0 [MODE]

Modes:
  --user                 Install into the current user's font directory (default)
  --prefix DIR           Install into an explicit directory
  --android-module      Install a KernelSU/Magisk system overlay (root + --yes)
  --uninstall           Remove user fonts; with --yes and root, remove the module
  --check               Show the user installation status
  --help                Show this help

System installation:
  sudo $0 --android-module --yes
  # or on Android:
  su -c 'sh $0 --android-module --yes'
  # then reboot

The Android module overlays /system/fonts/Meow-*.ttf and appends the matching
families to /system/etc/fonts.xml.  The original module directory is backed up
outside /data/adb/modules before it is replaced.
EOF
}

MODE=user
YES=0
PREFIX_DIR=

while [ $# -gt 0 ]; do
    case "$1" in
        --user) MODE=user ;;
        --prefix)
            [ $# -ge 2 ] || die "--prefix needs a directory"
            MODE=prefix
            PREFIX_DIR=$2
            shift
            ;;
        --android-module) MODE=android ;;
        --uninstall) MODE=uninstall ;;
        --check) MODE=check ;;
        --yes) YES=1 ;;
        --help|-h) usage; exit 0 ;;
        *) die "unknown option: $1 (use --help)" ;;
    esac
    shift
done

case "$MODE" in
    user) install_user ;;
    prefix)
        [ -n "$PREFIX_DIR" ] || die "--prefix needs a directory"
        require_fonts
        install_to_dir "$PREFIX_DIR"
        ;;
    android) install_android_module ;;
    uninstall)
        uninstall_user
        if [ "$(id -u)" -eq 0 ] && [ "$YES" -eq 1 ]; then
            uninstall_android_module
        fi
        ;;
    check) check_user ;;
esac
