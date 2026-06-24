module;
#include <algorithm>
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

            alignas(32) Float floatBuffer[16];
            alignas(32) Int32 intBuffer[16];

            for (Int32 x = xs; x < xe; x += 8) {
                __m256 sub_x = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
                __m256 sub_y = _mm256_set1_ps(y);
                __m256 PA_x = _mm256_sub_ps(_mm256_set1_ps(v1.pos.x), sub_x);
                __m256 PA_y = _mm256_sub_ps(_mm256_set1_ps(v1.pos.y), sub_y);
                __m256 PB_x = _mm256_sub_ps(_mm256_set1_ps(v2.pos.x), sub_x);
                __m256 PB_y = _mm256_sub_ps(_mm256_set1_ps(v2.pos.y), sub_y);
                __m256 PC_x = _mm256_sub_ps(_mm256_set1_ps(v3.pos.x), sub_x);
                __m256 PC_y = _mm256_sub_ps(_mm256_set1_ps(v3.pos.y), sub_y);

                __m256 sign_mask = _mm256_set1_ps(-0.0f);
                __m256 divide_inv = _mm256_set1_ps(0.5f);
                __m256 sBPC = _mm256_mul_ps(    // 除2
                    _mm256_andnot_ps(           // 绝对值
                        sign_mask,
                        T
                        ),
                    divide_inv
                    );
            }
            
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
}