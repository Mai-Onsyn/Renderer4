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

        drawText(format("FPS = %.2f", getFPS()), 0, 0, frameBuffer->getBuffer());
        drawText(format("Resolution = %d*%d", width, height), 0, 27, frameBuffer->getBuffer());
        tripleBuffer.commit();
    }

    void drawText(const String& text, const UInt32 ox, const UInt32 oy, UInt8* screen) {
        fontDrawer.drawText(text, ox, oy, Color{255, 255, 255, 255}, 27.0, screen, width, height);
    }
};
