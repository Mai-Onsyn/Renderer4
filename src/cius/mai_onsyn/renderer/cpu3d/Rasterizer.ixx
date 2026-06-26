module;
#include <algorithm>
#include <cstring>
#include <immintrin.h>
export module Rasterizer;
import Types;
import Triangle;
import Vertex;
import ScreenVertex;
import Tile;
import Vectors;
import FrameBuffer;
import Shader;
import SIMD;
import Color;

export namespace Rasterizer {
    void drawTriangle(const ScreenTriangle& triangle, const Tile* tile, const FrameBuffer* screenBuffer, Float* depthBuffer, const Uniform* uniform) {
        // 复制顶点
        ScreenVertex v1 = triangle.v1;
        ScreenVertex v2 = triangle.v2;
        ScreenVertex v3 = triangle.v3;

        // 排序顶点
        if (v1.pos.y > v2.pos.y) std::swap(v1, v2);
        if (v2.pos.y > v3.pos.y) std::swap(v2, v3);
        if (v1.pos.y > v2.pos.y) std::swap(v1, v2);

        const Float sABC_inv = 2.0f / abs((v3.pos.x - v1.pos.x) * (v2.pos.y - v1.pos.y) - (v3.pos.y - v1.pos.y) * (v2.pos.x - v1.pos.x));

        const Int64 ys = std::clamp(v1.pos.y, static_cast<Int64>(tile->y), static_cast<Int64>(tile->y + tile->height));
        const Int64 ye = std::clamp(v3.pos.y, static_cast<Int64>(tile->y), static_cast<Int64>(tile->y + tile->height));

        const Boolean unhorizontal_AB = v1.pos.y != v2.pos.y;
        const Boolean unhorizontal_BC = v2.pos.y != v3.pos.y;

        // 扫描线填充
        for (Int64 y = ys; y < ye; y++) {
            Int64 xa, xb = v1.pos.x + (v3.pos.x - v1.pos.x) * (y - v1.pos.y) / (v3.pos.y - v1.pos.y);

            if (y < v2.pos.y && unhorizontal_AB) {
                xa = v1.pos.x + (v2.pos.x - v1.pos.x) * (y - v1.pos.y) / (v2.pos.y - v1.pos.y);
            } else if (unhorizontal_BC) {
                xa = v2.pos.x + (v3.pos.x - v2.pos.x) * (y - v2.pos.y) / (v3.pos.y - v2.pos.y);
            }
            else continue;

            const Int64 xs = std::clamp(std::min(xa, xb), static_cast<Int64>(tile->x), static_cast<Int64>(tile->x + tile->width));
            const Int64 xe = std::clamp(std::max(xa, xb), static_cast<Int64>(tile->x), static_cast<Int64>(tile->x + tile->width));

            UInt8* screenRow = screenBuffer->getBuffer() + y * screenBuffer->width * 4;
            Float* depthRow = depthBuffer + y * screenBuffer->width;

            for (Int64 x = xs; x < xe; x++) {
                // 重心插值
                const VectorInt2D PA = {v1.pos.x - x, v1.pos.y - y};
                const VectorInt2D PB = {v2.pos.x - x, v2.pos.y - y};
                const VectorInt2D PC = {v3.pos.x - x, v3.pos.y - y};
                const Int64 sBPC = abs(PB.x * PC.y - PB.y * PC.x) >> 1;
                const Int64 sAPC = abs(PA.x * PC.y - PA.y * PC.x) >> 1;

                const Float alpha_s = static_cast<Float>(sBPC) * sABC_inv;
                const Float beta_s = static_cast<Float>(sAPC) * sABC_inv;
                const Float gamma_s = 1.0f - alpha_s - beta_s;

                const Float numerator1 = alpha_s * v1.invClipW;
                const Float numerator2 = beta_s * v2.invClipW;
                const Float numerator3 = gamma_s * v3.invClipW;
                const Float invSub = 1.0f / (numerator1 + numerator2 + numerator3);

                const Float w1 = numerator1 * invSub;
                const Float w2 = numerator2 * invSub;
                const Float w3 = numerator3 * invSub;

                const Float depth = v1.depth * w1 + v2.depth * w2 + v3.depth * w3;

                if (depth >= depthRow[x]) {
                    Fragment fragment;
                    fragment.u = v1.uv.x * w1 + v2.uv.x * w2 + v3.uv.x * w3;
                    fragment.v = v1.uv.y * w1 + v2.uv.y * w2 + v3.uv.y * w3;
                    fragment.normal = v1.normal * w1 + v2.normal * w2 + v3.normal * w3;
                    fragment.worldPos = v1.worldPos * w1 + v2.worldPos * w2 + v3.worldPos * w3;
                    const auto [r, g, b, a] = Shader::fragmentShader_Sequence(fragment, triangle.texture, uniform);
                    // const auto [r, g, b, a] = Shader::fragmentShader_NoLight(fragment, triangle.texture);

                    const UInt32 pixelIndex = x << 2;
                    depthRow[x] = depth;
                    screenRow[pixelIndex] = r;
                    screenRow[pixelIndex + 1] = g;
                    screenRow[pixelIndex + 2] = b;
                    screenRow[pixelIndex + 3] = a;
                }
            }
        }
    }

