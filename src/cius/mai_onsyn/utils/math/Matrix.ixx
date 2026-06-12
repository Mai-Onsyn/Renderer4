module;
#include <sstream>
export module Matrix;
import Types;

struct Matrix {
};

export struct Matrix4x4 final : public Matrix {
    Float m[16]{};
    Matrix4x4() = default;

    static Matrix4x4 I() {
        Matrix4x4 matrix4;
        for (int i = 0; i < 4; i++) {
            matrix4.m[i * 4 + i] = 1;
        }
        return matrix4;
    }

    Matrix4x4 operator*(const Matrix4x4 &other) const {
        Matrix4x4 result;
        result.m[0] = m[0] * other.m[0] + m[1] * other.m[4] + m[2] * other.m[8] + m[3] * other.m[12];
        result.m[1] = m[0] * other.m[1] + m[1] * other.m[5] + m[2] * other.m[9] + m[3] * other.m[13];
        result.m[2] = m[0] * other.m[2] + m[1] * other.m[6] + m[2] * other.m[10] + m[3] * other.m[14];
        result.m[3] = m[0] * other.m[3] + m[1] * other.m[7] + m[2] * other.m[11] + m[3] * other.m[15];

        result.m[4] = m[4] * other.m[0] + m[5] * other.m[4] + m[6] * other.m[8] + m[7] * other.m[12];
        result.m[5] = m[4] * other.m[1] + m[5] * other.m[5] + m[6] * other.m[9] + m[7] * other.m[13];
        result.m[6] = m[4] * other.m[2] + m[5] * other.m[6] + m[6] * other.m[10] + m[7] * other.m[14];
        result.m[7] = m[4] * other.m[3] + m[5] * other.m[7] + m[6] * other.m[11] + m[7] * other.m[15];

        result.m[8] = m[8] * other.m[0] + m[9] * other.m[4] + m[10] * other.m[8] + m[11] * other.m[12];
        result.m[9] = m[8] * other.m[1] + m[9] * other.m[5] + m[10] * other.m[9] + m[11] * other.m[13];
        result.m[10] = m[8] * other.m[2] + m[9] * other.m[6] + m[10] * other.m[10] + m[11] * other.m[14];
        result.m[11] = m[8] * other.m[3] + m[9] * other.m[7] + m[10] * other.m[11] + m[11] * other.m[15];

        result.m[12] = m[12] * other.m[0] + m[13] * other.m[4] + m[14] * other.m[8] + m[15] * other.m[12];
        result.m[13] = m[12] * other.m[1] + m[13] * other.m[5] + m[14] * other.m[9] + m[15] * other.m[13];
        result.m[14] = m[12] * other.m[2] + m[13] * other.m[6] + m[14] * other.m[10] + m[15] * other.m[14];
        result.m[15] = m[12] * other.m[3] + m[13] * other.m[7] + m[14] * other.m[11] + m[15] * other.m[15];
        return result;
    }

    String toString() const {
        std::stringstream ss;
        ss << "Matrix{";
        for (int i = 0; i < 15; i++) {
            ss << m[i] << ", ";
        }
        ss << m[15] << "}";
        return ss.str();
    }
};

export class Matrix3x3 final : public Matrix {
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

    Float calcDet() const {
        return m[0] * (m[4] * m[8] - m[5] * m[7]) - m[1] * (m[3] * m[8] - m[5] * m[6]) + m[2] * (m[3] * m[7] - m[4] * m[6]);
    }
};