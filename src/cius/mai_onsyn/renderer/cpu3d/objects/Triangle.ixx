module;
export module Triangle;
import Types;
import ScreenVertex;

export struct Triangle {
    UInt32 v1, v2, v3;
    UInt32 materialIndex;
};

export struct ScreenTriangle {
    ScreenVertex v1;
    ScreenVertex v2;
    ScreenVertex v3;
};