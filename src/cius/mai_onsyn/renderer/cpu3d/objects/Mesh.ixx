module;
export module Mesh;
import Types;
import Vectors;
import Vertex;
import Triangle;
import Format;

export struct Mesh {
    List<Vertex> vertices{};
    List<Triangle> triangles{};

    Mesh() = default;
    Mesh(const Mesh&) = default;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(const Mesh&) = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    String toString() const {
        return format("Mesh{%d vertices, %d triangles}", vertices.size(), triangles.size());
    }
};