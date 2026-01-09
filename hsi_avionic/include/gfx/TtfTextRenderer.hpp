#pragma once
#include <glad/glad.h>
#include <string>

class TtfTextRenderer {
private:
  struct BakedChar {
    float x0, y0, x1, y1;
    float xoff, yoff, xadvance;
  };

  GLuint program_ = 0;
  GLuint uColor_ = 0;
  GLuint tex_ = 0;
  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  
  static constexpr int atlas_w_ = 512;
  static constexpr int atlas_h_ = 512;
  BakedChar chars_[256];

  bool buildShader();
  const BakedChar* getCharMetrics(unsigned char c);

public:
  bool init(const std::string& ttf_path, float pixel_height);

  void drawTextNDC(const std::string& text, float x_ndc, float y_ndc,
                  float r, float g, float b);
  void drawTextCenteredNDC(const std::string& text, float cx_ndc, float cy_ndc,
                          float r, float g, float b);
  void drawTextCenteredNDCRotated(const char* text, float cx_ndc, float cy_ndc,
                                 float rotation_deg, float r, float g, float b);
  void drawTextLeftAligned(const char* text, float x, float y,
                          float r, float g, float b);
  void drawTextRightAligned(const char* text, float x, float y,
                           float r, float g, float b);
};
