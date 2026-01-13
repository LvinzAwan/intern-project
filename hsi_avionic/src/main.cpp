#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "config/AppConfig.hpp"
#include "core/ApplicationState.hpp"
#include "core/InputHandler.hpp"
#include "core/RenderEngine.hpp"
#include "compas/CompasRenderer.hpp"
#include "gfx/TtfTextRenderer.hpp"
#include "gfx/Shader.hpp"
#include "ui/HsiUiRenderer.hpp"
#include "data/HsiData.hpp"

using namespace WindowConfig;
using namespace DataConfig;
using namespace FontConfig;
using namespace ColorRGB;

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
  glViewport(0, 0, w, h);
}

bool initializeFonts(TtfTextRenderer fonts[]) {
  for (int i = 0; i < FONT_COUNT; ++i) {
    if (!fonts[i].init(PATHS[i], SIZES[i])) {
      std::cerr << "Failed to init font: " << PATHS[i] << "\n";
      return false;
    }
  }
  return true;
}

bool initializeApplication(GLFWwindow*& window, CompasRenderer& compas,
                          TtfTextRenderer fonts[], Shader& shader) {
  if (!glfwInit()) {
    std::cerr << "GLFW init failed\n";
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
  if (!window) {
    std::cerr << "Create window failed\n";
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowSizeLimits(window, WIDTH, HEIGHT, WIDTH, HEIGHT);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "GLAD load failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return false;
  }

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //Initialize CompasRenderer
  if (!compas.init(WIDTH, HEIGHT)) {
    std::cerr << "CompasRenderer init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return false;
  }

  //Initialize Fonts
  if (!initializeFonts(fonts)) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return false;
  }

  //Setup Shader
  const char* vs = R"(#version 330 core
    layout (location = 0) in vec2 aPos;
    void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
  )";
  const char* fs = R"(#version 330 core
    uniform vec3 uColor;
    uniform float uAlpha;
    out vec4 FragColor;
    void main() { FragColor = vec4(uColor, uAlpha); }
  )";
  shader.build(vs, fs);

  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  return true;
}

void initializeApplicationState(ApplicationState& state) {
  state.wind = {DataConfig::WIND_DIRECTION, DataConfig::WIND_SPEED, DisplayLayout::LEFT_OFFSET, DataConfig::WIND_Y, WHITE.r, WHITE.g, WHITE.b};
  state.gps = {DataConfig::GPS_STATUS, DisplayLayout::LEFT_OFFSET, DataConfig::GPS_Y, WHITE.r, WHITE.g, WHITE.b};
  state.ias = {DataConfig::IAS_VALUE, DisplayLayout::LEFT_OFFSET, DataConfig::IAS_Y, GRAY.r, GRAY.g, GRAY.b, GRAY.r, GRAY.g, GRAY.b};
  state.course = {DataConfig::COURSE_COG, DataConfig::COURSE_GS, DisplayLayout::RIGHT_OFFSET, DataConfig::COURSE_Y_COG, DataConfig::COURSE_Y_GS, WHITE.r, WHITE.g, WHITE.b};
  state.alt = {DataConfig::ALT_VALUE, DisplayLayout::RIGHT_OFFSET, DataConfig::ALT_Y, GRAY.r, GRAY.g, GRAY.b, GRAY.r, GRAY.g, GRAY.b};
  state.wp_left = {state.wp_left_bearing, DataConfig::WP_LEFT_DISTANCE, DataConfig::WP_LEFT_NAME, DataConfig::WP_LEFT_RUNWAY, DataConfig::WP_LEFT_APP_FREQ, DataConfig::WP_LEFT_INFO_FREQ, DisplayLayout::LEFT_OFFSET, DataConfig::WP_LEFT_Y, YELLOW.r, YELLOW.g, YELLOW.b};
  state.wp_right = {state.wp_right_bearing, DataConfig::WP_RIGHT_DISTANCE, DataConfig::WP_RIGHT_NAME, DataConfig::WP_RIGHT_RUNWAY, DataConfig::WP_RIGHT_APP_FREQ, DataConfig::WP_RIGHT_INFO_FREQ, DisplayLayout::RIGHT_OFFSET, DataConfig::WP_RIGHT_Y, GREEN.r, GREEN.g, GREEN.b};
  state.bug = {state.bug_heading, DataConfig::BUG_X, DataConfig::BUG_Y, MAGENTA.r, MAGENTA.g, MAGENTA.b};
}

int main() {
  GLFWwindow* window = nullptr;
  CompasRenderer compas;
  TtfTextRenderer fonts[FONT_COUNT];
  Shader shader;

  if (!initializeApplication(window, compas, fonts, shader)) {
    return 1;
  }

  HsiUiRenderer ui_renderer(fonts[INFO_VALUE], fonts[INFO_LABEL],
                            fonts[WAYPOINT_NAME], fonts[WAYPOINT_BEARING],
                            fonts[WAYPOINT_INFO], fonts[IAS_ALT_VALUE], fonts[IAS_ALT_LABEL]);

  ApplicationState state;
  initializeApplicationState(state);
  compas.setHeadingDeg(state.heading_deg);

  InputHandler input_handler;
  RenderEngine render_engine(shader);

  double last_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    double current_time = glfwGetTime();
    float delta_time = (float)(current_time - last_time);
    last_time = current_time;

    input_handler.processInput(window, state.heading_deg, state.bug_heading,
                               state.wp_left_bearing, state.wp_right_bearing, compas, delta_time);
    state.updateFromHeading();

    render_engine.renderFrame(compas, fonts, ui_renderer, state);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}