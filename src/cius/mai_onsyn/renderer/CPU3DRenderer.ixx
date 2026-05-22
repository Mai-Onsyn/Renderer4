module;
export module CPU3DRenderer;
import Renderer;
import Types;
import FrameBuffer;
import Color;

export class CPU3DRenderer final : public Renderer {
public:
    void renderFrame(FrameBuffer* main) override {
        UInt32 h = main->height;
        UInt32 w = main->width;
        auto buffer = main->getBuffer();
        for (UInt32 y = 0; y < h; ++y) {
            for (UInt32 x = 0; x < w; ++x) {
                Float drx = static_cast<Float>(x) / static_cast<Float>(w);
                Float dry = static_cast<Float>(y) / static_cast<Float>(h);
                buffer++[0] = static_cast<UInt8>((drx + dry) / 2 * 255);
                buffer++[0] = static_cast<UInt8>(drx * 255);
                buffer++[0] = static_cast<UInt8>(dry * 255);
                buffer++[0] = 255;
            }
        }
    }
};