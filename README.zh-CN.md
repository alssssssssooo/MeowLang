# Meow 语言

[English](README.md) | [繁體中文](README.zh-TW.md) | **简体中文** | [日本語](README.ja.md) | [한국어](README.ko.md)

<small>这是一门为了让猫科动物——例如老虎、猫、猫娘——也能当程序员而研发的编程语言。</small>

> **🐱 纯属娱乐，不可当作正规使用**
>
> MeowLang 是一门为了好玩而设计的 esoteric 玩具语言。**它不是正式语言，
> 绝不可用于严肃、学术或任何实际用途。** 不保证稳定性，也不提供支持承诺。
> 如果猫咪坐在键盘上而程序依然能运行，那就是一次成功的代码审查。

> **Fork 说明**
>
> 本仓库是 [Glavo/MeowLang](https://github.com/Glavo/MeowLang) 的 fork。
> **本 fork（包含原生 C 实现、轻量 DOCX 解析器、性能重写、喵语密码与多语言
> 文档）由 DeepSeek V4.1 Flash 开发。**
> 原始语言设计与原始 Java 实现归 Glavo 所有。

MeowLang 是一门 esoteric 编程语言，**它的源代码就是 Microsoft Word 文档**。
一支程序是一整片套用格式的 `喵` 符号，而**格式就是语法**：颜色、粗体、下划线、
高亮与斜体，决定每个 `喵` 的意义。编译后的 `.docx` 在 Word 里看起来像
被加密的猫，而原生解释器可以直接运行它。

![](./images/lambda.png)

## 特性

### 富文本编程

Meow 语言使用 Microsoft Word 作为编辑器，支持富文本编程。开发者可以在代码中
嵌入格式化文本、图片和其他内容，并将它们输出到终端。

![](./images/richtext.png)
![](./images/richtext-output.png)

### 一个关键字、零个名称

原始 Meow 只保留一个符号：猫的叫声。没有用户自定义标识符、括号或引号。
格式就是身份——两段 `喵` 只有在所有格式属性完全相同时，才是同一个符号。

开发者不需要为变量命名，而是给每个 `喵` 设置不同的颜色、高亮、粗体、下划线
等等。每一种不同的组合，就是一个不同的标识符。

![](./images/meow.png)

### 用任何语言写「喵」

解释器不强制你只能使用英语的「Meow」。它支持数十种语言中的猫叫声：

`meow` · `miaou` · `miao` · `nyan` · `にゃん` · `야옹` · `喵` · `喵呜` ·
`мяу` · `мјау` · `miyav` · `miau` · `miauw` · `μιάου` · `mjau` · `mjäu` ·
`mjav` · `mjal` · `nyávog` · `म्याऊं` · `meong` · `mjá` · `meo meo` ·
`მიაუ` · `myau` · `ngiyavuma` · `ማው` · `maw`

**词汇本身从来不是身份——格式才是。** 上面所有词汇都是可互换的同义词：只要
格式相同，两段符号就是同一个符号，不论它们各自来自哪一种语言。因此你可以用
自己思考时最顺的语言来手写原始 Meow，甚至在同一支程序里混用多种语言——这让
手写**更容易**，而不是更麻烦，因为「喵」本来就没有唯一正确的拼法。大小写也
不影响（`Μιάου`、`Мяу` 都能识别）。如果你使用下方的 Python 风格前端，更是
一个喵都不必打。

![](./images/i18n.png)

### 喵语密码——不需要字体

内置函数与宏以双音节的喵语密码词加上格式来书写。密码词只是视觉分类；
每个内置符号的真正身份，是
**8 种保留色 × 粗体 × 双下划线 × 黄色高亮**
的组合（共 64 种，实际使用 53 种）：

| 密码词 | 分类 |
| --- | --- |
| `喵喵` | 输入输出 |
| `喵嗚` | 数学与比较 |
| `嗚喵` | 列表与字符串 |
| `嗚嗚` | 控制流程与语法 |

由于密码只使用标准文字与格式，编译后的 DOCX 在任何 Word 安装中都能**直接
正确显示**，完全不需要安装任何东西。完整的代码对照请见
[喵语密码对照表](#喵语密码对照表)；共用的机器可读规格位于
`tools/meow_cipher.py`。

## 快速开始

```bash
# Termux / Android：
pkg install clang make zlib expat libpng libjpeg-turbo
# 其他系统：C11 编译器、make，以及 zlib/expat/libpng/libjpeg
make -C native

# 编译程序并运行
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```

2048 操作：

```text
w = 上    a = 左    s = 下    d = 右    q = 退出
```

## 编写程序：`.meow` 前端

直接手写原始 Meow，意味着要在 Word 里逐一调整每个符号的颜色、粗体、下划线与
斜体。可选的 Python 风格前端（`tools/meowc.py`）会替你完成这些事：你编写
普通的 Python 风格 `.meow` 源代码，它会编译成同样的带格式 `.docx`。运行时
仍然是原生 C 可执行文件；Java 与 Python 都不参与执行。

### 变量与赋值

```python
x = 10
x += 1
items = [1, 2, 3]
items[0] = 9
```

### 算术与比较

```python
print(1 + 2, 7 // 2, 7 % 3, 2 ** 10)
print(x > 5 and x < 100)
print(3 in items)
```

支持的运算符：`+` `-` `*` `/` `//` `%` `**`，比较运算符
`==` `!=` `<` `<=` `>` `>=`，以及 `and` `or` `not`。

`/` 与 `//` 都执行整数除法，并向负无穷取整。
这门语言没有浮点数。

### 条件判断

```python
if x > 10:
    print("big")
elif x == 10:
    print("ten")
else:
    print("small")
```

### 循环

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

`for` 可以迭代列表，也可以迭代文字的 Unicode 码位。

### 函数与提前返回

```python
def fact(n):
    if n <= 1:
        return 1
    return n * fact(n - 1)

print(fact(6))
```

函数支持递归与提前 `return`。

### 列表与字符串

```python
a = [1, 2, 3]
a.append(4)
print(a[0], a[-1], a[1:3], len(a))
print(str(123) + "!", int("42") + 1)
```

### 条件表达式

```python
print("yes" if 3 > 2 else "no")
```

### 名称映射

为了让运行时保持精简，前端会把部分 Python 名称映射到原生内置函数：

| Python 语法 | 运行时符号 | 说明 |
| --- | --- | --- |
| `print(...)` | `println` | 以空格分隔参数，最后换行 |
| `input(prompt)` | `readline` | 从标准输入读取一行 |
| `int(text)` | `parseInt` | 十进制整数字符串转整数 |
| `a.append(x)` | `push` | 将元素追加到列表末尾（原地修改） |
| `a.pop()` | `pop` | 移除并返回最后一个元素 |
| `x in a` | `contains(a, x)` | 列表成员或子字符串测试 |

### 尚未支持

- `break`、`continue` 与循环的 `else`
- 类、异常与 `with`
- 关键字参数、默认参数与可变长度参数
- `not in`、`is`，以及 `a < b < c` 这种连链比较
- 浮点数与 f-string
- `extend`、`insert`、`remove` 这类原地列表方法
- `.upper()`、`.lower()` 这类字符串方法

需要这些功能时，请使用原始 Meow 语法，或扩展 `native/meow.c` 与
`tools/meowc.py`。

### 前端命令行

```text
python3 tools/meowc.py SOURCE [-o OUTPUT.docx] [--dump-ast] [--dump-runtime]
```

- 未指定 `-o` 时，输出文件名沿用来源文件名，扩展名改为 `.docx`。
- `--dump-ast` 会打印前端 AST；`--dump-runtime` 会打印生成的运行时
  表达式。两者都供调试使用。

## 运行程序

```bash
./native/meow ./demo/HelloWorld.docx
```

可以一次传入多个 `.docx` 文件；每个文件都会获得全新的 context。

```bash
./native/meow ./demo/HelloWorld.docx ./play/card.docx
```

### 环境变量

| 变量 | 实现 | 作用 |
| --- | --- | --- |
| `MEOW_DEBUG=1` | 原生版 | 打印解析后的表达式与求值跟踪 |
| `MEOW_PLAIN=1` | Java 版 | 一律使用轻量纯文本控制台（不使用 JLine） |
| `MEOW_JLINE=1` | Java 版 | 即使 stdin 不是终端也强制使用 JLine |

## 手写原始 Meow

原始 Meow 直接在 Word 里编写。你不需要任何字体，只需要标准的格式设置面板。

### 格式模型

- 一个段落就是一个表达式列表。不是喵语词汇的 run（空白、引号外的普通文字）
  会被跳过。
- **斜体 `喵` 成对 = 一个分组**（括号）。开始与结束的 run 必须具有相同格式；
  编译器会依嵌套深度调整分组的灰度，但任何一致的斜体颜色都可以。
- **单下划线 `喵` 成对 = 文本字面量。** 编译后的格式使用颜色 `0x7030A0`；
  两个分隔符之间的内容会以富文本形式保留。
- **颜色唯一的普通 `喵` = 变量。** 颜色不可与下方的保留色板冲突。
- **带有保留格式的密码词 = 内置符号。**

保留色板（绝不可用于自己的变量）：
`0x000000`（字面量内容）、`0x7030A0`（字面量分隔符），以及八种密码颜色
`0xC00000`、`0xC07000`、`0x909000`、`0x008000`、`0x008080`、`0x0070C0`、
`0x8060C0`、`0xC000C0`。变量可以使用其他任何颜色。

可接受的喵语词汇（全部可互换，大小写不敏感）：*用任何语言写「喵」*一节列出的
28 个词，再加上四个密码词 `喵喵`、`喵嗚`、`嗚喵`、`嗚嗚`。

### 内置宏

| 宏 | 形态 | 意义 |
| --- | --- | --- |
| `let` | `(x value)` | 将 `value` 绑定到变量 `x` |
| `lambda` | `(params body...)` | 函数值；`params` 是单个标识符或标识符分组 |
| `if` | `(cond then [else])` | 条件判断 |
| `while` | `(cond body...)` | 当 `cond` 为真时重复执行 |
| `for` | `(var iterable body...)` | 迭代列表或文本 |
| `do` | `(body...)` | 依序求值 |
| `return` | 从所属函数提前返回 | |
| `and` / `or` | 短路布尔运算 | |
| `integer` | 字号编码的 `喵` | 旧版整数字面量（数值存储在字体大小中）；建议改用 `parseInt("...")` |

### 内置函数

- **输入输出**：`print`、`println`、`readline`、`clear`
- **算术**：`add`、`sub`、`mul`、`div`、`mod`、`pow`、`neg`、`abs`、
  `min`、`max`、`sum`
- **比较**：`eq`、`ne`、`lt`、`le`、`gt`、`ge`、`not`
- **列表**：`list`、`cons`、`head`、`tail`、`len`、`get`、`set`、`push`、
  `pop`、`concat`、`slice`、`range`、`contains`、`join`
- **字符串**：`str`、`parseInt`、`textLen`、`charAt`、`substring`
- **其他**：`random`、`srand`

### 喵语密码对照表

每个内置符号都是它的密码词、颜色、粗体、双下划线与黄色高亮的组合。
`native/meow.c` 中的原生表与编译器（`tools/meow_cipher.py`）由同一份规格
生成，因此两者永远不会不同步。

<details open>
<summary><code>喵喵</code> — 输入输出</summary>

| 内置函数 | 颜色 | 粗体 | 下划线 | 高亮 |
| --- | --- | --- | --- | --- |
| `print` | 深红 `0xC00000` | — | — | — |
| `readline` | 橙色 `0xC07000` | — | — | — |
| `println` | 暗金 `0x909000` | — | — | — |
| `clear` | 绿色 `0x008000` | — | — | — |

</details>

<details>
<summary><code>喵嗚</code> — 数学与比较</summary>

| 内置函数 | 颜色 | 粗体 | 下划线 | 高亮 |
| --- | --- | --- | --- | --- |
| `add` | 青绿 `0x008080` | — | — | — |
| `sub` | 蓝色 `0x0070C0` | — | — | — |
| `mul` | 紫色 `0x8060C0` | — | — | — |
| `div` | 洋红 `0xC000C0` | — | — | — |
| `mod` | 深红 `0xC00000` | **是** | — | — |
| `pow` | 橙色 `0xC07000` | **是** | — | — |
| `neg` | 暗金 `0x909000` | **是** | — | — |
| `abs` | 绿色 `0x008000` | **是** | — | — |
| `min` | 青绿 `0x008080` | **是** | — | — |
| `max` | 蓝色 `0x0070C0` | **是** | — | — |
| `sum` | 紫色 `0x8060C0` | **是** | — | — |
| `eq` | 洋红 `0xC000C0` | **是** | — | — |
| `ne` | 深红 `0xC00000` | — | 双下划线 | — |
| `lt` | 橙色 `0xC07000` | — | 双下划线 | — |
| `le` | 暗金 `0x909000` | — | 双下划线 | — |
| `gt` | 绿色 `0x008000` | — | 双下划线 | — |
| `ge` | 青绿 `0x008080` | — | 双下划线 | — |
| `not` | 蓝色 `0x0070C0` | — | 双下划线 | — |
| `parseInt` | 紫色 `0x8060C0` | — | 双下划线 | — |
| `random` | 洋红 `0xC000C0` | — | 双下划线 | — |
| `srand` | 深红 `0xC00000` | **是** | 双下划线 | — |

</details>

<details>
<summary><code>嗚喵</code> — 列表与字符串</summary>

| 内置函数 | 颜色 | 粗体 | 下划线 | 高亮 |
| --- | --- | --- | --- | --- |
| `contains` | 橙色 `0xC07000` | **是** | 双下划线 | — |
| `list` | 暗金 `0x909000` | **是** | 双下划线 | — |
| `cons` | 绿色 `0x008000` | **是** | 双下划线 | — |
| `head` | 青绿 `0x008080` | **是** | 双下划线 | — |
| `tail` | 蓝色 `0x0070C0` | **是** | 双下划线 | — |
| `len` | 紫色 `0x8060C0` | **是** | 双下划线 | — |
| `get` | 洋红 `0xC000C0` | **是** | 双下划线 | — |
| `set` | 深红 `0xC00000` | — | — | 黄色 |
| `push` | 橙色 `0xC07000` | — | — | 黄色 |
| `pop` | 暗金 `0x909000` | — | — | 黄色 |
| `concat` | 绿色 `0x008000` | — | — | 黄色 |
| `slice` | 青绿 `0x008080` | — | — | 黄色 |
| `range` | 蓝色 `0x0070C0` | — | — | 黄色 |
| `str` | 紫色 `0x8060C0` | — | — | 黄色 |
| `textLen` | 洋红 `0xC000C0` | — | — | 黄色 |
| `charAt` | 深红 `0xC00000` | **是** | — | 黄色 |
| `substring` | 橙色 `0xC07000` | **是** | — | 黄色 |
| `join` | 暗金 `0x909000` | **是** | — | 黄色 |

</details>

<details>
<summary><code>嗚嗚</code> — 控制流程与语法</summary>

| 内置函数 | 颜色 | 粗体 | 下划线 | 高亮 |
| --- | --- | --- | --- | --- |
| `let` | 绿色 `0x008000` | **是** | — | 黄色 |
| `lambda` | 青绿 `0x008080` | **是** | — | 黄色 |
| `integer` | 蓝色 `0x0070C0` | **是** | — | 黄色 |
| `if` | 紫色 `0x8060C0` | **是** | — | 黄色 |
| `while` | 洋红 `0xC000C0` | **是** | — | 黄色 |
| `for` | 深红 `0xC00000` | — | 双下划线 | 黄色 |
| `do` | 橙色 `0xC07000` | — | 双下划线 | 黄色 |
| `return` | 暗金 `0x909000` | — | 双下划线 | 黄色 |
| `and` | 绿色 `0x008000` | — | 双下划线 | 黄色 |
| `or` | 青绿 `0x008080` | — | 双下划线 | 黄色 |

</details>

## 示例

- `examples/hello.meow` — 输入与输出。
- `examples/test1.meow` — 算术、控制流程、函数、列表与字符串。
- `examples/game2048.meow` — 完整可玩的文本模式 2048。

```bash
python3 tools/meowc.py examples/test1.meow
./native/meow examples/test1.docx
```

## 两种实现

本 fork 提供同一套语言的两种实现：

| 实现 | 产物 | 运行环境 | 状态 |
| --- | --- | --- | --- |
| **原生版（推荐）** | `native/meow` | 无需运行时（原生 ELF 可执行文件） | 主要开发版本 |
| Java 版（旧版） | `meow.jar` | JVM 21 以上 | 保留作为参考 |

两者读取相同的 `.docx` 文件。原生版产生的文本输出与 Java 版一致，只有少数
ANSI 重置序列的写法不同。

## 本 fork 的新增内容

| 项目 | 上游 | 本 fork（Java） | 本 fork（原生） |
| --- | --- | --- | --- |
| DOCX 解析 | Apache POI + XMLBeans | `ZipFile` + StAX | 自制 ZIP 读取器 + expat |
| 运行环境 | JVM | JVM | 不需要 JVM |
| 产物大小 | 约 22.6 MB JAR | 约 1.5 MB JAR | 约 56 KB 可执行文件 |
| `demo/HelloWorld.docx` | 约 1.7 秒 | 约 0.42 秒 | 约 0.024 秒 |
| `play/card.docx` | 约 1.5 秒 | 约 0.37 秒 | 约 0.03 秒 |

原生版自行实现 ZIP 中央目录读取器，使用 zlib 解压条目，以 expat 解析
`word/document.xml` 及其关联文件，并以 libpng 与 libjpeg 解码内嵌的 PNG/JPEG
图片。图片会像 Java 版一样，以 ANSI 半块字符艺术输出。

### 性能

在 Termux / Android aarch64 上测量（原生版 50 次、Python 3.13.13 参考
30 次）：

| 工作负载 | 每次运行时间 |
| --- | --- |
| 原生 `meow --version`（仅启动） | 0.020 秒 |
| 原生完整 `demo/HelloWorld.docx`（ZIP + XML + 解释器 + PNG + ANSI） | 0.024 秒 |
| Python 3.13 裸启动（`python3 -c pass`） | 0.050 秒 |
| Python 3.13 仅解压 `word/document.xml` | 0.084 秒 |

原生可执行文件跑完整个 demo 的时间，比 Python 光是启动还快，也比 Python 仅取出
文档 XML 快三倍以上。Python 那两行并没有解析、解释或渲染任何内容；它们只是
下限，不是等效实现。

## 构建原生可执行文件

需求：

- C11 编译器（`clang` 或 `gcc`）
- `make`
- zlib、expat、libpng、libjpeg 开发文件

在 Termux 上：

```bash
pkg install clang make zlib expat libpng libjpeg-turbo
```

然后：

```bash
make -C native
```

或使用 CMake：

```bash
cmake -S native -B build-native
cmake --build build-native
```

可执行文件会输出到 `native/meow`。

## 符号字体（可选，旧版）

本 fork 的早期版本会把每个内置符号编码成自定义的 `Meow-<name>.ttf` 字体名称，
这意味着必须安装 48 个生成的字体，才能在 Word 里读取编译后的 DOCX。
**喵语密码已取代这套做法：新编译的 DOCX 完全不需要字体。**

`fonts/` 目录是为了兼容旧的字体重编码 DOCX（以及偏好那种外观的人）而保留。
安装字体永远不影响执行：DOCX 的文字仍然是喵符号，而解释器会忽略字体。

为当前用户安装（Linux、macOS、Termux）：

```bash
./fonts/install-fonts.sh --user
```

Android 系统级安装需要 root，且必须重新启动：

```bash
su -c 'sh fonts/install-fonts.sh --android-module --yes'
```

发行版安装脚本默认不安装字体；需要时请加上 `--fonts`。
详情（包含备份与卸载行为）请见 `fonts/README.md`。

## 构建并运行 Java 版（可选）

```bash
./gradlew meow
java -jar meow.jar ./demo/HelloWorld.docx
```

或使用自执行脚本：

```bash
./meow.sh ./demo/HelloWorld.docx
```

## 项目结构

```text
native/
├── meow.c                 原生解释器（ZIP + XML + ANSI + 图片）
├── Makefile               pkg-config / 备用构建
└── CMakeLists.txt         可移植的 CMake 构建
fonts/                     旧版、可选的符号字体
├── Meow-*.ttf             U+55B5 -> 可读的内置名称（旧编码）
├── install-fonts.sh       用户 / Android 模块安装脚本
├── README.md              安装与重新生成说明
└── NOTICE                 Droid Sans Mono 授权信息
tools/
├── meowc.py               Python 风格 .meow -> .docx 编译器
├── meow_cipher.py         喵语密码规格（唯一真实来源）
├── gen_meow_fonts.py      重新生成旧版 Meow-* 符号字体
└── package-release.sh     生成发行版压缩包
examples/
├── hello.meow
├── test1.meow
└── game2048.meow
src/main/java/org/glavo/meow/
├── Main.java              Java 入口点
├── Meow.java              符号（带格式属性的「Meow」）
├── MeowDocxReader.java    轻量 DOCX 读取器（ZipFile + StAX）
├── MeowRun.java           XWPFRun 的轻量替代品
├── MeowParser.java        解析器
├── MeowConsole.java       控制台抽象层（plain / JLine）
├── JlineConsole.java      延迟加载的 JLine 适配层
└── value/                 运行时值与内置函数
```

## 授权

本项目的所有源代码以
[GPLv3](https://www.gnu.org/licenses/gpl-3.0.html) 发布。

其他所有文件（`.docx`、`.md`、`.png` 等）以
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/) 发布。

`fonts/` 中生成的符号字体是例外：其字形轮廓衍生自 Droid Sans Mono，以
[Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) 发布。请见
`fonts/NOTICE` 与 `fonts/LICENSE-APACHE-2.0.txt`。

## 致谢

- 原始项目与语言设计：[Glavo](https://github.com/Glavo)
- **本 fork（原生实现、轻量解析器、性能改进、喵语密码与多语言文档）：
  DeepSeek V4.1 Flash**
- [OpenCode](https://opencode.ai) Free API：提供开发与测试本 fork 所需的
  免费 API 访问
- 由 [PLCT Lab](https://plctlab.org) 赞助（上游项目的赞助者；本 fork 与
  PLCT Lab 无隶属或赞助关系）

<img alt="OpenCode logo" src="./images/opencode-badge.svg" width="258" height="66">

<img alt="PLCT Lab logo" src="./images/plct-badge.svg" width="200" height="200">

<img alt="IntelliJ IDEA logo" src="./images/intellij-badge.svg" width="300" height="93">

原始项目使用 JetBrains IntelliJ IDEA 开发。
