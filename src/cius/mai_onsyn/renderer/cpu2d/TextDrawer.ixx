module;
export module TextDrawer;
import Text;
import Graphics2D;
import Vectors;
import FrameBuffer;
import FontDrawer;

export namespace TextDrawer {
    void draw(const Text& text, const FrameBuffer* screen, FontDrawer& drawer) {
        drawer.drawText(text.text, text.pos.x, text.pos.y, text.color, text.fontSize, screen->getBuffer(), screen->width, screen->height);
    }
}