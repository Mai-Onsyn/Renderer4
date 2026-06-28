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
        // mesh.vertices.emplace_back(Vertex{{pos.x - 1, pos.y + 1, pos.z}, {0, 0, -1}, {0, 0}});
        // mesh.vertices.emplace_back(Vertex{{pos.x - 1, pos.y - 1, pos.z}, {0, 0, -1}, {0, 1}});
        // mesh.vertices.emplace_back(Vertex{{pos.x - 1, pos.y - 1, pos.z}, {0, 0, -1}, {1, 1}});

        // +Z
        Vertex pzV1 = {{pos.x - half, pos.y + half, pos.z + half}, {0, 0, 1}, {0, 0}};
        Vertex pzV2 = {{pos.x - half, pos.y - half, pos.z + half}, {0, 0, 1}, {0, 1}};
        Vertex pzV3 = {{pos.x + half, pos.y - half, pos.z + half}, {0, 0, 1}, {1, 1}};
        Vertex pzV4 = {{pos.x + half, pos.y + half, pos.z + half}, {0, 0, 1}, {1, 0}};

        // -Z
        Vertex nzV1 = {{pos.x - half, pos.y + half, pos.z - half}, {0, 0, -1}, {0, 0}};
        Vertex nzV2 = {{pos.x - half, pos.y - half, pos.z - half}, {0, 0, -1}, {0, 1}};
        Vertex nzV3 = {{pos.x + half, pos.y - half, pos.z - half}, {0, 0, -1}, {1, 1}};
        Vertex nzV4 = {{pos.x + half, pos.y + half, pos.z - half}, {0, 0, -1}, {1, 0}};

        // +Y
        Vertex pyV1 = {{pos.x - half, pos.y + half, pos.z + half}, {0, 1, 0}, {0, 0}};
        Vertex pyV2 = {{pos.x + half, pos.y + half, pos.z + half}, {0, 1, 0}, {0, 1}};
        Vertex pyV3 = {{pos.x + half, pos.y + half, pos.z - half}, {0, 1, 0}, {1, 1}};
        Vertex pyV4 = {{pos.x - half, pos.y + half, pos.z - half}, {0, 1, 0}, {1, 0}};

        // -Y
        Vertex nyV1 = {{pos.x - half, pos.y - half, pos.z + half}, {0, -1, 0}, {0, 0}};
        Vertex nyV2 = {{pos.x - half, pos.y - half, pos.z - half}, {0, -1, 0}, {0, 1}};
        Vertex nyV3 = {{pos.x + half, pos.y - half, pos.z - half}, {0, -1, 0}, {1, 1}};
        Vertex nyV4 = {{pos.x + half, pos.y - half, pos.z + half}, {0, -1, 0}, {1, 0}};

        // -X
        Vertex nxV1 = {{pos.x - half, pos.y + half, pos.z + half}, {-1, 0, 0}, {0, 0}};
        Vertex nxV2 = {{pos.x - half, pos.y + half, pos.z - half}, {-1, 0, 0}, {0, 1}};
        Vertex nxV3 = {{pos.x - half, pos.y - half, pos.z - half}, {-1, 0, 0}, {1, 1}};
        Vertex nxV4 = {{pos.x - half, pos.y - half, pos.z + half}, {-1, 0, 0}, {1, 0}};

        // +X
        Vertex pxV1 = {{pos.x + half, pos.y + half, pos.z + half}, {1, 0, 0}, {0, 0}};
        Vertex pxV2 = {{pos.x + half, pos.y - half, pos.z + half}, {1, 0, 0}, {0, 1}};
        Vertex pxV3 = {{pos.x + half, pos.y - half, pos.z - half}, {1, 0, 0}, {1, 1}};
        Vertex pxV4 = {{pos.x + half, pos.y + half, pos.z - half}, {1, 0, 0}, {1, 0}};

        mesh.vertices.push_back(pzV1);
        mesh.vertices.push_back(pzV2);
        mesh.vertices.push_back(pzV3);
        mesh.vertices.push_back(pzV4);
        mesh.vertices.push_back(nzV1);
        mesh.vertices.push_back(nzV2);
        mesh.vertices.push_back(nzV3);
        mesh.vertices.push_back(nzV4);
        mesh.vertices.push_back(pyV1);
        mesh.vertices.push_back(pyV2);
        mesh.vertices.push_back(pyV3);
        mesh.vertices.push_back(pyV4);
        mesh.vertices.push_back(nyV1);
        mesh.vertices.push_back(nyV2);
        mesh.vertices.push_back(nyV3);
        mesh.vertices.push_back(nyV4);
        mesh.vertices.push_back(nxV1);
        mesh.vertices.push_back(nxV2);
        mesh.vertices.push_back(nxV3);
        mesh.vertices.push_back(nxV4);
        mesh.vertices.push_back(pxV1);
        mesh.vertices.push_back(pxV2);
        mesh.vertices.push_back(pxV3);
        mesh.vertices.push_back(pxV4);

        auto texture = make_unique<Texture>(1, 1);
        UInt8* textureBuffer = texture.get()->map_Kd.getBuffer();
        textureBuffer[0] = color.r;
        textureBuffer[1] = color.g;
        textureBuffer[2] = color.b;
        textureBuffer[3] = color.a;

        // +Z
        mesh.triangles.emplace_back(0, 1, 2, texture.get());
        mesh.triangles.emplace_back(0, 2, 3, texture.get());

        // -Z
        mesh.triangles.emplace_back(4, 6, 5, texture.get());
        mesh.triangles.emplace_back(4, 7, 6, texture.get());

        // +Y
        mesh.triangles.emplace_back(8, 9, 10, texture.get());
        mesh.triangles.emplace_back(8, 10, 11, texture.get());

        // -Y
        mesh.triangles.emplace_back(12, 13, 14, texture.get());
        mesh.triangles.emplace_back(12, 14, 15, texture.get());

        // -X
        mesh.triangles.emplace_back(16, 17, 18, texture.get());
        mesh.triangles.emplace_back(16, 18, 19, texture.get());

        // +X
        mesh.triangles.emplace_back(20, 21, 22, texture.get());
        mesh.triangles.emplace_back(20, 22, 23, texture.get());

        mesh.texture.push_back(move(texture));
        return mesh;
    }
}