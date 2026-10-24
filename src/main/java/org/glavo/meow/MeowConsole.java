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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UncheckedIOException;

/**
 * 主控台輸出／輸入的抽象層。
 *
 * <p>只有真正需要互動式輸入時才會載入 jline；一般批次執行或管線輸入
 * 直接使用 {@link System#out} 與 {@link BufferedReader}，可省下 jline
 * 約 0.4 秒的類別載入時間。</p>
 */
public final class MeowConsole implements AutoCloseable {

    private static final boolean INTERACTIVE = detectInteractive();

    private final BufferedReader plainReader = new BufferedReader(new InputStreamReader(System.in));

    private static boolean detectInteractive() {
        if ("1".equals(System.getenv("MEOW_PLAIN"))) {
            return false;
        }
        if ("1".equals(System.getenv("MEOW_JLINE"))) {
            return true;
        }
        try {
            var console = System.console();
            if (console == null) {
                return false;
            }
            // Console.isTerminal() 是 Java 22 才新增的方法；這裡用反射呼叫，
            // 讓專案仍能以 --release 21 編譯。
            try {
                var method = console.getClass().getMethod("isTerminal");
                return Boolean.TRUE.equals(method.invoke(console));
            } catch (NoSuchMethodException e) {
                // Java 21：System.console() 非 null 即代表有終端機
                return true;
            }
        } catch (Throwable ignored) {
            return false;
        }
    }

    public void print(String text) {
        System.out.print(text);
    }

    public void println(String text) {
        System.out.println(text);
    }

    public String readLine(String prompt) {
        if (INTERACTIVE) {
            return JlineConsole.readLine(prompt);
        }

        if (prompt != null && !prompt.isEmpty()) {
            System.out.print(prompt);
            System.out.flush();
        }

        try {
            String line = plainReader.readLine();
            return line == null ? "" : line;
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    @Override
    public void close() {
        if (INTERACTIVE) {
            JlineConsole.close();
        }
    }
}
