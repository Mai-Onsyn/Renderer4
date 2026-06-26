module;
#include <vector>
export module Mesh;
import Types;
import Vectors;
import Vertex;
import Triangle;
import Format;
import Texture;
import Logger;

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

    Boolean check() const {
        Int32 vertexCount = vertices.size();
        Set<Texture*> textureSet;
        for (auto& t : texture) {
            textureSet.emplace(t.get());
        }

        Boolean isMeshLlegal = true;
        for (const auto& triangle : triangles) {
            if (triangle.v1 >= vertexCount) {
                Log::error("Triangle vertex index out of range: %d >= %d", triangle.v1, vertexCount);
                isMeshLlegal = false;
            }
            if (triangle.v2 >= vertexCount) {
                Log::error("Triangle vertex index out of range: %d >= %d", triangle.v2, vertexCount);
                isMeshLlegal = false;
            }
            if (triangle.v3 >= vertexCount) {
                Log::error("Triangle vertex index out of range: %d >= %d", triangle.v3, vertexCount);
                isMeshLlegal = false;
            }

            Texture* triangleTexturePtr = triangle.texture;
            if (triangleTexturePtr) {
                if (!textureSet.contains(triangleTexturePtr)) {
                    Log::error("Triangle texture pointer is not nullptr but not in texture set");
                    isMeshLlegal = false;
                }
            }
        }
        return isMeshLlegal;
    }
};