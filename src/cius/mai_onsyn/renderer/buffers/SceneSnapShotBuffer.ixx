module;
#include <atomic>
export module SceneSnapShotBuffer;
import Types;

export template<typename SnapshotT>
class RenderSnapShotDoubleBuffer {
    UniquePtr<SnapshotT> front;
    UniquePtr<SnapshotT> back;

    Mutex mtx;
    Atomic<Boolean> hasNewSnapShot{false};
public:

    void submit(SnapshotT* snapShot) {
        LockGuard lock(mtx);
        back.reset(snapShot);
        hasNewSnapShot.store(true, std::memory_order_release);
    }

    void swap() {
        if (!hasNewSnapShot.load(std::memory_order_acquire)) return;

        UniquePtr<SnapshotT> oldFront;

        {
            LockGuard lock(mtx);
            oldFront = std::move(front);
            front = std::move(back);

            hasNewSnapShot.store(false, std::memory_order_relaxed);
        }
    }

    [[nodiscard]] const SnapshotT* getContex() const {
        return front.get();
    }
};