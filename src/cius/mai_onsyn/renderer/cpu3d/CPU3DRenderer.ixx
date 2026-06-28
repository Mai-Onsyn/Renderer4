module;
#include <memory>
#include <thread>
#include <vector>
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
import RenderPackage3D;
import Scene3D;
import Shader;
import TileTask;
import ShadowProcessor;
import Light;
import Camera;

constexpr Int32 SHADOW_RESOLUTION = 512;

template<typename T>
concept SceneType = std::derived_from<T, Scene3D>;

export template<SceneType SceneT>
class CPU3DRenderer final : public Renderer<Scene3DSnapShot> {
    ThreadPool executor;
    UniquePtr<Tile[]> tiles;
    UniquePtr<ShadowTile[]> shadowTiles;
    UniquePtr<UniquePtr<Runnable>[]> tasks;
    UniquePtr<UniquePtr<Runnable>[]> shadowTasks;
    Int32 tileCount = 0;
    Int32 shadowTileCount = 0;
    Int32 tileSize = 0;
    Int32 threadCount;

    ShadowCollection shadowMaps;
public:
    using SupportedScene = SceneT;

    CPU3DRenderer(const Int32 width, const Int32 height, const Int32 threadCount = 12, const Int32 tileSize = 64) :
    Renderer(width, height), executor(threadCount),
    shadowTileCount(SHADOW_RESOLUTION * SHADOW_RESOLUTION / (tileSize * tileSize)),
    tileSize(tileSize), threadCount(threadCount) {
        onResize(width, height);

        List<ShadowTile> tileList = ShadowTile::divideScreen(SHADOW_RESOLUTION, SHADOW_RESOLUTION, tileSize);
        shadowTileCount = tileList.size();
        shadowTiles = make_unique_for_overwrite<ShadowTile[]>(shadowTileCount);
        std::uninitialized_move(tileList.begin(), tileList.end(), shadowTiles.get());
        shadowTasks = make_unique_for_overwrite<UniquePtr<Runnable>[]>(shadowTileCount);

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
            Thread::yield();
            return;
        }
        const auto frameBuffer = tripleBuffer.getRenderBuffer();
        frameBuffer->clearScreen({135, 206, 250, 255});

        // 阴影贴图
        const String& lightName = sceneSnapShot->lights[0].name;
        Camera lightCamera{};
        lightCamera.vz = {0.59577, -0.42050, 0.68427};
        // lightCamera.vz = {-0.30548,-0.81225, 0.49692};
        lightCamera.vx = lightCamera.up.cross(lightCamera.vz).normalize();
        lightCamera.vy = lightCamera.vz.cross(lightCamera.vx).normalize();
        lightCamera.pos = sceneSnapShot->lights[0].pos;
        lightCamera.fov = 90;
        shadowMaps[lightName].vpMatrix = lightCamera.getProjectionMatrix(1) * lightCamera.getViewTransformMatrix();
        auto shadowTriangles = ShadowVertexProcessor::process(sceneSnapShot, lightCamera, executor, SHADOW_RESOLUTION, 20 * threadCount);
        ShadowTriangleProcessor::binning(shadowTiles.get(), shadowTriangles, shadowTileCount, tileSize, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
        for (Int32 i = 0; i < shadowTileCount; ++i) {
            auto* task = new ShadowTileTask(shadowTiles[i]);

            Int32 mapSize = SHADOW_RESOLUTION * SHADOW_RESOLUTION;
            if (!shadowMaps.contains(lightName) || shadowMaps[lightName].size < SHADOW_RESOLUTION) {
                shadowMaps[lightName].map = make_unique<Float[]>(mapSize + 8);
                shadowMaps[lightName].size = SHADOW_RESOLUTION;
            }

            task->setDrawBuffer(shadowMaps[lightName].map.get());
            task->setResolution(SHADOW_RESOLUTION);
            shadowTasks[i] = UniquePtr<Runnable>(task);
        }
        executor.submit(shadowTasks.get(), shadowTileCount);

        // 顶点变换
        Int64 vertexTransformStart = microTime();
        const List<List<ScreenTriangle>>& screenTriangles = VertexProcessor::process(sceneSnapShot, executor, 20 * threadCount);

        // binning
        Int64 binningStart = microTime();
        TriangleProcessor::binning(tiles.get(), screenTriangles, tileCount, tileSize, width, height);

        // 光栅化
        Int64 rasterizationStart = microTime();
        const UInt64 timeFactor = millisTime();
        const auto uniform = make_unique<Uniform>(sceneSnapShot->lights);
        uniform->ambient = {0.1};
        uniform->cameraPos = sceneSnapShot->cameraPos;
        uniform->cameraDir = sceneSnapShot->cameraDir;
        for (Int32 i = 0; i < tileCount; i++) {
            auto* task = new TileTask{tiles[i]};
            task->setDrawBuffer(frameBuffer, depthBuffer.get());
            task->setTimeFactor(timeFactor);
            task->setUniform(uniform.get());
            task->setShadowMaps(&shadowMaps);
            tasks[i] = UniquePtr<Runnable>(task);
        }
        executor.submit(tasks.get(), tileCount);

        // 尝试写深度
        {
            // Float* shadow = shadowMaps[lightName].map.get();
            // UInt8* buffer = frameBuffer->getBuffer();
            // Int32 idx = 0;
            // for (Int32 y = 0; y < SHADOW_RESOLUTION; y++) {
            //     Int32 row = y * width;
            //     for (Int32 x = 0; x < SHADOW_RESOLUTION; x++) {
            //         Int32 index = (row + x) << 2;
            //         UInt8 gray = static_cast<UInt8>(sqrt(shadow[idx++]) * 255);
            //         buffer[index + 0] = gray;
            //         buffer[index + 1] = gray;
            //         buffer[index + 2] = gray;
            //         buffer[index + 3] = 255;
            //     }
            // }
        }

        Int64 textDrawStart = microTime();
        drawText(format("FPS = %.2f", getFPS()), 8, 8, frameBuffer->getBuffer());
        drawText(format("1%% Low = %.2f", get1percentLowestFPS()), 8, 27 +8, frameBuffer->getBuffer());
        drawText(format("Resolution = %d*%d", width, height), 8, 54 + 8, frameBuffer->getBuffer());
        drawText(Stringf::format("Pos = %s", sceneSnapShot->cameraPos.toString()), 8, 81 + 8, frameBuffer->getBuffer());
        drawText(Stringf::format("View = %s", sceneSnapShot->cameraDir.toString()), 8, 108 + 8, frameBuffer->getBuffer());
        tripleBuffer.commit();

        // Log::debug("\nVertex transform cost %d\nbinning cost %d\nrasterization cost %d\ndraw text cost %d", binningStart - vertexTransformStart, rasterizationStart - binningStart, textDrawStart - rasterizationStart, microTime() - textDrawStart);
    }

    void drawText(const String& text, const Int32 ox, const Int32 oy, UInt8* screen) {
        fontDrawer.drawText(text, ox, oy, Color{255, 255, 255, 255}, 27.0, screen, width, height);
    }
};
