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
    CPU3DRenderer(const Int32 width, const Int32 height, const Int32 threadCount = 6, const Int32 tileSize = 64) : Renderer(width, height), executor(threadCount), tileSize(tileSize) {
        onResize(width, height);
        executor.start();
    }

    ~CPU3DRenderer() override {
        Renderer::stop();
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

        drawText(format("FPS = %.2f", getFPS()), 0, 0, frameBuffer->getBuffer());
        drawText(format("Resolution = %d*%d", width, height), 0, 27, frameBuffer->getBuffer());
        tripleBuffer.commit();
    }

    void drawText(const String& text, const Int32 ox, const Int32 oy, UInt8* screen) {
        fontDrawer.drawText(text, ox, oy, Color{255, 255, 255, 255}, 27.0, screen, width, height);
    }
};
