#include "ui/HsiUiRenderer.hpp"
#include <cstdio>

HsiUiRenderer::HsiUiRenderer(TtfTextRenderer& info_font, TtfTextRenderer& info_label_font)
  : info_font_(info_font), info_label_font_(info_label_font) {}

void HsiUiRenderer::renderWindGroup(const WindGroup& wind, float left_offset) {
  char wind_str[32];
  snprintf(wind_str, sizeof(wind_str), "WIND %03.0f/%.0f", wind.direction, wind.speed);
  info_label_font_.drawTextLeftAligned(wind_str, left_offset, wind.y, wind.r, wind.g, wind.b);
}

void HsiUiRenderer::renderGpsGroup(const GpsGroup& gps, float left_offset) {
  info_label_font_.drawTextLeftAligned(gps.status, left_offset, gps.y, gps.r, gps.g, gps.b);
}

void HsiUiRenderer::renderIasGroup(const IasGroup& ias, float left_offset) {
  char ias_str[32];
  info_label_font_.drawTextLeftAligned("IAS", left_offset, ias.y + 0.10f,
                                       ias.label_r, ias.label_g, ias.label_b);
  snprintf(ias_str, sizeof(ias_str), "%.0f", ias.value);
  info_font_.drawTextLeftAligned(ias_str, left_offset, ias.y,
                                 ias.value_r, ias.value_g, ias.value_b);
}

void HsiUiRenderer::renderCourseGroup(const CourseGroup& course, float right_offset) {
  char cog_str[32], gs_str[32];
  snprintf(cog_str, sizeof(cog_str), "COG  %.0f", course.cog_value);
  snprintf(gs_str, sizeof(gs_str), "GS  %.0f", course.gs_value);
  info_label_font_.drawTextRightAligned(cog_str, right_offset, course.y_cog,
                                        course.r, course.g, course.b);
  info_label_font_.drawTextRightAligned(gs_str, right_offset, course.y_gs,
                                        course.r, course.g, course.b);
}

void HsiUiRenderer::renderAltGroup(const AltGroup& alt, float right_offset) {
  char alt_str[32];
  info_label_font_.drawTextRightAligned("ALT", right_offset, alt.y + 0.10f,
                                        alt.label_r, alt.label_g, alt.label_b);
  snprintf(alt_str, sizeof(alt_str), "%.0f", alt.value);
  info_font_.drawTextRightAligned(alt_str, right_offset, alt.y,
                                  alt.value_r, alt.value_g, alt.value_b);
}

void HsiUiRenderer::renderWaypointLeft(const WaypointGroup& wp, float left_offset) {
  char bearing_str[32], dist_str[32], app_freq_str[32], info_freq_str[32];
  
  snprintf(bearing_str, sizeof(bearing_str), "%.0f deg", wp.bearing);
  info_font_.drawTextLeftAligned(bearing_str, left_offset, wp.y_start, wp.r, wp.g, wp.b);
  
  info_font_.drawTextLeftAligned(wp.name, left_offset, wp.y_start - 0.10f, wp.r, wp.g, wp.b);
  
  snprintf(dist_str, sizeof(dist_str), "%.1f km", wp.distance);
  info_label_font_.drawTextLeftAligned(dist_str, left_offset, wp.y_start - 0.20f, wp.r, wp.g, wp.b);
  
  info_label_font_.drawTextLeftAligned(wp.runway, left_offset, wp.y_start - 0.30f, wp.r, wp.g, wp.b);
  
  snprintf(app_freq_str, sizeof(app_freq_str), "(APP)%.3f", wp.app_freq);
  info_label_font_.drawTextLeftAligned(app_freq_str, left_offset, wp.y_start - 0.40f, wp.r, wp.g, wp.b);
  
  snprintf(info_freq_str, sizeof(info_freq_str), "(INF)%.3f", wp.info_freq);
  info_label_font_.drawTextLeftAligned(info_freq_str, left_offset, wp.y_start - 0.50f, wp.r, wp.g, wp.b);
}

void HsiUiRenderer::renderWaypointRight(const WaypointGroup& wp, float right_offset) {
  char bearing_str[32], dist_str[32], app_freq_str[32], info_freq_str[32];
  
  snprintf(bearing_str, sizeof(bearing_str), "%.0f deg", wp.bearing);
  info_font_.drawTextRightAligned(bearing_str, right_offset, wp.y_start, wp.r, wp.g, wp.b);
  
  info_font_.drawTextRightAligned(wp.name, right_offset, wp.y_start - 0.10f, wp.r, wp.g, wp.b);
  
  snprintf(dist_str, sizeof(dist_str), ">%.0f km", wp.distance);
  info_label_font_.drawTextRightAligned(dist_str, right_offset, wp.y_start - 0.20f, wp.r, wp.g, wp.b);
  
  info_label_font_.drawTextRightAligned(wp.runway, right_offset, wp.y_start - 0.30f, wp.r, wp.g, wp.b);
  
  snprintf(app_freq_str, sizeof(app_freq_str), "(ACC)%.3f", wp.app_freq);
  info_label_font_.drawTextRightAligned(app_freq_str, right_offset, wp.y_start - 0.40f, wp.r, wp.g, wp.b);
  
  snprintf(info_freq_str, sizeof(info_freq_str), "(ACC)%.3f", wp.info_freq);
  info_label_font_.drawTextRightAligned(info_freq_str, right_offset, wp.y_start - 0.50f, wp.r, wp.g, wp.b);
}

void HsiUiRenderer::renderBugGroup(const BugGroup& bug) {
  char bug_str[32];
  snprintf(bug_str, sizeof(bug_str), "BUG  %.0f deg", bug.value);
  info_font_.drawTextCenteredNDC(bug_str, bug.x, bug.y, bug.r, bug.g, bug.b);
}