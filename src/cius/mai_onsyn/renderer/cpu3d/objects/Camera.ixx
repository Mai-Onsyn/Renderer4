module;
#include <cmath>
export module Camera;
import Types;
import Vectors;
import Matrix;

export struct Camera {
    const Vector3D up{0.0f, 1.0f, 0.0f};
    Vector3D pos;
    Vector3D vx;
    Vector3D vy;
    Vector3D vz;

    Float fov;
    const Float near = 0.1;

    Camera() : pos({0, 0, 0}), vx({1, 0, 0}), vy({0, 1, 0}), vz({0, 0, 1}), fov(70.0f) {}

    void moveX(const float distance) {
        pos += vx * distance;
    }

    void moveY(const float distance) {
        pos += up * distance;
    }

    void moveZ(const float distance) {
        pos += (vx ^ up) * distance;
    }

    void rotateY(const float angle) {
        vy = vy.rotate(angle, vx);
        vz = vz.rotate(angle, vx);
        // vy = Vector3D::rotate(vx, vy, angle);
        // vz = Vector3D::rotate(vx, vz, angle);
    }

    void rotateX(const float angle) {
        vx = vx.rotate(angle, up);
        vy = vy.rotate(angle, up);
        vz = vz.rotate(angle, up);
        // vx = Vector3D::rotate(up, vx, angle);
        // vy = Vector3D::rotate(up, vy, angle);
        // vz = Vector3D::rotate(up, vz, angle);
    }

    [[nodiscard]] Matrix4x4 getViewTransformMatrix() const {
        Matrix4x4 matrix;
        matrix[0] = vx.x;
        matrix[1] = vx.y;
        matrix[2] = vx.z;
        matrix[3] = -(vx.x * pos.x + vx.y * pos.y + vx.z * pos.z);

        matrix[4] = vy.x;
        matrix[5] = vy.y;
        matrix[6] = vy.z;
        matrix[7] = -(vy.x * pos.x + vy.y * pos.y + vy.z * pos.z);

        matrix[8] = vz.x;
        matrix[9] = vz.y;
        matrix[10] = vz.z;
        matrix[11] = -(vz.x * pos.x + vz.y * pos.y + vz.z * pos.z);

        // matrix[12] = 0.0f;
        // matrix[13] = 0.0f;
        // matrix[14] = 0.0f;
        matrix[15] = 1.0f;

        return matrix;
    }

    [[nodiscard]] Matrix4x4 getProjectionMatrix(const Float aspect) const {
        Matrix4x4 matrix;
        matrix[0] = 1.0f / (aspect * std::tan(fov));
        matrix[5] = 1.0f / std::tan(fov);
        matrix[10] = 1;
        matrix[11] = -near;
        matrix[14] = 1;
        return matrix;
    }
};