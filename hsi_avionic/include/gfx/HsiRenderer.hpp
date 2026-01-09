#ifndef HSI_RENDERER_HPP
#define HSI_RENDERER_HPP

#include "gfx/TtfTextRenderer.hpp"

class HsiRenderer {
public:
  static void drawTextAtBearingRadial(TtfTextRenderer& ttf,
                                      const char* label,
                                      float bearing_deg,
                                      float radius,
                                      float aspect_fix,
                                      float heading_deg,
                                      float r, float g, float b);
  
  static void drawHeadingBox(float x, float y, float width, float height);
};

#endif 