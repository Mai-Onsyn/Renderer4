module;
#include <iostream>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <memory>
export module FontDrawer;
import Types;
import Image;
import Color;
import Time;
import Logger;

export class FontDrawer {
    UInt8Buffer fontDataBuffer;
    stbtt_fontinfo fontInfo{};
public:
    explicit FontDrawer(const String &path) {
        FILE* file = fopen(path.c_str(), "rb");
        if (!file) {
            Log::error("Failed to open font file: %s", path);
            return;
        }

        fseek(file, 0, SEEK_END);
        Int64 fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        fontDataBuffer = makeUInt8Buffer(static_cast<UInt32>(fileSize));
        size_t readBytes = fread(fontDataBuffer.get(), 1, fileSize, file);
        fclose(file);

        if (readBytes != static_cast<size_t>(fileSize)) {
            Log::error("Failed to read font file: %s", path);
            return;
        }

        Int32 fontOffset = stbtt_GetFontOffsetForIndex(fontDataBuffer.get(), 0);
        if (fontOffset < 0) {
            Log::error("Failed to find font offset inside collection: %s", path);
            return;
        }

        if (!stbtt_InitFont(&fontInfo, fontDataBuffer.get(), fontOffset)) {
            Log::error("Failed to initialize font: %s", path);
            return;
        }
    }
    ~FontDrawer() = default;

    alignas(16) UInt8 monoCharBuffer[512 * 512];
    void drawText(
        const String& text, const UInt32 ox, const UInt32 oy,
        const Color& c, Float fontSize,
        UInt8* screen, const UInt32 screenWidth, const UInt32 screenHeight
    ) {
        const Float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

        Int32 ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

        Int32 baselineY = static_cast<Int32>(ascent * scale);
        Int32 currentX = 0;
        Int64 textLength = text.length();

        const UInt32 fontColorR = c.r;
        const UInt32 fontColorG = c.g;
        const UInt32 fontColorB = c.b;
        const Int32 sWidth = static_cast<Int32>(screenWidth);
        const Int32 sHeight = static_cast<Int32>(screenHeight);

        Int64 i = 0;
        while (i < textLength) {
            // --- 核心修改：UTF-8 解码为 Unicode 码点 ---
            Int32 codepoint = 0;
            UInt8 c1 = static_cast<UInt8>(text[i]);

            if (c1 < 0x80) {
                codepoint = c1;
                i += 1;
            } else if ((c1 & 0xE0) == 0xC0) {
                if (i + 1 >= textLength) break;
                UInt8 c2 = static_cast<UInt8>(text[i + 1]);
                codepoint = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
                i += 2;
            } else if ((c1 & 0xF0) == 0xE0) {
                if (i + 2 >= textLength) break;
                UInt8 c2 = static_cast<UInt8>(text[i + 1]);
                UInt8 c3 = static_cast<UInt8>(text[i + 2]);
                codepoint = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                i += 3;
            } else if ((c1 & 0xF8) == 0xF0) {
                if (i + 3 >= textLength) break;
                UInt8 c2 = static_cast<UInt8>(text[i + 1]);
                UInt8 c3 = static_cast<UInt8>(text[i + 2]);
                UInt8 c4 = static_cast<UInt8>(text[i + 3]);
                codepoint = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
                i += 4;
            } else {
                i += 1; // 无效字节，跳过
                continue;
            }

            // --- 后续所有接口一律传入解码后的 codepoint 代替 text[i] ---
            Int32 advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &advance, &lsb);

            Int32 x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&fontInfo, codepoint, scale, scale, &x0, &y0, &x1, &y1);

            Int32 charW = x1 - x0;
            Int32 charH = y1 - y0;

            if (charW > 512) charW = 512;
            if (charH > 512) charH = 512;

            if (charW > 0 && charH > 0) {
                Int32 charX = currentX + static_cast<Int32>(lsb * scale) + static_cast<Int32>(ox);
                Int32 charY = baselineY + y0 + static_cast<Int32>(oy);

                stbtt_MakeCodepointBitmap(&fontInfo, monoCharBuffer, charW, charH, charW, scale, scale, codepoint);

                Int32 srcStartX = 0;
                Int32 srcStartY = 0;

                if (charY < 0) { srcStartY = -charY; charH -= srcStartY; charY = 0; }
                if (charX < 0) { srcStartX = -charX; charW -= srcStartX; charX = 0; }
                if (charY + charH > sHeight) { charH = sHeight - charY; }
                if (charX + charW > sWidth) { charW = sWidth - charX; }

                if (charW > 0 && charH > 0) {
                    const UInt8* srcPtrRow = monoCharBuffer + (srcStartY * (charW + srcStartX)) + srcStartX;
                    UInt8* destPtrRow = screen + (charY * sWidth + charX) * 4;

                    Int32 srcStride = charW + srcStartX;

                    for (Int32 cy = 0; cy < charH; ++cy) {
                        const UInt8* srcPtr = srcPtrRow;
                        UInt8* destPtr = destPtrRow;

                        for (Int32 cx = 0; cx < charW; ++cx) {
                            UInt8 alpha = *srcPtr++;

                            if (alpha == 255) {
                                destPtr[0] = static_cast<UInt8>(fontColorR);
                                destPtr[1] = static_cast<UInt8>(fontColorG);
                                destPtr[2] = static_cast<UInt8>(fontColorB);
                                destPtr[3] = 255;
                            } else if (alpha > 0) {
                                UInt32 invAlpha = 255 - alpha;
                                destPtr[0] = static_cast<UInt8>((destPtr[0] * invAlpha + fontColorR * alpha) >> 8);
                                destPtr[1] = static_cast<UInt8>((destPtr[1] * invAlpha + fontColorG * alpha) >> 8);
                                destPtr[2] = static_cast<UInt8>((destPtr[2] * invAlpha + fontColorB * alpha) >> 8);
                                destPtr[3] = 255;
                            }
                            destPtr += 4;
                        }
                        srcPtrRow += srcStride;
                        destPtrRow += sWidth * 4;
                    }
                }
            }

