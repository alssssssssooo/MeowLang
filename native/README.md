# Meow Native (C11)

`native/meow.c` is the native implementation of the Meow language, **developed
by DeepSeek V4.1 Flash**. It reads `.docx` files directly and executes them
without a JVM, Python, or any interpreter runtime.

## Build

Requirements:

- A C11 compiler (`clang` or `gcc`)
- `make` or CMake 3.16+
- Development headers and libraries for zlib, expat, libpng, and libjpeg

### Termux

```bash
pkg install clang make cmake pkg-config zlib expat libpng libjpeg-turbo
make -C native
```

### Linux

Debian / Ubuntu:

```bash
sudo apt install build-essential pkg-config zlib1g-dev libexpat1-dev \
    libpng-dev libjpeg-dev
make -C native
```

Fedora:

```bash
sudo dnf install gcc make pkgconf-pkg-config zlib-devel expat-devel \
    libpng-devel libjpeg-turbo-devel
make -C native
```

### CMake

```bash
cmake -S native -B build-native
cmake --build build-native
```

CMake uses `find_package` for zlib, expat, libpng, and libjpeg. Windows can use
the same `CMakeLists.txt`, but the build has not been tested on Windows yet:

```powershell
cmake -S native -B build-native -G "MinGW Makefiles"
cmake --build build-native
```

Windows requires MinGW-w64 or MSVC plus the four libraries above. vcpkg or
MSYS2 is recommended for dependencies. No pre-verified macOS build is provided.

## Run

```bash
./native/meow demo/HelloWorld.docx
./native/meow play/card.docx
```

Multiple files:

```bash
./native/meow a.docx b.docx
```

Debug mode:

```bash
MEOW_DEBUG=1 ./native/meow demo/HelloWorld.docx
```

Version:

```bash
./native/meow --version
```

## Symbol fonts

The native interpreter identifies built-ins by the `Meow-<name>` font of a
`喵` run. `fonts/` contains 48 generated symbol fonts that display those runs
as readable names (`return`, `if`, `substring`, ...) without changing the
executable text.

Install them for the current user:

```bash
make -C native install-fonts
# or
sh fonts/install-fonts.sh --user
```

For Android system-wide use (Word and other apps), install the KernelSU/Magisk
overlay and reboot:

```bash
su -c 'sh fonts/install-fonts.sh --android-module --yes'
```

`make -C native install` installs the `meow` binary; `cmake --install` also
copies `fonts/` to `share/meow`. The CMake `install-fonts` target runs the user
installer. See `fonts/README.md` for backup, uninstall, and regeneration
details. The font files are Apache-2.0; the rest of the project is GPLv3.

## Python-style frontend

Writing raw Meow requires setting colors, italics, underlines, and fonts for
every symbol. To write programs instead, use `tools/meowc.py`:

```bash
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```

The frontend compiles Python-style `.meow` source into a styled `.docx`; the
runtime is still this native binary. See
[`tools/README.md`](../tools/README.md) for syntax and limitations.

## Implementation

1. **ZIP**: scans the EOCD, central directory, and local headers directly.
   Method 8 entries are inflated with raw zlib; method 0 entries are copied.
2. **XML**: uses expat in namespace mode to parse `word/document.xml` and
   `word/_rels/document.xml.rels`.
3. **Symbols**: text, italic (brackets), underline, color, highlight, bold,
   font, and font size together define a `Meow` symbol. All fields must match
   for equality.
4. **Parser**: italics are brackets and single underlines delimit string
   literals. An unterminated literal extends to the end of the paragraph, as in
   the Java version.
5. **Evaluation**: tree walking. `let`, `lambda`, `integer`, `if`, `while`,
   `for`, `do`, `return`, `and`, and `or` are macros that receive unevaluated
   expressions; everything else is a function.
6. **Output**: ANSI bold, underline, 24-bit foreground, and background colors.
   PNG/JPEG images are decoded with libpng/libjpeg and rendered with `▀`
   half-block characters.

### Control flow

- `if` takes a condition, a then-branch, and an else-branch. Branches use `do`
  to group statements.
- `while` re-evaluates its condition before each iteration.
- `for` iterates over lists or text (by Unicode code point).
- `return` uses `setjmp`/`longjmp` for early function exit.
- `and` and `or` short-circuit and return the last evaluated value.
- `/` and `//` are integer division rounded toward negative infinity; `%` has
  the same sign as the divisor.

## Built-in functions

### Output and input

- `print(...)`: newline-separated arguments (original MeowLang semantics)
- `println(...)`: space-separated arguments and a trailing newline
- `readline([prompt])`: reads one line

### Arithmetic and comparison

- `add`, `sub`, `mul`, `div`, `mod`, `pow`, `neg`, `abs`
- `min`, `max`, `sum`
- `eq`, `ne`, `lt`, `le`, `gt`, `ge`
- `not`, `contains`

`add` supports integers, text, and lists. Text addition concatenates strings;
list addition creates a new list.

### Lists

- `list(...)`, `cons`, `head`, `tail`
- `len`, `get`, `set`, `push`, `pop`
- `concat`, `slice`, `range`

