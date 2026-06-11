module;
export module Camera;
import Types;
import Vectors;

export struct Camera {
    Vector3D pos;
    Vector3D vx;
    Vector3D vy;
    Vector3D vz;

    Camera() : pos({0, 0, 0}), vx({1, 0, 0}), vy({0, 1, 0}), vz({0, 0, 1}) {}
};