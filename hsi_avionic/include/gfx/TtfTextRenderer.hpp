#pragma once
#include <glad/glad.h>
#include <string>

class TtfTextRenderer {
public:
  bool init(const std::string& ttf_path, float pixel_height = 48.0f);
  void drawTextNDC(const std::string& text, float x_ndc, float y_ndc,
                   float r, float g, float b);
  void drawTextCenteredNDC(const std::string& text, float cx_ndc, float cy_ndc,
                         float r, float g, float b);
  void drawTextCenteredNDCRotated(const char* text, 
                                   float x, float y, 
                                   float rotation_deg,
                                   float r, float g, float b);
  void drawTextLeftAligned(const char* text, float x, float y, 
                          float r, float g, float b);
  void drawTextRightAligned(const char* text, float x, float y, 
                           float r, float g, float b);


private:
  // Font atlas
  GLuint tex_ = 0;
  int atlas_w_ = 512;
  int atlas_h_ = 512;

  // stb baked chars
  struct BakedChar { float x0,y0,x1,y1; float xoff,yoff,xadvance; };
  BakedChar chars_[96]{}; // ASCII 32..127

  // GL objects
  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  GLuint program_ = 0;

  // uniform locations
  GLint uColor_ = -1;

  bool buildShader();
};
