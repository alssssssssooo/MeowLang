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

import kala.ansi.AnsiString;
import org.glavo.meow.Meow;
import org.glavo.meow.MeowContext;
import org.glavo.meow.MeowHighlight;
import org.glavo.meow.MeowRun;
import org.glavo.meow.MeowUnderline;
import org.glavo.meow.MeowUtils;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.util.List;

@SuppressWarnings("resource")
public enum MeowBuiltinFunction implements MeowFunction {
    PRINT("print", 0xEE0000) {
        @Override
        public MeowValue applyValues(MeowContext context, List<MeowValue> args) {
            StringBuilder builder = new StringBuilder();

            boolean first = true;
            for (MeowValue value : args) {
                if (first) {
                    first = false;
                } else {
                    builder.append('\n');
                }

                if (value instanceof MeowText(List<MeowRun> content)) {
                    for (MeowRun node : content) {
                        var text = AnsiString.ofPlain(node.text());
                        if (!text.isEmpty()) {
                            if (node.bold()) {
                                text = text.overlay(AnsiString.Bold.On);
                            }
                            if (node.underline() != MeowUnderline.NONE) {
                                text = text.overlay(AnsiString.Underlined.On);
                            }
                            if (node.color() != null) {
                                int rgb = node.color();
                                if (rgb != 0) {
                                    text = text.overlay(AnsiString.Color.True(rgb >> 16, (rgb >> 8) & 0xFF, rgb & 0xFF));
                                }
                            }
                            if (node.highlight() != MeowHighlight.NONE) {
                                AnsiString.Attribute backgroundColor = MeowUtils.attributeOf(node.highlight());
                                if (backgroundColor != null) {
                                    text = text.overlay(backgroundColor);
                                }
                            }

                            if (node.italic()) {
                                // Why Kala Ansi unsupported Italic?
                                builder.append("\u001B[3m");
                            }

                            builder.append(text);

                            if (node.italic()) {
                                // Why Kala Ansi unsupported Italic?
                                builder.append("\u001b[0m");
                            }
                        }

                        for (byte[] data : node.pictures()) {
                            BufferedImage image;
                            try {
                                image = ImageIO.read(new ByteArrayInputStream(data));
                            } catch (IOException e) {
                                throw new UncheckedIOException(e);
                            }
                            if (image == null) {
                                continue;
                            }

                            if (!builder.isEmpty() && builder.charAt(builder.length() - 1) != '\n') {
                                builder.append("\n");
                            }

                            // TODO: Scale image to fit terminal width
                            for (int y = 0; y * 2 < image.getHeight(); y++) {
                                for (int x = 0; x < image.getWidth(); x++) {
                                    MeowUtils.appendTrueRgbEscape(builder, false, image.getRGB(x, y * 2));
                                    MeowUtils.appendTrueRgbEscape(builder, true, image.getRGB(x, y * 2 + 1));
                                    builder.append('▀');
                                }

                                builder.append("\u001b[0m\n");
                            }
                        }
                    }
                } else {
                    builder.append(value.toDisplayString(context));
                }
            }

            context.getConsole().println(builder.toString());
            return MeowUnit.UNIT;
        }
    },

    readline("readline", 0x00B0F0) {
        @Override
        public MeowValue applyValues(MeowContext context, List<MeowValue> args) {
            String result;
            if (args.isEmpty()) {
                result = context.getConsole().readLine(null);
            } else if (args.size() == 1) {
                if (args.getFirst() instanceof MeowText prompt) {
                    result = context.getConsole().readLine(prompt.toDisplayString(context));
                } else {
                    throw new IllegalArgumentException("Expected a single text argument for prompt, but got: " + args.getFirst().toDisplayString(context));
                }
            } else {
                throw new IllegalArgumentException("Expected 0 or 1 arguments, but got: " + args.size());
            }

            return new MeowText(List.of(MeowUtils.toRun(result)));
        }
    };

    private final String name;
    private final Meow meow;

    MeowBuiltinFunction(String name, int color) {
        this(name, color, MeowHighlight.YELLOW);
    }

    MeowBuiltinFunction(String name, int color, MeowHighlight highlightColor) {
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
    public abstract MeowValue applyValues(MeowContext context, List<MeowValue> args);
}
