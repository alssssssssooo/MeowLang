# Meow 言語

[English](README.md) | [繁體中文](README.zh-TW.md) | [简体中文](README.zh-CN.md) | **日本語** | [한국어](README.ko.md)

[![CI](https://github.com/alssssssssooo/MeowLang/actions/workflows/ci.yml/badge.svg)](https://github.com/alssssssssooo/MeowLang/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/v/release/alssssssssooo/MeowLang?display_name=tag&sort=semver)](https://github.com/alssssssssooo/MeowLang/releases/latest)

<small>この言語は、トラや猫、猫娘といったネコ科の動物もプログラマーになれるように開発されたプログラミング言語です。</small>

> **🐱 純粋に娯楽目的です。正式な用途には使用できません**
>
> MeowLang は遊びのために作られた esoteric なおもちゃの言語です。
> **本番言語ではなく、真剣な用途、学術用途、その他あらゆる実務には
> 決して使用しないでください。** 安定性は保証されず、サポートも約束されません。
> 猫がキーボードの上に乗ってもプログラムが動いたなら、それは成功した
> コードレビューです。

> **Fork について**
>
> このリポジトリは [Glavo/MeowLang](https://github.com/Glavo/MeowLang) の fork です。
> **本 fork（ネイティブ C 実装、軽量 DOCX パーサー、性能改善、Meow 暗号、
> 多言語ドキュメント）は DeepSeek V4.1 Flash が開発しました。**
> 元の言語設計と元の Java 実装は Glavo に帰属します。

MeowLang は esoteric プログラミング言語で、**ソースコードは Microsoft Word
文書そのもの**です。プログラムは書式を施した `喵` 記号の壁であり、
**書式が文法**です。色、太字、下線、ハイライト、斜体が、それぞれの `喵` の
意味を決めます。コンパイルされた `.docx` は Word では暗号化された猫のように
見えますが、ネイティブインタープリターはそのまま実行できます。

![](./images/lambda.png)

## 特徴

### リッチテキストプログラミング

Meow 言語は Microsoft Word をエディタとして使用します。リッチテキスト
プログラミングに対応しており、書式付きテキストや画像などをソースコードに
埋め込み、そのままターミナルへ出力できます。

![](./images/richtext.png)
![](./images/richtext-output.png)

### キーワードは 1 つ、名前は 0 個

生の Meow には記号が 1 つしかありません。猫の鳴き声です。ユーザー定義の
識別子、括弧、引用符はありません。書式がアイデンティティであり、2 つの `喵`
はすべての書式属性が一致したときにのみ同じ記号になります。

変数に名前を付ける代わりに、開発者は各 `喵` に異なる色、ハイライト、太字、
下線などを設定します。組み合わせが異なれば、それは別の識別子です。

![](./images/meow.png)

### どの言語でも「ニャー」と書ける

インタープリターは英語の「Meow」を強制しません。数十の言語の猫の鳴き声を
受け付けます：

`meow` · `miaou` · `miao` · `nyan` · `にゃん` · `야옹` · `喵` · `喵呜` ·
`мяу` · `мјау` · `miyav` · `miau` · `miauw` · `μιάου` · `mjau` · `mjäu` ·
`mjav` · `mjal` · `nyávog` · `म्याऊं` · `meong` · `mjá` · `meo meo` ·
`მიაუ` · `myau` · `ngiyavuma` · `ማው` · `maw`

**単語そのものがアイデンティティになることは決してありません。書式が
アイデンティティです。** 上記の単語はすべて互換性のある同義語です。書式が
一致すれば、それぞれがどの言語の単語であっても同じ記号として扱われます。
したがって、自分が考えるときに最も自然な言語で生の Meow を書くことも、
1 つのプログラム内で複数の言語を混ぜることもできます。これは手書きを
**難しくするのではなく、簡単にします**。「ニャー」に唯一正しい綴りはないから
です。大文字と小文字も区別されません（`Μιάου` や `Мяу` も認識されます）。
下記の Python 風フロントエンドを使えば、ニャーを 1 つも打つ必要はありません。

![](./images/i18n.png)

### Meow 暗号——フォント不要

組み込み関数とマクロは、2 音節の Meow 暗号語と書式で書きます。暗号語は
視覚的な分類にすぎず、各組み込み記号の本当のアイデンティティは
**8 色の予約色 × 太字 × 二重下線 × 黄色ハイライト**
の組み合わせです（全 64 通り、うち 53 通りを使用）：

| 暗号語 | 分類 |
| --- | --- |
| `喵喵` | 入出力 |
| `喵嗚` | 算術と比較 |
| `嗚喵` | リストと文字列 |
| `嗚嗚` | 制御フローと構文 |

暗号は標準のテキストと書式だけを使うため、コンパイルされた DOCX は
**どの Word でもそのまま正しく表示**されます。何もインストールする必要は
ありません。完全なコード対応表は
[Meow 暗号の対応表](#meow-暗号の対応表) を、共有の機械可読仕様は
`tools/meow_cipher.py` を参照してください。

## クイックスタート

```bash
# Termux / Android：
pkg install clang make zlib expat libpng libjpeg-turbo
# その他のシステム：C11 コンパイラ、make、zlib/expat/libpng/libjpeg
make -C native

# プログラムをコンパイルして実行
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```

2048 の操作：

```text
w = 上    a = 左    s = 下    d = 右    q = 終了
```

## プログラムを書く：`.meow` フロントエンド

生の Meow を手書きするには、Word で記号ごとに色、太字、下線、斜体を調整する
必要があります。任意の Python 風フロントエンド（`tools/meowc.py`）がその作業を
代行します。普通の Python 風 `.meow` ソースを書けば、同じ書式付き `.docx` に
コンパイルされます。実行時は引き続きネイティブ C 実行ファイルであり、Java も
Python も関与しません。

### 変数と代入

```python
x = 10
x += 1
items = [1, 2, 3]
items[0] = 9
```

### 算術と比較

```python
print(1 + 2, 7 // 2, 7 % 3, 2 ** 10)
print(x > 5 and x < 100)
print(3 in items)
```

対応演算子：`+` `-` `*` `/` `//` `%` `**`、比較演算子
`==` `!=` `<` `<=` `>` `>=`、および `and` `or` `not`。

`/` と `//` はどちらも整数除算で、負の無限大方向に丸めます。
この言語に浮動小数点数はありません。

### 条件分岐

```python
if x > 10:
    print("big")
elif x == 10:
    print("ten")
else:
    print("small")
```

### ループ

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

`for` はリストと、テキストの Unicode コードポイントを反復できます。

### 関数と早期リターン

```python
def fact(n):
    if n <= 1:
        return 1
    return n * fact(n - 1)

print(fact(6))
```

関数は再帰と早期 `return` をサポートします。

### リストと文字列

```python
a = [1, 2, 3]
a.append(4)
print(a[0], a[-1], a[1:3], len(a))
print(str(123) + "!", int("42") + 1)
```

### 条件式

```python
print("yes" if 3 > 2 else "no")
```

### 名前の対応

実行時を小さく保つため、フロントエンドは一部の Python 名をネイティブの
組み込み関数に対応付けます：

| Python 構文 | 実行時シンボル | 説明 |
| --- | --- | --- |
| `print(...)` | `println` | 引数を空白区切りで出力し、最後に改行 |
| `input(prompt)` | `readline` | 標準入力から 1 行読み込む |
| `int(text)` | `parseInt` | 10 進整数文字列を整数に変換 |
| `a.append(x)` | `push` | リスト末尾に要素を追加（インプレース） |
| `a.pop()` | `pop` | 最後の要素を削除して返す |
| `x in a` | `contains(a, x)` | リストの所属または部分文字列の判定 |

### 未対応

- `break`、`continue`、ループの `else`
- クラス、例外、`with`
- キーワード引数、デフォルト引数、可変長引数
- `not in`、`is`、`a < b < c` のような連鎖比較
- 浮動小数点数と f-string
- `extend`、`insert`、`remove` などのインプレースなリストメソッド
- `.upper()`、`.lower()` などの文字列メソッド

これらが必要な場合は、生の Meow 構文を使うか、`native/meow.c` と
`tools/meowc.py` を拡張してください。

### フロントエンドのコマンドライン

```text
python3 tools/meowc.py SOURCE [-o OUTPUT.docx] [--dump-ast] [--dump-runtime]
```

- `-o` を省略すると、出力ファイル名はソース名の拡張子を `.docx` にしたものに
  なります。
- `--dump-ast` はフロントエンドの AST を、`--dump-runtime` は生成された
  実行時式を出力します。どちらもデバッグ用です。

## プログラムの実行

```bash
./native/meow ./demo/HelloWorld.docx
```

複数の `.docx` ファイルを一度に渡せます。各ファイルは新しい context を
取得します。

```bash
./native/meow ./demo/HelloWorld.docx ./play/card.docx
```

### 環境変数

| 変数 | 実装 | 効果 |
| --- | --- | --- |
| `MEOW_DEBUG=1` | ネイティブ版 | 解析後の式と評価トレースを出力 |
| `MEOW_PLAIN=1` | Java 版 | 常に軽量なプレーンコンソールを使用（JLine 不使用） |
| `MEOW_JLINE=1` | Java 版 | stdin が端末でなくても JLine を強制使用 |

## 生の Meow を書く

生の Meow は Word で直接書きます。フォントは一切不要で、標準の書式設定
パネルだけを使います。

### 書式モデル

- 1 つの段落が 1 つの式リストです。Meow 語彙ではない run（空白、引用符の外の
  普通のテキスト）はスキップされます。
- **斜体の `喵` のペア = グループ**（括弧）。開きと閉じの run は同じ書式で
  なければなりません。コンパイラはネストの深さに応じてグループのグレースケール
  を変えますが、一貫した斜体色であれば何でも構いません。
- **一重下線の `喵` のペア = テキストリテラル。** コンパイル後の書式は色
  `0x7030A0` を使用し、区切り文字の間の内容はリッチテキストとして保持されます。
- **色が一意な通常の `喵` = 変数。** 色は下記の予約パレットと衝突しては
  いけません。
- **予約書式を持つ暗号語 = 組み込み記号。**

予約パレット（自分の変数には絶対に使わないでください）：
`0x000000`（リテラル内容）、`0x7030A0`（リテラル区切り）、および 8 つの暗号色
`0xC00000`、`0xC07000`、`0x909000`、`0x008000`、`0x008080`、`0x0070C0`、
`0x8060C0`、`0xC000C0`。変数にはそれ以外の任意の色を使えます。

受け付ける Meow 語彙（すべて互換、大文字小文字を区別しない）：
*どの言語でも「ニャー」と書ける* の節に挙げた 28 語に、4 つの暗号語
`喵喵`、`喵嗚`、`嗚喵`、`嗚嗚` を加えたものです。

### 組み込みマクロ

| マクロ | 形式 | 意味 |
| --- | --- | --- |
| `let` | `(x value)` | `value` を変数 `x` に束縛 |
| `lambda` | `(params body...)` | 関数値。`params` は 1 つの識別子か識別子のグループ |
| `if` | `(cond then [else])` | 条件分岐 |
| `while` | `(cond body...)` | `cond` が真の間、繰り返す |
| `for` | `(var iterable body...)` | リストまたはテキストを反復 |
| `do` | `(body...)` | 順に評価 |
| `return` | 所属する関数から早期リターン | |
| `and` / `or` | 短絡ブール演算 | |
| `integer` | フォントサイズで符号化された `喵` | 旧式の整数リテラル（値はフォントサイズに格納）。`parseInt("...")` の使用を推奨 |

### 組み込み関数

- **入出力**：`print`、`println`、`readline`、`clear`
- **算術**：`add`、`sub`、`mul`、`div`、`mod`、`pow`、`neg`、`abs`、
  `min`、`max`、`sum`
- **比較**：`eq`、`ne`、`lt`、`le`、`gt`、`ge`、`not`
- **リスト**：`list`、`cons`、`head`、`tail`、`len`、`get`、`set`、`push`、
  `pop`、`concat`、`slice`、`range`、`contains`、`join`
- **文字列**：`str`、`parseInt`、`textLen`、`charAt`、`substring`
- **その他**：`random`、`srand`

### Meow 暗号の対応表

各組み込み記号は、暗号語、色、太字、二重下線、黄色ハイライトの組み合わせ
です。`native/meow.c` のネイティブテーブルはコンパイラ
（`tools/meow_cipher.py`）と同じ仕様から生成されるため、両者がずれることは
ありません。

<details open>
<summary><code>喵喵</code> — 入出力</summary>

| 組み込み関数 | 色 | 太字 | 下線 | ハイライト |
| --- | --- | --- | --- | --- |
| `print` | 濃赤 `0xC00000` | — | — | — |
| `readline` | オレンジ `0xC07000` | — | — | — |
| `println` | 暗い金色 `0x909000` | — | — | — |
| `clear` | 緑 `0x008000` | — | — | — |

</details>

<details>
<summary><code>喵嗚</code> — 算術と比較</summary>

| 組み込み関数 | 色 | 太字 | 下線 | ハイライト |
| --- | --- | --- | --- | --- |
| `add` | 青緑 `0x008080` | — | — | — |
| `sub` | 青 `0x0070C0` | — | — | — |
| `mul` | 紫 `0x8060C0` | — | — | — |
| `div` | マゼンタ `0xC000C0` | — | — | — |
| `mod` | 濃赤 `0xC00000` | **あり** | — | — |
| `pow` | オレンジ `0xC07000` | **あり** | — | — |
| `neg` | 暗い金色 `0x909000` | **あり** | — | — |
| `abs` | 緑 `0x008000` | **あり** | — | — |
| `min` | 青緑 `0x008080` | **あり** | — | — |
| `max` | 青 `0x0070C0` | **あり** | — | — |
| `sum` | 紫 `0x8060C0` | **あり** | — | — |
| `eq` | マゼンタ `0xC000C0` | **あり** | — | — |
| `ne` | 濃赤 `0xC00000` | — | 二重 | — |
| `lt` | オレンジ `0xC07000` | — | 二重 | — |
| `le` | 暗い金色 `0x909000` | — | 二重 | — |
| `gt` | 緑 `0x008000` | — | 二重 | — |
| `ge` | 青緑 `0x008080` | — | 二重 | — |
| `not` | 青 `0x0070C0` | — | 二重 | — |
| `parseInt` | 紫 `0x8060C0` | — | 二重 | — |
| `random` | マゼンタ `0xC000C0` | — | 二重 | — |
| `srand` | 濃赤 `0xC00000` | **あり** | 二重 | — |

</details>

<details>
<summary><code>嗚喵</code> — リストと文字列</summary>

| 組み込み関数 | 色 | 太字 | 下線 | ハイライト |
| --- | --- | --- | --- | --- |
| `contains` | オレンジ `0xC07000` | **あり** | 二重 | — |
| `list` | 暗い金色 `0x909000` | **あり** | 二重 | — |
| `cons` | 緑 `0x008000` | **あり** | 二重 | — |
| `head` | 青緑 `0x008080` | **あり** | 二重 | — |
| `tail` | 青 `0x0070C0` | **あり** | 二重 | — |
| `len` | 紫 `0x8060C0` | **あり** | 二重 | — |
| `get` | マゼンタ `0xC000C0` | **あり** | 二重 | — |
| `set` | 濃赤 `0xC00000` | — | — | 黄色 |
| `push` | オレンジ `0xC07000` | — | — | 黄色 |
| `pop` | 暗い金色 `0x909000` | — | — | 黄色 |
| `concat` | 緑 `0x008000` | — | — | 黄色 |
| `slice` | 青緑 `0x008080` | — | — | 黄色 |
| `range` | 青 `0x0070C0` | — | — | 黄色 |
| `str` | 紫 `0x8060C0` | — | — | 黄色 |
| `textLen` | マゼンタ `0xC000C0` | — | — | 黄色 |
| `charAt` | 濃赤 `0xC00000` | **あり** | — | 黄色 |
| `substring` | オレンジ `0xC07000` | **あり** | — | 黄色 |
| `join` | 暗い金色 `0x909000` | **あり** | — | 黄色 |

</details>

<details>
<summary><code>嗚嗚</code> — 制御フローと構文</summary>

| 組み込み関数 | 色 | 太字 | 下線 | ハイライト |
| --- | --- | --- | --- | --- |
| `let` | 緑 `0x008000` | **あり** | — | 黄色 |
| `lambda` | 青緑 `0x008080` | **あり** | — | 黄色 |
| `integer` | 青 `0x0070C0` | **あり** | — | 黄色 |
| `if` | 紫 `0x8060C0` | **あり** | — | 黄色 |
| `while` | マゼンタ `0xC000C0` | **あり** | — | 黄色 |
| `for` | 濃赤 `0xC00000` | — | 二重 | 黄色 |
| `do` | オレンジ `0xC07000` | — | 二重 | 黄色 |
| `return` | 暗い金色 `0x909000` | — | 二重 | 黄色 |
| `and` | 緑 `0x008000` | — | 二重 | 黄色 |
| `or` | 青緑 `0x008080` | — | 二重 | 黄色 |

</details>

## 例

- `examples/hello.meow` — 入力と出力。
- `examples/test1.meow` — 算術、制御フロー、関数、リスト、文字列。
- `examples/game2048.meow` — 完全に遊べるテキストモードの 2048。

```bash
python3 tools/meowc.py examples/test1.meow
./native/meow examples/test1.docx
```

## 2 つの実装

本 fork には同じ言語の 2 つの実装が含まれています：

| 実装 | 成果物 | 実行環境 | 状態 |
| --- | --- | --- | --- |
| **ネイティブ版（推奨）** | `native/meow` | 不要（ネイティブ ELF 実行ファイル） | 主要開発版 |
| Java 版（旧版） | `meow.jar` | JVM 21 以上 | 参考用として保持 |

どちらも同じ `.docx` ファイルを読み込みます。ネイティブ版のテキスト出力は
Java 版と一致しますが、一部の ANSI リセットシーケンスの書き方だけが異なります。

## 本 fork の新機能

| 項目 | 上流 | 本 fork（Java） | 本 fork（ネイティブ） |
| --- | --- | --- | --- |
| DOCX 解析 | Apache POI + XMLBeans | `ZipFile` + StAX | 独自 ZIP リーダー + expat |
| 実行環境 | JVM | JVM | JVM 不要 |
| 成果物サイズ | 約 22.6 MB JAR | 約 1.5 MB JAR | 約 56 KB 実行ファイル |
| `demo/HelloWorld.docx` | 約 1.7 秒 | 約 0.42 秒 | 約 0.024 秒 |
| `play/card.docx` | 約 1.5 秒 | 約 0.37 秒 | 約 0.03 秒 |

ネイティブ版は ZIP 中央ディレクトリリーダーを自前で実装し、zlib でエントリを
展開、expat で `word/document.xml` とそのリレーションを解析し、libpng と
libjpeg で埋め込み PNG/JPEG をデコードします。画像は Java 版と同様に ANSI
ハーフブロックアートとして出力されます。

### 性能

Termux / Android aarch64 で測定（ネイティブ版 50 回、Python 3.13.13
リファレンス 30 回）：

| ワークロード | 1 回あたりの時間 |
| --- | --- |
| ネイティブ `meow --version`（起動のみ） | 0.020 秒 |
| ネイティブ完全版 `demo/HelloWorld.docx`（ZIP + XML + インタープリター + PNG + ANSI） | 0.024 秒 |
| Python 3.13 の素の起動（`python3 -c pass`） | 0.050 秒 |
| Python 3.13 で `word/document.xml` の展開のみ | 0.084 秒 |

ネイティブ実行ファイルはデモ全体を、Python が起動するだけの時間より速く
完了し、Python が文書 XML を抽出するだけの時間の 3 倍以上速いです。Python の
2 行は解析も解釈もレンダリングも行っておらず、下限値であって同等の実装では
ありません。

## ネイティブ実行ファイルのビルド

要件：

- C11 コンパイラ（`clang` または `gcc`）
- `make`
- zlib、expat、libpng、libjpeg の開発ファイル

Termux の場合：

```bash
pkg install clang make zlib expat libpng libjpeg-turbo
```

その後：

```bash
make -C native
```

または CMake を使用：

```bash
cmake -S native -B build-native
cmake --build build-native
```

実行ファイルは `native/meow` に出力されます。

## シンボルフォント（任意、旧版）

本 fork の初期バージョンは、各組み込み記号を独自の `Meow-<name>.ttf`
フォント名に符号化していました。そのため、コンパイルされた DOCX を Word で
読むには 48 個の生成フォントをインストールする必要がありました。
**Meow 暗号がこれを置き換えました。新しくコンパイルされた DOCX には
フォントが一切不要です。**

`fonts/` ディレクトリは、古いフォントエンコード形式の DOCX（およびその
見た目を好む人）との互換性のために残されています。フォントをインストールして
も実行には影響しません。DOCX のテキストは依然として Meow 記号であり、
インタープリターはフォントを無視します。

現在のユーザー向けにインストール（Linux、macOS、Termux）：

```bash
./fonts/install-fonts.sh --user
```

Android のシステム全体へのインストールには root と再起動が必要です：

```bash
su -c 'sh fonts/install-fonts.sh --android-module --yes'
```

リリースインストーラーはデフォルトでフォントをインストールしません。
必要に応じて `--fonts` を指定してください。
詳細（バックアップとアンインストールの動作を含む）は `fonts/README.md` を
参照してください。

## Java 版のビルドと実行（任意）

```bash
./gradlew meow
java -jar meow.jar ./demo/HelloWorld.docx
```

または自己実行スクリプトを使用：

```bash
./meow.sh ./demo/HelloWorld.docx
```

## プロジェクト構成

```text
native/
├── meow.c                 ネイティブインタープリター（ZIP + XML + ANSI + 画像）
├── Makefile               pkg-config / フォールバックビルド
└── CMakeLists.txt         ポータブルな CMake ビルド
fonts/                     旧版、任意のシンボルフォント
├── Meow-*.ttf             U+55B5 -> 読みやすい組み込み名（旧エンコード）
├── install-fonts.sh       ユーザー / Android モジュールインストーラー
├── README.md              インストールと再生成の手順
└── NOTICE                 Droid Sans Mono の帰属表示
tools/
├── meowc.py               Python 風 .meow -> .docx コンパイラ
├── meow_cipher.py         Meow 暗号の仕様（唯一の真実の情報源）
├── gen_meow_fonts.py      旧版 Meow-* シンボルフォントの再生成
└── package-release.sh     リリース tarball の作成
examples/
├── hello.meow
├── test1.meow
└── game2048.meow
src/main/java/org/glavo/meow/
├── Main.java              Java エントリーポイント
├── Meow.java              記号（書式属性を持つ「Meow」）
├── MeowDocxReader.java    軽量 DOCX リーダー（ZipFile + StAX）
├── MeowRun.java           XWPFRun の軽量代替
├── MeowParser.java        パーサー
├── MeowConsole.java       コンソール抽象化（plain / JLine）
├── JlineConsole.java      遅延読み込みの JLine アダプター
└── value/                 実行時の値と組み込み関数
```

## ライセンス

本リポジトリのすべてのソースコードは
[GPLv3](https://www.gnu.org/licenses/gpl-3.0.html) で公開されています。

その他のすべてのファイル（`.docx`、`.md`、`.png` など）は
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) で公開されて
います。

`fonts/` で生成されたシンボルフォントは例外です。その字形アウトラインは
Droid Sans Mono に由来し、
[Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) で公開されています。
`fonts/NOTICE` と `fonts/LICENSE-APACHE-2.0.txt` を参照してください。

## クレジット

- 元のプロジェクトと言語設計：[Glavo](https://github.com/Glavo)
- **本 fork（ネイティブ実装、軽量パーサー、性能改善、Meow 暗号、多言語
  ドキュメント）：DeepSeek V4.1 Flash**
- [OpenCode](https://opencode.ai) Free API：本 fork の開発とテストに使用した
  無料 API アクセスを提供
- [PLCT Lab](https://plctlab.org) によるスポンサー（上流プロジェクトの
  スポンサー。本 fork は PLCT Lab と提携・スポンサー関係にありません）

<img alt="OpenCode logo" src="./images/opencode-badge.svg" width="258" height="66">

<img alt="PLCT Lab logo" src="./images/plct-badge.svg" width="200" height="200">

<img alt="IntelliJ IDEA logo" src="./images/intellij-badge.svg" width="300" height="93">

元のプロジェクトは JetBrains IntelliJ IDEA で開発されました。
