module;
#include <vector>
#include <memory>
#include <string>
#include <immintrin.h>
#include <algorithm>
#include <cmath>
export module ShadowProcessor;
import Types;
import Matrix;
import ScreenVertex;
import Vertex;
import Vectors;
import RenderPackage3D;
import Logger;
import Texture;
import Time;
import RenderPackage3D;
import Thread;
import ThreadPool;
import SIMD;
import Triangle;
import Camera;

export struct ShadowData {
    UniquePtr<Float[]> map;
    Int32 size;
    Matrix4x4 vpMatrix;
};

export typedef Map<String, ShadowData> ShadowCollection;

struct ShadowClipVertex {
    Vector4D pos;
};

struct TransformRange {
    UInt32 pkgIndex;
    UInt32 start;
    UInt32 end;
};

struct ShadowTriangleIndices {
    Int32 v1, v2, v3;
};

export namespace ShadowVertexProcessor {
    ShadowClipVertex interpolate(const ShadowClipVertex& v1, const ShadowClipVertex& v2, const Float t) {
        return { v1.pos * (1 - t) + v2.pos * t };
    }

    // 最终数据在temp2中
    void clip(const ShadowClipVertex& v1, const ShadowClipVertex& v2, const ShadowClipVertex& v3, List<ShadowClipVertex>& temp1, List<ShadowClipVertex>& temp2) {
        // Log::debug("Run Clip");
        temp1.clear();
        temp2.clear();
        temp1.push_back(v1);
        temp1.push_back(v2);
        temp1.push_back(v3);

        // 近平面 (z >= 0)
        for (UInt32 i = 0; i < temp1.size(); i++) {
            const ShadowClipVertex& A = temp1[i];
            const ShadowClipVertex& B = temp1[(i + 1) % temp1.size()];
            const Boolean Ain = A.pos.z >= 0;
            const Boolean Bin = B.pos.z >= 0;
            if (Ain && Bin) {
                temp2.push_back(B);
            } else if (Ain) {
                const Float t = A.pos.z / (A.pos.z - B.pos.z);
                temp2.push_back(interpolate(A, B, t));
            } else if (Bin) {
                const Float t = A.pos.z / (A.pos.z - B.pos.z);
                temp2.push_back(interpolate(A, B, t));
                temp2.push_back(B);
            }
        }

        // 左平面 (x >= -w)
        temp1.clear();
        for (UInt32 i = 0; i < temp2.size(); i++) {
            const ShadowClipVertex& A = temp2[i];
            const ShadowClipVertex& B = temp2[(i + 1) % temp2.size()];
            const Boolean Ain = A.pos.x >= -A.pos.w;
            const Boolean Bin = B.pos.x >= -B.pos.w;
            if (Ain && Bin) {
                temp1.push_back(B);
            } else if (Ain) {
                const Float t = (-A.pos.w - A.pos.x) / ((B.pos.x - A.pos.x) + (B.pos.w - A.pos.w));
                temp1.push_back(interpolate(A, B, t));
            } else if (Bin) {
                const Float t = (-A.pos.w - A.pos.x) / ((B.pos.x - A.pos.x) + (B.pos.w - A.pos.w));
                temp1.push_back(interpolate(A, B, t));
                temp1.push_back(B);
            }
        }

        // 右平面 (x <= w)
        temp2.clear();
        for (UInt32 i = 0; i < temp1.size(); i++) {
            const ShadowClipVertex& A = temp1[i];
            const ShadowClipVertex& B = temp1[(i + 1) % temp1.size()];
            const Boolean Ain = A.pos.x <= A.pos.w;
            const Boolean Bin = B.pos.x <= B.pos.w;
            if (Ain && Bin) {
                temp2.push_back(B);
            } else if (Ain) {
                const Float t = (A.pos.w - A.pos.x) / ((B.pos.x - A.pos.x) - (B.pos.w - A.pos.w));
                temp2.push_back(interpolate(A, B, t));
            } else if (Bin) {
                const Float t = (A.pos.w - A.pos.x) / ((B.pos.x - A.pos.x) - (B.pos.w - A.pos.w));
                temp2.push_back(interpolate(A, B, t));
                temp2.push_back(B);
            }
        }

        // 下平面 (y >= -w)
        temp1.clear();
        for (UInt32 i = 0; i < temp2.size(); i++) {
            const ShadowClipVertex& A = temp2[i];
            const ShadowClipVertex& B = temp2[(i + 1) % temp2.size()];
            const Boolean Ain = A.pos.y >= -A.pos.w;
            const Boolean Bin = B.pos.y >= -B.pos.w;
            if (Ain && Bin) {
                temp1.push_back(B);
            } else if (Ain) {
                const Float t = (-A.pos.w - A.pos.y) / ((B.pos.y - A.pos.y) + (B.pos.w - A.pos.w));
                temp1.push_back(interpolate(A, B, t));
            } else if (Bin) {
                const Float t = (-A.pos.w - A.pos.y) / ((B.pos.y - A.pos.y) + (B.pos.w - A.pos.w));
                temp1.push_back(interpolate(A, B, t));
                temp1.push_back(B);
            }
        }

        // 上平面 (y <= w)
        temp2.clear();
        for (UInt32 i = 0; i < temp1.size(); i++) {
            const ShadowClipVertex& A = temp1[i];
            const ShadowClipVertex& B = temp1[(i + 1) % temp1.size()];
            const Boolean Ain = A.pos.y <= A.pos.w;
            const Boolean Bin = B.pos.y <= B.pos.w;
            if (Ain && Bin) {
                temp2.push_back(B);
            } else if (Ain) {
                const Float t = (A.pos.w - A.pos.y) / ((B.pos.y - A.pos.y) - (B.pos.w - A.pos.w));
                temp2.push_back(interpolate(A, B, t));
            } else if (Bin) {
                const Float t = (A.pos.w - A.pos.y) / ((B.pos.y - A.pos.y) - (B.pos.w - A.pos.w));
                temp2.push_back(interpolate(A, B, t));
                temp2.push_back(B);
            }
        }
    }

