module;
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <utility>
#include <string>
#include <memory>
export module FramebufferWindow;
import FrameBuffer;

export class FramebufferWindow {
    struct KeyInvokerBase         { virtual ~KeyInvokerBase() = default;         virtual void invoke(int, int, int, int) const = 0; };
    struct MousePosInvokerBase    { virtual ~MousePosInvokerBase() = default;    virtual void invoke(double, double) const = 0; };
    struct MouseButtonInvokerBase { virtual ~MouseButtonInvokerBase() = default; virtual void invoke(int, int, int) const = 0; };
    struct ResizeInvokerBase      { virtual ~ResizeInvokerBase() = default;      virtual void invoke(int, int) const = 0; };

    template<typename F>
    struct KeyInvoker final : KeyInvokerBase {
        F lambda;
        KeyInvoker(F&& f) : lambda(std::forward<F>(f)) {}
        void invoke(int k, int s, int a, int m) const override { lambda(k, s, a, m); }
    };

    template<typename F>
    struct MousePosInvoker final : MousePosInvokerBase {
        F lambda;
        MousePosInvoker(F&& f) : lambda(std::forward<F>(f)) {}
        void invoke(double x, double y) const override { lambda(x, y); }
    };

    template<typename F>
    struct MouseButtonInvoker final : MouseButtonInvokerBase {
        F lambda;
        MouseButtonInvoker(F&& f) : lambda(std::forward<F>(f)) {}
        void invoke(int b, int a, int m) const override { lambda(b, a, m); }
    };

    template<typename F>
    struct ResizeInvoker final : ResizeInvokerBase {
        F lambda;
        ResizeInvoker(F&& f) : lambda(std::forward<F>(f)) {}
        void invoke(int w, int h) const override { lambda(w, h); }
    };

public:
    FramebufferWindow(const int width, const int height, std::string title)
        : m_width(width), m_height(height), m_title(std::move(title)) {}

    ~FramebufferWindow() {
        cleanup();
    }

    void moveMouse(const int x, const int y) {
        glfwSetCursorPos(m_window, x, y);
    }

    void disableCursor() {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void enableCursor() {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    bool isFocused() const {
        return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) == GLFW_TRUE;
    }

    bool initialize() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, this);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            std::cerr << "Failed to initialize GLAD\n";
            return false;
        }

        setupQuad();
        setupTexture();
        setupCallbacks();

        return true;
    }

    [[nodiscard]] bool shouldClose() const {
        return glfwWindowShouldClose(m_window);
    }

    static void setVsync(bool vsync) {
        glfwSwapInterval(vsync ? 1 : 0);
    }

    template<typename F> void setKeyCallback(F&& cb) { m_keyCb = std::make_unique<KeyInvoker<std::decay_t<F>>>(std::forward<F>(cb)); }
    template<typename F> void setMousePosCallback(F&& cb) { m_mousePosCb = std::make_unique<MousePosInvoker<std::decay_t<F>>>(std::forward<F>(cb)); }
    template<typename F> void setMouseButtonCallback(F&& cb) { m_mouseButtonCb = std::make_unique<MouseButtonInvoker<std::decay_t<F>>>(std::forward<F>(cb)); }
    template<typename F> void setResizeCallback(F&& cb) { m_resizeCb = std::make_unique<ResizeInvoker<std::decay_t<F>>>(std::forward<F>(cb)); }

    void update(const FrameBuffer* pixelData) {
        glfwPollEvents();

        glBindTexture(GL_TEXTURE_2D, m_textureID);

        if (m_texWidth != pixelData->width || m_texHeight != pixelData->height) {
            m_texWidth = pixelData->width;
            m_texHeight = pixelData->height;

            // 显式重新分配 GPU 纹理内存容量
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texWidth, m_texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData->getBuffer());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shaderProgram);
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(m_window);
    }

    void update() {
        glfwPollEvents();
    }

private:
    int m_width;
    int m_height;
    int m_texWidth = 0;
    int m_texHeight = 0;
    std::string m_title;
    GLFWwindow* m_window = nullptr;

    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    GLuint m_textureID = 0;
    GLuint m_shaderProgram = 0;

    // 内部改用底层的虚基类智能指针存储
    std::unique_ptr<KeyInvokerBase>         m_keyCb;
    std::unique_ptr<MousePosInvokerBase>    m_mousePosCb;
    std::unique_ptr<MouseButtonInvokerBase> m_mouseButtonCb;
    std::unique_ptr<ResizeInvokerBase>      m_resizeCb;

    void setupCallbacks() const {
        glfwSetKeyCallback(m_window, [](GLFWwindow* w, const int k, const int s, const int a, const int m) {
            const auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w));
            if (self && self->m_keyCb) self->m_keyCb->invoke(k, s, a, m);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* w, const double x, const double y) {
            const auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w));
            if (self && self->m_mousePosCb) self->m_mousePosCb->invoke(x, y);
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* w, const int b, const int a, const int m) {
            const auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w));
            if (self && self->m_mouseButtonCb) self->m_mouseButtonCb->invoke(b, a, m);
        });

        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* w, const int width, const int height) {
            if (auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w))) {
                glViewport(0, 0, width, height);

                self->m_width = width;
                self->m_height = height;

                if (self->m_resizeCb) {
                    self->m_resizeCb->invoke(width, height);
                }
            }
        });
    }

    void setupQuad() {
        constexpr float vertices[] = {
             1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f,  1.0f, 0.0f,  0.0f, 0.0f
        };
        const unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        const auto vShaderCode = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec2 aTexCoord;
            out vec2 TexCoord;
            void main() {
                gl_Position = vec4(aPos, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        const auto fShaderCode = R"(
            #version 330 core
            out vec4 FragColor;
            in vec2 TexCoord;
            uniform sampler2D screenTexture;
            void main() {
                FragColor = texture(screenTexture, TexCoord);
            }
        )";

        const GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, nullptr);
        glCompileShader(vertex);

        const GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, nullptr);
        glCompileShader(fragment);

        m_shaderProgram = glCreateProgram();
        glAttachShader(m_shaderProgram, vertex);
        glAttachShader(m_shaderProgram, fragment);
        glLinkProgram(m_shaderProgram);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void setupTexture() {
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    void cleanup() const {
        if (m_vao) {
            glDeleteVertexArrays(1, &m_vao);
            glDeleteBuffers(1, &m_vbo);
            glDeleteBuffers(1, &m_ebo);
            glDeleteTextures(1, &m_textureID);
            glDeleteProgram(m_shaderProgram);
        }
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();
    }
};