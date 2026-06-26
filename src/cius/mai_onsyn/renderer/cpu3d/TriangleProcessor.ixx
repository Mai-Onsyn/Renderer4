module;
#include <algorithm>
#include <cmath>
#include <immintrin.h>
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

    void binning(Tile* tiles, const List<List<ScreenTriangle>>& trianglesList, const UInt32 tileCount, const Int32 tileSize, const Int32 screenW, const Int32 screenH) {
        const Int32 tileRows = (screenH + tileSize - 1) / tileSize;
        const Int32 tileCols = (screenW + tileSize - 1) / tileSize;
        const Int32 shift = std::log2(tileSize);

        for (UInt32 i = 0; i < tileCount; i++) {
            tiles[i].triangles.clear();
        }

        // AVX2 常量向量初始化
        const __m256i v_zero   = _mm256_setzero_si256();
        const __m256i v_scrW   = _mm256_set1_epi32(screenW);
        const __m256i v_scrH   = _mm256_set1_epi32(screenH);
        const __m256i v_cols_1 = _mm256_set1_epi32(tileCols - 1);
        const __m256i v_rows_1 = _mm256_set1_epi32(tileRows - 1);
        const __m256i v_shift  = _mm256_set1_epi32(shift);

        // 用于接收 AVX2 计算结果的栈数组
        alignas(32) Int32 xStartArr[8];
        alignas(32) Int32 xEndArr[8];
        alignas(32) Int32 yStartArr[8];
        alignas(32) Int32 yEndArr[8];

        // 外层遍历每一个独立的 List (Chunk)
        for (const auto& chunk : trianglesList) {
            const UInt32 size = chunk.size();
            if (size == 0) continue;

            const UInt32 chunkCount = size & ~7; // 当前 chunk 内能被 8 整除的部分
            UInt32 tIdx = 0;

            // 1. 内层 AVX2 批量处理当前 chunk 里的三角形
            for (; tIdx < chunkCount; tIdx += 8) {
                __m256i v1x = _mm256_setr_epi32(chunk[tIdx+0].v1.pos.x, chunk[tIdx+1].v1.pos.x, chunk[tIdx+2].v1.pos.x, chunk[tIdx+3].v1.pos.x,
                                                chunk[tIdx+4].v1.pos.x, chunk[tIdx+5].v1.pos.x, chunk[tIdx+6].v1.pos.x, chunk[tIdx+7].v1.pos.x);
                __m256i v2x = _mm256_setr_epi32(chunk[tIdx+0].v2.pos.x, chunk[tIdx+1].v2.pos.x, chunk[tIdx+2].v2.pos.x, chunk[tIdx+3].v2.pos.x,
                                                chunk[tIdx+4].v2.pos.x, chunk[tIdx+5].v2.pos.x, chunk[tIdx+6].v2.pos.x, chunk[tIdx+7].v2.pos.x);
                __m256i v3x = _mm256_setr_epi32(chunk[tIdx+0].v3.pos.x, chunk[tIdx+1].v3.pos.x, chunk[tIdx+2].v3.pos.x, chunk[tIdx+3].v3.pos.x,
                                                chunk[tIdx+4].v3.pos.x, chunk[tIdx+5].v3.pos.x, chunk[tIdx+6].v3.pos.x, chunk[tIdx+7].v3.pos.x);

                __m256i v1y = _mm256_setr_epi32(chunk[tIdx+0].v1.pos.y, chunk[tIdx+1].v1.pos.y, chunk[tIdx+2].v1.pos.y, chunk[tIdx+3].v1.pos.y,
                                                chunk[tIdx+4].v1.pos.y, chunk[tIdx+5].v1.pos.y, chunk[tIdx+6].v1.pos.y, chunk[tIdx+7].v1.pos.y);
                __m256i v2y = _mm256_setr_epi32(chunk[tIdx+0].v2.pos.y, chunk[tIdx+1].v2.pos.y, chunk[tIdx+2].v2.pos.y, chunk[tIdx+3].v2.pos.y,
                                                chunk[tIdx+4].v2.pos.y, chunk[tIdx+5].v2.pos.y, chunk[tIdx+6].v2.pos.y, chunk[tIdx+7].v2.pos.y);
                __m256i v3y = _mm256_setr_epi32(chunk[tIdx+0].v3.pos.y, chunk[tIdx+1].v3.pos.y, chunk[tIdx+2].v3.pos.y, chunk[tIdx+3].v3.pos.y,
                                                chunk[tIdx+4].v3.pos.y, chunk[tIdx+5].v3.pos.y, chunk[tIdx+6].v3.pos.y, chunk[tIdx+7].v3.pos.y);

                // 计算包围盒 AABB
                __m256i xMin = _mm256_min_epi32(v1x, _mm256_min_epi32(v2x, v3x));
                __m256i xMax = _mm256_max_epi32(v1x, _mm256_max_epi32(v2x, v3x));
                __m256i yMin = _mm256_min_epi32(v1y, _mm256_min_epi32(v2y, v3y));
                __m256i yMax = _mm256_max_epi32(v1y, _mm256_max_epi32(v2y, v3y));

                xMin = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_scrW, xMin));
                xMax = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_scrW, xMax));
                yMin = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_scrH, yMin));
                yMax = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_scrH, yMax));

                __m256i tileXStart_8 = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_cols_1, _mm256_srlv_epi32(xMin, v_shift)));
                __m256i tileXEnd_8   = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_cols_1, _mm256_srlv_epi32(xMax, v_shift)));
                __m256i tileYStart_8 = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_rows_1, _mm256_srlv_epi32(yMin, v_shift)));
                __m256i tileYEnd_8   = _mm256_max_epi32(v_zero, _mm256_min_epi32(v_rows_1, _mm256_srlv_epi32(yMax, v_shift)));

                _mm256_store_si256(reinterpret_cast<__m256i*>(xStartArr), tileXStart_8);
                _mm256_store_si256(reinterpret_cast<__m256i*>(xEndArr),   tileXEnd_8);
                _mm256_store_si256(reinterpret_cast<__m256i*>(yStartArr), tileYStart_8);
                _mm256_store_si256(reinterpret_cast<__m256i*>(yEndArr),   tileYEnd_8);

                for (UInt32 i = 0; i < 8; ++i) {
                    const UInt32 currentTIdx = tIdx + i;
                    for (Int32 tileY = yStartArr[i]; tileY <= yEndArr[i]; tileY++) {
                        const Int32 row = tileY * tileCols;
                        for (Int32 tileX = xStartArr[i]; tileX <= xEndArr[i]; tileX++) {
                            // 注意：这里传入的是当前 chunk 的元素物理地址
                            tiles[row + tileX].triangles.push_back(&chunk[currentTIdx]);
                        }
                    }
                }
            }

            // 2. 处理当前 chunk 剩下的不满足 8 个的尾数（标量处理）
            for (; tIdx < size; tIdx++) {
                const auto& [v1, v2, v3, texture] = chunk[tIdx];
                const Int32 xMin = std::max(static_cast<Int64>(0),       std::min(v1.pos.x, std::min(v2.pos.x, v3.pos.x)));
                const Int32 xMax = std::min(static_cast<Int64>(screenW), std::max(v1.pos.x, std::max(v2.pos.x, v3.pos.x)));
                const Int32 yMin = std::max(static_cast<Int64>(0),       std::min(v1.pos.y, std::min(v2.pos.y, v3.pos.y)));
                const Int32 yMax = std::min(static_cast<Int64>(screenH), std::max(v1.pos.y, std::max(v2.pos.y, v3.pos.y)));

                const Int32 tileXStart = std::clamp(xMin >> shift, 0, tileCols - 1);
                const Int32 tileXEnd   = std::clamp(xMax >> shift, 0, tileCols - 1);
                const Int32 tileYStart = std::clamp(yMin >> shift, 0, tileRows - 1);
                const Int32 tileYEnd   = std::clamp(yMax >> shift, 0, tileRows - 1);

                for (Int32 tileY = tileYStart; tileY <= tileYEnd; tileY++) {
                    const Int32 row = tileY * tileCols;
                    for (Int32 tileX = tileXStart; tileX <= tileXEnd; tileX++) {
                        tiles[row + tileX].triangles.push_back(&chunk[tIdx]);
                    }
                }
            }
        }
    }
}