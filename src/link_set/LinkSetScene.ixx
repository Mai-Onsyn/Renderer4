module;
#include <vector>
export module LinkSetScene;
import Scene2D;
import Types;
import FramebufferWindow;
import InputManager;
import Logger;
import Box;
import Text;
import Graphics2D;
import Color;
import LinkSetAppStatics;

using namespace Graphics2D;

enum class OpState : UInt8 {
    NONE, INPUT_A, INPUT_B, FIND, INSERT, INTERSECT, UNION, DIFFERENCE, SYMMETRIC_DIFF
};

enum class InputTarget : UInt8 {
    NONE, SET_A, SET_B, FIND_VAL, INSERT_VAL
};

export class LinkSetScene final : public Scene2D {
    void drawNavigation(InputManager* input, Int32 windowWidth, Int32 windowHeight) {
        Box navBase{
            Rect2D{{0, 0}, {static_cast<Float>(windowWidth), LinkSetTheme::NavHeight}},
            Alignment::TopLeft,
            LinkSetTheme::NavBase
        };
        Text navTitle{
            "链表集合运算演示",
            {LinkSetTheme::NavHorizontalPadding, LinkSetTheme::NavVerticalPadding},
            Alignment::TopLeft,
            LinkSetTheme::TitleFontPx,
            LinkSetTheme::OnNaveBase
        };
        boxes.push_back(move(navBase));
        texts.push_back(move(navTitle));
    }
public:
    void update(InputManager* input, Int32 tps, Int32 windowWidth, Int32 windowHeight, FramebufferWindow* window) override {
        boxes.clear();
        texts.clear();
        drawNavigation(input, windowWidth, windowHeight);
    }
};