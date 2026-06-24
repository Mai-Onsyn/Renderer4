module;
export module Light;
import Vectors;
import Color;
import Types;
import Format;

export enum class LightType {
    Face,
    Point
};

export struct Light {
    String name;
    Vector3D pos;
    Color color;
    LightType type;
    Vector3D dir = {0, 0, 0};
    Float intensity = 0.6f;
    Float range = 600.0f;
    Float a = 0.000007f;
    Float b = 0.000003f;

    Light(const String &name, const Vector3D &pos, const Color &color, const LightType& type = LightType::Point)
        : name(name),
          pos(pos),
          color(color),
          type(type) {}

    String toString() const {
        return format("Light{name=%s, pos=%s, color=%s}", name, pos.toString(), color.toString());
    }
};