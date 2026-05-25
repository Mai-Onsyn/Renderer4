module;
#include <string>
export module Application;
import FramebufferWindow;
import Types;
import FrameBuffer;
import Renderer;
import CPU3DRenderer;
import Logger;
import Functions;
import Time;
import Queue;
import Thread;

constexpr UInt32 TARGET_WINDOW_FRAMERATE = 100;

export class Application {
    FramebufferWindow* window;
    Renderer* renderer;

    UInt32 resizeRequestWidth, resizeRequestHeight;
public:
    Application(const String& title, const Int32 width, const Int32 height) {
        window = new FramebufferWindow(width, height, title);
        renderer = new CPU3DRenderer(width, height);
        resizeRequestWidth = width;
        resizeRequestHeight = height;
        FramebufferWindow::setVsync(true);

        window->setKeyCallback([this](int key, int scancode, int action, int mods) {
            if (action == 1) { // GLFW_PRESS
                Log::info("Key " + toString(key) + " Pressed");
            }
        });

        window->setMousePosCallback([](double xpos, double ypos) {
            Log::info("Mouse Position: X=" + toString(static_cast<Int32>(xpos)) + ", Y=" + toString(static_cast<Int32>(ypos)));
        });

        window->setMouseButtonCallback([](int button, int action, int mods) {
            if (action == 1) {
                Log::info("Mouse Button Pressed: " + toString(button));
            }
            if (action == 0) {
                Log::info("Mouse Button Released: " + toString(button));
            }
        });

        window->setResizeCallback([this](const int newWidth, const int newHeight) {
            resizeRequestWidth = std::max(newWidth, 1);
            resizeRequestHeight = std::max(newHeight, 1);
            // Log::info("Window resized to: " + toString(newWidth) + "x" + toString(newHeight));
        });
    }
    ~Application() {
        delete window;
        delete renderer;
    }

    [[nodiscard]] bool initialize() const {
        return window->initialize();
    }

    void run() const {
        if (!window->initialize())
            throw RuntimeError("Failed to initialize window");

        renderer->start();
        UInt8 index = 0;
        UInt64 lastResizeTimestamp = millisTime();
        UInt64 frameRateControlStart = millisTime();
        UInt64 framePerMillis = 1000 / TARGET_WINDOW_FRAMERATE;
        while (!window->shouldClose()) {
            if (
                resizeRequestWidth != renderer->width &&
                resizeRequestHeight != renderer->height &&
                millisTime() - lastResizeTimestamp > 1000
            ) {
                renderer->resize(resizeRequestWidth, resizeRequestHeight);
                lastResizeTimestamp = millisTime();
                Log::info("Renderer resized to: " + toString(resizeRequestWidth) + "x" + toString(resizeRequestHeight));
            }

            if (const auto buffer = renderer->getDisplayBuffer(); buffer != nullptr) {
                if (resizeRequestWidth == buffer->width && resizeRequestHeight == buffer->height) window->update(buffer);
                renderer->releaseDisplayBuffer();
            }

            if (index++ % 128 == 0) {
                Log::info("Renderer: " + toString(renderer->getFPS()) + " FPS");
            }

            UInt64 elapsed = millisTime() - frameRateControlStart;
            if (elapsed < framePerMillis) {
                Thread::sleep(framePerMillis - elapsed);
            }
            frameRateControlStart = millisTime();
        }

        renderer->stop();
    }
};
