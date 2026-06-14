module;
export module ScreenVertex;
import Vectors;
import Types;

export struct ScreenVertex {
    VectorInt2D pos;
    Float depth;
    Float invClipW; // 裁剪空间的w的倒数 用于透视矫正插值
    Vector3D normal;

    ScreenVertex() = default;
    ScreenVertex(const VectorInt2D& pos, const Float depth, const Float invClipW, const Vector3D& normal):
        pos(pos), depth(depth), invClipW(invClipW), normal(normal) {}
};