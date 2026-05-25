module;
#include <iostream>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <memory>
export module FontDrawer;
import Types;
import Image;
import Color;

export class FontDrawer {
    UInt8Buffer fontDataBuffer;
    stbtt_fontinfo fontInfo{};
public:
    explicit FontDrawer(const String &path) {
        FILE* file = fopen(path.c_str(), "rb");
        if (!file) {
            throw std::runtime_error("Failed to open font file: " + path);
        }

        fseek(file, 0, SEEK_END);
        Int64 fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        fontDataBuffer = makeUInt8Buffer(static_cast<UInt32>(fileSize));
        size_t readBytes = fread(fontDataBuffer.get(), 1, fileSize, file);
        fclose(file);

        if (readBytes != static_cast<size_t>(fileSize)) {
            throw std::runtime_error("Failed to read font file: " + path);
        }

        // --- 核心修改部分 ---
        // 1. 获取 TTC 文件中第一个字体的偏移量（如果传入的是普通 ttf，该函数也会安全返回 0）
        Int32 fontOffset = stbtt_GetFontOffsetForIndex(fontDataBuffer.get(), 0);
        if (fontOffset < 0) {
            throw std::runtime_error("Failed to find font offset inside collection: " + path);
        }

        // 2. 传入计算好的偏移量进行初始化
        if (!stbtt_InitFont(&fontInfo, fontDataBuffer.get(), fontOffset)) {
            throw std::runtime_error("Failed to initialize font: " + path);
        }
    }
    ~FontDrawer() = default;

    // 2. 绘制字体的函数
    void drawText(String text, Color c, Float fontSize, Image* bufferImage) {
        UInt8* destPixels = bufferImage->getBuffer();

        Float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

        Int32 ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

        Int32 textHeight = static_cast<Int32>((ascent - descent) * scale);
        Int32 textWidth = 0;
        Int64 textLength = text.length();

        // 1. 预遍历：计算文本实际需要的绝对宽度
        for (Int64 i = 0; i < textLength; ++i) {
            Int32 advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, text[i], &advance, &lsb);
            textWidth += static_cast<Int32>(advance * scale);
            if (i + 1 < textLength) {
                textWidth += static_cast<Int32>(stbtt_GetCodepointKernAdvance(&fontInfo, text[i], text[i + 1]) * scale);
            }
        }

        // 2. 局部清屏：只清空即将要写入的前部分缓冲区区域
        UInt32 totalBytesToClear = static_cast<UInt32>(textWidth * textHeight * 4);
        memset(destPixels, 0, totalBytesToClear);

        Int32 currentX = 0;
        Int32 baselineY = static_cast<Int32>(ascent * scale);

        // 3. 遍历并实时绘制每个字符
        for (Int64 i = 0; i < textLength; ++i) {
            Int32 advance, lsb;
            stbtt_GetCodepointHMetrics(&fontInfo, text[i], &advance, &lsb);

            Int32 x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&fontInfo, text[i], scale, scale, &x0, &y0, &x1, &y1);

            // 计算当前字符在目标画布中的起始像素坐标
            Int32 charX = currentX + static_cast<Int32>(lsb * scale);
            Int32 charY = baselineY + y0;

            Int32 charW = x1 - x0;
            Int32 charH = y1 - y0;

            if (charW > 0 && charH > 0) {
                // 栈上固定的临时单通道缓冲区
                alignas(16) UInt8 monoCharBuffer[256 * 256] = { 0 };

                Int32 renderW = charW > 256 ? 256 : charW;
                Int32 renderH = charH > 256 ? 256 : charH;

                stbtt_MakeCodepointBitmap(&fontInfo, monoCharBuffer, renderW, renderH, renderW, scale, scale, text[i]);

                // 4. 写入外部缓冲区，此时 Stride 完全由当前计算出的 textWidth 决定
                for (Int32 cy = 0; cy < renderH; ++cy) {
                    for (Int32 cx = 0; cx < renderW; ++cx) {
                        UInt8 alpha = monoCharBuffer[cx + cy * renderW];
                        if (alpha > 0) {
                            Int32 destX = charX + cx;
                            Int32 destY = charY + cy;

                            // 按照计算出的文字总宽度作为行宽进行内存映射
                            Int64 idx = static_cast<Int64>(destX + destY * textWidth) * 4;
                            destPixels[idx + 0] = static_cast<UInt8>(c.r);
                            destPixels[idx + 1] = static_cast<UInt8>(c.g);
                            destPixels[idx + 2] = static_cast<UInt8>(c.b);
                            destPixels[idx + 3] = alpha;
                        }
                    }
                }
            }

            // 移动横坐标
            currentX += static_cast<Int32>(advance * scale);
            if (i + 1 < textLength) {
                currentX += static_cast<Int32>(stbtt_GetCodepointKernAdvance(&fontInfo, text[i], text[i + 1]) * scale);
            }
        }

        // 5. 将宽高作为返回值，写入 Image 结构体告知外部渲染器
        bufferImage->width = static_cast<UInt32>(textWidth);
        bufferImage->height = static_cast<UInt32>(textHeight);
    }
};