module;
export module ScreenVertex;
import Vectors;
import Types;

export struct ScreenVertex {
    VectorInt2D pos;
    Float depth;
    Float invClipW; // 裁剪空间的w的倒数 用于透视矫正插值
    Vector3D normal;
    Vector2D uv;
    Vector3D worldPos;

    ScreenVertex() = default;
    ScreenVertex(const VectorInt2D& pos, const Float depth, const Float invClipW, const Vector3D& normal, const Vector2D& uv) :
        pos(pos), depth(depth), invClipW(invClipW), normal(normal), uv(uv) {}
};