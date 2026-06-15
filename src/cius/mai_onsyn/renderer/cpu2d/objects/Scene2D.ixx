module;
export module Scene2D;
import Types;
import Scene;
import Scene2DSnapShot;
import FramebufferWindow;
import InputManager;

export class Scene2D final : public Scene<Scene2DSnapShot> {
public:
    using SnapShot = Scene2DSnapShot;

    void update(InputManager *input, Int32 tps, Int32 windowWidth, Int32 windowHeight, FramebufferWindow *window) override {

    }

    [[nodiscard]] Scene2DSnapShot* createSnapShot(Int32 windowWidth, Int32 windowHeight) const override {
        return nullptr;
    }
};