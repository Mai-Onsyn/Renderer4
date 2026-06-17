module;
export module Mesh;
import Types;
import Vectors;
import Vertex;
import Triangle;
import Format;
import Texture;

export struct Mesh {
    List<Vertex> vertices{};
    List<Triangle> triangles{};
    List<UniquePtr<Texture>> texture{};

    Mesh() = default;
    Mesh(const Mesh&) = default;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(const Mesh&) = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    String toString() const {
        return format("Mesh{%d vertices, %d triangles}", vertices.size(), triangles.size());
    }
};