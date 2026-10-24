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

/**
 * 螢光標記樣式。用來取代 POI 的 {@code STHighlightColor.Enum}，
 * 只需要文件裡實際會出現的幾種顏色。
 */
public enum MeowHighlight {
    NONE,
    BLACK,
    BLUE,
    CYAN,
    GREEN,
    MAGENTA,
    RED,
    YELLOW,
    WHITE,
    LIGHT_GRAY,
    DARK_GRAY;

    public static MeowHighlight from(String value) {
        if (value == null || value.equals("none")) {
            return NONE;
        }
        return switch (value) {
            case "black" -> BLACK;
            case "blue" -> BLUE;
            case "cyan" -> CYAN;
            case "green" -> GREEN;
            case "magenta" -> MAGENTA;
            case "red" -> RED;
            case "yellow" -> YELLOW;
            case "white" -> WHITE;
            case "lightGray" -> LIGHT_GRAY;
            case "darkGray" -> DARK_GRAY;
            default -> NONE;
        };
    }
}
