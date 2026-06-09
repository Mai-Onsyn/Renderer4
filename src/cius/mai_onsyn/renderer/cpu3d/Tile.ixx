module;
#include <algorithm>
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
        const Float aspectRatio = static_cast<Float>(screenWidth) / static_cast<Float>(screenHeight);

        Int32 mostSimilarIndex = -1;
        Float mostSimilarDistance = 99999.0;
        Int32 index = 0;
        for (const auto& [row, col] : factors) {
            Float currentRatio = static_cast<Float>(col) / static_cast<Float>(row);
            Float distance = fabs(aspectRatio - currentRatio);
            if (distance < mostSimilarDistance) {
                mostSimilarDistance = distance;
                mostSimilarIndex = index;
            }
            Log::debug(format("row %.4f, colum %.4f", row, col));

            if (fabs(1 / distance - currentRatio) < 1e-4 && currentRatio < 1) {
                mostSimilarDistance = distance;
                mostSimilarIndex = index;
            }
            index++;
        }
        Int32 rows = factors[mostSimilarIndex].first;
        Int32 cols = factors[mostSimilarIndex].second;

        Int32 baseTileWidth = screenWidth / cols;
        Int32 baseTileHeight = screenHeight / rows;
        for (Int32 r = 0; r < rows; ++r) {
            for (Int32 c = 0; c < cols; ++c) {
                Int32 x = c * baseTileWidth;
                Int32 y = r * baseTileHeight;

                Int32 width  = (c == cols - 1) ? (screenWidth - x)  : baseTileWidth;
                Int32 height = (r == rows - 1) ? (screenHeight - y) : baseTileHeight;

                if (width > 0 && height > 0) {
                    tiles.push_back(Tile(x, y, width, height));
                }
            }
        }

        return tiles;
    }

    [[nodiscard]] String toString() const {
        return format("Tile(x=%d, y=%d, w=%d, h=%d)", x, y, width, height);
    }
};