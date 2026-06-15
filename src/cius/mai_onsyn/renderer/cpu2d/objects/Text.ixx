module;
export module Text;
import Graphics2D;
import Color;
import Types;
import Vectors;

using namespace Graphics2D;

export struct Text {
    String text;
    Vector2D pos;
    Alignment align = Alignment::TopLeft;
    Float fontSize = 16.0f;
    Color color;
};