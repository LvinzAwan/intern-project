#include "core/RenderEngine.hpp"
#include "config/AppConfig.hpp"
#include "gfx/HsiRenderer.hpp"
#include <cstdio>

RenderEngine::RenderEngine(Shader& shader) : shader_(shader) {}

void RenderEngine::renderFrame(CompasRenderer& compas,
                               TtfTextRenderer fonts[],
                               HsiUiRenderer& ui,
                               ApplicationState& state) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  //Render compass
  renderCompass(compas, fonts[FontConfig::CARDINAL], fonts[FontConfig::NUMBERS], state.heading_deg);

  //Render heading readout
  renderHeadingDisplay(fonts[FontConfig::HEADING_VALUE], fonts[FontConfig::HEADING_LABEL], state.heading_deg);

  //Render IAS/ALT frames
  HsiRenderer::drawIasAltFrame(
      DataConfig::IAS_FRAME_X, DataConfig::IAS_FRAME_Y,
      DataConfig::IAS_FRAME_WIDTH, DataConfig::IAS_FRAME_HEIGHT,
      ColorRGB::WHITE.r, ColorRGB::WHITE.g, ColorRGB::WHITE.b, true);

  HsiRenderer::drawIasAltFrame(
      DataConfig::ALT_FRAME_X, DataConfig::ALT_FRAME_Y,
      DataConfig::ALT_FRAME_WIDTH, DataConfig::ALT_FRAME_HEIGHT,
      ColorRGB::WHITE.r, ColorRGB::WHITE.g, ColorRGB::WHITE.b, false);

  //Render side panels
  ui.renderWindGroup(state.wind, DisplayLayout::LEFT_OFFSET);
  ui.renderGpsGroup(state.gps, DisplayLayout::LEFT_OFFSET);
  ui.renderIasGroup(state.ias, DisplayLayout::LEFT_OFFSET);

  ui.renderCogGroup(state.course, DisplayLayout::RIGHT_OFFSET);
  ui.renderGsGroup(state.course, DisplayLayout::RIGHT_OFFSET);
  ui.renderAltGroup(state.alt, DisplayLayout::RIGHT_OFFSET);

  ui.renderWaypointLeft(state.wp_left, DisplayLayout::LEFT_OFFSET);
  ui.renderWaypointRight(state.wp_right, DisplayLayout::RIGHT_OFFSET);
  ui.renderBugGroup(state.bug);

  //Render overlays on the compass
  renderNavigationOverlays(compas, state);
}

void RenderEngine::renderCompass(CompasRenderer& compas,
                                 TtfTextRenderer& ttf_cardinal,
                                 TtfTextRenderer& ttf_numbers,
                                 float heading_deg) {
  compas.drawRing();
  compas.drawTicks();
  compas.drawCardinalMarkers();
  compas.drawHeadingIndicator();
  compas.drawAircraftSymbol(WindowConfig::ASPECT_FIX);

  //Cardinal letters
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "N",   0.0f, DisplayLayout::CARDINAL_RADIUS, WindowConfig::ASPECT_FIX, heading_deg,
                                       ColorRGB::YELLOW.r, ColorRGB::YELLOW.g, ColorRGB::YELLOW.b);
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "E",  90.0f, DisplayLayout::CARDINAL_RADIUS, WindowConfig::ASPECT_FIX, heading_deg,
                                       ColorRGB::YELLOW.r, ColorRGB::YELLOW.g, ColorRGB::YELLOW.b);
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "S", 180.0f, DisplayLayout::CARDINAL_RADIUS, WindowConfig::ASPECT_FIX, heading_deg,
                                       ColorRGB::YELLOW.r, ColorRGB::YELLOW.g, ColorRGB::YELLOW.b);
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "W", 270.0f, DisplayLayout::CARDINAL_RADIUS, WindowConfig::ASPECT_FIX, heading_deg,
                                       ColorRGB::YELLOW.r, ColorRGB::YELLOW.g, ColorRGB::YELLOW.b);

  //Heading numbers
  static constexpr int kBearings[] = {30, 60, 120, 150, 210, 240, 300, 330};
  for (int bearing : kBearings) {
    char label[16];
    std::snprintf(label, sizeof(label), "%d", bearing / 10);

    HsiRenderer::drawTextAtBearingRadial(ttf_numbers, label, (float)bearing,
                                         DisplayLayout::NUMBER_RADIUS, WindowConfig::ASPECT_FIX, heading_deg,
                                         ColorRGB::WHITE.r, ColorRGB::WHITE.g, ColorRGB::WHITE.b);
  }
}

void RenderEngine::renderHeadingDisplay(TtfTextRenderer& ttf_heading,
                                        TtfTextRenderer& ttf_label,
                                        float heading_deg) {
  float display_heading = 360.0f - static_cast<int>(heading_deg);
  if (display_heading >= 360.0f) display_heading -= 360.0f;

  char heading_str[8];
  std::snprintf(heading_str, sizeof(heading_str), "%03d", static_cast<int>(display_heading));

  ttf_heading.drawTextCenteredNDC(heading_str, 0.0f, 0.82f,
                                 ColorRGB::YELLOW.r, ColorRGB::YELLOW.g, ColorRGB::YELLOW.b);

  ttf_label.drawTextCenteredNDC("HDG", -0.20f, 0.85f,
                               ColorRGB::WHITE.r, ColorRGB::WHITE.g, ColorRGB::WHITE.b);

  ttf_label.drawTextCenteredNDC("°M", 0.17f, 0.85f,
                               ColorRGB::WHITE.r, ColorRGB::WHITE.g, ColorRGB::WHITE.b);
}

void RenderEngine::renderNavigationOverlays(CompasRenderer& compas,
                                            const ApplicationState& state) {
  shader_.use();

  //Right waypoint
  compas.drawWaypointArrowDouble(state.wp_right_bearing, state.heading_deg, WindowConfig::ASPECT_FIX, 0.50f);

  //Left waypoint
  compas.drawWaypointArrowSingle(state.wp_left_bearing, state.heading_deg, WindowConfig::ASPECT_FIX, 0.50f);

  compas.drawWaypointCircles(state.wp_left_bearing, state.heading_deg, WindowConfig::ASPECT_FIX, 0.50f,
                             CircleConfig::SPACING, CircleConfig::RADIUS,
                             CircleConfig::OPACITY, CircleConfig::LINE_WIDTH);

  compas.drawPerpendicularLine(state.wp_left_bearing, state.heading_deg, WindowConfig::ASPECT_FIX,
                               PerpLineConfig::SPACING, PerpLineConfig::LINE_LENGTH, PerpLineConfig::LINE_WIDTH);

  compas.drawToFromFlag(state.wp_left_bearing, state.heading_deg, WindowConfig::ASPECT_FIX, true, 0.50f);

  //Heading box
  HsiRenderer::drawHeadingBox(DataConfig::HEADING_BOX_X, DataConfig::HEADING_BOX_Y,
                              DataConfig::HEADING_BOX_WIDTH, DataConfig::HEADING_BOX_HEIGHT,
                              ColorRGB::YELLOW.r, ColorRGB::YELLOW.g, ColorRGB::YELLOW.b);

  //Bug marker
  compas.drawBugTriangle(state.bug_heading, state.heading_deg, WindowConfig::ASPECT_FIX, 0.73f);
}
