module;
#include <cstring>
#include <sstream>
#include <immintrin.h>
export module Matrix;
import Types;
import Vectors;

export class Matrix3x3 {
public:
    Float m[9]{};
    Matrix3x3() = default;

    Matrix3x3(const Float n0, const Float n1, const Float n2,
              const Float n3, const Float n4, const Float n5,
              const Float n6, const Float n7, const Float n8) {
        m[0] = n0;
        m[1] = n1;
        m[2] = n2;
        m[3] = n3;
        m[4] = n4;
        m[5] = n5;
        m[6] = n6;
        m[7] = n7;
        m[8] = n8;
    }

    Float& operator[](const Int32 idx) {
        return m[idx];
    }

    const Float& operator[](const Int32 idx) const {
        return m[idx];
    }

    Matrix3x3 inverse() const {
        const Float det = calcDet();
        if (det == 0.0f) {
            throw RuntimeError("Matrix3x3::inverse() - determinant is 0");
        }
        const Float invDet = 1.0f / det;

        return Matrix3x3{
            (m[4] * m[8] - m[5] * m[7]) * invDet,
            (m[2] * m[7] - m[1] * m[8]) * invDet,
            (m[1] * m[5] - m[2] * m[4]) * invDet,

            (m[5] * m[6] - m[3] * m[8]) * invDet,
            (m[0] * m[8] - m[2] * m[6]) * invDet,
            (m[2] * m[3] - m[0] * m[5]) * invDet,

            (m[3] * m[7] - m[4] * m[6]) * invDet,
            (m[1] * m[6] - m[0] * m[7]) * invDet,
            (m[0] * m[4] - m[1] * m[3]) * invDet
        };
    }

    Matrix3x3 transpose() const {
        return Matrix3x3{
            m[0], m[3], m[6],
            m[1], m[4], m[7],
            m[2], m[5], m[8]
        };
    }

    Matrix3x3 operator*(const Matrix3x3 &other) const {
        return Matrix3x3{
            m[0] * other[0] + m[1] * other[3] + m[2] * other[6],
            m[0] * other[1] + m[1] * other[4] + m[2] * other[7],
            m[0] * other[2] + m[1] * other[5] + m[2] * other[8],
            m[3] * other[0] + m[4] * other[3] + m[5] * other[6],
            m[3] * other[1] + m[4] * other[4] + m[5] * other[7],
            m[3] * other[2] + m[4] * other[5] + m[5] * other[8],
            m[6] * other[0] + m[7] * other[3] + m[8] * other[6],
            m[6] * other[1] + m[7] * other[4] + m[8] * other[7],
            m[6] * other[2] + m[7] * other[5] + m[8] * other[8]
        };
    }

    Vector3D operator*(const Vector3D &other) const {
        return Vector3D{
            m[0] * other.x + m[1] * other.y + m[2] * other.z,
            m[3] * other.x + m[4] * other.y + m[5] * other.z,
            m[6] * other.x + m[7] * other.y + m[8] * other.z
        };
    }

    [[nodiscard]] Float calcDet() const {
        return m[0] * (m[4] * m[8] - m[5] * m[7]) - m[1] * (m[3] * m[8] - m[5] * m[6]) + m[2] * (m[3] * m[7] - m[4] * m[6]);
    }
};

export class alignas(32) Matrix4x4 {
    alignas(32) Float m[16]{};
public:
    Matrix4x4() {
        memset(m, 0, sizeof(m));
    };

    Matrix4x4(Matrix4x4&& other) noexcept {
        memcpy(this->m, other.m, sizeof(m));
    }
    Matrix4x4& operator=(Matrix4x4&& other) noexcept {
        memcpy(this->m, other.m, sizeof(m));
        return *this;
    }

    Matrix4x4& operator=(const Matrix4x4& other) {
        memcpy(this->m, other.m, sizeof(m));
        return *this;
    }

