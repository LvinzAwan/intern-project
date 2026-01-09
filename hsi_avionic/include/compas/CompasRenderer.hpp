#pragma once

#include "gfx/Shader.hpp"
#include <glad/glad.h>

class CompasRenderer {
public:
  bool init(int width, int height);

  void setHeadingDeg(float h) { heading_deg_ = h; }
  float getHeadingDeg() const { return heading_deg_; }

  void drawRing();
  void drawTicks();
  void drawCardinalMarkers();
  void drawHeadingIndicator();

  void drawBugTriangle(float bearing_deg, float heading_deg, float aspect_fix, float radius);
  void drawWaypointArrowDouble(float bearing_deg, float heading_deg, float aspect_fix, float radius);
  void drawWaypointArrowSingle(float bearing_deg, float heading_deg, float aspect_fix, float radius);
  void drawWaypointCircles(float bearing_deg, float heading_deg, float aspect_fix, float radius,
                          float circle_spacing = 0.045f, float circle_radius = 0.015f,
                          float circle_opacity = 0.5f, float line_width = 1.5f);
  void drawAircraftSymbol(float aspect_fix);

  /**
   * Draw perpendicular line at waypoint circle
   */
  void drawPerpendicularLine(float bearing_deg, float heading_deg, float aspect_fix,
                            float circle_spacing, float line_length = 0.15f, float line_width = 3.5f);

  void updatePerpLineOffset(float delta);
  void setPerpLineOffset(float offset);
  float getPerpLineOffset() const { return perp_line_offset_; }

private:
  void buildRingGeometry(float radius_ndc, int segments);
  void buildTicksGeometry(float radius_ndc,
                         float len_cardinal,
                         float len_major,
                         float len_medium,
                         float len_minor);
  void buildCardinalMarkersGeometry(float radius_ndc, float size_ndc);
  void buildHeadingIndicatorGeometry();

  Shader shader_;

  GLuint vao_ = 0;
  GLuint vbo_ = 0;
  int vertex_count_ = 0;

  GLuint cardinal_vao_ = 0;
  GLuint cardinal_vbo_ = 0;
  int cardinal_count_ = 0;

  GLuint major_vao_ = 0;
  GLuint major_vbo_ = 0;
  int major_count_ = 0;

  GLuint medium_vao_ = 0;
  GLuint medium_vbo_ = 0;
  int medium_count_ = 0;

  GLuint minor_vao_ = 0;
  GLuint minor_vbo_ = 0;
  int minor_count_ = 0;

  GLuint markers_vao_ = 0;
  GLuint markers_vbo_ = 0;
  int markers_vertex_count_ = 0;

  GLuint heading_indicator_vao_ = 0;
  GLuint heading_indicator_vbo_ = 0;
  int heading_indicator_vertex_count_ = 0;

  float heading_deg_ = 0.0f;

  int width_ = 800;
  int height_ = 600;

  float tick_outer_r_ = 0.70f;
  float tick_inner_r_90_ = 0.10f;
  float tick_inner_r_30_ = 0.08f;
  float tick_inner_r_10_ = 0.06f;
  float tick_inner_r_5_  = 0.04f;

  // Perpendicular line parameters
  float perp_line_offset_ = 0.0f;    // Current offset position (slider)
};
