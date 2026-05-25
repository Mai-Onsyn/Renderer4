module;
export module FrameBuffer;
import Types;
import Color;

export class FrameBuffer {
    UInt8Buffer colorMap;
public:
    UInt32 width, height;

    FrameBuffer() {
        colorMap = nullptr;
        width = 0;
        height = 0;
    }
    FrameBuffer(const UInt32 width, const UInt32 height): width(width), height(height) {
        colorMap = makeUInt8Buffer(width * height * 4);
    }
    FrameBuffer(FrameBuffer&& other) noexcept = default;
    FrameBuffer& operator=(FrameBuffer&&) noexcept = default;
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    [[nodiscard]] inline UInt8* getBuffer() const {
        return colorMap.get();
    }

    void resize(const UInt32 width, const UInt32 height) {
        this->width = width;
        this->height = height;
        colorMap = makeUInt8Buffer(width * height * 4);
    }

    inline void setPixel(const UInt32 x, const UInt32 y, const Color c) const {
        UInt8* pixel = colorMap.get() + (y * width + x) * 4;
        pixel[0] = c.r;
        pixel[1] = c.g;
        pixel[2] = c.b;
        pixel[3] = c.a;
    }
};
