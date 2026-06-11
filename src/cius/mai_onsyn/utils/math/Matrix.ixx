module;
#include <sstream>
export module Matrix;
import Types;

class Matrix {

};

export class Matrix4x4 final : public Matrix {
    UniquePtr<Float[]> m;
public:
    Matrix4x4() : m(new Float[16]) {}

    static Matrix4x4 I() {
        Matrix4x4 matrix4;
        for (int i = 0; i < 4; i++) {
            matrix4.m[i * 4 + i] = 1;
        }
        return matrix4;
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