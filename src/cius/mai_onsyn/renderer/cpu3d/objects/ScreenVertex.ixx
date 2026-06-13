module;
export module ScreenVertex;
import Vectors;
import Types;

export struct ScreenVertex {
    VectorInt2D pos;
    Float depth;
    Vector3D normal;

    ScreenVertex() = default;
    ScreenVertex(const VectorInt2D& pos, const Float depth, const Vector3D& normal) : pos(pos), depth(depth), normal(normal) {}
};