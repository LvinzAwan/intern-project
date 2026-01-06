cmake_minimum_required(VERSION 3.16)
project(hsi_avionic LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.9
)
FetchContent_MakeAvailable(glfw)

add_library(glad STATIC
  lib/glad/src/glad.c
)

target_include_directories(glad PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include
)

add_executable(hsi_avionic
  src/main.cpp
  src/Shader.cpp
  src/CompasRenderer.cpp
  src/TextRenderer.cpp
  src/TtfTextRenderer.cpp
)

target_include_directories(hsi_avionic PRIVATE include lib)
target_link_libraries(hsi_avionic PRIVATE glfw glad)
