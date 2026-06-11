module;
export module Light;
import Vectors;
import Color;
import Types;
import Format;

export struct Light {
    String name;
    Vector3D pos;
    Color color;

    Light(const String &name, const Vector3D &pos, const Color &color)
        : name(name),
          pos(pos),
          color(color) {
    }

    String toString() const {
        return format("Light{name=%s, pos=%s, color=%s}", name, pos.toString(), color.toString());
    }
};