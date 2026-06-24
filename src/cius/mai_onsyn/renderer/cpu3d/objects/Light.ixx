module;
export module Light;
import Vectors;
import Color;
import Types;
import Format;

export enum class LightType {
    Directional,
    Point
};

export struct Light {
    String name;
    Vector3D pos;
    Color color;
    LightType type;
    Vector3D dir = {0, 0, 0};
    Float intensity = 1.0f;
    Float range = 100.0f;

    Light(const String &name, const Vector3D &pos, const Color &color, const LightType& type = LightType::Point)
        : name(name),
          pos(pos),
          color(color),
          type(type) {}

    String toString() const {
        return format("Light{name=%s, pos=%s, color=%s}", name, pos.toString(), color.toString());
    }
};