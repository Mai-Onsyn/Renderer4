module;
export module Box;
import Graphics2D;
import Color;
import Types;

using namespace Graphics2D;

export struct Box {
    Rect2D rect{};
    Alignment align = Alignment::TopLeft;
    Color fillColor = Color::Transparent;
    Color borderColor = Color::Transparent;
    Float borderWidth = 0;
    Float cornerRadius = 0;
};