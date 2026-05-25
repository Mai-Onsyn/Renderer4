module;
export module CPU3DRenderer;
import Renderer;
import Types;
import FrameBuffer;
import Color;
import Image;
import Functions;
import Format;

export class CPU3DRenderer final : public Renderer {
public:
    CPU3DRenderer(const UInt32 width, const UInt32 height) : Renderer(width, height) {}

    Float index = 0;
    void renderFrame() override {
        if (index > 2) index = 0;

        UInt32 h = height;
        UInt32 w = width;
        Float offset = index;

        auto buffer = tripleBuffer.getRenderBuffer()->getBuffer();
        for (UInt32 y = 0; y < h; ++y) {
            for (UInt32 x = 0; x < w; ++x) {
                Float drx = map(static_cast<Float>(x) / static_cast<Float>(w) + offset);
                Float dry = map(static_cast<Float>(y) / static_cast<Float>(h));
                buffer++[0] = static_cast<UInt8>(-(drx + dry) / 2 * 255);
                buffer++[0] = static_cast<UInt8>(drx * 255);
                buffer++[0] = static_cast<UInt8>(dry * 255);
                buffer++[0] = 255;
            }
        }
        drawText(format("FPS = %.2f", getFPS()), 0, 0, tripleBuffer.getRenderBuffer()->getBuffer(), textBuffer);
        // textBuffer->save("D:/Users/Desktop/text.png");
        index += 0.01f;
        tripleBuffer.commit();
    }

    // void renderFrame() override {
        // memset(main.getBuffer(), 192, main.width * main.height << 2);
    // }

    static Float map(Float f) {
        Boolean b = static_cast<Int32>(f) % 2 == 0;
        while (f > 1) f -= 1;
        if (b) {
            f = 1 - f;
        }
        return f;
    }

    void drawText(const String& text, const UInt32 ox, const UInt32 oy, UInt8* screen, Image* buffer) {
        fontDrawer.drawText(text, Color{255, 255, 255, 255}, 27.0, buffer);

        UInt8* imagePtr = buffer->getBuffer();
        for (UInt32 y = 0; y < buffer->height; y++) {
            if (y + oy >= height) break;
            for (UInt32 x = 0; x < buffer->width; x++) {
                if (x + ox >= width) break;

                UInt32 screenOffset = (x + ox + (y + oy) * width) << 2;
                UInt32 imageOffset = (x + y * buffer->width) << 2;
                if (imagePtr[imageOffset + 3] == 255) {
                    screen[screenOffset + 0] = imagePtr[imageOffset + 0];
                    screen[screenOffset + 1] = imagePtr[imageOffset + 1];
                    screen[screenOffset + 2] = imagePtr[imageOffset + 2];
                    screen[screenOffset + 3] = imagePtr[imageOffset + 3];
                }
                else if (imagePtr[imageOffset + 3] > 0) {
                    screen[screenOffset + 0] = (screen[screenOffset + 0] * (255 - imagePtr[imageOffset + 3]) + imagePtr[imageOffset + 0] * imagePtr[imageOffset + 3]) >> 8;
                    screen[screenOffset + 1] = (screen[screenOffset + 1] * (255 - imagePtr[imageOffset + 3]) + imagePtr[imageOffset + 1] * imagePtr[imageOffset + 3]) >> 8;
                    screen[screenOffset + 2] = (screen[screenOffset + 2] * (255 - imagePtr[imageOffset + 3]) + imagePtr[imageOffset + 2] * imagePtr[imageOffset + 3]) >> 8;
                    screen[screenOffset + 3] = 255;
                }
            }
        }
    }
};
