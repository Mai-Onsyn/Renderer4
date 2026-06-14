module;
#include <string>
#include "GLFW/glfw3.h"
#include <cmath>
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
import InputManager;
import Entity;
import Matrix;

constexpr UInt32 TARGET_WINDOW_FRAMERATE = 100;

export class Application {
    UniquePtr<FramebufferWindow> window;
    UniquePtr<Renderer> renderer;
    UniquePtr<Scene> scene;
    UniquePtr<InputManager> inputManager;

    Queue<SceneOperation*> sceneOperations{};

    Int32 windowWidth, windowHeight;
    Queue<Pair<Int32, Int32>> resizeRequests{};
    UInt64 lastResizeTimestamp = millisTime();

    Boolean mouseGrabbed = false;
public:
    Application(const String& title, const Int32 width, const Int32 height) {
        window = make_unique<FramebufferWindow>(width, height, title);
        renderer = make_unique<CPU3DRenderer>(width, height);
        scene = make_unique<Scene>("Test Scene");
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

            // 摄像机更新
            updateCamera();
            rotateTest();

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
                if (windowWidth == buffer->width && windowHeight == buffer->height) {
                    window->update(buffer);
                } else window->update();
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

    Float r = 0;
    void rotateTest() {
        auto entity = scene.get()->getEntity("Test Entity");
        if (!entity) return;
        entity->transform.modelMatrix[0] = cos(r);
        entity->transform.modelMatrix[8] = sin(r);
        entity->transform.modelMatrix[2] = -sin(r);
        entity->transform.modelMatrix[10] = cos(r);
        r += 0.03f;
    }

    void updateCamera() {
        if (window->isFocused()) {
            Float step = 0.05f;
            if (inputManager->isKeyPressed(GLFW_KEY_LEFT_CONTROL))
                step *= 4;
            // 移动
            if (inputManager->isKeyPressed(GLFW_KEY_A))
                scene->getCamera().moveX(-step);
            if (inputManager->isKeyPressed(GLFW_KEY_D))
                scene->getCamera().moveX(step);
            if (inputManager->isKeyPressed(GLFW_KEY_W))
                scene->getCamera().moveZ(step);
            if (inputManager->isKeyPressed(GLFW_KEY_S))
                scene->getCamera().moveZ(-step);
            if (inputManager->isKeyPressed(GLFW_KEY_LEFT_SHIFT))
                scene->getCamera().moveY(-step);
            if (inputManager->isKeyPressed(GLFW_KEY_SPACE))
                scene->getCamera().moveY(step);
            // 视角
            if (inputManager->isKeyPressed(GLFW_KEY_UP))
                scene->getCamera().rotateY(-step / 4);
            if (inputManager->isKeyPressed(GLFW_KEY_DOWN))
                scene->getCamera().rotateY(step / 4);
            if (inputManager->isKeyPressed(GLFW_KEY_LEFT))
                scene->getCamera().rotateX(-step / 4);
            if (inputManager->isKeyPressed(GLFW_KEY_RIGHT))
                scene->getCamera().rotateX(step / 4);

            // 捕获鼠标
            if (inputManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                mouseGrabbed = true;
                // window->moveMouse(windowWidth >> 1, windowHeight >> 1);
                window->disableCursor();
                inputManager->consumeMouseMove();
            }
            // 释放鼠标
            if (inputManager->isKeyPressed(GLFW_KEY_ESCAPE)) {
                mouseGrabbed = false;
                window->enableCursor();
            }

            // 鼠标移动视角
            if (mouseGrabbed) {
                const Float mouseStep = 0.002f * TARGET_WINDOW_FRAMERATE / 100;
                const auto delta = inputManager->consumeMouseMove();
                scene->getCamera().rotateX(delta.x * mouseStep);
                scene->getCamera().rotateY(delta.y * mouseStep);
            }
        }
    }

    void addSceneUpdate(SceneOperation* op) {
        sceneOperations.push(op);
    }
};
