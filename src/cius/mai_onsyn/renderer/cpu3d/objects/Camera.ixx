module;
export module Camera;
import Types;
import Vectors;
import Matrix;

export struct Camera {
    Vector3D pos;
    Vector3D vx;
    Vector3D vy;
    Vector3D vz;

    Camera() : pos({0, 0, 0}), vx({1, 0, 0}), vy({0, 1, 0}), vz({0, 0, 1}) {}

    Matrix4x4 getViewTransformMatrix() const {
        Matrix4x4 matrix;
        matrix.m[0] = vx.x;
        matrix.m[1] = vx.y;
        matrix.m[2] = vx.z;
        matrix.m[3] = -(vx.x * pos.x + vx.y * pos.y + vx.z * pos.z);

        matrix.m[4] = vy.x;
        matrix.m[5] = vy.y;
        matrix.m[6] = vy.z;
        matrix.m[7] = -(vy.x * pos.x + vy.y * pos.y + vy.z * pos.z);

        matrix.m[8] = vz.x;
        matrix.m[9] = vz.y;
        matrix.m[10] = vz.z;
        matrix.m[11] = -(vz.x * pos.x + vz.y * pos.y + vz.z * pos.z);

        matrix.m[12] = 0.0f;
        matrix.m[13] = 0.0f;
        matrix.m[14] = 0.0f;
        matrix.m[15] = 1.0f;

        return matrix;
    }
};