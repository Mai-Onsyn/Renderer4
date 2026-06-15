module;
#include <cmath>
#include <memory>
#include <thread>
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
import VertexProcessor;
import TrianglePorcessor;
import Vectors;
import Triangle;
import RenderPackage;
import Scene3D;

export class CPU3DRenderer final : public Renderer<Scene3DSnapShot> {
    ThreadPool executor;
    UniquePtr<Tile[]> tiles;
    UniquePtr<UniquePtr<Runnable>[]> tasks;
    Int32 tileCount = 0;
    Int32 tileSize = 0;
public:
    using SupportedScene = Scene3D;
    CPU3DRenderer(const Int32 width, const Int32 height, const Int32 threadCount = 12, const Int32 tileSize = 64) : Renderer(width, height), executor(threadCount), tileSize(tileSize) {
        onResize(width, height);
        executor.start();
    }

    ~CPU3DRenderer() override {
        Renderer::stop();
        executor.stop();
    }

    void onResize(const Int32 width, const Int32 height) override {
        List<Tile> tileList = Tile::divideScreen(width, height, tileSize);
        tileCount = tileList.size();
        tiles = make_unique_for_overwrite<Tile[]>(tileCount);
        std::uninitialized_move(tileList.begin(), tileList.end(), tiles.get());

        tasks = make_unique_for_overwrite<UniquePtr<Runnable>[]>(tileCount);
    }

    void renderFrame() override {
        snapShotBuffer.swap();
        const auto sceneSnapShot = snapShotBuffer.getContex();
        if (sceneSnapShot == nullptr) {
            std::this_thread::yield();
            return;
        }
        const auto frameBuffer = tripleBuffer.getRenderBuffer();
        frameBuffer->clearScreen({135, 206, 250, 255});

        const List<ScreenTriangle>& screenTriangles = VertexProcessor::process(sceneSnapShot);
        TriangleProcessor::binning(tiles.get(), screenTriangles, tileCount, tileSize, width, height);

        const UInt64 timeFactor = millisTime();
        for (Int32 i = 0; i < tileCount; i++) {
            auto* task = new TileTask{tiles[i]};
            task->setTriangleList(screenTriangles.data());
            task->setDrawBuffer(frameBuffer, depthBuffer.get());
            task->setTimeFactor(timeFactor);
            tasks[i] = UniquePtr<Runnable>(task);
        }
        executor.submit(tasks.get(), tileCount);

        drawText(format("FPS = %.2f", getFPS()), 8, 8, frameBuffer->getBuffer());
        drawText(format("Resolution = %d*%d", width, height), 8, 27 + 8, frameBuffer->getBuffer());
        tripleBuffer.commit();
    }

    void drawText(const String& text, const Int32 ox, const Int32 oy, UInt8* screen) {
        fontDrawer.drawText(text, ox, oy, Color{255, 255, 255, 255}, 27.0, screen, width, height);
    }
};
