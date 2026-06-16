module;
#include <string>
#include <cmath>
export module Application;
import FramebufferWindow;
import Types;
import InputManager;
import Queue;
import Scene;
import Time;
import Logger;
import Thread;
import Functions;

constexpr UInt32 TARGET_WINDOW_FRAMERATE = 100;

template<typename T>
concept RendererType = requires { typename T::SupportedScene; };

export template<RendererType RendererT>
class Application {
    using SceneT = RendererT::SupportedScene;

    UniquePtr<FramebufferWindow> window;
    UniquePtr<RendererT> renderer;
    UniquePtr<SceneT> scene;
    UniquePtr<InputManager> inputManager;

    Queue<SceneOperation<SceneT>*> sceneOperations{};

    Int32 windowWidth, windowHeight;
    Queue<Pair<Int32, Int32>> resizeRequests{};
    UInt64 lastResizeTimestamp = millisTime();

    Boolean mouseGrabbed = false;
public:
    Application(const String& title, const Int32 width, const Int32 height) {
        window = make_unique<FramebufferWindow>(width, height, title);
        renderer = make_unique<RendererT>(width, height);
        scene = make_unique<SceneT>();
        inputManager = make_unique<InputManager>();
        windowWidth = width;
        windowHeight = height;
        FramebufferWindow::setVsync(true);

        window->setKeyCallback([this](const Int32 key, const Int32 scancode, const Int32 action, const Int32 mods) {
            inputManager->updateKey(key, action);
        });

        window->setMousePosCallback([this](const Double xpos, const Double ypos) {
            inputManager->updateMousePosition(static_cast<Float>(xpos), static_cast<Float>(ypos));
        });

        window->setMouseButtonCallback([this](const Int32 button, const Int32 action, const Int32 mods) {
            inputManager->updateMouseButton(button, action);
        });

        window->setResizeCallback([this](const Int32 newWidth, const Int32 newHeight) {
            resizeRequests.push({std::max(newWidth, 1), std::max(newHeight, 1)});
            lastResizeTimestamp = millisTime();
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
            if (!resizeRequests.empty() && millisTime() - lastResizeTimestamp > 200) {
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
            scene.get()->update(inputManager.get(), TARGET_WINDOW_FRAMERATE, windowWidth, windowHeight, window.get());

            // 场景操作
            while (!sceneOperations.empty()) {
                SceneOperation<SceneT>* operation = sceneOperations.pop();
                operation->invoke(scene.get());
                delete operation;
            }

            // 场景快照提交
            renderer->submitSnapShot(scene->createSnapShot(windowWidth, windowHeight));

            // 渲染器渲染
            if (const auto buffer = renderer->getDisplayBuffer(); buffer != nullptr) {
                if (windowWidth == buffer->width && windowHeight == buffer->height) {
                    window->update(buffer);
                } else window->update();
                renderer->releaseDisplayBuffer();
            }

            // 渲染器帧率显示
            if (index++ % 128 == 0) {
                Log::info("Renderer: %d FPS", renderer.get()->getFPS());
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

    void addSceneUpdate(SceneOperation<SceneT>* op) {
        sceneOperations.push(op);
    }
};