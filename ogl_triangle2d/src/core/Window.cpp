#include "core/Window.hpp"
#include "core/Log.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

static void framebuffer_cb(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

namespace core {

Window::Window(int width, int height, const std::string& title)
    : window_(nullptr)
{
    if (!glfwInit())
        throw std::runtime_error("GLFW init failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window_)
        throw std::runtime_error("Failed to create window");

    glfwMakeContextCurrent(window_);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("GLEW init failed");

    glfwSetFramebufferSizeCallback(window_, framebuffer_cb);

    info("Window created");
}

Window::~Window() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Window::should_close() const {
    return glfwWindowShouldClose(window_);
}

void Window::poll() {
    glfwPollEvents();
}

void Window::swap() {
    glfwSwapBuffers(window_);
}

GLFWwindow* Window::native() const {
    return window_;
}

}
