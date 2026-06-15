module;
#include <vector>
#include <string>
export module VertexProcessor;
import Types;
import Matrix;
import Triangle;
import ScreenVertex;
import Vertex;
import Vectors;
import RenderPackage;
import Logger;

struct ClipVertex {
    Vector4D pos;
    Vector3D normal;

    String toString() const {
        return "ClipVertex(pos=" + pos.toString() + ", normal=" + normal.toString() + ")";
    }
};

export namespace VertexProcessor {
    ClipVertex interpolate(const ClipVertex& v1, const ClipVertex& v2, const Float t) {
        return {
            v1.pos * (1 - t) + v2.pos * t,
            v1.normal * (1 - t) + v2.normal * t
        };
    }

    List<ClipVertex> clip(const ClipVertex& v1, const ClipVertex& v2, const ClipVertex& v3) {
        List<ClipVertex> temp1;
        List<ClipVertex> temp2;
        temp1.reserve(6);
        temp2.reserve(6);
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

        return temp2;
    }

    ScreenTriangle toScreenTriangle(const ClipVertex& v1, const ClipVertex& v2, const ClipVertex& v3, const Matrix4x4& viewPortMatrix) {
        const Vector4D v1ndc{v1.pos.x / v1.pos.w, v1.pos.y / v1.pos.w, v1.pos.z / v1.pos.w, 1.0f};
        const Vector4D v2ndc{v2.pos.x / v2.pos.w, v2.pos.y / v2.pos.w, v2.pos.z / v2.pos.w, 1.0f};
        const Vector4D v3ndc{v3.pos.x / v3.pos.w, v3.pos.y / v3.pos.w, v3.pos.z / v3.pos.w, 1.0f};

        const Vector4D v1vp = viewPortMatrix * v1ndc;
        const Vector4D v2vp = viewPortMatrix * v2ndc;
        const Vector4D v3vp = viewPortMatrix * v3ndc;

        return {
            {{static_cast<Int64>(v1vp.x), static_cast<Int64>(v1vp.y)}, 1.0f - v1ndc.z, v1.pos.w,  v1.normal},
            {{static_cast<Int64>(v2vp.x), static_cast<Int64>(v2vp.y)}, 1.0f - v2ndc.z, v2.pos.w,  v2.normal},
            {{static_cast<Int64>(v3vp.x), static_cast<Int64>(v3vp.y)}, 1.0f - v3ndc.z, v3.pos.w,  v3.normal}
        };
    }

    List<ScreenTriangle> process(const Scene3DSnapShot* sceneSnapShot) {
        List<ScreenTriangle> result;

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
            for (UInt32 i = 0; i < pkg.vertexCount; i++) {
                clipSpaceVertex[i].pos = mvp * vertexes[i].pos;
                clipSpaceVertex[i].normal = normalMatrix * vertexes[i].normal;
            }

            for (UInt32 i = 0; i < pkg.triangleCount; i++) {
                const Triangle& triangle = triangles[i];
                const ClipVertex& v1 = clipSpaceVertex[triangle.v1];
                const ClipVertex& v2 = clipSpaceVertex[triangle.v2];
                const ClipVertex& v3 = clipSpaceVertex[triangle.v3];

                const Boolean satisfyV1z = v1.pos.z > 0;
                const Boolean satisfyV2z = v2.pos.z > 0;
                const Boolean satisfyV3z = v3.pos.z > 0;
                // 全部在近平面后方
                if (!satisfyV1z && !satisfyV2z && !satisfyV3z) continue;

                const Boolean satisfyV1xLeft = v1.pos.x > -v1.pos.w;
                const Boolean satisfyV2xLeft = v2.pos.x > -v2.pos.w;
                const Boolean satisfyV3xLeft = v3.pos.x > -v3.pos.w;
                // 全部在x-外
                if (!satisfyV1xLeft && !satisfyV2xLeft && !satisfyV3xLeft) continue;

                const Boolean satisfyV1xRight = v1.pos.x < v1.pos.w;
                const Boolean satisfyV2xRight = v2.pos.x < v2.pos.w;
                const Boolean satisfyV3xRight = v3.pos.x < v3.pos.w;
                // 全部在x+外
                if (!satisfyV1xRight && !satisfyV2xRight && !satisfyV3xRight) continue;

                const Boolean satisfyV1yBottom = v1.pos.y > -v1.pos.w;
                const Boolean satisfyV2yBottom = v2.pos.y > -v2.pos.w;
                const Boolean satisfyV3yBottom = v3.pos.y > -v3.pos.w;
                // 全部在y-外
                if (!satisfyV1yBottom && !satisfyV2yBottom && !satisfyV3yBottom) continue;

                const Boolean satisfyV1yTop = v1.pos.y < v1.pos.w;
                const Boolean satisfyV2yTop = v2.pos.y < v2.pos.w;
                const Boolean satisfyV3yTop = v3.pos.y < v3.pos.w;
                // 全部在y+外
                if (!satisfyV1yTop && !satisfyV2yTop && !satisfyV3yTop) continue;

                // 全部在视锥体内
                if (
                    satisfyV1z && satisfyV2z && satisfyV3z &&
                    satisfyV1xLeft && satisfyV2xLeft && satisfyV3xLeft &&
                    satisfyV1xRight && satisfyV2xRight && satisfyV3xRight &&
                    satisfyV1yBottom && satisfyV2yBottom && satisfyV3yBottom &&
                    satisfyV1yTop && satisfyV2yTop && satisfyV3yTop
                    ) {
                    result.push_back(toScreenTriangle(v1, v2, v3, viewPortMatrix));
                }
                // 需要裁剪
                else {
                    // 后续切分三角形操作
                    switch (const List<ClipVertex> clippedVertices = clip(v1, v2, v3); clippedVertices.size()) {
                        case 0: break;
                        case 3: {
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[1], clippedVertices[2], viewPortMatrix));
                            break;
                        }
                        case 4: {
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[1], clippedVertices[2], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[2], clippedVertices[3], viewPortMatrix));
                            break;
                        }
                        case 5: {
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[1], clippedVertices[2], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[2], clippedVertices[3], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[3], clippedVertices[4], viewPortMatrix));
                            break;
                        }
                        case 6: {
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[1], clippedVertices[2], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[2], clippedVertices[3], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[3], clippedVertices[4], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[4], clippedVertices[5], viewPortMatrix));
                            break;
                        }
                        case 7: {
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[1], clippedVertices[2], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[2], clippedVertices[3], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[3], clippedVertices[4], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[4], clippedVertices[5], viewPortMatrix));
                            result.push_back(toScreenTriangle(clippedVertices[0], clippedVertices[5], clippedVertices[6], viewPortMatrix));
                            break;
                        }
                        default: break; // 极端情况 给了
                        // default: throw RuntimeError("Invalid number of clipped vertices: " + std::to_string(clippedVertices.size()));
                    }
                }
            }
        }

        return result;
    }
}