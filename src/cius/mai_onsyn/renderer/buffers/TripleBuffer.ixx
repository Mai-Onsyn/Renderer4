module;
#include <memory>
export module TripleBuffer;
import Types;
import FrameBuffer;

typedef std::unique_ptr<FrameBuffer> BufferPtr;
using std::make_unique;

export class TripleBuffer {
    BufferPtr buffers[3];
    Int8 displayBuffer = -1;
    Int8 readyBuffer = -1;
    Int8 renderBuffer = -1;
    UInt64 bufferSize;
    Mutex mtx;

    [[nodiscard]] Int8 findFreeBuffer() const {
        for (Int8 i = 0; i < 3; i++) {
            if (i != displayBuffer && i != readyBuffer) {
                return i;
            }
        }
        return -1;
    }
public:
    TripleBuffer(const UInt32 width, const UInt32 height) : bufferSize(width * height * 4) {
        if (width == 0 || height == 0) {
            throw std::invalid_argument("Buffer dimensions cannot be zero");
        }

        for (auto& buffer : buffers) {
            buffer = make_unique<FrameBuffer>(width, height);
        }

        displayBuffer = 0;
        readyBuffer = -1;
        renderBuffer = 1;
    }
    ~TripleBuffer() = default;

    TripleBuffer(const TripleBuffer&) = delete;
    TripleBuffer& operator=(const TripleBuffer&) = delete;

    FrameBuffer* getRenderBuffer() {
        LockGuard lock(mtx);
        return buffers[renderBuffer].get();
    }

    void commit() {
        LockGuard lock(mtx);
        readyBuffer = renderBuffer;
        renderBuffer = findFreeBuffer();
        if (renderBuffer == -1) {
            renderBuffer = displayBuffer;
        }
    }

    FrameBuffer* getDisplayBuffer() {
        LockGuard lock(mtx);
        if (readyBuffer != -1) {
            displayBuffer = readyBuffer;
            readyBuffer = -1;
        }
        return buffers[displayBuffer].get();
    }

    void releaseDisplayBuffer() {
        LockGuard lock(mtx);
        // displayBuffer = -1;
        if (readyBuffer != -1) {
            displayBuffer = readyBuffer;
            readyBuffer = -1;
        }
    }

    void resize(const UInt32 width, const UInt32 height) {
        LockGuard lock(mtx);
        for (const auto& buffer : buffers) {
            buffer->resize(width, height);
        }
    }
};
