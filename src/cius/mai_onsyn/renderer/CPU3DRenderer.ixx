module;
#include <cmath>
export module CPU3DRenderer;
import Renderer;
import Types;
import FrameBuffer;
import Color;
import Image;
import Functions;
import Format;
import Time;

export class CPU3DRenderer final : public Renderer {
public:
    CPU3DRenderer(const UInt32 width, const UInt32 height) : Renderer(width, height) {}

    const UInt64 CYCLE_DURATION_MS = 5000;
    void renderFrame() override {
        UInt32 h = height;
        UInt32 w = width;

        UInt64 currentTime = millisTime();
        Float timeFactor = static_cast<Float>(currentTime % CYCLE_DURATION_MS) / static_cast<Float>(CYCLE_DURATION_MS);

        auto frameBuffer = tripleBuffer.getRenderBuffer();
        auto buffer = frameBuffer->getBuffer();

        Float invW = 1.0f / static_cast<Float>(w);
        Float invH = 1.0f / static_cast<Float>(h);
        for (UInt32 y = 0; y < h; ++y) {
            const Float gradY = static_cast<Float>(y) * invH;

            for (UInt32 x = 0; x < w; ++x) {
                const Float gradX = static_cast<Float>(x) * invW;

                Float hue = timeFactor + gradX + gradY;

                hue -= static_cast<Int32>(hue);
                if (hue < 0.0f) hue += 1.0f;

                Float r = 0.0f, g = 0.0f, b = 0.0f;
                const Float h6 = hue * 6.0f;
                const auto sector = static_cast<Int32>(h6);
                const auto fract = h6 - static_cast<Float>(sector);

                switch (sector) {
                    case 0: r = 1.0f;         g = fract;        b = 0.0f;         break; // 红 -> 黄
                    case 1: r = 1.0f - fract; g = 1.0f;         b = 0.0f;         break; // 黄 -> 绿
                    case 2: r = 0.0f;         g = 1.0f;         b = fract;        break; // 绿 -> 青
                    case 3: r = 0.0f;         g = 1.0f - fract; b = 1.0f;         break; // 青 -> 蓝
                    case 4: r = fract;        g = 0.0f;         b = 1.0f;         break; // 蓝 -> 品红
                    default:r = 1.0f;         g = 0.0f;         b = 1.0f - fract; break; // 品红 -> 红
                }

                buffer[0] = static_cast<UInt8>(r * 255.0f);
                buffer[1] = static_cast<UInt8>(g * 255.0f);
                buffer[2] = static_cast<UInt8>(b * 255.0f);
                buffer[3] = 255;
                buffer += 4;
            }
        }

        drawText(format("FPS = %.2f", getFPS()), 0, 0, frameBuffer->getBuffer(), textBuffer);
        drawText(format("Resolution = %d*%d", width, height), 0, textBuffer->height, frameBuffer->getBuffer(), textBuffer);
        tripleBuffer.commit();
    }

    void drawText(const String& text, const UInt32 ox, const UInt32 oy, UInt8* screen, Image* buffer) {
        fontDrawer.drawText(text, Color{255, 255, 255, 255}, 27.0, buffer);

        const UInt8* imagePtr = buffer->getBuffer();
        for (UInt32 y = 0; y < buffer->height; y++) {
            if (y + oy >= height) break;
            const UInt32 offestYl = (y + oy) * width;
            const UInt32 yl = y * buffer->width;
            for (UInt32 x = 0; x < buffer->width; x++) {
                if (x + ox >= width) break;

                const UInt32 screenOffset = (x + ox + offestYl) << 2;
                const UInt32 imageOffset = (x + yl) << 2;
                if (const UInt8 alpha = imagePtr[imageOffset + 3]; alpha == 255) {
                    screen[screenOffset + 0] = imagePtr[imageOffset + 0];
                    screen[screenOffset + 1] = imagePtr[imageOffset + 1];
                    screen[screenOffset + 2] = imagePtr[imageOffset + 2];
                    screen[screenOffset + 3] = 255;
                }
                else if (alpha > 0) {
                    screen[screenOffset + 0] = (screen[screenOffset + 0] * (255 - alpha) + imagePtr[imageOffset + 0] * alpha) >> 8;
                    screen[screenOffset + 1] = (screen[screenOffset + 1] * (255 - alpha) + imagePtr[imageOffset + 1] * alpha) >> 8;
                    screen[screenOffset + 2] = (screen[screenOffset + 2] * (255 - alpha) + imagePtr[imageOffset + 2] * alpha) >> 8;
                    screen[screenOffset + 3] = 255;
                }
            }
        }
    }
};
