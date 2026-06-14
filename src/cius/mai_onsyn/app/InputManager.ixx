module;
#include <new>
export module InputManager;
import Types;
import Vectors;

export class InputManager {
    Map<Int32, Boolean> keyStates{};
    Map<Int32, Boolean> mouseButtonStates{};
    Vector2D mousePosition{};
    Vector2D mouseDelta{};

    Mutex mtx;
public:
    void updateKey(const Int32 key, const Int32 action) {
        LockGuard lock(mtx);
        if (action == 1) {
            keyStates[key] = true;
        } else if (action == 0) {
            keyStates[key] = false;
        }
    }

    void updateMouseButton(const Int32 button, const Int32 action) {
        LockGuard lock(mtx);
        if (action == 1) {
            mouseButtonStates[button] = true;
        } else if (action == 0) {
            mouseButtonStates[button] = false;
        }
    }

    void updateMousePosition(const Float x, const Float y) {
        LockGuard lock(mtx);
        const Vector2D newPos = {x, y};
        mouseDelta += newPos - mousePosition;
        mousePosition = newPos;
    }

    Boolean isKeyPressed(const Int32 key) {
        LockGuard lock(mtx);
        return keyStates[key];
    }

    Boolean isMouseButtonPressed(const Int32 button) {
        LockGuard lock(mtx);
        return mouseButtonStates[button];
    }

    Vector2D getMousePosition() {
        LockGuard lock(mtx);
        return mousePosition;
    }

    Vector2D consumeMouseMove() {
        LockGuard lock(mtx);
        const Vector2D delta = mouseDelta;
        mouseDelta = {0, 0};
        return delta;
    }
};
