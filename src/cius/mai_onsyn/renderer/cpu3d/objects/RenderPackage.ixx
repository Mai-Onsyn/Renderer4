module;
#include <atomic>
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

    const Vertex* vertices;
    UInt64 vertexCount;

    const Triangle* triangles;
    UInt64 triangleCount;
};

export struct SceneSnapShot {
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Vector3D cameraPos;

    Color ambientLight;
    List<Light> lights;
    List<RenderPackage> renderPackages;
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

        LockGuard lock(mtx);
        std::swap(front, back);
        hasNewSnapShot.store(false, std::memory_order_relaxed);
    }

    [[nodiscard]] const SceneSnapShot* getContex() const {
        return front.get();
    }
};