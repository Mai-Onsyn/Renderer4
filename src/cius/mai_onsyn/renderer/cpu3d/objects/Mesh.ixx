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

    String toString() const {
        return format("Mesh{%d vertices, %d triangles}", vertices.size(), triangles.size());
    }
};