`range` accepts one to three arguments and supports negative steps, for example
`range(5, 0, -1)`. Negative indexes are supported. `set`, `push`, and `pop`
modify lists in place.

### Text

- `str`, `parseInt`, `textLen`, `charAt`, `substring`, `join`

Text indexes and lengths count Unicode code points, not UTF-8 bytes.

### Other

- `random([limit])`, `random(low, high)`, `srand(seed)`, `clear()`

`random(limit)` returns `0` to `limit - 1`; `random(low, high)` returns an
integer including both endpoints.

## Supported features

- Paragraphs, runs, text, tabs, and line breaks
- Bold, italic, underline, color, highlight, font, and font size
- DrawingML images (`a:blip`) and VML images (`v:imagedata` inside
  `w:drawing`)
- Multilingual Meow words (`meow`, `喵`, `мяу`, `Μιάου`, and so on)
- `MEOW_DEBUG=1`

## Known limitations

- `readline` reads line by line from standard input. It has no JLine-style line
  editing, history, or completion.
- Images are not resized to the terminal width yet (same as the Java version).
- ZIP64 is not supported.
- `w:tab`, `w:br`, and `w:cr` are handled in a simplified way.
- ANSI reset sequences differ slightly from the Java version, but the visual
  result is the same.
- Integers use `long`; Windows (LLP64) and Linux/Android (LP64) have different
  widths.
- `break`, `continue`, floating-point numbers, classes, and exceptions are not
  supported yet.
- Windows build configuration is provided but has not been tested on a Windows
  device.

## License

GPLv3. The original Java implementation and language design belong to Glavo;
the native implementation was developed by DeepSeek V4.1 Flash.

---

# Meow 原生版（繁體中文摘要）

`native/meow.c` 是 Meow 語言的原生實作，**由 DeepSeek V4.1 Flash 開發**。
它直接讀取 `.docx` 檔並執行，不需要 JVM、Python 或任何直譯器 runtime。

## 建置

需求：C11 編譯器、`make` 或 CMake 3.16 以上，以及 zlib、expat、libpng、
libjpeg 的開發檔案。

Termux：

```bash
pkg install clang make cmake pkg-config zlib expat libpng libjpeg-turbo
make -C native
```

CMake：

```bash
cmake -S native -B build-native
cmake --build build-native
```

Windows 已提供 CMake 設定，但尚未實測。

## 執行

```bash
./native/meow demo/HelloWorld.docx
./native/meow play/card.docx
MEOW_DEBUG=1 ./native/meow demo/HelloWorld.docx
./native/meow --version
```

## 符號字型

原生版以 `喵` 文字所屬的 `Meow-<name>` 字型辨識內建符號。`fonts/` 提供 48 個
自動產生的字型，會把 `喵` 顯示成 `return`、`if`、`substring` 等可讀名稱，
但 DOCX 內的文字仍是 `喵`，執行語意不變。

安裝到目前使用者：

```bash
make -C native install-fonts
# 或
sh fonts/install-fonts.sh --user
```

Android 系統層（Word 等 App）需要 root 與重新開機：

```bash
su -c 'sh fonts/install-fonts.sh --android-module --yes'
```

`make -C native install` 會安裝 `meow` 執行檔；`cmake --install` 也會把
`fonts/` 複製到 `share/meow`。備份、解除安裝與重新產生字型的說明請見
`fonts/README.md`。字型檔採用 Apache-2.0，其餘專案為 GPLv3。

## Python 風格前端

```bash
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```

前端只負責把 `.meow` 編譯成喵樣式 `.docx`，執行期仍是原生 C 執行檔。詳細
語法與限制請見 [`tools/README.md`](../tools/README.md)。

## 內建函式

- 輸出與輸入：`print`、`println`、`readline`
- 算術與比較：`add`、`sub`、`mul`、`div`、`mod`、`pow`、`neg`、`abs`、
  `min`、`max`、`sum`、`eq`、`ne`、`lt`、`le`、`gt`、`ge`、`not`、
  `contains`
- 列表：`list`、`cons`、`head`、`tail`、`len`、`get`、`set`、`push`、
  `pop`、`concat`、`slice`、`range`
- 文字：`str`、`parseInt`、`textLen`、`charAt`、`substring`、`join`
- 其他：`random`、`srand`、`clear`

`range` 支援 1 到 3 個參數與負步長；索引支援負數；`set`、`push`、`pop`
會就地修改列表。文字索引與長度以 Unicode 碼點計算。

## 已知限制

- `readline` 沒有 JLine 的行編輯、歷史紀錄與補齊功能。
- 圖片尚未依終端機寬度縮放。
- 尚未支援 ZIP64。
- 尚未支援 `break`、`continue`、浮點數、類別與例外處理。
- 整數使用 `long`；Windows 與 Linux／Android 的位元寬度不同。
- Windows 建置設定已提供，但尚未在 Windows 裝置上實測。

## 授權

GPLv3。原始 Java 實作與語言設計由 Glavo 所有；原生實作由
DeepSeek V4.1 Flash 開發。
