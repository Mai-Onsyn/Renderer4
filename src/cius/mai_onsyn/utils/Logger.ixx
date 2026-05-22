module;
#include <iostream>
export module Logger;
import Types;

using std::cout;
using std::endl;

Mutex mtx;
ConditionVariable cv;
export class Log {
public:
    static void info(const std::string& msg) {
        {
            LockGuard lock(mtx);
            cout << msg << endl;
        }
        cv.notify_all();
    }
};