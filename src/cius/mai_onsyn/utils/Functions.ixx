module;
#include <string>
export module Functions;
import Types;

using std::to_string;

export String toString(const Int32 value) {
    return to_string(value);
}

export String toString(const UInt32 value) {
    return to_string(value);
}

export String toString(const Float value) {
    return to_string(value);
}