module;
#include <vector>
#include <memory>
#include <string>
#include <immintrin.h>
export module VertexProcessor;
import Types;
import Matrix;
import Triangle;
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

struct ClipVertex {
    Vector4D pos;
    Vector3D normal;
    Vector2D uv;
    Vector3D worldPos;

    String toString() const {
        return "ClipVertex(pos=" + pos.toString() + ", normal=" + normal.toString() + ")";
    }
};

struct TransformRange {
    UInt32 pkgIndex;
    UInt32 start;
    UInt32 end;
};

export namespace VertexProcessor {
    ClipVertex interpolate(const ClipVertex& v1, const ClipVertex& v2, const Float t) {
        return {
            v1.pos * (1 - t) + v2.pos * t,
            v1.normal * (1 - t) + v2.normal * t,
            v1.uv * (1 - t) + v2.uv * t,
            v1.worldPos * (1 - t) + v2.worldPos * t
        };
    }

    // 最终数据在temp2中
    void clip(const ClipVertex& v1, const ClipVertex& v2, const ClipVertex& v3, List<ClipVertex>& temp1, List<ClipVertex>& temp2) {
        // Log::debug("Run Clip");
        temp1.clear();
        temp2.clear();
        temp1.push_back(v1);
        temp1.push_back(v2);
        temp1.push_back(v3);

        // 近平面 (z >= 0)
        for (UInt32 i = 0; i < temp1.size(); i++) {
            const ClipVertex& A = temp1[i];
            const ClipVertex& B = temp1[(i + 1) % temp1.size()];
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
            const ClipVertex& A = temp2[i];
            const ClipVertex& B = temp2[(i + 1) % temp2.size()];
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
            const ClipVertex& A = temp1[i];
            const ClipVertex& B = temp1[(i + 1) % temp1.size()];
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
            const ClipVertex& A = temp2[i];
            const ClipVertex& B = temp2[(i + 1) % temp2.size()];
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
            const ClipVertex& A = temp1[i];
            const ClipVertex& B = temp1[(i + 1) % temp1.size()];
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

    inline void processAllInTriangle(const ClipVertex& v1, const ClipVertex& v2, const ClipVertex& v3,
                                     const Float hsw, const Float hsh, const Triangle& triangle, List<ScreenTriangle>& result) {
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
            {{static_cast<Int64>(resVpX[0]), static_cast<Int64>(resVpY[0])}, 1.0f - resNdcZ[0], resInvW[0], v1.normal, v1.uv, v1.worldPos},
            {{static_cast<Int64>(resVpX[1]), static_cast<Int64>(resVpY[1])}, 1.0f - resNdcZ[1], resInvW[1], v2.normal, v2.uv, v2.worldPos},
            {{static_cast<Int64>(resVpX[2]), static_cast<Int64>(resVpY[2])}, 1.0f - resNdcZ[2], resInvW[2], v3.normal, v3.uv, v3.worldPos},
            triangle.texture
        });
    }


