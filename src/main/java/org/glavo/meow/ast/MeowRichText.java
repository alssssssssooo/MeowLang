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
package org.glavo.meow.ast;

import org.glavo.meow.MeowContext;
import org.glavo.meow.MeowRun;
import org.glavo.meow.MeowUtils;
import org.glavo.meow.value.MeowText;
import org.glavo.meow.value.MeowValue;

import java.util.List;

public record MeowRichText(List<MeowRun> content) implements MeowExpression {
    @Override
    public MeowValue eval(MeowContext context) {
        return new MeowText(content);
    }

    @Override
    public String toDebugString() {
        return MeowUtils.toDebugString(content);
    }
}
