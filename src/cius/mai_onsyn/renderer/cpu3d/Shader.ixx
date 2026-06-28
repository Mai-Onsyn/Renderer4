module;
#include <cmath>
#include <algorithm>
#include <bit>
#include <immintrin.h>
#include <map>
#include <string>
// #include <iostream>
export module Shader;
import Types;
import ScreenVertex;
import Color;
import Vectors;
import Light;
import Texture;
import SIMD;
import ShadowProcessor;

export struct Uniform {
    const List<Light>& lights;
    Vector3D cameraPos;
    Vector3D cameraDir;
    FColor ambient;

    Uniform(const List<Light>& lights) : lights(lights) {}
};

export struct Fragment {
    Float u, v;
    Vector3D normal;
    Vector3D worldPos;
};

export struct alignas(32) _m256_Fragment {
    UInt8 uvR[8], uvG[8], uvB[8];
    Float nX[8], nY[8], nZ[8];
    Float pX[8], pY[8], pZ[8];
};

export struct alignas(32) _m256_Result {
    UInt8 r[8], g[8], b[8], a[8];
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

                FColor I_light = light.color * (light.intensity * attn_dist * attn_dir);    // 最终光强


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


    /**
     * 法向量已归一化
     * 输入已采样的UV UInt8颜色
     */
    _m256_Result fragmentShader_avx2(const _m256_Fragment& fragment, const Texture* texture, const Uniform* uniform, ShadowCollection* shadows, const Int32 count) {
        _m256_Result result;
        FColor Kd = texture ? FColor(texture->Kd) : FColor(0.8f, 0.8f, 0.8f);
        FColor Ka = texture ? FColor(texture->Ka) : FColor(0.8f, 0.8f, 0.8f);
        FColor Ks = texture ? FColor(texture->Ks) : FColor(0.5f, 0.5f, 0.5f);
        Float Ns = texture ? texture->Ns : 1;

        alignas(32) Float stackVx[8];
        alignas(32) Float stackVy[8];
        alignas(32) Float stackVz[8];

        alignas(32) Float stackC_diff_r[8];
        alignas(32) Float stackC_diff_g[8];
        alignas(32) Float stackC_diff_b[8];

        alignas(32) Float stackI_amb_r[8];
        alignas(32) Float stackI_amb_g[8];
        alignas(32) Float stackI_amb_b[8];

        alignas(32) Float stackSum_r[8];
        alignas(32) Float stackSum_g[8];
        alignas(32) Float stackSum_b[8];

        alignas(32) Int32 stackShadowOffsets[8]{};
        alignas(32) Float stackMapDepths[8]{};

        Vec8f inv255 = Vec8f(0.0039215686274509803921568627451f);   // 1/255
        Vec8f T_diff_r = Vec8f(
            fragment.uvR[0], fragment.uvR[1], fragment.uvR[2], fragment.uvR[3],
            fragment.uvR[4], fragment.uvR[5], fragment.uvR[6], fragment.uvR[7]) * inv255;
        Vec8f T_diff_g = Vec8f(
            fragment.uvG[0], fragment.uvG[1], fragment.uvG[2], fragment.uvG[3],
            fragment.uvG[4], fragment.uvG[5], fragment.uvG[6], fragment.uvG[7]) * inv255;
        Vec8f T_diff_b = Vec8f(
            fragment.uvB[0], fragment.uvB[1], fragment.uvB[2], fragment.uvB[3],
            fragment.uvB[4], fragment.uvB[5], fragment.uvB[6], fragment.uvB[7]) * inv255;

        (Vec8f(Kd.r) * T_diff_r).store(stackC_diff_r);
        (Vec8f(Kd.g) * T_diff_g).store(stackC_diff_g);
        (Vec8f(Kd.b) * T_diff_b).store(stackC_diff_b);

        Vec8f I_amb_r = T_diff_r * (uniform->ambient.r * Ka.r);
        Vec8f I_amb_g = T_diff_g * (uniform->ambient.g * Ka.g);
        Vec8f I_amb_b = T_diff_b * (uniform->ambient.b * Ka.b);
        I_amb_r.store(stackI_amb_r);
        I_amb_g.store(stackI_amb_g);
        I_amb_b.store(stackI_amb_b);
        I_amb_r.store(stackSum_r);
        I_amb_g.store(stackSum_g);
        I_amb_b.store(stackSum_b);

        Vec8f vx_nn = Vec8f(uniform->cameraPos.x) - Vec8f(fragment.pX);
        Vec8f vy_nn = Vec8f(uniform->cameraPos.y) - Vec8f(fragment.pY);
        Vec8f vz_nn = Vec8f(uniform->cameraPos.z) - Vec8f(fragment.pZ);
        Vec8f vLenInv = Vec8f::fma(vx_nn, vx_nn, Vec8f::fma(vy_nn, vy_nn, vz_nn * vz_nn)).invSqrt();
        // Vec8f vLenInv = Vec8f(1.0f) / (vx_nn * vx_nn + vy_nn * vy_nn + vz_nn * vz_nn).sqrt();
        (vx_nn * vLenInv).store(stackVx);
        (vy_nn * vLenInv).store(stackVy);
        (vz_nn * vLenInv).store(stackVz);

        for (const auto& light : uniform->lights) {
            Vec8f L_dir_x = Vec8f(light.pos.x) - Vec8f(fragment.pX);
            Vec8f L_dir_y = Vec8f(light.pos.y) - Vec8f(fragment.pY);
            Vec8f L_dir_z = Vec8f(light.pos.z) - Vec8f(fragment.pZ);
            Vec8f distance = Vec8f::fma(L_dir_x, L_dir_x, Vec8f::fma(L_dir_y, L_dir_y, L_dir_z * L_dir_z)).sqrt();
            // Vec8f distance = (L_dir_x * L_dir_x + L_dir_y * L_dir_y + L_dir_z * L_dir_z).sqrt();
            // if (distance > light.range) continue;

            Vec8f invDistance = Vec8f(1) / distance;
            Vec8f L_x = L_dir_x * invDistance;
            Vec8f L_y = L_dir_y * invDistance;
            Vec8f L_z = L_dir_z * invDistance;

            Vec8f attn_dist = Vec8f::fma(distance, light.a, distance * distance * light.b + 1).inv();
            // Vec8f attn_dist = Vec8f(1) / (Vec8f(1) + distance * light.a + distance * distance * light.b);
            Vec8f attn_dir = Vec8f(1);
            if (light.type == LightType::Face) {
                Vec8f zero(0.0f);
                attn_dir = Vec8f::max(zero, zero - Vec8f::dot3D(light.dir.x, light.dir.y, light.dir.z, L_x, L_y, L_z));
            }

            Vec8f light_decay = attn_dist * attn_dir * light.intensity;
            Vec8f I_light_r = light_decay * light.color.r;
            Vec8f I_light_g = light_decay * light.color.g;
            Vec8f I_light_b = light_decay * light.color.b;

            Vec8f nDotL = Vec8f::dot3D(L_x, L_y, L_z, fragment.nX, fragment.nY, fragment.nZ);
            if (nDotL < 0.0f) {
                continue;
            }

            Vec8f sqrtHalfLambert = nDotL * Vec8f(0.5f) + Vec8f(0.5f);
            Vec8f Diffuse_r = I_light_r * sqrtHalfLambert * sqrtHalfLambert;
            Vec8f Diffuse_g = I_light_g * sqrtHalfLambert * sqrtHalfLambert;
            Vec8f Diffuse_b = I_light_b * sqrtHalfLambert * sqrtHalfLambert;

            Vec8f H_x_nn = L_x + stackVx;
            Vec8f H_y_nn = L_y + stackVy;
            Vec8f H_z_nn = L_z + stackVz;
            Vec8f hLenInv = Vec8f::fma(H_x_nn, H_x_nn, Vec8f::fma(H_y_nn, H_y_nn, H_z_nn * H_z_nn)).invSqrt();
            Vec8f H_x = H_x_nn * hLenInv;
            Vec8f H_y = H_y_nn * hLenInv;
            Vec8f H_z = H_z_nn * hLenInv;

            Vec8f pow_max_0_N_dot_H = _mm256_pow_ps_approx(
                Vec8f::max(Vec8f(0.0f), Vec8f::dot3D(
                    fragment.nX, fragment.nY, fragment.nZ,
                    H_x, H_y, H_z)
                    ), _mm256_set1_ps(Ns));
            Vec8f Specular_r = I_light_r * pow_max_0_N_dot_H;
            Vec8f Specular_g = I_light_g * pow_max_0_N_dot_H;
            Vec8f Specular_b = I_light_b * pow_max_0_N_dot_H;

            if (shadows->contains(light.name)) {
                // 阴影
                const auto& [map, size, matrix] = (*shadows)[light.name];
                // 世界空间
                Vec8f sx = fragment.pX;
                Vec8f sy = fragment.pY;
                Vec8f sz = fragment.pZ;
                Vec8f w = 1.0f;
                Vec8f::matrixMul(matrix, sx, sy, sz, w);
                // 裁剪空间
                w = w.inv();
                sx *= w;
                sy *= w;
                sz *= w;
                sz = Vec8f(1) - sz; // 深度
                // ndc空间
                sx = (sx * 0.5f + 0.5f).clamp(0, 1);  // [0, 1]的shadow map uv
                sy = Vec8f(1) - (sy * 0.5f + 0.5f).clamp(0, 1);  // [0, 1]的shadow map uv
                // shadow map uv映射
                Vec8i x = static_cast<Vec8i>(sx * size).clamp(0, size - 1);
                Vec8i y = static_cast<Vec8i>(sy * size).clamp(0, size - 1);
                Vec8i shadowOffsets = y * size + x;
                shadowOffsets.store(stackShadowOffsets);
                for (UInt32 i = 0; i < count; i++) {
                    const auto& shadowOffset = stackShadowOffsets[i];
                    stackMapDepths[i] = map.get()[shadowOffset];
                }
                // Float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.00005);
                Vec8f shadowDistances = Vec8f(stackMapDepths) * Vec8f(0.9f);
                Vec8f cmp = shadowDistances.cmpBigger(sz);
                Vec8f shadowMask = _mm256_blendv_ps(Vec8f(1), Vec8f(0.2), cmp);

                (Vec8f::fma(stackC_diff_r, Diffuse_r, Vec8f::fma(Specular_r, Ks.r, stackSum_r)) * shadowMask).store(stackSum_r);
                (Vec8f::fma(stackC_diff_g, Diffuse_g, Vec8f::fma(Specular_g, Ks.g, stackSum_g)) * shadowMask).store(stackSum_g);
                (Vec8f::fma(stackC_diff_b, Diffuse_b, Vec8f::fma(Specular_b, Ks.b, stackSum_b)) * shadowMask).store(stackSum_b);
            } else {
                Vec8f::fma(stackC_diff_r, Diffuse_r, Vec8f::fma(Specular_r, Ks.r, stackSum_r)).store(stackSum_r);
                Vec8f::fma(stackC_diff_g, Diffuse_g, Vec8f::fma(Specular_g, Ks.g, stackSum_g)).store(stackSum_g);
                Vec8f::fma(stackC_diff_b, Diffuse_b, Vec8f::fma(Specular_b, Ks.b, stackSum_b)).store(stackSum_b);
            }
        }

        storeColorVec(static_cast<Vec8i>(Vec8f(stackSum_r).clamp(0, 1) * 255.0f), result.r);
        storeColorVec(static_cast<Vec8i>(Vec8f(stackSum_g).clamp(0, 1) * 255.0f), result.g);
        storeColorVec(static_cast<Vec8i>(Vec8f(stackSum_b).clamp(0, 1) * 255.0f), result.b);

        return result;
    }