    void processRange(const Scene3DSnapShot* snapShot, const TransformRange& range, const ClipVertex* clipSpaceVertex, const Matrix4x4& viewPortMatrix, List<ScreenTriangle>& result) {
        List<ClipVertex> globalTemp1;
        List<ClipVertex> globalTemp2;
        globalTemp1.reserve(12);
        globalTemp2.reserve(12);
        result.reserve(range.end - range.start);

        const Triangle* triangles = snapShot->renderPackages[range.pkgIndex].triangles;

        for (UInt32 i = range.start; i < range.end; i++) {
            const Triangle& triangle = triangles[i];
            const ClipVertex& v1 = clipSpaceVertex[triangle.v1];
            const ClipVertex& v2 = clipSpaceVertex[triangle.v2];
            const ClipVertex& v3 = clipSpaceVertex[triangle.v3];

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
                processAllInTriangle(v1, v2, v3, viewPortMatrix[3], viewPortMatrix[7], triangle, result);
                continue;
            }

            clip(v1, v2, v3, globalTemp1, globalTemp2);
            const UInt32 totalVertices = globalTemp2.size();
            if (totalVertices < 3) continue;

            const ClipVertex& clipV1 = globalTemp2[0];
            const Float invW1 = 1.0f / clipV1.pos.w;
            const Vector4D v1ndc{clipV1.pos.x * invW1, clipV1.pos.y * invW1, clipV1.pos.z * invW1, 1.0f};
            const Vector4D v1vp = viewPortMatrix * v1ndc;

            for (UInt32 t = 1; t < totalVertices - 1; t++) {
                const ClipVertex& clipV2 = globalTemp2[t];
                const ClipVertex& clipV3 = globalTemp2[t + 1];

                const Float invW2 = 1.0f / clipV2.pos.w;
                const Float invW3 = 1.0f / clipV3.pos.w;

                const Vector4D v2ndc{clipV2.pos.x * invW2, clipV2.pos.y * invW2, clipV2.pos.z * invW2, 1.0f};
                const Vector4D v3ndc{clipV3.pos.x * invW3, clipV3.pos.y * invW3, clipV3.pos.z * invW3, 1.0f};

                if (isBackFace(v1ndc, v2ndc, v3ndc)) continue;

                const Vector4D v2vp = viewPortMatrix * v2ndc;
                const Vector4D v3vp = viewPortMatrix * v3ndc;

                result.push_back({
                    {{static_cast<Int64>(v1vp.x), static_cast<Int64>(v1vp.y)}, 1.0f - v1ndc.z, invW1, clipV1.normal, clipV1.uv, clipV1.worldPos},
                    {{static_cast<Int64>(v2vp.x), static_cast<Int64>(v2vp.y)}, 1.0f - v2ndc.z, invW2, clipV2.normal, clipV2.uv, clipV2.worldPos},
                    {{static_cast<Int64>(v3vp.x), static_cast<Int64>(v3vp.y)}, 1.0f - v3ndc.z, invW3, clipV3.normal, clipV3.uv, clipV3.worldPos},
                    triangle.texture
                });
            }
        }
    }

    class VertexProcessorTask final : public Runnable {
    public:
        ClipVertex* resultArray;
        TransformRange range;
        const Scene3DSnapShot* sceneSnapShot;
        Matrix4x4 m;
        Matrix4x4 vp;
        Matrix3x3 nMatrix;

        void run() override {
            for (UInt32 i = range.start; i < range.end; i++) {
                const auto&[pos, normal, uv] = sceneSnapShot->renderPackages[range.pkgIndex].vertices[i];
                Vector4D wordPos = m * pos;
                resultArray[i].worldPos = static_cast<Vector3D>(wordPos);
                resultArray[i].pos = vp * wordPos;
                resultArray[i].normal = nMatrix * normal;
                resultArray[i].uv = uv;
            }
        }
    };

    class TriangleProcessorTask final : public Runnable {
    public:
        const Scene3DSnapShot* snapShot;
        TransformRange range;
        const ClipVertex* clipVertexes;
        Matrix4x4 viewPortMatrix;
        List<ScreenTriangle> result{};

        void run() override {
            result.reserve((range.end - range.start) * 0.8);
            processRange(snapShot, range, clipVertexes, viewPortMatrix, result);
            // Log::debug("Task %d done, has %d screen triangles", range.pkgIndex, result.size());
        }
    };

    List<List<ScreenTriangle>> process(const Scene3DSnapShot* sceneSnapShot, ThreadPool& threadPool, const Int32 taskCount) {
        Int64 step1Start = millisTime();
        Int32 totalTriangles = 0;
        Int32 totalVertexes = 0;
        for (auto& pkg : sceneSnapShot->renderPackages) {
            totalVertexes += pkg.vertexCount;
            totalTriangles += pkg.triangleCount;
        }
        auto tasks = make_unique_for_overwrite<UniquePtr<Runnable>[]>(taskCount + 100);
        Int32 taskWritePos = 0;

        Int32 pkgIndex = 0;
        List<UniquePtr<ClipVertex[]>> clipSpaceVertexList;
        clipSpaceVertexList.reserve(sceneSnapShot->renderPackages.size());
        for (auto& pkg : sceneSnapShot->renderPackages) {
            Int32 vertexTaskForThisPkg = static_cast<Float>(pkg.vertexCount) / totalVertexes * taskCount;
            if (vertexTaskForThisPkg < 1) vertexTaskForThisPkg = 1;
            Int32 vertexPerTask = static_cast<Int32>(static_cast<Float>(pkg.vertexCount) / vertexTaskForThisPkg);
            if (vertexPerTask < 1) vertexPerTask = 1;

            const Matrix4x4 vp = sceneSnapShot->projectionMatrix * sceneSnapShot->viewMatrix;
            const Matrix3x3 normalMatrix = static_cast<Matrix3x3>(pkg.modelMatrix).inverse().transpose();

            UniquePtr<ClipVertex[]> clipSpaceVertex = make_unique_for_overwrite<ClipVertex[]>(pkg.vertexCount);

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
                task->m = pkg.modelMatrix;
                task->vp = vp;
                task->nMatrix = normalMatrix;

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
        viewPortMatrix[0] = 0.5f * sceneSnapShot->screenWidth;
        viewPortMatrix[3] = 0.5f * sceneSnapShot->screenWidth;
        viewPortMatrix[5] = -0.5f * sceneSnapShot->screenHeight;
        viewPortMatrix[7] = 0.5f * sceneSnapShot->screenHeight;
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

        List<List<ScreenTriangle>> finalResult;
        finalResult.reserve(taskWritePos);
        for (Int32 i = 0; i < taskWritePos; i++) {
            auto* t = static_cast<TriangleProcessorTask*>(tasks[i].get());
            finalResult.push_back(move(t->result));
        }

        // Log::debug("\nStep 1 prepare %d\nStep 1 submit %d\nStep 2 prepare %d\nStep 2 submit %d\nMerge %d", step1end - step1Start, step2start - step1end, step2end - step2start, mergeStart - step2end, millisTime() - mergeStart);

        return finalResult;
    }

    List<ScreenTriangle> process(const Scene3DSnapShot* sceneSnapShot) {
        List<ScreenTriangle> result;
        List<ClipVertex> globalTemp1;
        List<ClipVertex> globalTemp2;
        globalTemp1.reserve(12);
        globalTemp2.reserve(12);

        Int64 totalTriangleCount = 0;
        for (const auto& pkg : sceneSnapShot->renderPackages)
            totalTriangleCount += pkg.triangleCount;
        result.reserve(totalTriangleCount * 1.1);

        const Matrix4x4 view_project = sceneSnapShot->projectionMatrix * sceneSnapShot->viewMatrix;
        Matrix4x4 viewPortMatrix{};
        viewPortMatrix[0] = 0.5f * sceneSnapShot->screenWidth;
        viewPortMatrix[3] = 0.5f * sceneSnapShot->screenWidth;
        viewPortMatrix[5] = -0.5f * sceneSnapShot->screenHeight;
        viewPortMatrix[7] = 0.5f * sceneSnapShot->screenHeight;
        viewPortMatrix[10] = -1.0f;
        viewPortMatrix[11] = 1.0f;
        viewPortMatrix[15] = 1.0f;

        for (const auto & pkg : sceneSnapShot->renderPackages) {
            const Matrix4x4 mvp = view_project * pkg.modelMatrix;
            // Log::debug(mvp.toString());
            const Matrix3x3 normalMatrix = static_cast<Matrix3x3>(pkg.modelMatrix).inverse().transpose();

            const Vertex* vertexes = pkg.vertices;
            const Triangle* triangles = pkg.triangles;

            UniquePtr<ClipVertex[]> clipSpaceVertex = make_unique_for_overwrite<ClipVertex[]>(pkg.vertexCount);
            // Int64 start = millisTime();
            for (UInt32 i = 0; i < pkg.vertexCount; i++) {
                clipSpaceVertex[i].pos = mvp * vertexes[i].pos;
                clipSpaceVertex[i].normal = normalMatrix * vertexes[i].normal;
                clipSpaceVertex[i].uv = vertexes[i].uv;
            }
            // Log::debug("clip space vertex transform time: %d", millisTime() - start);

            for (UInt32 i = 0; i < pkg.triangleCount; i++) {
                const Triangle& triangle = triangles[i];
                const ClipVertex& v1 = clipSpaceVertex[triangle.v1];
                const ClipVertex& v2 = clipSpaceVertex[triangle.v2];
                const ClipVertex& v3 = clipSpaceVertex[triangle.v3];

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
                    processAllInTriangle(v1, v2, v3, viewPortMatrix[3], viewPortMatrix[7], triangle, result);
                    continue;
                }

                clip(v1, v2, v3, globalTemp1, globalTemp2);
                const UInt32 totalVertices = globalTemp2.size();
                if (totalVertices < 3) continue;

                const ClipVertex& clipV1 = globalTemp2[0];
                const Float invW1 = 1.0f / clipV1.pos.w;
                const Vector4D v1ndc{clipV1.pos.x * invW1, clipV1.pos.y * invW1, clipV1.pos.z * invW1, 1.0f};
                const Vector4D v1vp = viewPortMatrix * v1ndc;

                for (UInt32 t = 1; t < totalVertices - 1; t++) {
                    const ClipVertex& clipV2 = globalTemp2[t];
                    const ClipVertex& clipV3 = globalTemp2[t + 1];

                    const Float invW2 = 1.0f / clipV2.pos.w;
                    const Float invW3 = 1.0f / clipV3.pos.w;

                    const Vector4D v2ndc{clipV2.pos.x * invW2, clipV2.pos.y * invW2, clipV2.pos.z * invW2, 1.0f};
                    const Vector4D v3ndc{clipV3.pos.x * invW3, clipV3.pos.y * invW3, clipV3.pos.z * invW3, 1.0f};

                    if (isBackFace(v1ndc, v2ndc, v3ndc)) continue;

                    const Vector4D v2vp = viewPortMatrix * v2ndc;
                    const Vector4D v3vp = viewPortMatrix * v3ndc;

                    result.push_back({
                        {{static_cast<Int64>(v1vp.x), static_cast<Int64>(v1vp.y)}, 1.0f - v1ndc.z, invW1, clipV1.normal, clipV1.uv, clipV1.worldPos},
                        {{static_cast<Int64>(v2vp.x), static_cast<Int64>(v2vp.y)}, 1.0f - v2ndc.z, invW2, clipV2.normal, clipV2.uv, clipV2.worldPos},
                        {{static_cast<Int64>(v3vp.x), static_cast<Int64>(v3vp.y)}, 1.0f - v3ndc.z, invW3, clipV3.normal, clipV3.uv, clipV3.worldPos},
                        triangle.texture
                    });
                }
            }
        }

        return result;
    }
}