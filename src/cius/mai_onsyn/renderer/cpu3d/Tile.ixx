module;
#include <algorithm>
#include <cmath>
#include <cstring>
#include <thread>
export module Tile;
import Types;
import Vectors;
import Format;
import Logger;
import Thread;
import Time;
import FrameBuffer;

export class LocalBuffer {
public:
    UInt8Buffer pixelBuffer;
    FloatBuffer depthBuffer;

    LocalBuffer(const Int32 width, const Int32 height) {
        pixelBuffer = makeUInt8Buffer(width * height * 4);
        depthBuffer = makeFloatBuffer(width * height);
    }
};

export class Tile {
public:
    Int32 x, y, width, height, size;
    LocalBuffer localBuffer;

    Tile(const Int32 x, const Int32 y, const Int32 width, const Int32 height):
        x(x), y(y), width(width), height(height), size(width * height), localBuffer(LocalBuffer(width, height)) {}

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

    Float timeFactor = 0.0f;
public:
    explicit TileTask(Tile& tile) {
        this->tile = &tile;
    }
    explicit TileTask(Tile* tile) {
        this->tile = tile;
    }

    void setTimeFactor(const Float timeFactor) {
        this->timeFactor = timeFactor;
    }

    void setDrawBuffer(FrameBuffer* screenBuffer, Float* depthBuffer) {
        this->screenBuffer = screenBuffer;
        this->depthBuffer = depthBuffer;
    }

    void run() override {
        memset(tile->localBuffer.pixelBuffer.get(), 0, sizeof(Float) * tile->width * tile->height);
        Int32 w = screenBuffer->width;
        Int32 h = screenBuffer->height;

        Float invW = 1.0f / static_cast<Float>(w);
        Float invH = 1.0f / static_cast<Float>(h);

        for (Int32 y = tile->y; y < tile->y + tile->height; y++) {
            const Float gradY = static_cast<Float>(y) * invH;
            auto buffer = tile->localBuffer.pixelBuffer.get() + (y - tile->y) * tile->width * 4;
            for (Int32 x = tile->x; x < tile->x + tile->width; x++) {
                const Float gradX = static_cast<Float>(x) * invW;

                Float hue = timeFactor + gradX + gradY;

                hue -= static_cast<Int32>(hue);
                if (hue < 0.0f) hue += 1.0f;

                Float r = 0.0f, g = 0.0f, b = 0.0f;
                const Float h6 = hue * 6.0f;
                const auto sector = static_cast<Int32>(h6);
                const auto fract = h6 - static_cast<Float>(sector);

                switch (sector) {
                    case 0: r = 1.0f;         g = fract;        b = 0.0f;         break; // 红 -> 黄
                    case 1: r = 1.0f - fract; g = 1.0f;         b = 0.0f;         break; // 黄 -> 绿
                    case 2: r = 0.0f;         g = 1.0f;         b = fract;        break; // 绿 -> 青
                    case 3: r = 0.0f;         g = 1.0f - fract; b = 1.0f;         break; // 青 -> 蓝
                    case 4: r = fract;        g = 0.0f;         b = 1.0f;         break; // 蓝 -> 品红
                    default:r = 1.0f;         g = 0.0f;         b = 1.0f - fract; break; // 品红 -> 红
                }

                buffer[0] = static_cast<UInt8>(r * 255.0f);
                buffer[1] = static_cast<UInt8>(g * 255.0f);
                buffer[2] = static_cast<UInt8>(b * 255.0f);
                buffer[3] = 255;
                buffer += 4;
            }
        }

        mergeToScreen();
    }

    // void run() override {
    //     memset(tile->localBuffer.pixelBuffer.get(), 128, tile->size * sizeof(Int32));
    //     memset(tile->localBuffer.depthBuffer.get(), 0, tile->size * sizeof(Float));
    //
    //     mergeToScreen();
    // }

    void mergeToScreen() const {
        auto screenPtr = reinterpret_cast<Int32*>(screenBuffer->getBuffer());

        const Int32 screenWidth = screenBuffer->width;
        const Int32 startOffset = tile->y * screenWidth + tile->x;

        screenPtr += startOffset;
        Float* currentDepthBuffer = depthBuffer + startOffset;

        for (Int32 y = 0; y < tile->height; y++) {
            memcpy(screenPtr,
                   tile->localBuffer.pixelBuffer.get() + y * (tile->width << 2),
                   tile->width * sizeof(Int32));

            memcpy(currentDepthBuffer,
                   tile->localBuffer.depthBuffer.get() + y * tile->width,
                   tile->width * sizeof(Float));

            screenPtr += screenWidth;
            currentDepthBuffer += screenWidth;
        }
    }
};