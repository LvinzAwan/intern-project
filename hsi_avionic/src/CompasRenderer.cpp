#include "compas/CompasRenderer.hpp"
#include <vector>
#include <cmath>

static float deg2rad(float d) { return d * 3.1415926535f / 180.0f; }

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

  buildRingGeometry(0.70f, 200);

  tick_outer_r_ = 0.70f;
  tick_inner_r_90_ = 0.10f;
  tick_inner_r_30_ = 0.08f;
  tick_inner_r_10_ = 0.06f;
  tick_inner_r_5_  = 0.04f;
  
  buildTicksGeometry(tick_outer_r_, tick_inner_r_90_, tick_inner_r_30_, tick_inner_r_10_, tick_inner_r_5_);
  buildCardinalMarkersGeometry(0.70f, 0.06f);
  buildHeadingIndicatorGeometry();

  return true;
}

void CompasRenderer::buildRingGeometry(float radius_ndc, int segments) {
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
                                        float len_cardinal,
                                        float len_major, 
                                        float len_medium, 
                                        float len_minor) {
  float aspect_fix = (float)height_ / (float)width_;

  std::vector<float> v_cardinal, v_major, v_medium, v_minor;
  v_cardinal.reserve(240);
  v_major.reserve(360); 
  v_medium.reserve(720); 
  v_minor.reserve(1440);

  auto push_tick = [&](std::vector<float>& v, int deg, float len) {
    float angle = deg - heading_deg_;
    float a = angle * 3.1415926535f / 180.0f;

    float x0 = std::cos(a) * radius_ndc * aspect_fix;
    float y0 = std::sin(a) * radius_ndc;

    float x1 = std::cos(a) * (radius_ndc - len) * aspect_fix;
    float y1 = std::sin(a) * (radius_ndc - len);

    v.push_back(x0); v.push_back(y0);
    v.push_back(x1); v.push_back(y1);
  };

  for (int deg = 0; deg < 360; ++deg) {
    if (deg % 90 == 0) {
      push_tick(v_cardinal, deg, len_cardinal);
    } else if (deg % 30 == 0) {
      push_tick(v_major, deg, len_major);
    } else if (deg % 10 == 0) {
      push_tick(v_medium, deg, len_medium);
    } else if (deg % 5 == 0) {
      push_tick(v_minor, deg, len_minor);
    }
  }

  cardinal_count_ = (int)(v_cardinal.size() / 2);
  major_count_  = (int)(v_major.size() / 2);
  medium_count_ = (int)(v_medium.size() / 2);
  minor_count_  = (int)(v_minor.size() / 2);

  auto upload = [](GLuint& vao, GLuint& vbo, const std::vector<float>& v) {
    if (vao != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(v.size() * sizeof(float)), v.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  };

  upload(cardinal_vao_, cardinal_vbo_, v_cardinal);
  upload(major_vao_, major_vbo_, v_major);
  upload(medium_vao_, medium_vbo_, v_medium);
  upload(minor_vao_, minor_vbo_, v_minor);
}

void CompasRenderer::buildCardinalMarkersGeometry(float radius_ndc, float size_ndc) {
  float aspect_fix = (float)height_ / (float)width_;

  std::vector<float> verts;
  verts.reserve(4 * 3 * 2);

  auto add_triangle_inward = [&](float bearing_deg) {
    float angle = bearing_deg - heading_deg_;
    float a = angle * 3.1415926535f / 180.0f;
    
    float cos_a = std::cos(a);
    float sin_a = std::sin(a);

    float cx = cos_a * radius_ndc * aspect_fix;
    float cy = sin_a * radius_ndc;

    float inx = -cos_a;
    float iny = -sin_a;

    float x0 = cx + inx * size_ndc * aspect_fix;
    float y0 = cy + iny * size_ndc;

    float tx = -sin_a;
    float ty = cos_a;

    float half = size_ndc * 0.6f;

    float x1 = cx + tx * half * aspect_fix;
    float y1 = cy + ty * half;

    float x2 = cx - tx * half * aspect_fix;
    float y2 = cy - ty * half;

    verts.insert(verts.end(), {x0, y0, x1, y1, x2, y2});
  };

  add_triangle_inward(0.0f);    // N
  add_triangle_inward(90.0f);   // E
  add_triangle_inward(180.0f);  // S
  add_triangle_inward(270.0f);  // W

  markers_vertex_count_ = (int)(verts.size() / 2);

  if (markers_vao_ != 0) glDeleteBuffers(1, &markers_vbo_);
  if (markers_vao_ != 0) glDeleteVertexArrays(1, &markers_vao_);

  glGenVertexArrays(1, &markers_vao_);
  glGenBuffers(1, &markers_vbo_);

  glBindVertexArray(markers_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, markers_vbo_);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void CompasRenderer::buildHeadingIndicatorGeometry() {
  float aspect_fix = (float)height_ / (float)width_;
  
  std::vector<float> verts;
  
  // Parameter untuk penanda heading
  float indicator_y = 0.77f;  // Posisi Y
  float arrow_height = 0.12f;  // Tinggi panah
  float arrow_width = 0.06f;   // Lebar panah
  
  // Segitiga pointing down (panah utama)
  float x_center = 0.0f;
  float x_left = -arrow_width * aspect_fix;
  float x_right = arrow_width * aspect_fix;
  
  float y_top = indicator_y;
  float y_bottom = indicator_y - arrow_height;
  
  // Vertex segitiga
  verts.push_back(x_center);
  verts.push_back(y_bottom);
  verts.push_back(x_left);
  verts.push_back(y_top);
  verts.push_back(x_right);
  verts.push_back(y_top);
  
  heading_indicator_vertex_count_ = 3;
  
  if (heading_indicator_vao_ != 0) glDeleteBuffers(1, &heading_indicator_vbo_);
  if (heading_indicator_vao_ != 0) glDeleteVertexArrays(1, &heading_indicator_vao_);
  
  glGenVertexArrays(1, &heading_indicator_vao_);
  glGenBuffers(1, &heading_indicator_vbo_);
  
  glBindVertexArray(heading_indicator_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, heading_indicator_vbo_);
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
  buildTicksGeometry(tick_outer_r_, tick_inner_r_90_, tick_inner_r_30_, tick_inner_r_10_, tick_inner_r_5_);

  shader_.use();

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);

  glBindVertexArray(cardinal_vao_);
  glLineWidth(5.0f);
  glDrawArrays(GL_LINES, 0, cardinal_count_);

  glBindVertexArray(major_vao_);
  glLineWidth(3.5f);
  glDrawArrays(GL_LINES, 0, major_count_);

  glBindVertexArray(medium_vao_);
  glLineWidth(2.0f);
  glDrawArrays(GL_LINES, 0, medium_count_);

  glBindVertexArray(minor_vao_);
  glLineWidth(1.0f);
  glDrawArrays(GL_LINES, 0, minor_count_);

  glBindVertexArray(0);
}

void CompasRenderer::drawCardinalMarkers() {
  buildCardinalMarkersGeometry(0.70f, 0.06f);

  shader_.use();
  glBindVertexArray(markers_vao_);

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);

  glDrawArrays(GL_TRIANGLES, 0, markers_vertex_count_);

  glBindVertexArray(0);
}

void CompasRenderer::drawHeadingIndicator() {
  shader_.use();
  glBindVertexArray(heading_indicator_vao_);

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 0.0f);  // Yellow

  glDrawArrays(GL_TRIANGLES, 0, heading_indicator_vertex_count_);

  glBindVertexArray(0);
}




