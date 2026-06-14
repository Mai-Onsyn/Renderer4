module;
#include <algorithm>
#include <cmath>
export module TrianglePorcessor;
import Types;
import Vectors;
import ScreenVertex;
import Triangle;
import Tile;
import Logger;

export namespace TriangleProcessor {
    using std::min;
    using std::max;

    void binning(Tile* tiles, const List<ScreenTriangle>& triangles, const UInt32 tileCount, const Int32 tileSize, const Int32 screenW, const Int32 screenH) {
        const Int32 tileRows = (screenH + tileSize - 1) / tileSize;
        const Int32 tileCols = (screenW + tileSize - 1) / tileSize;

        for (UInt32 i = 0; i < tileCount; i++) {
            tiles[i].triangleIndices.clear();
        }
        for (UInt32 tIdx = 0; tIdx < triangles.size(); tIdx++) {
            const auto& [v1, v2, v3] = triangles[tIdx];
            const Int32 xMin = max(static_cast<Int64>(0),       min(v1.pos.x, min(v2.pos.x, v3.pos.x)));
            const Int32 xMax = min(static_cast<Int64>(screenW), max(v1.pos.x, max(v2.pos.x, v3.pos.x)));
            const Int32 yMin = max(static_cast<Int64>(0),       min(v1.pos.y, min(v2.pos.y, v3.pos.y)));
            const Int32 yMax = min(static_cast<Int64>(screenH), max(v1.pos.y, max(v2.pos.y, v3.pos.y)));

            const Int32 tileXStart = std::clamp(xMin / tileSize, 0, tileCols - 1);
            const Int32 tileXEnd   = std::clamp(xMax / tileSize, 0, tileCols - 1);
            const Int32 tileYStart = std::clamp(yMin / tileSize, 0, tileRows - 1);
            const Int32 tileYEnd   = std::clamp(yMax / tileSize, 0, tileRows - 1);

            // Log::debug("%d, %d, %d, %d", tileXStart, tileXEnd, tileYStart, tileYEnd);

            for (Int32 tileX = tileXStart; tileX <= tileXEnd; tileX++) {
                for (Int32 tileY = tileYStart; tileY <= tileYEnd; tileY++) {
                    tiles[tileY * tileCols + tileX].triangleIndices.push_back(tIdx);
                }
            }
        }
    }
}