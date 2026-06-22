module;
#include <sstream>
export module RenderPackage3D;
import Types;
import Matrix;
import Vectors;
import Light;
import Color;
import Vertex;
import Triangle;

export struct RenderPackage3D {
    String name;
    Matrix4x4 modelMatrix;

    // 场景持有生命周期 这里是引用指针
    const Vertex* vertices;
    UInt64 vertexCount;

    // 场景持有生命周期 这里是引用指针
    const Triangle* triangles;
    UInt64 triangleCount;

    String toString() const {
        std::stringstream ss;
        ss << "RenderPackage(name=" << name << ", vertices=" << vertexCount << ", triangles=" << triangleCount << ")";
        return ss.str();
    }
};

export struct Scene3DSnapShot {
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Vector3D cameraPos;
    Vector3D cameraDir;

    Int32 screenWidth, screenHeight;
    Float near;

    Color ambientLight;
    List<Light> lights;
    List<RenderPackage3D> renderPackages;

    [[nodiscard]] String toString() const {
        std::stringstream ss;
        ss << "SceneSnapShot{";
        ss << "screenWidth=" << screenWidth << ", ";
        ss << "screenHeight=" << screenHeight << ", ";
        ss << "viewMatrix=" << viewMatrix.toString() << ", ";
        ss << "projectionMatrix=" << projectionMatrix.toString() << ", ";
        ss << "cameraPos=" << cameraPos.toString() << ", ";
        ss << "ambientLight=" << ambientLight.toString() << ", ";
        ss << "lights=[";
        for (UInt32 i = 0; i < lights.size(); i++) {
            ss << lights[i].toString();
            if (i != lights.size() - 1) {
                ss << ", ";
            }
        }
        ss << "], renderPackages=[";
        for (UInt32 i = 0; i < renderPackages.size(); i++) {
            ss << renderPackages[i].toString();
            if (i != renderPackages.size() - 1) {
                ss << ", ";
            }
        }
        ss << "]}";
        return ss.str();
    }
};