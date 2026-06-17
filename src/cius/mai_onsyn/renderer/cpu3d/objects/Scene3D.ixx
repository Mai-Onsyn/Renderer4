module;
#include <string>
#include <sstream>
#include "GLFW/glfw3.h"
#include <cmath>
export module Scene3D;
import Types;
import RenderPackage3D;
import Mesh;
import Light;
import Camera;
import Entity;
import Matrix;
import FramebufferWindow;
import InputManager;
import Scene;

export class Scene3D final : public Scene<Scene3DSnapShot> {
    List<Entity> entities{};
    List<Light> lights{};
    Camera camera{};

    Boolean mouseGrabbed = false;
public:
    using SnapShot = Scene3DSnapShot;

    void addEntity(Entity e) { entities.emplace_back(move(e)); }

    void removeEntity(const String& meshName) {
        for (UInt32 i = 0; i < entities.size(); i++) {
            if (meshName == entities[i].name) {
                entities.erase(entities.begin() + i);
            }
        }
    }

    Entity* getEntity(const String& meshName) {
        for (UInt32 i = 0; i < entities.size(); i++) {
            if (meshName == entities[i].name) {
                return &entities[i];
            }
        }
        return nullptr;
    }

    void addLight(Light light) { lights.emplace_back(move(light)); }

    void removeLight(const String& lightName) {
        for (UInt32 i = 0; i < lights.size(); i++) {
            if (lightName == lights[i].name) {
                lights.erase(lights.begin() + i);
            }
        }
    }

    [[nodiscard]] Scene3DSnapShot* createSnapShot(const Int32 screenWidth, const Int32 screenHeight) const override {
        auto* snapShot = new Scene3DSnapShot;
        snapShot->lights = lights;
        snapShot->ambientLight = {255, 255, 255, 255};
        snapShot->viewMatrix = camera.getViewTransformMatrix();
        snapShot->projectionMatrix = camera.getProjectionMatrix(static_cast<Float>(screenWidth) / static_cast<Float>(screenHeight));
        snapShot->cameraPos = camera.pos;

        snapShot->screenWidth = screenWidth;
        snapShot->screenHeight = screenHeight;
        snapShot->near = camera.near;

        for (const auto& entity : entities) {
            RenderPackage3D pkg;
            pkg.name = entity.name;
            pkg.modelMatrix = entity.transform.modelMatrix;
            pkg.vertexCount = entity.mesh.vertices.size();
            pkg.triangleCount = entity.mesh.triangles.size();

            pkg.vertices = entity.mesh.vertices.data();
            pkg.triangles = entity.mesh.triangles.data();

            snapShot->renderPackages.emplace_back(move(pkg));
        }
        return snapShot;
    }

    Camera& getCamera() {
        return camera;
    }

    String toString() const {
        std::stringstream ss;
        ss << "Scene(entities=[";
        Int32 i = 0;
        for (const auto& entity : entities) {
            ss << entity.toString();
            if (i++ != entities.size() - 1) {
                ss << ", ";
            }
        }
        ss << "], lights=[";
        i = 0;
        for (const auto& light : lights) {
            ss << light.toString();
            if (i++ != lights.size() - 1) {
                ss << ", ";
            }
        }
        ss << "])";
        return ss.str();
    }

    void update(InputManager* input, const Int32 tps, Int32 windowWidth, Int32 windowHeight, FramebufferWindow* window) override {
        // rotateTest();
        updateCamera(window, input, tps);
    }


    Float r = 0;
    void rotateTest() {
        auto entity = getEntity("Test Entity");
        if (!entity) return;
        entity->transform.modelMatrix[0] = cos(r);
        entity->transform.modelMatrix[8] = sin(r);
        entity->transform.modelMatrix[2] = -sin(r);
        entity->transform.modelMatrix[10] = cos(r);
        r += 0.03f;
    }

    void updateCamera(FramebufferWindow* window, InputManager* inputManager, const Int32 tps) {
        if (window->isFocused()) {
            Float step = 0.05f;
            if (inputManager->isKeyPressed(GLFW_KEY_LEFT_CONTROL))
                step *= 4;
            // 移动
            if (inputManager->isKeyPressed(GLFW_KEY_A))
                camera.moveX(-step);
            if (inputManager->isKeyPressed(GLFW_KEY_D))
                camera.moveX(step);
            if (inputManager->isKeyPressed(GLFW_KEY_W))
                camera.moveZ(step);
            if (inputManager->isKeyPressed(GLFW_KEY_S))
                camera.moveZ(-step);
            if (inputManager->isKeyPressed(GLFW_KEY_LEFT_SHIFT))
                camera.moveY(-step);
            if (inputManager->isKeyPressed(GLFW_KEY_SPACE))
                camera.moveY(step);
            // 视角
            if (inputManager->isKeyPressed(GLFW_KEY_UP))
                camera.rotateY(-step / 4);
            if (inputManager->isKeyPressed(GLFW_KEY_DOWN))
                camera.rotateY(step / 4);
            if (inputManager->isKeyPressed(GLFW_KEY_LEFT))
                camera.rotateX(-step / 4);
            if (inputManager->isKeyPressed(GLFW_KEY_RIGHT))
                camera.rotateX(step / 4);

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
                const Float mouseStep = 0.002f * tps / 100;
                const auto delta = inputManager->consumeMouseMove();
                camera.rotateX(delta.x * mouseStep);
                camera.rotateY(delta.y * mouseStep);
            }
        }
    }
};