    _m256_Result fragmentShader_avx2_NoLight(const _m256_Fragment& fragment) {
        _m256_Result result;
        memcpy(&result, &fragment, 32);
        return result;
    }
}



        

        // 常量 1/255
        // __m256 inv255 = _mm256_set1_ps(1.0f / 255.0f);
        //
        // // 从 fragment 的 RGB 通道加载并归一化（注意顺序：低位在前）
        // __m256 T_diff_r = _mm256_mul_ps(
        //     _mm256_set_ps(fragment.uvR[7], fragment.uvR[6], fragment.uvR[5], fragment.uvR[4],
        //                   fragment.uvR[3], fragment.uvR[2], fragment.uvR[1], fragment.uvR[0]),
        //     inv255
        // );
        // __m256 T_diff_g = _mm256_mul_ps(
        //     _mm256_set_ps(fragment.uvG[7], fragment.uvG[6], fragment.uvG[5], fragment.uvG[4],
        //                   fragment.uvG[3], fragment.uvG[2], fragment.uvG[1], fragment.uvG[0]),
        //     inv255
        // );
        // __m256 T_diff_b = _mm256_mul_ps(
        //     _mm256_set_ps(fragment.uvB[7], fragment.uvB[6], fragment.uvB[5], fragment.uvB[4],
        //                   fragment.uvB[3], fragment.uvB[2], fragment.uvB[1], fragment.uvB[0]),
        //     inv255
        // );
        //
        // // 存储 Kd * 纹理颜色
        // _mm256_storeu_ps(stackC_diff_r, _mm256_mul_ps(_mm256_set1_ps(Kd.r), T_diff_r));
        // _mm256_storeu_ps(stackC_diff_g, _mm256_mul_ps(_mm256_set1_ps(Kd.g), T_diff_g));
        // _mm256_storeu_ps(stackC_diff_b, _mm256_mul_ps(_mm256_set1_ps(Kd.b), T_diff_b));
        //
        // // 环境光贡献
        // __m256 ambient_r = _mm256_mul_ps(_mm256_set1_ps(ambient.r * Ka.r), T_diff_r);
        // __m256 ambient_g = _mm256_mul_ps(_mm256_set1_ps(ambient.g * Ka.g), T_diff_g);
        // __m256 ambient_b = _mm256_mul_ps(_mm256_set1_ps(ambient.b * Ka.b), T_diff_b);
        // _mm256_storeu_ps(stackI_amb_r, ambient_r);
        // _mm256_storeu_ps(stackI_amb_g, ambient_g);
        // _mm256_storeu_ps(stackI_amb_b, ambient_b);
        // _mm256_storeu_ps(stackSum_r, ambient_r);
        // _mm256_storeu_ps(stackSum_g, ambient_g);
        // _mm256_storeu_ps(stackSum_b, ambient_b);
        //
        // // 计算视线方向（从片段指向相机）
        // __m256 vx_nn = _mm256_sub_ps(_mm256_set1_ps(uniform->cameraPos.x), _mm256_loadu_ps(fragment.pX));
        // __m256 vy_nn = _mm256_sub_ps(_mm256_set1_ps(uniform->cameraPos.y), _mm256_loadu_ps(fragment.pY));
        // __m256 vz_nn = _mm256_sub_ps(_mm256_set1_ps(uniform->cameraPos.z), _mm256_loadu_ps(fragment.pZ));
        // __m256 lenSqV = _mm256_add_ps(
        //     _mm256_add_ps(_mm256_mul_ps(vx_nn, vx_nn), _mm256_mul_ps(vy_nn, vy_nn)),
        //     _mm256_mul_ps(vz_nn, vz_nn)
        // );
        // __m256 lenInvV = _mm256_div_ps(_mm256_set1_ps(1.0f), _mm256_sqrt_ps(lenSqV));
        // __m256 Vx = _mm256_mul_ps(vx_nn, lenInvV);
        // __m256 Vy = _mm256_mul_ps(vy_nn, lenInvV);
        // __m256 Vz = _mm256_mul_ps(vz_nn, lenInvV);
        // _mm256_storeu_ps(stackVx, Vx);
        // _mm256_storeu_ps(stackVy, Vy);
        // _mm256_storeu_ps(stackVz, Vz);
        //
        // // 遍历光源
        // for (const auto& light : uniform->lights) {
        //     // 光源方向向量
        //     __m256 L_dir_x = _mm256_sub_ps(_mm256_set1_ps(light.pos.x), _mm256_loadu_ps(fragment.pX));
        //     __m256 L_dir_y = _mm256_sub_ps(_mm256_set1_ps(light.pos.y), _mm256_loadu_ps(fragment.pY));
        //     __m256 L_dir_z = _mm256_sub_ps(_mm256_set1_ps(light.pos.z), _mm256_loadu_ps(fragment.pZ));
        //     __m256 distSq = _mm256_add_ps(
        //         _mm256_add_ps(_mm256_mul_ps(L_dir_x, L_dir_x), _mm256_mul_ps(L_dir_y, L_dir_y)),
        //         _mm256_mul_ps(L_dir_z, L_dir_z)
        //     );
        //     __m256 distance = _mm256_sqrt_ps(distSq);
        //     // if (distance > light.range) continue;  // 标量条件保留，这里不展开
        //
        //     __m256 invDistance = _mm256_div_ps(_mm256_set1_ps(1.0f), distance);
        //     __m256 L_x = _mm256_mul_ps(L_dir_x, invDistance);
        //     __m256 L_y = _mm256_mul_ps(L_dir_y, invDistance);
        //     __m256 L_z = _mm256_mul_ps(L_dir_z, invDistance);
        //
        //     // 距离衰减
        //     __m256 attn_dist = _mm256_div_ps(
        //         _mm256_set1_ps(1.0f),
        //         _mm256_add_ps(
        //             _mm256_add_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(distance, _mm256_set1_ps(light.a))),
        //             _mm256_mul_ps(_mm256_mul_ps(distance, distance), _mm256_set1_ps(light.b))
        //         )
        //     );
        //
        //     // 方向衰减（聚光灯）
        //     __m256 attn_dir;
        //     if (light.type == LightType::Face) {
        //         // dot3D(light.dir, L)
        //         __m256 dot = _mm256_add_ps(
        //             _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(light.dir.x), L_x),
        //                           _mm256_mul_ps(_mm256_set1_ps(light.dir.y), L_y)),
        //             _mm256_mul_ps(_mm256_set1_ps(light.dir.z), L_z)
        //         );
        //         attn_dir = _mm256_max_ps(_mm256_set1_ps(0.0f), _mm256_sub_ps(_mm256_set1_ps(0.0f), dot)); // max(0, -dot)
        //     } else {
        //         attn_dir = _mm256_set1_ps(1.0f);
        //     }
        //
        //     __m256 light_decay = _mm256_mul_ps(_mm256_mul_ps(attn_dist, attn_dir), _mm256_set1_ps(light.intensity));
        //     __m256 I_light_r = _mm256_mul_ps(light_decay, _mm256_set1_ps(light.color.r));
        //     __m256 I_light_g = _mm256_mul_ps(light_decay, _mm256_set1_ps(light.color.g));
        //     __m256 I_light_b = _mm256_mul_ps(light_decay, _mm256_set1_ps(light.color.b));
        //
        //     // 半兰伯特（sqrtHalfLambert = 0.5 * dot(N,L) + 0.5）
        //     __m256 nDotL = _mm256_add_ps(
        //         _mm256_add_ps(_mm256_mul_ps(L_x, _mm256_loadu_ps(fragment.nX)),
        //                       _mm256_mul_ps(L_y, _mm256_loadu_ps(fragment.nY))),
        //         _mm256_mul_ps(L_z, _mm256_loadu_ps(fragment.nZ))
        //     );
        //     __m256 sqrtHalfLambert = _mm256_add_ps(_mm256_mul_ps(nDotL, _mm256_set1_ps(0.5f)), _mm256_set1_ps(0.5f));
        //     __m256 sqHalfLambert = _mm256_mul_ps(sqrtHalfLambert, sqrtHalfLambert); // 平方
        //
        //     __m256 Diffuse_r = _mm256_mul_ps(I_light_r, sqHalfLambert);
        //     __m256 Diffuse_g = _mm256_mul_ps(I_light_g, sqHalfLambert);
        //     __m256 Diffuse_b = _mm256_mul_ps(I_light_b, sqHalfLambert);
        //
        //     // 高光半向量 H = normalize(L + V)
        //     __m256 H_x_nn = _mm256_add_ps(L_x, Vx);
        //     __m256 H_y_nn = _mm256_add_ps(L_y, Vy);
        //     __m256 H_z_nn = _mm256_add_ps(L_z, Vz);
        //     __m256 lenSqH = _mm256_add_ps(
        //         _mm256_add_ps(_mm256_mul_ps(H_x_nn, H_x_nn), _mm256_mul_ps(H_y_nn, H_y_nn)),
        //         _mm256_mul_ps(H_z_nn, H_z_nn)
        //     );
        //     __m256 lenInvH = _mm256_div_ps(_mm256_set1_ps(1.0f), _mm256_sqrt_ps(lenSqH));
        //     __m256 H_x = _mm256_mul_ps(H_x_nn, lenInvH);
        //     __m256 H_y = _mm256_mul_ps(H_y_nn, lenInvH);
        //     __m256 H_z = _mm256_mul_ps(H_z_nn, lenInvH);
        //
        //     // 计算 N·H，并取 max(0, dot) 然后进行幂运算
        //     __m256 nDotH = _mm256_add_ps(
        //         _mm256_add_ps(_mm256_mul_ps(_mm256_loadu_ps(fragment.nX), H_x),
        //                       _mm256_mul_ps(_mm256_loadu_ps(fragment.nY), H_y)),
        //         _mm256_mul_ps(_mm256_loadu_ps(fragment.nZ), H_z)
        //     );
        //     __m256 maxDot = _mm256_max_ps(_mm256_set1_ps(0.0f), nDotH);
        //     __m256 powVal = _mm256_pow_ps_approx(maxDot, _mm256_set1_ps(Ns));
        //
        //     __m256 Specular_r = _mm256_mul_ps(I_light_r, powVal);
        //     __m256 Specular_g = _mm256_mul_ps(I_light_g, powVal);
        //     __m256 Specular_b = _mm256_mul_ps(I_light_b, powVal);
        //
        //     // 累加光照结果
        //     __m256 diff_r = _mm256_loadu_ps(stackC_diff_r);
        //     __m256 diff_g = _mm256_loadu_ps(stackC_diff_g);
        //     __m256 diff_b = _mm256_loadu_ps(stackC_diff_b);
        //
        //     __m256 newSum_r = _mm256_add_ps(
        //         _mm256_add_ps(_mm256_mul_ps(diff_r, Diffuse_r),
        //                       _mm256_mul_ps(Specular_r, _mm256_set1_ps(Ks.r))),
        //         _mm256_loadu_ps(stackSum_r)
        //     );
        //     __m256 newSum_g = _mm256_add_ps(
        //         _mm256_add_ps(_mm256_mul_ps(diff_g, Diffuse_g),
        //                       _mm256_mul_ps(Specular_g, _mm256_set1_ps(Ks.g))),
        //         _mm256_loadu_ps(stackSum_g)
        //     );
        //     __m256 newSum_b = _mm256_add_ps(
        //         _mm256_add_ps(_mm256_mul_ps(diff_b, Diffuse_b),
        //                       _mm256_mul_ps(Specular_b, _mm256_set1_ps(Ks.b))),
        //         _mm256_loadu_ps(stackSum_b)
        //     );
        //
        //     _mm256_storeu_ps(stackSum_r, newSum_r);
        //     _mm256_storeu_ps(stackSum_g, newSum_g);
        //     _mm256_storeu_ps(stackSum_b, newSum_b);
        // }
        //
        // // 最终将颜色钳制到 [0,1]，乘以255并转为整型
        // __m256 sum_r = _mm256_loadu_ps(stackSum_r);
        // __m256 sum_g = _mm256_loadu_ps(stackSum_g);
        // __m256 sum_b = _mm256_loadu_ps(stackSum_b);
        //
        // // 强制使用 min/max 双向死死卡住范围
        // __m256 zero = _mm256_setzero_ps();
        // __m256 one  = _mm256_set1_ps(1.0f);
        //
        // __m256 clamped_r = _mm256_min_ps(_mm256_max_ps(sum_r, zero), one);
        // __m256 clamped_g = _mm256_min_ps(_mm256_max_ps(sum_g, zero), one);
        // __m256 clamped_b = _mm256_min_ps(_mm256_max_ps(sum_b, zero), one);
        //
        // // 推荐使用 _mm256_cvtps_epi32 代替 _mm256_cvttps_epi32，这能提供更平滑的色彩四舍五入
        // __m256i color_r = _mm256_cvtps_epi32(_mm256_mul_ps(clamped_r, _mm256_set1_ps(255.0f)));
        // __m256i color_g = _mm256_cvtps_epi32(_mm256_mul_ps(clamped_g, _mm256_set1_ps(255.0f)));
        // __m256i color_b = _mm256_cvtps_epi32(_mm256_mul_ps(clamped_b, _mm256_set1_ps(255.0f)));
        //
        // storeColorVec(color_r, result.r);
        // storeColorVec(color_g, result.g);
        // storeColorVec(color_b, result.b);