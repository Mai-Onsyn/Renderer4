module;
#include <vector>
export module Scene2D;
import Types;
import Scene;
import RendererPackage2D;
import FramebufferWindow;
import InputManager;
import Box;
import Text;

// export class Scene2D;
// using UpdateCallback = void (*)(Scene2D* scene,
//                                 InputManager* input,
//                                 Int32 tps,
//                                 Int32 windowWidth,
//                                 Int32 windowHeight,
//                                 FramebufferWindow* window);

export class Scene2D : public Scene<Scene2DSnapShot> {
protected:
    List<Box> boxes{};
    List<Text> texts{};

    // UpdateCallback callback = nullptr;
public:
    using SnapShot = Scene2DSnapShot;

    void addBox(const Box& box) {
        boxes.emplace_back(box);
    }

    void addText(const Text& text) {
        texts.emplace_back(text);
    }

    // void setUpdateCallback(const UpdateCallback callback) {
    //     this->callback = callback;
    // }

    void update(InputManager *input, Int32 tps, Int32 windowWidth, Int32 windowHeight, FramebufferWindow *window) override {
        // if (callback) {
        //     callback(this, input, tps, windowWidth, windowHeight, window);
        // }
    }

    [[nodiscard]] Scene2DSnapShot* createSnapShot(Int32 windowWidth, Int32 windowHeight) const override {
        const auto snapShot = new Scene2DSnapShot{};
        for (const auto& box : boxes) {
            RendererPackage2D pkg;
            pkg.cmd = Command2DType::DrawRect;
            pkg.box = box;
            snapShot->nodes.push_back(move(pkg));
        }
        for (const auto& text : texts) {
            RendererPackage2D pkg;
            pkg.cmd = Command2DType::DrawText;
            pkg.text = text;
            snapShot->nodes.push_back(move(pkg));
        }
        return snapShot;
    }
};