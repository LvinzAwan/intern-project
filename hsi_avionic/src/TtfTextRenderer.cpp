#include "gfx/TtfTextRenderer.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>  // Tambahkan ini untuk std::strlen

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

  // Gunakan stbtt_PackBegin untuk lebih fleksibel
  stbtt_pack_context pc;
  if (!stbtt_PackBegin(&pc, bitmap.data(), atlas_w_, atlas_h_, 0, 1, nullptr)) {
    std::cerr << "stbtt_PackBegin failed\n";
    return false;
  }

  stbtt_PackSetOversampling(&pc, 1, 1);

  // Bake range 1: ASCII (32-126)
  stbtt_packedchar baked_ascii[95];
  stbtt_PackFontRange(&pc, ttf.data(), 0, pixel_height, 32, 95, baked_ascii);

  // Bake range 2: Simbol derajat (176-176)
  stbtt_packedchar baked_special[1];
  stbtt_PackFontRange(&pc, ttf.data(), 0, pixel_height, 176, 1, baked_special);

  stbtt_PackEnd(&pc);

  // Copy ASCII ke chars_ (index 0-94 untuk char 32-126)
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

  // Copy simbol derajat ke index 144 (char 176 - 32 = 144)
  chars_[144] = {
      static_cast<float>(baked_special[0].x0),
      static_cast<float>(baked_special[0].y0),
      static_cast<float>(baked_special[0].x1),
      static_cast<float>(baked_special[0].y1),
      baked_special[0].xoff,
      baked_special[0].yoff,
      baked_special[0].xadvance
  };

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

  float s = 0.0020f;

  std::vector<float> verts;
  verts.reserve(text.size() * 6 * 4);

  float x = 0.0f, y = 0.0f;
  for (unsigned char c : text) {
    const BakedChar* bc = nullptr;
    
    // Handle ASCII (32-126)
    if (c >= 32 && c <= 126) {
      bc = &chars_[c - 32];
    }
    // Handle simbol derajat (176)
    else if (c == 176) {
      bc = &chars_[144];  // Index 144 untuk derajat
    }
    else {
      continue;  // Skip karakter tidak dikenal
    }

    // posisi glyph dalam "pixel" space
    float x0 = x + bc->xoff;
    float y0 = y - bc->yoff;
    float x1 = x0 + (bc->x1 - bc->x0);
    float y1 = y0 - (bc->y1 - bc->y0);

    // convert ke NDC + anchor
    float X0 = x_ndc + x0 * s;
    float Y0 = y_ndc + y0 * s;
    float X1 = x_ndc + x1 * s;
    float Y1 = y_ndc + y1 * s;

    // UV
    float U0 = bc->x0 / (float)atlas_w_;
    float V0 = bc->y0 / (float)atlas_h_;
    float U1 = bc->x1 / (float)atlas_w_;
    float V1 = bc->y1 / (float)atlas_h_;

    // 2 tris
    verts.insert(verts.end(), { X0, Y0, U0, V0,  X1, Y0, U1, V0,  X1, Y1, U1, V1 });
    verts.insert(verts.end(), { X0, Y0, U0, V0,  X1, Y1, U1, V1,  X0, Y1, U0, V1 });

    x += bc->xadvance;
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

  for (unsigned char c : text) {
    const BakedChar* bc = nullptr;
    
    if (c >= 32 && c <= 126) {
      bc = &chars_[c - 32];
    } else if (c == 176) {
      bc = &chars_[144];
    } else {
      continue;
    }

    float gx0 = x + bc->xoff;
    float gy0 = 0.0f - bc->yoff;
    float gx1 = gx0 + (bc->x1 - bc->x0);
    float gy1 = gy0 - (bc->y1 - bc->y0);

    if (gx0 < minx) minx = gx0;
    if (gy1 < miny) miny = gy1;
    if (gx1 > maxx) maxx = gx1;
    if (gy0 > maxy) maxy = gy0;

    x += bc->xadvance;
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

void TtfTextRenderer::drawTextCenteredNDCRotated(const char* text,
                                                  float cx_ndc, float cy_ndc,
                                                  float rotation_deg,
                                                  float r, float g, float b) {
  if (!program_ || !tex_) return;

  // Konversi rotasi ke radian
  float angle_rad = rotation_deg * 3.14159265359f / 180.0f;
  float cos_a = std::cos(angle_rad);
  float sin_a = std::sin(angle_rad);

  float s = 0.0020f;  // Tambahkan semicolon di sini!

  std::vector<float> verts;
  verts.reserve(std::strlen(text) * 6 * 4);

  // Hitung bounding box untuk centering
  float x = 0.0f;
  float minx =  1e9f, miny =  1e9f;
  float maxx = -1e9f, maxy = -1e9f;

  for (const char* pc = text; *pc; ++pc) {
    unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = nullptr;
    
    if (c >= 32 && c <= 126) {
      bc = &chars_[c - 32];
    } else if (c == 176) {
      bc = &chars_[144];
    } else {
      continue;
    }

    float gx0 = x + bc->xoff;
    float gy0 = 0.0f - bc->yoff;
    float gx1 = gx0 + (bc->x1 - bc->x0);
    float gy1 = gy0 - (bc->y1 - bc->y0);

    if (gx0 < minx) minx = gx0;
    if (gy1 < miny) miny = gy1;
    if (gx1 > maxx) maxx = gx1;
    if (gy0 > maxy) maxy = gy0;

    x += bc->xadvance;
  }

  if (minx > maxx) return; // string kosong

  // Offset untuk centering (dalam font units)
  float offset_x = -(minx + maxx) * 0.5f;
  float offset_y = -(miny + maxy) * 0.5f;

  // Render dengan rotasi
  x = 0.0f;
  for (const char* pc = text; *pc; ++pc) {
    unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = nullptr;
    
    if (c >= 32 && c <= 126) {
      bc = &chars_[c - 32];
    } else if (c == 176) {
      bc = &chars_[144];
    } else {
      continue;
    }

    // Posisi glyph dalam font units (dengan offset untuk centering)
    float x0 = x + bc->xoff + offset_x;
    float y0 = 0.0f - bc->yoff + offset_y;
    float x1 = x0 + (bc->x1 - bc->x0);
    float y1 = y0 - (bc->y1 - bc->y0);

    // UV coordinates
    float U0 = bc->x0 / (float)atlas_w_;
    float V0 = bc->y0 / (float)atlas_h_;
    float U1 = bc->x1 / (float)atlas_w_;
    float V1 = bc->y1 / (float)atlas_h_;

    // 4 corners (dalam font units, belum di-scale)
    float corners[4][2] = {
      { x0, y0 },  // top-left
      { x1, y0 },  // top-right
      { x1, y1 },  // bottom-right
      { x0, y1 }   // bottom-left
    };

    // Rotasi dan scale setiap corner, lalu translate ke posisi akhir
    float rotated[4][2];
    for (int i = 0; i < 4; i++) {
      float fx = corners[i][0] * s;
      float fy = corners[i][1] * s;
      
      // Rotasi
      float rx = fx * cos_a - fy * sin_a;
      float ry = fx * sin_a + fy * cos_a;
      
      // Translate ke center
      rotated[i][0] = rx + cx_ndc;
      rotated[i][1] = ry + cy_ndc;
    }

    // Build triangles: (0,1,2) dan (0,2,3)
    verts.insert(verts.end(), { 
      rotated[0][0], rotated[0][1], U0, V0,
      rotated[1][0], rotated[1][1], U1, V0,
      rotated[2][0], rotated[2][1], U1, V1
    });
    verts.insert(verts.end(), { 
      rotated[0][0], rotated[0][1], U0, V0,
      rotated[2][0], rotated[2][1], U1, V1,
      rotated[3][0], rotated[3][1], U0, V1
    });

    x += bc->xadvance;
  }

  // Render
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

void TtfTextRenderer::drawTextLeftAligned(const char* text, float x, float y, 
                                         float r, float g, float b) {
  if (!text || !program_ || !tex_) return;

  // Hitung bounding box untuk mengetahui lebar text
  float s = 0.0020f;
  float x_pos = 0.0f;
  float minx = 1e9f, maxx = -1e9f;

  for (const char* pc = text; *pc; ++pc) {
    unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = nullptr;
    
    if (c >= 32 && c <= 126) {
      bc = &chars_[c - 32];
    } else if (c == 176) {
      bc = &chars_[144];
    } else {
      continue;
    }

    float gx0 = x_pos + bc->xoff;
    float gx1 = gx0 + (bc->x1 - bc->x0);

    if (gx0 < minx) minx = gx0;
    if (gx1 > maxx) maxx = gx1;

    x_pos += bc->xadvance;
  }

  if (minx > maxx) return; // string kosong

  // x adalah posisi kiri, jadi offset = minx * s
  float x_ndc = x + (minx * s);
  
  drawTextNDC(text, x_ndc, y, r, g, b);
}

void TtfTextRenderer::drawTextRightAligned(const char* text, float x, float y, 
                                          float r, float g, float b) {
  if (!text || !program_ || !tex_) return;

  float s = 0.0020f;
  float x_pos = 0.0f;
  float minx = 1e9f, maxx = -1e9f;

  // Hitung bounding box untuk mengetahui lebar text
  for (const char* pc = text; *pc; ++pc) {
    unsigned char c = (unsigned char)*pc;
    const BakedChar* bc = nullptr;
    
    if (c >= 32 && c <= 126) {
      bc = &chars_[c - 32];
    } else if (c == 176) {
      bc = &chars_[144];
    } else {
      continue;
    }

    float gx0 = x_pos + bc->xoff;
    float gx1 = gx0 + (bc->x1 - bc->x0);

    if (gx0 < minx) minx = gx0;
    if (gx1 > maxx) maxx = gx1;

    x_pos += bc->xadvance;
  }

  if (minx > maxx) return; // string kosong

  // x adalah posisi kanan, jadi geser ke kiri sejauh (maxx * s)
  float x_ndc = x - (maxx * s);
  
  drawTextNDC(std::string(text), x_ndc, y, r, g, b);
}

