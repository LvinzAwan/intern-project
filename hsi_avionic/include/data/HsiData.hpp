#ifndef HSI_DATA_HPP
#define HSI_DATA_HPP

#include "gfx/TtfTextRenderer.hpp"

struct WindGroup {
  float direction;
  float speed;
  float x, y;
  float r, g, b;
};

struct GpsGroup {
  const char* status;
  float x, y;
  float r, g, b;
};

struct IasGroup {
  float value;
  float x, y;
  float label_r, label_g, label_b;
  float value_r, value_g, value_b;
};

struct CourseGroup {
  float cog_value;
  float gs_value;
  float x, y_cog, y_gs;
  float r, g, b;
};

struct AltGroup {
  float value;
  float x, y;
  float label_r, label_g, label_b;
  float value_r, value_g, value_b;
};

struct WaypointGroup {
  float bearing;
  float distance;
  const char* name;
  const char* runway;
  float app_freq;
  float info_freq;
  float x, y_start;
  float r, g, b;
};

struct BugGroup {
  float value;
  float x, y;
  float r, g, b;
};

#endif // HSI_DATA_HPP