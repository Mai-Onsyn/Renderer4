#include <iostream>
#include <memory>

import Application;
import Time;
import Types;
import Tile;
import MathUtil;
import ThreadPool;
import Thread;
import Logger;

int main() {
    Application app("DisplayWindow", 800, 600);
    app.run();

    // auto rawPtrs = std::make_unique<UniquePtr<Runnable>[]>(10);
    //
    // for (int i = 0; i < 10; i++) {
    //     Tile* t = new Tile(0, 0, i, i);
    //     if (i < 5) {
    //         // 前5个任务属于线程0的防区，让它们很慢
    //         rawPtrs[i] = std::make_unique<TileTask>(t, 1000);
    //     } else {
    //         // 后5个任务属于线程1的防区，让它们极快
    //         rawPtrs[i] = std::make_unique<TileTask>(t, 0);
    //     }
    // }
    //
    // ThreadPool tp(2);
    // tp.start();
    //
    // Log::debug("=== Submit Tasks ===");
    // tp.submit(rawPtrs.get(), 10);
    // Log::debug("=== All Finished ===");
    //
    // // Thread::sleep(10000);
    //
    // tp.stop();

    return 0;
}