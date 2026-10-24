#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MeowPy 前端編譯器：把 Python 風格的 .meow 原始碼編譯成 MeowLang 的 .docx。

執行期仍然是 native/meow（C11，不依賴 Java）。編譯後的 DOCX 只包含
由「喵／嗚」組合與粗體、雙底線、顏色構成的密碼符號，原始碼外觀就像加密過的內容。

用法：
    python3 tools/meowc.py program.meow -o program.docx
    python3 tools/meowc.py program.meow --dump-ast
"""

from __future__ import annotations

import argparse
import sys
import zipfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any
from xml.sax.saxutils import escape as xml_escape

from meow_cipher import MEOW_CIPHER_STYLES

# ---------------------------------------------------------------------------
# 內建符號樣式（必須與 native/meow.c 的 register_builtins 完全一致）
# ---------------------------------------------------------------------------

RESERVED_COLORS = {
    0x000000,  # 文字內容
    0x7030A0,  # 文字常值分隔符
    0xC00000,  # 喵語密碼色 1（深紅）
    0xC07000,  # 喵語密碼色 2（橙）
    0x909000,  # 喵語密碼色 3（暗金）
    0x008000,  # 喵語密碼色 4（綠）
    0x008080,  # 喵語密碼色 5（青）
    0x0070C0,  # 喵語密碼色 6（藍）
    0x8060C0,  # 喵語密碼色 7（紫）
    0xC000C0,  # 喵語密碼色 8（洋紅）
}

# name -> RunSpec 關鍵字參數（喵語密碼：格式即身分，文字僅作視覺分類）。
# 內建識別完全靠顏色／粗體／雙底線／高亮的組合，不再依賴任何字型。
BUILTIN_STYLES: dict[str, dict[str, Any]] = MEOW_CIPHER_STYLES

# Python 名稱 -> Meow 執行期名稱
NAME_ALIASES = {
    "print": "println",
    "input": "readline",
    "int": "parseInt",
}

COMPARE_OPS = {"==", "!=", "<", "<=", ">", ">=", "in"}
ARITH_OPS = {"+", "-", "*", "/", "//", "%", "**"}
ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "//=", "%=", "**="}
AUG_TO_BIN = {
    "+=": "+", "-=": "-", "*=": "*", "/=": "/",
    "//=": "//", "%=": "%", "**=": "**",
}

# 優先級：數字越大，綁得越緊
PREC = {
    "or": 1,
    "and": 2,
    "not": 3,
    "==": 4, "!=": 4, "<": 4, "<=": 4, ">": 4, ">=": 4, "in": 4,
    "+": 5, "-": 5,
    "*": 6, "/": 6, "//": 6, "%": 6,
    "unary": 7,
    "**": 8,
}

KEYWORDS = {
    "if", "elif", "else", "while", "for", "in", "def", "return", "pass",
    "and", "or", "not", "True", "False", "None",
}


class CompileError(Exception):
    pass


# ---------------------------------------------------------------------------
# 詞法分析
# ---------------------------------------------------------------------------

@dataclass
class Token:
    kind: str
    value: Any
    line: int
    col: int


def split_logical_lines(source: str) -> list[tuple[str, int]]:
    """去掉註解、把括號內換行接起來，回傳 (邏輯行, 起始行號)。"""
    lines: list[tuple[str, int]] = []
    buf: list[str] = []
    depth = 0
    in_string: str | None = None
    escaped = False
    line_no = 1
    start_line = 1
    i = 0
    n = len(source)

    while i < n:
        c = source[i]

        if in_string is not None:
            buf.append(c)
            if escaped:
                escaped = False
            elif c == "\\":
                escaped = True
            elif c == in_string:
                in_string = None
            if c == "\n":
                line_no += 1
            i += 1
            continue

        if c in ("'", '"'):
            in_string = c
            buf.append(c)
            i += 1
            continue

        if c == "#":
            while i < n and source[i] != "\n":
                i += 1
            continue

        if c in "([{":
            depth += 1
            buf.append(c)
            i += 1
            continue

        if c in ")]}":
            depth = max(0, depth - 1)
            buf.append(c)
            i += 1
            continue

        if c == "\n":
            if depth == 0:
                lines.append(("".join(buf), start_line))
                buf = []
                start_line = line_no + 1
            else:
                buf.append(" ")
            line_no += 1
            i += 1
            continue

        buf.append(c)
        i += 1

    if buf:
        lines.append(("".join(buf), start_line))
    return lines


_MULTI_OPS = ["**=", "//=", "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", "**", "//"]
_SINGLE_OPS = set("+-*/%=<>()[]{}.,:;")


def tokenize_line(text: str, line_no: int) -> list[Token]:
    tokens: list[Token] = []
    i = 0
    n = len(text)

    while i < n:
        c = text[i]

        if c.isspace():
            i += 1
            continue

        if c.isalpha() or c == "_":
            j = i + 1
            while j < n and (text[j].isalnum() or text[j] == "_"):
                j += 1
            tokens.append(Token("NAME", text[i:j], line_no, i + 1))
            i = j
            continue

        if c.isdigit():
            j = i + 1
            while j < n and text[j].isdigit():
                j += 1
            if j < n and (text[j].isalpha() or text[j] == "_"):
                raise CompileError(f"第 {line_no} 行：數字後面不能直接接名稱")
            tokens.append(Token("NUMBER", int(text[i:j]), line_no, i + 1))
            i = j
            continue

        if c in ("'", '"'):
            quote = c
            j = i + 1
            out: list[str] = []
            escaped = False
            closed = False
            while j < n:
                ch = text[j]
                if escaped:
                    mapping = {
                        "n": "\n", "r": "\r", "t": "\t", "\\": "\\",
                        "'": "'", '"': '"', "0": "\0",
                    }
                    out.append(mapping.get(ch, ch))
                    escaped = False
                elif ch == "\\":
                    escaped = True
                elif ch == quote:
                    closed = True
                    j += 1
                    break
                else:
                    out.append(ch)
                j += 1
            if not closed:
                raise CompileError(f"第 {line_no} 行：字串沒有結尾引號")
            tokens.append(Token("STRING", "".join(out), line_no, i + 1))
            i = j
            continue

        matched = None
        for op in _MULTI_OPS:
            if text.startswith(op, i):
                matched = op
                break
        if matched:
            tokens.append(Token("OP", matched, line_no, i + 1))
            i += len(matched)
            continue

        if c in _SINGLE_OPS:
            tokens.append(Token("OP", c, line_no, i + 1))
            i += 1
            continue

        raise CompileError(f"第 {line_no} 行：無法辨識的字元 {c!r}")

    tokens.append(Token("NEWLINE", None, line_no, n + 1))
    return tokens


def lex(source: str) -> list[Token]:
    tokens: list[Token] = []
    indent_stack = [0]
    first_statement = True
    line_no = 1

    for raw, line_no in split_logical_lines(source):
        if not raw.strip():
            continue

        expanded = raw.expandtabs(8)
        stripped = expanded.lstrip(" ")
        indent = len(expanded) - len(stripped)

        if first_statement:
            first_statement = False
            if indent != 0:
                raise CompileError(f"第 {line_no} 行：程式第一行不能縮排")

        if indent > indent_stack[-1]:
            indent_stack.append(indent)
            tokens.append(Token("INDENT", indent, line_no, 1))
        else:
            while indent < indent_stack[-1]:
                indent_stack.pop()
                tokens.append(Token("DEDENT", None, line_no, 1))
            if indent != indent_stack[-1]:
                raise CompileError(f"第 {line_no} 行：縮排不一致")

        tokens.extend(tokenize_line(stripped, line_no))

    while len(indent_stack) > 1:
        indent_stack.pop()
        tokens.append(Token("DEDENT", None, line_no, 1))
    tokens.append(Token("EOF", None, line_no, 1))
    return tokens


# ---------------------------------------------------------------------------
# 語法分析
# ---------------------------------------------------------------------------

class Parser:
    def __init__(self, tokens: list[Token]):
        self.tokens = tokens
        self.i = 0

    def peek(self, offset: int = 0) -> Token:
        return self.tokens[min(self.i + offset, len(self.tokens) - 1)]

    def at(self, kind: str, value: Any = None) -> bool:
        tok = self.peek()
        if tok.kind != kind:
            return False
        return value is None or tok.value == value

    def advance(self) -> Token:
        tok = self.peek()
        if tok.kind != "EOF":
            self.i += 1
        return tok

    def accept(self, kind: str, value: Any = None) -> Token | None:
        if self.at(kind, value):
            return self.advance()
        return None

    def expect(self, kind: str, value: Any = None) -> Token:
        if not self.at(kind, value):
            tok = self.peek()
            want = value if value is not None else kind
            raise CompileError(f"第 {tok.line} 行：預期 {want}，但得到 {tok.value!r}")
        return self.advance()

    def at_compound(self) -> bool:
        return (
            self.at("NAME", "if")
            or self.at("NAME", "while")
            or self.at("NAME", "for")
            or self.at("NAME", "def")
        )

    def parse_module(self) -> list[Any]:
        statements: list[Any] = []
        while not self.at("EOF"):
            if self.at("NEWLINE"):
                self.advance()
                continue
            compound = self.at_compound()
            stmt = self.parse_statement()
            if stmt is not None:
                statements.append(stmt)
            if self.at("NEWLINE"):
                self.advance()
            elif not compound and not self.at("EOF"):
                tok = self.peek()
                raise CompileError(f"第 {tok.line} 行：敘述後面有多餘的內容 {tok.value!r}")
        return statements

    def parse_statement(self) -> Any:
        if self.at("NAME", "if"):
            return self.parse_if()
        if self.at("NAME", "while"):
            return self.parse_while()
        if self.at("NAME", "for"):
            return self.parse_for()
        if self.at("NAME", "def"):
            return self.parse_def()
        if self.at("NAME", "return"):
            return self.parse_return()
        if self.at("NAME", "pass"):
            self.advance()
            return ("pass",)
        return self.parse_simple_statement()

    def parse_simple_statement(self) -> Any:
        expr = self.parse_expression()
        if self.at("OP") and self.peek().value in ASSIGN_OPS:
            op = self.advance().value
            value = self.parse_expression()
            if expr[0] == "name":
                return ("assign", expr[1], value, op)
            if expr[0] == "index":
                return ("index_assign", expr[1], expr[2], value, op)
            raise CompileError("指派目標必須是變數或索引")
        return ("expr", expr)

    def parse_block(self) -> list[Any]:
        self.expect("OP", ":")
        if self.at("NEWLINE"):
            self.advance()
            self.expect("INDENT")
            statements: list[Any] = []
            while not self.at("DEDENT") and not self.at("EOF"):
                if self.at("NEWLINE"):
                    self.advance()
                    continue
                compound = self.at_compound()
                stmt = self.parse_statement()
                if stmt is not None:
                    statements.append(stmt)
                if self.at("NEWLINE"):
                    self.advance()
                elif not compound and not self.at("DEDENT") and not self.at("EOF"):
                    tok = self.peek()
                    raise CompileError(f"第 {tok.line} 行：區塊內有多餘的內容")
            self.expect("DEDENT")
            return statements

        statements = []
        while True:
            statements.append(self.parse_simple_statement())
            if not self.accept("OP", ";"):
                break
        self.expect("NEWLINE")
        return statements

    def parse_if(self) -> Any:
        self.expect("NAME", "if")
        branches = []
        condition = self.parse_expression()
        branches.append((condition, self.parse_block()))
        while self.at("NAME", "elif"):
            self.advance()
            condition = self.parse_expression()
            branches.append((condition, self.parse_block()))
        else_block: list[Any] = []
        if self.at("NAME", "else"):
            self.advance()
            else_block = self.parse_block()
        return ("if", branches, else_block)

    def parse_while(self) -> Any:
        self.expect("NAME", "while")
        condition = self.parse_expression()
        return ("while", condition, self.parse_block())

    def parse_for(self) -> Any:
        self.expect("NAME", "for")
        name = self.expect("NAME").value
        self.expect("NAME", "in")
        iterable = self.parse_expression()
        return ("for", name, iterable, self.parse_block())

    def parse_def(self) -> Any:
        self.expect("NAME", "def")
        name = self.expect("NAME").value
        self.expect("OP", "(")
        params: list[str] = []
        if not self.at("OP", ")"):
            while True:
                params.append(self.expect("NAME").value)
                if not self.accept("OP", ","):
                    break
                if self.at("OP", ")"):
                    break
        self.expect("OP", ")")
        return ("def", name, params, self.parse_block())

    def parse_return(self) -> Any:
        self.expect("NAME", "return")
        if self.at("NEWLINE") or self.at("DEDENT") or self.at("EOF") or self.at("OP", ";"):
            return ("return", None)
        return ("return", self.parse_expression())

    def parse_expression(self, min_prec: int = 0) -> Any:
        left = self.parse_prefix()

        while True:
            tok = self.peek()
            op = tok.value if tok.kind in ("OP", "NAME") else None
            if op not in PREC:
                break
            prec = PREC[op]
            if prec < min_prec:
                break

            self.advance()
            if op == "**":
                right = self.parse_expression(prec)
            else:
                right = self.parse_expression(prec + 1)
            left = ("bin", op, left, right)

        if min_prec == 0 and self.at("NAME", "if"):
            self.advance()
            condition = self.parse_expression()
            self.expect("NAME", "else")
            other = self.parse_expression()
            left = ("cond", left, condition, other)

        return left

    def parse_prefix(self) -> Any:
        if self.at("NAME", "not"):
            self.advance()
            return ("unary", "not", self.parse_expression(PREC["not"]))
        if self.at("OP", "+") or self.at("OP", "-"):
            op = self.advance().value
            return ("unary", op, self.parse_expression(PREC["unary"]))

        node = self.parse_atom()

        while True:
            if self.at("OP", "("):
                node = ("call", node, self.parse_arguments())
            elif self.at("OP", "["):
                node = self.parse_index(node)
            elif self.at("OP", "."):
                self.advance()
                name = self.expect("NAME").value
                if self.at("OP", "("):
                    node = ("method", node, name, self.parse_arguments())
                else:
                    node = ("attr", node, name)
            else:
                break
        return node

    def parse_arguments(self) -> list[Any]:
        self.expect("OP", "(")
        args: list[Any] = []
        if not self.at("OP", ")"):
            while True:
                args.append(self.parse_expression())
                if not self.accept("OP", ","):
                    break
                if self.at("OP", ")"):
                    break
        self.expect("OP", ")")
        return args

    def parse_index(self, target: Any) -> Any:
        self.expect("OP", "[")
        if self.at("OP", ":"):
            start = None
        else:
            start = self.parse_expression()
            if self.at("OP", "]"):
                self.advance()
                return ("index", target, start)
        self.expect("OP", ":")
        end = None if self.at("OP", "]") else self.parse_expression()
        self.expect("OP", "]")
        return ("slice", target, start, end)

    def parse_atom(self) -> Any:
        tok = self.peek()
        if tok.kind == "NUMBER":
            self.advance()
            return ("num", tok.value)
        if tok.kind == "STRING":
            self.advance()
            return ("str", tok.value)
        if tok.kind == "NAME":
            self.advance()
            if tok.value == "True":
                return ("bool", True)
            if tok.value == "False":
                return ("bool", False)
            if tok.value == "None":
                return ("none",)
            return ("name", tok.value)
        if tok.kind == "OP" and tok.value == "(":
            self.advance()
            expr = self.parse_expression()
            self.expect("OP", ")")
            return expr
        if tok.kind == "OP" and tok.value == "[":
            self.advance()
            items: list[Any] = []
            if not self.at("OP", "]"):
                while True:
                    items.append(self.parse_expression())
                    if not self.accept("OP", ","):
                        break
                    if self.at("OP", "]"):
                        break
            self.expect("OP", "]")
            return ("list", items)
        raise CompileError(f"第 {tok.line} 行：預期的運算式，但得到 {tok.value!r}")


# ---------------------------------------------------------------------------
# 收集使用者名稱（用來判斷是否遮蔽內建名稱）
# ---------------------------------------------------------------------------

def collect_user_names(node: Any, names: set[str]) -> None:
    if isinstance(node, list):
        for item in node:
            collect_user_names(item, names)
        return
    if not isinstance(node, tuple):
        return

    tag = node[0]
    if tag == "assign":
        names.add(node[1])
        collect_user_names(node[2], names)
    elif tag == "index_assign":
        collect_user_names(node[1], names)
        collect_user_names(node[2], names)
        collect_user_names(node[3], names)
    elif tag == "def":
        names.add(node[1])
        for param in node[2]:
            names.add(param)
        collect_user_names(node[3], names)
    elif tag == "for":
        names.add(node[1])
        collect_user_names(node[2], names)
        collect_user_names(node[3], names)
    elif tag == "if":
        for condition, block in node[1]:
            collect_user_names(condition, names)
            collect_user_names(block, names)
        collect_user_names(node[2], names)
    elif tag == "while":
        collect_user_names(node[1], names)
        collect_user_names(node[2], names)
    elif tag == "return":
        collect_user_names(node[1], names)
    elif tag == "expr":
        collect_user_names(node[1], names)
    elif tag == "bin":
        collect_user_names(node[2], names)
        collect_user_names(node[3], names)
    elif tag == "unary":
        collect_user_names(node[2], names)
    elif tag == "call":
        collect_user_names(node[1], names)
        collect_user_names(node[2], names)
    elif tag == "method":
        collect_user_names(node[1], names)
        collect_user_names(node[3], names)
    elif tag == "index":
        collect_user_names(node[1], names)
        collect_user_names(node[2], names)
    elif tag == "slice":
        collect_user_names(node[1], names)
        collect_user_names(node[2], names)
        collect_user_names(node[3], names)
    elif tag == "list":
        collect_user_names(node[1], names)
    elif tag == "cond":
        collect_user_names(node[1], names)
        collect_user_names(node[2], names)
        collect_user_names(node[3], names)


# ---------------------------------------------------------------------------
# 產生執行期運算式
# ---------------------------------------------------------------------------

class CodeGen:
    def __init__(self, user_names: set[str]):
        self.user_names = user_names

    def ident(self, name: str) -> tuple[str, str]:
        return ("ident", name)

    def bident(self, name: str) -> tuple[str, str]:
        return ("builtin_ident", name)

    def alias(self, name: str) -> str:
        if name in self.user_names:
            return name
        return NAME_ALIASES.get(name, name)

    def gen_module(self, statements: list[Any]) -> list[Any]:
        result = []
        for stmt in statements:
            expr = self.gen_stmt(stmt)
            if expr is not None:
                result.append(expr)
        return result

    def gen_block(self, statements: list[Any]) -> Any:
        body = []
        for stmt in statements:
            expr = self.gen_stmt(stmt)
            if expr is not None:
                body.append(expr)
        return ("call", self.bident("do"), body)

    def gen_stmt(self, stmt: Any) -> Any:
        tag = stmt[0]

        if tag == "pass":
            return None

        if tag == "expr":
            return self.gen_expr(stmt[1])

        if tag == "assign":
            _, name, value, op = stmt
            if op == "=":
                rhs = self.gen_expr(value)
            else:
                rhs = self.bin_expr(AUG_TO_BIN[op], self.ident(name), self.gen_expr(value))
            return ("call", self.bident("let"), [self.ident(name), rhs])

        if tag == "index_assign":
            _, target, index, value, op = stmt
            obj = self.gen_expr(target)
            idx = self.gen_expr(index)
            if op == "=":
                val = self.gen_expr(value)
            else:
                current = ("call", self.bident("get"), [obj, idx])
                val = self.bin_expr(AUG_TO_BIN[op], current, self.gen_expr(value))
            return ("call", self.bident("set"), [obj, idx, val])

        if tag == "if":
            _, branches, else_block = stmt
            otherwise = self.gen_block(else_block) if else_block else ("unit",)
            for condition, block in reversed(branches):
                otherwise = ("call", self.bident("if"), [
                    self.gen_expr(condition), self.gen_block(block), otherwise,
                ])
            return otherwise

        if tag == "while":
            _, condition, block = stmt
            return ("call", self.bident("while"), [
                self.gen_expr(condition), self.gen_block(block),
            ])

        if tag == "for":
            _, name, iterable, block = stmt
            return ("call", self.bident("for"), [
                self.ident(name), self.gen_expr(iterable), self.gen_block(block),
            ])

        if tag == "def":
            _, name, params, block = stmt
            params_ast = ("ast_list", [self.ident(p) for p in params])
            body = []
            for s in block:
                expr = self.gen_stmt(s)
                if expr is not None:
                    body.append(expr)
            body.append(("call", self.bident("return"), [("unit",)]))
            lambda_expr = ("call", self.bident("lambda"), [params_ast, *body])
            return ("call", self.bident("let"), [self.ident(name), lambda_expr])

        if tag == "return":
            value = ("unit",) if stmt[1] is None else self.gen_expr(stmt[1])
            return ("call", self.bident("return"), [value])

        raise CompileError(f"未知的敘述：{tag}")

    def gen_expr(self, expr: Any) -> Any:
        tag = expr[0]

        if tag == "num":
            return ("int", expr[1])
        if tag == "str":
            return ("text", expr[1])
        if tag == "bool":
            return ("int", 1 if expr[1] else 0)
        if tag == "none":
            return ("unit",)
        if tag == "name":
            return self.ident(self.alias(expr[1]))
        if tag == "list":
            return ("call", self.bident("list"), [self.gen_expr(x) for x in expr[1]])
        if tag == "bin":
            return self.bin_expr(expr[1], self.gen_expr(expr[2]), self.gen_expr(expr[3]))
        if tag == "unary":
            op = expr[1]
            value = self.gen_expr(expr[2])
            if op == "+":
                return value
            if op == "-":
                return ("call", self.bident("neg"), [value])
            if op == "not":
                return ("call", self.bident("not"), [value])
            raise CompileError(f"未知的一元運算子：{op}")
        if tag == "call":
            func = self.gen_expr(expr[1])
            args = [self.gen_expr(x) for x in expr[2]]
            return ("call", func, args)
        if tag == "method":
            obj = self.gen_expr(expr[1])
            name = expr[2]
            args = [self.gen_expr(x) for x in expr[3]]
            if name == "append":
                return ("call", self.bident("push"), [obj, *args])
            if name == "pop":
                return ("call", self.bident("pop"), [obj, *args])
            raise CompileError(f"不支援的方法：{name}()")
        if tag == "index":
            return ("call", self.bident("get"), [self.gen_expr(expr[1]), self.gen_expr(expr[2])])
        if tag == "slice":
            obj = self.gen_expr(expr[1])
            start = ("int", 0) if expr[2] is None else self.gen_expr(expr[2])
            if expr[3] is None:
                return ("call", self.bident("slice"), [obj, start])
            return ("call", self.bident("slice"), [obj, start, self.gen_expr(expr[3])])
        if tag == "cond":
            return ("call", self.bident("if"), [
                self.gen_expr(expr[2]), self.gen_expr(expr[1]), self.gen_expr(expr[3]),
            ])

        raise CompileError(f"未知的運算式：{tag}")

    def bin_expr(self, op: str, left: Any, right: Any) -> Any:
        if op == "in":
            # Python 的 x in y 對應 contains(y, x)
            return ("call", self.bident("contains"), [right, left])
        mapping = {
            "+": "add", "-": "sub", "*": "mul", "/": "div", "//": "div",
            "%": "mod", "**": "pow",
            "==": "eq", "!=": "ne", "<": "lt", "<=": "le", ">": "gt", ">=": "ge",
            "and": "and", "or": "or",
        }
        if op not in mapping:
            raise CompileError(f"未知的運算子：{op}")
        return ("call", self.bident(mapping[op]), [left, right])


# ---------------------------------------------------------------------------
# DOCX 產生
# ---------------------------------------------------------------------------

@dataclass
class RunSpec:
    text: str
    color: int | None = None
    highlight: str | None = None
    italic: bool = False
    underline: bool = False
    bold: bool = False
    underline_style: str | None = None
    font: str | None = None
    size: int | None = None


def hsv_to_rgb(h: float, s: float, v: float) -> int:
    i = int(h * 6.0) % 6
    f = h * 6.0 - int(h * 6.0)
    p = int(255 * v * (1.0 - s))
    q = int(255 * v * (1.0 - s * f))
    t = int(255 * v * (1.0 - s * (1.0 - f)))
    vv = int(255 * v)
    if i == 0:
        r, g, b = vv, t, p
    elif i == 1:
        r, g, b = q, vv, p
    elif i == 2:
        r, g, b = p, vv, t
    elif i == 3:
        r, g, b = p, q, vv
    elif i == 4:
        r, g, b = t, p, vv
    else:
        r, g, b = vv, p, q
    return (r << 16) | (g << 8) | b


class Encoder:
    def __init__(self):
        self.var_colors: dict[str, int] = {}
        self.used_colors = set(RESERVED_COLORS)
        self.candidate = 0

    def color_for(self, name: str) -> int:
        if name in self.var_colors:
            return self.var_colors[name]
        while True:
            index = self.candidate
            self.candidate += 1
            h = (index * 0.618033988749895 + 0.11) % 1.0
            s = 0.58 + 0.12 * (index % 4)
            v = 0.72 + 0.06 * (index % 3)
            color = hsv_to_rgb(h, s, v)
            if color not in self.used_colors and (color & 0xFFFFFF) != 0:
                break
        self.used_colors.add(color)
        self.var_colors[name] = color
        return color

    def style_for(self, name: str, user_names: set[str]) -> RunSpec:
        if name in user_names or name not in BUILTIN_STYLES:
            return RunSpec("\u55b5", color=self.color_for(name))
        return RunSpec(**BUILTIN_STYLES[name])

    def group(self, items: list[list[RunSpec]], depth: int) -> list[RunSpec]:
        color = ((depth + 1) * 0x010101) & 0xFFFFFF
        open_token = RunSpec("\u55b5", color=color, italic=True)
        close_token = RunSpec("\u55b5", color=color, italic=True)
        result = [open_token]
        for index, item in enumerate(items):
            if index > 0:
                result.append(RunSpec(" "))
            result.extend(item)
        result.append(close_token)
        return result

    def text_literal(self, value: str) -> list[RunSpec]:
        open_token = RunSpec("\u55b5", color=0x7030A0, underline=True)
        close_token = RunSpec("\u55b5", color=0x7030A0, underline=True)
        result = [open_token]
        if value:
            result.append(RunSpec(value, color=0x000000))
        result.append(close_token)
        return result

    def emit_item(self, expr: Any, depth: int, user_names: set[str]) -> list[RunSpec]:
        tag = expr[0]

        if tag == "ident":
            return [self.style_for(expr[1], user_names)]
        if tag == "builtin_ident":
            return [RunSpec(**BUILTIN_STYLES[expr[1]])]
        if tag == "text":
            return self.text_literal(expr[1])
        if tag == "int":
            return self.group([
                self.emit_item(("builtin_ident", "parseInt"), depth + 1, user_names),
                self.text_literal(str(expr[1])),
            ], depth)
        if tag == "unit":
            return self.group([], depth)
        if tag == "ast_list":
            return self.group([
                self.emit_item(item, depth + 1, user_names) for item in expr[1]
            ], depth)
        if tag == "call":
            func = expr[1]
            args = expr[2]
            items = [self.emit_item(func, depth + 1, user_names)]
            items.extend(self.emit_item(arg, depth + 1, user_names) for arg in args)
            return self.group(items, depth)

        raise CompileError(f"無法編碼的執行期運算式：{tag}")


def text_content_xml(text: str) -> str:
    if text == "":
        return ""
    parts = []
    for piece in _split_keep(text, "\n\t"):
        if piece == "\n":
            parts.append("<w:br/>")
        elif piece == "\t":
            parts.append("<w:tab/>")
        elif piece:
            parts.append(f'<w:t xml:space="preserve">{xml_escape(piece)}</w:t>')
    return "".join(parts)


def _split_keep(text: str, separators: str) -> list[str]:
    result = []
    current = []
    for ch in text:
        if ch in separators:
            if current:
                result.append("".join(current))
                current = []
            result.append(ch)
        else:
            current.append(ch)
    if current:
        result.append("".join(current))
    return result


def run_xml(run: RunSpec) -> str:
    props = []
    if run.font:
        f = xml_escape(run.font, {'"': "&quot;"})
        props.append(f'<w:rFonts w:ascii="{f}" w:hAnsi="{f}"/>')
    if run.bold:
        props.append("<w:b/>")
    if run.italic:
        props.append("<w:i/>")
    if run.underline_style:
        props.append(f'<w:u w:val="{run.underline_style}"/>')
    elif run.underline:
        props.append('<w:u w:val="single"/>')
    if run.color is not None:
        props.append(f'<w:color w:val="{run.color:06X}"/>')
    if run.size is not None:
        props.append(f'<w:sz w:val="{run.size}"/><w:szCs w:val="{run.size}"/>')
    if run.highlight:
        props.append(f'<w:highlight w:val="{run.highlight}"/>')

    rpr = f'<w:rPr>{"".join(props)}</w:rPr>' if props else ""
    content = text_content_xml(run.text)
    return f"<w:r>{rpr}{content}</w:r>"


def paragraph_xml(runs: list[RunSpec]) -> str:
    return "<w:p>" + "".join(run_xml(run) for run in runs) + "</w:p>"


CONTENT_TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>"""