    void drawTriangleAvx2(const ScreenTriangle& triangle, const Tile* tile, const FrameBuffer* screenBuffer, Float* depthBuffer, const Uniform* uniform) {
        // 复制顶点
        ScreenVertex v1 = triangle.v1;
        ScreenVertex v2 = triangle.v2;
        ScreenVertex v3 = triangle.v3;

        // 排序顶点
        if (v1.pos.y > v2.pos.y) std::swap(v1, v2);
        if (v2.pos.y > v3.pos.y) std::swap(v2, v3);
        if (v1.pos.y > v2.pos.y) std::swap(v1, v2);

        const Float sABC_inv = 2.0f / std::abs((v3.pos.x - v1.pos.x) * (v2.pos.y - v1.pos.y) - (v3.pos.y - v1.pos.y) * (v2.pos.x - v1.pos.x));

        const Int32 ys = std::clamp(static_cast<Int32>(v1.pos.y), static_cast<Int32>(tile->y), tile->y + tile->height);
        const Int32 ye = std::clamp(static_cast<Int32>(v3.pos.y), static_cast<Int32>(tile->y), tile->y + tile->height);

        const Boolean unhorizontal_AB = v1.pos.y != v2.pos.y;
        const Boolean unhorizontal_BC = v2.pos.y != v3.pos.y;

        Int32 textureWidth = 0;
        Int32 textureHeight = 0;
        UInt32* texturePtr = nullptr;
        if (triangle.texture && triangle.texture->getKdData()) {
            textureWidth = triangle.texture->map_Kd.width;
            textureHeight = triangle.texture->map_Kd.height;
            texturePtr = reinterpret_cast<UInt32*>(triangle.texture->getKdData());
        }
        // Vec8f textMaxW = Vec8f(static_cast<Float>(textureWidth - 1));
        // Vec8f textMaxH = Vec8f(static_cast<Float>(textureHeight - 1));
        // Vec8f zeroVec = Vec8f(0.0f);
        // __m256i textMaxW = _mm256_set1_epi32(textureWidth - 1);
        // __m256i textMaxH = _mm256_set1_epi32(textureHeight - 1);

        alignas(32) Float depthStackBuffer[8];
        alignas(32) Int32 uvOffsetStackBuffer[8];

        // 扫描线填充
        for (Int32 y = ys; y < ye; y++) {
            Int32 xa, xb = v1.pos.x + (v3.pos.x - v1.pos.x) * (y - v1.pos.y) / (v3.pos.y - v1.pos.y);

            if (y < v2.pos.y && unhorizontal_AB) {
                xa = v1.pos.x + (v2.pos.x - v1.pos.x) * (y - v1.pos.y) / (v2.pos.y - v1.pos.y);
            } else if (unhorizontal_BC) {
                xa = v2.pos.x + (v3.pos.x - v2.pos.x) * (y - v2.pos.y) / (v3.pos.y - v2.pos.y);
            }
            else continue;

            const Int32 xs = std::clamp(std::min(xa, xb), static_cast<Int32>(tile->x), tile->x + tile->width);
            const Int32 xe = std::clamp(std::max(xa, xb), static_cast<Int32>(tile->x), tile->x + tile->width);

            UInt8* screenRow = screenBuffer->getBuffer() + y * screenBuffer->width * 4;
            Float* depthRow = depthBuffer + y * screenBuffer->width;

            for (Int32 x = xs; x < xe; x += 8) {
                Int32 size = std::min(xe - x, 8);
                _m256_Fragment fragment{};

                Vec8f subX = Vec8f(x) + Vec8f(0, 1, 2, 3, 4, 5, 6, 7);
                Vec8f subY = Vec8f(y);

                Vec8f PAx = Vec8f(v1.pos.x) - subX;
                Vec8f PAy = Vec8f(v1.pos.y) - subY;
                Vec8f PBx = Vec8f(v2.pos.x) - subX;
                Vec8f PBy = Vec8f(v2.pos.y) - subY;
                Vec8f PCx = Vec8f(v3.pos.x) - subX;
                Vec8f PCy = Vec8f(v3.pos.y) - subY;

                Vec8f signMask = Vec8f(-0.0f);
                Vec8f half = Vec8f(0.5f);
                Vec8f sBPC = Vec8f::andNot(signMask, PBx * PCy - PBy * PCx) * half;
                Vec8f sAPC = Vec8f::andNot(signMask, PAx * PCy - PAy * PCx) * half;

                Vec8f invABC = Vec8f(sABC_inv);
                Vec8f alphaS = sBPC * invABC;
                Vec8f betaS = sAPC * invABC;
                Vec8f gammaS = Vec8f(1.0f) - (alphaS + betaS);

                Vec8f depth8f = Vec8f(v1.depth) * alphaS + Vec8f(v2.depth) * betaS + Vec8f(v3.depth) * gammaS;

                // 批量深度剔除
                __m256 rowDepth = _mm256_loadu_ps(&depthRow[x]);
                __m256 cmp = _mm256_cmp_ps(depth8f, rowDepth, _CMP_GT_OQ);
                Int32 mask = _mm256_movemask_ps(cmp);
                if ((((size == 8) ? 0xFF : ((1 << size) - 1)) & mask) == 0) continue;
                depth8f.store(depthStackBuffer);

                Vec8f n1 = alphaS * Vec8f(v1.invClipW);
                Vec8f n2 = betaS * Vec8f(v2.invClipW);
                Vec8f n3 = gammaS * Vec8f(v3.invClipW);
                Vec8f invSub = Vec8f(1.0f) / (n1 + n2 + n3);

                // 重心插值分量
                Vec8f w1 = n1 * invSub;
                Vec8f w2 = n2 * invSub;
                Vec8f w3 = n3 * invSub;

                // 批量纹理坐标转换
                if (texturePtr) {
                    Vec8f u8f = Vec8f(v1.uv.x) * w1 + Vec8f(v2.uv.x) * w2 + Vec8f(v3.uv.x) * w3;
                    Vec8f v8f = Vec8f(v1.uv.y) * w1 + Vec8f(v2.uv.y) * w2 + Vec8f(v3.uv.y) * w3;
                    Vec8i textX = static_cast<Vec8i>(u8f * Vec8f(textureWidth)).clamp(0, textureWidth - 1);
                    Vec8i textY = static_cast<Vec8i>(v8f * Vec8f(textureHeight)).clamp(0, textureHeight - 1);
                    Vec8i uvOffsets = textY * Vec8i(textureWidth) + textX;
                    uvOffsets.store(uvOffsetStackBuffer);
                }

                // 法向量插值并归一化
                Vec8f nx_nn  = Vec8f(v1.normal.x) * w1 + Vec8f(v2.normal.x) * w2 + Vec8f(v3.normal.x) * w3;
                Vec8f ny_nn = Vec8f(v1.normal.y) * w1 + Vec8f(v2.normal.y) * w2 + Vec8f(v3.normal.y) * w3;
                Vec8f nz_nn = Vec8f(v1.normal.z) * w1 + Vec8f(v2.normal.z) * w2 + Vec8f(v3.normal.z) * w3;
                Vec8f lenInv = Vec8f(1.0f) / (nx_nn  * nx_nn  + ny_nn * ny_nn + nz_nn * nz_nn).sqrt();
                Vec8f nx = nx_nn * lenInv;
                Vec8f ny = ny_nn * lenInv;
                Vec8f nz = nz_nn * lenInv;
                nx.store(fragment.nX);
                ny.store(fragment.nY);
                nz.store(fragment.nZ);

                // 世界坐标插值
                Vec8f worldX = Vec8f(v1.worldPos.x) * w1 + Vec8f(v2.worldPos.x) * w2 + Vec8f(v3.worldPos.x) * w3;
                Vec8f worldY = Vec8f(v1.worldPos.y) * w1 + Vec8f(v2.worldPos.y) * w2 + Vec8f(v3.worldPos.y) * w3;
                Vec8f worldZ = Vec8f(v1.worldPos.z) * w1 + Vec8f(v2.worldPos.z) * w2 + Vec8f(v3.worldPos.z) * w3;
                worldX.store(fragment.pX);
                worldY.store(fragment.pY);
                worldZ.store(fragment.pZ);

                // 填充纹理颜色
                for (Int32 i = 0; i < size; i++) {
                    if (texturePtr) {
                        const auto [r, g, b, a] = std::bit_cast<Color>(texturePtr[uvOffsetStackBuffer[i]]);
                        fragment.uvR[i] = r;
                        fragment.uvG[i] = g;
                        fragment.uvB[i] = b;
                    } else {
                        fragment.uvR[i] = 255;
                        fragment.uvG[i] = 255;
                        fragment.uvB[i] = 255;
                    }
                }

                // 并行光照计算
                auto [r, g, b, a] = Shader::fragmentShader_avx2(fragment, triangle.texture, uniform, size);
                // auto [r, g, b, a] = Shader::fragmentShader_avx2_NoLight(fragment);

                // 填充像素
                for (Int32 i = 0; i < size; i++) {
                    const Float depth = depthStackBuffer[i];
                    if (const Int32 col = x + i; depth >= depthRow[col]) {
                        const Int32 pixelIndex = col << 2;
                        depthRow[col] = depth;
                        screenRow[pixelIndex] = r[i];
                        screenRow[pixelIndex + 1] = g[i];
                        screenRow[pixelIndex + 2] = b[i];
                        screenRow[pixelIndex + 3] = a[i];
                    }
                }
            }
        }
    }
}



                // 1. 生成子像素坐标偏移
                // __m256 subX = _mm256_add_ps(_mm256_set1_ps(x), _mm256_set_ps(7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f));
                // __m256 subY = _mm256_set1_ps(y);
                //
                // // 2. 计算各顶点相对当前像素的向量
                // __m256 PAx = _mm256_sub_ps(_mm256_set1_ps(v1.pos.x), subX);
                // __m256 PAy = _mm256_sub_ps(_mm256_set1_ps(v1.pos.y), subY);
                // __m256 PBx = _mm256_sub_ps(_mm256_set1_ps(v2.pos.x), subX);
                // __m256 PBy = _mm256_sub_ps(_mm256_set1_ps(v2.pos.y), subY);
                // __m256 PCx = _mm256_sub_ps(_mm256_set1_ps(v3.pos.x), subX);
                // __m256 PCy = _mm256_sub_ps(_mm256_set1_ps(v3.pos.y), subY);
                //
                // // 3. 计算面积（带符号），并取绝对值的一半
                // __m256 signMask = _mm256_set1_ps(-0.0f);
                // __m256 half = _mm256_set1_ps(0.5f);
                //
                // __m256 crossPB = _mm256_sub_ps(_mm256_mul_ps(PBx, PCy), _mm256_mul_ps(PBy, PCx));
                // __m256 sBPC = _mm256_mul_ps(_mm256_andnot_ps(signMask, crossPB), half);
                //
                // __m256 crossPA = _mm256_sub_ps(_mm256_mul_ps(PAx, PCy), _mm256_mul_ps(PAy, PCx));
                // __m256 sAPC = _mm256_mul_ps(_mm256_andnot_ps(signMask, crossPA), half);
                //
                // // 4. 重心坐标（未校正透视）
                // __m256 invABC = _mm256_set1_ps(sABC_inv);
                // __m256 alphaS = _mm256_mul_ps(sBPC, invABC);
                // __m256 betaS  = _mm256_mul_ps(sAPC, invABC);
                // __m256 gammaS = _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_add_ps(alphaS, betaS));
                //
                // // 5. 透视校正插值因子
                // __m256 n1 = _mm256_mul_ps(alphaS, _mm256_set1_ps(v1.invClipW));
                // __m256 n2 = _mm256_mul_ps(betaS,  _mm256_set1_ps(v2.invClipW));
                // __m256 n3 = _mm256_mul_ps(gammaS, _mm256_set1_ps(v3.invClipW));
                // __m256 sumN = _mm256_add_ps(_mm256_add_ps(n1, n2), n3);
                // __m256 invSub = _mm256_div_ps(_mm256_set1_ps(1.0f), sumN);
                //
                // // 6. 最终权重
                // __m256 w1 = _mm256_mul_ps(n1, invSub);
                // __m256 w2 = _mm256_mul_ps(n2, invSub);
                // __m256 w3 = _mm256_mul_ps(n3, invSub);
                //
                // // 7. 插值深度
                // __m256 depth8f = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.depth), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.depth), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.depth), w3)
                // );
                //
                // // 8. 深度剔除（批量）
                // __m256 rowDepth = _mm256_loadu_ps(&depthRow[x]);
                // __m256 cmp = _mm256_cmp_ps(depth8f, rowDepth, _CMP_GT_OQ);
                // int mask = _mm256_movemask_ps(cmp);
                // if ((((size == 8) ? 0xFF : ((1 << size) - 1)) & mask) == 0) continue;
                // _mm256_storeu_ps(depthStackBuffer, depth8f);
                //
                // // 9. 纹理坐标插值并转为整型（带钳位）
                // __m256 u8f = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.uv.x), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.uv.x), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.uv.x), w3)
                // );
                // __m256 v8f = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.uv.y), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.uv.y), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.uv.y), w3)
                // );
                //
                // __m256 uScaled = _mm256_mul_ps(u8f, _mm256_set1_ps((float)textureWidth));
                // __m256 vScaled = _mm256_mul_ps(v8f, _mm256_set1_ps((float)textureHeight));
                //
                // __m256i textX = _mm256_min_epi32(
                //     _mm256_max_epi32(_mm256_cvttps_epi32(uScaled), _mm256_set1_epi32(0)),
                //     textMaxW
                // );
                // __m256i textY = _mm256_min_epi32(
                //     _mm256_max_epi32(_mm256_cvttps_epi32(vScaled), _mm256_set1_epi32(0)),
                //     textMaxH
                // );
                //
                // // 10. 计算纹理偏移并存储
                // __m256i uvOffsets = _mm256_add_epi32(
                //     _mm256_mullo_epi32(textY, _mm256_set1_epi32(textureWidth)),
                //     textX
                // );
                // _mm256_store_si256((__m256i*)uvOffsetStackBuffer, uvOffsets);  // 若未对齐请改用 _mm256_storeu_si256
                //
                // // 11. 法向量插值并归一化
                // __m256 nx_nn = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.normal.x), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.normal.x), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.normal.x), w3)
                // );
                // __m256 ny_nn = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.normal.y), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.normal.y), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.normal.y), w3)
                // );
                // __m256 nz_nn = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.normal.z), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.normal.z), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.normal.z), w3)
                // );
                //
                // __m256 lenSq = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(nx_nn, nx_nn),
                //                   _mm256_mul_ps(ny_nn, ny_nn)),
                //     _mm256_mul_ps(nz_nn, nz_nn)
                // );
                // __m256 lenInv = _mm256_div_ps(_mm256_set1_ps(1.0f), _mm256_sqrt_ps(lenSq));
                //
                // __m256 nx = _mm256_mul_ps(nx_nn, lenInv);
                // __m256 ny = _mm256_mul_ps(ny_nn, lenInv);
                // __m256 nz = _mm256_mul_ps(nz_nn, lenInv);
                //
                // _mm256_storeu_ps(fragment.nX, nx);
                // _mm256_storeu_ps(fragment.nY, ny);
                // _mm256_storeu_ps(fragment.nZ, nz);
                //
                // // 12. 世界坐标插值
                // __m256 worldX = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.worldPos.x), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.worldPos.x), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.worldPos.x), w3)
                // );
                // __m256 worldY = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.worldPos.y), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.worldPos.y), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.worldPos.y), w3)
                // );
                // __m256 worldZ = _mm256_add_ps(
                //     _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(v1.worldPos.z), w1),
                //                   _mm256_mul_ps(_mm256_set1_ps(v2.worldPos.z), w2)),
                //     _mm256_mul_ps(_mm256_set1_ps(v3.worldPos.z), w3)
                // );
                //
                // _mm256_storeu_ps(fragment.pX, worldX);
                // _mm256_storeu_ps(fragment.pY, worldY);
                // _mm256_storeu_ps(fragment.pZ, worldZ);