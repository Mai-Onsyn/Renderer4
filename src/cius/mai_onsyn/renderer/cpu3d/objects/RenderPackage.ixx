module;
#include <atomic>
#include <sstream>
export module RenderPackage;
import Types;
import Matrix;
import Vectors;
import Light;
import Color;
import Vertex;
import Triangle;

export struct RenderPackage {
    String name;
    Matrix4x4 modelMatrix;

    // 场景持有生命周期 这里是引用指针
    const Vertex* vertices;
    UInt64 vertexCount;

    // 场景持有生命周期 这里是引用指针
    const Triangle* triangles;
    UInt64 triangleCount;

    String toString() const {
        std::stringstream ss;
        ss << "RenderPackage(name=" << name << ", vertices=" << vertexCount << ", triangles=" << triangleCount << ")";
        return ss.str();
    }
};

export struct SceneSnapShot {
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Vector3D cameraPos;

    Int32 screenWidth, screenHeight;
    Float near;

    Color ambientLight;
    List<Light> lights;
    List<RenderPackage> renderPackages;

    [[nodiscard]] String toString() const {
        std::stringstream ss;
        ss << "SceneSnapShot{";
        ss << "screenWidth=" << screenWidth << ", ";
        ss << "screenHeight=" << screenHeight << ", ";
        ss << "viewMatrix=" << viewMatrix.toString() << ", ";
        ss << "projectionMatrix=" << projectionMatrix.toString() << ", ";
        ss << "cameraPos=" << cameraPos.toString() << ", ";
        ss << "ambientLight=" << ambientLight.toString() << ", ";
        ss << "lights=[";
        for (UInt32 i = 0; i < lights.size(); i++) {
            ss << lights[i].toString();
            if (i != lights.size() - 1) {
                ss << ", ";
            }
        }
        ss << "], renderPackages=[";
        for (UInt32 i = 0; i < renderPackages.size(); i++) {
            ss << renderPackages[i].toString();
            if (i != renderPackages.size() - 1) {
                ss << ", ";
            }
        }
        ss << "]}";
        return ss.str();
    }
};

export class RenderSnapShotDoubleBuffer {
    UniquePtr<SceneSnapShot> front;
    UniquePtr<SceneSnapShot> back;

    Mutex mtx;
    Atomic<Boolean> hasNewSnapShot{false};
public:

    void submit(SceneSnapShot* snapShot) {
        LockGuard lock(mtx);
        back.reset(snapShot);
        hasNewSnapShot.store(true, std::memory_order_release);
    }

    void swap() {
        if (!hasNewSnapShot.load(std::memory_order_acquire)) return;

        UniquePtr<SceneSnapShot> oldFront;

        {
            LockGuard lock(mtx);
            oldFront = std::move(front);
            front = std::move(back);

            hasNewSnapShot.store(false, std::memory_order_relaxed);
        }
    }

    [[nodiscard]] const SceneSnapShot* getContex() const {
        return front.get();
    }
};