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

import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamConstants;
import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Path;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/**
 * 輕量 docx 讀取器。只使用 {@link java.util.zip.ZipFile} 與 StAX，
 * 直接解析 {@code word/document.xml}，不需要 Apache POI。
 *
 * <p>支援一般段落、粗體、斜體、底線、顏色、字級、螢光標記，
 * 以及 DrawingML／VML 的內嵌圖片。</p>
 */
public final class MeowDocxReader {

    private static final String NS_W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private static final String NS_W_STRICT = "http://purl.oclc.org/ooxml/wordprocessingml/main";

    private static final XMLInputFactory FACTORY = XMLInputFactory.newFactory();

    static {
        // 避免 XXE 與外部實體
        FACTORY.setProperty(XMLInputFactory.SUPPORT_DTD, false);
        FACTORY.setProperty(XMLInputFactory.IS_SUPPORTING_EXTERNAL_ENTITIES, false);
    }

    public static List<List<MeowRun>> read(Path file) throws IOException {
        try (ZipFile zip = new ZipFile(file.toFile())) {
            ZipEntry entry = zip.getEntry("word/document.xml");
            if (entry == null) {
                throw new IOException("不是有效的 docx 檔（找不到 word/document.xml）：" + file);
            }

            Map<String, byte[]> relationships = readRelationships(zip);

            try (InputStream input = zip.getInputStream(entry)) {
                return parseDocument(input, relationships);
            } catch (XMLStreamException e) {
                throw new IOException("解析 word/document.xml 失敗：" + file, e);
            }
        }
    }

    // ---------------------------------------------------------------------
    // 關聯檔（word/_rels/document.xml.rels）：rId -> 圖片位元組
    // ---------------------------------------------------------------------

    private static Map<String, byte[]> readRelationships(ZipFile zip) throws IOException {
        Map<String, byte[]> result = new LinkedHashMap<>();

        ZipEntry entry = zip.getEntry("word/_rels/document.xml.rels");
        if (entry == null) {
            return result;
        }

        try (InputStream input = zip.getInputStream(entry)) {
            XMLStreamReader reader = FACTORY.createXMLStreamReader(input);
            try {
                while (reader.hasNext()) {
                    if (reader.next() == XMLStreamConstants.START_ELEMENT
                            && "Relationship".equals(reader.getLocalName())) {
                        String id = attribute(reader, "Id");
                        String target = attribute(reader, "Target");
                        String type = attribute(reader, "Type");

                        if (id != null && target != null && type != null && type.endsWith("/image")) {
                            ZipEntry media = zip.getEntry(resolveTarget(target));
                            if (media != null) {
                                try (InputStream in = zip.getInputStream(media)) {
                                    result.put(id, in.readAllBytes());
                                }
                            }
                        }
                    }
                }
            } finally {
                reader.close();
            }
        } catch (XMLStreamException e) {
            throw new IOException("解析 document.xml.rels 失敗", e);
        }

        return result;
    }

    private static String resolveTarget(String target) {
        String path = target.startsWith("/") ? target.substring(1) : "word/" + target;

        ArrayDeque<String> parts = new ArrayDeque<>();
        for (String part : path.split("/")) {
            if (part.isEmpty() || part.equals(".")) {
                continue;
            }
            if (part.equals("..")) {
                if (!parts.isEmpty()) {
                    parts.removeLast();
                }
            } else {
                parts.addLast(part);
            }
        }
        return String.join("/", parts);
    }

    // ---------------------------------------------------------------------
    // document.xml
    // ---------------------------------------------------------------------

