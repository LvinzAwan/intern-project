cmake_minimum_required(VERSION 3.16)
project(ogl_triangle2d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

add_library(core
    src/core/Window.cpp
    src/core/Log.cpp
)

target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC glfw OpenGL::GL GLEW::GLEW)

add_executable(app
    src/app/main.cpp
    src/app/TriangleApp.cpp
)

target_link_libraries(app PRIVATE core)
