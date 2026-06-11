module;
#include <cmath>
#include <memory>
export module CPU3DRenderer;
import Renderer;
import Types;
import FrameBuffer;
import Color;
import Image;
import Functions;
import Format;
import Time;
import ThreadPool;
import Tile;
import Logger;
import Thread;

export class CPU3DRenderer final : public Renderer {
    ThreadPool executor;
    UniquePtr<Tile[]> tiles;
    Int32 tileCount = 0;
    Int32 tileSize = 0;
public:
    CPU3DRenderer(const Int32 width, const Int32 height, const Int32 threadCount = 12, const Int32 tileSize = 64) : Renderer(width, height), executor(threadCount), tileSize(tileSize) {
        onResize(width, height);
        executor.start();
    }

    ~CPU3DRenderer() override {
        executor.stop();
    }

    // void printTiles() const {
    //     for (Int32 i = 0; i < tileCount; i++) {
    //         Log::debug(tiles[i].toString());
    //     }
    // }

    void onResize(const Int32 width, const Int32 height) override {
        List<Tile> tileList = Tile::divideScreen(width, height, tileSize);
        tileCount = tileList.size();
        tiles = make_unique_for_overwrite<Tile[]>(tileCount);
        std::uninitialized_move(tileList.begin(), tileList.end(), tiles.get());
        // printTiles();
    }

    void renderFrame() override {
        const auto frameBuffer = tripleBuffer.getRenderBuffer();
        auto tileTasks = std::make_unique<UniquePtr<Runnable>[]>(tileCount);

        const UInt64 CYCLE_DURATION_MS = 5000;
        Float timeFactor = static_cast<Float>(millisTime() % CYCLE_DURATION_MS) / static_cast<Float>(CYCLE_DURATION_MS);
        for (Int32 i = 0; i < tileCount; i++) {
            auto* task = new TileTask{tiles[i]};
            task->setDrawBuffer(frameBuffer, depthBuffer.get());
            task->setTimeFactor(timeFactor);
            tileTasks[i].reset(task);
        }
        executor.submit(tileTasks.get(), tileCount);

        // renderFrame2(frameBuffer);
        // drawText(format("FPS = %.2f", getFPS()), 0, 0, frameBuffer->getBuffer());
        // drawText(format("Resolution = %d*%d", width, height), 0, 27, frameBuffer->getBuffer());
        tripleBuffer.commit();
    }

    const UInt64 CYCLE_DURATION_MS = 5000;
    void renderFrame2(const FrameBuffer* frameBuffer) {
        Int32 h = height;
        Int32 w = width;

        UInt64 currentTime = millisTime();
        Float timeFactor = static_cast<Float>(currentTime % CYCLE_DURATION_MS) / static_cast<Float>(CYCLE_DURATION_MS);

        auto buffer = frameBuffer->getBuffer();

        Float invW = 1.0f / static_cast<Float>(w);
        Float invH = 1.0f / static_cast<Float>(h);
        for (Int32 y = 0; y < h; ++y) {
            const Float gradY = static_cast<Float>(y) * invH;

            for (Int32 x = 0; x < w; ++x) {
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
    }

    void drawText(const String& text, const Int32 ox, const Int32 oy, UInt8* screen) {
        fontDrawer.drawText(text, ox, oy, Color{255, 255, 255, 255}, 27.0, screen, width, height);
    }
};
