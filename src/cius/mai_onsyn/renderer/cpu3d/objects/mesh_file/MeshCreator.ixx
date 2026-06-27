module;
#include <memory>
export module MeshCreator;
import Types;
import Mesh;
import Vectors;
import Matrix;
import Transform;
import Triangle;
import Color;
import Vertex;
import Texture;

export namespace MeshCreator {
    Mesh createBox(const Float len, const Vector3D& pos, const Color& color) {
        Mesh mesh;
        const Float half = len / 2.0f;
        mesh.vertices.emplace_back(Vertex{{pos.x - 1, pos.y + 1, pos.z}, {0, 0, -1}, {0, 0}});
        mesh.vertices.emplace_back(Vertex{{pos.x - 1, pos.y - 1, pos.z}, {0, 0, -1}, {0, 1}});
        mesh.vertices.emplace_back(Vertex{{pos.x - 1, pos.y - 1, pos.z}, {0, 0, -1}, {1, 1}});

        auto texture = make_unique<Texture>(1, 1);
        UInt8* textureBuffer = texture.get()->map_Kd.getBuffer();
        textureBuffer[0] = color.r;
        textureBuffer[1] = color.g;
        textureBuffer[2] = color.b;
        textureBuffer[3] = color.a;
        mesh.triangles.emplace_back(0, 2, 1, texture.get());
        mesh.texture.push_back(move(texture));
        return mesh;
    }
}