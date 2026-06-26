module;
#include <immintrin.h>
#include <algorithm>
export module TileTask;
import Types;
import Thread;
import Tile;
import FrameBuffer;
import Shader;
import ScreenVertex;
import Vectors;
import Rasterizer;
import Triangle;
import Color;

export class TileTask final : public Runnable {
    Tile* tile;
    FrameBuffer* screenBuffer = nullptr;
    Float* depthBuffer = nullptr;
    Uniform* uniform;

    UInt64 timeFactor = 0.0f;

    void clearScreenArea(const Color& c) const {
        const Int32 color = c.a << 24 | c.b << 16 | c.g << 8 | c.r;
        const __m256i colorVec = _mm256_set1_epi32(color);
        const __m256i depthVec = _mm256_set1_epi32(0);
        auto* pixels = reinterpret_cast<UInt32*>(screenBuffer->getBuffer());
        auto* depths = reinterpret_cast<Float*>(depthBuffer);

        // 计算安全的裁剪边界
        const UInt32 maxY = std::min(tile->y + tile->height, screenBuffer->height);
        const UInt32 maxX = std::min(tile->x + tile->width, screenBuffer->width);

        for (UInt32 y = tile->y; y < maxY; y++) {
            const UInt32 rowOffset = y * screenBuffer->width;
            UInt32 x = tile->x;

            // 计算当前行剩余可用的安全宽度
            UInt32 span = maxX - x;
            for (; x < tile->x + (span & ~7); x += 8) {
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(&pixels[rowOffset + x]), colorVec);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(&depths[rowOffset + x]), depthVec);
            }
            for (; x < maxX; x++) {
                pixels[rowOffset + x] = color;
                depths[rowOffset + x] = 0.0f;
            }
        }
    }
public:
    explicit TileTask(Tile& tile) {
        this->tile = &tile;
    }
    explicit TileTask(Tile* tile) {
        this->tile = tile;
    }

    void setTimeFactor(const UInt64 timeFactor) {
        this->timeFactor = timeFactor;
    }

    void setDrawBuffer(FrameBuffer* screenBuffer, Float* depthBuffer) {
        this->screenBuffer = screenBuffer;
        this->depthBuffer = depthBuffer;
    }

    void setUniform(Uniform* uniform) {
        this->uniform = uniform;
    }

    void run() override {
        clearScreenArea({135, 206, 250, 255});

        if (const UInt32 size = tile->triangles.size(); size > 0) {
            for (UInt32 i = 0; i < size; i++) {
                Rasterizer::drawTriangleAvx2(*tile->triangles[i], tile, screenBuffer, depthBuffer, uniform);
            }
        }
    }
};