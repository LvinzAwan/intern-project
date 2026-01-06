#pragma once
#include <glad/glad.h>
#include "gfx/Shader.hpp"

class CompasRenderer {
public:
  bool init(int width, int height);
  void drawRing();
  void drawTicks();
  void drawCardinalMarkers();
  void setHeadingDeg(float deg) { heading_deg_ = deg; }
  void rebuildTicks();
  
private:
  Shader shader_;
  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  int vertex_count_ = 0;

  int width_ = 800;
  int height_ = 600;

  float heading_deg_ = 0.0f;

  GLuint ticks_vao_ = 0;
  GLuint ticks_vbo_ = 0;
  int ticks_vertex_count_ = 0;

  GLuint major_vao_ = 0, major_vbo_ = 0;
  GLuint medium_vao_ = 0, medium_vbo_ = 0;
  GLuint minor_vao_ = 0, minor_vbo_ = 0;

  int major_count_ = 0;
  int medium_count_ = 0;
  int minor_count_ = 0;

  GLuint markers_vao_ = 0;
  GLuint markers_vbo_ = 0;
  int markers_vertex_count_ = 0;

  float tick_outer_r_ = 0.66f;
  float tick_inner_r_30_ = 0.58f;
  float tick_inner_r_10_ = 0.60f;
  float tick_inner_r_5_  = 0.62f;

  void buildRingGeometry(float radius_ndc, int segments);
  void buildTicksGeometry(float radius_ndc,
                        float len_major, float len_medium, float len_minor);
  void buildCardinalMarkersGeometry(float radius_ndc, float size_ndc);

};
