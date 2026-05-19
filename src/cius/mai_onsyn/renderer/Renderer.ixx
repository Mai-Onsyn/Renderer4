module;
export module Renderer;
import Types;

export class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void renderFrame(UInt8Buffer mainBuffer, Int32 width, Int32 height) = 0;
};