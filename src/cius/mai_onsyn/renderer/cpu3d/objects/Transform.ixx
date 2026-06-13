module;
export module Transform;
import Matrix;
import Types;
import Format;

export class alignas(32) Transform {
    public:
    Matrix4x4 modelMatrix;

    Transform() : modelMatrix(Matrix4x4::I()) {}
    explicit Transform(Matrix4x4 modelMatrix) : modelMatrix(move(modelMatrix)) {}

    // 手写或显式接管拷贝与移动
    Transform(const Transform& other) noexcept {
        this->modelMatrix = other.modelMatrix; // 这里会调用 Matrix4x4 安全的赋值或拷贝
    }
    Transform(Transform&& other) noexcept {
        this->modelMatrix = move(other.modelMatrix);
    }
    Transform& operator=(const Transform& other) noexcept {
        if (this != &other) {
            this->modelMatrix = other.modelMatrix;
        }
        return *this;
    }
    Transform& operator=(Transform&& other) noexcept {
        if (this != &other) {
            this->modelMatrix = move(other.modelMatrix);
        }
        return *this;
    }

    String toString() const {
        return format("Transform{modelMatrix=%s}", modelMatrix.toString());
    }
};