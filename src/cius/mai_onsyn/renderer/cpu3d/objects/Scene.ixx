module;
#include <utility>
#include <string>
#include <sstream>
export module Scene;
import Types;
import RenderPackage;
import RenderPackage;
import Mesh;
import Light;
import Camera;
import Entity;

export class Scene {
    String name;

    List<Entity> entities{};
    List<Light> lights{};
    Camera camera{};
public:
    explicit Scene(String name): name(move(name)) {}
    Scene(): Scene("") {}

    [[nodiscard]] String getName() const { return name; }

    void addEntity(Entity mesh) { entities.push_back(move(mesh)); }

    void removeEntity(const String& meshName) {
        for (UInt32 i = 0; i < entities.size(); i++) {
            if (meshName == entities[i].name) {
                entities.erase(entities.begin() + i);
            }
        }
    }

    void addLight(Light light) { lights.push_back(move(light)); }

    void removeLight(const String& lightName) {
        for (UInt32 i = 0; i < lights.size(); i++) {
            if (lightName == lights[i].name) {
                lights.erase(lights.begin() + i);
            }
        }
    }

    String toString() const {
        std::stringstream ss;
        ss << "Scene(name=" << name << ", entities=[";
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
};

export struct SceneOperation {
    virtual void invoke(Scene* scene) = 0;
    virtual ~SceneOperation() = default;
};

export template <typename F>
struct SceneOperationImpl final : SceneOperation {
    F callable;
    explicit SceneOperationImpl(F&& f) : callable(std::forward<F>(f)) {}

    void invoke(Scene* scene) override {
        callable(scene);
    }
};

export template <typename F>
auto makeSceneOperationUnique(F&& f) {
    using LambdaType = std::decay_t<F>;
    return make_unique<SceneOperationImpl<LambdaType>>(std::forward<F>(f));
}

export template <typename F>
SceneOperation* makeSceneOperation(F&& f) {
    using LambdaType = std::decay_t<F>;
    return new SceneOperationImpl<LambdaType>(std::forward<F>(f));
}