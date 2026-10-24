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
 * 底線樣式。主要用來區分「沒有底線」與「單底線（字串）」，
 * 其餘樣式保留給未來擴充。
 */
public enum MeowUnderline {
    NONE,
    SINGLE,
    DOUBLE,
    THICK,
    DOTTED,
    DASH,
    DOT_DASH,
    DOT_DOT_DASH,
    WAVE,
    OTHER;

    public static MeowUnderline from(String value) {
        if (value == null) {
            return NONE;
        }
        return switch (value) {
            case "single" -> SINGLE;
            case "double" -> DOUBLE;
            case "thick" -> THICK;
            case "dotted" -> DOTTED;
            case "dash" -> DASH;
            case "dotDash" -> DOT_DASH;
            case "dotDotDash" -> DOT_DOT_DASH;
            case "wave" -> WAVE;
            case "none" -> NONE;
            default -> OTHER;
        };
    }
}
