module;
#include <algorithm>
#include <memory>
#include <thread>
#include <atomic>

export module ThreadPool;
import Thread;
import Types;
import Logger;

// 保证你的结构体中字段对齐和基础类型匹配
struct TaskRange {
    Atomic<Int32> current{0};
    Atomic<Int32> end{0};
};

export class ThreadPool {
    UInt32 size;
    UniquePtr<UniquePtr<Thread>[]> threads;
    UniquePtr<TaskRange[]> threadRanges;

    // 改为匹配 UniquePtr<Runnable>* 类型的数组成员指针
    UniquePtr<Runnable>* currentTasks = nullptr;

    Atomic<Int32> remainingTasks{0};
    mutable Mutex waitMtx;
    mutable ConditionVariable waitCv;

    void threadBody(const UInt32 threadIndex, const StopToken& st) {
        while (!st.stop_requested()) {
            // 没有任何提交的任务波次时，释放时间片
            if (remainingTasks.load(std::memory_order_relaxed) <= 0) {
                Thread::yield();
                continue;
            }

            Runnable* task = nullptr;

            TaskRange& myRange = threadRanges[threadIndex];
            Int32 myIdx = myRange.current.load(std::memory_order_relaxed);
            Int32 myEnd = myRange.end.load(std::memory_order_relaxed);

            // 1. 尝试获取自己区间的任务
            if (myIdx < myEnd) {
                if (myRange.current.compare_exchange_strong(myIdx, myIdx + 1, std::memory_order_relaxed)) {
                    // currentTasks 是 UniquePtr<Runnable>*，用 [idx].get() 获取裸指针
                    task = currentTasks[myIdx].get();
                }
            }

            // 2. 自己区间空了，尝试去窃取其他线程的任务
            if (!task) {
                for (UInt32 i = 0; i < size; ++i) {
                    UInt32 targetIndex = (threadIndex + i + 1) % size;
                    TaskRange& targetRange = threadRanges[targetIndex];

                    Int32 targetIdx = targetRange.current.load(std::memory_order_relaxed);
                    Int32 targetEnd = targetRange.end.load(std::memory_order_relaxed);

                    if (targetIdx < targetEnd) {
                        if (targetRange.current.compare_exchange_strong(targetIdx, targetIdx + 1, std::memory_order_relaxed)) {
                            task = currentTasks[targetIdx].get();
                            break;
                        }
                    }
                }
            }

            // 3. 执行领到的任务
            if (task) {
                task->run();

                // 减小计数，如果是最后一个完成任务的线程，唤醒被阻塞的调用线程
                if (remainingTasks.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    UniqueLock lock(waitMtx);
                    waitCv.notify_all();
                }
            } else {
                Thread::yield();
            }
        }
    }

public:
    explicit ThreadPool(const UInt32 threadCount) : size(threadCount) {
        threads = std::make_unique<std::unique_ptr<Thread>[]>(threadCount);

        // 修复报错关键：必须在构造函数中，线程启动前完成内存分配
        threadRanges = std::make_unique<TaskRange[]>(threadCount);

        for (UInt32 i = 0; i < threadCount; ++i) {
            auto task = [this, i](const StopToken& st) {
                threadBody(i, st);
            };
            threads[i] = std::make_unique<BasicThread<decltype(task)>>(std::move(task));
        }
    }

    void start() const {
        for (UInt32 i = 0; i < size; i++) {
            threads[i]->start();
        }
    }

    void stop() const {
        for (UInt32 i = 0; i < size; i++) {
            threads[i]->interrupt();
        }
    }

    void submit(UniquePtr<Runnable>* tasks, const Int32 totalTasks, const Boolean wait = true) {
        if (!tasks || totalTasks == 0) return;

        currentTasks = tasks;
        remainingTasks.store(totalTasks, std::memory_order_release);

        Int32 tasksPerThread = totalTasks / size;
        Int32 remainder = totalTasks % size;

        Int32 currentStart = 0;
        for (UInt32 i = 0; i < size; ++i) {
            Int32 rangeSize = tasksPerThread + (i < remainder ? 1 : 0);

            threadRanges[i].end.store(currentStart + rangeSize, std::memory_order_relaxed);
            // 必须最后 store current，作为内存栅栏让子线程可见新边界
            threadRanges[i].current.store(currentStart, std::memory_order_release);

            currentStart += rangeSize;
        }

        if (wait) {
            UniqueLock lock(waitMtx);
            waitCv.wait(lock, [this] {
                return remainingTasks.load(std::memory_order_relaxed) <= 0;
            });
        }
    }
};