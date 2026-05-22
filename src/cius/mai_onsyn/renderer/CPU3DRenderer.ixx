module;
#include <cmath>
#include <cstring>
export module CPU3DRenderer;
import Renderer;
import Types;
import FrameBuffer;
import Color;

export class CPU3DRenderer final : public Renderer {
public:
    CPU3DRenderer(const UInt32 width, const UInt32 height) : Renderer(width, height) {}

    Float index = 0;
    void renderFrame() override {

        UInt32 h = main.height;
        UInt32 w = main.width;
        Float offset = std::sin(index);

        auto buffer = main.getBuffer();
        for (UInt32 y = 0; y < h; ++y) {
            for (UInt32 x = 0; x < w; ++x) {
                Float drx = map(static_cast<Float>(x) / static_cast<Float>(w) + offset);
                Float dry = map(static_cast<Float>(y) / static_cast<Float>(h) + offset);
                buffer++[0] = static_cast<UInt8>(-(drx + dry) / 2 * 255);
                buffer++[0] = static_cast<UInt8>(drx * 255);
                buffer++[0] = static_cast<UInt8>(dry * 255);
                buffer++[0] = 255;
            }
        }
        index += 0.01f;
    }

    // void renderFrame() override {
        // memset(main.getBuffer(), 192, main.width * main.height << 2);
    // }

    Float map(Float f) {
        while (f < 0) f = -f;
        while (f > 1) f = 2 - f;
        return f;
    }

    FrameBuffer& getDisplayBuffer() override {
        return main;
    }
};
