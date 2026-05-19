module;
#include <mutex>
#include <queue>
#define Queue std::queue
export module BlockingQueue;
import Types;

using std::move;

export template<typename T>
class BlockingQueue {
    Queue<T> queue;
    Mutex mtx;
    ConditionVariable cv;
    Boolean isClosed = false;
public:
    void push(const T& value) {
        {
            LockGuard lock(mtx);
            queue.push(value);
        }
        cv.notify_all();
    }

    void push(T&& value) {
        {
            LockGuard lock(mtx);
            queue.push(move(value));
        }
        cv.notify_all();
    }

    T pop() {
        UniqueLock lock(mtx);

        cv.wait(lock, [this] {
            return !queue.empty() || isClosed;
        });

        if (queue.empty()) {
            throw RuntimeError("Queue closed");
        }

        T value = move(queue.front());
        queue.pop();

        return value;
    }

    void close() {
        {
            LockGuard lock(mtx);
            isClosed = true;
        }
        cv.notify_all();
    }
};