    Boolean isBackFace(const Vector4D& ndc1, const Vector4D& ndc2, const Vector4D& ndc3) {
        return (ndc2.x - ndc1.x) * (ndc3.y - ndc1.y) - (ndc2.y - ndc1.y) * (ndc3.x - ndc1.x) >= 0.0f;
    }

    inline void processAllInTriangle(const ShadowClipVertex& v1, const ShadowClipVertex& v2, const ShadowClipVertex& v3,
                                     const Float hsw, const Float hsh, List<ShadowTriangle>& result) {
        __m256 vecX = _mm256_setr_ps(v1.pos.x, v2.pos.x, v3.pos.x, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        __m256 vecY = _mm256_setr_ps(v1.pos.y, v2.pos.y, v3.pos.y, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        __m256 vecZ = _mm256_setr_ps(v1.pos.z, v2.pos.z, v3.pos.z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        __m256 vecW = _mm256_setr_ps(v1.pos.w, v2.pos.w, v3.pos.w, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        __m256 invW = _mm256_div_ps(_mm256_set1_ps(1.0f), vecW);

        __m256 ndcX = _mm256_mul_ps(vecX, invW);
        __m256 ndcY = _mm256_mul_ps(vecY, invW);
        __m256 ndcZ = _mm256_mul_ps(vecZ, invW);

        alignas(32) Float resNdcX[8];
        alignas(32) Float resNdcY[8];
        alignas(32) Float resNdcZ[8];
        alignas(32) Float resInvW[8];
        _mm256_store_ps(resNdcX, ndcX);
        _mm256_store_ps(resNdcY, ndcY);
        _mm256_store_ps(resNdcZ, ndcZ);
        _mm256_store_ps(resInvW, invW);

        // 背面剔除
        Float backFaceCross = (resNdcX[1] - resNdcX[0]) * (resNdcY[2] - resNdcY[0])
                            - (resNdcY[1] - resNdcY[0]) * (resNdcX[2] - resNdcX[0]);
        if (backFaceCross >= 0.0f) return;

        // 视口映射
        __m256 vpX = _mm256_add_ps(_mm256_mul_ps(ndcX, _mm256_set1_ps(hsw)), _mm256_set1_ps(hsw));
        __m256 vpY = _mm256_add_ps(_mm256_mul_ps(ndcY, _mm256_set1_ps(-hsh)), _mm256_set1_ps(hsh));

        alignas(32) Float resVpX[8];
        alignas(32) Float resVpY[8];
        _mm256_store_ps(resVpX, vpX);
        _mm256_store_ps(resVpY, vpY);

        result.push_back({
            {static_cast<Int64>(resVpX[0]), static_cast<Int64>(resVpY[0])},
            {static_cast<Int64>(resVpX[1]), static_cast<Int64>(resVpY[1])},
            {static_cast<Int64>(resVpX[2]), static_cast<Int64>(resVpY[2])},
            v1.pos.w,
            v2.pos.w,
            v3.pos.w
        });
    }


    void processRange(const Scene3DSnapShot* snapShot, const TransformRange& range, const ShadowClipVertex* clipSpaceVertex, const Matrix4x4& viewPortMatrix, List<ShadowTriangle>& result) {
        List<ShadowClipVertex> globalTemp1;
        List<ShadowClipVertex> globalTemp2;
        globalTemp1.reserve(12);
        globalTemp2.reserve(12);
        result.reserve(range.end - range.start);

        const Triangle* triangles = snapShot->renderPackages[range.pkgIndex].triangles;

        for (UInt32 i = range.start; i < range.end; i++) {
            const Triangle& triangle = triangles[i];
            const ShadowClipVertex& v1 = clipSpaceVertex[triangle.v1];
            const ShadowClipVertex& v2 = clipSpaceVertex[triangle.v2];
            const ShadowClipVertex& v3 = clipSpaceVertex[triangle.v3];

            if (v1.pos.z <= 0 && v2.pos.z <= 0 && v3.pos.z <= 0) continue;
            if (v1.pos.x <= -v1.pos.w && v2.pos.x <= -v2.pos.w && v3.pos.x <= -v3.pos.w) continue;
            if (v1.pos.x >= v1.pos.w && v2.pos.x >= v2.pos.w && v3.pos.x >= v3.pos.w) continue;
            if (v1.pos.y <= -v1.pos.w && v2.pos.y <= -v2.pos.w && v3.pos.y <= -v3.pos.w) continue;
            if (v1.pos.y >= v1.pos.w && v2.pos.y >= v2.pos.w && v3.pos.y >= v3.pos.w) continue;

            const Boolean allIn =
                v1.pos.z > 0 && v2.pos.z > 0 && v3.pos.z > 0 &&
                v1.pos.x > -v1.pos.w && v2.pos.x > -v2.pos.w && v3.pos.x > -v3.pos.w &&
                v1.pos.x < v1.pos.w && v2.pos.x < v2.pos.w && v3.pos.x < v3.pos.w &&
                v1.pos.y > -v1.pos.w && v2.pos.y > -v2.pos.w && v3.pos.y > -v3.pos.w &&
                v1.pos.y < v1.pos.w && v2.pos.y < v2.pos.w && v3.pos.y < v3.pos.w;

            if (allIn) {
                processAllInTriangle(v1, v2, v3, viewPortMatrix[3], viewPortMatrix[7], result);
                continue;
            }

            clip(v1, v2, v3, globalTemp1, globalTemp2);
            const UInt32 totalVertices = globalTemp2.size();
            if (totalVertices < 3) continue;

            const ShadowClipVertex& clipV1 = globalTemp2[0];
            const Float invW1 = 1.0f / clipV1.pos.w;
            const Vector4D v1ndc{clipV1.pos.x * invW1, clipV1.pos.y * invW1, clipV1.pos.z * invW1, 1.0f};
            const Vector4D v1vp = viewPortMatrix * v1ndc;

            for (UInt32 t = 1; t < totalVertices - 1; t++) {
                const ShadowClipVertex& clipV2 = globalTemp2[t];
                const ShadowClipVertex& clipV3 = globalTemp2[t + 1];

                const Float invW2 = 1.0f / clipV2.pos.w;
                const Float invW3 = 1.0f / clipV3.pos.w;

                const Vector4D v2ndc{clipV2.pos.x * invW2, clipV2.pos.y * invW2, clipV2.pos.z * invW2, 1.0f};
                const Vector4D v3ndc{clipV3.pos.x * invW3, clipV3.pos.y * invW3, clipV3.pos.z * invW3, 1.0f};

                if (isBackFace(v1ndc, v2ndc, v3ndc)) continue;

                const Vector4D v2vp = viewPortMatrix * v2ndc;
                const Vector4D v3vp = viewPortMatrix * v3ndc;

                result.push_back({
                    {static_cast<Int64>(v1vp.x), static_cast<Int64>(v1vp.y)},
                    {static_cast<Int64>(v2vp.x), static_cast<Int64>(v2vp.y)},
                    {static_cast<Int64>(v3vp.x), static_cast<Int64>(v3vp.y)},
                    clipV1.pos.w,
                    clipV2.pos.w,
                    clipV3.pos.w
                });
            }
        }
    }

    class VertexProcessorTask final : public Runnable {
    public:
        ShadowClipVertex* resultArray;
        TransformRange range;
        const Scene3DSnapShot* sceneSnapShot;
        Matrix4x4 mvp;

        void run() override {
            for (UInt32 i = range.start; i < range.end; i++) {
                resultArray[i].pos = mvp * sceneSnapShot->renderPackages[range.pkgIndex].vertices[i].pos;
            }
        }
    };

    class TriangleProcessorTask final : public Runnable {
    public:
        const Scene3DSnapShot* snapShot;
        TransformRange range;
        const ShadowClipVertex* clipVertexes;
        Matrix4x4 viewPortMatrix;
        List<ShadowTriangle> result{};

        void run() override {
            result.reserve((range.end - range.start) * 0.8);
            processRange(snapShot, range, clipVertexes, viewPortMatrix, result);
            // Log::debug("Task %d done, has %d screen triangles", range.pkgIndex, result.size());
        }
    };

    List<List<ShadowTriangle>> process(const Scene3DSnapShot* sceneSnapShot, const Camera& lightCamera, ThreadPool& threadPool, const Int32 resolution, const Int32 taskCount) {
        Int64 step1Start = millisTime();
        Int32 totalTriangles = 0;
        Int32 totalVertexes = 0;
        for (auto& pkg : sceneSnapShot->renderPackages) {
            totalVertexes += pkg.vertexCount;
            totalTriangles += pkg.triangleCount;
        }
        auto tasks = make_unique_for_overwrite<UniquePtr<Runnable>[]>(taskCount + 10);
        Int32 taskWritePos = 0;

        Int32 pkgIndex = 0;
        List<UniquePtr<ShadowClipVertex[]>> clipSpaceVertexList;
        clipSpaceVertexList.reserve(sceneSnapShot->renderPackages.size());
        for (auto& pkg : sceneSnapShot->renderPackages) {
            Int32 vertexTaskForThisPkg = static_cast<Float>(pkg.vertexCount) / totalVertexes * taskCount;
            if (vertexTaskForThisPkg < 1) vertexTaskForThisPkg = 1;
            Int32 vertexPerTask = static_cast<Int32>(static_cast<Float>(pkg.vertexCount) / vertexTaskForThisPkg);
            if (vertexPerTask < 1) vertexPerTask = 1;

            const Matrix4x4 mvp = lightCamera.getProjectionMatrix(1) * lightCamera.getViewTransformMatrix() * pkg.modelMatrix;

            UniquePtr<ShadowClipVertex[]> clipSpaceVertex = make_unique_for_overwrite<ShadowClipVertex[]>(pkg.vertexCount);

            for (Int32 i = 0; i < pkg.vertexCount; i += vertexPerTask) {
                Int32 vertexCount = std::min(vertexPerTask, static_cast<Int32>(pkg.vertexCount - i));

                TransformRange range{};
                range.pkgIndex = pkgIndex;
                range.start = i;
                range.end = i + vertexCount;

                auto* task = new VertexProcessorTask;
                task->range = range;
                task->sceneSnapShot = sceneSnapShot;
                task->resultArray = clipSpaceVertex.get();
                task->mvp = mvp;

                tasks[taskWritePos++] = UniquePtr<Runnable>(task);
            }
            clipSpaceVertexList.push_back(move(clipSpaceVertex));
            pkgIndex++;
        }
        Int64 step1end = millisTime();
        threadPool.submit(tasks.get(), taskWritePos);
        Int64 step2start = millisTime();
        std::fill_n(tasks.get(), taskWritePos, nullptr);
        taskWritePos = 0;
        pkgIndex = 0;
        Matrix4x4 viewPortMatrix{};
        viewPortMatrix[0] = 0.5f * resolution;
        viewPortMatrix[3] = 0.5f * resolution;
        viewPortMatrix[5] = -0.5f * resolution;
        viewPortMatrix[7] = 0.5f * resolution;
        viewPortMatrix[10] = -1.0f;
        viewPortMatrix[11] = 1.0f;
        viewPortMatrix[15] = 1.0f;
        for (auto& pkg : sceneSnapShot->renderPackages) {
            Int32 triangleTaskForThisPkg = static_cast<Float>(pkg.triangleCount) / totalTriangles * taskCount;
            if (triangleTaskForThisPkg < 1) triangleTaskForThisPkg = 1;
            Int32 trianglePerTask = static_cast<Int32>(static_cast<Float>(pkg.triangleCount) / triangleTaskForThisPkg);
            if (trianglePerTask < 1) trianglePerTask = 1;

            for (Int32 i = 0; i < pkg.triangleCount; i += trianglePerTask) {
                Int32 triangleCount = std::min(trianglePerTask, static_cast<Int32>(pkg.triangleCount - i));

                TransformRange range{};
                range.pkgIndex = pkgIndex;
                range.start = i;
                range.end = i + triangleCount;

                auto* task = new TriangleProcessorTask;
                task->snapShot = sceneSnapShot;
                task->range = range;
                task->clipVertexes = clipSpaceVertexList[pkgIndex].get();
                task->viewPortMatrix = viewPortMatrix;

                tasks[taskWritePos++] = UniquePtr<Runnable>(task);
            }
            pkgIndex++;
        }
        Int64 step2end = millisTime();
        threadPool.submit(tasks.get(), taskWritePos);
        Int64 mergeStart = millisTime();

        List<List<ShadowTriangle>> finalResult;
        finalResult.reserve(taskWritePos);
        for (Int32 i = 0; i < taskWritePos; i++) {
            auto* t = static_cast<TriangleProcessorTask*>(tasks[i].get());
            finalResult.push_back(move(t->result));
        }

        // Log::debug("\nStep 1 prepare %d\nStep 1 submit %d\nStep 2 prepare %d\nStep 2 submit %d\nMerge %d", step1end - step1Start, step2start - step1end, step2end - step2start, mergeStart - step2end, millisTime() - mergeStart);

        return finalResult;
    }
}

export class ShadowTile {
public:
    Int32 x, y, width, height, size;
    List<const ShadowTriangle*> shadowTriangles{};

    ShadowTile(const Int32 x, const Int32 y, const Int32 width, const Int32 height):
        x(x), y(y), width(width), height(height), size(width * height) {}

    ShadowTile(): ShadowTile(0, 0, 0, 0) {}

    static List<ShadowTile> divideScreen(const Int32 screenWidth, const Int32 screenHeight, const Int32 tileSize) {
        List<ShadowTile> tiles;

        for (Int32 y = 0; y < screenHeight; y += tileSize) {
            for (Int32 x = 0; x < screenWidth; x += tileSize) {
                const Int32 tileWidth = std::min(tileSize, screenWidth - x);
                const Int32 tileHeight = std::min(tileSize, screenHeight - y);
                tiles.emplace_back(x, y, tileWidth, tileHeight);
            }
        }

        return tiles;
    }
};

export namespace ShadowTriangleProcessor {
    using std::min;
    using std::max;

    void binning(ShadowTile* tiles, const List<List<ShadowTriangle>>& trianglesList, const UInt32 tileCount, const Int32 tileSize, const Int32 screenW, const Int32 screenH) {
        const Int32 tileRows = (screenH + tileSize - 1) / tileSize;
        const Int32 tileCols = (screenW + tileSize - 1) / tileSize;
        const Int32 shift = std::log2(tileSize);

        for (UInt32 i = 0; i < tileCount; i++) {
            tiles[i].shadowTriangles.clear();
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
                __m256i v1x = _mm256_setr_epi32(chunk[tIdx+0].v1.x, chunk[tIdx+1].v1.x, chunk[tIdx+2].v1.x, chunk[tIdx+3].v1.x,
                                                chunk[tIdx+4].v1.x, chunk[tIdx+5].v1.x, chunk[tIdx+6].v1.x, chunk[tIdx+7].v1.x);
                __m256i v2x = _mm256_setr_epi32(chunk[tIdx+0].v2.x, chunk[tIdx+1].v2.x, chunk[tIdx+2].v2.x, chunk[tIdx+3].v2.x,
                                                chunk[tIdx+4].v2.x, chunk[tIdx+5].v2.x, chunk[tIdx+6].v2.x, chunk[tIdx+7].v2.x);
                __m256i v3x = _mm256_setr_epi32(chunk[tIdx+0].v3.x, chunk[tIdx+1].v3.x, chunk[tIdx+2].v3.x, chunk[tIdx+3].v3.x,
                                                chunk[tIdx+4].v3.x, chunk[tIdx+5].v3.x, chunk[tIdx+6].v3.x, chunk[tIdx+7].v3.x);

                __m256i v1y = _mm256_setr_epi32(chunk[tIdx+0].v1.y, chunk[tIdx+1].v1.y, chunk[tIdx+2].v1.y, chunk[tIdx+3].v1.y,
                                                chunk[tIdx+4].v1.y, chunk[tIdx+5].v1.y, chunk[tIdx+6].v1.y, chunk[tIdx+7].v1.y);
                __m256i v2y = _mm256_setr_epi32(chunk[tIdx+0].v2.y, chunk[tIdx+1].v2.y, chunk[tIdx+2].v2.y, chunk[tIdx+3].v2.y,
                                                chunk[tIdx+4].v2.y, chunk[tIdx+5].v2.y, chunk[tIdx+6].v2.y, chunk[tIdx+7].v2.y);
                __m256i v3y = _mm256_setr_epi32(chunk[tIdx+0].v3.y, chunk[tIdx+1].v3.y, chunk[tIdx+2].v3.y, chunk[tIdx+3].v3.y,
                                                chunk[tIdx+4].v3.y, chunk[tIdx+5].v3.y, chunk[tIdx+6].v3.y, chunk[tIdx+7].v3.y);

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
                            tiles[row + tileX].shadowTriangles.push_back(&chunk[currentTIdx]);
                        }
                    }
                }
            }

            // 2. 处理当前 chunk 剩下的不满足 8 个的尾数（标量处理）
            for (; tIdx < size; tIdx++) {
                const auto& [v1, v2, v3, d1, d2, d3] = chunk[tIdx];
                const Int32 xMin = std::max(static_cast<Int64>(0),       std::min(v1.x, std::min(v2.x, v3.x)));
                const Int32 xMax = std::min(static_cast<Int64>(screenW), std::max(v1.x, std::max(v2.x, v3.x)));
                const Int32 yMin = std::max(static_cast<Int64>(0),       std::min(v1.y, std::min(v2.y, v3.y)));
                const Int32 yMax = std::min(static_cast<Int64>(screenH), std::max(v1.y, std::max(v2.y, v3.y)));

                const Int32 tileXStart = std::clamp(xMin >> shift, 0, tileCols - 1);
                const Int32 tileXEnd   = std::clamp(xMax >> shift, 0, tileCols - 1);
                const Int32 tileYStart = std::clamp(yMin >> shift, 0, tileRows - 1);
                const Int32 tileYEnd   = std::clamp(yMax >> shift, 0, tileRows - 1);

                for (Int32 tileY = tileYStart; tileY <= tileYEnd; tileY++) {
                    const Int32 row = tileY * tileCols;
                    for (Int32 tileX = tileXStart; tileX <= tileXEnd; tileX++) {
                        tiles[row + tileX].shadowTriangles.push_back(&chunk[tIdx]);
                    }
                }
            }
        }
    }
}

