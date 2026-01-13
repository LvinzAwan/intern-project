#include "gfx/TtfTextRenderer.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>

//GL shader utils
static GLuint compileShader(GLenum type, const char* src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  GLint ok = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok) return shader;

  char log[1024];
  glGetShaderInfoLog(shader, 1024, nullptr, log);
  std::cerr << "TTF shader compile error:\n" << log << "\n";

  glDeleteShader(shader);
  return 0;
}

static bool readFileBytes(const std::string& path, std::vector<unsigned char>& out) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return false;

  f.seekg(0, std::ios::end);
  const auto size = static_cast<size_t>(f.tellg());
  f.seekg(0, std::ios::beg);

  out.resize(size);
  f.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(size));
  return true;
}

bool TtfTextRenderer::buildShader() {
  static const char* kVs = R"(
    #version 330 core
    layout (location=0) in vec2 aPos;
    layout (location=1) in vec2 aUV;
    out vec2 vUV;
    void main() {
      vUV = aUV;
      gl_Position = vec4(aPos, 0.0, 1.0);
    }
  )";

  static const char* kFs = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 FragColor;

    uniform sampler2D uTex;
    uniform vec3 uColor;

    void main() {
      float a = texture(uTex, vUV).r;   // atlas alpha stored in red channel
      FragColor = vec4(uColor, a);
    }
  )";

  const GLuint v = compileShader(GL_VERTEX_SHADER, kVs);
  const GLuint f = compileShader(GL_FRAGMENT_SHADER, kFs);
  if (!v || !f) return false;

  program_ = glCreateProgram();
  glAttachShader(program_, v);
  glAttachShader(program_, f);
  glLinkProgram(program_);

  glDeleteShader(v);
  glDeleteShader(f);

  GLint ok = 0;
  glGetProgramiv(program_, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetProgramInfoLog(program_, 1024, nullptr, log);
    std::cerr << "TTF program link error:\n" << log << "\n";

    glDeleteProgram(program_);
    program_ = 0;
    return false;
  }

  glUseProgram(program_);
  glUniform1i(glGetUniformLocation(program_, "uTex"), 0);
  uColor_ = glGetUniformLocation(program_, "uColor");
  return true;
}

bool TtfTextRenderer::init(const std::string& ttf_path, float pixel_height) {
  if (!buildShader()) return false;

  std::vector<unsigned char> ttf;
  if (!readFileBytes(ttf_path, ttf)) {
    std::cerr << "Failed to read TTF: " << ttf_path << "\n";
    return false;
  }

  std::vector<unsigned char> bitmap(atlas_w_ * atlas_h_, 0);

  stbtt_pack_context pc{};
  if (!stbtt_PackBegin(&pc, bitmap.data(), atlas_w_, atlas_h_, 0, 1, nullptr)) {
    std::cerr << "stbtt_PackBegin failed\n";
    return false;
  }

  stbtt_PackSetOversampling(&pc, 1, 1);

  stbtt_packedchar baked_ascii[95];
  stbtt_PackFontRange(&pc, ttf.data(), 0, pixel_height, 32, 95, baked_ascii);

  stbtt_packedchar baked_degree[1];
  stbtt_PackFontRange(&pc, ttf.data(), 0, pixel_height, 176, 1, baked_degree);

  stbtt_PackEnd(&pc);

  for (int i = 0; i < 95; ++i) {
    chars_[i] = {
      static_cast<float>(baked_ascii[i].x0),
      static_cast<float>(baked_ascii[i].y0),
      static_cast<float>(baked_ascii[i].x1),
      static_cast<float>(baked_ascii[i].y1),
      baked_ascii[i].xoff,
      baked_ascii[i].yoff,
      baked_ascii[i].xadvance
    };
  }

  chars_[144] = {
    static_cast<float>(baked_degree[0].x0),
    static_cast<float>(baked_degree[0].y0),
    static_cast<float>(baked_degree[0].x1),
    static_cast<float>(baked_degree[0].y1),
    baked_degree[0].xoff,
    baked_degree[0].yoff,
    baked_degree[0].xadvance
  };

  glGenTextures(1, &tex_);
  glBindTexture(GL_TEXTURE_2D, tex_);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas_w_, atlas_h_, 0,
               GL_RED, GL_UNSIGNED_BYTE, bitmap.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return true;
}

