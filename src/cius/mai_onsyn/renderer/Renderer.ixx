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

export class Renderer {
    Thread* thread = nullptr;
    Float fps = 0;

    std::binary_semaphore sem_main_to_render{0}; // 主线程通知渲染线程
    std::binary_semaphore sem_render_to_main{0}; // 渲染线程通知主线程
    std::atomic<bool> resize_requested{false};
    std::atomic<UInt32> pending_width{0};
    std::atomic<UInt32> pending_height{0};
protected:
    TripleBuffer tripleBuffer;
    Boolean resizeSignal = false;
    Mutex mtx;
    ConditionVariable cv;
    FontDrawer fontDrawer{""};
    Image* textBuffer = new Image(800, 600);

    virtual void renderFrame() = 0;
public:
    UInt32 width, height;

    Renderer(const UInt32 width, const UInt32 height):
        tripleBuffer(width, height),
        fontDrawer("assets/fonts/msyh.ttf"),
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
            UInt64 timestamp = millisTime();
            UInt32 renderedFrameCount = 0;
            while (!st.stop_requested()) {

                if (resize_requested.load(std::memory_order_relaxed)) {
                    sem_render_to_main.release();

                    sem_main_to_render.acquire();
                }

                renderFrame();
                renderedFrameCount++;
                if (const UInt64 currentTime = millisTime(); currentTime - timestamp >= 1000) {
                    fps = static_cast<Float>(renderedFrameCount) / static_cast<Float>(currentTime - timestamp) * 1000;
                    timestamp = currentTime;
                    renderedFrameCount = 0;
                }
            }
        });
        thread->start();
    }

    void resize(const UInt32 width, const UInt32 height) {
        pending_width.store(width, std::memory_order_relaxed);
        pending_height.store(height, std::memory_order_relaxed);
        resize_requested.store(true, std::memory_order_release);

        sem_render_to_main.acquire();

        tripleBuffer.resize(pending_width.load(std::memory_order_relaxed),
                    pending_height.load(std::memory_order_relaxed));
        this->width = width;
        this->height = height;

        resize_requested.store(false, std::memory_order_relaxed);
        sem_main_to_render.release();
    }

    [[nodiscard]] Float getFPS() const {
        return fps;
    }

    void stop() const {
        thread->interrupt();
    }
};