export class ShadowTileTask final : public Runnable {
    ShadowTile* tile;
    Float* shadowMap;
    Int32 resolution;

    void clearMap() const {
        const __m256i depthVec = _mm256_set1_epi32(0x42424242);
        auto* depths = reinterpret_cast<Float*>(shadowMap);

        // 计算安全的裁剪边界
        const UInt32 maxY = std::min(tile->y + tile->height, resolution);
        const UInt32 maxX = std::min(tile->x + tile->width, resolution);

        for (UInt32 y = tile->y; y < maxY; y++) {
            const UInt32 rowOffset = y * resolution;
            UInt32 x = tile->x;

            // 计算当前行剩余可用的安全宽度
            UInt32 span = maxX - x;
            for (; x < tile->x + (span & ~7); x += 8) {
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(&depths[rowOffset + x]), depthVec);
            }
            for (; x < maxX; x++) {
                depths[rowOffset + x] = 1e30f;
            }
        }
    }
public:
    explicit ShadowTileTask(ShadowTile& tile) {
        this->tile = &tile;
    }
    explicit ShadowTileTask(ShadowTile* tile) {
        this->tile = tile;
    }


    void setDrawBuffer(Float* shadowMap) {
        this->shadowMap = shadowMap;
    }

    void setResolution(const Int32 r) {
        this->resolution = r;
    }

    void run() override {
        clearMap();

        if (const UInt32 size = tile->shadowTriangles.size(); size > 0) {
            for (UInt32 i = 0; i < size; i++) {
                drawShadowMapAvx2(*tile->shadowTriangles[i], tile, shadowMap, resolution);
            }
        }
    }

    void drawShadowMapAvx2(const ShadowTriangle& triangle, const ShadowTile* tile, Float* shadowMap, const Int32 resolution) {
        // 复制顶点
        VectorInt2D v1 = triangle.v1;
        VectorInt2D v2 = triangle.v2;
        VectorInt2D v3 = triangle.v3;
        Float d1 = triangle.d1;
        Float d2 = triangle.d2;
        Float d3 = triangle.d3;

        // 排序顶点
        if (v1.y > v2.y) {
            std::swap(v1, v2);
            std::swap(d1, d2);
        }
        if (v2.y > v3.y) {
            std::swap(v2, v3);
            std::swap(d2, d3);
        }
        if (v1.y > v2.y) {
            std::swap(v1, v2);
            std::swap(d1, d2);
        }

        const Float sABC_inv = 2.0f / std::abs((v3.x - v1.x) * (v2.y - v1.y) - (v3.y - v1.y) * (v2.x - v1.x));

        const Int32 ys = std::clamp(static_cast<Int32>(v1.y), static_cast<Int32>(tile->y), tile->y + tile->height);
        const Int32 ye = std::clamp(static_cast<Int32>(v3.y), static_cast<Int32>(tile->y), tile->y + tile->height);

        const Boolean unhorizontal_AB = v1.y != v2.y;
        const Boolean unhorizontal_BC = v2.y != v3.y;

        alignas(32) Float depthStackBuffer[8];

        // 扫描线填充
        for (Int32 y = ys; y < ye; y++) {
            Int32 xa, xb = v1.x + (v3.x - v1.x) * (y - v1.y) / (v3.y - v1.y);

            if (y < v2.y && unhorizontal_AB) {
                xa = v1.x + (v2.x - v1.x) * (y - v1.y) / (v2.y - v1.y);
            } else if (unhorizontal_BC) {
                xa = v2.x + (v3.x - v2.x) * (y - v2.y) / (v3.y - v2.y);
            }
            else continue;

            const Int32 xs = std::clamp(std::min(xa, xb), static_cast<Int32>(tile->x), tile->x + tile->width);
            const Int32 xe = std::clamp(std::max(xa, xb), static_cast<Int32>(tile->x), tile->x + tile->width);

            Float* shadowRow = shadowMap + y * resolution;

            for (Int32 x = xs; x < xe; x += 8) {
                Int32 size = std::min(xe - x, 8);

                Vec8f subX = Vec8f(x) + Vec8f(0, 1, 2, 3, 4, 5, 6, 7);
                Vec8f subY = Vec8f(y);

                Vec8f PAx = Vec8f(v1.x) - subX;
                Vec8f PAy = Vec8f(v1.y) - subY;
                Vec8f PBx = Vec8f(v2.x) - subX;
                Vec8f PBy = Vec8f(v2.y) - subY;
                Vec8f PCx = Vec8f(v3.x) - subX;
                Vec8f PCy = Vec8f(v3.y) - subY;

                Vec8f signMask = Vec8f(-0.0f);
                Vec8f half = Vec8f(0.5f);
                Vec8f sBPC = Vec8f::andNot(signMask, Vec8f::fms(PBx, PCy, PBy * PCx)) * half;
                Vec8f sAPC = Vec8f::andNot(signMask, Vec8f::fms(PAx, PCy, PAy * PCx)) * half;

                Vec8f invABC = Vec8f(sABC_inv);
                Vec8f alphaS = sBPC * invABC;
                Vec8f betaS = sAPC * invABC;
                Vec8f gammaS = Vec8f(1.0f) - (alphaS + betaS);

                Vec8f depth8f = Vec8f::fma(alphaS, d1, Vec8f::fma(betaS, d2, gammaS * d3));

                // 批量深度剔除
                __m256 rowDepth = _mm256_loadu_ps(&shadowRow[x]);
                __m256 cmp = _mm256_cmp_ps(depth8f, rowDepth, _CMP_NGT_UQ);
                Int32 mask = _mm256_movemask_ps(cmp);
                if ((((1 << size) - 1) & mask) == 0) continue;
                depth8f.store(depthStackBuffer);

                // 填充像素
                for (Int32 i = 0; i < size; i++) {
                    const Float depth = depthStackBuffer[i];
                        if (const Int32 col = x + i;  depth < shadowRow[col]) {
                        shadowRow[col] = depth;
                    }
                }
            }
        }
    }
};