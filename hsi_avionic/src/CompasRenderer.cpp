#include "compas/CompasRenderer.hpp"
#include <vector>
#include <cmath>

bool CompasRenderer::init(int width, int height) {
  width_ = width;
  height_ = height;

  // Setup vertex and fragment shaders
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

  // Build compass ring geometry
  buildRingGeometry(0.70f, 200);

  // Initialize tick mark radii (outer to inner)
  tick_outer_r_ = 0.70f;
  tick_inner_r_90_ = 0.10f;   // 90 degree ticks
  tick_inner_r_30_ = 0.08f;   // 30 degree ticks
  tick_inner_r_10_ = 0.06f;   // 10 degree ticks
  tick_inner_r_5_  = 0.04f;   // 5 degree ticks
  
  buildTicksGeometry(tick_outer_r_, tick_inner_r_90_, tick_inner_r_30_, tick_inner_r_10_, tick_inner_r_5_);
  buildCardinalMarkersGeometry(0.70f, 0.06f);
  buildHeadingIndicatorGeometry();

  return true;
}

/**
 * Build circular ring geometry for compass border
 */
void CompasRenderer::buildRingGeometry(float radius_ndc, int segments) {
  float aspect_fix = (float)height_ / (float)width_;

  std::vector<float> verts;
  verts.reserve((segments + 1) * 2);

  // Generate circle vertices
  for (int i = 0; i < segments; i++) {
    float t = (float)i / (float)segments;
    float a = t * 2.0f * 3.1415926535f;

    float x = std::cos(a) * radius_ndc * aspect_fix;
    float y = std::sin(a) * radius_ndc;

    verts.push_back(x);
    verts.push_back(y);
  }

  vertex_count_ = segments;

  // Upload to GPU
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

/**
 * Build tick mark geometry for bearing intervals (90°, 30°, 10°, 5°)
 */
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

  // Helper: Add tick line based on angle
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

  // Generate tick marks for each degree interval
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

  // Helper: Upload geometry to GPU
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

/**
 * Build cardinal marker triangles (N, E, S, W)
 */
void CompasRenderer::buildCardinalMarkersGeometry(float radius_ndc, float size_ndc) {
  float aspect_fix = (float)height_ / (float)width_;

  std::vector<float> verts;
  verts.reserve(4 * 3 * 2);

  // Helper: Add inward-pointing triangle at bearing
  auto add_triangle_inward = [&](float bearing_deg) {
    float angle = bearing_deg - heading_deg_;
    float a = angle * 3.1415926535f / 180.0f;
    
    float cos_a = std::cos(a);
    float sin_a = std::sin(a);

    float cx = cos_a * radius_ndc * aspect_fix;
    float cy = sin_a * radius_ndc;

    // Inward vector pointing towards compass center
    float inx = -cos_a;
    float iny = -sin_a;

    // Tip vertex (pointing inward)
    float x0 = cx + inx * size_ndc * aspect_fix;
    float y0 = cy + iny * size_ndc;

    // Tangent vector (perpendicular to radial direction)
    float tx = -sin_a;
    float ty = cos_a;

    float half = size_ndc * 0.6f;

    // Base vertices
    float x1 = cx + tx * half * aspect_fix;
    float y1 = cy + ty * half;

    float x2 = cx - tx * half * aspect_fix;
    float y2 = cy - ty * half;

    verts.insert(verts.end(), {x0, y0, x1, y1, x2, y2});
  };

  // Add triangles for N, E, S, W
  add_triangle_inward(0.0f);    // North
  add_triangle_inward(90.0f);   // East
  add_triangle_inward(180.0f);  // South
  add_triangle_inward(270.0f);  // West

  markers_vertex_count_ = (int)(verts.size() / 2);

  // Upload to GPU
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

/**
 * Build heading indicator arrow (yellow triangle at top)
 */
void CompasRenderer::buildHeadingIndicatorGeometry() {
  float aspect_fix = (float)height_ / (float)width_;
  
  std::vector<float> verts;
  
  // Heading indicator parameters
  float indicator_y = 0.77f;    // Y position
  float arrow_height = 0.12f;   // Arrow height
  float arrow_width = 0.06f;    // Arrow width
  
  // Triangle vertices (pointing downward)
  float x_center = 0.0f;
  float x_left = -arrow_width * aspect_fix;
  float x_right = arrow_width * aspect_fix;
  
  float y_top = indicator_y;
  float y_bottom = indicator_y - arrow_height;
  
  verts.push_back(x_center);
  verts.push_back(y_bottom);
  verts.push_back(x_left);
  verts.push_back(y_top);
  verts.push_back(x_right);
  verts.push_back(y_top);
  
  heading_indicator_vertex_count_ = 3;
  
  // Upload to GPU
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

// Render compass outer ring (white circle)
void CompasRenderer::drawRing() {
  shader_.use();
  glBindVertexArray(vao_);

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);

  glLineWidth(10.0f);
  glDrawArrays(GL_LINE_LOOP, 0, vertex_count_);

  glBindVertexArray(0);
}

// Render all tick marks (90°, 30°, 10°, 5°)
void CompasRenderer::drawTicks() {
  buildTicksGeometry(tick_outer_r_, tick_inner_r_90_, tick_inner_r_30_, tick_inner_r_10_, tick_inner_r_5_);

  shader_.use();
  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);

  // Draw 90 degree ticks (thickest)
  glBindVertexArray(cardinal_vao_);
  glLineWidth(5.0f);
  glDrawArrays(GL_LINES, 0, cardinal_count_);

  // Draw 30 degree ticks
  glBindVertexArray(major_vao_);
  glLineWidth(3.5f);
  glDrawArrays(GL_LINES, 0, major_count_);

  // Draw 10 degree ticks
  glBindVertexArray(medium_vao_);
  glLineWidth(2.0f);
  glDrawArrays(GL_LINES, 0, medium_count_);

  // Draw 5 degree ticks (thinnest)
  glBindVertexArray(minor_vao_);
  glLineWidth(1.0f);
  glDrawArrays(GL_LINES, 0, minor_count_);

  glBindVertexArray(0);
}

// Render cardinal marker triangles (N, E, S, W)
void CompasRenderer::drawCardinalMarkers() {
  buildCardinalMarkersGeometry(0.70f, 0.06f);

  shader_.use();
  glBindVertexArray(markers_vao_);

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 1.0f);

  glDrawArrays(GL_TRIANGLES, 0, markers_vertex_count_);

  glBindVertexArray(0);
}

