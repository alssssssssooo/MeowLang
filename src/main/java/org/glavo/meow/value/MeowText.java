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
package org.glavo.meow.value;

import org.glavo.meow.MeowContext;
import org.glavo.meow.MeowRun;
import org.glavo.meow.MeowUtils;
import org.glavo.meow.ast.MeowExpression;

import java.util.ArrayList;
import java.util.List;

public record MeowText(List<MeowRun> content) implements MeowFunction {

    @Override
    public String getName() {
        return "text@" + Integer.toHexString(System.identityHashCode(this));
    }

    @Override
    public MeowValue applyValues(MeowContext context, List<MeowValue> args) {
        List<MeowRun> results = new ArrayList<>(this.content.size() + args.size());
        results.addAll(this.content);
        for (MeowValue arg : args) {
            if (arg instanceof MeowText(List<MeowRun> argContent)) {
                results.addAll(argContent);
            } else {
                results.add(MeowUtils.toRun(arg.toDisplayString(context)));
            }
        }
        return new MeowText(results);
    }

    @Override
    public String toDebugString(MeowContext context) {
        return MeowUtils.toDebugString(content);
    }

    @Override
    public String toDisplayString(MeowContext context) {
        StringBuilder builder = new StringBuilder();
        for (MeowRun run : content) {
            builder.append(run.text());
        }
        return builder.toString();
    }
}
