module;
export module Triangle;
import Types;
import ScreenVertex;
import Texture;

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