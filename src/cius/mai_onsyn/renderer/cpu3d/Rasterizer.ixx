module;
#include <algorithm>
#include <bit>
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
import ShadowProcessor;

export namespace Rasterizer {
    void drawTriangle(const ScreenTriangle& triangle, const Tile* tile, const FrameBuffer* screenBuffer, Float* depthBuffer, const Uniform* uniform, ShadowCollection* shadowMap) {
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

                const Float depth = v1.depth * alpha_s + v2.depth * beta_s + v3.depth * gamma_s;
                if (depth < depthRow[x]) continue;

                const Float numerator1 = alpha_s * v1.invClipW;
                const Float numerator2 = beta_s * v2.invClipW;
                const Float numerator3 = gamma_s * v3.invClipW;
                const Float invSub = 1.0f / (numerator1 + numerator2 + numerator3);

                const Float w1 = numerator1 * invSub;
                const Float w2 = numerator2 * invSub;
                const Float w3 = numerator3 * invSub;

                Fragment fragment;
                fragment.u = v1.uv.x * w1 + v2.uv.x * w2 + v3.uv.x * w3;
                fragment.u = fragment.u - floor(fragment.u);
                fragment.v = v1.uv.y * w1 + v2.uv.y * w2 + v3.uv.y * w3;
                fragment.v = fragment.v - floor(fragment.v);
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

    void drawTriangleAvx2(const ScreenTriangle& triangle, const Tile* tile, const FrameBuffer* screenBuffer, Float* depthBuffer, const Uniform* uniform, ShadowCollection* shadows) {
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

        alignas(32) Float depthStackBuffer[8];
        alignas(32) Int32 uvOffsetStackBuffer[8];
        alignas(32) UInt8 alphaStackBuffer[8];

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
                // Vec8f sBPC = Vec8f::andNot(signMask, Vec8f::fms(PBx, PCy, PBy * PCx)) * half;
                // Vec8f sAPC = Vec8f::andNot(signMask, Vec8f::fms(PAx, PCy, PAy * PCx)) * half;
                Vec8f sBPC = Vec8f::andNot(signMask, PBx * PCy - PBy * PCx) * half;
                Vec8f sAPC = Vec8f::andNot(signMask, PAx * PCy - PAy * PCx) * half;

                Vec8f invABC = Vec8f(sABC_inv);
                Vec8f alphaS = sBPC * invABC;
                Vec8f betaS = sAPC * invABC;
                Vec8f gammaS = Vec8f(1.0f) - alphaS - betaS;

                // Vec8f depth8f = Vec8f::fma(alphaS, v1.depth, Vec8f::fma(betaS, v2.depth, gammaS * v3.depth));
                Vec8f depth8f = Vec8f(v1.depth) * alphaS + Vec8f(v2.depth) * betaS + Vec8f(v3.depth) * gammaS;

                // 批量深度剔除
                __m256 rowDepth = _mm256_loadu_ps(&depthRow[x]);
                __m256 cmp = _mm256_cmp_ps(depth8f, rowDepth, _CMP_GE_OQ);
                Int32 mask = _mm256_movemask_ps(cmp);
                if ((((1 << size) - 1) & mask) == 0) continue;
                depth8f.store(depthStackBuffer);

                Vec8f n1 = alphaS * Vec8f(v1.invClipW);
                Vec8f n2 = betaS * Vec8f(v2.invClipW);
                Vec8f n3 = gammaS * Vec8f(v3.invClipW);
                Vec8f invSub = (n1 + n2 + n3).inv();

                // 重心插值分量
                Vec8f w1 = n1 * invSub;
                Vec8f w2 = n2 * invSub;
                Vec8f w3 = n3 * invSub;

                // 法向量插值并归一化
                // Vec8f nx_nn = Vec8f::fma(w1, v1.normal.x, Vec8f::fma(w2, v2.normal.x, w3 * v3.normal.x));
                // Vec8f ny_nn = Vec8f::fma(w1, v1.normal.y, Vec8f::fma(w2, v2.normal.y, w3 * v3.normal.y));
                // Vec8f nz_nn = Vec8f::fma(w1, v1.normal.z, Vec8f::fma(w2, v2.normal.z, w3 * v3.normal.z));
                Vec8f nx_nn = Vec8f(v1.normal.x) * w1 + Vec8f(v2.normal.x) * w2 + Vec8f(v3.normal.x) * w3;
                Vec8f ny_nn = Vec8f(v1.normal.y) * w1 + Vec8f(v2.normal.y) * w2 + Vec8f(v3.normal.y) * w3;
                Vec8f nz_nn = Vec8f(v1.normal.z) * w1 + Vec8f(v2.normal.z) * w2 + Vec8f(v3.normal.z) * w3;
                // Vec8f lenInv = Vec8f::fma(nx_nn, nx_nn, Vec8f::fma(ny_nn, ny_nn, nz_nn * nz_nn)).invSqrt();
                Vec8f lenInv = (nx_nn * nx_nn + ny_nn * ny_nn + nz_nn * nz_nn).invSqrt();
                Vec8f nx = nx_nn * lenInv;
                Vec8f ny = ny_nn * lenInv;
                Vec8f nz = nz_nn * lenInv;
                nx.store(fragment.nX);
                ny.store(fragment.nY);
                nz.store(fragment.nZ);

                // 世界坐标插值
                // Vec8f worldX = Vec8f::fma(w1, v1.worldPos.x, Vec8f::fma(w2, v2.worldPos.x, w3 * v3.worldPos.x));
                // Vec8f worldY = Vec8f::fma(w1, v1.worldPos.y, Vec8f::fma(w2, v2.worldPos.y, w3 * v3.worldPos.y));
                // Vec8f worldZ = Vec8f::fma(w1, v1.worldPos.z, Vec8f::fma(w2, v2.worldPos.z, w3 * v3.worldPos.z));
                Vec8f worldX = Vec8f(v1.worldPos.x) * w1 + Vec8f(v2.worldPos.x) * w2 + Vec8f(v3.worldPos.x) * w3;
                Vec8f worldY = Vec8f(v1.worldPos.y) * w1 + Vec8f(v2.worldPos.y) * w2 + Vec8f(v3.worldPos.y) * w3;
                Vec8f worldZ = Vec8f(v1.worldPos.z) * w1 + Vec8f(v2.worldPos.z) * w2 + Vec8f(v3.worldPos.z) * w3;
                worldX.store(fragment.pX);
                worldY.store(fragment.pY);
                worldZ.store(fragment.pZ);

                // 批量纹理坐标转换
                if (texturePtr) {
                    Vec8f u8f = Vec8f(v1.uv.x) * w1 + Vec8f(v2.uv.x) * w2 + Vec8f(v3.uv.x) * w3;
                    Vec8f v8f = Vec8f(v1.uv.y) * w1 + Vec8f(v2.uv.y) * w2 + Vec8f(v3.uv.y) * w3;
                    u8f = u8f - u8f.floor();
                    v8f = v8f - v8f.floor();
                    Vec8i textX = static_cast<Vec8i>(u8f * Vec8f(textureWidth)).clamp(0, textureWidth - 1);
                    Vec8i textY = static_cast<Vec8i>(v8f * Vec8f(textureHeight)).clamp(0, textureHeight - 1);
                    Vec8i uvOffsets = textY * Vec8i(textureWidth) + textX;
                    uvOffsets.store(uvOffsetStackBuffer);

                    for (Int32 i = 0; i < size; i++) {
                        const auto [r, g, b, a] = std::bit_cast<Color>(texturePtr[uvOffsetStackBuffer[i]]);
                        fragment.uvR[i] = r;
                        fragment.uvG[i] = g;
                        fragment.uvB[i] = b;
                        alphaStackBuffer[i] = a;
                    }
                } else {
                    for (Int32 i = 0; i < size; i++) {
                        fragment.uvR[i] = 255;
                        fragment.uvG[i] = 255;
                        fragment.uvB[i] = 255;
                        alphaStackBuffer[i] = 255;
                    }
                }

                // 并行光照计算
                auto [r, g, b, a] = Shader::fragmentShader_avx2(fragment, triangle.texture, uniform, shadows, size);
                // auto [r, g, b, a] = Shader::fragmentShader_avx2_NoLight(fragment);

                // 填充像素
                for (Int32 i = 0; i < size; i++) {
                    const Float depth = depthStackBuffer[i];
                    if (const Int32 col = x + i; depth >= depthRow[col] && alphaStackBuffer[i] > 0) {
                        const Int32 pixelIndex = col << 2;
                        depthRow[col] = depth;
                        screenRow[pixelIndex] = r[i];
                        screenRow[pixelIndex + 1] = g[i];
                        screenRow[pixelIndex + 2] = b[i];
                        screenRow[pixelIndex + 3] = alphaStackBuffer[i];
                    }
                }
            }
        }
    }
}