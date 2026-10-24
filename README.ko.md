# Meow 언어

[English](README.md) | [繁體中文](README.zh-TW.md) | [简体中文](README.zh-CN.md) | [日本語](README.ja.md) | **한국어**

[![CI](https://github.com/alssssssssooo/MeowLang/actions/workflows/ci.yml/badge.svg)](https://github.com/alssssssssooo/MeowLang/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/v/release/alssssssssooo/MeowLang?display_name=tag&sort=semver)](https://github.com/alssssssssooo/MeowLang/releases/latest)

<small>이 언어는 호랑이, 고양이, 고양이 소녀 같은 고양잇과 동물도 프로그래머가 될 수 있도록 개발된 프로그래밍 언어입니다.</small>

> **🐱 순수한 오락용이며 정식 용도로 사용할 수 없습니다**
>
> MeowLang은 재미를 위해 만들어진 esoteric 장난감 언어입니다.
> **프로덕션 언어가 아니며, 진지한 용도, 학술적 용도, 그 밖의 모든 실무에
> 절대 사용해서는 안 됩니다.** 안정성은 보장되지 않으며 지원도 약속되지
> 않습니다. 고양이가 키보드 위에 올라가도 프로그램이 실행된다면, 그것은
> 성공적인 코드 리뷰입니다.

> **Fork 안내**
>
> 이 저장소는 [Glavo/MeowLang](https://github.com/Glavo/MeowLang)의 fork입니다.
> **이 fork(네이티브 C 구현, 경량 DOCX 파서, 성능 개선, Meow 암호, 다국어
> 문서)는 DeepSeek V4.1 Flash가 개발했습니다.**
> 원래 언어 설계와 원래 Java 구현은 Glavo에 귀속됩니다.

MeowLang은 esoteric 프로그래밍 언어로, **소스 코드가 곧 Microsoft Word
문서**입니다. 프로그램은 서식이 적용된 `喵` 기호의 벽이며, **서식이 곧
문법**입니다. 색상, 굵게, 밑줄, 강조 표시, 기울임이 각 `喵`의 의미를
결정합니다. 컴파일된 `.docx`는 Word에서 암호화된 고양이처럼 보이지만,
네이티브 인터프리터가 그대로 실행할 수 있습니다.

![](./images/lambda.png)

## 특징

### 리치 텍스트 프로그래밍

Meow 언어는 Microsoft Word를 편집기로 사용합니다. 리치 텍스트 프로그래밍을
지원하므로 서식이 있는 텍스트, 이미지 등을 소스 코드에 직접 넣고 터미널로
출력할 수 있습니다.

![](./images/richtext.png)
![](./images/richtext-output.png)

### 키워드 하나, 이름 없음

원시 Meow에는 기호가 하나뿐입니다. 고양이 울음소리입니다. 사용자 정의 식별자,
괄호, 따옴표는 없습니다. 서식이 곧 정체성이며, 두 `喵`는 모든 서식 속성이
완전히 같을 때만 같은 기호가 됩니다.

변수 이름을 고민하는 대신 개발자는 각 `喵`에 서로 다른 색상, 강조 표시, 굵게,
밑줄 등을 지정합니다. 조합이 다르면 서로 다른 식별자로 취급됩니다.

![](./images/meow.png)

### 어떤 언어로든 "야옹"이라고 쓰기

인터프리터는 영어 "Meow"만 강요하지 않습니다. 수십 개 언어의 고양이
울음소리를 받아들입니다:

`meow` · `miaou` · `miao` · `nyan` · `にゃん` · `야옹` · `喵` · `喵呜` ·
`мяу` · `мјау` · `miyav` · `miau` · `miauw` · `μιάου` · `mjau` · `mjäu` ·
`mjav` · `mjal` · `nyávog` · `म्याऊं` · `meong` · `mjá` · `meo meo` ·
`მიაუ` · `myau` · `ngiyavuma` · `ማው` · `maw`

**단어 자체가 정체성이 되는 일은 결코 없습니다. 서식이 곧 정체성입니다.**
위의 모든 단어는 호환되는 동의어입니다. 서식이 같으면 각 단어가 어느 언어에서
왔든 같은 기호로 취급됩니다. 따라서 자신이 생각할 때 가장 편한 언어로 원시
Meow를 작성할 수 있고, 한 프로그램 안에서 여러 언어를 섞을 수도 있습니다.
이것은 손으로 쓰는 일을 **더 어렵게 만드는 것이 아니라 더 쉽게 만듭니다**.
"야옹"에는 유일하게 올바른 표기가 없기 때문입니다. 대소문자도 구분하지
않습니다(`Μιάου`, `Мяу`도 인식됨). 아래의 Python 스타일 프론트엔드를 사용하면
야옹을 하나도 입력할 필요가 없습니다.

![](./images/i18n.png)

### Meow 암호 — 글꼴 불필요

내장 함수와 매크로는 두 음절의 Meow 암호어와 서식으로 작성합니다. 암호어는
시각적 분류일 뿐이며, 각 내장 기호의 진짜 정체성은
**예약된 8가지 색 × 굵게 × 이중 밑줄 × 노란색 강조 표시**
의 조합입니다(총 64가지, 그중 53가지 사용):

| 암호어 | 분류 |
| --- | --- |
| `喵喵` | 입출력 |
| `喵嗚` | 산술과 비교 |
| `嗚喵` | 리스트와 문자열 |
| `嗚嗚` | 제어 흐름과 구문 |

암호는 표준 텍스트와 서식만 사용하므로, 컴파일된 DOCX는 **어떤 Word에서도
설치 없이 그대로 올바르게 표시**됩니다. 전체 코드 대응표는
[Meow 암호 대응표](#meow-암호-대응표)를, 공유 기계 판독 가능 명세는
`tools/meow_cipher.py`를 참조하세요.

## 빠른 시작

```bash
# Termux / Android:
pkg install clang make zlib expat libpng libjpeg-turbo
# 기타 시스템: C11 컴파일러, make, zlib/expat/libpng/libjpeg
make -C native

# 프로그램 컴파일 및 실행
python3 tools/meowc.py examples/game2048.meow -o game2048.docx
./native/meow game2048.docx
```

2048 조작:

```text
w = 위    a = 왼쪽    s = 아래    d = 오른쪽    q = 종료
```

## 프로그램 작성: `.meow` 프론트엔드

원시 Meow를 직접 작성하려면 Word에서 기호마다 색상, 굵게, 밑줄, 기울임을
일일이 조정해야 합니다. 선택적 Python 스타일 프론트엔드(`tools/meowc.py`)가
그 작업을 대신합니다. 평범한 Python 스타일 `.meow` 소스를 작성하면 동일한
서식의 `.docx`로 컴파일됩니다. 실행 시점에는 여전히 네이티브 C 실행 파일이며,
Java와 Python은 실행에 관여하지 않습니다.

### 변수와 대입

```python
x = 10
x += 1
items = [1, 2, 3]
items[0] = 9
```

### 산술과 비교

```python
print(1 + 2, 7 // 2, 7 % 3, 2 ** 10)
print(x > 5 and x < 100)
print(3 in items)
```

지원 연산자: `+` `-` `*` `/` `//` `%` `**`, 비교 연산자
`==` `!=` `<` `<=` `>` `>=`, 그리고 `and` `or` `not`.

`/`와 `//`는 모두 정수 나눗셈이며 음의 무한대 방향으로 내림합니다.
이 언어에는 부동소수점 수가 없습니다.

### 조건문

```python
if x > 10:
    print("big")
elif x == 10:
    print("ten")
else:
    print("small")
```

### 루프

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

`for`는 리스트와 텍스트의 유니코드 코드 포인트를 순회할 수 있습니다.

### 함수와 조기 반환

```python
def fact(n):
    if n <= 1:
        return 1
    return n * fact(n - 1)

print(fact(6))
```

함수는 재귀와 조기 `return`을 지원합니다.

### 리스트와 문자열

```python
a = [1, 2, 3]
a.append(4)
print(a[0], a[-1], a[1:3], len(a))
print(str(123) + "!", int("42") + 1)
```

### 조건식

```python
print("yes" if 3 > 2 else "no")
```

### 이름 대응

실행 시점을 작게 유지하기 위해 프론트엔드는 일부 Python 이름을 네이티브 내장
함수에 대응시킵니다:

| Python 구문 | 실행 시점 기호 | 설명 |
| --- | --- | --- |
| `print(...)` | `println` | 인수를 공백으로 구분해 출력하고 줄바꿈 |
| `input(prompt)` | `readline` | 표준 입력에서 한 줄 읽기 |
| `int(text)` | `parseInt` | 10진 정수 문자열을 정수로 변환 |
| `a.append(x)` | `push` | 리스트 끝에 요소 추가(제자리 수정) |
| `a.pop()` | `pop` | 마지막 요소를 제거하고 반환 |
| `x in a` | `contains(a, x)` | 리스트 포함 여부 또는 부분 문자열 검사 |

### 아직 지원하지 않는 기능

- `break`, `continue`, 루프의 `else`
- 클래스, 예외, `with`
- 키워드 인수, 기본 인수, 가변 길이 인수
- `not in`, `is`, `a < b < c` 같은 연쇄 비교
- 부동소수점 수와 f-string
- `extend`, `insert`, `remove` 같은 제자리 리스트 메서드
- `.upper()`, `.lower()` 같은 문자열 메서드

이 기능이 필요하면 원시 Meow 구문을 사용하거나 `native/meow.c`와
`tools/meowc.py`를 확장하세요.

### 프론트엔드 명령줄

```text
python3 tools/meowc.py SOURCE [-o OUTPUT.docx] [--dump-ast] [--dump-runtime]
```

- `-o`를 생략하면 출력 파일 이름은 소스 이름에서 확장자만 `.docx`로 바뀝니다.
- `--dump-ast`는 프론트엔드 AST를, `--dump-runtime`은 생성된 실행 시점
  표현식을 출력합니다. 둘 다 디버깅용입니다.

## 프로그램 실행

```bash
./native/meow ./demo/HelloWorld.docx
```

여러 `.docx` 파일을 한 번에 넘길 수 있습니다. 각 파일은 새로운 context를
받습니다.

```bash
./native/meow ./demo/HelloWorld.docx ./play/card.docx
```

### 환경 변수

| 변수 | 구현 | 효과 |
| --- | --- | --- |
| `MEOW_DEBUG=1` | 네이티브 | 파싱된 표현식과 평가 추적 출력 |
| `MEOW_PLAIN=1` | Java | 항상 경량 일반 콘솔 사용(JLine 미사용) |
| `MEOW_JLINE=1` | Java | stdin이 터미널이 아니어도 JLine 강제 사용 |

## 원시 Meow 작성

원시 Meow는 Word에서 직접 작성합니다. 글꼴은 전혀 필요 없고 표준 서식 패널만
사용합니다.

### 형식 모델

- 하나의 단락이 하나의 표현식 리스트입니다. Meow 어휘가 아닌 run(공백, 따옴표
  밖의 일반 텍스트)은 건너뜁니다.
- **기울임 `喵` 쌍 = 그룹**(괄호). 여는 run과 닫는 run은 같은 서식이어야
  합니다. 컴파일러는 중첩 깊이에 따라 그룹의 회색조를 바꾸지만, 일관된 기울임
  색상이면 무엇이든 됩니다.
- **한 줄 밑줄 `喵` 쌍 = 텍스트 리터럴.** 컴파일된 형식은 색상 `0x7030A0`을
  사용하며, 구분자 사이의 내용은 리치 텍스트로 보존됩니다.
- **색상이 고유한 일반 `喵` = 변수.** 색상은 아래 예약 팔레트와 충돌해서는 안
  됩니다.
- **예약 서식을 가진 암호어 = 내장 기호.**

예약 팔레트(자신의 변수에 절대 사용하지 마세요):
`0x000000`(리터럴 내용), `0x7030A0`(리터럴 구분자), 그리고 8가지 암호 색상
`0xC00000`, `0xC07000`, `0x909000`, `0x008000`, `0x008080`, `0x0070C0`,
`0x8060C0`, `0xC000C0`. 변수에는 그 밖의 다른 색상을 사용할 수 있습니다.

허용되는 Meow 어휘(모두 호환, 대소문자 구분 없음): *어떤 언어로든 "야옹"이라고
쓰기* 절에 나열된 28개 단어와 4개 암호어 `喵喵`, `喵嗚`, `嗚喵`, `嗚嗚`.

### 내장 매크로

| 매크로 | 형태 | 의미 |
| --- | --- | --- |
| `let` | `(x value)` | `value`를 변수 `x`에 바인딩 |
| `lambda` | `(params body...)` | 함수 값. `params`는 식별자 하나 또는 식별자 그룹 |
| `if` | `(cond then [else])` | 조건 분기 |
| `while` | `(cond body...)` | `cond`가 참인 동안 반복 |
| `for` | `(var iterable body...)` | 리스트 또는 텍스트 순회 |
| `do` | `(body...)` | 순서대로 평가 |
| `return` | 속한 함수에서 조기 반환 | |
| `and` / `or` | 단락 부울 연산 | |
| `integer` | 글꼴 크기로 인코딩된 `喵` | 레거시 정수 리터럴(값이 글꼴 크기에 저장됨). `parseInt("...")` 사용 권장 |

### 내장 함수

- **입출력**: `print`, `println`, `readline`, `clear`
- **산술**: `add`, `sub`, `mul`, `div`, `mod`, `pow`, `neg`, `abs`,
  `min`, `max`, `sum`
- **비교**: `eq`, `ne`, `lt`, `le`, `gt`, `ge`, `not`
- **리스트**: `list`, `cons`, `head`, `tail`, `len`, `get`, `set`, `push`,
  `pop`, `concat`, `slice`, `range`, `contains`, `join`
- **문자열**: `str`, `parseInt`, `textLen`, `charAt`, `substring`
- **기타**: `random`, `srand`

### Meow 암호 대응표

각 내장 기호는 암호어, 색상, 굵게, 이중 밑줄, 노란색 강조 표시의 조합입니다.
`native/meow.c`의 네이티브 테이블은 컴파일러(`tools/meow_cipher.py`)와 같은
명세에서 생성되므로 둘이 어긋날 수 없습니다.

<details open>
<summary><code>喵喵</code> — 입출력</summary>

| 내장 함수 | 색상 | 굵게 | 밑줄 | 강조 표시 |
| --- | --- | --- | --- | --- |
| `print` | 진한 빨강 `0xC00000` | — | — | — |
| `readline` | 주황 `0xC07000` | — | — | — |
| `println` | 어두운 금색 `0x909000` | — | — | — |
| `clear` | 초록 `0x008000` | — | — | — |

</details>

<details>
<summary><code>喵嗚</code> — 산술과 비교</summary>

| 내장 함수 | 색상 | 굵게 | 밑줄 | 강조 표시 |
| --- | --- | --- | --- | --- |
| `add` | 청록 `0x008080` | — | — | — |
| `sub` | 파랑 `0x0070C0` | — | — | — |
| `mul` | 보라 `0x8060C0` | — | — | — |
| `div` | 자홍 `0xC000C0` | — | — | — |
| `mod` | 진한 빨강 `0xC00000` | **예** | — | — |
| `pow` | 주황 `0xC07000` | **예** | — | — |
| `neg` | 어두운 금색 `0x909000` | **예** | — | — |
| `abs` | 초록 `0x008000` | **예** | — | — |
| `min` | 청록 `0x008080` | **예** | — | — |
| `max` | 파랑 `0x0070C0` | **예** | — | — |
| `sum` | 보라 `0x8060C0` | **예** | — | — |
| `eq` | 자홍 `0xC000C0` | **예** | — | — |
| `ne` | 진한 빨강 `0xC00000` | — | 이중 | — |
| `lt` | 주황 `0xC07000` | — | 이중 | — |
| `le` | 어두운 금색 `0x909000` | — | 이중 | — |
| `gt` | 초록 `0x008000` | — | 이중 | — |
| `ge` | 청록 `0x008080` | — | 이중 | — |
| `not` | 파랑 `0x0070C0` | — | 이중 | — |
| `parseInt` | 보라 `0x8060C0` | — | 이중 | — |
| `random` | 자홍 `0xC000C0` | — | 이중 | — |
| `srand` | 진한 빨강 `0xC00000` | **예** | 이중 | — |

</details>

<details>
<summary><code>嗚喵</code> — 리스트와 문자열</summary>

| 내장 함수 | 색상 | 굵게 | 밑줄 | 강조 표시 |
| --- | --- | --- | --- | --- |
| `contains` | 주황 `0xC07000` | **예** | 이중 | — |
| `list` | 어두운 금색 `0x909000` | **예** | 이중 | — |
| `cons` | 초록 `0x008000` | **예** | 이중 | — |
| `head` | 청록 `0x008080` | **예** | 이중 | — |
| `tail` | 파랑 `0x0070C0` | **예** | 이중 | — |
| `len` | 보라 `0x8060C0` | **예** | 이중 | — |
| `get` | 자홍 `0xC000C0` | **예** | 이중 | — |
| `set` | 진한 빨강 `0xC00000` | — | — | 노란색 |
| `push` | 주황 `0xC07000` | — | — | 노란색 |
| `pop` | 어두운 금색 `0x909000` | — | — | 노란색 |
| `concat` | 초록 `0x008000` | — | — | 노란색 |
| `slice` | 청록 `0x008080` | — | — | 노란색 |
| `range` | 파랑 `0x0070C0` | — | — | 노란색 |
| `str` | 보라 `0x8060C0` | — | — | 노란색 |
| `textLen` | 자홍 `0xC000C0` | — | — | 노란색 |
| `charAt` | 진한 빨강 `0xC00000` | **예** | — | 노란색 |
| `substring` | 주황 `0xC07000` | **예** | — | 노란색 |
| `join` | 어두운 금색 `0x909000` | **예** | — | 노란색 |

</details>

<details>
<summary><code>嗚嗚</code> — 제어 흐름과 구문</summary>

| 내장 함수 | 색상 | 굵게 | 밑줄 | 강조 표시 |
| --- | --- | --- | --- | --- |
| `let` | 초록 `0x008000` | **예** | — | 노란색 |
| `lambda` | 청록 `0x008080` | **예** | — | 노란색 |
| `integer` | 파랑 `0x0070C0` | **예** | — | 노란색 |
| `if` | 보라 `0x8060C0` | **예** | — | 노란색 |
| `while` | 자홍 `0xC000C0` | **예** | — | 노란색 |
| `for` | 진한 빨강 `0xC00000` | — | 이중 | 노란색 |
| `do` | 주황 `0xC07000` | — | 이중 | 노란색 |
| `return` | 어두운 금색 `0x909000` | — | 이중 | 노란색 |
| `and` | 초록 `0x008000` | — | 이중 | 노란색 |
| `or` | 청록 `0x008080` | — | 이중 | 노란색 |

</details>

## 예제

- `examples/hello.meow` — 입력과 출력.
- `examples/test1.meow` — 산술, 제어 흐름, 함수, 리스트, 문자열.
- `examples/game2048.meow` — 완전히 플레이 가능한 텍스트 모드 2048.

```bash
python3 tools/meowc.py examples/test1.meow
./native/meow examples/test1.docx
```

## 두 가지 구현

이 fork에는 같은 언어의 두 가지 구현이 포함되어 있습니다:

| 구현 | 산출물 | 실행 환경 | 상태 |
| --- | --- | --- | --- |
| **네이티브 버전(권장)** | `native/meow` | 불필요(네이티브 ELF 실행 파일) | 주요 개발 버전 |
| Java 버전(레거시) | `meow.jar` | JVM 21 이상 | 참고용으로 유지 |

둘 다 같은 `.docx` 파일을 읽습니다. 네이티브 버전의 텍스트 출력은 Java 버전과
일치하며, 일부 ANSI 리셋 시퀀스의 표기만 다릅니다.

## 이 fork의 새로운 기능

| 항목 | 업스트림 | 이 fork(Java) | 이 fork(네이티브) |
| --- | --- | --- | --- |
| DOCX 파싱 | Apache POI + XMLBeans | `ZipFile` + StAX | 자체 ZIP 리더 + expat |
| 실행 환경 | JVM | JVM | JVM 불필요 |
| 산출물 크기 | 약 22.6 MB JAR | 약 1.5 MB JAR | 약 56 KB 실행 파일 |
| `demo/HelloWorld.docx` | 약 1.7초 | 약 0.42초 | 약 0.024초 |
| `play/card.docx` | 약 1.5초 | 약 0.37초 | 약 0.03초 |

네이티브 버전은 ZIP 중앙 디렉터리 리더를 직접 구현하고, zlib로 항목을
압축 해제하며, expat로 `word/document.xml`과 그 관계를 파싱하고, libpng와
libjpeg로 내장 PNG/JPEG를 디코딩합니다. 이미지는 Java 버전과 마찬가지로 ANSI
하프 블록 아트로 출력됩니다.

### 성능

Termux / Android aarch64에서 측정(네이티브 50회, Python 3.13.13 참조 30회):

| 작업 부하 | 실행당 시간 |
| --- | --- |
| 네이티브 `meow --version`(시작만) | 0.020초 |
| 네이티브 전체 `demo/HelloWorld.docx`(ZIP + XML + 인터프리터 + PNG + ANSI) | 0.024초 |
| Python 3.13 순수 시작(`python3 -c pass`) | 0.050초 |
| Python 3.13 `word/document.xml` 압축 해제만 | 0.084초 |

네이티브 실행 파일은 전체 데모를 Python이 시작만 하는 시간보다 빠르게
완료하며, Python이 문서 XML만 추출하는 시간보다 3배 이상 빠릅니다. Python
두 행은 파싱, 해석, 렌더링을 전혀 하지 않으며, 하한값일 뿐 동등한 구현이
아닙니다.

## 네이티브 실행 파일 빌드

요구 사항:

- C11 컴파일러(`clang` 또는 `gcc`)
- `make`
- zlib, expat, libpng, libjpeg 개발 파일

Termux에서:

```bash
pkg install clang make zlib expat libpng libjpeg-turbo
```

그다음:

```bash
make -C native
```

또는 CMake 사용:

```bash
cmake -S native -B build-native
cmake --build build-native
```

실행 파일은 `native/meow`에 출력됩니다.

## 기호 글꼴(선택, 레거시)

이 fork의 초기 버전은 각 내장 기호를 사용자 정의 `Meow-<name>.ttf` 글꼴
이름으로 인코딩했습니다. 따라서 컴파일된 DOCX를 Word에서 읽으려면 생성된
48개 글꼴을 설치해야 했습니다. **Meow 암호가 이를 대체했습니다. 새로 컴파일된
DOCX에는 글꼴이 전혀 필요하지 않습니다.**

`fonts/` 디렉터리는 이전 글꼴 인코딩 DOCX(및 그 모양을 선호하는 사용자)와의
호환성을 위해 유지됩니다. 글꼴을 설치해도 실행에는 영향을 주지 않습니다.
DOCX 텍스트는 여전히 Meow 기호이며, 인터프리터는 글꼴을 무시합니다.

현재 사용자용 설치(Linux, macOS, Termux):

```bash
./fonts/install-fonts.sh --user
```

Android 시스템 전체 설치는 root와 재부팅이 필요합니다:

```bash
su -c 'sh fonts/install-fonts.sh --android-module --yes'
```

릴리스 설치 프로그램은 기본적으로 글꼴을 설치하지 않습니다. 필요하면
`--fonts`를 지정하세요.
자세한 내용(백업 및 제거 동작 포함)은 `fonts/README.md`를 참조하세요.

## Java 버전 빌드 및 실행(선택)

```bash
./gradlew meow
java -jar meow.jar ./demo/HelloWorld.docx
```

또는 자체 실행 스크립트 사용:

```bash
./meow.sh ./demo/HelloWorld.docx
```

## 프로젝트 구조

```text
native/
├── meow.c                 네이티브 인터프리터(ZIP + XML + ANSI + 이미지)
├── Makefile               pkg-config / 대체 빌드
└── CMakeLists.txt         이식 가능한 CMake 빌드
fonts/                     레거시, 선택적 기호 글꼴
├── Meow-*.ttf             U+55B5 -> 읽기 쉬운 내장 이름(이전 인코딩)
├── install-fonts.sh       사용자 / Android 모듈 설치 프로그램
├── README.md              설치 및 재생성 안내
└── NOTICE                 Droid Sans Mono 저작자 표시
tools/
├── meowc.py               Python 스타일 .meow -> .docx 컴파일러
├── meow_cipher.py         Meow 암호 명세(단일 진실 공급원)
├── gen_meow_fonts.py      레거시 Meow-* 기호 글꼴 재생성
└── package-release.sh     릴리스 tarball 생성
examples/
├── hello.meow
├── test1.meow
└── game2048.meow
src/main/java/org/glavo/meow/
├── Main.java              Java 진입점
├── Meow.java              기호(서식 속성을 가진 "Meow")
├── MeowDocxReader.java    경량 DOCX 리더(ZipFile + StAX)
├── MeowRun.java           XWPFRun의 경량 대체품
├── MeowParser.java        파서
├── MeowConsole.java       콘솔 추상화(plain / JLine)
├── JlineConsole.java      지연 로딩 JLine 어댑터
└── value/                 실행 시점 값과 내장 함수
```

## 라이선스

이 저장소의 모든 소스 코드는
[GPLv3](https://www.gnu.org/licenses/gpl-3.0.html)로 배포됩니다.

그 밖의 모든 파일(`.docx`, `.md`, `.png` 등)은
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/)으로
배포됩니다.

`fonts/`에서 생성된 기호 글꼴은 예외입니다. 글리프 윤곽은 Droid Sans Mono에서
파생되었으며
[Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0)으로 배포됩니다.
`fonts/NOTICE`와 `fonts/LICENSE-APACHE-2.0.txt`를 참조하세요.

## 크레딧

- 원래 프로젝트와 언어 설계: [Glavo](https://github.com/Glavo)
- **이 fork(네이티브 구현, 경량 파서, 성능 개선, Meow 암호, 다국어 문서):
  DeepSeek V4.1 Flash**
- [OpenCode](https://opencode.ai) Free API: 이 fork의 개발과 테스트에 사용한
  무료 API 액세스 제공
- [PLCT Lab](https://plctlab.org) 후원(업스트림 프로젝트의 후원자. 이 fork는
  PLCT Lab과 제휴 또는 후원 관계가 아닙니다)

<img alt="OpenCode logo" src="./images/opencode-badge.svg" width="258" height="66">

<img alt="PLCT Lab logo" src="./images/plct-badge.svg" width="200" height="200">

<img alt="IntelliJ IDEA logo" src="./images/intellij-badge.svg" width="300" height="93">

원래 프로젝트는 JetBrains IntelliJ IDEA로 개발되었습니다.
