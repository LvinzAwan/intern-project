#include "gfx/TtfTextRenderer.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include <fstream>
#include <vector>
#include <iostream>

// ---------------- shader helpers ----------------
static GLuint compile(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(s, 1024, nullptr, log);
    std::cerr << "TTF shader compile error:\n" << log << "\n";
    glDeleteShader(s);
    return 0;
  }
  return s;
}

bool TtfTextRenderer::buildShader() {
  const char* vs = R"(
    #version 330 core
    layout (location=0) in vec2 aPos;
    layout (location=1) in vec2 aUV;
    out vec2 vUV;
    void main() {
      vUV = aUV;
      gl_Position = vec4(aPos, 0.0, 1.0);
    }
  )";

  const char* fs = R"(
    #version 330 core
    in vec2 vUV;
    out vec4 FragColor;

    uniform sampler2D uTex;
    uniform vec3 uColor;

    void main() {
      // atlas adalah alpha (coverage) di channel merah
      float a = texture(uTex, vUV).r;
      FragColor = vec4(uColor, a);
    }
  )";

  GLuint v = compile(GL_VERTEX_SHADER, vs);
  GLuint f = compile(GL_FRAGMENT_SHADER, fs);
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

// ---------------- init: load TTF, bake atlas, create texture ----------------
static bool read_file_bytes(const std::string& path, std::vector<unsigned char>& out) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return false;
  f.seekg(0, std::ios::end);
  size_t sz = (size_t)f.tellg();
  f.seekg(0, std::ios::beg);
  out.resize(sz);
  f.read((char*)out.data(), (std::streamsize)sz);
  return true;
}

bool TtfTextRenderer::init(const std::string& ttf_path, float pixel_height) {
  if (!buildShader()) return false;

  std::vector<unsigned char> ttf;
  if (!read_file_bytes(ttf_path, ttf)) {
    std::cerr << "Failed to read TTF: " << ttf_path << "\n";
    return false;
  }

  // bake atlas grayscale
  std::vector<unsigned char> bitmap(atlas_w_ * atlas_h_, 0);

  // stb_truetype punya fungsi bake yang enak buat ASCII basic
  // Kita simpan hasilnya ke chars_ tapi perlu adapt struct sesuai stb
  stbtt_bakedchar baked[96];
  int res = stbtt_BakeFontBitmap(ttf.data(), 0, pixel_height,
                                 bitmap.data(), atlas_w_, atlas_h_,
                                 32, 96, baked);
  if (res <= 0) {
    std::cerr << "stbtt_BakeFontBitmap failed (atlas too small?)\n";
    return false;
  }

  // copy ke struct lokal (biar header kita gak tergantung tipe stb)
  for (int i = 0; i < 96; ++i) {
    chars_[i] = {
        static_cast<float>(baked[i].x0),
        static_cast<float>(baked[i].y0),
        static_cast<float>(baked[i].x1),
        static_cast<float>(baked[i].y1),
        baked[i].xoff,
        baked[i].yoff,
        baked[i].xadvance
    };
  }

  // create texture (single channel)
  glGenTextures(1, &tex_);
  glBindTexture(GL_TEXTURE_2D, tex_);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas_w_, atlas_h_, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  // geometry buffers (dynamic)
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

  // pos (2) + uv (2)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return true;
}

// ---------------- draw ----------------
void TtfTextRenderer::drawTextNDC(const std::string& text, float x_ndc, float y_ndc,
                                 float r, float g, float b) {
  if (!program_ || !tex_) return;

  // Kita gambar dalam NDC langsung.
  // Anggap 1 pixel atlas ~ 1 unit "pixel" di layar virtual.
  // Untuk NDC, kita perlu skala. Biar simple: pakai faktor kecil.
  // Nanti kita kalibrasi di step berikutnya.
  float s = 0.0020f; // nanti kamu adjust biar pas

  std::vector<float> verts;
  verts.reserve(text.size() * 6 * 4);

  float x = 0.0f, y = 0.0f;
  for (char c : text) {
    if (c < 32 || c > 127) continue;
    const BakedChar& bc = chars_[c - 32];

    // posisi glyph dalam "pixel" space
    float x0 = x + bc.xoff;
    float y0 = y - bc.yoff; // perhatikan: stb yoff ke atas
    float x1 = x0 + (bc.x1 - bc.x0);
    float y1 = y0 - (bc.y1 - bc.y0);

    // convert ke NDC + anchor
    float X0 = x_ndc + x0 * s;
    float Y0 = y_ndc + y0 * s;
    float X1 = x_ndc + x1 * s;
    float Y1 = y_ndc + y1 * s;

    // UV
    float U0 = bc.x0 / (float)atlas_w_;
    float V0 = bc.y0 / (float)atlas_h_;
    float U1 = bc.x1 / (float)atlas_w_;
    float V1 = bc.y1 / (float)atlas_h_;

    // 2 tris
    // (X0,Y0)-(X1,Y0)-(X1,Y1) and (X0,Y0)-(X1,Y1)-(X0,Y1)
    verts.insert(verts.end(), { X0, Y0, U0, V0,  X1, Y0, U1, V0,  X1, Y1, U1, V1 });
    verts.insert(verts.end(), { X0, Y0, U0, V0,  X1, Y1, U1, V1,  X0, Y1, U0, V1 });

    x += bc.xadvance;
  }

  glUseProgram(program_);
  glUniform3f(uColor_, r, g, b);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_DYNAMIC_DRAW);

  glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(verts.size() / 4));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void TtfTextRenderer::drawTextCenteredNDC(const std::string& text, float cx_ndc, float cy_ndc,
                                         float r, float g, float b) {
  // hitung bounding box dalam "font units" (pakai baked metrics)
  float x = 0.0f;
  float minx =  1e9f, miny =  1e9f;
  float maxx = -1e9f, maxy = -1e9f;

  for (char c : text) {
    if (c < 32 || c > 127) continue;
    const BakedChar& bc = chars_[c - 32];

    float gx0 = x + bc.xoff;
    float gy0 = 0.0f - bc.yoff;
    float gx1 = gx0 + (bc.x1 - bc.x0);
    float gy1 = gy0 - (bc.y1 - bc.y0);

    if (gx0 < minx) minx = gx0;
    if (gy1 < miny) miny = gy1;
    if (gx1 > maxx) maxx = gx1;
    if (gy0 > maxy) maxy = gy0;

    x += bc.xadvance;
  }

  if (minx > maxx) {
    // string kosong / karakter invalid
    return;
  }

  // Faktor skala NDC yang sekarang kamu pakai di drawTextNDC
  float s = 0.0020f; // HARUS sama dengan yang dipakai di drawTextNDC

  float w_ndc = (maxx - minx) * s;
  float h_ndc = (maxy - miny) * s;

  // kita geser supaya bounding box center tepat di (cx, cy)
  float x_ndc = cx_ndc - w_ndc * 0.5f - (minx * s);
  float y_ndc = cy_ndc - h_ndc * 0.5f - (maxy * s);

  drawTextNDC(text, x_ndc, y_ndc, r, g, b);
}

