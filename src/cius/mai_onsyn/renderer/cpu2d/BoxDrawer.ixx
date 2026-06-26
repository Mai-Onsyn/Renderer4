module;
#include <algorithm>
export module BoxDrawer;
import Box;
import Graphics2D;
import Vectors;
import FrameBuffer;
import Types;
import Functions;

using namespace Graphics2D;

export namespace BoxDrawer {
    Int32 clamp(const Int32 v, const Int32 min, const Int32 max) {
        return std::clamp(v, min, max);
    }

    void draw(const Box& box, const FrameBuffer* screen) {
        const auto&[topLeft, size] = box.rect;

        const Int32 ys = clamp(topLeft.y, 0, screen->height);
        const Int32 ye = clamp(topLeft.y + size.y, 0, screen->height);
        if (ys >= ye) return;

        UInt8* buffer = screen->getBuffer();
        for (Int32 y = ys; y < ye; y++) {
            const Int32 rowIdx = y * screen->width;
            const Int32 xs = clamp(topLeft.x, 0, screen->width);
            const Int32 xe = clamp(topLeft.x + size.x, 0, screen->width);

            if (xs >= xe) continue;
            avx2Fill(buffer, rowIdx + xs, box.fillColor, xe - xs);
        }
    }
}