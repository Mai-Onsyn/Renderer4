module;
export module Triangle;
import Types;
import ScreenVertex;
import Texture;
import Vectors;

export struct Triangle {
    UInt32 v1, v2, v3;
    Texture* texture;
};

export struct ScreenTriangle {
    ScreenVertex v1;
    ScreenVertex v2;
    ScreenVertex v3;
    Texture* texture;
};


export struct ShadowTriangle {
    VectorInt2D v1, v2, v3;
    Float d1, d2, d3;
};