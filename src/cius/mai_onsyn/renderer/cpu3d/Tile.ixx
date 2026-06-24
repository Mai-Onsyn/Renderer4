module;
#include <algorithm>
#include <cmath>
#include <cstring>
#include <thread>
#include <immintrin.h>
export module Tile;
import Types;
import Vectors;
import Format;
import Logger;
import Thread;
import Time;
import FrameBuffer;
import Color;
import ScreenVertex;
import Triangle;
import Shader;

// export class LocalBuffer {
// public:
//     UInt8Buffer pixelBuffer;
//     FloatBuffer depthBuffer;
//
//     LocalBuffer(const Int32 width, const Int32 height) {
//         pixelBuffer = makeUInt8Buffer(width * height * 4);
//         depthBuffer = makeFloatBuffer(width * height);
//     }
// };

export class Tile {
public:
    Int32 x, y, width, height, size;
    // LocalBuffer localBuffer;
    List<UInt32> triangleIndices{};
    List<const ScreenTriangle*> triangles{};

    Tile(const Int32 x, const Int32 y, const Int32 width, const Int32 height):
        x(x), y(y), width(width), height(height), size(width * height) {}

    Tile(): Tile(0, 0, 0, 0) {}

    static List<Tile> divideScreen(const Int32 screenWidth, const Int32 screenHeight, const Int32 tileSize) {
        List<Tile> tiles;

        for (Int32 y = 0; y < screenHeight; y += tileSize) {
            for (Int32 x = 0; x < screenWidth; x += tileSize) {
                const Int32 tileWidth = std::min(tileSize, screenWidth - x);
                const Int32 tileHeight = std::min(tileSize, screenHeight - y);
                tiles.emplace_back(x, y, tileWidth, tileHeight);
            }
        }

        return tiles;
    }

    [[nodiscard]] String toString() const {
        return format("Tile(x=%d, y=%d, w=%d, h=%d)", x, y, width, height);
    }
};

export class TileTask final : public Runnable {
    Tile* tile;
    FrameBuffer* screenBuffer = nullptr;
    Float* depthBuffer = nullptr;
    // const ScreenTriangle *triangleList;
    Uniform* uniform;

    UInt64 timeFactor = 0.0f;

