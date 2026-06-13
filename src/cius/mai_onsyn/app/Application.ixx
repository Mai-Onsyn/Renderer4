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
import Scene;

constexpr UInt32 TARGET_WINDOW_FRAMERATE = 100;

export class Application {
    UniquePtr<FramebufferWindow> window;
    UniquePtr<Renderer> renderer;
    UniquePtr<Scene> scene;

    Queue<SceneOperation*> sceneOperations{};

    Int32 windowWidth, windowHeight;
    Queue<Pair<Int32, Int32>> resizeRequests{};
    UInt64 lastResizeTimestamp = millisTime();
public:
    Application(const String& title, const Int32 width, const Int32 height) {
        window = make_unique<FramebufferWindow>(width, height, title);
        renderer = make_unique<CPU3DRenderer>(width, height);
        scene = make_unique<Scene>("Test Scene");
        windowWidth = width;
        windowHeight = height;
        FramebufferWindow::setVsync(true);

        window->setKeyCallback([this](int key, int scancode, int action, int mods) {
            if (action == 1) { // GLFW_PRESS
                Log::info("Key %d Pressed", key);
            }
        });

        window->setMousePosCallback([](double xpos, double ypos) {
            Log::info("Mouse Position: X=%d, Y=%d", xpos, ypos);
        });

        window->setMouseButtonCallback([](int button, int action, int mods) {
            if (action == 1) {
                Log::info("Mouse Button Pressed: %d", button);
            }
            if (action == 0) {
                Log::info("Mouse Button Released: %d", button);
            }
        });

        window->setResizeCallback([this](const int newWidth, const int newHeight) {
            resizeRequests.push({std::max(newWidth, 1), std::max(newHeight, 1)});
            lastResizeTimestamp = millisTime();
            // resizeRequestWidth = std::max(newWidth, 1);
            // resizeRequestHeight = std::max(newHeight, 1);
            // Log::info("Window resized to: " + toString(newWidth) + "x" + toString(newHeight));
        });
    }

    [[nodiscard]] bool initialize() const {
        return window->initialize();
    }

    void run() {
        if (!window->initialize())
            throw RuntimeError("Failed to initialize window");

        renderer->start();
        UInt8 index = 0;
        UInt64 frameRateControlStart = millisTime();
        UInt64 framePerMillis = 1000 / TARGET_WINDOW_FRAMERATE;
        while (!window->shouldClose()) {
            // 窗口大小改变
            if (!resizeRequests.empty() && millisTime() - lastResizeTimestamp > 1000) {
                Pair<Int32, Int32> resizeRequest = resizeRequests.pop();
                while (!resizeRequests.empty()) {
                    resizeRequest = resizeRequests.pop();
                }
                windowWidth = resizeRequest.first;
                windowHeight = resizeRequest.second;
                renderer->resize(windowWidth, windowHeight);
                Log::info("Renderer resized to: %d*%d", windowWidth, windowHeight);
            }

            // 场景更新
            while (!sceneOperations.empty()) {
                SceneOperation* operation = sceneOperations.pop();
                operation->invoke(scene.get());
                delete operation;
            }

            // 场景快照提交
            renderer->submitSnapShot(scene->createSnapShot(windowWidth, windowHeight));

            // 渲染器渲染
            if (const auto buffer = renderer->getDisplayBuffer(); buffer != nullptr) {
                window->update(buffer);
                renderer->releaseDisplayBuffer();
            }

            // 渲染器帧率显示
            if (index++ % 128 == 0) {
                Log::info("Renderer: " + toString(renderer->getFPS()) + " FPS");
            }

            // 窗口循环速度控制
            const UInt64 elapsed = millisTime() - frameRateControlStart;
            if (elapsed < framePerMillis) {
                Thread::sleep(framePerMillis - elapsed);
            }
            frameRateControlStart = millisTime();
        }

        renderer->stop();
    }

    void addSceneUpdate(SceneOperation* op) {
        sceneOperations.push(op);
    }
};
