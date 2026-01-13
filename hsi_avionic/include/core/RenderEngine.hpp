#pragma once

#include <glad/glad.h>
#include "compas/CompasRenderer.hpp"
#include "gfx/TtfTextRenderer.hpp"
#include "gfx/Shader.hpp"
#include "ui/HsiUiRenderer.hpp"
#include "core/ApplicationState.hpp"
#include "config/AppConfig.hpp"

class RenderEngine {
public:
  RenderEngine(Shader& shader);

  void renderFrame(CompasRenderer& compas, 
                   TtfTextRenderer fonts[],
                   HsiUiRenderer& ui_renderer,
                   ApplicationState& state);

private:
  Shader& shader_;

  void renderCompass(CompasRenderer& compas, TtfTextRenderer& ttf_cardinal,
                     TtfTextRenderer& ttf_numbers, float heading_deg);

  void renderHeadingDisplay(TtfTextRenderer& ttf_heading, TtfTextRenderer& ttf_label,
                            float heading_deg);

  void renderNavigationOverlays(CompasRenderer& compas, const ApplicationState& state);
};