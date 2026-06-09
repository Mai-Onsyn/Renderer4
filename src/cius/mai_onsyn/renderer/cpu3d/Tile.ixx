module;
#include <cmath>
export module Tile;
import Types;
import Vectors;
import Format;
import Logger;
import MathUtil;

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
    Int32 x, y, width, height;
    LocalBuffer localBuffer;

    Tile(const Int32 x, const Int32 y, const Int32 width, const Int32 height):
        x(x), y(y), width(width), height(height), localBuffer(LocalBuffer(width, height)) {}

    static List<Tile> divideScreen(const Int32 screenWidth, const Int32 screenHeight, const Int32 count) {
        List<Tile> tiles;
        List<Pair<Int32, Int32>> factors = findAllFactors(count);
        Float aspectRatio = static_cast<Float>(screenWidth) / static_cast<Float>(screenHeight);

        Int32 mostSimilarIndex = -1;
        Float mostSimilarDistance = 99999.0;
        Int32 index = 0;
        for (const auto& [row, col] : factors) {
            Float distance = abs(aspectRatio - static_cast<Float>(row) / static_cast<Float>(col));
            if (distance < mostSimilarDistance) {
                mostSimilarDistance = distance;
                mostSimilarIndex = index;
            }
            index++;
        }

        Log::debug(format("Most similar aspect ratio: %d:%d", factors[mostSimilarIndex].first, factors[mostSimilarIndex].second));

        return tiles;
    }

    [[nodiscard]] String toString() const {
        return format("Tile(x=%d, y=%d, w=%d, h=%d)", x, y, width, height);
    }
};