// Render heading indicator arrow at top (yellow)
void CompasRenderer::drawHeadingIndicator() {
  shader_.use();
  glBindVertexArray(heading_indicator_vao_);

  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 0.0f);  // Yellow

  glDrawArrays(GL_TRIANGLES, 0, heading_indicator_vertex_count_);

  glBindVertexArray(0);
}

/**
 * Draw bug triangle (magenta) indicating desired heading
 */
void CompasRenderer::drawBugTriangle(float bearing_deg, float heading_deg, float aspect_fix, float radius) {
  float rotated_bearing = bearing_deg + heading_deg;
  float angle_rad = rotated_bearing * 3.1415926535f / 180.0f;
  
  // Center position on compass circle
  float cx = std::sin(angle_rad) * radius;
  float cy = std::cos(angle_rad) * radius;
  cx *= aspect_fix;

  // Inward pointing vector (towards compass center)
  float inx = -std::sin(angle_rad);
  float iny = -std::cos(angle_rad);

  float tri_size = 0.10f;
  
  // Tip vertex (pointing inward)
  float x0 = cx + inx * tri_size * aspect_fix;
  float y0 = cy + iny * tri_size;

  // Tangent vector (perpendicular)
  float tx = -iny;
  float ty = inx;
  float half = tri_size * 0.6f;

  // Base vertices
  float x1 = cx + tx * half * aspect_fix;
  float y1 = cy + ty * half;

  float x2 = cx - tx * half * aspect_fix;
  float y2 = cy - ty * half;

  float vertices[] = {x0, y0, x1, y1, x2, y2};

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glLineWidth(5.0f);
  glDrawArrays(GL_LINE_LOOP, 0, 3);
  
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
}