const TtfTextRenderer::BakedChar* TtfTextRenderer::getCharMetrics(unsigned char c) {
  if (c >= 32 && c <= 126) return &chars_[c - 32];
  if (c == 176) return &chars_[144];
  return nullptr;
}

void TtfTextRenderer::drawTextNDC(const std::string& text, float x_ndc, float y_ndc,
                                 float r, float g, float b) {
  if (!program_ || !tex_ || text.empty()) return;

  constexpr float kScale = 0.0020f;

  std::vector<float> verts;
  verts.reserve(text.size() * 6 * 4);

  float pen_x = 0.0f;

  for (unsigned char c : text) {
    const BakedChar* bc = getCharMetrics(c);
    if (!bc) continue;

    const float x0 = pen_x + bc->xoff;
    const float y0 = y_ndc - bc->yoff;
    const float x1 = x0 + (bc->x1 - bc->x0);
    const float y1 = y0 - (bc->y1 - bc->y0);

    const float X0 = x_ndc + x0 * kScale;
    const float Y0 = y_ndc + y0 * kScale;
    const float X1 = x_ndc + x1 * kScale;
    const float Y1 = y_ndc + y1 * kScale;

    const float U0 = bc->x0 / (float)atlas_w_;
    const float V0 = bc->y0 / (float)atlas_h_;
    const float U1 = bc->x1 / (float)atlas_w_;
    const float V1 = bc->y1 / (float)atlas_h_;

    verts.insert(verts.end(), { X0, Y0, U0, V0,  X1, Y0, U1, V0,  X1, Y1, U1, V1 });
    verts.insert(verts.end(), { X0, Y0, U0, V0,  X1, Y1, U1, V1,  X0, Y1, U0, V1 });

    pen_x += bc->xadvance;
  }

  if (verts.empty()) return;

  glUseProgram(program_);
  glUniform3f(uColor_, r, g, b);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               (GLsizeiptr)(verts.size() * sizeof(float)),
               verts.data(),
               GL_DYNAMIC_DRAW);

  glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 4));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void TtfTextRenderer::drawTextCenteredNDC(const std::string& text, float cx_ndc, float cy_ndc,
                                         float r, float g, float b) {
  if (text.empty()) return;

  float pen_x = 0.0f;
  float minx = 1e9f, miny = 1e9f;
  float maxx = -1e9f, maxy = -1e9f;

  for (unsigned char c : text) {
    const BakedChar* bc = getCharMetrics(c);
    if (!bc) continue;

    const float gx0 = pen_x + bc->xoff;
    const float gy0 = -bc->yoff;
    const float gx1 = gx0 + (bc->x1 - bc->x0);
    const float gy1 = gy0 - (bc->y1 - bc->y0);

    minx = std::min(minx, gx0);
    miny = std::min(miny, gy1);
    maxx = std::max(maxx, gx1);
    maxy = std::max(maxy, gy0);

    pen_x += bc->xadvance;
  }

  if (minx > maxx) return;

  constexpr float kScale = 0.0020f;

  const float w_ndc = (maxx - minx) * kScale;
  const float h_ndc = (maxy - miny) * kScale;

  const float x_ndc = cx_ndc - 0.5f * w_ndc - (minx * kScale);
  const float y_ndc = cy_ndc - 0.5f * h_ndc - (miny * kScale);

  drawTextNDC(text, x_ndc, y_ndc, r, g, b);
}

