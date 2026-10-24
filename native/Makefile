# Meow Language - native build
#
# 原生版本由 DeepSeek V4.1 Flash 開發。
# 需要 C11 編譯器，以及 zlib、expat、libpng、libjpeg 的開發標頭與函式庫。
#
# 一般 Linux / Android(Termux)：
#   make
#
# 若套件安裝在非標準路徑，可覆寫：
#   make CPPFLAGS="$(pkg-config --cflags zlib expat libpng libjpeg)" \
#        LDLIBS="$(pkg-config --libs zlib expat libpng libjpeg) -lm"

CC          ?= cc
PKG_CONFIG  ?= pkg-config
PKGS        := zlib expat libpng libjpeg

CFLAGS      ?= -O2 -std=c11 -Wall -Wextra
CPPFLAGS    ?=
LDFLAGS     ?=
LDLIBS      ?=

HAVE_PKG := $(shell $(PKG_CONFIG) --exists $(PKGS) 2>/dev/null && echo 1)

ifeq ($(HAVE_PKG),1)
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags $(PKGS))
LDLIBS   += $(shell $(PKG_CONFIG) --libs $(PKGS))
else
LDLIBS   += -lz -lexpat -lpng -ljpeg
endif

LDLIBS += -lm

TARGET  := meow
SOURCE  := meow.c
FONT_INSTALLER ?= ../fonts/install-fonts.sh
PREFIX  ?= /usr/local
BINDIR  ?= $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCE) $(LDLIBS)

install: $(TARGET)
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 755 $(TARGET) "$(DESTDIR)$(BINDIR)/meow"

install-fonts:
	sh "$(FONT_INSTALLER)" --user

clean:
	rm -f $(TARGET) meow_new

.PHONY: all clean install install-fonts
