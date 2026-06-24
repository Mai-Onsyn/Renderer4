module;
#include <cmath>
#include <algorithm>
export module Shader;
import Types;
import ScreenVertex;
import Color;
import Vectors;
import Light;
import Texture;

export struct Uniform {
    const List<Light>& lights;
    Vector3D cameraPos;
    Vector3D cameraDir;
    Color ambient;

    Uniform(const List<Light>& lights) : lights(lights) {}
};

export struct Fragment {
    Int64 x, y;
    Float u, v;
    Float depth;
    Vector3D normal;
    Vector3D worldPos;
};

export namespace Shader {

    Color fragmentShader(const Fragment& fragment, const Texture* texture, const Uniform* uniform) {
        Color Kd;   // uv颜色
        if (texture && texture->getKdData()) {
            const Int32 textureW = texture->getWidth();
            const Int32 textureH = texture->getHeight();
            const Int32 tx = std::min(static_cast<Int32>(fragment.u * textureW), textureW - 1);
            const Int32 ty = std::min(static_cast<Int32>(fragment.v * textureH), textureH - 1);
            const Int32 offset = std::clamp(ty * textureW + tx, 0, textureW * textureH - 1);
            Kd = std::bit_cast<Color>(reinterpret_cast<UInt32*>(texture->getKdData())[offset]);
        }
        else Kd = Color::White;

        Color Ks = texture->Ks;
        Float Ns = texture->Ns;
        Color ambientColor = uniform->ambient * Kd;

        for (const Light& light : uniform->lights) {
            switch (light.type) {
                Vector3D Di;
                Float di;
                Vector3D Li;
                case LightType::Point: {
                    Di = light.pos - fragment.worldPos;
                    di = Di.length();
                    Li = Di.normalize();
                }
                case LightType::Directional: {
                    Li = light.dir;
                }
                Float att = 1 / (1 + (di / light.range) * (di / light.range));
                const Float NdotL = std::max(0.0f, Li.dot(fragment.normal));
                default: throw RuntimeError("Unknown light type: " + static_cast<Int32>(light.type));
            }
        }

        return Kd;
    }
}