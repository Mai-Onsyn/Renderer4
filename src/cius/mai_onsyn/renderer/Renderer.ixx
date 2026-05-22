module;
#include <semaphore>
#include <atomic>
export module Renderer;
import Types;
import FrameBuffer;
import Thread;
import Logger;
import Time;

export class Renderer {
    Thread* thread = nullptr;
    Float fps = 0;

    std::binary_semaphore sem_main_to_render{0}; // 主线程通知渲染线程
    std::binary_semaphore sem_render_to_main{0}; // 渲染线程通知主线程
    std::atomic<bool> resize_requested{false};
    std::atomic<UInt32> pending_width{0};
    std::atomic<UInt32> pending_height{0};
public:
    Renderer(const UInt32 width, const UInt32 height) : main(FrameBuffer(width, height)) {}
    virtual ~Renderer() {
        delete thread;
    }
    virtual FrameBuffer& getDisplayBuffer() = 0;

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

        main.resize(pending_width.load(std::memory_order_relaxed),
                    pending_height.load(std::memory_order_relaxed));

        resize_requested.store(false, std::memory_order_relaxed);
        sem_main_to_render.release();
    }

    [[nodiscard]] Float getFPS() const {
        return fps;
    }

    void stop() const {
        thread->interrupt();
    }
protected:
    FrameBuffer main;
    Boolean resizeSignal = false;
    Mutex mtx;
    ConditionVariable cv;
    virtual void renderFrame() = 0;
};
