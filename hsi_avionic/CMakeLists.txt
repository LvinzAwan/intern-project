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

# GLAD Library
add_library(glad STATIC
  lib/glad/src/glad.c
)

target_include_directories(glad PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include
)

# Source files
set(SOURCE_FILES
  src/main.cpp
  src/gfx/Shader.cpp
  src/compas/CompasRenderer.cpp
  src/gfx/TtfTextRenderer.cpp
  src/gfx/HsiRenderer.cpp
  src/ui/HsiUiRenderer.cpp
  src/core/InputHandler.cpp
  src/core/RenderEngine.cpp
)

# Header files
set(HEADER_FILES
  include/gfx/Shader.hpp
  include/gfx/TtfTextRenderer.hpp
  include/gfx/HsiRenderer.hpp
  include/compas/CompasRenderer.hpp
  include/ui/HsiUiRenderer.hpp
  include/data/HsiData.hpp
  include/core/ApplicationState.hpp
  include/core/InputHandler.hpp
  include/core/RenderEngine.hpp
)

# ==================== COMPILER OPTIONS ====================
add_compile_options(-Wno-unused-variable)

if(MSVC)
  target_compile_options(hsi_avionic PRIVATE /W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==================== CREATE EXECUTABLE ====================
add_executable(hsi_avionic ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(hsi_avionic PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Link libraries
target_link_libraries(hsi_avionic PRIVATE glfw glad)