            currentX += static_cast<Int32>(advance * scale);

            // 字距微调 (Kern) 同样需要处理下一个字符的解码
            if (i < textLength) {
                Int32 nextCodepoint = 0;
                UInt8 n1 = static_cast<UInt8>(text[i]);
                if (n1 < 0x80) nextCodepoint = n1;
                else if ((n1 & 0xE0) == 0xC0 && i + 1 < textLength) nextCodepoint = ((n1 & 0x1F) << 6) | (static_cast<UInt8>(text[i + 1]) & 0x3F);
                else if ((n1 & 0xF0) == 0xE0 && i + 2 < textLength) nextCodepoint = ((n1 & 0x0F) << 12) | ((static_cast<UInt8>(text[i + 1]) & 0x3F) << 6) | (static_cast<UInt8>(text[i + 2]) & 0x3F);
                else if ((n1 & 0xF0) == 0xF0 && i + 3 < textLength) nextCodepoint = ((n1 & 0x07) << 18) | ((static_cast<UInt8>(text[i + 1]) & 0x3F) << 12) | ((static_cast<UInt8>(text[i + 2]) & 0x3F) << 6) | (static_cast<UInt8>(text[i + 3]) & 0x3F);

                if (nextCodepoint > 0) {
                    currentX += static_cast<Int32>(stbtt_GetCodepointKernAdvance(&fontInfo, codepoint, nextCodepoint) * scale);
                }
            }
        }
    }

    Float getTextWidth(const String& text, const Float fontSize) const {
        const Float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);
        Int64 textLength = text.length();
        Int32 totalAdvance = 0;

        Int64 i = 0;
        while (i < textLength) {
            // 1. 同步使用标准的 UTF-8 解码状态机，确保中英文宽度都准确
            Int32 codepoint = 0;
            UInt8 c1 = static_cast<UInt8>(text[i]);

            if (c1 < 0x80) {
                codepoint = c1;
                i += 1;
            } else if ((c1 & 0xE0) == 0xC0) {
                if (i + 1 >= textLength) break;
                UInt8 c2 = static_cast<UInt8>(text[i + 1]);
                codepoint = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
                i += 2;
            } else if ((c1 & 0xF0) == 0xE0) {
                if (i + 2 >= textLength) break;
                UInt8 c2 = static_cast<UInt8>(text[i + 1]);
                UInt8 c3 = static_cast<UInt8>(text[i + 2]);
                codepoint = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                i += 3;
            } else if ((c1 & 0xF8) == 0xF0) {
                if (i + 3 >= textLength) break;
                UInt8 c2 = static_cast<UInt8>(text[i + 1]);
                UInt8 c3 = static_cast<UInt8>(text[i + 2]);
                UInt8 c4 = static_cast<UInt8>(text[i + 3]);
                codepoint = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
                i += 4;
            } else {
                i += 1;
                continue;
            }

            // 2. 仅获取当前字元的步进宽度（Advance Width）
            Int32 advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, codepoint, &advance, &lsb);
            totalAdvance += advance;

            // 3. 计算字距微调（Kerning）
            if (i < textLength) {
                Int32 nextCodepoint = 0;
                UInt8 n1 = static_cast<UInt8>(text[i]);
                if (n1 < 0x80) nextCodepoint = n1;
                else if ((n1 & 0xE0) == 0xC0 && i + 1 < textLength) nextCodepoint = ((n1 & 0x1F) << 6) | (static_cast<UInt8>(text[i + 1]) & 0x3F);
                else if ((n1 & 0xF0) == 0xE0 && i + 2 < textLength) nextCodepoint = ((n1 & 0x0F) << 12) | ((static_cast<UInt8>(text[i + 1]) & 0x3F) << 6) | (static_cast<UInt8>(text[i + 2]) & 0x3F);
                else if ((n1 & 0xF0) == 0xF0 && i + 3 < textLength) nextCodepoint = ((n1 & 0x07) << 18) | ((static_cast<UInt8>(text[i + 1]) & 0x3F) << 12) | ((static_cast<UInt8>(text[i + 2]) & 0x3F) << 6) | (static_cast<UInt8>(text[i + 3]) & 0x3F);

                if (nextCodepoint > 0) {
                    totalAdvance += stbtt_GetCodepointKernAdvance(&fontInfo, codepoint, nextCodepoint);
                }
            }
        }

        // 4. 统一应用缩放比例，返回最终的像素宽度
        return static_cast<Float>(totalAdvance) * scale;
    }
};