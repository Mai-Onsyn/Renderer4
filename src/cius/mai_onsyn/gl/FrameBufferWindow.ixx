module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <functional>

export module FramebufferWindow;

export class FramebufferWindow {
public:
    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    using MousePosCallback = std::function<void(double xpos, double ypos)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    using ResizeCallback = std::function<void(int width, int height)>;

    FramebufferWindow(const int width, const int height, std::string title)
        : m_width(width), m_height(height), m_title(std::move(title)) {}

    ~FramebufferWindow() {
        cleanup();
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

    void setKeyCallback(const KeyCallback &cb) { m_keyCb = cb; }
    void setMousePosCallback(const MousePosCallback &cb) { m_mousePosCb = cb; }
    void setMouseButtonCallback(const MouseButtonCallback &cb) { m_mouseButtonCb = cb; }
    void setResizeCallback(const ResizeCallback& cb) { m_resizeCb = cb; }

    void update(const std::vector<uint8_t>& pixelData) const {
        glfwPollEvents();

        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shaderProgram);
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(m_window);
    }

private:
    int m_width;
    int m_height;
    std::string m_title;
    GLFWwindow* m_window = nullptr;

    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    GLuint m_textureID = 0;
    GLuint m_shaderProgram = 0;

    KeyCallback m_keyCb;
    MousePosCallback m_mousePosCb;
    MouseButtonCallback m_mouseButtonCb;
    ResizeCallback m_resizeCb;

    void setupCallbacks() const {
        glfwSetKeyCallback(m_window, [](GLFWwindow* w, const int k, const int s, const int a, const int m) {
            const auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w));
            if (self && self->m_keyCb) self->m_keyCb(k, s, a, m);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* w, const double x, const double y) {
            const auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w));
            if (self && self->m_mousePosCb) self->m_mousePosCb(x, y);
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* w, const int b, const int a, const int m) {
            const auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w));
            if (self && self->m_mouseButtonCb) self->m_mouseButtonCb(b, a, m);
        });

        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* w, const int width, const int height) {
            if (auto* self = static_cast<FramebufferWindow*>(glfwGetWindowUserPointer(w))) {
                glViewport(0, 0, width, height);

                self->m_width = width;
                self->m_height = height;

                if (self->m_resizeCb) {
                    self->m_resizeCb(width, height);
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