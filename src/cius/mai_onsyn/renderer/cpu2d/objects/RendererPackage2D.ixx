module;
export module RendererPackage2D;
import Types;
import Vectors;
import Color;
import Graphics2D;
import Box;
import Text;

export enum class Command2DType : UInt8 {
    DrawText,
    DrawRect
};

export struct RendererPackage2D {
    Command2DType cmd;
    Box box;
    Text text;
};

export struct Scene2DSnapShot {
    List<RendererPackage2D> nodes;
};