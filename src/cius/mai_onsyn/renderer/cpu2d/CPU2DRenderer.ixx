module;
export module CPU2DRenderer;
import Renderer;
import Types;
import Scene2DSnapShot;
import Scene2D;
import Thread;
import Logger;

export class CPU2DRenderer final : public Renderer<Scene2DSnapShot> {
public:
    using SupportedScene = Scene2D;

    CPU2DRenderer(const Int32 width, const Int32 height) : Renderer(width, height) {

    }

    ~CPU2DRenderer() override {
        Renderer::stop();
    }

    void renderFrame() override {
        auto buffer = tripleBuffer.getRenderBuffer();
        buffer->clearScreen({135, 206, 250, 255});
        tripleBuffer.commit();
        // Log::debug("Cpu 2D renderer is running");
    }

    void onResize(Int32 width, Int32 height) override {

    }
};