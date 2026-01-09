#include "ui/HsiUiRenderer.hpp"
#include <cstdio>

HsiUiRenderer::HsiUiRenderer(TtfTextRenderer& info_font, TtfTextRenderer& info_label_font,
                             TtfTextRenderer& waypoint_name_font, TtfTextRenderer& waypoint_bearing_font,
                             TtfTextRenderer& waypoint_info_font)
  : info_font_(info_font), info_label_font_(info_label_font),
    waypoint_name_font_(waypoint_name_font), waypoint_bearing_font_(waypoint_bearing_font),
    waypoint_info_font_(waypoint_info_font) {}

void HsiUiRenderer::renderWindGroup(const WindGroup& wind, float left_offset) {
  char wind_str[32];
  snprintf(wind_str, sizeof(wind_str), "WIND %03.0f/%.0f", wind.direction, wind.speed);
  info_label_font_.drawTextLeftAligned(wind_str, left_offset, wind.y, wind.r, wind.g, wind.b);
}

void HsiUiRenderer::renderGpsGroup(const GpsGroup& gps, float left_offset) {
  info_label_font_.drawTextLeftAligned(gps.status, left_offset, gps.y, gps.r, gps.g, gps.b);
}

void HsiUiRenderer::renderCogGroup(const CourseGroup& course, float right_offset) {
  char cog_str[32];

  snprintf(cog_str, sizeof(cog_str), "COG %.0f°", course.cog_value);
  info_font_.drawTextRightAligned(cog_str, right_offset, course.y_cog,
                                  course.r, course.g, course.b);
}

void HsiUiRenderer::renderGsGroup(const CourseGroup& course, float right_offset) {
  char gs_str[32];

  snprintf(gs_str, sizeof(gs_str), "GS %.0f", course.gs_value);
  info_font_.drawTextRightAligned(gs_str, right_offset, course.y_gs,
                                  course.r, course.g, course.b);
}

void HsiUiRenderer::renderIasGroup(const IasGroup& ias, float left_offset) {
  info_label_font_.drawTextLeftAligned("IAS", left_offset, ias.y + 0.10f,
                                       ias.label_r, ias.label_g, ias.label_b);
  
  char ias_str[32];
  snprintf(ias_str, sizeof(ias_str), "%.0f", ias.value);
  info_font_.drawTextLeftAligned(ias_str, left_offset, ias.y,
                                 ias.value_r, ias.value_g, ias.value_b);
}

void HsiUiRenderer::renderAltGroup(const AltGroup& alt, float right_offset) {
  info_label_font_.drawTextRightAligned("ALT", right_offset, alt.y + 0.10f,
                                        alt.label_r, alt.label_g, alt.label_b);
  
  char alt_str[32];
  snprintf(alt_str, sizeof(alt_str), "%.0f", alt.value);
  info_font_.drawTextRightAligned(alt_str, right_offset, alt.y,
                                  alt.value_r, alt.value_g, alt.value_b);
}

void HsiUiRenderer::renderWaypointLeft(const WaypointGroup& wp, float left_offset) {
  float y_current = wp.y_start;
  const float line_spacing = 0.085f;
  const float bearing_to_name = 0.13f; 
  char buffer[64];

  snprintf(buffer, sizeof(buffer), "%.0f°", wp.bearing);
  waypoint_bearing_font_.drawTextLeftAligned(buffer, left_offset, y_current,
                                             wp.r, wp.g, wp.b);
  y_current -= bearing_to_name;

  waypoint_name_font_.drawTextLeftAligned(wp.name, left_offset, y_current,
                                          wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  snprintf(buffer, sizeof(buffer), "%.1f km", wp.distance);
  waypoint_info_font_.drawTextLeftAligned(buffer, left_offset, y_current,
                                          wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  waypoint_info_font_.drawTextLeftAligned(wp.runway, left_offset, y_current,
                                          wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  snprintf(buffer, sizeof(buffer), "(APP)%.3f", wp.app_freq);
  waypoint_info_font_.drawTextLeftAligned(buffer, left_offset, y_current,
                                          wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  snprintf(buffer, sizeof(buffer), "(INF)%.3f", wp.info_freq);
  waypoint_info_font_.drawTextLeftAligned(buffer, left_offset, y_current,
                                          wp.r, wp.g, wp.b);
}

void HsiUiRenderer::renderWaypointRight(const WaypointGroup& wp, float right_offset) {
  float y_current = wp.y_start;
  const float line_spacing = 0.085f;
  const float bearing_to_name = 0.13f;  
  char buffer[64];

  snprintf(buffer, sizeof(buffer), "%.0f°", wp.bearing);
  waypoint_bearing_font_.drawTextRightAligned(buffer, right_offset, y_current,
                                              wp.r, wp.g, wp.b);
  y_current -= bearing_to_name;

  waypoint_name_font_.drawTextRightAligned(wp.name, right_offset, y_current,
                                           wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  snprintf(buffer, sizeof(buffer), ">%.0f km", wp.distance);
  waypoint_info_font_.drawTextRightAligned(buffer, right_offset, y_current,
                                           wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  waypoint_info_font_.drawTextRightAligned(wp.runway, right_offset, y_current,
                                           wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  snprintf(buffer, sizeof(buffer), "(ACC)%.3f", wp.app_freq);
  waypoint_info_font_.drawTextRightAligned(buffer, right_offset, y_current,
                                           wp.r, wp.g, wp.b);
  y_current -= line_spacing;

  snprintf(buffer, sizeof(buffer), "(ACC)%.3f", wp.info_freq);
  waypoint_info_font_.drawTextRightAligned(buffer, right_offset, y_current,
                                           wp.r, wp.g, wp.b);
}

void HsiUiRenderer::renderBugGroup(const BugGroup& bug) {
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "BUG %.0f °", bug.value);
  
  info_font_.drawTextCenteredNDC(buffer, bug.x, bug.y,  
                                 bug.r, bug.g, bug.b);
}