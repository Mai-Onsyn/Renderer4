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
            // throw std::runtime_error("Failed to open font file: " + path);
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
            // throw std::runtime_error("Failed to read font file: " + path);
            Log::error("Failed to read font file: %s", path);
            return;
        }

        // --- 核心修改部分 ---
        // 1. 获取 TTC 文件中第一个字体的偏移量（如果传入的是普通 ttf，该函数也会安全返回 0）
        Int32 fontOffset = stbtt_GetFontOffsetForIndex(fontDataBuffer.get(), 0);
        if (fontOffset < 0) {
            // throw std::runtime_error("Failed to find font offset inside collection: " + path);
            Log::error("Failed to find font offset inside collection: %s", path);
            return;
        }

        // 2. 传入计算好的偏移量进行初始化
        if (!stbtt_InitFont(&fontInfo, fontDataBuffer.get(), fontOffset)) {
            // throw std::runtime_error("Failed to initialize font: " + path);
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

        for (Int64 i = 0; i < textLength; ++i) {
            Int32 advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, text[i], &advance, &lsb);

            Int32 x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&fontInfo, text[i], scale, scale, &x0, &y0, &x1, &y1);

            Int32 charW = x1 - x0;
            Int32 charH = y1 - y0;

            if (charW > 512) charW = 512;
            if (charH > 512) charH = 512;

            if (charW > 0 && charH > 0) {
                Int32 charX = currentX + static_cast<Int32>(lsb * scale) + static_cast<Int32>(ox);
                Int32 charY = baselineY + y0 + static_cast<Int32>(oy);

                stbtt_MakeCodepointBitmap(&fontInfo, monoCharBuffer, charW, charH, charW, scale, scale, text[i]);

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
            if (i + 1 < textLength) {
                currentX += static_cast<Int32>(stbtt_GetCodepointKernAdvance(&fontInfo, text[i], text[i + 1]) * scale);
            }
        }
    }
};