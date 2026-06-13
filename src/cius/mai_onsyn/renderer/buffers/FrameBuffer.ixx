module;
// #include <immintrin.h>
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

    // void clearScreen(const Color c) const {
    //     const Int32 color = c.a << 24 | c.b << 16 | c.g << 8 | c.r;
    //     const __m256i colorVec = _mm256_set1_epi32(color);
    //     auto* pixels = reinterpret_cast<UInt32*>(colorMap.get());
    //     const UInt64 totalPixels = width * height;
    //     for (UInt64 i = 0; i < totalPixels; i += 8) {
    //         _mm256_storeu_si256(reinterpret_cast<__m256i*>(&pixels[i]), colorVec);
    //     }
    //     for (UInt64 i = (totalPixels / 8) * 8; i < totalPixels; ++i) {
    //         pixels[i] = color;
    //     }
    // }

    inline void setPixel(const UInt32 x, const UInt32 y, const Color c) const {
        UInt8* pixel = colorMap.get() + (y * width + x) * 4;
        pixel[0] = c.r;
        pixel[1] = c.g;
        pixel[2] = c.b;
        pixel[3] = c.a;
    }
};
