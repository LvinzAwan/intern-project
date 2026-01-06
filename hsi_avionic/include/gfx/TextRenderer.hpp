#pragma once
#include <glad/glad.h>
#include "gfx/Shader.hpp"

class TextRenderer {
public:
  bool init();
  void drawTextNDC(const char* text, float x_ndc, float y_ndc, float scale,
                   float r, float g, float b);
  void drawTextCenteredNDC(const char* text, float cx_ndc, float cy_ndc, float scale,
                         float r, float g, float b);


private:
  Shader shader_;
  GLuint vao_ = 0;
  GLuint vbo_ = 0;
};
