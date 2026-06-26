module;
export module Vertex;
import Types;
import Vectors;

export struct Vertex {
    Vector3D pos;
    Vector3D normal;
    Vector2D uv;

    Boolean operator==(const Vertex& other) const {
        return pos == other.pos && normal == other.normal && uv == other.uv;
    }
};