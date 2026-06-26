module;
#include <algorithm>
#include <cmath>
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

export class Tile {
public:
    Int32 x, y, width, height, size;
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
