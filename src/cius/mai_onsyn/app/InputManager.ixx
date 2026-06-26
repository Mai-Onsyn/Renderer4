module;
#include <new>
export module InputManager;
import Types;
import Vectors;
import Queue;

export class InputManager {
    Map<Int32, Boolean> keyStates{};
    Map<Int32, Boolean> mouseButtonStates{};
    Vector2D mousePosition{};
    Vector2D mouseDelta{};
    Queue<Int32> keyInputs;
    Queue<Pair<Int32, Vector2D>> mouseInputs;

    Mutex mtx;
public:
    void updateKey(const Int32 key, const Int32 action) {
        LockGuard lock(mtx);
        if (action == 1) {          // Press
            keyStates[key] = true;
            keyInputs.push(key);
        } else if (action == 0) {   // Release
            keyStates[key] = false;
        } else if (action == 2) {   // Repeat
            keyInputs.push(key);
        }
    }

    void updateMouseButton(const Int32 button, const Int32 action) {
        LockGuard lock(mtx);
        if (action == 1) {
            mouseButtonStates[button] = true;
            mouseInputs.push({button, mousePosition});
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

    Boolean hasKeyInput() {
        return !keyInputs.empty();
    }

    Boolean hasMouseButtonInput() {
        return !mouseInputs.empty();
    }

    Pair<Int32, Vector2D> consumeMouseButtonInput() {
        return mouseInputs.pop();
    }

    Int32 consumeKeyInput() {
        return keyInputs.pop();
    }

    void clearAllKeyInputs() {
        keyInputs.clear();
    }

    void clearAllMouseInputs() {
        mouseInputs.clear();
    }
};