void TtfTextRenderer::drawTextCenteredNDCRotated(const char* text,
                                                float cx_ndc, float cy_ndc,
                                                float rotation_deg,
                                                float r, float g, float b) {
  if (!text || !*text || !program_ || !tex_) return;

  constexpr float kScale = 0.0020f;

  const float angle_rad = rotation_deg * 3.14159265359f / 180.0f;
  const float cos_a = std::cos(angle_rad);
  const float sin_a = std::sin(angle_rad);

  std::vector<float> verts;
  verts.reserve(std::strlen(text) * 6 * 4);

  float pen_x = 0.0f;
  float minx = 1e9f, miny = 1e9f;
  float maxx = -1e9f, maxy = -1e9f;

  for (const char* pc = text; *pc; ++pc) {
    const unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = getCharMetrics(c);
    if (!bc) continue;

    const float gx0 = pen_x + bc->xoff;
    const float gy0 = -bc->yoff;
    const float gx1 = gx0 + (bc->x1 - bc->x0);
    const float gy1 = gy0 - (bc->y1 - bc->y0);

    minx = std::min(minx, gx0);
    miny = std::min(miny, gy1);
    maxx = std::max(maxx, gx1);
    maxy = std::max(maxy, gy0);

    pen_x += bc->xadvance;
  }

  if (minx > maxx) return;

  const float offset_x = -(minx + maxx) * 0.5f;
  const float offset_y = -(miny + maxy) * 0.5f;

  pen_x = 0.0f;
  for (const char* pc = text; *pc; ++pc) {
    const unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = getCharMetrics(c);
    if (!bc) continue;

    const float x0 = pen_x + bc->xoff + offset_x;
    const float y0 = -bc->yoff + offset_y;
    const float x1 = x0 + (bc->x1 - bc->x0);
    const float y1 = y0 - (bc->y1 - bc->y0);

    const float U0 = bc->x0 / (float)atlas_w_;
    const float V0 = bc->y0 / (float)atlas_h_;
    const float U1 = bc->x1 / (float)atlas_w_;
    const float V1 = bc->y1 / (float)atlas_h_;

    const float corners[4][2] = {
      { x0, y0 }, { x1, y0 }, { x1, y1 }, { x0, y1 }
    };

    float p[4][2];
    for (int i = 0; i < 4; ++i) {
      const float fx = corners[i][0] * kScale;
      const float fy = corners[i][1] * kScale;

      const float rx = fx * cos_a - fy * sin_a;
      const float ry = fx * sin_a + fy * cos_a;

      p[i][0] = rx + cx_ndc;
      p[i][1] = ry + cy_ndc;
    }

    verts.insert(verts.end(), {
      p[0][0], p[0][1], U0, V0,
      p[1][0], p[1][1], U1, V0,
      p[2][0], p[2][1], U1, V1
    });
    verts.insert(verts.end(), {
      p[0][0], p[0][1], U0, V0,
      p[2][0], p[2][1], U1, V1,
      p[3][0], p[3][1], U0, V1
    });

    pen_x += bc->xadvance;
  }

  if (verts.empty()) return;

  glUseProgram(program_);
  glUniform3f(uColor_, r, g, b);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               (GLsizeiptr)(verts.size() * sizeof(float)),
               verts.data(),
               GL_DYNAMIC_DRAW);

  glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 4));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void TtfTextRenderer::drawTextLeftAligned(const char* text, float x, float y,
                                         float r, float g, float b) {
  if (!text || !*text || !program_ || !tex_) return;

  constexpr float kScale = 0.0020f;

  float pen_x = 0.0f;
  float minx = 1e9f, maxx = -1e9f;

  for (const char* pc = text; *pc; ++pc) {
    const unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = getCharMetrics(c);
    if (!bc) continue;

    const float gx0 = pen_x + bc->xoff;
    const float gx1 = gx0 + (bc->x1 - bc->x0);

    minx = std::min(minx, gx0);
    maxx = std::max(maxx, gx1);

    pen_x += bc->xadvance;
  }

  if (minx > maxx) return;

  const float x_ndc = x - (minx * kScale);
  drawTextNDC(std::string(text), x_ndc, y, r, g, b);
}

void TtfTextRenderer::drawTextRightAligned(const char* text, float x, float y,
                                          float r, float g, float b) {
  if (!text || !*text || !program_ || !tex_) return;

  constexpr float kScale = 0.0020f;

  float pen_x = 0.0f;
  float minx = 1e9f, maxx = -1e9f;

  for (const char* pc = text; *pc; ++pc) {
    const unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = getCharMetrics(c);
    if (!bc) continue;

    const float gx0 = pen_x + bc->xoff;
    const float gx1 = gx0 + (bc->x1 - bc->x0);

    minx = std::min(minx, gx0);
    maxx = std::max(maxx, gx1);

    pen_x += bc->xadvance;
  }

  if (minx > maxx) return;

  const float x_ndc = x - (maxx * kScale);
  drawTextNDC(std::string(text), x_ndc, y, r, g, b);
}