    static Matrix4x4 I() {
        Matrix4x4 matrix4;
        for (int i = 0; i < 4; i++) {
            matrix4.m[i * 4 + i] = 1;
        }
        return matrix4;
    }

    explicit operator Matrix3x3() const {
        return {m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10]};
    }

    Float& operator[](const Int32 idx) {
        return m[idx];
    }

    const Float& operator[](const Int32 idx) const {
        return m[idx];
    }

    Matrix4x4 operator*(const Matrix4x4 &other) const {
        Matrix4x4 result;
        result.m[0] = m[0] * other[0] + m[1] * other[4] + m[2] * other[8] + m[3] * other[12];
        result.m[1] = m[0] * other[1] + m[1] * other[5] + m[2] * other[9] + m[3] * other[13];
        result.m[2] = m[0] * other[2] + m[1] * other[6] + m[2] * other[10] + m[3] * other[14];
        result.m[3] = m[0] * other[3] + m[1] * other[7] + m[2] * other[11] + m[3] * other[15];

        result.m[4] = m[4] * other[0] + m[5] * other[4] + m[6] * other[8] + m[7] * other[12];
        result.m[5] = m[4] * other[1] + m[5] * other[5] + m[6] * other[9] + m[7] * other[13];
        result.m[6] = m[4] * other[2] + m[5] * other[6] + m[6] * other[10] + m[7] * other[14];
        result.m[7] = m[4] * other[3] + m[5] * other[7] + m[6] * other[11] + m[7] * other[15];

        result.m[8] = m[8] * other[0] + m[9] * other[4] + m[10] * other[8] + m[11] * other[12];
        result.m[9] = m[8] * other[1] + m[9] * other[5] + m[10] * other[9] + m[11] * other[13];
        result.m[10] = m[8] * other[2] + m[9] * other[6] + m[10] * other[10] + m[11] * other[14];
        result.m[11] = m[8] * other[3] + m[9] * other[7] + m[10] * other[11] + m[11] * other[15];

        result.m[12] = m[12] * other[0] + m[13] * other[4] + m[14] * other[8] + m[15] * other[12];
        result.m[13] = m[12] * other[1] + m[13] * other[5] + m[14] * other[9] + m[15] * other[13];
        result.m[14] = m[12] * other[2] + m[13] * other[6] + m[14] * other[10] + m[15] * other[14];
        result.m[15] = m[12] * other[3] + m[13] * other[7] + m[14] * other[11] + m[15] * other[15];
        return result;
    }

    Vector4D operator*(const Vector4D &other) const {
        // return {
        //     m[0] * other.x + m[1] * other.y + m[2] * other.z + m[3] * other.w,
        //     m[4] * other.x + m[5] * other.y + m[6] * other.z + m[7] * other.w,
        //     m[8] * other.x + m[9] * other.y + m[10] * other.z + m[11] * other.w,
        //     m[12] * other.x + m[13] * other.y + m[14] * other.z + m[15] * other.w
        // };
        // 1. 广播 vector 的各个分量到 SSE 寄存器 (x,x,x,x), (y,y,y,y)...
        __m128 vx = _mm_set1_ps(other.x);
        __m128 vy = _mm_set1_ps(other.y);
        __m128 vz = _mm_set1_ps(other.z);
        __m128 vw = _mm_set1_ps(other.w);

        // 2. 加载矩阵的 4 列 (每列 4 个 float)
        // 注意：这里的加载假设矩阵在内存中是按列存储的。
        // 如果是按行存储（即 m[0],m[1],m[2],m[3] 是第一行），则需要加载行，对应关系微调
        // 依据你原版代码：m[0]*x + m[1]*y + m[2]*z + m[3]*w 是第一行，说明是行优先存储（Row-Major）

        // 针对行优先存储，我们需要直接加载 4 个行向量
        __m128 row0 = _mm_loadu_ps(&m[0]);
        __m128 row1 = _mm_loadu_ps(&m[4]);
        __m128 row2 = _mm_loadu_ps(&m[8]);
        __m128 row3 = _mm_loadu_ps(&m[12]);

        // 传统的 SSE 点积方法速度慢。我们改用转置或直接利用 AVX2 双通道并行：
        // 但更直接高效的方式是利用 AVX2 一次性加载半个矩阵（2行）
        __m256 row01 = _mm256_loadu_ps(&m[0]);  // 包含 row0 和 row1
        __m256 row23 = _mm256_loadu_ps(&m[8]);  // 包含 row2 和 row3

        // 把 vector 广播到 256 位寄存器
        __m256 v_xxxx_xxxx = _mm256_set_ps(other.x, other.x, other.x, other.x, other.x, other.x, other.x, other.x);
        // 或者更快的洗牌指令：
        __m256 v_xyzw = _mm256_set_ps(other.w, other.z, other.y, other.x, other.w, other.z, other.y, other.x);

        // 【最优解：基于原代码行优先的点积并行化】
        // 针对单向量，用 SSE/AVX 混合的点积指令 _mm_dp_ps 最直观，但 FMA 广播列更高效。
        // 如果维持行存储，直接用普通乘加：
        __m128 r0 = _mm_mul_ps(row0, _mm_set_ps(other.w, other.z, other.y, other.x)); // 这样做需要 shuffle，不划算。

        // 正确的行优先广播融合（无需依赖矩阵转置）：
        // 我们可以把结果写成：
        __m128 col0 = _mm_set_ps(m[12], m[8], m[4], m[0]);
        __m128 col1 = _mm_set_ps(m[13], m[9], m[5], m[1]);
        __m128 col2 = _mm_set_ps(m[14], m[10], m[6], m[2]);
        __m128 col3 = _mm_set_ps(m[15], m[11], m[7], m[3]);

        __m128 res = _mm_mul_ps(col0, vx);
        res = _mm_fmadd_ps(col1, vy, res);
        res = _mm_fmadd_ps(col2, vz, res);
        res = _mm_fmadd_ps(col3, vw, res);

        Vector4D out;
        _mm_storeu_ps(&out.x, res);
        return out;
    }

    Vector4D operator*(const Vector3D &other) const {
        // return {
        //     m[0] * other.x + m[1] * other.y + m[2] * other.z + m[3],
        //     m[4] * other.x + m[5] * other.y + m[6] * other.z + m[7],
        //     m[8] * other.x + m[9] * other.y + m[10] * other.z + m[11],
        //     m[12] * other.x + m[13] * other.y + m[14] * other.z + m[15]
        // };
        __m128 vx = _mm_set1_ps(other.x);
        __m128 vy = _mm_set1_ps(other.y);
        __m128 vz = _mm_set1_ps(other.z);

        // 提取列
        __m128 col0 = _mm_set_ps(m[12], m[8], m[4], m[0]);
        __m128 col1 = _mm_set_ps(m[13], m[9], m[5], m[1]);
        __m128 col2 = _mm_set_ps(m[14], m[10], m[6], m[2]);
        __m128 col3 = _mm_set_ps(m[15], m[11], m[7], m[3]); // w=1，直接作为基底加进去

        // FMA 链式计算: col0 * x + col1 * y + col2 * z + col3
        __m128 res = _mm_fmadd_ps(col0, vx, col3);
        res = _mm_fmadd_ps(col1, vy, res);
        res = _mm_fmadd_ps(col2, vz, res);

        Vector4D out;
        _mm_storeu_ps(&out.x, res);
        return out;
    }

    [[nodiscard]] String toString() const {
        std::stringstream ss;
        ss << "Matrix{";
        for (int i = 0; i < 15; i++) {
            ss << m[i] << ", ";
        }
        ss << m[15] << "}";
        return ss.str();
    }
};