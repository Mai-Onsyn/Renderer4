module;
#include <utility>
export module Scene;
import FramebufferWindow;
import InputManager;
import Types;

export template<typename SnapShot>
class Scene {
public:
    virtual ~Scene() = default;
    virtual void update(InputManager* input, Int32 tps, Int32 windowWidth, Int32 windowHeight, FramebufferWindow* window) = 0;
    virtual SnapShot* createSnapShot(Int32 windowWidth, Int32 windowHeight) const = 0;
};

export template<typename SceneT>
struct SceneOperation {
    virtual void invoke(SceneT* scene) = 0;
    virtual ~SceneOperation() = default;
};

export template <typename F, typename SceneT>
struct SceneOperationImpl final : SceneOperation<SceneT> {
    F callable;
    explicit SceneOperationImpl(F&& f) : callable(std::forward<F>(f)) {}

    void invoke(SceneT* scene) override {
        callable(scene);
    }
};

export template <typename SceneT, typename F>
auto makeSceneOperationUnique(F&& f) {
    using LambdaType = std::decay_t<F>;
    return make_unique<SceneOperationImpl<LambdaType, SceneT>>(std::forward<F>(f));
}

export template <typename SceneT, typename F>
SceneOperation<SceneT>* makeSceneOperation(F&& f) {
    using LambdaType = std::decay_t<F>;
    return new SceneOperationImpl<LambdaType, SceneT>(std::forward<F>(f));
}