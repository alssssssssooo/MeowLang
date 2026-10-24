/*
 * Copyright (C) 2025 Glavo
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
package org.glavo.meow;

import java.util.Locale;
import java.util.Objects;
import java.util.Set;

public final class Meow {
    public static final boolean DEBUG = "true".equals(System.getProperty("meow.debug"));

    private static final Set<String> MEOW = Set.of(
            "meow", "miaou", "miao", "nyan",
            "にゃん", "야옹", "喵", "喵呜",
            "мяу", "miyav", "miau", "miauw", "μιάου", "mjau", "nyávog", "म्याऊं", "meong", "mjá", "meo meo",
            "mjäu", "mjav", "мјау", "mjal", "მიაუ", "myau", "ngiyavuma", "ማው", "maw"
    );

    public static Meow of(MeowRun term) {
        if (term == null) {
            return null;
        }

        if (!term.pictures().isEmpty()) {
            return null;
        }

        if (MEOW.contains(term.text().trim().toLowerCase(Locale.ROOT))) {
            return new Meow(term);
        } else {
            return null;
        }
    }

    public static Meow builtin(int color, MeowHighlight highlightColor) {
        return new Meow(
                false,
                MeowUnderline.NONE,
                color,
                highlightColor,
                false,
                null,
                null
        );
    }

    private final String rawName;
    private final boolean isBracket;
    private final MeowUnderline underline;
    private final int color;
    private final MeowHighlight highlightColor;
    private final boolean isBold;
    private final String font;
    private final Double fontSize;
    private final int hash;

    public Meow(MeowRun term) {
        this.rawName = term.text();
        this.isBracket = term.italic();
        this.color = term.color() == null ? 0 : term.color();
        this.highlightColor = term.highlight();
        this.isBold = term.bold();
        this.underline = term.underline();
        this.font = term.font();
        this.fontSize = term.fontSize();
        this.hash = Objects.hash(isBracket, color, isBold, highlightColor, underline, font, fontSize);
    }

    public Meow(boolean isBracket, MeowUnderline underline, int color, MeowHighlight highlightColor,
                boolean isBold, String font, Double fontSize) {
        this.rawName = "";
        this.isBracket = isBracket;
        this.underline = underline;
        this.color = color;
        this.highlightColor = highlightColor;
        this.isBold = isBold;
        this.font = font;
        this.fontSize = fontSize;
        this.hash = Objects.hash(isBracket, color, isBold, highlightColor, underline, font, fontSize);
    }

    public boolean isBracket() {
        return isBracket;
    }

    public MeowUnderline getUnderline() {
        return underline;
    }

    public int getColor() {
        return color;
    }

    public MeowHighlight getHighlightColor() {
        return highlightColor;
    }

    public String getFont() {
        return font;
    }

    public Double getFontSize() {
        return fontSize;
    }

    @Override
    public int hashCode() {
        return hash;
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof Meow that
               && this.isBracket == that.isBracket
               && this.color == that.color
               && this.highlightColor == that.highlightColor
               && this.isBold == that.isBold
               && this.underline == that.underline
               && Objects.equals(this.font, that.font)
               && Objects.equals(this.fontSize, that.fontSize);
    }

    @Override
    public String toString() {
        return "Meow[%s, isBracket=%s, color=%06X, highlightColor=%s, isBold=%s, underline=%s, font=%s, fontSize=%s]"
                .formatted(rawName, isBracket, color, highlightColor, isBold, underline, font, fontSize);
    }
}