void CompasRenderer::drawWaypointArrowDouble(float bearing_deg, float heading_deg, float aspect_fix, float radius) {
  float rotated_bearing = bearing_deg + heading_deg;
  float angle_rad = rotated_bearing * 3.1415926535f / 180.0f;
  
  float start_radius = -0.50f;
  float sx = std::sin(angle_rad) * start_radius;
  float sy = std::cos(angle_rad) * start_radius;
  sx *= aspect_fix;

  float end_radius = radius;
  float ex = std::sin(angle_rad) * end_radius;
  float ey = std::cos(angle_rad) * end_radius;
  ex *= aspect_fix;

  float outx = std::sin(angle_rad);
  float outy = std::cos(angle_rad);

  float tx = outy;
  float ty = -outx;

  float arrow_head_length = 0.08f;
  float arrow_head_width = 0.04f;

  float base_x = ex - outx * arrow_head_length * aspect_fix;
  float base_y = ey - outy * arrow_head_length;

  float line_offset = 0.017f;
  
  float sx1 = sx + tx * line_offset * aspect_fix;
  float sy1 = sy + ty * line_offset;
  float ex1 = base_x + tx * line_offset * aspect_fix;
  float ey1 = base_y + ty * line_offset;
  
  float sx2 = sx - tx * line_offset * aspect_fix;
  float sy2 = sy - ty * line_offset;
  float ex2 = base_x - tx * line_offset * aspect_fix;
  float ey2 = base_y - ty * line_offset;

  float tip_x = ex;
  float tip_y = ey;

  float left_x = base_x + tx * arrow_head_width * aspect_fix;
  float left_y = base_y + ty * arrow_head_width;

  float right_x = base_x - tx * arrow_head_width * aspect_fix;
  float right_y = base_y - ty * arrow_head_width;

  // ─── DRAW DOUBLE LINES ───
  float line_vertices[] = {
    sx1, sy1, ex1, ey1,
    sx2, sy2, ex2, ey2
  };

  GLuint line_VAO, line_VBO;
  glGenVertexArrays(1, &line_VAO);
  glGenBuffers(1, &line_VBO);
  
  glBindVertexArray(line_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, line_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glLineWidth(3.5f);
  glDrawArrays(GL_LINES, 0, 4);
  
  glBindVertexArray(0);
  glDeleteBuffers(1, &line_VBO);
  glDeleteVertexArrays(1, &line_VAO);

  // ─── DRAW ARROW HEAD FILLED ───
  float arrow_vertices[] = {
    // Left triangle (FILLED)
    base_x, base_y,
    left_x, left_y,
    tip_x, tip_y,
    
    // Right triangle (FILLED)
    base_x, base_y,
    tip_x, tip_y,
    right_x, right_y
  };

  GLuint arrow_VAO, arrow_VBO;
  glGenVertexArrays(1, &arrow_VAO);
  glGenBuffers(1, &arrow_VBO);
  
  glBindVertexArray(arrow_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, arrow_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(arrow_vertices), arrow_vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  shader_.use();
  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 0.0f, 1.0f, 0.0f);  // Green

  glDrawArrays(GL_TRIANGLES, 0, 6);
  
  glBindVertexArray(0);
  glDeleteBuffers(1, &arrow_VBO);
  glDeleteVertexArrays(1, &arrow_VAO);
}

void CompasRenderer::drawWaypointArrowSingle(float bearing_deg, float heading_deg, float aspect_fix, float radius) {
  float rotated_bearing = bearing_deg + heading_deg;
  float angle_rad = rotated_bearing * 3.1415926535f / 180.0f;
  
  float start_radius = -0.50f;
  float sx = std::sin(angle_rad) * start_radius;
  float sy = std::cos(angle_rad) * start_radius;
  sx *= aspect_fix;

  float end_radius = radius;
  float ex = std::sin(angle_rad) * end_radius;
  float ey = std::cos(angle_rad) * end_radius;
  ex *= aspect_fix;

  float outx = std::sin(angle_rad);
  float outy = std::cos(angle_rad);

  float tx = outy;
  float ty = -outx;

  float arrow_head_length = 0.08f;
  float arrow_head_width = 0.04f;

  float base_x = ex - outx * arrow_head_length * aspect_fix;
  float base_y = ey - outy * arrow_head_length;

  float tip_x = ex;
  float tip_y = ey;

  float left_x = base_x + tx * arrow_head_width * aspect_fix;
  float left_y = base_y + ty * arrow_head_width;

  float right_x = base_x - tx * arrow_head_width * aspect_fix;
  float right_y = base_y - ty * arrow_head_width;

  // ─── DRAW SINGLE LINE ───
  float line_vertices[] = {
    sx, sy, base_x, base_y
  };

  GLuint line_VAO, line_VBO;
  glGenVertexArrays(1, &line_VAO);
  glGenBuffers(1, &line_VBO);
  
  glBindVertexArray(line_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, line_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glLineWidth(8.0f);
  glDrawArrays(GL_LINES, 0, 2);
  
  glBindVertexArray(0);
  glDeleteBuffers(1, &line_VBO);
  glDeleteVertexArrays(1, &line_VAO);

  // ─── DRAW ARROW HEAD FILLED ───
  float arrow_vertices[] = {
    // Left triangle (FILLED)
    base_x, base_y,
    left_x, left_y,
    tip_x, tip_y,
    
    // Right triangle (FILLED)
    base_x, base_y,
    tip_x, tip_y,
    right_x, right_y
  };

  GLuint arrow_VAO, arrow_VBO;
  glGenVertexArrays(1, &arrow_VAO);
  glGenBuffers(1, &arrow_VBO);
  
  glBindVertexArray(arrow_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, arrow_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(arrow_vertices), arrow_vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  shader_.use();
  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 0.0f);  // Yellow

  glDrawArrays(GL_TRIANGLES, 0, 6);
  
  glBindVertexArray(0);
  glDeleteBuffers(1, &arrow_VBO);
  glDeleteVertexArrays(1, &arrow_VAO);
}

/**
 * Draw small circles perpendicular to waypoint arrow
 * 7 circles positioned perpendicular (90 degrees) to the yellow waypoint arrow
 * 
 * @param bearing_deg Target bearing direction
 * @param heading_deg Current heading
 * @param aspect_fix Aspect ratio correction
 * @param radius Circle positioning radius
 * @param circle_spacing Distance between circles
 * @param circle_radius Size of each circle
 * @param circle_opacity Alpha transparency (0.0-1.0)
 * @param line_width Ketebalan garis circle (default: 1.5f)
 */
void CompasRenderer::drawWaypointCircles(float bearing_deg, float heading_deg, float aspect_fix, 
                                         float radius, float circle_spacing, float circle_radius,
                                         float circle_opacity, float line_width) {
  float rotated_bearing = bearing_deg + heading_deg;
  float angle_rad = rotated_bearing * 3.1415926535f / 180.0f;
  
  // Arrow direction
  float outx = std::sin(angle_rad);
  float outy = std::cos(angle_rad);

  // Perpendicular direction (90 degrees to arrow)
  float perpx = std::cos(angle_rad);
  float perpy = -std::sin(angle_rad);

  // Arrow center position
  float center_radius = (radius - 0.50f) / 2.0f;  // Midpoint between start and end
  float cx = std::sin(angle_rad) * center_radius;
  float cy = std::cos(angle_rad) * center_radius;
  cx *= aspect_fix;

  // Circle parameters
  const int NUM_CIRCLES = 7;
  const int CIRCLE_SEGMENTS = 12;        // Segments per circle

  // Draw circles perpendicular to arrow (spanning left and right)
  for (int i = -(NUM_CIRCLES / 2); i <= (NUM_CIRCLES / 2); ++i) {
    // Position along perpendicular axis
    float offset = (float)i * circle_spacing;
    
    float circle_x = cx + perpx * offset * aspect_fix;
    float circle_y = cy + perpy * offset;

    // Generate circle vertices
    std::vector<float> circle_verts;
    circle_verts.reserve((CIRCLE_SEGMENTS + 1) * 2);

    for (int j = 0; j < CIRCLE_SEGMENTS; ++j) {
      float angle = (float)j / (float)CIRCLE_SEGMENTS * 2.0f * 3.1415926535f;
      float x = circle_x + std::cos(angle) * circle_radius * aspect_fix;
      float y = circle_y + std::sin(angle) * circle_radius;
      circle_verts.push_back(x);
      circle_verts.push_back(y);
    }

    // Upload and render circle
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(circle_verts.size() * sizeof(float)), 
                 circle_verts.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set line width dan opacity
    glLineWidth(line_width);
    glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_SEGMENTS);
    
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
  }
}

/**
 * Draw perpendicular line inside circle array
 * Line is perpendicular (90 degrees) to waypoint arrow
 * Line moves along the circle array with keyboard control
 * 
 * @param bearing_deg Target bearing direction
 * @param heading_deg Current heading
 * @param aspect_fix Aspect ratio correction
 * @param circle_spacing Distance between circles
 * @param line_length Length of perpendicular line (adjustable)
 * @param line_width Thickness of line (adjustable)
 */
void CompasRenderer::drawPerpendicularLine(float bearing_deg, float heading_deg, float aspect_fix,
                                          float circle_spacing, float line_length, float line_width) {
  float rotated_bearing = bearing_deg + heading_deg;
  float angle_rad = rotated_bearing * 3.1415926535f / 180.0f;
  
  // Arrow direction (along the arrow line)
  float outx = std::sin(angle_rad);
  float outy = std::cos(angle_rad);

  // Perpendicular direction (90 degrees to arrow - left/right direction)
  float perpx = std::cos(angle_rad);
  float perpy = -std::sin(angle_rad);

  // Arrow center position
  float center_radius = 0.0f;
  float cx = std::sin(angle_rad) * center_radius;
  float cy = std::cos(angle_rad) * center_radius;
  cx *= aspect_fix;

  // Calculate max offset based on circle spacing (7 circles)
  const int NUM_CIRCLES = 7;
  float max_offset = (NUM_CIRCLES / 2.0f) * circle_spacing;

  // Clamp offset to max allowed
  float clamped_offset = perp_line_offset_;
  if (clamped_offset > max_offset) {
    clamped_offset = max_offset;
  }
  if (clamped_offset < -max_offset) {
    clamped_offset = -max_offset;
  }

  // Line center position (along perpendicular direction at current offset)
  float line_center_x = cx + perpx * clamped_offset * aspect_fix;
  float line_center_y = cy + perpy * clamped_offset;

  // Line extends up and down (along arrow direction)
  float start_x = line_center_x - outx * (line_length / 2.0f) * aspect_fix;
  float start_y = line_center_y - outy * (line_length / 2.0f);
  
  float end_x = line_center_x + outx * (line_length / 2.0f) * aspect_fix;
  float end_y = line_center_y + outy * (line_length / 2.0f);

  // Create line vertices
  float vertices[] = {
    start_x, start_y,
    end_x, end_y
  };

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glLineWidth(line_width);
  glDrawArrays(GL_LINES, 0, 2);
  
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
}

/**
 * Update perpendicular line offset (slider movement)
 * Positive = right, Negative = left
 * Bergerak di antara circle array
 */
void CompasRenderer::updatePerpLineOffset(float delta) {
  perp_line_offset_ += delta;
}

/**
 * Set perpendicular line to absolute position
 */
void CompasRenderer::setPerpLineOffset(float offset) {
  perp_line_offset_ = offset;
}

/**
 * Draw aircraft symbol (gray) at center of compass
 * Represents current aircraft heading and position
 */
void CompasRenderer::drawAircraftSymbol(float aspect_fix) {
  // Aircraft dimensions (sleek and elongated design)
  float nose_y = 0.30f;         // Nose tip
  float cockpit_y = 0.20f;      // Cockpit base
  float wing_front_y = 0.08f;   // Front edge of wings
  float wing_back_y = -0.02f;   // Back edge of wings
  float body_mid_y = -0.12f;    // Fuselage middle
  float tail_wing_y = -0.16f;   // Horizontal stabilizer
  float tail_end_y = -0.24f;    // Vertical stabilizer
  
  float cockpit_w = 0.040f;     // Cockpit width (slender)
  float body_w = 0.048f;        // Fuselage width (slender)
  float wing_w = 0.50f;         // Wing span (elongated)
  float wing_back_w = 0.58f;    // Back wing span
  float tail_wing_w = 0.12f;    // Horizontal stabilizer span
  float tail_end_w = 0.26f;     // Vertical stabilizer width
  float tail_vert_w = 0.040f;   // Vertical stabilizer thickness
  
  std::vector<float> vertices = {
    // === NOSE (pointed with smooth curve) ===
    0.0f, nose_y,
    -cockpit_w * 0.6f * aspect_fix, nose_y - 0.04f,
    -cockpit_w * aspect_fix, cockpit_y,
    
    // === LEFT BODY (cockpit to wing) ===
    -body_w * 0.9f * aspect_fix, cockpit_y - 0.02f,
    -body_w * aspect_fix, wing_front_y + 0.04f,
    
    // === LEFT WING ===
    -wing_w * 0.5f * aspect_fix, wing_front_y,
    -wing_back_w * 0.5f * aspect_fix, wing_back_y,
    
    // === LEFT BODY (wing to tail) ===
    -body_w * aspect_fix, wing_back_y,
    -body_w * aspect_fix, body_mid_y,
    -body_w * 0.85f * aspect_fix, tail_wing_y,
    
    // === LEFT TAIL WING (horizontal stabilizer) ===
    -tail_wing_w * 0.6f * aspect_fix, tail_wing_y,
    -tail_end_w * 0.6f * aspect_fix, tail_end_y,
    
    // === LEFT VERTICAL STABILIZER (V-notch inward) ===
    -tail_vert_w * aspect_fix, tail_end_y,
    -tail_vert_w * 0.4f * aspect_fix, tail_end_y + 0.03f,
    
    // === CENTER VERTICAL STABILIZER (V-notch apex) ===
    0.0f, tail_end_y - 0.03f,
    
    // === RIGHT VERTICAL STABILIZER (V-notch inward) ===
    tail_vert_w * 0.4f * aspect_fix, tail_end_y + 0.03f,
    tail_vert_w * aspect_fix, tail_end_y,
    
    // === RIGHT TAIL WING (horizontal stabilizer) ===
    tail_end_w * 0.6f * aspect_fix, tail_end_y,
    tail_wing_w * 0.6f * aspect_fix, tail_wing_y,
    
    // === RIGHT BODY (tail to wing) ===
    body_w * 0.85f * aspect_fix, tail_wing_y,
    body_w * aspect_fix, body_mid_y,
    body_w * aspect_fix, wing_back_y,
    
    // === RIGHT WING ===
    wing_back_w * 0.5f * aspect_fix, wing_back_y,
    wing_w * 0.5f * aspect_fix, wing_front_y,
    
    // === RIGHT BODY (wing to cockpit) ===
    body_w * aspect_fix, wing_front_y + 0.04f,
    body_w * 0.9f * aspect_fix, cockpit_y - 0.02f,
    
    // === NOSE (pointed with smooth curve) ===
    cockpit_w * aspect_fix, cockpit_y,
    cockpit_w * 0.6f * aspect_fix, nose_y - 0.04f,
    0.0f, nose_y
  };

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  shader_.use();
  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 0.55f, 0.55f, 0.55f);  // Gray color

  glLineWidth(3.5f);
  glDrawArrays(GL_LINE_STRIP, 0, vertices.size() / 2);
  
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
}

  /**
   * Draw TO/FROM flag triangle
   * @param bearing_deg Target bearing
   * @param heading_deg Current heading
   * @param aspect_fix Aspect ratio fix
   * @param is_to If true draws TO (pointing same direction), if false draws FROM (180° reversed)
   * @param radius Distance from center
   */
