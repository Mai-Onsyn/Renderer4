module;
#include <vector>
export module CPU2DRenderer;
import Renderer;
import Types;
import RendererPackage2D;
import Scene2D;
import Thread;
import Logger;
import TextDrawer;
import BoxDrawer;

template<typename T>
concept SceneType = std::derived_from<T, Scene2D>;

export template<SceneType SceneT> class CPU2DRenderer final : public Renderer<Scene2DSnapShot> {
public:
    using SupportedScene = SceneT;

    CPU2DRenderer(const Int32 width, const Int32 height) : Renderer(width, height) {

    }

    ~CPU2DRenderer() override {
        Renderer::stop();
    }

    void renderFrame() override {
        auto buffer = tripleBuffer.getRenderBuffer();
        buffer->clearScreen({240, 240, 240, 255});

        snapShotBuffer.swap();
        auto snapShot = snapShotBuffer.getContex();
        if (snapShot) {
            if (const UInt64 size = snapShot->nodes.size(); size > 0) {
                for (const auto&[cmd, box, text] : snapShot->nodes) {
                    if (cmd == Command2DType::DrawText) {
                        TextDrawer::draw(text, buffer, fontDrawer);
                    } else if (cmd == Command2DType::DrawRect) {
                        BoxDrawer::draw(box, buffer);
                    }
                }
            }
        } else {
            Log::error("SnapShot is NULL");
            Thread::yield();
        }

        tripleBuffer.commit();
    }

    void onResize(Int32 width, Int32 height) override {}
};