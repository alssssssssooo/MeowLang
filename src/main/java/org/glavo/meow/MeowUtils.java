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

import kala.ansi.AnsiString;

import java.util.List;

public final class MeowUtils {
    public static boolean isBlank(MeowRun term) {
        return term.pictures().isEmpty() && term.text().isBlank();
    }

    public static int findEnd(List<MeowRun> expression, Meow startBracket, int startBracketIndex) {
        int end = startBracketIndex + 1;
        while (end < expression.size()) {
            if (startBracket.equals(Meow.of(expression.get(end)))) {
                return end;
            }
            end++;
        }
        return end;
    }

    public static void checkArgsCount(List<?> args, int expected) {
        if (args.size() != expected) {
            throw new IllegalArgumentException("Expected " + expected + " arguments, but got " + args.size() + ": " + args);
        }
    }

    public static void appendTrueRgbEscape(StringBuilder builder, boolean background, int color) {
        builder.append("\u001b[")
                .append(background ? "48" : "38")
                .append(";2;")
                .append((color >> 16) & 0xFF)
                .append(';')
                .append((color >> 8) & 0xFF)
                .append(';')
                .append(color & 0xFF)
                .append('m');
    }

    public static AnsiString.Attribute attributeOf(MeowHighlight color) {
        if (color == MeowHighlight.BLACK) {
            return AnsiString.Back.Black;
        } else if (color == MeowHighlight.BLUE) {
            return AnsiString.Back.Blue;
        } else if (color == MeowHighlight.CYAN) {
            return AnsiString.Back.Cyan;
        } else if (color == MeowHighlight.GREEN) {
            return AnsiString.Back.Green;
        } else if (color == MeowHighlight.MAGENTA) {
            return AnsiString.Back.Magenta;
        } else if (color == MeowHighlight.RED) {
            return AnsiString.Back.Red;
        } else if (color == MeowHighlight.YELLOW) {
            return AnsiString.Back.Yellow;
        } else if (color == MeowHighlight.WHITE) {
            return AnsiString.Back.White;
        } else if (color == MeowHighlight.LIGHT_GRAY) {
            return AnsiString.Back.LightGray;
        } else if (color == MeowHighlight.DARK_GRAY) {
            return AnsiString.Back.DarkGray;
        } else {
            // TODO: Dark colors
            return null;
        }
    }

    public static String toDebugString(List<MeowRun> runs) {
        StringBuilder builder = new StringBuilder();
        builder.append('"');

        for (MeowRun run : runs) {
            String text = run.text();
            for (int i = 0; i < text.length(); i++) {
                char ch = text.charAt(i);

                switch (ch) {
                    case '\n':
                        builder.append("\\n");
                        break;
                    case '\r':
                        builder.append("\\r");
                        break;
                    case '\t':
                        builder.append("\\t");
                        break;
                    case '\b':
                        builder.append("\\b");
                        break;
                    case '\f':
                        builder.append("\\f");
                        break;
                    case '"':
                        builder.append("\\\"");
                        break;
                    case '\\':
                        builder.append("\\\\");
                        break;
                    case ' ':
                        builder.append(' ');
                        break;
                    default: {
                        if (Character.isISOControl(ch) || Character.isWhitespace(ch) || !Character.isDefined(ch)) {
                            builder.append("\\u%04x".formatted((int) ch));
                        } else {
                            builder.append(ch);
                        }
                    }
                }

            }

        }

        builder.append('"');
        return builder.toString();
    }

    public static MeowRun toRun(String content) {
        return new MeowRun(
                content,
                false,
                MeowUnderline.NONE,
                null,
                MeowHighlight.NONE,
                false,
                null,
                null,
                List.of()
        );
    }

    private MeowUtils() {
    }
}