    private static List<List<MeowRun>> parseDocument(InputStream input, Map<String, byte[]> relationships)
            throws XMLStreamException {
        List<List<MeowRun>> paragraphs = new ArrayList<>();
        List<MeowRun> currentParagraph = null;

        RunBuilder run = new RunBuilder();
        boolean inRun = false;
        boolean inText = false;
        boolean inDrawing = false;

        XMLStreamReader reader = FACTORY.createXMLStreamReader(input);
        try {
            while (reader.hasNext()) {
                int event = reader.next();

                switch (event) {
                    case XMLStreamConstants.START_ELEMENT -> {
                        switch (reader.getLocalName()) {
                            case "p" -> {
                                if (isWord(reader)) {
                                    currentParagraph = new ArrayList<>();
                                }
                            }
                            case "r" -> {
                                if (isWord(reader)) {
                                    run.reset();
                                    inRun = true;
                                }
                            }
                            case "b" -> {
                                if (inRun) {
                                    run.bold = onOff(reader);
                                }
                            }
                            case "i" -> {
                                if (inRun) {
                                    run.italic = onOff(reader);
                                }
                            }
                            case "u" -> {
                                if (inRun) {
                                    String value = attribute(reader, "val");
                                    run.underline = value == null
                                            ? MeowUnderline.SINGLE
                                            : MeowUnderline.from(value);
                                }
                            }
                            case "color" -> {
                                if (inRun) {
                                    run.color = parseColor(attribute(reader, "val"));
                                }
                            }
                            case "sz" -> {
                                if (inRun) {
                                    String value = attribute(reader, "val");
                                    if (value != null) {
                                        try {
                                            run.fontSize = Double.parseDouble(value) / 2.0;
                                        } catch (NumberFormatException ignored) {
                                        }
                                    }
                                }
                            }
                            case "highlight" -> {
                                if (inRun) {
                                    run.highlight = MeowHighlight.from(attribute(reader, "val"));
                                }
                            }
                            case "rFonts" -> {
                                if (inRun) {
                                    run.font = attribute(reader, "ascii");
                                }
                            }
                            case "t" -> {
                                if (inRun && isWord(reader)) {
                                    inText = true;
                                }
                            }
                            case "tab" -> {
                                if (inRun) {
                                    run.text.append('\t');
                                }
                            }
                            case "br", "cr" -> {
                                if (inRun) {
                                    run.text.append('\n');
                                }
                            }
                            case "drawing" -> {
                                if (isWord(reader)) {
                                    inDrawing = true;
                                }
                            }
                            case "blip" -> {
                                if (inDrawing) {
                                    addPicture(run, relationships, attribute(reader, "embed"));
                                }
                            }
                            case "imagedata" -> {
                                if (inDrawing) {
                                    addPicture(run, relationships, attribute(reader, "id"));
                                }
                            }
                            default -> {
                            }
                        }
                    }

                    case XMLStreamConstants.END_ELEMENT -> {
                        switch (reader.getLocalName()) {
                            case "t" -> inText = false;
                            case "r" -> {
                                if (inRun && isWord(reader)) {
                                    if (!run.text.isEmpty() || !run.pictures.isEmpty()) {
                                        if (currentParagraph != null) {
                                            currentParagraph.add(run.build());
                                        }
                                    }
                                    inRun = false;
                                }
                            }
                            case "p" -> {
                                if (isWord(reader) && currentParagraph != null) {
                                    paragraphs.add(currentParagraph);
                                    currentParagraph = null;
                                }
                            }
                            case "drawing" -> inDrawing = false;
                            default -> {
                            }
                        }
                    }

                    case XMLStreamConstants.CHARACTERS, XMLStreamConstants.CDATA -> {
                        if (inText) {
                            run.text.append(reader.getText());
                        }
                    }

                    default -> {
                    }
                }
            }
        } finally {
            reader.close();
        }

        return paragraphs;
    }

    private static void addPicture(RunBuilder run, Map<String, byte[]> relationships, String id) {
        if (id == null) {
            return;
        }
        byte[] data = relationships.get(id);
        if (data != null) {
            run.pictures.add(data);
        }
    }

    private static boolean isWord(XMLStreamReader reader) {
        String namespace = reader.getNamespaceURI();
        return NS_W.equals(namespace) || NS_W_STRICT.equals(namespace);
    }

    private static String attribute(XMLStreamReader reader, String localName) {
        for (int i = 0; i < reader.getAttributeCount(); i++) {
            if (localName.equals(reader.getAttributeLocalName(i))) {
                return reader.getAttributeValue(i);
            }
        }
        return null;
    }

    private static boolean onOff(XMLStreamReader reader) {
        String value = attribute(reader, "val");
        return value == null || !(value.equals("0") || value.equals("off") || value.equalsIgnoreCase("false"));
    }

    private static Integer parseColor(String value) {
        if (value == null || value.equals("auto")) {
            return null;
        }
        try {
            return Integer.parseInt(value, 16);
        } catch (NumberFormatException e) {
            return null;
        }
    }

    private static final class RunBuilder {
        StringBuilder text = new StringBuilder();
        boolean italic;
        boolean bold;
        MeowUnderline underline = MeowUnderline.NONE;
        Integer color;
        MeowHighlight highlight = MeowHighlight.NONE;
        String font;
        Double fontSize;
        List<byte[]> pictures = new ArrayList<>();

        void reset() {
            text.setLength(0);
            italic = false;
            bold = false;
            underline = MeowUnderline.NONE;
            color = null;
            highlight = MeowHighlight.NONE;
            font = null;
            fontSize = null;
            pictures = new ArrayList<>();
        }

        MeowRun build() {
            return new MeowRun(text.toString(), italic, underline, color, highlight, bold, font, fontSize, pictures);
        }
    }

    private MeowDocxReader() {
    }
}
