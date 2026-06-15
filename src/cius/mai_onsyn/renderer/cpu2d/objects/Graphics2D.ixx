module;
export module Graphics2D;
import Types;
import Vectors;
import Color;

export typedef Vector2D Size;

export namespace Graphics2D {
    enum class Alignment : UInt8 {
        TopLeft, TopCenter, TopRight,
        CenterLeft, Center, CenterRight,
        BottomLeft, BottomCenter, BottomRight
    };

    struct Rect2D {
        Vector2D topLeft;
        Size size;
    };
}