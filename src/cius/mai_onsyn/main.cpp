#include <chrono>
#include <iostream>
#include <vector>

import FramebufferWindow;
import Time;
import Types;

int main() {
    const int width = 800;
    const int height = 600;

    FramebufferWindow window(width, height, "Window");

    if (!window.initialize()) {
        return -1;
    }

    // 1. 注册键盘事件捕获
    window.setKeyCallback([](int key, int scancode, int action, int mods) {
        if (action == 1) { // GLFW_PRESS
            std::cout << "Key.ixx Pressed: " << key << "\n";
        }
    });

    // 2. 注册鼠标移动事件捕获
    window.setMousePosCallback([](double xpos, double ypos) {
        std::cout << "Mouse Position: X=" << xpos << ", Y=" << ypos << "\r" << std::flush;
    });

    // 3. 注册鼠标按键事件捕获
    window.setMouseButtonCallback([](int button, int action, int mods) {
        if (action == 1) {
            std::cout << "\nMouse Button Pressed: " << button << "\n";
        }
    });

    window.setResizeCallback([](int newWidth, int newHeight) {
        std::cout << "\nWindow resized to: " << newWidth << "x" << newHeight << std::endl;
    });

    // 创建 CPU 端的本地帧缓冲区 (RGBA 格式，4字节/像素)
    std::vector<uint8_t> framebuffer(width * height * 4, 255);

    uint8_t colorOffset = 0;
    Int64 start = millisTime();
    while (!window.shouldClose()) {
        colorOffset++;
        Int64 fill = nanoTime();
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = (y * width + x) * 4;
                framebuffer[index + 0] = static_cast<uint8_t>(x * 255 / width + colorOffset);
                framebuffer[index + 1] = static_cast<uint8_t>(y * 255 / height);
                framebuffer[index + 2] = 128;
                framebuffer[index + 3] = 255;
            }
        }
        Int64 upd = nanoTime();
        // std::cout << "fill: " << (upd - fill) / 1000 << "\n";
        window.update(framebuffer);
        // std::cout << "update: " << (nanoTime() - upd) / 1000 << "\n";
        if (colorOffset % 128 == 0) {
            std::cout << "FPS: " << 1000.0 / (millisTime() - start) * 128 << "\n";
            start = millisTime();
        }
    }

    return 0;
}