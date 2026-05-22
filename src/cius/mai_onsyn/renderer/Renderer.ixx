module;
export module Renderer;
import Types;
import FrameBuffer;

export class Renderer {
    FrameBuffer* windowBuffer;
public:
    virtual ~Renderer() = default;
    virtual void renderFrame(FrameBuffer* main) = 0;
};