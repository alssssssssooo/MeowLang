# Meow 語言

[English](README.md) | **繁體中文** | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | [한국어](README.ko.md)

[![CI](https://github.com/alssssssssooo/MeowLang/actions/workflows/ci.yml/badge.svg)](https://github.com/alssssssssooo/MeowLang/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/v/release/alssssssssooo/MeowLang?display_name=tag&sort=semver)](https://github.com/alssssssssooo/MeowLang/releases/latest)

<small>這是一門為了讓貓科動物——例如老虎、貓、貓娘——也能當程式設計師而研發的程式語言。</small>

> **🐱 純屬娛樂，不可當作正規使用**
>
> MeowLang 是一門為了好玩而設計的 esoteric 玩具語言。**它不是正式語言，
> 絕不可用於嚴肅、學術或任何實際用途。** 不保證穩定性，也不提供支援承諾。
> 如果貓咪坐在鍵盤上而程式依然能跑，那就是一次成功的程式碼審查。

> **Fork 說明**
>
> 本專案是 [Glavo/MeowLang](https://github.com/Glavo/MeowLang) 的 fork。
> **本 fork（包含原生 C 實作、輕量 DOCX 解析器、效能改寫、喵語密碼與多語言
> 文件）由 DeepSeek V4.1 Flash 開發。**
> 原始語言設計與原始 Java 實作歸 Glavo 所有。

MeowLang 是一門 esoteric 程式語言，**它的原始碼就是 Microsoft Word 文件**。
一支程式是一整片套用格式的 `喵` 符號，而**格式就是語法**：顏色、粗體、底線、
醒目提示與斜體，決定每個 `喵` 的意義。編譯後的 `.docx` 在 Word 裡看起來像
被加密的貓，而原生解譯器可以直接執行它。

![](./images/lambda.png)

## 特性

### 富文本程式設計

Meow 語言使用 Microsoft Word 作為編輯器，支援富文本程式設計。開發者可以在
程式碼中嵌入格式化文字、圖片與其他內容，並將它們輸出到終端機。

![](./images/richtext.png)
![](./images/richtext-output.png)

### 一個關鍵字、零個名稱

原始 Meow 只保留一個符號：貓的叫聲。沒有使用者自訂識別字、括號或引號。
格式就是身分——兩段 `喵` 只有在所有格式屬性完全相同時，才是同一個符號。

開發者不需要為變數命名，而是給每個 `喵` 不同的顏色、醒目提示、粗體、底線
等等。每一種不同的組合，就是一個不同的識別字。

![](./images/meow.png)

### 用任何語言寫「喵」

解譯器不強制你只能使用英文的「Meow」。它支援數十種語言中的貓叫聲：

`meow` · `miaou` · `miao` · `nyan` · `にゃん` · `야옹` · `喵` · `喵呜` ·
`мяу` · `мјау` · `miyav` · `miau` · `miauw` · `μιάου` · `mjau` · `mjäu` ·
`mjav` · `mjal` · `nyávog` · `म्याऊं` · `meong` · `mjá` · `meo meo` ·
`მიაუ` · `myau` · `ngiyavuma` · `ማው` · `maw`

**詞彙本身從來不是身分——格式才是。** 上面所有詞彙都是可互換的同義詞：只要
格式相同，兩段符號就是同一個符號，不論它們各自來自哪一種語言。因此你可以用
自己思考時最順的語言來手寫原始 Meow，甚至在同一支程式裡混用多種語言——這讓
手寫**更容易**，而不是更麻煩，因為「喵」本來就沒有唯一正確的拼法。大小寫也
不影響（`Μιάου`、`Мяу` 都能辨識）。如果你使用下方的 Python 風格前端，更是
一個喵都不必打。

![](./images/i18n.png)

### 喵語密碼——不需要字型

內建函式與巨集以雙音節的喵語密碼詞加上格式來書寫。密碼詞只是視覺分類；
每個內建符號的真正身分，是
**8 種保留色 × 粗體 × 雙底線 × 黃色醒目提示**
的組合（共 64 種，實際使用 53 種）：

| 密碼詞 | 分類 |
| --- | --- |
| `喵喵` | 輸出入 |
| `喵嗚` | 數學與比較 |
| `嗚喵` | 列表與字串 |
| `嗚嗚` | 控制流程與語法 |

由於密碼只使用標準文字與格式，編譯後的 DOCX 在任何 Word 安裝中都能**直接
正確顯示**，完全不需要安裝任何東西。完整的代碼對照請見
[喵語密碼對照表](#喵語密碼對照表)；共用的機器可讀規格位於
`tools/meow_cipher.py`。

## 快速開始

```bash
# Termux / Android：
pkg install clang make zlib expat libpng libjpeg-turbo
# 其他系統：C11 編譯器、make，以及 zlib/expat/libpng/libjpeg
make -C native

# 編譯程式並執行
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```

2048 操作：

```text
w = 上    a = 左    s = 下    d = 右    q = 離開
```

## 撰寫程式：`.meow` 前端

直接手寫原始 Meow，意味著要在 Word 裡逐一調整每個符號的顏色、粗體、底線與
斜體。選用的 Python 風格前端（`tools/meowc.py`）會替你完成這些事：你撰寫
普通的 Python 風格 `.meow` 原始碼，它會編譯成同樣的帶格式 `.docx`。執行時期
仍然是原生 C 執行檔；Java 與 Python 都不參與執行。

### 變數與賦值

```python
x = 10
x += 1
items = [1, 2, 3]
items[0] = 9
```

### 算術與比較

```python
print(1 + 2, 7 // 2, 7 % 3, 2 ** 10)
print(x > 5 and x < 100)
print(3 in items)
```

支援的運算子：`+` `-` `*` `/` `//` `%` `**`，比較運算子
`==` `!=` `<` `<=` `>` `>=`，以及 `and` `or` `not`。

`/` 與 `//` 都執行整數除法，並向負無限大取整。
這門語言沒有浮點數。

### 條件判斷

```python
if x > 10:
    print("big")
elif x == 10:
    print("ten")
else:
    print("small")
```

### 迴圈

```python
i = 0
while i < 5:
    print(i)
    i += 1

for n in range(1, 6):
    print(n)

for ch in "meow":
    print(ch)
```

`for` 可以迭代列表，也可以迭代文字的 Unicode 碼位。

### 函式與提前回傳

```python
def fact(n):
    if n <= 1:
        return 1
    return n * fact(n - 1)

print(fact(6))
```

函式支援遞迴與提前 `return`。

### 列表與字串

```python
a = [1, 2, 3]
a.append(4)
print(a[0], a[-1], a[1:3], len(a))
print(str(123) + "!", int("42") + 1)
```

### 條件運算式

```python
print("yes" if 3 > 2 else "no")
```

### 名稱對應

為了讓執行時期保持精簡，前端會把部分 Python 名稱對應到原生內建函式：

| Python 語法 | 執行時期符號 | 說明 |
| --- | --- | --- |
| `print(...)` | `println` | 以空格分隔參數，最後換行 |
| `input(prompt)` | `readline` | 從標準輸入讀取一行 |
| `int(text)` | `parseInt` | 十進位整數字串轉整數 |
| `a.append(x)` | `push` | 將元素附加到列表尾端（原地修改） |
| `a.pop()` | `pop` | 移除並回傳最後一個元素 |
| `x in a` | `contains(a, x)` | 列表成員或子字串測試 |

### 尚未支援

- `break`、`continue` 與迴圈的 `else`
- 類別、例外與 `with`
- 關鍵字引數、預設引數與可變長度引數
- `not in`、`is`，以及 `a < b < c` 這種連鎖比較
- 浮點數與 f-string
- `extend`、`insert`、`remove` 這類原地列表方法
- `.upper()`、`.lower()` 這類字串方法

需要這些功能時，請使用原始 Meow 語法，或擴充 `native/meow.c` 與
`tools/meowc.py`。

### 前端命令列

```text
python3 tools/meowc.py SOURCE [-o OUTPUT.docx] [--dump-ast] [--dump-runtime]
```

- 未指定 `-o` 時，輸出檔名沿用來源檔名，副檔名改為 `.docx`。
- `--dump-ast` 會印出前端 AST；`--dump-runtime` 會印出產生的執行時期
  運算式。兩者都供除錯使用。

## 執行程式

```bash
./native/meow ./demo/HelloWorld.docx
```

可以一次傳入多個 `.docx` 檔案；每個檔案都會取得全新的 context。

```bash
./native/meow ./demo/HelloWorld.docx ./play/card.docx
```

### 環境變數

| 變數 | 實作 | 作用 |
| --- | --- | --- |
| `MEOW_DEBUG=1` | 原生版 | 印出解析後的運算式與求值追蹤 |
| `MEOW_PLAIN=1` | Java 版 | 一律使用輕量純文字主控台（不使用 JLine） |
| `MEOW_JLINE=1` | Java 版 | 即使 stdin 不是終端機也強制使用 JLine |

## 手寫原始 Meow

原始 Meow 直接在 Word 裡撰寫。你不需要任何字型，只需要標準的格式設定面板。

### 格式模型

- 一個段落就是一個運算式列表。不是喵語詞彙的 run（空白、引號外的普通文字）
  會被略過。
- **斜體 `喵` 成對 = 一個群組**（括號）。開啟與關閉的 run 必須具有相同格式；
  編譯器會依巢狀深度調整群組的灰階，但任何一致的斜體顏色都可以。
- **單底線 `喵` 成對 = 文字字面量。** 編譯後的格式使用顏色 `0x7030A0`；
  兩個分隔符之間的內容會以富文本形式保留。
- **顏色唯一的普通 `喵` = 變數。** 顏色不可與下方的保留色票衝突。
- **帶有保留格式的密碼詞 = 內建符號。**

保留色票（絕不可用於自己的變數）：
`0x000000`（字面量內容）、`0x7030A0`（字面量分隔符），以及八種密碼顏色
`0xC00000`、`0xC07000`、`0x909000`、`0x008000`、`0x008080`、`0x0070C0`、
`0x8060C0`、`0xC000C0`。變數可以使用其他任何顏色。

可接受的喵語詞彙（全部可互換，大小寫不敏感）：*用任何語言寫「喵」*一節列出的
28 個詞，再加上四個密碼詞 `喵喵`、`喵嗚`、`嗚喵`、`嗚嗚`。

### 內建巨集

| 巨集 | 形態 | 意義 |
| --- | --- | --- |
| `let` | `(x value)` | 將 `value` 綁定到變數 `x` |
| `lambda` | `(params body...)` | 函式值；`params` 是單一識別字或識別字群組 |
| `if` | `(cond then [else])` | 條件判斷 |
| `while` | `(cond body...)` | 當 `cond` 為真時重複執行 |
| `for` | `(var iterable body...)` | 迭代列表或文字 |
| `do` | `(body...)` | 依序求值 |
| `return` | 從所屬函式提前回傳 | |
| `and` / `or` | 短路布林運算 | |
| `integer` | 字級編碼的 `喵` | 舊版整數字面量（數值儲存在字型大小中）；建議改用 `parseInt("...")` |

### 內建函式

- **輸出入**：`print`、`println`、`readline`、`clear`
- **算術**：`add`、`sub`、`mul`、`div`、`mod`、`pow`、`neg`、`abs`、
  `min`、`max`、`sum`
- **比較**：`eq`、`ne`、`lt`、`le`、`gt`、`ge`、`not`
- **列表**：`list`、`cons`、`head`、`tail`、`len`、`get`、`set`、`push`、
  `pop`、`concat`、`slice`、`range`、`contains`、`join`
- **字串**：`str`、`parseInt`、`textLen`、`charAt`、`substring`
- **其他**：`random`、`srand`

### 喵語密碼對照表

每個內建符號都是它的密碼詞、顏色、粗體、雙底線與黃色醒目提示的組合。
`native/meow.c` 中的原生表與編譯器（`tools/meow_cipher.py`）由同一份規格
產生，因此兩者永遠不會不同步。

<details open>
<summary><code>喵喵</code> — 輸出入</summary>

| 內建函式 | 顏色 | 粗體 | 底線 | 醒目提示 |
| --- | --- | --- | --- | --- |
| `print` | 深紅 `0xC00000` | — | — | — |
| `readline` | 橙色 `0xC07000` | — | — | — |
| `println` | 暗金 `0x909000` | — | — | — |
| `clear` | 綠色 `0x008000` | — | — | — |

</details>

<details>
<summary><code>喵嗚</code> — 數學與比較</summary>

| 內建函式 | 顏色 | 粗體 | 底線 | 醒目提示 |
| --- | --- | --- | --- | --- |
| `add` | 藍綠 `0x008080` | — | — | — |
| `sub` | 藍色 `0x0070C0` | — | — | — |
| `mul` | 紫色 `0x8060C0` | — | — | — |
| `div` | 洋紅 `0xC000C0` | — | — | — |
| `mod` | 深紅 `0xC00000` | **是** | — | — |
| `pow` | 橙色 `0xC07000` | **是** | — | — |
| `neg` | 暗金 `0x909000` | **是** | — | — |
| `abs` | 綠色 `0x008000` | **是** | — | — |
| `min` | 藍綠 `0x008080` | **是** | — | — |
| `max` | 藍色 `0x0070C0` | **是** | — | — |
| `sum` | 紫色 `0x8060C0` | **是** | — | — |
| `eq` | 洋紅 `0xC000C0` | **是** | — | — |
| `ne` | 深紅 `0xC00000` | — | 雙底線 | — |
| `lt` | 橙色 `0xC07000` | — | 雙底線 | — |
| `le` | 暗金 `0x909000` | — | 雙底線 | — |
| `gt` | 綠色 `0x008000` | — | 雙底線 | — |
| `ge` | 藍綠 `0x008080` | — | 雙底線 | — |
| `not` | 藍色 `0x0070C0` | — | 雙底線 | — |
| `parseInt` | 紫色 `0x8060C0` | — | 雙底線 | — |
| `random` | 洋紅 `0xC000C0` | — | 雙底線 | — |
| `srand` | 深紅 `0xC00000` | **是** | 雙底線 | — |

</details>

<details>
<summary><code>嗚喵</code> — 列表與字串</summary>

| 內建函式 | 顏色 | 粗體 | 底線 | 醒目提示 |
| --- | --- | --- | --- | --- |
| `contains` | 橙色 `0xC07000` | **是** | 雙底線 | — |
| `list` | 暗金 `0x909000` | **是** | 雙底線 | — |
| `cons` | 綠色 `0x008000` | **是** | 雙底線 | — |
| `head` | 藍綠 `0x008080` | **是** | 雙底線 | — |
| `tail` | 藍色 `0x0070C0` | **是** | 雙底線 | — |
| `len` | 紫色 `0x8060C0` | **是** | 雙底線 | — |
| `get` | 洋紅 `0xC000C0` | **是** | 雙底線 | — |
| `set` | 深紅 `0xC00000` | — | — | 黃色 |
| `push` | 橙色 `0xC07000` | — | — | 黃色 |
| `pop` | 暗金 `0x909000` | — | — | 黃色 |
| `concat` | 綠色 `0x008000` | — | — | 黃色 |
| `slice` | 藍綠 `0x008080` | — | — | 黃色 |
| `range` | 藍色 `0x0070C0` | — | — | 黃色 |
| `str` | 紫色 `0x8060C0` | — | — | 黃色 |
| `textLen` | 洋紅 `0xC000C0` | — | — | 黃色 |
| `charAt` | 深紅 `0xC00000` | **是** | — | 黃色 |
| `substring` | 橙色 `0xC07000` | **是** | — | 黃色 |
| `join` | 暗金 `0x909000` | **是** | — | 黃色 |

</details>

<details>
<summary><code>嗚嗚</code> — 控制流程與語法</summary>

| 內建函式 | 顏色 | 粗體 | 底線 | 醒目提示 |
| --- | --- | --- | --- | --- |
| `let` | 綠色 `0x008000` | **是** | — | 黃色 |
| `lambda` | 藍綠 `0x008080` | **是** | — | 黃色 |
| `integer` | 藍色 `0x0070C0` | **是** | — | 黃色 |
| `if` | 紫色 `0x8060C0` | **是** | — | 黃色 |
| `while` | 洋紅 `0xC000C0` | **是** | — | 黃色 |
| `for` | 深紅 `0xC00000` | — | 雙底線 | 黃色 |
| `do` | 橙色 `0xC07000` | — | 雙底線 | 黃色 |
| `return` | 暗金 `0x909000` | — | 雙底線 | 黃色 |
| `and` | 綠色 `0x008000` | — | 雙底線 | 黃色 |
| `or` | 藍綠 `0x008080` | — | 雙底線 | 黃色 |

</details>

## 範例

- `examples/hello.meow` — 輸入與輸出。
- `examples/test1.meow` — 算術、控制流程、函式、列表與字串。
- `examples/game2048.meow` — 完整可玩的文字模式 2048。

```bash
python3 tools/meowc.py examples/test1.meow
./native/meow examples/test1.docx
```

## 兩種實作

本 fork 提供同一套語言的兩種實作：

| 實作 | 產物 | 執行環境 | 狀態 |
| --- | --- | --- | --- |
| **原生版（建議）** | `native/meow` | 無需執行環境（原生 ELF 執行檔） | 主要開發版本 |
| Java 版（舊版） | `meow.jar` | JVM 21 以上 | 保留作為參考 |

兩者讀取相同的 `.docx` 檔案。原生版產生的文字輸出與 Java 版一致，只有少數
ANSI 重設序列的寫法不同。

## 本 fork 的新增內容

| 項目 | 上游 | 本 fork（Java） | 本 fork（原生） |
| --- | --- | --- | --- |
| DOCX 解析 | Apache POI + XMLBeans | `ZipFile` + StAX | 自製 ZIP 讀取器 + expat |
| 執行環境 | JVM | JVM | 不需要 JVM |
| 產物大小 | 約 22.6 MB JAR | 約 1.5 MB JAR | 約 56 KB 執行檔 |
| `demo/HelloWorld.docx` | 約 1.7 秒 | 約 0.42 秒 | 約 0.024 秒 |
| `play/card.docx` | 約 1.5 秒 | 約 0.37 秒 | 約 0.03 秒 |

原生版自行實作 ZIP 中央目錄讀取器，使用 zlib 解壓縮項目，以 expat 解析
`word/document.xml` 及其關聯檔，並以 libpng 與 libjpeg 解碼內嵌的 PNG/JPEG
圖片。圖片會像 Java 版一樣，以 ANSI 半塊字元藝術輸出。

### 效能

在 Termux / Android aarch64 上測量（原生版 50 次、Python 3.13.13 參考
30 次）：

| 工作負載 | 每次執行時間 |
| --- | --- |
| 原生 `meow --version`（僅啟動） | 0.020 秒 |
| 原生完整 `demo/HelloWorld.docx`（ZIP + XML + 解譯器 + PNG + ANSI） | 0.024 秒 |
| Python 3.13 裸啟動（`python3 -c pass`） | 0.050 秒 |
| Python 3.13 僅解壓 `word/document.xml` | 0.084 秒 |

原生執行檔跑完整個 demo 的時間，比 Python 光是啟動還快，也比 Python 僅取出
文件 XML 快三倍以上。Python 那兩列並沒有解析、解譯或渲染任何內容；它們只是
下限，不是等效實作。

## 建置原生執行檔

需求：

- C11 編譯器（`clang` 或 `gcc`）
- `make`
- zlib、expat、libpng、libjpeg 開發檔案

在 Termux 上：

```bash
pkg install clang make zlib expat libpng libjpeg-turbo
```

然後：

```bash
make -C native
```

或使用 CMake：

```bash
cmake -S native -B build-native
cmake --build build-native
```

執行檔會輸出到 `native/meow`。

## 符號字型（選用，舊版）

本 fork 的早期版本會把每個內建符號編碼成自訂的 `Meow-<name>.ttf` 字型名稱，
這意味著必須安裝 48 個產生的字型，才能在 Word 裡讀取編譯後的 DOCX。
**喵語密碼已取代這套做法：新編譯的 DOCX 完全不需要字型。**

`fonts/` 目錄是為了相容於舊的字型編碼 DOCX（以及偏好那種外觀的人）而保留。
安裝字型永遠不影響執行：DOCX 的文字仍然是喵符號，而解譯器會忽略字型。

為目前使用者安裝（Linux、macOS、Termux）：

```bash
./fonts/install-fonts.sh --user
```

Android 系統層級安裝需要 root，且必須重新開機：

```bash
su -c 'sh fonts/install-fonts.sh --android-module --yes'
```

發行版安裝腳本預設不安裝字型；需要時請加上 `--fonts`。
詳情（包含備份與解除安裝行為）請見 `fonts/README.md`。

## 建置並執行 Java 版（選用）

```bash
./gradlew meow
java -jar meow.jar ./demo/HelloWorld.docx
```

或使用自執行腳本：

```bash
./meow.sh ./demo/HelloWorld.docx
```

## 專案結構

```text
native/
├── meow.c                 原生解譯器（ZIP + XML + ANSI + 圖片）
├── Makefile               pkg-config / 備援建置
└── CMakeLists.txt         可攜的 CMake 建置
fonts/                     舊版、選用的符號字型
├── Meow-*.ttf             U+55B5 -> 可讀的內建名稱（舊編碼）
├── install-fonts.sh       使用者 / Android 模組安裝腳本
├── README.md              安裝與重新產生說明
└── NOTICE                 Droid Sans Mono 授權資訊
tools/
├── meowc.py               Python 風格 .meow -> .docx 編譯器
├── meow_cipher.py         喵語密碼規格（唯一真實來源）
├── gen_meow_fonts.py      重新產生舊版 Meow-* 符號字型
└── package-release.sh     產生發行版壓縮檔
examples/
├── hello.meow
├── test1.meow
└── game2048.meow
src/main/java/org/glavo/meow/
├── Main.java              Java 進入點
├── Meow.java              符號（帶格式屬性的「Meow」）
├── MeowDocxReader.java    輕量 DOCX 讀取器（ZipFile + StAX）
├── MeowRun.java           XWPFRun 的輕量替代品
├── MeowParser.java        剖析器
├── MeowConsole.java       主控台抽象層（plain / JLine）
├── JlineConsole.java      延遲載入的 JLine 轉接層
└── value/                 執行時期值與內建函式
```

## 授權

本專案的所有原始碼以
[GPLv3](https://www.gnu.org/licenses/gpl-3.0.html) 釋出。

其他所有檔案（`.docx`、`.md`、`.png` 等）以
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) 釋出。

`fonts/` 中產生的符號字型是例外：其字形輪廓衍生自 Droid Sans Mono，以
[Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) 釋出。請見
`fonts/NOTICE` 與 `fonts/LICENSE-APACHE-2.0.txt`。

## 致謝

- 原始專案與語言設計：[Glavo](https://github.com/Glavo)
- **本 fork（原生實作、輕量剖析器、效能改進、喵語密碼與多語言文件）：
  DeepSeek V4.1 Flash**
- [OpenCode](https://opencode.ai) Free API：提供開發與測試本 fork 所需的
  免費 API 存取
- 由 [PLCT Lab](https://plctlab.org) 贊助（上游專案的贊助者；本 fork 與
  PLCT Lab 無隸屬或贊助關係）

<img alt="OpenCode logo" src="./images/opencode-badge.svg" width="258" height="66">

<img alt="PLCT Lab logo" src="./images/plct-badge.svg" width="200" height="200">

<img alt="IntelliJ IDEA logo" src="./images/intellij-badge.svg" width="300" height="93">

原始專案使用 JetBrains IntelliJ IDEA 開發。
