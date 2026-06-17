module;
// #include "GLFW/glfw3.h"
export module LinkSetAppStatics;
import Types;
import Color;

export namespace LinkSetTheme {
    constexpr Color Background{248, 248, 248};
    constexpr Color OnBackground{0, 0, 0};
    constexpr Color NavBase{26, 32, 36};
    constexpr Color NavBaseContainer{47, 56, 61};
    constexpr Color NavBaseContainerActive{47, 115, 223};
    constexpr Color OnNaveBase{255, 255, 255};
    constexpr Color FooterBase{236, 239, 242};
    constexpr Color OnFooterBase{35, 40, 46};
    constexpr Color PrimaryA{27, 100, 217};
    constexpr Color PrimaryB{52, 169, 76};
    constexpr Color PrimaryC{248, 101, 19};
    constexpr Color Outline{192, 192, 192};

    constexpr Float TitleFontPx = 40.0f;
    constexpr Float BodyTextPx = 24.0f;
    constexpr Float BodyTitlePx = 30.0f;
}

export namespace LLegalInput {
    constexpr Int32 Enter = 257;    //GLFW_KEY_ENTER
    constexpr Int32 Backspace = 259;//GLFW_KEY_BACKSPACE
    constexpr Int32 Space = 32;     //GLFW_KEY_SPACE
    constexpr Int32 Key_0 = 48;     //GLFW_KEY_0
    constexpr Int32 Key_9 = 57;     //GLFW_KEY_9
    constexpr Int32 Num_0 = 320;    //GLFW_KEY_KP_0
    constexpr Int32 Num_9 = 329;    //GLFW_KEY_KP_9
    constexpr Int32 F1 = 290;       //GLFW_KEY_F1
    constexpr Int32 Comma = 44;     //GLFW_KEY_COMMA

    constexpr Int32 MouseLeft = 0;  //GLFW_MOUSE_BUTTON_1
    constexpr Int32 MouseRight = 2; //GLFW_MOUSE_BUTTON_3
}