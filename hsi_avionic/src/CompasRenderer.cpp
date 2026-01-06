#include "compas/CompasRenderer.hpp"
#include <vector>
#include <cmath>

static float deg2rad(float d) { return d * 3.1415926535f / 180.0f; }

static void cardToNDC(float deg_on_card,
                      float radius,
                      float heading_deg,
                      float aspect_fix,
                      float& out_x,
                      float& out_y) {
  // 1) card berputar kebalikan heading
  float a = deg2rad(deg_on_card - heading_deg);

  // 2) titik lingkaran (tanpa aspect dulu)
  float x = std::cos(a) * radius;
  float y = std::sin(a) * radius;

  // 3) baru apply aspect
  x *= aspect_fix;

  out_x = x;
  out_y = y;
}

bool CompasRenderer::init(int width, int height) {
  width_ = width;
  height_ = height;

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

  // Ring dulu: radius 0.70 NDC, 200 segmen biar halus
  buildRingGeometry(0.70f, 200);
//   buildTicksGeometry(
//     0.70f,  // sama dengan radius ring
//     0.12f,  // major (tiap 30°) lebih panjang
//     0.08f,  // medium (tiap 10°)
//     0.05f   // minor (tiap 5°)
//   );

  tick_outer_r_ = 0.70f;
  tick_inner_r_30_ = 0.12f;
  tick_inner_r_10_ = 0.08f;
  tick_inner_r_5_  = 0.05f;
  rebuildTicks();
  
  buildCardinalMarkersGeometry(0.70f, 0.06f);

  return true;
}

void CompasRenderer::buildRingGeometry(float radius_ndc, int segments) {
  // Biar lingkaran nggak jadi oval di 800x600, kita koreksi aspect:
  // x dikali (height/width)
  float aspect_fix = (float)height_ / (float)width_;

  std::vector<float> verts;
  verts.reserve((segments + 1) * 2);

  for (int i = 0; i < segments; i++) {
    float t = (float)i / (float)segments;
    float a = t * 2.0f * 3.1415926535f;

    float x = std::cos(a) * radius_ndc * aspect_fix;
    float y = std::sin(a) * radius_ndc;

    verts.push_back(x);
    verts.push_back(y);
  }

  vertex_count_ = segments;

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void CompasRenderer::buildTicksGeometry(float radius_ndc,
                                        float len_major, float len_medium, float len_minor) {
  float aspect_fix = (float)height_ / (float)width_;

  std::vector<float> v_major, v_medium, v_minor;
  v_major.reserve(360); v_medium.reserve(720); v_minor.reserve(1440);

  auto push_tick = [&](std::vector<float>& v, int deg, float len) {
    float a = (deg - heading_deg_)  * 3.1415926535f / 180.0f;

    float x0 = std::cos(a) * radius_ndc * aspect_fix;
    float y0 = std::sin(a) * radius_ndc;

    float x1 = std::cos(a) * (radius_ndc - len) * aspect_fix;
    float y1 = std::sin(a) * (radius_ndc - len);

    v.push_back(x0); v.push_back(y0);
    v.push_back(x1); v.push_back(y1);
  };

  for (int deg = 0; deg < 360; ++deg) {
    if (deg % 30 == 0) {
      push_tick(v_major, deg, len_major);
    } else if (deg % 10 == 0) {
      push_tick(v_medium, deg, len_medium);
    } else if (deg % 5 == 0) {
      push_tick(v_minor, deg, len_minor);
    }
  }

  major_count_  = (int)(v_major.size() / 2);
  medium_count_ = (int)(v_medium.size() / 2);
  minor_count_  = (int)(v_minor.size() / 2);

  auto upload = [](GLuint& vao, GLuint& vbo, const std::vector<float>& v) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size() * sizeof(float)), v.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  };

  upload(major_vao_, major_vbo_, v_major);
  upload(medium_vao_, medium_vbo_, v_medium);
  upload(minor_vao_, minor_vbo_, v_minor);
}

void CompasRenderer::buildCardinalMarkersGeometry(float radius_ndc, float size_ndc) {
  float aspect_fix = (float)height_ / (float)width_;

  // 4 segitiga, masing-masing 3 vertex, tiap vertex 2 float
  std::vector<float> verts;
  verts.reserve(4 * 3 * 2);

  auto add_triangle_inward = [&](float nx, float ny) {
    // (nx, ny) itu arah unit (contoh north = (0,1))
    // Pusat segitiga kita taruh di ring
    float cx = nx * radius_ndc * aspect_fix;
    float cy = ny * radius_ndc;

    // arah ke dalam = -n
    float inx = -nx;
    float iny = -ny;

    // v0: ujung segitiga mengarah ke dalam (tip)
    float x0 = cx + inx * size_ndc * aspect_fix;
    float y0 = cy + iny * size_ndc;

    // v1 & v2: basis segitiga (pakai vektor tangensial)
    // t = (-ny, nx)
    float tx = -ny;
    float ty = nx;

    float half = size_ndc * 0.6f;

    float x1 = cx + tx * half * aspect_fix;
    float y1 = cy + ty * half;

    float x2 = cx - tx * half * aspect_fix;
    float y2 = cy - ty * half;

    verts.insert(verts.end(), {x0, y0, x1, y1, x2, y2});
  };

  // North, East, South, West
  add_triangle_inward(0.0f, 1.0f);   // 0° (atas)
  add_triangle_inward(1.0f, 0.0f);   // 90° (kanan)
  add_triangle_inward(0.0f, -1.0f);  // 180° (bawah)
  add_triangle_inward(-1.0f, 0.0f);  // 270° (kiri)

  markers_vertex_count_ = (int)(verts.size() / 2);

  glGenVertexArrays(1, &markers_vao_);
  glGenBuffers(1, &markers_vbo_);

  glBindVertexArray(markers_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, markers_vbo_);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void CompasRenderer::drawRing() {
  shader_.use();
  glBindVertexArray(vao_);

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);

  glLineWidth(10.0f);
  glDrawArrays(GL_LINE_LOOP, 0, vertex_count_);

  glBindVertexArray(0);
}

void CompasRenderer::drawTicks() {
  shader_.use();

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);
//   rebuildTicks();

  // Major: 30°
  glBindVertexArray(major_vao_);
  glLineWidth(4.0f);
  glDrawArrays(GL_LINES, 0, major_count_);

  // Medium: 10°
  glBindVertexArray(medium_vao_);
  glLineWidth(2.5f);
  glDrawArrays(GL_LINES, 0, medium_count_);

  // Minor: 5°
  glBindVertexArray(minor_vao_);
  glLineWidth(1.5f);
  glDrawArrays(GL_LINES, 0, minor_count_);

  glBindVertexArray(0);
}

void CompasRenderer::drawCardinalMarkers() {
  shader_.use();
  glBindVertexArray(markers_vao_);

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);

  glDrawArrays(GL_TRIANGLES, 0, markers_vertex_count_);

  glBindVertexArray(0);
}

void CompasRenderer::rebuildTicks() {
  buildTicksGeometry(tick_outer_r_, tick_inner_r_30_, tick_inner_r_10_, tick_inner_r_5_);
}




