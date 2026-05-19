module;
#include <chrono>
export module Time;
import Types;

using namespace std::chrono;
using std::chrono::duration_cast;

export Int64 millisTime() {
    const auto now = system_clock::now();
    const auto millis = duration_cast<milliseconds>(now.time_since_epoch());
    return millis.count();
}

export Int64 nanoTime() {
    const auto now = steady_clock::now();
    const auto nanos = duration_cast<nanoseconds>(now.time_since_epoch());
    return nanos.count();
}