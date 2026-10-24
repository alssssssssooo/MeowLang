/*
 * Meow Language - native implementation
 *
 * Copyright (C) 2025 Glavo (original Java implementation)
 * Native rewrite, lightweight DOCX reader and performance work:
 *   DeepSeek V4.1 Flash (2026)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <expat.h>
#include <jpeglib.h>
#include <png.h>
#include <zlib.h>

/* ------------------------------------------------------------------ */
/* 基本工具                                                            */
/* ------------------------------------------------------------------ */

static void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fputs("meow: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

static void *xmalloc(size_t n) {
    void *p = malloc(n ? n : 1);
    if (!p) {
        die("out of memory");
    }
    return p;
}

static void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n ? n : 1);
    if (!q) {
        die("out of memory");
    }
    return q;
}

static char *xstrdup(const char *s) {
    if (!s) {
        return NULL;
    }
    size_t n = strlen(s);
    char *p = xmalloc(n + 1);
    memcpy(p, s, n + 1);
    return p;
}

static char *xstrndup(const char *s, size_t n) {
    char *p = xmalloc(n + 1);
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

static int ascii_ieq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char) *a) != tolower((unsigned char) *b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

/* ------------------------------------------------------------------ */
/* 動態緩衝區                                                          */
/* ------------------------------------------------------------------ */

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Buf;

static void buf_reserve(Buf *b, size_t extra) {
    if (b->len + extra + 1 > b->cap) {
        size_t cap = b->cap ? b->cap : 32;
        while (cap < b->len + extra + 1) {
            cap *= 2;
        }
        b->data = xrealloc(b->data, cap);
        b->cap = cap;
    }
}

static void buf_putc(Buf *b, char c) {
    buf_reserve(b, 1);
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
}

static void buf_write(Buf *b, const char *s, size_t n) {
    buf_reserve(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void buf_puts(Buf *b, const char *s) {
    buf_write(b, s, strlen(s));
}

static void buf_printf(Buf *b, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return;
    }
    buf_reserve(b, (size_t) n);
    vsnprintf(b->data + b->len, (size_t) n + 1, fmt, ap2);
    va_end(ap2);
    b->len += (size_t) n;
}

static void buf_free(Buf *b) {
    free(b->data);
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}

/* ------------------------------------------------------------------ */
/* ZIP 讀取（中央目錄 + zlib inflate）                                 */
/* ------------------------------------------------------------------ */

typedef struct {
    char *name;
    uint16_t method;
    uint32_t comp_size;
    uint32_t uncomp_size;
    uint32_t local_offset;
} ZipEntry;

typedef struct {
    unsigned char *data;
    size_t size;
    ZipEntry *entries;
    int count;
} Zip;

static uint16_t rd16(const unsigned char *p) {
    return (uint16_t) (p[0] | (p[1] << 8));
}

static uint32_t rd32(const unsigned char *p) {
    return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
}

static int zip_open(Zip *z, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    long file_size = ftell(f);
    if (file_size < 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    z->data = xmalloc((size_t) file_size);
    z->size = (size_t) file_size;
    if (fread(z->data, 1, z->size, f) != z->size) {
        fclose(f);
        return 0;
    }
    fclose(f);

    if (z->size < 22) {
        return 0;
    }

    size_t start = z->size > 65557 ? z->size - 65557 : 0;
    size_t eocd = 0;
    int found = 0;
    for (size_t i = z->size - 22 + 1; i-- > start;) {
        if (rd32(z->data + i) == 0x06054b50) {
            eocd = i;
            found = 1;
            break;
        }
    }
    if (!found) {
        return 0;
    }

    uint16_t total = rd16(z->data + eocd + 10);
    uint32_t offset = rd32(z->data + eocd + 16);

    z->entries = xmalloc(sizeof(ZipEntry) * (total ? total : 1));
    z->count = 0;

    size_t p = offset;
    for (uint16_t i = 0; i < total; i++) {
        if (p + 46 > z->size || rd32(z->data + p) != 0x02014b50) {
            break;
        }
        uint16_t name_len = rd16(z->data + p + 28);
        uint16_t extra_len = rd16(z->data + p + 30);
        uint16_t comment_len = rd16(z->data + p + 32);

        ZipEntry *e = &z->entries[z->count++];
        e->method = rd16(z->data + p + 10);
        e->comp_size = rd32(z->data + p + 20);
        e->uncomp_size = rd32(z->data + p + 24);
        e->local_offset = rd32(z->data + p + 42);
        e->name = xstrndup((const char *) z->data + p + 46, name_len);

        p += 46 + name_len + extra_len + comment_len;
    }

    return 1;
}

static ZipEntry *zip_find(Zip *z, const char *name) {
    for (int i = 0; i < z->count; i++) {
        if (strcmp(z->entries[i].name, name) == 0) {
            return &z->entries[i];
        }
    }
    return NULL;
}

static unsigned char *zip_extract(Zip *z, ZipEntry *e, size_t *out_len) {
    if (e->local_offset + 30 > z->size) {
        return NULL;
    }
    const unsigned char *p = z->data + e->local_offset;
    if (rd32(p) != 0x04034b50) {
        return NULL;
    }
    uint16_t name_len = rd16(p + 26);
    uint16_t extra_len = rd16(p + 28);
    const unsigned char *src = p + 30 + name_len + extra_len;
    if ((size_t) (src - z->data) + e->comp_size > z->size) {
        return NULL;
    }

    if (e->method == 0) {
        unsigned char *out = xmalloc(e->comp_size);
        memcpy(out, src, e->comp_size);
        *out_len = e->comp_size;
        return out;
    }

    if (e->method != 8) {
        return NULL;
    }

    unsigned char *out = xmalloc(e->uncomp_size ? e->uncomp_size : 1);
    z_stream s;
    memset(&s, 0, sizeof(s));
    if (inflateInit2(&s, -MAX_WBITS) != Z_OK) {
        return NULL;
    }
    s.next_in = (Bytef *) src;
    s.avail_in = e->comp_size;
    s.next_out = out;
    s.avail_out = e->uncomp_size;
    int r = inflate(&s, Z_FINISH);
    inflateEnd(&s);
    if (r != Z_STREAM_END) {
        return NULL;
    }
    *out_len = e->uncomp_size;
    return out;
}

/* ------------------------------------------------------------------ */
/* 資料模型                                                            */
/* ------------------------------------------------------------------ */

typedef enum {
    UL_NONE,
    UL_SINGLE,
    UL_DOUBLE,
    UL_THICK,
    UL_DOTTED,
    UL_DASH,
    UL_DOT_DASH,
    UL_DOT_DOT_DASH,
    UL_WAVE,
    UL_OTHER
} Underline;

typedef enum {
    HL_NONE,
    HL_BLACK,
    HL_BLUE,
    HL_CYAN,
    HL_GREEN,
    HL_MAGENTA,
    HL_RED,
    HL_YELLOW,
    HL_WHITE,
    HL_LIGHT_GRAY,
    HL_DARK_GRAY
} Highlight;

typedef struct {
    unsigned char *data;
    size_t len;
} Blob;

typedef struct {
    char *text;
    int italic;
    int bold;
    Underline underline;
    int color;          /* -1 表示未設定 */
    Highlight highlight;
    char *font;
    int size_half;      /* -1 表示未設定，否則為半點單位 */
    Blob *pictures;
    int picture_count;
} Run;

typedef struct {
    Run **runs;
    int count;
} Paragraph;

typedef struct {
    Paragraph *paragraphs;
    int count;
} Document;

typedef struct {
    int italic;         /* 斜體 = 括號 */
    Underline underline;
    int color;          /* 0 表示預設 */
    Highlight highlight;
    int bold;
    const char *font;
    int size_half;      /* -1 表示未設定 */
} Meow;

/* ------------------------------------------------------------------ */
/* 詞彙表                                                              */
/* ------------------------------------------------------------------ */

static const char *const MEOW_WORDS[] = {
    "meow", "miaou", "miao", "nyan",
    "にゃん", "야옹", "喵", "喵呜", "喵喵", "喵嗚", "嗚喵", "嗚嗚",
    "мяу", "miyav", "miau", "miauw", "μιάου", "mjau", "nyávog", "म्याऊं", "meong", "mjá", "meo meo",
    "mjäu", "mjav", "мјау", "mjal", "მიაუ", "myau", "ngiyavuma", "ማው", "maw",
    NULL
};

/*
 * 簡單的 Unicode 小寫轉換。只實作 Meow 詞彙表實際需要用到的字集：
 * ASCII、Latin-1、希臘字母與西里爾字母。
 * 這樣不需要額外依賴 ICU，也能正確辨識 Μιάου、Мяу 等大寫開頭的單字。
 */
static unsigned int simple_lower(unsigned int cp) {
    if (cp >= 'A' && cp <= 'Z') {
        return cp + 32;
    }
    if (cp >= 0x00C0 && cp <= 0x00DE && cp != 0x00D7) {
        return cp + 32;
    }
    if (cp == 0x0386) return 0x03AC;
    if (cp == 0x0388) return 0x03AD;
    if (cp == 0x0389) return 0x03AE;
    if (cp == 0x038A) return 0x03AF;
    if (cp == 0x038C) return 0x03CC;
    if (cp == 0x038E) return 0x03CD;
    if (cp == 0x038F) return 0x03CE;
    if (cp >= 0x0391 && cp <= 0x03A9 && cp != 0x03A2) {
        return cp + 32;
    }
    if (cp >= 0x0400 && cp <= 0x040F) {
        return cp + 80;
    }
    if (cp >= 0x0410 && cp <= 0x042F) {
        return cp + 32;
    }
    if (cp >= 0x0460 && cp <= 0x0481 && (cp % 2) == 0) {
        return cp + 1;
    }
    if (cp >= 0x048A && cp <= 0x04BF && (cp % 2) == 0) {
        return cp + 1;
    }
    return cp;
}

static int utf8_decode(const unsigned char *s, unsigned int *out, int *length) {
    unsigned char c = s[0];
    if (c < 0x80) {
        *out = c;
        *length = 1;
        return 1;
    }
    if ((c & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80) {
        *out = ((unsigned int) (c & 0x1F) << 6) | (s[1] & 0x3F);
        *length = 2;
        return 1;
    }
    if ((c & 0xF0) == 0xE0 && (s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
        *out = ((unsigned int) (c & 0x0F) << 12) |
               ((unsigned int) (s[1] & 0x3F) << 6) |
               (s[2] & 0x3F);
        *length = 3;
        return 1;
    }
    if ((c & 0xF8) == 0xF0 && (s[1] & 0xC0) == 0x80 &&
        (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
        *out = ((unsigned int) (c & 0x07) << 18) |
               ((unsigned int) (s[1] & 0x3F) << 12) |
               ((unsigned int) (s[2] & 0x3F) << 6) |
               (s[3] & 0x3F);
        *length = 4;
        return 1;
    }
    *out = c;
    *length = 1;
    return 0;
}

static void utf8_encode(Buf *b, unsigned int cp) {
    if (cp < 0x80) {
        buf_putc(b, (char) cp);
    } else if (cp < 0x800) {
        buf_putc(b, (char) (0xC0 | (cp >> 6)));
        buf_putc(b, (char) (0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        buf_putc(b, (char) (0xE0 | (cp >> 12)));
        buf_putc(b, (char) (0x80 | ((cp >> 6) & 0x3F)));
        buf_putc(b, (char) (0x80 | (cp & 0x3F)));
    } else {
        buf_putc(b, (char) (0xF0 | (cp >> 18)));
        buf_putc(b, (char) (0x80 | ((cp >> 12) & 0x3F)));
        buf_putc(b, (char) (0x80 | ((cp >> 6) & 0x3F)));
        buf_putc(b, (char) (0x80 | (cp & 0x3F)));
    }
}

static char *utf8_lower(const char *s) {
    Buf b = {0};
    const unsigned char *p = (const unsigned char *) s;
    while (*p) {
        unsigned int cp;
        int len;
        utf8_decode(p, &cp, &len);
        utf8_encode(&b, simple_lower(cp));
        p += len;
    }
    return b.data ? b.data : xstrdup("");
}

static int is_meow_word(const char *s) {
    for (int i = 0; MEOW_WORDS[i]; i++) {
        if (strcmp(s, MEOW_WORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int meow_equal(const Meow *a, const Meow *b) {
    if (a->italic != b->italic || a->underline != b->underline || a->color != b->color ||
        a->highlight != b->highlight || a->bold != b->bold || a->size_half != b->size_half) {
        return 0;
    }
    if (!a->font || !b->font) {
        return a->font == b->font;
    }
    return strcmp(a->font, b->font) == 0;
}

static int meow_of(const Run *run, Meow *out) {
    if (!run || run->picture_count > 0 || !run->text) {
        return 0;
    }

    const char *s = run->text;
    while (*s && isspace((unsigned char) *s)) {
        s++;
    }
    const char *end = s + strlen(s);
    while (end > s && isspace((unsigned char) end[-1])) {
        end--;
    }

    char *trimmed = xstrndup(s, (size_t) (end - s));
    char *lower = utf8_lower(trimmed);
    int ok = is_meow_word(lower);
    free(trimmed);
    free(lower);

    if (!ok) {
        return 0;
    }

    out->italic = run->italic;
    out->underline = run->underline;
    out->color = run->color < 0 ? 0 : run->color;
    out->highlight = run->highlight;
    out->bold = run->bold;
    out->font = run->font;
    out->size_half = run->size_half;
    return 1;
}

/* ------------------------------------------------------------------ */
/* 運算式                                                              */
/* ------------------------------------------------------------------ */

typedef enum {
    EX_IDENT,
    EX_LIST,
    EX_RICH
} ExprKind;

typedef struct Expr Expr;

struct Expr {
    ExprKind kind;
    union {
        Meow ident;
        struct {
            Expr **items;
            int count;
        } list;
        struct {
            Run **runs;
            int count;
        } rich;
    } u;
};

/* ------------------------------------------------------------------ */
/* 執行期數值                                                          */
/* ------------------------------------------------------------------ */

typedef enum {
    VAL_UNIT,
    VAL_INTEGER,
    VAL_TEXT,
    VAL_LIST,
    VAL_FUNCTION,
    VAL_MACRO,
    VAL_LAMBDA
} ValueKind;

typedef enum {
    BF_PRINT,
    BF_READLINE,
    BF_PRINTLN,
    BF_ADD,
    BF_SUB,
    BF_MUL,
    BF_DIV,
    BF_MOD,
    BF_POW,
    BF_NEG,
    BF_ABS,
    BF_MIN,
    BF_MAX,
    BF_SUM,
    BF_EQ,
    BF_NE,
    BF_LT,
    BF_LE,
    BF_GT,
    BF_GE,
    BF_NOT,
    BF_CONTAINS,
    BF_LIST,
    BF_CONS,
    BF_HEAD,
    BF_TAIL,
    BF_LEN,
    BF_GET,
    BF_SET,
    BF_PUSH,
    BF_POP,
    BF_CONCAT,
    BF_SLICE,
    BF_RANGE,
    BF_STR,
    BF_PARSE_INT,
    BF_TEXT_LEN,
    BF_CHAR_AT,
    BF_SUBSTRING,
    BF_JOIN,
    BF_RANDOM,
    BF_SRAND,
    BF_CLEAR
} BuiltinFunction;

typedef enum {
    BM_LET,
    BM_LAMBDA,
    BM_INTEGER,
    BM_IF,
    BM_WHILE,
    BM_FOR,
    BM_DO,
    BM_RETURN,
    BM_AND,
    BM_OR
} BuiltinMacro;

typedef struct Value Value;
typedef struct Context Context;

struct Value {
    ValueKind kind;
    union {
        long integer;
        struct {
            Run **runs;
            int count;
        } text;
        struct {
            Value **items;
            int count;
        } list;
        BuiltinFunction function;
        BuiltinMacro macro;
        struct {
            Context *declaration;
            Meow *parameters;
            int parameter_count;
            Expr **body;
            int body_count;
        } lambda;
    } u;
};

typedef struct Binding {
    Meow key;
    Value *value;
    struct Binding *next;
} Binding;

struct Context {
    Context *parent;
    Binding *bindings;
};

static int g_debug = 0;

/* return 巨集使用的長跳躍框架；每個 lambda 呼叫推入一層。 */
typedef struct ReturnFrame {
    jmp_buf buf;
    Value *value;
    struct ReturnFrame *prev;
} ReturnFrame;

static ReturnFrame *g_return_frame = NULL;
static Value *g_return_value = NULL;

static Value *val_unit(void);
static Value *val_integer(long v);
static Value *val_text(Run **runs, int count);
static Value *val_text_cstr(const char *s);
static Value *val_list(Value **items, int count);
static Value *val_function(BuiltinFunction f);
static Value *val_macro(BuiltinMacro m);
static Value *val_lambda(Context *decl, Meow *params, int np, Expr **body, int nb);
static int value_truthy(Value *v);
static int value_equal(Value *a, Value *b);

static Value UNIT_VALUE = {VAL_UNIT, {0}};

static Meow meow_builtin_style(int color, int bold, Underline ul, Highlight hl) {
    Meow m;
    memset(&m, 0, sizeof(m));
    m.italic = 0;
    m.underline = ul;
    m.color = color;
    m.highlight = hl;
    m.bold = bold;
    m.font = NULL;   /* 喵語密碼：內建符號不再依賴字型 */
    m.size_half = -1;
    return m;
}

/* ------------------------------------------------------------------ */
/* Context                                                             */
/* ------------------------------------------------------------------ */

static Context *ctx_new(Context *parent) {
    Context *c = xmalloc(sizeof(Context));
    c->parent = parent;
    c->bindings = NULL;
    return c;
}

static void ctx_set(Context *c, Meow key, Value *v) {
    for (Binding *b = c->bindings; b; b = b->next) {
        if (meow_equal(&b->key, &key)) {
            b->value = v;
            return;
        }
    }
    Binding *b = xmalloc(sizeof(Binding));
    b->key = key;
    b->value = v;
    b->next = c->bindings;
    c->bindings = b;
}

static Value *ctx_get(Context *c, Meow key) {
    for (; c; c = c->parent) {
        for (Binding *b = c->bindings; b; b = b->next) {
            if (meow_equal(&b->key, &key)) {
                return b->value;
            }
        }
    }
    return &UNIT_VALUE;
}

/* ------------------------------------------------------------------ */
/* 解析器                                                              */
/* ------------------------------------------------------------------ */

static int find_end(Run **runs, int count, Meow start, int from) {
    int end = from + 1;
    while (end < count) {
        Meow m;
        if (meow_of(runs[end], &m) && meow_equal(&m, &start)) {
            return end;
        }
        end++;
    }
    return end;
}

static Expr *expr_new(ExprKind kind) {
    Expr *e = xmalloc(sizeof(Expr));
    memset(e, 0, sizeof(*e));
    e->kind = kind;
    return e;
}

static Expr *parse_run_list(Run **runs, int count, int top) {
    Expr **items = NULL;
    int item_count = 0;
    int item_cap = 0;

    for (int i = 0; i < count; i++) {
        Meow m;
        if (!meow_of(runs[i], &m)) {
            continue;
        }

        if (m.italic || m.underline == UL_SINGLE) {
            int end = find_end(runs, count, m, i);
            int content_count = end - (i + 1);
            if (content_count < 0) {
                content_count = 0;
            }

            if (m.italic) {
                Expr *e = parse_run_list(runs + i + 1, content_count, 0);
                if (item_count == item_cap) {
                    item_cap = item_cap ? item_cap * 2 : 8;
                    items = xrealloc(items, sizeof(Expr *) * (size_t) item_cap);
                }
                items[item_count++] = e;
            } else if (m.underline == UL_SINGLE) {
                Expr *e = expr_new(EX_RICH);
                e->u.rich.runs = runs + i + 1;
                e->u.rich.count = content_count;
                if (item_count == item_cap) {
                    item_cap = item_cap ? item_cap * 2 : 8;
                    items = xrealloc(items, sizeof(Expr *) * (size_t) item_cap);
                }
                items[item_count++] = e;
            }

            i = end;
        } else {
            Expr *e = expr_new(EX_IDENT);
            e->u.ident = m;
            if (item_count == item_cap) {
                item_cap = item_cap ? item_cap * 2 : 8;
                items = xrealloc(items, sizeof(Expr *) * (size_t) item_cap);
            }
            items[item_count++] = e;
        }
    }

    if (top && item_count == 1) {
        return items[0];
    }

    Expr *e = expr_new(EX_LIST);
    e->u.list.items = items;
    e->u.list.count = item_count;
    return e;
}

/* ------------------------------------------------------------------ */
/* 數值建構                                                            */
/* ------------------------------------------------------------------ */

static Value *val_integer(long v) {
    Value *x = xmalloc(sizeof(Value));
    memset(x, 0, sizeof(*x));
    x->kind = VAL_INTEGER;
    x->u.integer = v;
    return x;
}

static Value *val_text(Run **runs, int count) {
    Value *x = xmalloc(sizeof(Value));
    memset(x, 0, sizeof(*x));
    x->kind = VAL_TEXT;
    x->u.text.runs = runs;
    x->u.text.count = count;
    return x;
}

static Run *run_from_cstr(const char *s) {
    Run *r = xmalloc(sizeof(Run));
    memset(r, 0, sizeof(*r));
    r->text = xstrdup(s ? s : "");
    r->color = -1;
    r->size_half = -1;
    return r;
}

static Value *val_text_cstr(const char *s) {
    Run **runs = xmalloc(sizeof(Run *));
    runs[0] = run_from_cstr(s);
    return val_text(runs, 1);
}

static Value *val_list(Value **items, int count) {
    Value *x = xmalloc(sizeof(Value));
    memset(x, 0, sizeof(*x));
    x->kind = VAL_LIST;
    x->u.list.count = count;
    x->u.list.items = xmalloc(sizeof(Value *) * (size_t) (count ? count : 1));
    if (count > 0) {
        memcpy(x->u.list.items, items, sizeof(Value *) * (size_t) count);
    }
    return x;
}

static Value *val_function(BuiltinFunction f) {
    Value *x = xmalloc(sizeof(Value));
    memset(x, 0, sizeof(*x));
    x->kind = VAL_FUNCTION;
    x->u.function = f;
    return x;
}

static Value *val_macro(BuiltinMacro m) {
    Value *x = xmalloc(sizeof(Value));
    memset(x, 0, sizeof(*x));
    x->kind = VAL_MACRO;
    x->u.macro = m;
    return x;
}

static Value *val_lambda(Context *decl, Meow *params, int np, Expr **body, int nb) {
    Value *x = xmalloc(sizeof(Value));
    memset(x, 0, sizeof(*x));
    x->kind = VAL_LAMBDA;
    x->u.lambda.declaration = decl;
    x->u.lambda.parameters = params;
    x->u.lambda.parameter_count = np;
    x->u.lambda.body = body;
    x->u.lambda.body_count = nb;
    return x;
}

static Value *val_unit(void) {
    return &UNIT_VALUE;
}

/* ------------------------------------------------------------------ */
/* 顯示字串                                                            */
/* ------------------------------------------------------------------ */

static char *text_to_string(Run **runs, int count) {
    Buf b = {0};
    for (int i = 0; i < count; i++) {
        if (runs[i]->text) {
            buf_puts(&b, runs[i]->text);
        }
    }
    return b.data ? b.data : xstrdup("");
}

static char *value_to_string(Value *v);

static void append_text_quoted(Buf *b, const char *s) {
    buf_putc(b, '"');
    for (const unsigned char *p = (const unsigned char *) s; *p; p++) {
        switch (*p) {
            case '\\': buf_puts(b, "\\\\"); break;
            case '"':  buf_puts(b, "\\\""); break;
            case '\n': buf_puts(b, "\\n"); break;
            case '\r': buf_puts(b, "\\r"); break;
            case '\t': buf_puts(b, "\\t"); break;
            default:   buf_putc(b, (char) *p); break;
        }
    }
    buf_putc(b, '"');
}

static void append_value_repr(Buf *b, Value *v, int depth) {
    if (depth > 32) {
        buf_puts(b, "...");
        return;
    }

    switch (v->kind) {
        case VAL_UNIT:
            buf_puts(b, "None");
            break;
        case VAL_INTEGER:
            buf_printf(b, "%ld", v->u.integer);
            break;
        case VAL_TEXT: {
            char *s = text_to_string(v->u.text.runs, v->u.text.count);
            if (depth == 0) {
                buf_puts(b, s);
            } else {
                append_text_quoted(b, s);
            }
            free(s);
            break;
        }
        case VAL_LIST:
            buf_putc(b, '[');
            for (int i = 0; i < v->u.list.count; i++) {
                if (i > 0) buf_puts(b, ", ");
                append_value_repr(b, v->u.list.items[i], depth + 1);
            }
            buf_putc(b, ']');
            break;
        case VAL_FUNCTION:
            buf_puts(b, "<function>");
            break;
        case VAL_MACRO:
            buf_puts(b, "<macro>");
            break;
        case VAL_LAMBDA:
            buf_puts(b, "<lambda>");
            break;
    }
}

static char *value_to_string(Value *v) {
    Buf b = {0};
    append_value_repr(&b, v, 0);
    return b.data ? b.data : xstrdup("");
}

static int text_equal(Value *a, Value *b) {
    char *sa = text_to_string(a->u.text.runs, a->u.text.count);
    char *sb = text_to_string(b->u.text.runs, b->u.text.count);
    int eq = strcmp(sa, sb) == 0;
    free(sa);
    free(sb);
    return eq;
}

static int value_truthy(Value *v) {
    switch (v->kind) {
        case VAL_UNIT:
            return 0;
        case VAL_INTEGER:
            return v->u.integer != 0;
        case VAL_TEXT: {
            char *s = text_to_string(v->u.text.runs, v->u.text.count);
            int truthy = s[0] != '\0';
            free(s);
            return truthy;
        }
        case VAL_LIST:
            return v->u.list.count != 0;
        case VAL_FUNCTION:
        case VAL_MACRO:
        case VAL_LAMBDA:
            return 1;
    }
    return 0;
}

static int value_equal(Value *a, Value *b) {
    if (a == b) {
        return 1;
    }
    if (!a || !b || a->kind != b->kind) {
        return 0;
    }

    switch (a->kind) {
        case VAL_UNIT:
            return 1;
        case VAL_INTEGER:
            return a->u.integer == b->u.integer;
        case VAL_TEXT:
            return text_equal(a, b);
        case VAL_LIST:
            if (a->u.list.count != b->u.list.count) {
                return 0;
            }
            for (int i = 0; i < a->u.list.count; i++) {
                if (!value_equal(a->u.list.items[i], b->u.list.items[i])) {
                    return 0;
                }
            }
            return 1;
        case VAL_FUNCTION:
            return a->u.function == b->u.function;
        case VAL_MACRO:
            return a->u.macro == b->u.macro;
        case VAL_LAMBDA:
            return 0;
    }
    return 0;
}

/* 回傳 1 表示可比較，並把結果寫入 out（-1/0/1）。 */
static int value_compare(Value *a, Value *b, int *out) {
    if (a->kind == VAL_INTEGER && b->kind == VAL_INTEGER) {
        *out = a->u.integer < b->u.integer ? -1 : a->u.integer > b->u.integer ? 1 : 0;
        return 1;
    }
    if (a->kind == VAL_TEXT && b->kind == VAL_TEXT) {
        char *sa = text_to_string(a->u.text.runs, a->u.text.count);
        char *sb = text_to_string(b->u.text.runs, b->u.text.count);
        int cmp = strcmp(sa, sb);
        free(sa);
        free(sb);
        *out = cmp < 0 ? -1 : cmp > 0 ? 1 : 0;
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Unicode 與列表輔助                                                  */
/* ------------------------------------------------------------------ */

static int utf8_count(const char *s) {
    int n = 0;
    const unsigned char *p = (const unsigned char *) s;
    while (*p) {
        unsigned int cp;
        int len;
        if (!utf8_decode(p, &cp, &len) || len <= 0) {
            p++;
        } else {
            p += len;
        }
        n++;
    }
    return n;
}

static long utf8_byte_offset(const char *s, long index) {
    long i = 0;
    const unsigned char *p = (const unsigned char *) s;
    while (*p && i < index) {
        unsigned int cp;
        int len;
        if (!utf8_decode(p, &cp, &len) || len <= 0) {
            p++;
        } else {
            p += len;
        }
        i++;
    }
    return (long) (p - (const unsigned char *) s);
}

static char *text_slice_cstr(const char *s, long start, long end) {
    long n = utf8_count(s);
    if (start < 0) start += n;
    if (end < 0) end += n;
    if (start < 0) start = 0;
    if (end < 0) end = 0;
    if (start > n) start = n;
    if (end > n) end = n;
    if (end < start) end = start;

    long b0 = utf8_byte_offset(s, start);
    long b1 = utf8_byte_offset(s, end);
    return xstrndup(s + b0, (size_t) (b1 - b0));
}

static int list_normalize_index(long *index, int count) {
    if (*index < 0) {
        *index += count;
    }
    return *index >= 0 && *index < count;
}

static void list_push_value(Value *list, Value *item) {
    int n = list->u.list.count;
    list->u.list.items = xrealloc(list->u.list.items, sizeof(Value *) * (size_t) (n + 1));
    list->u.list.items[n] = item;
    list->u.list.count = n + 1;
}

static int list_pop_value(Value *list, long index, Value **out) {
    if (list->kind != VAL_LIST || !list_normalize_index(&index, list->u.list.count)) {
        return 0;
    }
    *out = list->u.list.items[index];
    for (int i = (int) index; i + 1 < list->u.list.count; i++) {
        list->u.list.items[i] = list->u.list.items[i + 1];
    }
    list->u.list.count--;
    return 1;
}

static void require_list(Value *v, const char *where) {
    if (v->kind != VAL_LIST) {
        die("%s: expected a list, but got %s", where, value_to_string(v));
    }
}

static void require_text(Value *v, const char *where) {
    if (v->kind != VAL_TEXT) {
        die("%s: expected text, but got %s", where, value_to_string(v));
    }
}

static void require_integer(Value *v, const char *where) {
    if (v->kind != VAL_INTEGER) {
        die("%s: expected an integer, but got %s", where, value_to_string(v));
    }
}

/* ------------------------------------------------------------------ */
/* 圖片解碼                                                            */
/* ------------------------------------------------------------------ */

typedef struct {
    int width;
    int height;
    unsigned char *rgba;
} Image;

static int decode_png(const unsigned char *data, size_t len, Image *img) {
    png_image p;
    memset(&p, 0, sizeof(p));
    p.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&p, data, len)) {
        return 0;
    }

    p.format = PNG_FORMAT_RGBA;
    size_t size = PNG_IMAGE_SIZE(p);
    unsigned char *buffer = xmalloc(size);

    if (!png_image_finish_read(&p, NULL, buffer, 0, NULL)) {
        png_image_free(&p);
        free(buffer);
        return 0;
    }

    img->width = (int) p.width;
    img->height = (int) p.height;
    img->rgba = buffer;
    png_image_free(&p);
    return 1;
}

struct meow_jpeg_error {
    struct jpeg_error_mgr pub;
    jmp_buf jump;
};

static void meow_jpeg_error_exit(j_common_ptr cinfo) {
    struct meow_jpeg_error *err = (struct meow_jpeg_error *) cinfo->err;
    longjmp(err->jump, 1);
}

static int decode_jpeg(const unsigned char *data, size_t len, Image *img) {
    struct jpeg_decompress_struct cinfo;
    struct meow_jpeg_error jerr;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = meow_jpeg_error_exit;

    if (setjmp(jerr.jump)) {
        jpeg_destroy_decompress(&cinfo);
        return 0;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, (unsigned char *) data, (unsigned long) len);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    int w = (int) cinfo.output_width;
    int h = (int) cinfo.output_height;
    int components = cinfo.output_components;

    unsigned char *rgba = xmalloc((size_t) w * (size_t) h * 4);
    unsigned char *row = xmalloc((size_t) w * (size_t) components);

    while (cinfo.output_scanline < cinfo.output_height) {
        JSAMPROW rows[1] = {row};
        jpeg_read_scanlines(&cinfo, rows, 1);
        int y = (int) cinfo.output_scanline - 1;
        for (int x = 0; x < w; x++) {
            unsigned char r = row[x * components];
            unsigned char g = row[x * components + (components > 1 ? 1 : 0)];
            unsigned char b = row[x * components + (components > 2 ? 2 : 0)];
            unsigned char *p = rgba + ((size_t) y * (size_t) w + (size_t) x) * 4;
            p[0] = r;
            p[1] = g;
            p[2] = b;
            p[3] = 255;
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(row);

    img->width = w;
    img->height = h;
    img->rgba = rgba;
    return 1;
}

static int decode_image(const Blob *blob, Image *img) {
    if (blob->len >= 8 && blob->data[0] == 0x89 && blob->data[1] == 'P' &&
        blob->data[2] == 'N' && blob->data[3] == 'G') {
        return decode_png(blob->data, blob->len, img);
    }
    if (blob->len >= 2 && blob->data[0] == 0xFF && blob->data[1] == 0xD8) {
        return decode_jpeg(blob->data, blob->len, img);
    }
    return 0;
}

static int image_pixel(const Image *img, int x, int y) {
    if (x < 0 || x >= img->width || y < 0 || y >= img->height) {
        return 0;
    }
    const unsigned char *p = img->rgba + ((size_t) y * (size_t) img->width + (size_t) x) * 4;
    return ((int) p[0] << 16) | ((int) p[1] << 8) | (int) p[2];
}

static void ansi_truecolor(Buf *b, int background, int color) {
    buf_printf(b, "\x1b[%d;2;%d;%d;%dm",
               background ? 48 : 38,
               (color >> 16) & 0xFF,
               (color >> 8) & 0xFF,
               color & 0xFF);
}

static void render_image(Buf *out, const Blob *blob) {
    Image img;
    memset(&img, 0, sizeof(img));
    if (!decode_image(blob, &img)) {
        return;
    }

    if (out->len > 0 && out->data[out->len - 1] != '\n') {
        buf_putc(out, '\n');
    }

    for (int y = 0; y * 2 < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            int top = image_pixel(&img, x, y * 2);
            int bottom = image_pixel(&img, x, y * 2 + 1);
            ansi_truecolor(out, 0, top);
            ansi_truecolor(out, 1, bottom);
            buf_puts(out, "▀");
        }
        buf_puts(out, "\x1b[0m\n");
    }

    free(img.rgba);
}

/* ------------------------------------------------------------------ */
/* 文字輸出（ANSI）                                                    */
/* ------------------------------------------------------------------ */

static const char *highlight_code(Highlight h) {
    switch (h) {
        case HL_BLACK: return "\x1b[40m";
        case HL_BLUE: return "\x1b[44m";
        case HL_CYAN: return "\x1b[46m";
        case HL_GREEN: return "\x1b[42m";
        case HL_MAGENTA: return "\x1b[45m";
        case HL_RED: return "\x1b[41m";
        case HL_YELLOW: return "\x1b[43m";
        case HL_WHITE: return "\x1b[107m";
        case HL_LIGHT_GRAY: return "\x1b[47m";
        case HL_DARK_GRAY: return "\x1b[100m";
        default: return NULL;
    }
}

static void render_text(Buf *out, Run **runs, int count) {
    for (int i = 0; i < count; i++) {
        Run *run = runs[i];
        if (run->text && run->text[0]) {
            Buf prefix = {0};
            if (run->bold) {
                buf_puts(&prefix, "\x1b[1m");
            }
            if (run->underline != UL_NONE) {
                buf_puts(&prefix, "\x1b[4m");
            }
            if (run->color >= 0 && run->color != 0) {
                ansi_truecolor(&prefix, 0, run->color);
            }
            if (run->highlight != HL_NONE) {
                const char *code = highlight_code(run->highlight);
                if (code) {
                    buf_puts(&prefix, code);
                }
            }
            if (run->italic) {
                buf_puts(&prefix, "\x1b[3m");
            }

            if (prefix.len > 0) {
                buf_write(out, prefix.data, prefix.len);
            }
            buf_puts(out, run->text);

            if (prefix.len > 0 || run->italic) {
                buf_puts(out, "\x1b[0m");
            }
            buf_free(&prefix);
        }

        for (int p = 0; p < run->picture_count; p++) {
            render_image(out, &run->pictures[p]);
        }
    }
}

/* ------------------------------------------------------------------ */
/* 求值                                                                */
/* ------------------------------------------------------------------ */

static Value *eval(Context *ctx, Expr *expr);
static Value *apply_value(Context *ctx, Value *fn, Expr **args, int count);

static void require_args(int count, int expected) {
    if (count != expected) {
        die("expected %d arguments, but got %d", expected, count);
    }
}

static Value *text_apply(Context *ctx, Value *self, Value **args, int count) {
    int total = self->u.text.count;
    for (int i = 0; i < count; i++) {
        total += args[i]->kind == VAL_TEXT ? args[i]->u.text.count : 1;
    }

    Run **runs = xmalloc(sizeof(Run *) * (size_t) (total ? total : 1));
    int n = 0;
    for (int i = 0; i < self->u.text.count; i++) {
        runs[n++] = self->u.text.runs[i];
    }
    for (int i = 0; i < count; i++) {
        if (args[i]->kind == VAL_TEXT) {
            for (int j = 0; j < args[i]->u.text.count; j++) {
                runs[n++] = args[i]->u.text.runs[j];
            }
        } else {
            Run *r = xmalloc(sizeof(Run));
            memset(r, 0, sizeof(*r));
            r->text = value_to_string(args[i]);
            r->color = -1;
            r->size_half = -1;
            runs[n++] = r;
        }
    }

    (void) ctx;
    return val_text(runs, n);
}

static char *read_line(const char *prompt) {
    if (prompt && prompt[0]) {
        fputs(prompt, stdout);
        fflush(stdout);
    }

    Buf b = {0};
    int c;
    int got = 0;
    while ((c = fgetc(stdin)) != EOF) {
        got = 1;
        if (c == '\n') {
            break;
        }
        buf_putc(&b, (char) c);
    }
    if (!got && b.len == 0) {
        return xstrdup("");
    }
    if (b.len > 0 && b.data[b.len - 1] == '\r') {
        b.data[--b.len] = '\0';
    }
    return b.data ? b.data : xstrdup("");
}

static Value *print_values(Value **args, int count, const char *separator, int newline) {
    Buf out = {0};

    for (int i = 0; i < count; i++) {
        if (i > 0) {
            buf_puts(&out, separator);
        }
        if (args[i]->kind == VAL_TEXT) {
            render_text(&out, args[i]->u.text.runs, args[i]->u.text.count);
        } else {
            char *s = value_to_string(args[i]);
            buf_puts(&out, s);
            free(s);
        }
    }

    if (newline) {
        buf_putc(&out, '\n');
    }
    if (out.len > 0) {
        fwrite(out.data, 1, out.len, stdout);
    }
    fflush(stdout);
    buf_free(&out);
    return val_unit();
}

static Value *fn_print(Context *ctx, Value **args, int count) {
    (void) ctx;
    return print_values(args, count, "\n", 1);
}

static Value *fn_println(Context *ctx, Value **args, int count) {
    (void) ctx;
    return print_values(args, count, " ", 1);
}

static Value *fn_readline(Context *ctx, Value **args, int count) {
    (void) ctx;
    char *prompt = NULL;
    if (count == 1) {
        require_text(args[0], "readline prompt");
        prompt = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
    } else if (count > 1) {
        die("expected 0 or 1 arguments, but got %d", count);
    }

    char *line = read_line(prompt);
    free(prompt);
    return val_text_cstr(line);
}

static long as_int(Value *v, const char *where) {
    require_integer(v, where);
    return v->u.integer;
}

static Value *list_concat(Value *a, Value *b) {
    int na = a->u.list.count;
    int nb = b->u.list.count;
    Value **items = xmalloc(sizeof(Value *) * (size_t) (na + nb ? na + nb : 1));
    if (na > 0) memcpy(items, a->u.list.items, sizeof(Value *) * (size_t) na);
    if (nb > 0) memcpy(items + na, b->u.list.items, sizeof(Value *) * (size_t) nb);
    return val_list(items, na + nb);
}

static Value *fn_arith(Context *ctx, BuiltinFunction f, Value **args, int count) {
    switch (f) {
        case BF_ADD:
            require_args(count, 2);
            if (args[0]->kind == VAL_INTEGER && args[1]->kind == VAL_INTEGER) {
                return val_integer(args[0]->u.integer + args[1]->u.integer);
            }
            if (args[0]->kind == VAL_TEXT && args[1]->kind == VAL_TEXT) {
                return text_apply(ctx, args[0], &args[1], 1);
            }
            if (args[0]->kind == VAL_LIST && args[1]->kind == VAL_LIST) {
                return list_concat(args[0], args[1]);
            }
            die("add: expected two integers, two texts, or two lists");
            break;

        case BF_SUB:
        case BF_MUL:
        case BF_DIV:
        case BF_MOD:
        case BF_POW: {
            require_args(count, 2);
            long a = as_int(args[0], "arithmetic");
            long b = as_int(args[1], "arithmetic");
            if (f == BF_SUB) return val_integer(a - b);
            if (f == BF_MUL) return val_integer(a * b);
            if (f == BF_POW) {
                if (b < 0) die("pow: negative exponent");
                long result = 1;
                while (b > 0) {
                    if (b & 1) result *= a;
                    b >>= 1;
                    if (b) a *= a;
                }
                return val_integer(result);
            }
            if (b == 0) die("division by zero");
            if (f == BF_DIV) {
                long q = a / b;
                long r = a % b;
                if (r != 0 && ((r < 0) != (b < 0))) q--;
                return val_integer(q);
            }
            long r = a % b;
            if (r != 0 && ((r < 0) != (b < 0))) r += b;
            return val_integer(r);
        }

        case BF_NEG:
            require_args(count, 1);
            return val_integer(-as_int(args[0], "neg"));

        case BF_ABS: {
            require_args(count, 1);
            long v = as_int(args[0], "abs");
            return val_integer(v < 0 ? -v : v);
        }

        case BF_MIN:
        case BF_MAX: {
            if (count < 1) die("expected at least 1 argument");
            Value *best = args[0];
            for (int i = 1; i < count; i++) {
                int cmp;
                if (!value_compare(best, args[i], &cmp)) {
                    die("min/max: values are not comparable");
                }
                if ((f == BF_MIN && cmp > 0) || (f == BF_MAX && cmp < 0)) {
                    best = args[i];
                }
            }
            return best;
        }

        case BF_SUM: {
            require_args(count, 1);
            require_list(args[0], "sum");
            long total = 0;
            for (int i = 0; i < args[0]->u.list.count; i++) {
                total += as_int(args[0]->u.list.items[i], "sum");
            }
            return val_integer(total);
        }

        default:
            break;
    }
    die("unknown arithmetic function");
    return val_unit();
}

static Value *fn_compare(BuiltinFunction f, Value **args, int count) {
    if (f == BF_NOT) {
        require_args(count, 1);
        return val_integer(!value_truthy(args[0]));
    }

    if (f == BF_CONTAINS) {
        require_args(count, 2);
        if (args[0]->kind == VAL_LIST) {
            for (int i = 0; i < args[0]->u.list.count; i++) {
                if (value_equal(args[0]->u.list.items[i], args[1])) {
                    return val_integer(1);
                }
            }
            return val_integer(0);
        }
        if (args[0]->kind == VAL_TEXT && args[1]->kind == VAL_TEXT) {
            char *haystack = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
            char *needle = text_to_string(args[1]->u.text.runs, args[1]->u.text.count);
            int found = strstr(haystack, needle) != NULL;
            free(haystack);
            free(needle);
            return val_integer(found);
        }
        die("contains: expected a list or two texts");
    }

    require_args(count, 2);
    if (f == BF_EQ || f == BF_NE) {
        int eq = value_equal(args[0], args[1]);
        return val_integer(f == BF_EQ ? eq : !eq);
    }

    int cmp;
    if (!value_compare(args[0], args[1], &cmp)) {
        die("comparison: values are not comparable");
    }
    switch (f) {
        case BF_LT: return val_integer(cmp < 0);
        case BF_LE: return val_integer(cmp <= 0);
        case BF_GT: return val_integer(cmp > 0);
        case BF_GE: return val_integer(cmp >= 0);
        default: break;
    }
    die("unknown comparison function");
    return val_unit();
}

static Value *fn_list(BuiltinFunction f, Value **args, int count) {
    switch (f) {
        case BF_LIST:
            return val_list(args, count);

        case BF_CONS: {
            require_args(count, 2);
            require_list(args[1], "cons");
            int n = args[1]->u.list.count;
            Value **items = xmalloc(sizeof(Value *) * (size_t) (n + 1));
            items[0] = args[0];
            if (n > 0) memcpy(items + 1, args[1]->u.list.items, sizeof(Value *) * (size_t) n);
            return val_list(items, n + 1);
        }

        case BF_HEAD: {
            require_args(count, 1);
            require_list(args[0], "head");
            if (args[0]->u.list.count == 0) die("head: empty list");
            return args[0]->u.list.items[0];
        }

        case BF_TAIL: {
            require_args(count, 1);
            require_list(args[0], "tail");
            int n = args[0]->u.list.count;
            return val_list(args[0]->u.list.items + (n > 0 ? 1 : 0), n > 0 ? n - 1 : 0);
        }

        case BF_LEN: {
            require_args(count, 1);
            if (args[0]->kind == VAL_LIST) return val_integer(args[0]->u.list.count);
            if (args[0]->kind == VAL_TEXT) {
                char *s = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
                long n = utf8_count(s);
                free(s);
                return val_integer(n);
            }
            die("len: expected a list or text");
        }

        case BF_GET: {
            require_args(count, 2);
            long index = as_int(args[1], "get index");
            if (args[0]->kind == VAL_LIST) {
                int n = args[0]->u.list.count;
                if (!list_normalize_index(&index, n)) die("get: index out of range");
                return args[0]->u.list.items[index];
            }
            if (args[0]->kind == VAL_TEXT) {
                char *s = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
                long n = utf8_count(s);
                if (!list_normalize_index(&index, (int) n)) die("get: index out of range");
                char *ch = text_slice_cstr(s, index, index + 1);
                free(s);
                return val_text_cstr(ch);
            }
            die("get: expected a list or text");
        }

        case BF_SET: {
            require_args(count, 3);
            require_list(args[0], "set");
            long index = as_int(args[1], "set index");
            if (!list_normalize_index(&index, args[0]->u.list.count)) die("set: index out of range");
            args[0]->u.list.items[index] = args[2];
            return args[0];
        }

        case BF_PUSH: {
            require_args(count, 2);
            require_list(args[0], "push");
            list_push_value(args[0], args[1]);
            return args[0];
        }

        case BF_POP: {
            if (count < 1 || count > 2) die("pop: expected 1 or 2 arguments");
            require_list(args[0], "pop");
            long index = count == 2 ? as_int(args[1], "pop index") : args[0]->u.list.count - 1;
            Value *out = NULL;
            if (!list_pop_value(args[0], index, &out)) die("pop: index out of range");
            return out;
        }

        case BF_CONCAT: {
            require_args(count, 2);
            require_list(args[0], "concat");
            require_list(args[1], "concat");
            return list_concat(args[0], args[1]);
        }

        case BF_SLICE: {
            if (count < 2 || count > 3) die("slice: expected 2 or 3 arguments");
            long start = as_int(args[1], "slice start");
            if (args[0]->kind == VAL_LIST) {
                int n = args[0]->u.list.count;
                long end = count == 3 ? as_int(args[2], "slice end") : n;
                if (start < 0) start += n;
                if (end < 0) end += n;
                if (start < 0) start = 0;
                if (end < 0) end = 0;
                if (start > n) start = n;
                if (end > n) end = n;
                if (end < start) end = start;
                return val_list(args[0]->u.list.items + start, (int) (end - start));
            }
            if (args[0]->kind == VAL_TEXT) {
                char *s = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
                long n = utf8_count(s);
                long end = count == 3 ? as_int(args[2], "slice end") : n;
                char *part = text_slice_cstr(s, start, end);
                free(s);
                return val_text_cstr(part);
            }
            die("slice: expected a list or text");
        }

        case BF_RANGE: {
            if (count < 1 || count > 3) die("range: expected 1 to 3 arguments");
            long start = 0;
            long stop = as_int(args[0], "range");
            long step = 1;
            if (count >= 2) {
                start = stop;
                stop = as_int(args[1], "range");
            }
            if (count == 3) step = as_int(args[2], "range");
            if (step == 0) die("range: step must not be zero");

            long n;
            if (step > 0) {
                n = stop > start ? (stop - start + step - 1) / step : 0;
            } else {
                n = start > stop ? (start - stop + (-step) - 1) / (-step) : 0;
            }
            if (n < 0) n = 0;
            Value **items = xmalloc(sizeof(Value *) * (size_t) (n ? n : 1));
            for (long i = 0; i < n; i++) {
                items[i] = val_integer(start + i * step);
            }
            return val_list(items, (int) n);
        }

        default:
            break;
    }
    die("unknown list function");
    return val_unit();
}

static Value *fn_text(BuiltinFunction f, Value **args, int count) {
    switch (f) {
        case BF_STR:
            require_args(count, 1);
            if (args[0]->kind == VAL_TEXT) return args[0];
            return val_text_cstr(value_to_string(args[0]));

        case BF_PARSE_INT: {
            require_args(count, 1);
            if (args[0]->kind == VAL_INTEGER) return args[0];
            require_text(args[0], "parseInt");
            char *s = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
            errno = 0;
            char *end = NULL;
            long v = strtol(s, &end, 10);
            while (end && *end && isspace((unsigned char) *end)) end++;
            if (end == s || !end || *end != '\0' || errno == ERANGE) {
                die("parseInt: invalid integer: %s", s);
            }
            free(s);
            return val_integer(v);
        }

        case BF_TEXT_LEN: {
            require_args(count, 1);
            require_text(args[0], "textLen");
            char *s = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
            long n = utf8_count(s);
            free(s);
            return val_integer(n);
        }

        case BF_CHAR_AT: {
            require_args(count, 2);
            require_text(args[0], "charAt");
            char *s = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
            long n = utf8_count(s);
            long index = as_int(args[1], "charAt index");
            if (!list_normalize_index(&index, (int) n)) die("charAt: index out of range");
            char *ch = text_slice_cstr(s, index, index + 1);
            free(s);
            Value *result = val_text_cstr(ch);
            free(ch);
            return result;
        }

        case BF_SUBSTRING: {
            require_args(count, 3);
            require_text(args[0], "substring");
            char *s = text_to_string(args[0]->u.text.runs, args[0]->u.text.count);
            long n = utf8_count(s);
            long start = as_int(args[1], "substring start");
            long length = as_int(args[2], "substring length");
            if (start < 0) start += n;
            if (start < 0) start = 0;
            if (start > n) start = n;
            if (length < 0) length = 0;
            long end = start + length;
            if (end > n) end = n;
            char *part = text_slice_cstr(s, start, end);
            free(s);
            Value *result = val_text_cstr(part);
            free(part);
            return result;
        }

        case BF_JOIN: {
            require_args(count, 2);
            require_list(args[0], "join");
            require_text(args[1], "join separator");
            char *sep = text_to_string(args[1]->u.text.runs, args[1]->u.text.count);
            Buf b = {0};
            for (int i = 0; i < args[0]->u.list.count; i++) {
                if (i > 0) buf_puts(&b, sep);
                require_text(args[0]->u.list.items[i], "join item");
                char *item = text_to_string(args[0]->u.list.items[i]->u.text.runs,
                                            args[0]->u.list.items[i]->u.text.count);
                buf_puts(&b, item);
                free(item);
            }
            free(sep);
            Value *result = val_text_cstr(b.data ? b.data : "");
            buf_free(&b);
            return result;
        }

        default:
            break;
    }
    die("unknown text function");
    return val_unit();
}

static Value *fn_misc(BuiltinFunction f, Value **args, int count) {
    switch (f) {
        case BF_RANDOM: {
            if (count > 2) die("random: expected 0 to 2 arguments");
            if (count == 0) return val_integer(rand());
            long a = 0;
            long b = as_int(args[0], "random bound");
            if (count == 2) {
                a = b;
                b = as_int(args[1], "random bound");
            }
            if (b <= a) die("random: upper bound must be greater than lower bound");
            return val_integer(a + rand() % (b - a + (count == 2 ? 1 : 0)));
        }

        case BF_SRAND:
            require_args(count, 1);
            srand((unsigned int) as_int(args[0], "srand"));
            return val_unit();

        case BF_CLEAR:
            require_args(count, 0);
            fputs("\x1b[2J\x1b[H", stdout);
            fflush(stdout);
            return val_unit();

        default:
            break;
    }
    die("unknown function");
    return val_unit();
}

static Value *builtin_function_apply(BuiltinFunction f, Context *ctx, Value **args, int count) {
    switch (f) {
        case BF_PRINT:
            return fn_print(ctx, args, count);
        case BF_PRINTLN:
            return fn_println(ctx, args, count);
        case BF_READLINE:
            return fn_readline(ctx, args, count);

        case BF_ADD:
        case BF_SUB:
        case BF_MUL:
        case BF_DIV:
        case BF_MOD:
        case BF_POW:
        case BF_NEG:
        case BF_ABS:
        case BF_MIN:
        case BF_MAX:
        case BF_SUM:
            return fn_arith(ctx, f, args, count);

        case BF_EQ:
        case BF_NE:
        case BF_LT:
        case BF_LE:
        case BF_GT:
        case BF_GE:
        case BF_NOT:
        case BF_CONTAINS:
            return fn_compare(f, args, count);

        case BF_LIST:
        case BF_CONS:
        case BF_HEAD:
        case BF_TAIL:
        case BF_LEN:
        case BF_GET:
        case BF_SET:
        case BF_PUSH:
        case BF_POP:
        case BF_CONCAT:
        case BF_SLICE:
        case BF_RANGE:
            return fn_list(f, args, count);

        case BF_STR:
        case BF_PARSE_INT:
        case BF_TEXT_LEN:
        case BF_CHAR_AT:
        case BF_SUBSTRING:
        case BF_JOIN:
            return fn_text(f, args, count);

        case BF_RANDOM:
        case BF_SRAND:
        case BF_CLEAR:
            return fn_misc(f, args, count);
    }
    return val_unit();
}

static Value *lambda_apply(Value *fn, Value **args, int count) {
    if (count != fn->u.lambda.parameter_count) {
        die("expected %d arguments, but got %d",
            fn->u.lambda.parameter_count, count);
    }

    Context *nest = ctx_new(fn->u.lambda.declaration);
    for (int i = 0; i < count; i++) {
        ctx_set(nest, fn->u.lambda.parameters[i], args[i]);
    }

    ReturnFrame frame;
    frame.prev = g_return_frame;
    g_return_frame = &frame;
    g_return_value = NULL;

    if (setjmp(frame.buf) != 0) {
        Value *result = g_return_value ? g_return_value : val_unit();
        g_return_frame = frame.prev;
        return result;
    }

    Value *result = val_unit();
    for (int i = 0; i < fn->u.lambda.body_count; i++) {
        result = eval(nest, fn->u.lambda.body[i]);
    }
    g_return_frame = frame.prev;
    return result;
}

static Value *macro_process(BuiltinMacro m, Context *ctx, Expr **args, int count) {
    switch (m) {
        case BM_LET: {
            require_args(count, 2);
            if (args[0]->kind != EX_IDENT) {
                die("expected identifier for let");
            }
            Value *v = eval(ctx, args[1]);
            ctx_set(ctx, args[0]->u.ident, v);
            return v;
        }

        case BM_LAMBDA: {
            if (count < 1) {
                die("expected at least 1 argument for lambda");
            }

            Meow *params = NULL;
            int np = 0;

            if (args[0]->kind == EX_IDENT) {
                params = xmalloc(sizeof(Meow));
                params[0] = args[0]->u.ident;
                np = 1;
            } else if (args[0]->kind == EX_LIST) {
                np = args[0]->u.list.count;
                params = xmalloc(sizeof(Meow) * (size_t) (np ? np : 1));
                for (int i = 0; i < np; i++) {
                    if (args[0]->u.list.items[i]->kind != EX_IDENT) {
                        die("expected identifier in lambda parameter list");
                    }
                    params[i] = args[0]->u.list.items[i]->u.ident;
                }
            } else {
                die("expected parameter list for lambda");
            }

            return val_lambda(ctx, params, np, args + 1, count - 1);
        }

        case BM_INTEGER: {
            require_args(count, 1);
            if (args[0]->kind != EX_IDENT) {
                die("expected an identifier for integer");
            }
            int half = args[0]->u.ident.size_half;
            if (half < 0) {
                die("font size not set for integer");
            }
            return val_integer(half / 2);
        }

        case BM_IF: {
            if (count < 2 || count > 3) {
                die("if: expected 2 or 3 arguments, but got %d", count);
            }
            Value *condition = eval(ctx, args[0]);
            if (value_truthy(condition)) {
                return eval(ctx, args[1]);
            }
            if (count == 3) {
                return eval(ctx, args[2]);
            }
            return val_unit();
        }

        case BM_WHILE: {
            if (count < 2) {
                die("while: expected a condition and a body");
            }
            while (value_truthy(eval(ctx, args[0]))) {
                for (int i = 1; i < count; i++) {
                    eval(ctx, args[i]);
                }
            }
            return val_unit();
        }

        case BM_FOR: {
            if (count < 3) {
                die("for: expected a variable, an iterable and a body");
            }
            if (args[0]->kind != EX_IDENT) {
                die("for: expected an identifier");
            }
            Value *sequence = eval(ctx, args[1]);

            if (sequence->kind == VAL_LIST) {
                for (int i = 0; i < sequence->u.list.count; i++) {
                    ctx_set(ctx, args[0]->u.ident, sequence->u.list.items[i]);
                    for (int j = 2; j < count; j++) {
                        eval(ctx, args[j]);
                    }
                }
                return val_unit();
            }

            if (sequence->kind == VAL_TEXT) {
                char *s = text_to_string(sequence->u.text.runs, sequence->u.text.count);
                const unsigned char *p = (const unsigned char *) s;
                while (*p) {
                    unsigned int cp;
                    int len;
                    if (!utf8_decode(p, &cp, &len) || len <= 0) {
                        len = 1;
                    }
                    char *ch = xstrndup((const char *) p, (size_t) len);
                    ctx_set(ctx, args[0]->u.ident, val_text_cstr(ch));
                    free(ch);
                    for (int j = 2; j < count; j++) {
                        eval(ctx, args[j]);
                    }
                    p += len;
                }
                free(s);
                return val_unit();
            }

            die("for: expected a list or text");
        }

        case BM_DO: {
            Value *result = val_unit();
            for (int i = 0; i < count; i++) {
                result = eval(ctx, args[i]);
            }
            return result;
        }

        case BM_RETURN: {
            if (count > 1) {
                die("return: expected 0 or 1 argument");
            }
            if (!g_return_frame) {
                die("return: not inside a function");
            }
            g_return_value = count == 1 ? eval(ctx, args[0]) : val_unit();
            longjmp(g_return_frame->buf, 1);
        }

        case BM_AND: {
            Value *result = val_integer(1);
            for (int i = 0; i < count; i++) {
                result = eval(ctx, args[i]);
                if (!value_truthy(result)) {
                    return result;
                }
            }
            return result;
        }

        case BM_OR: {
            Value *result = val_integer(0);
            for (int i = 0; i < count; i++) {
                result = eval(ctx, args[i]);
                if (value_truthy(result)) {
                    return result;
                }
            }
            return result;
        }
    }
    return val_unit();
}

static Value *apply_value(Context *ctx, Value *fn, Expr **args, int count) {
    if (fn->kind == VAL_MACRO) {
        return macro_process(fn->u.macro, ctx, args, count);
    }

    /* 與 Java 版一致：即使是 unit 或 list，也會先求值所有參數。 */
    Value **values = xmalloc(sizeof(Value *) * (size_t) (count ? count : 1));
    for (int i = 0; i < count; i++) {
        values[i] = eval(ctx, args[i]);
    }

    switch (fn->kind) {
        case VAL_FUNCTION:
            return builtin_function_apply(fn->u.function, ctx, values, count);
        case VAL_LAMBDA:
            return lambda_apply(fn, values, count);
        case VAL_TEXT:
            return text_apply(ctx, fn, values, count);
        case VAL_INTEGER: {
            long index = fn->u.integer;
            if (index < 0 || index >= count) {
                die("integer index out of bounds: %ld (argument count %d)", index, count);
            }
            return values[index];
        }
        case VAL_UNIT:
        case VAL_LIST:
            return fn;
        default:
            die("cannot apply this value");
    }
    return val_unit();
}

static Value *eval(Context *ctx, Expr *expr) {
    switch (expr->kind) {
        case EX_IDENT:
            return ctx_get(ctx, expr->u.ident);
        case EX_RICH:
            return val_text(expr->u.rich.runs, expr->u.rich.count);
        case EX_LIST:
            if (expr->u.list.count == 0) {
                return val_unit();
            }
            return apply_value(ctx,
                               eval(ctx, expr->u.list.items[0]),
                               expr->u.list.items + 1,
                               expr->u.list.count - 1);
    }
    return val_unit();
}

/* ------------------------------------------------------------------ */
/* DOCX 讀取                                                           */
/* ------------------------------------------------------------------ */

#define NS_W "http://schemas.openxmlformats.org/wordprocessingml/2006/main"
#define NS_W_STRICT "http://purl.oclc.org/ooxml/wordprocessingml/main"

typedef struct {
    char *id;
    Blob blob;
} Rel;

typedef struct {
    Rel *items;
    int count;
} RelMap;

typedef struct {
    Buf text;
    int italic;
    int bold;
    Underline underline;
    int color;
    Highlight highlight;
    char *font;
    int size_half;
    Blob *pictures;
    int picture_count;
    int picture_cap;
} RunBuilder;

typedef struct {
    Document *doc;
    Paragraph *current;
    RunBuilder run;
    int in_run;
    int in_text;
    int in_drawing;
    RelMap *rels;
    int depth;
} DocState;

static const char *local_name(const char *qname) {
    const char *p = strrchr(qname, '\x1f');
    return p ? p + 1 : qname;
}

static int name_in_ns(const char *qname, const char *ns) {
    size_t n = strlen(ns);
    return strncmp(qname, ns, n) == 0 && qname[n] == '\x1f';
}

static int is_word(const char *qname) {
    return name_in_ns(qname, NS_W) || name_in_ns(qname, NS_W_STRICT);
}

static const char *attr_local(const XML_Char **atts, const char *local) {
    for (int i = 0; atts[i]; i += 2) {
        if (strcmp(local_name(atts[i]), local) == 0) {
            return atts[i + 1];
        }
    }
    return NULL;
}

static Highlight parse_highlight(const char *s) {
    if (!s || strcmp(s, "none") == 0) return HL_NONE;
    if (strcmp(s, "black") == 0) return HL_BLACK;
    if (strcmp(s, "blue") == 0) return HL_BLUE;
    if (strcmp(s, "cyan") == 0) return HL_CYAN;
    if (strcmp(s, "green") == 0) return HL_GREEN;
    if (strcmp(s, "magenta") == 0) return HL_MAGENTA;
    if (strcmp(s, "red") == 0) return HL_RED;
    if (strcmp(s, "yellow") == 0) return HL_YELLOW;
    if (strcmp(s, "white") == 0) return HL_WHITE;
    if (strcmp(s, "lightGray") == 0) return HL_LIGHT_GRAY;
    if (strcmp(s, "darkGray") == 0) return HL_DARK_GRAY;
    return HL_NONE;
}

static Underline parse_underline(const char *s) {
    if (!s) return UL_SINGLE;
    if (strcmp(s, "single") == 0) return UL_SINGLE;
    if (strcmp(s, "double") == 0) return UL_DOUBLE;
    if (strcmp(s, "thick") == 0) return UL_THICK;
    if (strcmp(s, "dotted") == 0) return UL_DOTTED;
    if (strcmp(s, "dash") == 0) return UL_DASH;
    if (strcmp(s, "dotDash") == 0) return UL_DOT_DASH;
    if (strcmp(s, "dotDotDash") == 0) return UL_DOT_DOT_DASH;
    if (strcmp(s, "wave") == 0) return UL_WAVE;
    if (strcmp(s, "none") == 0) return UL_NONE;
    return UL_OTHER;
}

static int parse_color(const char *s) {
    if (!s || strcmp(s, "auto") == 0) {
        return -1;
    }
    return (int) strtol(s, NULL, 16);
}

static int parse_on_off(const char *s) {
    if (!s) return 1;
    if (strcmp(s, "0") == 0 || ascii_ieq(s, "false") || ascii_ieq(s, "off")) {
        return 0;
    }
    return 1;
}

static void run_builder_reset(RunBuilder *b) {
    b->text.len = 0;
    if (b->text.data) {
        b->text.data[0] = '\0';
    }
    b->italic = 0;
    b->bold = 0;
    b->underline = UL_NONE;
    b->color = -1;
    b->highlight = HL_NONE;
    b->font = NULL;
    b->size_half = -1;
    b->picture_count = 0;
}

static void run_builder_add_picture(RunBuilder *b, Blob blob) {
    if (b->picture_count == b->picture_cap) {
        b->picture_cap = b->picture_cap ? b->picture_cap * 2 : 4;
        b->pictures = xrealloc(b->pictures, sizeof(Blob) * (size_t) b->picture_cap);
    }
    b->pictures[b->picture_count++] = blob;
}

static void document_add_paragraph(Document *doc, Paragraph p) {
    doc->paragraphs = xrealloc(doc->paragraphs, sizeof(Paragraph) * (size_t) (doc->count + 1));
    doc->paragraphs[doc->count++] = p;
}

static void add_run_to_paragraph(Paragraph *para, RunBuilder *b) {
    Run *r = xmalloc(sizeof(Run));
    memset(r, 0, sizeof(*r));
    r->text = xstrdup(b->text.data ? b->text.data : "");
    r->italic = b->italic;
    r->bold = b->bold;
    r->underline = b->underline;
    r->color = b->color;
    r->highlight = b->highlight;
    r->font = xstrdup(b->font);
    r->size_half = b->size_half;
    r->pictures = b->pictures;
    r->picture_count = b->picture_count;

    para->runs = xrealloc(para->runs, sizeof(Run *) * (size_t) (para->count + 1));
    para->runs[para->count++] = r;

    b->pictures = NULL;
    b->picture_count = 0;
    b->picture_cap = 0;
}

static const Blob *rel_lookup(RelMap *rels, const char *id) {
    if (!id || !rels) return NULL;
    for (int i = 0; i < rels->count; i++) {
        if (strcmp(rels->items[i].id, id) == 0) {
            return &rels->items[i].blob;
        }
    }
    return NULL;
}

static void XMLCALL doc_start(void *user, const XML_Char *name, const XML_Char **atts) {
    DocState *st = user;
    const char *local = local_name(name);

    if (strcmp(local, "p") == 0 && is_word(name)) {
        st->current = xmalloc(sizeof(Paragraph));
        st->current->runs = NULL;
        st->current->count = 0;
        return;
    }

    if (strcmp(local, "r") == 0 && is_word(name)) {
        run_builder_reset(&st->run);
        st->in_run = 1;
        return;
    }

    if (!st->in_run) {
        return;
    }

    if (strcmp(local, "b") == 0) {
        st->run.bold = parse_on_off(attr_local(atts, "val"));
    } else if (strcmp(local, "i") == 0) {
        st->run.italic = parse_on_off(attr_local(atts, "val"));
    } else if (strcmp(local, "u") == 0) {
        const char *v = attr_local(atts, "val");
        st->run.underline = v ? parse_underline(v) : UL_SINGLE;
    } else if (strcmp(local, "color") == 0) {
        st->run.color = parse_color(attr_local(atts, "val"));
    } else if (strcmp(local, "sz") == 0) {
        const char *v = attr_local(atts, "val");
        if (v) {
            st->run.size_half = (int) strtol(v, NULL, 10);
        }
    } else if (strcmp(local, "highlight") == 0) {
        st->run.highlight = parse_highlight(attr_local(atts, "val"));
    } else if (strcmp(local, "rFonts") == 0) {
        const char *v = attr_local(atts, "ascii");
        st->run.font = v ? xstrdup(v) : NULL;
    } else if (strcmp(local, "t") == 0 && is_word(name)) {
        st->in_text = 1;
    } else if (strcmp(local, "tab") == 0) {
        buf_putc(&st->run.text, '\t');
    } else if (strcmp(local, "br") == 0 || strcmp(local, "cr") == 0) {
        buf_putc(&st->run.text, '\n');
    } else if (strcmp(local, "drawing") == 0 && is_word(name)) {
        st->in_drawing = 1;
    } else if (strcmp(local, "blip") == 0) {
        if (st->in_drawing) {
            const Blob *blob = rel_lookup(st->rels, attr_local(atts, "embed"));
            if (blob) {
                run_builder_add_picture(&st->run, *blob);
            }
        }
    } else if (strcmp(local, "imagedata") == 0) {
        if (st->in_drawing) {
            const Blob *blob = rel_lookup(st->rels, attr_local(atts, "id"));
            if (blob) {
                run_builder_add_picture(&st->run, *blob);
            }
        }
    }
}

static void XMLCALL doc_end(void *user, const XML_Char *name) {
    DocState *st = user;
    const char *local = local_name(name);

    if (strcmp(local, "t") == 0) {
        st->in_text = 0;
    } else if (strcmp(local, "r") == 0 && is_word(name)) {
        if (st->in_run) {
            if (st->run.text.len > 0 || st->run.picture_count > 0) {
                if (st->current) {
                    add_run_to_paragraph(st->current, &st->run);
                }
            }
            st->in_run = 0;
        }
    } else if (strcmp(local, "p") == 0 && is_word(name)) {
        if (st->current) {
            document_add_paragraph(st->doc, *st->current);
            free(st->current);
            st->current = NULL;
        }
    } else if (strcmp(local, "drawing") == 0 && is_word(name)) {
        st->in_drawing = 0;
    }
}

static void XMLCALL doc_text(void *user, const XML_Char *s, int len) {
    DocState *st = user;
    if (st->in_text) {
        buf_write(&st->run.text, s, (size_t) len);
    }
}

typedef struct {
    RelMap *rels;
    Zip *zip;
} RelState2;

static void XMLCALL rel_start2(void *user, const XML_Char *name, const XML_Char **atts) {
    RelState2 *st = user;
    if (strcmp(local_name(name), "Relationship") != 0) {
        return;
    }

    const char *id = attr_local(atts, "Id");
    const char *target = attr_local(atts, "Target");
    const char *type = attr_local(atts, "Type");
    if (!id || !target || !type) {
        return;
    }

    size_t type_len = strlen(type);
    if (type_len < 6 || strcmp(type + type_len - 6, "/image") != 0) {
        return;
    }

    Buf path = {0};
    if (target[0] == '/') {
        buf_puts(&path, target + 1);
    } else {
        buf_puts(&path, "word/");
        buf_puts(&path, target);
    }

    char *parts[256];
    int np = 0;
    char *p = path.data;
    while (p && *p) {
        while (*p == '/') p++;
        if (!*p) break;
        char *tok = p;
        while (*p && *p != '/') p++;
        if (*p) *p++ = '\0';
        if (strcmp(tok, ".") == 0) continue;
        if (strcmp(tok, "..") == 0) {
            if (np > 0) np--;
            continue;
        }
        if (np < 256) parts[np++] = tok;
    }

    Buf norm = {0};
    for (int i = 0; i < np; i++) {
        if (i > 0) buf_putc(&norm, '/');
        buf_puts(&norm, parts[i]);
    }

    ZipEntry *e = norm.data ? zip_find(st->zip, norm.data) : NULL;
    if (e) {
        size_t len = 0;
        unsigned char *data = zip_extract(st->zip, e, &len);
        if (data) {
            st->rels->items = xrealloc(st->rels->items,
                                       sizeof(Rel) * (size_t) (st->rels->count + 1));
            st->rels->items[st->rels->count].id = xstrdup(id);
            st->rels->items[st->rels->count].blob.data = data;
            st->rels->items[st->rels->count].blob.len = len;
            st->rels->count++;
        }
    }

    buf_free(&path);
    buf_free(&norm);
}

static void parse_rels(Zip *zip, RelMap *rels) {
    ZipEntry *e = zip_find(zip, "word/_rels/document.xml.rels");
    if (!e) {
        return;
    }

    size_t len = 0;
    unsigned char *data = zip_extract(zip, e, &len);
    if (!data) {
        return;
    }

    XML_Parser parser = XML_ParserCreateNS(NULL, '\x1f');
    RelState2 state = {rels, zip};
    XML_SetUserData(parser, &state);
    XML_SetStartElementHandler(parser, rel_start2);
    if (XML_Parse(parser, (const char *) data, (int) len, 1) == XML_STATUS_ERROR) {
        die("failed to parse document.xml.rels: %s",
            XML_ErrorString(XML_GetErrorCode(parser)));
    }
    XML_ParserFree(parser);
    free(data);
}

static Document *read_docx(const char *path) {
    Zip zip;
    memset(&zip, 0, sizeof(zip));
    if (!zip_open(&zip, path)) {
        die("cannot open docx file: %s", path);
    }

    RelMap rels;
    memset(&rels, 0, sizeof(rels));
    parse_rels(&zip, &rels);

    ZipEntry *entry = zip_find(&zip, "word/document.xml");
    if (!entry) {
        die("not a valid docx file (word/document.xml not found): %s", path);
    }

    size_t len = 0;
    unsigned char *data = zip_extract(&zip, entry, &len);
    if (!data) {
        die("cannot extract word/document.xml: %s", path);
    }

    Document *doc = xmalloc(sizeof(Document));
    memset(doc, 0, sizeof(*doc));

    DocState state;
    memset(&state, 0, sizeof(state));
    state.doc = doc;
    state.rels = &rels;

    XML_Parser parser = XML_ParserCreateNS(NULL, '\x1f');
    XML_SetUserData(parser, &state);
    XML_SetElementHandler(parser, doc_start, doc_end);
    XML_SetCharacterDataHandler(parser, doc_text);

    if (XML_Parse(parser, (const char *) data, (int) len, 1) == XML_STATUS_ERROR) {
        die("failed to parse word/document.xml: %s",
            XML_ErrorString(XML_GetErrorCode(parser)));
    }
    XML_ParserFree(parser);
    free(data);

    return doc;
}

/* ------------------------------------------------------------------ */
/* 除錯輸出                                                            */
/* ------------------------------------------------------------------ */

typedef struct {
    const char *name;
    int color;
    int bold;
    Underline underline;
    Highlight highlight;
    BuiltinFunction function;
    Meow meow;
} FunctionSpec;

typedef struct {
    const char *name;
    int color;
    int bold;
    Underline underline;
    Highlight highlight;
    BuiltinMacro macro;
    Meow meow;
} MacroSpec;

#define MEOW_FN(name_, color_, bold_, ul_, hl_, fn_) \
    { .name = name_, .color = color_, .bold = bold_, .underline = ul_, .highlight = hl_, .function = fn_ }
#define MEOW_MC(name_, color_, bold_, ul_, hl_, macro_) \
    { .name = name_, .color = color_, .bold = bold_, .underline = ul_, .highlight = hl_, .macro = macro_ }

static FunctionSpec g_functions[] = {
    MEOW_FN("print", 0xC00000, 0, UL_NONE, HL_NONE, BF_PRINT),
    MEOW_FN("readline", 0xC07000, 0, UL_NONE, HL_NONE, BF_READLINE),
    MEOW_FN("println", 0x909000, 0, UL_NONE, HL_NONE, BF_PRINTLN),
    MEOW_FN("clear", 0x008000, 0, UL_NONE, HL_NONE, BF_CLEAR),
    MEOW_FN("add", 0x008080, 0, UL_NONE, HL_NONE, BF_ADD),
    MEOW_FN("sub", 0x0070C0, 0, UL_NONE, HL_NONE, BF_SUB),
    MEOW_FN("mul", 0x8060C0, 0, UL_NONE, HL_NONE, BF_MUL),
    MEOW_FN("div", 0xC000C0, 0, UL_NONE, HL_NONE, BF_DIV),
    MEOW_FN("mod", 0xC00000, 1, UL_NONE, HL_NONE, BF_MOD),
    MEOW_FN("pow", 0xC07000, 1, UL_NONE, HL_NONE, BF_POW),
    MEOW_FN("neg", 0x909000, 1, UL_NONE, HL_NONE, BF_NEG),
    MEOW_FN("abs", 0x008000, 1, UL_NONE, HL_NONE, BF_ABS),
    MEOW_FN("min", 0x008080, 1, UL_NONE, HL_NONE, BF_MIN),
    MEOW_FN("max", 0x0070C0, 1, UL_NONE, HL_NONE, BF_MAX),
    MEOW_FN("sum", 0x8060C0, 1, UL_NONE, HL_NONE, BF_SUM),
    MEOW_FN("eq", 0xC000C0, 1, UL_NONE, HL_NONE, BF_EQ),
    MEOW_FN("ne", 0xC00000, 0, UL_DOUBLE, HL_NONE, BF_NE),
    MEOW_FN("lt", 0xC07000, 0, UL_DOUBLE, HL_NONE, BF_LT),
    MEOW_FN("le", 0x909000, 0, UL_DOUBLE, HL_NONE, BF_LE),
    MEOW_FN("gt", 0x008000, 0, UL_DOUBLE, HL_NONE, BF_GT),
    MEOW_FN("ge", 0x008080, 0, UL_DOUBLE, HL_NONE, BF_GE),
    MEOW_FN("not", 0x0070C0, 0, UL_DOUBLE, HL_NONE, BF_NOT),
    MEOW_FN("parseInt", 0x8060C0, 0, UL_DOUBLE, HL_NONE, BF_PARSE_INT),
    MEOW_FN("random", 0xC000C0, 0, UL_DOUBLE, HL_NONE, BF_RANDOM),
    MEOW_FN("srand", 0xC00000, 1, UL_DOUBLE, HL_NONE, BF_SRAND),
    MEOW_FN("contains", 0xC07000, 1, UL_DOUBLE, HL_NONE, BF_CONTAINS),
    MEOW_FN("list", 0x909000, 1, UL_DOUBLE, HL_NONE, BF_LIST),
    MEOW_FN("cons", 0x008000, 1, UL_DOUBLE, HL_NONE, BF_CONS),
    MEOW_FN("head", 0x008080, 1, UL_DOUBLE, HL_NONE, BF_HEAD),
    MEOW_FN("tail", 0x0070C0, 1, UL_DOUBLE, HL_NONE, BF_TAIL),
    MEOW_FN("len", 0x8060C0, 1, UL_DOUBLE, HL_NONE, BF_LEN),
    MEOW_FN("get", 0xC000C0, 1, UL_DOUBLE, HL_NONE, BF_GET),
    MEOW_FN("set", 0xC00000, 0, UL_NONE, HL_YELLOW, BF_SET),
    MEOW_FN("push", 0xC07000, 0, UL_NONE, HL_YELLOW, BF_PUSH),
    MEOW_FN("pop", 0x909000, 0, UL_NONE, HL_YELLOW, BF_POP),
    MEOW_FN("concat", 0x008000, 0, UL_NONE, HL_YELLOW, BF_CONCAT),
    MEOW_FN("slice", 0x008080, 0, UL_NONE, HL_YELLOW, BF_SLICE),
    MEOW_FN("range", 0x0070C0, 0, UL_NONE, HL_YELLOW, BF_RANGE),
    MEOW_FN("str", 0x8060C0, 0, UL_NONE, HL_YELLOW, BF_STR),
    MEOW_FN("textLen", 0xC000C0, 0, UL_NONE, HL_YELLOW, BF_TEXT_LEN),
    MEOW_FN("charAt", 0xC00000, 1, UL_NONE, HL_YELLOW, BF_CHAR_AT),
    MEOW_FN("substring", 0xC07000, 1, UL_NONE, HL_YELLOW, BF_SUBSTRING),
    MEOW_FN("join", 0x909000, 1, UL_NONE, HL_YELLOW, BF_JOIN),
};

static MacroSpec g_macros[] = {
    MEOW_MC("let", 0x008000, 1, UL_NONE, HL_YELLOW, BM_LET),
    MEOW_MC("lambda", 0x008080, 1, UL_NONE, HL_YELLOW, BM_LAMBDA),
    MEOW_MC("integer", 0x0070C0, 1, UL_NONE, HL_YELLOW, BM_INTEGER),
    MEOW_MC("if", 0x8060C0, 1, UL_NONE, HL_YELLOW, BM_IF),
    MEOW_MC("while", 0xC000C0, 1, UL_NONE, HL_YELLOW, BM_WHILE),
    MEOW_MC("for", 0xC00000, 0, UL_DOUBLE, HL_YELLOW, BM_FOR),
    MEOW_MC("do", 0xC07000, 0, UL_DOUBLE, HL_YELLOW, BM_DO),
    MEOW_MC("return", 0x909000, 0, UL_DOUBLE, HL_YELLOW, BM_RETURN),
    MEOW_MC("and", 0x008000, 0, UL_DOUBLE, HL_YELLOW, BM_AND),
    MEOW_MC("or", 0x008080, 0, UL_DOUBLE, HL_YELLOW, BM_OR),
};

#undef MEOW_FN
#undef MEOW_MC

static int g_function_count = (int) (sizeof(g_functions) / sizeof(g_functions[0]));
static int g_macro_count = (int) (sizeof(g_macros) / sizeof(g_macros[0]));

static const char *builtin_symbol(const Meow *m) {
    for (int i = 0; i < g_function_count; i++) {
        if (meow_equal(m, &g_functions[i].meow)) return g_functions[i].name;
    }
    for (int i = 0; i < g_macro_count; i++) {
        if (meow_equal(m, &g_macros[i].meow)) return g_macros[i].name;
    }
    return NULL;
}

static char *expr_debug(Expr *e);

static char *expr_debug(Expr *e) {
    Buf b = {0};
    switch (e->kind) {
        case EX_IDENT: {
            const char *name = builtin_symbol(&e->u.ident);
            if (name) {
                buf_puts(&b, name);
            } else {
                buf_printf(&b, "$%d", (int) (e->u.ident.color ^ e->u.ident.highlight * 131));
            }
            break;
        }
        case EX_RICH: {
            buf_putc(&b, '"');
            for (int i = 0; i < e->u.rich.count; i++) {
                const char *s = e->u.rich.runs[i]->text;
                if (s) buf_puts(&b, s);
            }
            buf_putc(&b, '"');
            break;
        }
        case EX_LIST: {
            buf_putc(&b, '[');
            for (int i = 0; i < e->u.list.count; i++) {
                if (i > 0) buf_puts(&b, ", ");
                char *s = expr_debug(e->u.list.items[i]);
                buf_puts(&b, s);
                free(s);
            }
            buf_putc(&b, ']');
            break;
        }
    }
    return b.data ? b.data : xstrdup("");
}

/* ------------------------------------------------------------------ */
/* 主程式                                                              */
/* ------------------------------------------------------------------ */

static void register_builtins(Context *root) {
    for (int i = 0; i < g_function_count; i++) {
        g_functions[i].meow = meow_builtin_style(g_functions[i].color,
                                                 g_functions[i].bold,
                                                 g_functions[i].underline,
                                                 g_functions[i].highlight);
        ctx_set(root, g_functions[i].meow, val_function(g_functions[i].function));
    }
    for (int i = 0; i < g_macro_count; i++) {
        g_macros[i].meow = meow_builtin_style(g_macros[i].color,
                                              g_macros[i].bold,
                                              g_macros[i].underline,
                                              g_macros[i].highlight);
        ctx_set(root, g_macros[i].meow, val_macro(g_macros[i].macro));
    }
}

static void eval_file(Context *ctx, const char *path) {
    Document *doc = read_docx(path);

    for (int i = 0; i < doc->count; i++) {
        Paragraph *p = &doc->paragraphs[i];
        Expr *expr = parse_run_list(p->runs, p->count, 1);

        if (g_debug) {
            char *s = expr_debug(expr);
            printf("[DEBUG] >>> [%s]\n", s);
            free(s);
        }

        Value *result = eval(ctx, expr);

        if (g_debug) {
            char *s = value_to_string(result);
            printf("[DEBUG] |   >>> return %s\n", s);
            free(s);
        }
    }
}

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "--version") == 0) {
        printf("Meow Language (native) 1.2\n");
        printf("Native rewrite, lightweight DOCX reader and performance work: DeepSeek V4.1 Flash\n");
        printf("Original project: Glavo\n");
        return 0;
    }

    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        printf("Usage: %s <program.docx> [more.docx ...]\n", argv[0]);
        printf("\n");
        printf("Meow Language (native) 1.2\n");
        printf("Developed by DeepSeek V4.1 Flash (based on Glavo/MeowLang)\n");
        return argc < 2 ? 1 : 0;
    }

    const char *debug = getenv("MEOW_DEBUG");
    g_debug = debug && strcmp(debug, "0") != 0;

    srand((unsigned int) time(NULL));

    Context *root = ctx_new(NULL);
    register_builtins(root);

    for (int i = 1; i < argc; i++) {
        Context *ctx = ctx_new(root);
        eval_file(ctx, argv[i]);
    }

    return 0;
}
