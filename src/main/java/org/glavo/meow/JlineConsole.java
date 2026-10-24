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

import org.jline.reader.LineReader;
import org.jline.reader.LineReaderBuilder;
import org.jline.terminal.Terminal;
import org.jline.terminal.TerminalBuilder;

import java.io.IOException;
import java.io.UncheckedIOException;

/**
 * jline 的延遲載入包裝。只有在互動式終端機執行時才會被初始化，
 * 因此一般批次執行不會載入 jline 的類別。
 */
final class JlineConsole {

    private static Terminal terminal;
    private static LineReader reader;

    static synchronized String readLine(String prompt) {
        if (reader == null) {
            try {
                terminal = TerminalBuilder.builder()
                        .system(true)
                        .build();
            } catch (IOException e) {
                throw new UncheckedIOException(e);
            }
            reader = LineReaderBuilder.builder()
                    .terminal(terminal)
                    .build();
        }

        return prompt == null || prompt.isEmpty()
                ? reader.readLine()
                : reader.readLine(prompt);
    }

    static synchronized void close() {
        if (terminal != null) {
            try {
                terminal.close();
            } catch (IOException ignored) {
            }
            terminal = null;
            reader = null;
        }
    }

    private JlineConsole() {
    }
}
