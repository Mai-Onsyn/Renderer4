module;
export module TextDrawer;
import Text;
import Graphics2D;
import Vectors;
import FrameBuffer;
import FontDrawer;
import Types;

export namespace TextDrawer {
    void draw(const Text& text, const FrameBuffer* screen, FontDrawer& drawer) {
        const Float width = drawer.getTextWidth(text.text, text.fontSize);
        const Float height = text.fontSize;
        Vector2D topLeft = text.pos;

        switch (text.align) {
            case Graphics2D::Alignment::TopLeft: break;
            case Graphics2D::Alignment::TopCenter: topLeft.x -= width / 2.0f; break;
            case Graphics2D::Alignment::TopRight: topLeft.x -= width; break;
            case Graphics2D::Alignment::CenterLeft: topLeft.y -= height / 2.0f; break;
            case Graphics2D::Alignment::Center: topLeft.x -= width / 2.0f; topLeft.y -= height / 2.0f; break;
            case Graphics2D::Alignment::CenterRight: topLeft.x -= width; topLeft.y -= height / 2.0f; break;
            case Graphics2D::Alignment::BottomLeft: topLeft.y -= height; break;
            case Graphics2D::Alignment::BottomCenter: topLeft.x -= width / 2.0f; topLeft.y -= height; break;
            case Graphics2D::Alignment::BottomRight: topLeft.x -= width; topLeft.y -= height; break;
        }

        drawer.drawText(text.text, topLeft.x, topLeft.y, text.color, text.fontSize, screen->getBuffer(), screen->width, screen->height);
    }
}