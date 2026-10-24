#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
喵語密碼（Meow Cipher）：無字型的內建符號編碼。

內建符號不再依賴 Meow-* 自訂字型，改以「喵／嗚組合＋格式」呈現：
  * 文字樣式（僅視覺分類）：
      喵喵 = 輸出入      喵嗚 = 數學／比較
      嗚喵 = 列表／字串  嗚嗚 = 控制／語法
  * 格式維度（解譯器的唯一識別）：
      8 色 × 粗體 × 雙底線 × 黃色高亮 = 64 組合，使用其中 53 個。

本檔是唯一事實來源：tools/meowc.py 直接引用；
native/meow.c 的內建表格必須與本檔產生的 C 表格完全一致。
"""

from __future__ import annotations

# 密碼保留色（同步於 meowc.py 的 RESERVED_COLORS 與 meow.c 的內建表格）
CIPHER_COLORS = [
    0xC00000,  # 深紅
    0xC07000,  # 橙
    0x909000,  # 暗金
    0x008000,  # 綠
    0x008080,  # 青
    0x0070C0,  # 藍
    0x8060C0,  # 紫
    0xC000C0,  # 洋紅
]

# 分類：喵／嗚文字 -> 內建名稱（串接順序即全域編號順序）
CATEGORIES: dict[str, list[str]] = {
    "\u55b5\u55b5": ["print", "readline", "println", "clear"],
    "\u55b5\u55da": [
        "add", "sub", "mul", "div", "mod", "pow", "neg", "abs",
        "min", "max", "sum", "eq", "ne", "lt", "le", "gt", "ge",
        "not", "parseInt", "random", "srand",
    ],
    "\u55da\u55b5": [
        "contains", "list", "cons", "head", "tail", "len", "get",
        "set", "push", "pop", "concat", "slice", "range", "str",
        "textLen", "charAt", "substring", "join",
    ],
    "\u55da\u55da": [
        "let", "lambda", "integer", "if", "while", "for", "do",
        "return", "and", "or",
    ],
}


def _style(index: int, word: str) -> dict:
    """依全域編號展開成 RunSpec 關鍵字參數（格式即身分）。"""
    return {
        "text": word,
        "color": CIPHER_COLORS[index % 8],
        "bold": (index // 8) % 2 == 1,
        "underline_style": "double" if (index // 16) % 2 == 1 else None,
        "highlight": "yellow" if (index // 32) % 2 == 1 else None,
    }


def cipher_styles() -> dict[str, dict]:
    styles: dict[str, dict] = {}
    index = 0
    for word, names in CATEGORIES.items():
        for name in names:
            styles[name] = _style(index, word)
            index += 1
    assert len(styles) == 53, f"喵語密碼需要 53 個內建，目前 {len(styles)}"
    return styles


MEOW_CIPHER_STYLES = cipher_styles()

# native/meow.c 的列舉對應（函數與巨集）
FUNCTION_ENUMS = {
    "print": "BF_PRINT", "readline": "BF_READLINE", "println": "BF_PRINTLN",
    "add": "BF_ADD", "sub": "BF_SUB", "mul": "BF_MUL", "div": "BF_DIV",
    "mod": "BF_MOD", "pow": "BF_POW", "neg": "BF_NEG", "abs": "BF_ABS",
    "min": "BF_MIN", "max": "BF_MAX", "sum": "BF_SUM", "eq": "BF_EQ",
    "ne": "BF_NE", "lt": "BF_LT", "le": "BF_LE", "gt": "BF_GT",
    "ge": "BF_GE", "not": "BF_NOT", "contains": "BF_CONTAINS",
    "list": "BF_LIST", "cons": "BF_CONS", "head": "BF_HEAD",
    "tail": "BF_TAIL", "len": "BF_LEN", "get": "BF_GET", "set": "BF_SET",
    "push": "BF_PUSH", "pop": "BF_POP", "concat": "BF_CONCAT",
    "slice": "BF_SLICE", "range": "BF_RANGE", "str": "BF_STR",
    "parseInt": "BF_PARSE_INT", "textLen": "BF_TEXT_LEN",
    "charAt": "BF_CHAR_AT", "substring": "BF_SUBSTRING",
    "join": "BF_JOIN", "random": "BF_RANDOM", "srand": "BF_SRAND",
    "clear": "BF_CLEAR",
}

MACRO_ENUMS = {
    "let": "BM_LET", "lambda": "BM_LAMBDA", "integer": "BM_INTEGER",
    "if": "BM_IF", "while": "BM_WHILE", "for": "BM_FOR", "do": "BM_DO",
    "return": "BM_RETURN", "and": "BM_AND", "or": "BM_OR",
}


def _c_args(style: dict) -> str:
    color = f"0x{style['color']:06X}"
    bold = "1" if style["bold"] else "0"
    ul = "UL_DOUBLE" if style["underline_style"] else "UL_NONE"
    hl = "HL_YELLOW" if style["highlight"] else "HL_NONE"
    return f"{color}, {bold}, {ul}, {hl}"


def cipher_c_function_lines() -> str:
    """產生 native/meow.c 的 g_functions 表格內容（不含前後括號）。"""
    lines = []
    for word, names in CATEGORIES.items():
        for name in names:
            if name in MACRO_ENUMS:
                continue
            lines.append(
                f'    MEOW_FN("{name}", {_c_args(MEOW_CIPHER_STYLES[name])}, '
                f"{FUNCTION_ENUMS[name]}),"
            )
    return "\n".join(lines)


def cipher_c_macro_lines() -> str:
    """產生 native/meow.c 的 g_macros 表格內容（不含前後括號）。"""
    lines = []
    for word, names in CATEGORIES.items():
        for name in names:
            if name not in MACRO_ENUMS:
                continue
            lines.append(
                f'    MEOW_MC("{name}", {_c_args(MEOW_CIPHER_STYLES[name])}, '
                f"{MACRO_ENUMS[name]}),"
            )
    return "\n".join(lines)
