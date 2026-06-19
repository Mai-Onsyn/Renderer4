module;
#include <semaphore>
#include <atomic>
export module Renderer;
import Types;
import FrameBuffer;
import Thread;
import Logger;
import Time;
import TripleBuffer;
import FontDrawer;
import Image;
import Color;
import SceneSnapShotBuffer;

constexpr Float TARGET_FPS = 100;

export template<typename SnapshotT>
class Renderer {
    Thread* thread = nullptr;
    Float fps = 0;

    Semaphore sem_main_to_render{0}; // 主线程通知渲染线程
    Semaphore sem_render_to_main{0}; // 渲染线程通知主线程
    Atomic<bool> resize_requested{false};
    Atomic<Int32> pending_width{0};
    Atomic<Int32> pending_height{0};
protected:
    TripleBuffer tripleBuffer;
    UniquePtr<Float[]> depthBuffer;
    RenderSnapShotDoubleBuffer<SnapshotT> snapShotBuffer{};
    Boolean resizeSignal = false;
    Mutex mtx;
    ConditionVariable cv;
    FontDrawer fontDrawer{"assets/fonts/AaDeGuLaJian-2.ttf"};
    Image* textBuffer = new Image(800, 600);

    virtual void renderFrame() = 0;
    virtual void onResize(Int32 width, Int32 height) = 0;
public:
    Int32 width, height;

    Renderer(const Int32 width, const Int32 height):
        tripleBuffer(width, height),
        depthBuffer(makeFloatBuffer(width * height)),
        width(width), height(height) {}

    virtual ~Renderer() {
        stop();
        delete thread;
    }

    FrameBuffer* getDisplayBuffer() {
        return tripleBuffer.getDisplayBuffer();
    }

    void releaseDisplayBuffer() {
        tripleBuffer.releaseDisplayBuffer();
    }

    void start() {
        thread = new BasicThread([this](const StopToken& st) {
            UInt64 fpsUpdateTimestamp = millisTime();
            UInt64 lastFrameTimestamp = millisTime();
            UInt32 renderedFrameCount = 0;
            while (!st.stop_requested()) {

                if (resize_requested.load(std::memory_order_acquire)) {
                    Int32 target_w = pending_width.load(std::memory_order_relaxed);
                    Int32 target_h = pending_height.load(std::memory_order_relaxed);

                    tripleBuffer.resize(target_w, target_h);
                    depthBuffer.reset(new Float[target_w * target_h]);
                    this->width = target_w;
                    this->height = target_h;

                    onResize(target_w, target_h);

                    resize_requested.store(false, std::memory_order_relaxed);
                }

                renderFrame();
                renderedFrameCount++;
                const UInt64 currentTime = millisTime();
                if (currentTime - fpsUpdateTimestamp >= 1000) {
                    fps = static_cast<Float>(renderedFrameCount) / static_cast<Float>(currentTime - fpsUpdateTimestamp) * 1000;
                    fpsUpdateTimestamp = currentTime;
                    renderedFrameCount = 0;
                }
                if constexpr (TARGET_FPS != 0) {
                    const UInt64 sleepTime = 1000.0f / TARGET_FPS - (currentTime - lastFrameTimestamp);
                    Thread::sleep(sleepTime);
                    lastFrameTimestamp = currentTime;
                }
            }
        });
        thread->start();
    }

    void resize(const Int32 width, const Int32 height) {
        pending_width.store(width, std::memory_order_relaxed);
        pending_height.store(height, std::memory_order_relaxed);
        resize_requested.store(true, std::memory_order_release);
    }

    [[nodiscard]] Float getFPS() const {
        return fps;
    }

    void stop() const {
        if (thread) {
            thread->interrupt();
            thread->join();
        }
    }

    void submitSnapShot(SnapshotT* snapShot) {
        snapShotBuffer.submit(snapShot);
    }
};
