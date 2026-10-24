# Examples

These `.meow` files are compiled into MeowLang `.docx` programs with
`tools/meowc.py` and executed by `native/meow`.

```bash
# Compile everything
for f in examples/*.meow; do
    python3 tools/meowc.py "$f" -o "${f%.meow}.docx"
done

# Run
./native/meow examples/hello.docx
./native/meow examples/test1.docx
./native/meow examples/game2048.docx
```

## hello.meow

The smallest input/output example:

```text
What is your name? Meow
Hello, Meow !
2 + 3 = 5
```

## test1.meow

A combined syntax test: arithmetic, integer division, comparisons,
`if`/`elif`/`else`, `while`, `for`, functions, recursion, early return, lists,
slices, strings, `in`, `and`/`or`/`not`, and conditional expressions.

## game2048.meow

A complete text-mode 2048 game. The board is a list of 16 integers. Every move
compresses, merges, and scores each row, then adds a random 2 or 4.

Controls:

```text
w = up
a = left
s = down
d = right
q = quit
```

Build and run:

```bash
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```

Example output:

```text
+------+------+------+------+
|      . |      4 |      . |      . |
+------+------+------+------+
|      . |      . |      . |      . |
+------+------+------+------+
|      . |      . |      . |      . |
+------+------+------+------+
|      . |      . |      . |      2 |
+------+------+------+------+
Score: 0
Move (w/a/s/d, q):
```

---

# 範例（繁體中文摘要）

這些 `.meow` 檔案使用 `tools/meowc.py` 編譯成 MeowLang 的 `.docx`，再由
`native/meow` 執行。

```bash
for f in examples/*.meow; do
    python3 tools/meowc.py "$f" -o "${f%.meow}.docx"
done

./native/meow examples/hello.docx
./native/meow examples/test1.docx
./native/meow examples/game2048.docx
```

## hello.meow

最簡單的輸入／輸出範例：

```text
What is your name? Meow
Hello, Meow !
2 + 3 = 5
```

## test1.meow

綜合語法測試：算術、整數除法、比較、`if`／`elif`／`else`、`while`、`for`、
函式、遞迴、提前返回、列表、切片、字串、`in`、`and`／`or`／`not` 與三元
運算式。

## game2048.meow

完整文字版 2048。棋盤是 16 個整數的列表；每次移動會壓縮、合併、計分，
再隨機產生一個 2 或 4。

操作：`w` = 上、`a` = 左、`s` = 下、`d` = 右、`q` = 離開。

```bash
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```
