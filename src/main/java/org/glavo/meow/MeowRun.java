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

import java.util.List;

/**
 * 一個 Word run 的輕量表示，取代原本的 {@code XWPFRun}。
 * 只保留 Meow 語言實際會用到的格式屬性。
 */
public record MeowRun(
        String text,
        boolean italic,
        MeowUnderline underline,
        Integer color,
        MeowHighlight highlight,
        boolean bold,
        String font,
        Double fontSize,
        List<byte[]> pictures
) {
    public MeowRun {
        if (text == null) {
            text = "";
        }
        if (underline == null) {
            underline = MeowUnderline.NONE;
        }
        if (highlight == null) {
            highlight = MeowHighlight.NONE;
        }
        if (pictures == null) {
            pictures = List.of();
        }
    }
}
