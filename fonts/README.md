# MeowLang Symbol Fonts

These fonts make MeowLang DOCX files readable without changing their
executable content.  Every generated `Meow-<name>.ttf` font maps the character
`喵` (U+55B5) to a glyph that spells the built-in name.  For example:

- a run with `Meow-return` is displayed as **return**
- a run with `Meow-println` is displayed as **println**
- a run with `Meow-substring` is displayed as **substring**

The underlying DOCX text remains `喵`, and the native/Java interpreter still
identifies the symbol from the run's font name, so execution is unchanged.

本目錄提供 MeowLang 的符號字型。每個 `Meow-<name>.ttf` 會把 `喵`（U+55B5）
畫成對應的內建名稱，讓 DOCX 在安裝字型後可直接閱讀；DOCX 內的實際文字仍是
`喵`，執行語意不變。

## Contents

- `Meow-*.ttf` — 48 generated symbol fonts, one for each built-in symbol.
- `install-fonts.sh` — user, prefix, Android module, uninstall, and check modes.
- `NOTICE` — attribution for the source outlines.
- `LICENSE-APACHE-2.0.txt` — Apache License 2.0 for the derived outlines.

## Install for the current user

Linux, macOS, and Termux:

```sh
./install-fonts.sh --user
```

Termux installs into `~/.fonts`; other Unix-like systems install into
`${XDG_DATA_HOME:-~/.local/share}/fonts`.  If `fc-cache` is available, the
script refreshes the font cache automatically.  Existing `Meow-*.ttf` files
are backed up to a timestamped `.meow-fonts-backup-*` directory first.

## Install for Android system-wide (Word and other apps)

Android apps such as Word do not read a user font directory.  On a rooted
device with KernelSU or Magisk, install the overlay module:

```sh
su -c 'sh install-fonts.sh --android-module --yes'
```

Then reboot.  The module:

- adds `/system/fonts/Meow-*.ttf`
- appends the matching `<family>` entries to `/system/etc/fonts.xml`
- backs up an existing `meow_fonts` module outside `/data/adb/modules`

This is a system-level change.  Review the script before running it, and keep
a way to remove the module (KernelSU/Magisk manager or
`sh install-fonts.sh --uninstall --yes`) in case an app misbehaves.

Android's FontManagerService (`cmd font update`) is not used because it requires
fs-verity-signed font files; the overlay module does not.

## Windows

Double-click each `.ttf` and choose **Install**, or copy the files to
`%LOCALAPPDATA%\Microsoft\Windows\Fonts`.  A PowerShell helper is not included
yet because the Windows native build has not been tested on this device.

## Release integration

The release tarball produced by `tools/package-release.sh` contains this
directory and calls `install-fonts.sh --user` from its top-level `install.sh`.
The native Makefile also exposes:

```sh
make -C native install-fonts
```

## Regenerate

The generated fonts are committed so end users do not need Python or
fontTools.  To regenerate them from `native/meow.c`:

```sh
python3 -m pip install fonttools
python3 tools/gen_meow_fonts.py --force
python3 tools/gen_meow_fonts.py --check
```

The generator uses the ASCII outlines of `/system/fonts/DroidSansMono.ttf`
(or another monospace font passed with `--source`).  Each output font has a
1000-unit em square and a compact 500-unit advance per character.

## License

The font files are derived from Droid Sans Mono, Copyright (C) 2008 The Android
Open Source Project, licensed under the Apache License, Version 2.0.  See
`NOTICE` and `LICENSE-APACHE-2.0.txt`.  The rest of MeowLang is licensed under
GPL-3.0-or-later; see the repository `README.md` and `CREDITS.md`.

The generator and these fonts were developed by **DeepSeek V4.1 Flash** for
this fork.
