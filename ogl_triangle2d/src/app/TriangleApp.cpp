#include "app/TriangleApp.hpp"
#include "core/Window.hpp"
#include "core/Log.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

static GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

namespace app {

int run() {
    core::Window window(800, 600, "OpenGL Triangle");

    const char* vs = R"(
        #version 330 core
        layout (location = 0) in vec2 pos;
        void main() { gl_Position = vec4(pos, 0.0, 1.0); }
    )";

    const char* fs = R"(
        #version 330 core
        out vec4 color;
        void main() { color = vec4(0.9, 0.3, 0.2, 1.0); }
    )";

    GLuint program = glCreateProgram();
    glAttachShader(program, compile(GL_VERTEX_SHADER, vs));
    glAttachShader(program, compile(GL_FRAGMENT_SHADER, fs));
    glLinkProgram(program);

    float vertices[] = {
        -0.6f, -0.4f,
         0.6f, -0.4f,
         0.0f,  0.6f
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    while (!window.should_close()) {
        if (glfwGetKey(window.native(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.swap();
        window.poll();
    }

    return 0;
}

}
