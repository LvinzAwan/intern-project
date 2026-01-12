#ifndef HSI_UI_RENDERER_HPP
#define HSI_UI_RENDERER_HPP

#include "gfx/TtfTextRenderer.hpp"
#include "data/HsiData.hpp"

class HsiUiRenderer {
public:
  HsiUiRenderer(TtfTextRenderer& info_font, TtfTextRenderer& info_label_font,
                TtfTextRenderer& waypoint_name_font, TtfTextRenderer& waypoint_bearing_font,
                TtfTextRenderer& waypoint_info_font, TtfTextRenderer& ttf_info_side, TtfTextRenderer& ttf_info_label_side);

  void renderWindGroup(const WindGroup& wind, float left_offset);
  void renderGpsGroup(const GpsGroup& gps, float left_offset);
  void renderIasGroup(const IasGroup& ias, float left_offset);
  void renderCogGroup(const CourseGroup& course, float right_offset); 
  void renderGsGroup(const CourseGroup& course, float right_offset);     
  void renderAltGroup(const AltGroup& alt, float right_offset);
  void renderWaypointLeft(const WaypointGroup& wp, float left_offset);
  void renderWaypointRight(const WaypointGroup& wp, float right_offset);
  void renderBugGroup(const BugGroup& bug);
private:
  TtfTextRenderer& info_font_;
  TtfTextRenderer& info_label_font_;
  TtfTextRenderer& waypoint_name_font_;
  TtfTextRenderer& waypoint_bearing_font_;
  TtfTextRenderer& waypoint_info_font_;
  TtfTextRenderer& info_side_;
  TtfTextRenderer& info_label_side_;
};

#endif // HSI_UI_RENDERER_HPP