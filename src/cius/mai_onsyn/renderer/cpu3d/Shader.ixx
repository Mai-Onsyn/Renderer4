module;
export module Shader;
import Types;
import ScreenVertex;
import Color;
import Vectors;
import Light;

export struct Fragment {
    Int64 x, y;
    Float depth;
    Color uvColor;
    Vector3D normal;
};

export namespace Shader {

    Color fragmentShader(const Fragment& fragment) {
        return fragment.uvColor;
    }
}