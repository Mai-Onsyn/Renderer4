module;
#include <iostream>
export module Logger;
import Types;

using std::cout;
using std::endl;

class Log {
    static Mutex mtx;
    static ConditionVariable cv;

public:
    static void info(const std::string& msg) {
        {
            LockGuard lock(mtx);
            cout << msg << endl;
        }
        cv.notify_all();
    }
};