ROOT_RELS = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>"""


def write_docx(path: Path, paragraphs: list[str]) -> None:
    document = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
        '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">'
        "<w:body>"
        + "".join(paragraphs)
        + "<w:sectPr/>"
        "</w:body></w:document>"
    )

    path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        archive.writestr("[Content_Types].xml", CONTENT_TYPES)
        archive.writestr("_rels/.rels", ROOT_RELS)
        archive.writestr("word/document.xml", document)


# ---------------------------------------------------------------------------
# 命令列
# ---------------------------------------------------------------------------

def compile_source(source: str) -> tuple[list[Any], list[Any], list[str]]:
    tokens = lex(source)
    parser = Parser(tokens)
    module = parser.parse_module()

    user_names: set[str] = set()
    collect_user_names(module, user_names)

    codegen = CodeGen(user_names)
    runtime_exprs = codegen.gen_module(module)

    encoder = Encoder()
    paragraphs = [
        paragraph_xml(encoder.emit_item(expr, 0, user_names))
        for expr in runtime_exprs
    ]
    return module, runtime_exprs, paragraphs


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="把 Python 風格的 .meow 編譯成 MeowLang 的 .docx",
    )
    parser.add_argument("source", type=Path, help="輸入的 .meow 檔案")
    parser.add_argument("-o", "--output", type=Path, help="輸出的 .docx 檔案")
    parser.add_argument("--dump-ast", action="store_true", help="印出 Python 風格 AST")
    parser.add_argument("--dump-runtime", action="store_true", help="印出執行期運算式")
    parser.add_argument("--version", action="version", version="meowc 1.2")
    args = parser.parse_args(argv)

    try:
        source = args.source.read_text(encoding="utf-8")
        module, runtime_exprs, paragraphs = compile_source(source)
    except FileNotFoundError:
        print(f"meowc: 找不到檔案：{args.source}", file=sys.stderr)
        return 1
    except CompileError as error:
        print(f"meowc: {error}", file=sys.stderr)
        return 1

    if args.dump_ast:
        for stmt in module:
            print(stmt)
    if args.dump_runtime:
        for expr in runtime_exprs:
            print(expr)

    output = args.output
    if output is None:
        output = args.source.with_suffix(".docx")
    write_docx(output, paragraphs)
    print(f"已產生 {output}（{len(paragraphs)} 個敘述）")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
