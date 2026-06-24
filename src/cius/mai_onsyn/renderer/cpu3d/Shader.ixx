module;
#include <cmath>
#include <algorithm>
#include <immintrin.h>
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
    Float u, v;
    Vector3D normal;
    Vector3D worldPos;
};

export struct alignas(32) _m256_Fragment {
    Float uvR[8], uvG[8], uvB[8];
    Float nX[8], nY[8], nZ[8];
    Float pX[8], pY[8], pZ[8];
};

export struct alignas(32) _m256_Result {
    Float r[8], g[8], b[8], a[8];
};

export namespace Shader {
    Color fragmentShader_Sequence(const Fragment& fragment, const Texture* texture, const Uniform* uniform) {
        FColor lightSum = {0, 0, 0};
        if (texture && texture->getKdData()) {
            FColor Kd = texture->Kd;
            FColor T_diff;   // uv颜色
            const Int32 textureW = texture->getWidth();
            const Int32 textureH = texture->getHeight();
            const Int32 tx = std::min(static_cast<Int32>(fragment.u * textureW), textureW - 1);
            const Int32 ty = std::min(static_cast<Int32>(fragment.v * textureH), textureH - 1);
            const Int32 offset = std::clamp(ty * textureW + tx, 0, textureW * textureH - 1);
            T_diff = std::bit_cast<Color>(reinterpret_cast<UInt32*>(texture->getKdData())[offset]);

            FColor C_diff = Kd * T_diff;        // 漫反射颜色

            FColor C_spec = texture->Ks;        // 镜面反射颜色

            // Float A = texture->d;               // 不透明度

            Vector3D N = fragment.normal.normalize();                           // 法线
            Vector3D V = (uniform->cameraPos - fragment.worldPos).normalize();    // 视线
            FColor I_amb = FColor{uniform->ambient} * FColor{texture->Ka} * T_diff;           // 环境光

            lightSum = I_amb;                      // 光照总和
            for (auto& light : uniform->lights) {
                Vector3D L_dir = light.pos - fragment.worldPos;     // 光源方向
                Float d_i = L_dir.length();                         // 光源距离
                if (d_i > light.range) continue;
                Vector3D L = L_dir / d_i;                           // 光源方向单位向量

                Float attn_dist = 1 / (1 + light.a * d_i + light.b * d_i * d_i);

                Float attn_dir = 1;
                if (light.type == LightType::Face)
                    attn_dir = std::max(0.0f, -light.dir.dot(L));

                FColor I_light = FColor(light.color) * (light.intensity * attn_dist * attn_dir);    // 最终光强


                Float sqrtHalfLambert = N.dot(L) * 0.5 + 0.5;
                FColor Diffuse = I_light * sqrtHalfLambert * sqrtHalfLambert;                            // 漫反射项
                Vector3D H = (V + L).normalize();                                                   // 半程向量
                FColor Specular = I_light * pow(std::max(0.0f, N.dot(H)), texture->Ns);     // 镜面反射项

                FColor color_i = C_diff * Diffuse + C_spec * Specular;

                lightSum += color_i;
            }
        }

        return static_cast<Color>(lightSum.clamp());
    }

    Color fragmentShader_NoLight(const Fragment& fragment, const Texture* texture) {
        if (texture && texture->getKdData()) {
            const Int32 textureW = texture->getWidth();
            const Int32 textureH = texture->getHeight();
            const Int32 tx = std::min(static_cast<Int32>(fragment.u * textureW), textureW - 1);
            const Int32 ty = std::min(static_cast<Int32>(fragment.v * textureH), textureH - 1);
            const Int32 offset = std::clamp(ty * textureW + tx, 0, textureW * textureH - 1);
            return std::bit_cast<Color>(reinterpret_cast<UInt32*>(texture->getKdData())[offset]);
        }
        return Color::White;
    }

    void fragmentShader_avx2(const _m256_Fragment& fragment, const Texture* texture, const Uniform* uniform, _m256_Result& result) {

    }
}