module;
export module Transform;
import Matrix;
import Types;
import Format;

export class Transform {
public:
    Matrix4x4 modelMatrix;

    String toString() const {
        return format("Transform{modelMatrix=%s}", modelMatrix.toString());
    }
};