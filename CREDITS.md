# Credits and Development Attribution

## Developer of this fork

**This fork was developed by DeepSeek V4.1 Flash.**

Scope of the work:

- Native C11 interpreter (`native/meow.c`, `native/Makefile`,
  `native/CMakeLists.txt`)
  - Self-contained ZIP central-directory reader with zlib decompression
  - DOCX XML parsing with expat (`word/document.xml` and
    `word/_rels/document.xml.rels`)
  - Embedded PNG/JPEG decoding with libpng and libjpeg, rendered as ANSI
    half-block characters
  - Unicode case conversion for ASCII, Latin-1, Greek, and Cyrillic
  - Rich-text formatting, colors, highlights, font sizes, and images
  - Control flow, lists, strings, and a Python-style source frontend
- Lightweight Java implementation
  - Replaced Apache POI with `ZipFile` + StAX
  - Removed POI and Log4j dependencies
  - Lazy JLine loading
  - Reduced the JAR from about 22.6 MB to about 1.5 MB
- Multilingual documentation (English, Traditional Chinese, Simplified
  Chinese, Japanese, Korean)
- Symbol fonts
  - 48 `Meow-*` fonts that map U+55B5 to readable built-in names
  - `tools/gen_meow_fonts.py` generator
  - `fonts/install-fonts.sh` user and Android module installer
  - KernelSU/Magisk overlay module and release packaging
- Performance measurements and output comparison tests

## Original project

- Project: MeowLang
- Original author: [Glavo](https://github.com/Glavo)
- Original repository: <https://github.com/Glavo/MeowLang>
- Original language design, Java implementation, and `.docx` examples: Glavo

This fork preserves the original GPLv3 license and all copyright notices.

## Sponsors and tools

- [PLCT Lab](https://plctlab.org): sponsored the original project
- [OpenCode](https://opencode.ai) Free API: provided the free API access used
  to develop and test this fork
- Droid Sans Mono (The Android Open Source Project, Apache-2.0): source
  outlines for the generated `Meow-*` symbol fonts
- JetBrains IntelliJ IDEA: development tool for the original project

## License

- All source code: GPLv3
- Other files (`.docx`, `.md`, `.png`, and so on): CC BY-SA 4.0
- `fonts/Meow-*.ttf`: Apache-2.0 (derived from Droid Sans Mono); see
  `fonts/NOTICE` and `fonts/LICENSE-APACHE-2.0.txt`

---

# 致謝與開發者標注（繁體中文）

## 本 fork 的開發者

**本 fork 由 DeepSeek V4.1 Flash 開發。**

開發範圍包含：

- 原生 C11 解譯器（`native/meow.c`、`native/Makefile`、
  `native/CMakeLists.txt`）
  - 自製 ZIP 中央目錄讀取與 zlib 解壓縮
  - 以 expat 解析 `word/document.xml` 與 `word/_rels/document.xml.rels`
  - 以 libpng、libjpeg 解碼內嵌圖片，並輸出 ANSI 半塊字元圖
  - Unicode 大小寫轉換（ASCII、Latin-1、希臘字母、西里爾字母）
  - 富文本格式、顏色、螢光標記、字級與圖片輸出
  - 控制流程、列表、字串，以及 Python 風格前端
- Java 輕量版重構
  - 以 `ZipFile` + StAX 取代 Apache POI
  - 移除 POI、Log4j 依賴
  - JLine 延遲載入
  - JAR 大小由約 22.6 MB 降至約 1.5 MB
- 多語言文件（英文、繁體中文、簡體中文、日文、韓文）
- 符號字型
  - 48 個 `Meow-*` 字型，將 U+55B5 映射為可讀的內建名稱
  - `tools/gen_meow_fonts.py` 產生器
  - `fonts/install-fonts.sh` 使用者與 Android 模組安裝器
  - KernelSU／Magisk 覆蓋模組與發行版包裝
- 效能測試與輸出比對驗證

## 原始專案

- 專案名稱：MeowLang
- 原作者：[Glavo](https://github.com/Glavo)
- 原始倉庫：<https://github.com/Glavo/MeowLang>
- 原始語言設計、Java 實作與 `.docx` 範例：Glavo

本 fork 保留原始專案的 GPLv3 授權與所有著作權聲明。

## 贊助與工具

- [PLCT 實驗室](https://plctlab.org)：贊助原始專案
- [OpenCode](https://opencode.ai) Free API：提供免費 API 存取，協助本 fork
  的開發與測試
- Droid Sans Mono（The Android Open Source Project，Apache-2.0）：提供
  `Meow-*` 符號字型的來源輪廓
- JetBrains IntelliJ IDEA：原始專案開發工具

## 授權

- 所有原始碼：GPLv3
- 其他檔案（`.docx`、`.md`、`.png` 等）：CC BY-SA 4.0
- `fonts/Meow-*.ttf`：Apache-2.0（衍生自 Droid Sans Mono），詳見
  `fonts/NOTICE` 與 `fonts/LICENSE-APACHE-2.0.txt`