    void clearScreenArea(const Color& c) const {
        const Int32 color = c.a << 24 | c.b << 16 | c.g << 8 | c.r;
        const __m256i colorVec = _mm256_set1_epi32(color);
        const __m256i depthVec = _mm256_set1_epi32(0);
        auto* pixels = reinterpret_cast<UInt32*>(screenBuffer->getBuffer());
        auto* depths = reinterpret_cast<Float*>(depthBuffer);
        for (UInt32 y = tile->y; y < tile->y + tile->height; y++) {
            const UInt32 rowOffset = y * screenBuffer->width;
            UInt32 x = tile->x;
            for (; x < tile->x + (tile->width & ~7); x += 8) {
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(&pixels[rowOffset + x]), colorVec);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(&depths[rowOffset + x]), depthVec);
            }
            for (; x < tile->x + tile->width; x++) {
                pixels[rowOffset + x] = color;
                depths[rowOffset + x] = 0.0f;
            }
        }
    }

    void drawTriangle(const ScreenTriangle& triangle) const {
        // 复制顶点
        ScreenVertex v1 = triangle.v1;
        ScreenVertex v2 = triangle.v2;
        ScreenVertex v3 = triangle.v3;

        // 排序顶点
        if (v1.pos.y > v2.pos.y) std::swap(v1, v2);
        if (v2.pos.y > v3.pos.y) std::swap(v2, v3);
        if (v1.pos.y > v2.pos.y) std::swap(v1, v2);

        const Float sABC_inv = 2.0f / abs((v3.pos.x - v1.pos.x) * (v2.pos.y - v1.pos.y) - (v3.pos.y - v1.pos.y) * (v2.pos.x - v1.pos.x));

        const Int64 ys = std::clamp(v1.pos.y, static_cast<Int64>(tile->y), static_cast<Int64>(tile->y + tile->height));
        const Int64 ye = std::clamp(v3.pos.y, static_cast<Int64>(tile->y), static_cast<Int64>(tile->y + tile->height));

        const Boolean unhorizontal_AB = v1.pos.y != v2.pos.y;
        const Boolean unhorizontal_BC = v2.pos.y != v3.pos.y;

        // UInt32* texturePixels = nullptr;
        // Int32 textureW = 0;
        // Int32 textureH = 0;
        // if (triangle.texture && triangle.texture->getKdData()) {
        //     texturePixels = reinterpret_cast<UInt32*>(triangle.texture->getKdData());
        //     textureW = triangle.texture->getWidth();
        //     textureH = triangle.texture->getHeight();
        // }
        // 扫描线填充
        for (Int64 y = ys; y < ye; y++) {
            Int64 xa, xb = v1.pos.x + (v3.pos.x - v1.pos.x) * (y - v1.pos.y) / (v3.pos.y - v1.pos.y);

            if (y < v2.pos.y && unhorizontal_AB) {
                xa = v1.pos.x + (v2.pos.x - v1.pos.x) * (y - v1.pos.y) / (v2.pos.y - v1.pos.y);
            } else if (unhorizontal_BC) {
                xa = v2.pos.x + (v3.pos.x - v2.pos.x) * (y - v2.pos.y) / (v3.pos.y - v2.pos.y);
            }
            else continue;

            const Int64 xs = std::clamp(std::min(xa, xb), static_cast<Int64>(tile->x), static_cast<Int64>(tile->x + tile->width));
            const Int64 xe = std::clamp(std::max(xa, xb), static_cast<Int64>(tile->x), static_cast<Int64>(tile->x + tile->width));

            UInt8* screenRow = screenBuffer->getBuffer() + y * screenBuffer->width * 4;
            Float* depthRow = depthBuffer + y * screenBuffer->width;

            for (Int64 x = xs; x < xe; x++) {
                const Boolean isBorder = y == ys || y == ye - 1 || x == xs || x == xe - 1;
                // 重心插值
                const VectorInt2D PA = {v1.pos.x - x, v1.pos.y - y};
                const VectorInt2D PB = {v2.pos.x - x, v2.pos.y - y};
                const VectorInt2D PC = {v3.pos.x - x, v3.pos.y - y};
                const Int64 sBPC = abs(PB.x * PC.y - PB.y * PC.x) >> 1;
                const Int64 sAPC = abs(PA.x * PC.y - PA.y * PC.x) >> 1;

                const Float alpha_s = static_cast<Float>(sBPC) * sABC_inv;
                const Float beta_s = static_cast<Float>(sAPC) * sABC_inv;
                const Float gamma_s = 1.0f - alpha_s - beta_s;

                const Float numerator1 = alpha_s * v1.invClipW;
                const Float numerator2 = beta_s * v2.invClipW;
                const Float numerator3 = gamma_s * v3.invClipW;
                const Float invSub = 1.0f / (numerator1 + numerator2 + numerator3);

                const Float w1 = numerator1 * invSub;
                const Float w2 = numerator2 * invSub;
                const Float w3 = numerator3 * invSub;

                const Float depth = v1.depth * w1 + v2.depth * w2 + v3.depth * w3;

                if (depth >= depthRow[x]) {
                    Fragment fragment;
                    // if (texturePixels) {
                    //     const Float u = v1.uv.x * w1 + v2.uv.x * w2 + v3.uv.x * w3;
                    //     const Float v = v1.uv.y * w1 + v2.uv.y * w2 + v3.uv.y * w3;
                    //     Int32 tx = std::min(static_cast<Int32>(u * textureW), textureW - 1);
                    //     Int32 ty = std::min(static_cast<Int32>(v * textureH), textureH - 1);
                    //     Int32 offset = std::clamp(ty * textureW + tx, 0, textureW * textureH - 1);
                    //     fragment.uvColor = std::bit_cast<Color>(texturePixels[offset]);
                    // }
                    // else fragment.uvColor = {static_cast<UInt8>(isBorder ? 255 : 0), 64, 96, 255};
                    // fragment.uvColor = {255, 255, 255, 255};
                    fragment.x = x;
                    fragment.y = y;
                    fragment.u = v1.uv.x * w1 + v2.uv.x * w2 + v3.uv.x * w3;
                    fragment.v = v1.uv.y * w1 + v2.uv.y * w2 + v3.uv.y * w3;
                    fragment.depth = depth;
                    fragment.normal = v1.normal * w1 + v2.normal * w2 + v3.normal * w3;
                    fragment.worldPos = v1.worldPos * w1 + v2.worldPos * w2 + v3.worldPos * w3;
                    const auto [r, g, b, a] = Shader::fragmentShader(fragment, triangle.texture, uniform);

                    const UInt32 pixelIndex = x << 2;
                    depthRow[x] = depth;
                    screenRow[pixelIndex] = r;
                    screenRow[pixelIndex + 1] = g;
                    screenRow[pixelIndex + 2] = b;
                    screenRow[pixelIndex + 3] = a;
                }
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
                drawTriangle(*tile->triangles[i]);
            }
        }
    }

    // void run() override {
    //     memset(tile->localBuffer.pixelBuffer.get(), 0, sizeof(Float) * tile->width * tile->height);
    //     Int32 w = screenBuffer->width;
    //     Int32 h = screenBuffer->height;
    //
    //     Float invW = 1.0f / static_cast<Float>(w);
    //     Float invH = 1.0f / static_cast<Float>(h);
    //
    //     for (Int32 y = tile->y; y < tile->y + tile->height; y++) {
    //         const Float gradY = static_cast<Float>(y) * invH;
    //         auto buffer = tile->localBuffer.pixelBuffer.get() + (y - tile->y) * tile->width * 4;
    //         for (Int32 x = tile->x; x < tile->x + tile->width; x++) {
    //             const Float gradX = static_cast<Float>(x) * invW;
    //
    //             Float hue = timeFactor + gradX + gradY;
    //
    //             hue -= static_cast<Int32>(hue);
    //             if (hue < 0.0f) hue += 1.0f;
    //
    //             Float r = 0.0f, g = 0.0f, b = 0.0f;
    //             const Float h6 = hue * 6.0f;
    //             const auto sector = static_cast<Int32>(h6);
    //             const auto fract = h6 - static_cast<Float>(sector);
    //
    //             switch (sector) {
    //                 case 0: r = 1.0f;         g = fract;        b = 0.0f;         break; // 红 -> 黄
    //                 case 1: r = 1.0f - fract; g = 1.0f;         b = 0.0f;         break; // 黄 -> 绿
    //                 case 2: r = 0.0f;         g = 1.0f;         b = fract;        break; // 绿 -> 青
    //                 case 3: r = 0.0f;         g = 1.0f - fract; b = 1.0f;         break; // 青 -> 蓝
    //                 case 4: r = fract;        g = 0.0f;         b = 1.0f;         break; // 蓝 -> 品红
    //                 default:r = 1.0f;         g = 0.0f;         b = 1.0f - fract; break; // 品红 -> 红
    //             }
    //
    //             buffer[0] = static_cast<UInt8>(r * 255.0f);
    //             buffer[1] = static_cast<UInt8>(g * 255.0f);
    //             buffer[2] = static_cast<UInt8>(b * 255.0f);
    //             buffer[3] = 255;
    //             buffer += 4;
    //         }
    //     }
    //
    //     mergeToScreen();
    // }

    // void mergeToScreen() const {
    //     auto screenPtr = reinterpret_cast<Int32*>(screenBuffer->getBuffer());
    //
    //     const Int32 screenWidth = screenBuffer->width;
    //     const Int32 startOffset = tile->y * screenWidth + tile->x;
    //
    //     screenPtr += startOffset;
    //     Float* currentDepthBuffer = depthBuffer + startOffset;
    //
    //     for (Int32 y = 0; y < tile->height; y++) {
    //         memcpy(screenPtr,
    //                tile->localBuffer.pixelBuffer.get() + y * (tile->width << 2),
    //                tile->width * sizeof(Int32));
    //
    //         memcpy(currentDepthBuffer,
    //                tile->localBuffer.depthBuffer.get() + y * tile->width,
    //                tile->width * sizeof(Float));
    //
    //         screenPtr += screenWidth;
    //         currentDepthBuffer += screenWidth;
    //     }
    // }

    // void clearBuffer(const Color& c) const {
    //     const Int32 color = c.a << 24 | c.b << 16 | c.g << 8 | c.r;
    //     const __m256i colorVec = _mm256_set1_epi32(color);
    //     auto* pixels = reinterpret_cast<UInt32*>(tile->localBuffer.pixelBuffer.get());
    //     const UInt64 totalPixels = tile->width * tile->height;
    //     for (UInt64 i = 0; i < totalPixels; i += 8) {
    //         _mm256_storeu_si256(reinterpret_cast<__m256i*>(&pixels[i]), colorVec);
    //     }
    //     for (UInt64 i = (totalPixels / 8) * 8; i < totalPixels; i++) {
    //         pixels[i] = color;
    //     }
    // }
};