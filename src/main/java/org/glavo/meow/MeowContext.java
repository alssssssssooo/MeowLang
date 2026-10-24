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

import org.glavo.meow.ast.MeowExpression;
import org.glavo.meow.value.MeowBuiltinFunction;
import org.glavo.meow.value.MeowBuiltinMacro;
import org.glavo.meow.value.MeowUnit;
import org.glavo.meow.value.MeowValue;

import java.io.IOException;
import java.nio.file.Path;
import java.text.MessageFormat;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public final class MeowContext {

    public static final MeowContext ROOT = new MeowContext();

    static {
        for (var macro : MeowBuiltinMacro.values()) {
            ROOT.setValue(macro.getMeow(), macro);
        }
        for (var function : MeowBuiltinFunction.values()) {
            ROOT.setValue(function.getMeow(), function);
        }
    }

    // ---------------

    private final MeowContext parent;
    private final MeowConsole console;
    private final Map<Meow, MeowValue> binding = new LinkedHashMap<>();


    // For ROOT
    private MeowContext() {
        this.parent = null;
        this.console = null;
    }

    public MeowContext(MeowContext parent) {
        this.parent = parent;
        this.console = parent.console;
    }

    public MeowContext(MeowConsole console) {
        this.parent = ROOT;
        this.console = console;
    }

    public MeowConsole getConsole() {
        return console;
    }

    public MeowValue getValue(Meow key) {
        MeowContext context = this;
        while (context != null) {
            MeowValue value = context.binding.get(key);
            if (value != null) {
                return value;
            } else {
                context = context.parent;
            }
        }

        return MeowUnit.UNIT;
    }

    public void setValue(Meow key, MeowValue value) {
        binding.put(key, value);
    }

    public void evalFile(Path file) throws IOException {
        evalDocument(MeowDocxReader.read(file));
    }

    public void evalDocument(List<List<MeowRun>> paragraphs) {
        for (List<MeowRun> paragraph : paragraphs) {
            MeowExpression expr = MeowParser.parse(paragraph, true);
            log(">>> [{0}]", expr.toDebugString());
            MeowValue result = expr.eval(this);
            log("|   >>> return {0} in {1}", result.toDebugString(this), this);
        }
    }

    // For debug...
    // Although it is very simple, it is enough for early development :)

    public void log(String pattern, Object... args) {
        if (Meow.DEBUG) {
            console.println("[DEBUG] " + MessageFormat.format(pattern, args));
        }
    }

    @Override
    public String toString() {
        return "Context@" + Integer.toHexString(hashCode());
    }
}
