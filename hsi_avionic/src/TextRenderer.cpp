#include "gfx/TextRenderer.hpp"

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb/stb_easy_font.h"

#include <vector>

bool TextRenderer::init() {
  // Vertex: vec2 position (NDC)
  const char* vs = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
  )";

  const char* fs = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 uColor;
    void main() { FragColor = vec4(uColor, 1.0); }
  )";

  if (!shader_.build(vs, fs)) return false;

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  return true;
}

void TextRenderer::drawTextNDC(const char* text, float x_ndc, float y_ndc, float scale,
                              float r, float g, float b) {
  // stb_easy_font bikin quad vertex dalam "pixel-ish" coords (0..)
  // Kita generate dulu, lalu kita ubah ke NDC manual.
  char buffer[99999]; // cukup untuk string pendek
  int quad_count = stb_easy_font_print(0.0f, 0.0f, (char*)text, nullptr, buffer, sizeof(buffer));

  // Format buffer: tiap vertex punya 4 float (x, y, z, w) -> kita pakai x,y saja
  // Tiap quad: 4 vertex, tapi output stb adalah list quad (4 verts per quad)
  const float* v = (const float*)buffer;

  // Konversi: koordinat stb (unit ~ pixel 1) kita scale jadi NDC.
  // Kita treat 1 unit stb = 1 "pixel" virtual, lalu kali scale ke NDC.
  // Biarkan sederhana: scale NDC langsung.
  std::vector<float> verts;
  verts.reserve(quad_count * 6 * 2); // 2 triangle per quad, 3 vertex per tri, 2 float

  for (int i = 0; i < quad_count; ++i) {
    // 4 vertex per quad
    // v0, v1, v2, v3
    float x0 = v[i * 16 + 0],  y0 = v[i * 16 + 1];
    float x1 = v[i * 16 + 4],  y1 = v[i * 16 + 5];
    float x2 = v[i * 16 + 8],  y2 = v[i * 16 + 9];
    float x3 = v[i * 16 + 12], y3 = v[i * 16 + 13];

    // ubah ke NDC + posisi
    auto to_ndc = [&](float px, float py) {
      float X = x_ndc + px * scale;
      float Y = y_ndc + py * scale;
      return std::pair<float,float>(X, Y);
    };

    auto p0 = to_ndc(x0, y0);
    auto p1 = to_ndc(x1, y1);
    auto p2 = to_ndc(x2, y2);
    auto p3 = to_ndc(x3, y3);

    // quad -> 2 triangle: (0,1,2) dan (0,2,3)
    verts.insert(verts.end(), {p0.first, p0.second, p1.first, p1.second, p2.first, p2.second});
    verts.insert(verts.end(), {p0.first, p0.second, p2.first, p2.second, p3.first, p3.second});
  }

  shader_.use();
  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, r, g, b);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_DYNAMIC_DRAW);

  glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 2));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void TextRenderer::drawTextCenteredNDC(const char* text, float cx_ndc, float cy_ndc, float scale,
                                      float r, float g, float b) {
  // stb_easy_font: lebar dalam "unit" font
  int w = stb_easy_font_width((char*)text);
  int h = 12; // tinggi standar stb_easy_font kira-kira 12 unit

  float x0 = cx_ndc - (w * scale) * 0.5f;
  float y0 = cy_ndc - (h * scale) * 0.5f;

  drawTextNDC(text, x0, y0, scale, r, g, b);
}

