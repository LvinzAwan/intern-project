#pragma once
#include <string>

struct GLFWwindow;

namespace core {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool should_close() const;
    void poll();
    void swap();

    GLFWwindow* native() const;

private:
    GLFWwindow* window_;
};

}