void CompasRenderer::drawToFromFlag(float bearing_deg, float heading_deg, float aspect_fix,
                                     bool is_to, float radius) {
  // ===== GUNAKAN STATE DARI CLASS =====
  bool flag_is_to = is_to_flag_;  // Gunakan state dari member variable
  
  // ===== GUNAKAN FORMULA YANG SAMA DENGAN drawWaypointArrowSingle =====
  float rotated_bearing = bearing_deg + heading_deg;
  float angle_rad = rotated_bearing * 3.1415926535f / 180.0f;
  
  // Arrow direction vectors (SAMA SEPERTI ARROW)
  float outx = std::sin(angle_rad);
  float outy = std::cos(angle_rad);

  float tx = outy;
  float ty = -outx;

  // ===== POSISI FLAG: SEBELAH KANAN BAWAH DEKAT PANGKAL ARROW =====
  float end_radius = radius;
  float ex = std::sin(angle_rad) * end_radius;
  float ey = std::cos(angle_rad) * end_radius;
  ex *= aspect_fix;

  float arrow_head_length = 0.7f;
  float base_x = ex - outx * arrow_head_length * aspect_fix;
  float base_y = ey - outy * arrow_head_length;

  // ===== FLAG CENTER =====
  float flag_offset_right = 0.20f;
  float flag_offset_down = 0.04f;
  
  float flag_center_x = base_x + tx * flag_offset_right * aspect_fix - outx * flag_offset_down * aspect_fix;
  float flag_center_y = base_y + ty * flag_offset_right - outy * flag_offset_down;

  // ===== FLAG DIRECTION: TO vs FROM (BERDASARKAN STATE) =====
  float flag_angle_offset = flag_is_to ? 0.0f : 180.0f;
  float flag_rotated_bearing = bearing_deg + heading_deg + flag_angle_offset;
  if (flag_rotated_bearing >= 360.0f) flag_rotated_bearing -= 360.0f;
  
  float flag_angle_rad = flag_rotated_bearing * 3.1415926535f / 180.0f;
  
  float flag_outx = std::sin(flag_angle_rad);
  float flag_outy = std::cos(flag_angle_rad);
  
  float flag_tx = flag_outy;
  float flag_ty = -flag_outx;

  // ===== TRIANGLE VERTICES (HANYA 2 GARIS LANCIP) =====
  float tri_size = 0.10f;
  
  // Tip (pointing in flag direction)
  float x1 = flag_center_x + flag_outx * tri_size * aspect_fix;
  float y1 = flag_center_y + flag_outy * tri_size;
  
  // Base left
  float x2 = flag_center_x + flag_tx * tri_size * 0.5f * aspect_fix;
  float y2 = flag_center_y + flag_ty * tri_size * 0.5f;
  
  // Base right
  float x3 = flag_center_x - flag_tx * tri_size * 0.5f * aspect_fix;
  float y3 = flag_center_y - flag_ty * tri_size * 0.5f;
  
  // ===== HANYA DUA GARIS: TIP-LEFT dan TIP-RIGHT =====
  std::vector<float> vertices = {
    x1, y1,  // Tip
    x2, y2,  // Base left
    x1, y1,  // Tip (kembali ke tip)
    x3, y3   // Base right
  };
  
  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  shader_.use();
  GLint loc = glGetUniformLocation(shader_.id(), "uColor");
  glUniform3f(loc, 1.0f, 1.0f, 0.0f);  // Yellow

  // ===== RENDER DUA GARIS YANG MEMBENTUK LANCIP =====
  glLineWidth(5.5f);
  glDrawArrays(GL_LINES, 0, 4);  // 4 vertices = 2 garis
  
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}