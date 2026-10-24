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

import org.glavo.meow.Meow;
import org.glavo.meow.MeowContext;
import org.glavo.meow.MeowHighlight;
import org.glavo.meow.MeowUtils;
import org.glavo.meow.ast.MeowExpression;
import org.glavo.meow.ast.MeowExpressionList;
import org.glavo.meow.ast.MeowIdentifier;

import java.util.List;

public enum MeowBuiltinMacro implements MeowMacro {
    LET("let", 0x00B0F0) {
        @Override
        public MeowValue process(MeowContext context, List<MeowExpression> args) {
            MeowUtils.checkArgsCount(args, 2);
            MeowExpression name = args.getFirst();
            if (!(name instanceof MeowIdentifier(Meow meow))) {
                throw new IllegalArgumentException("Expected identifier, but got " + name);
            }

            MeowValue value = args.get(1).eval(context);
            context.setValue(meow, value);
            return value;
        }
    },
    LAMBDA("lambda", 0x00B050) {
        @Override
        public MeowValue process(MeowContext context, List<MeowExpression> args) {
            if (args.isEmpty()) {
                throw new IllegalArgumentException("Expected at least 2 arguments, but got 0" + ": " + args);
            }

            List<Meow> parameters;
            if (args.getFirst() instanceof MeowIdentifier(Meow meow)) {
                parameters = List.of(meow);
            } else if (args.getFirst() instanceof MeowExpressionList(List<MeowExpression> nodes)) {
                parameters = nodes.stream().map(node -> {
                    if (!(node instanceof MeowIdentifier(Meow meow))) {
                        throw new IllegalArgumentException("Expected identifier, but got " + node);
                    }
                    return meow;
                }).toList();
            } else {
                throw new IllegalArgumentException("Expected parameter list, but got " + args.getFirst());
            }

            return new MeowLambda(context, parameters, args.subList(1, args.size()));
        }
    },
    INTEGER("integer", 0x00000000) { // TODO
        @Override
        public MeowValue process(MeowContext context, List<MeowExpression> args) {
            MeowUtils.checkArgsCount(args, 1);
            if (args.getFirst() instanceof MeowIdentifier(Meow meow)) {
                Double fontSize = meow.getFontSize();
                if (fontSize == null) {
                    throw new IllegalArgumentException("Font size not set");
                }
                return new MeowIntegerValue(fontSize.longValue());
            } else {
                throw new IllegalArgumentException("Expected an identifier, but got " + args.getFirst());
            }
        }
    };

    private final String name;
    private final Meow meow;

    MeowBuiltinMacro(String name, int color) {
        this(name, color, MeowHighlight.LIGHT_GRAY);
    }

    MeowBuiltinMacro(String name, int color, MeowHighlight highlightColor) {
        this.name = name;
        this.meow = Meow.builtin(color, highlightColor);
    }

    @Override
    public String getName() {
        return name;
    }

    public Meow getMeow() {
        return meow;
    }

    @Override
    public abstract MeowValue process(MeowContext context, List<MeowExpression> args);
}
