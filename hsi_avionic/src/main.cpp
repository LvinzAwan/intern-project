#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "compas/CompasRenderer.hpp"
#include "gfx/TtfTextRenderer.hpp"
#include "gfx/HsiRenderer.hpp"
#include "gfx/Shader.hpp"
#include "ui/HsiUiRenderer.hpp"
#include "data/HsiData.hpp"

// Constants
namespace {
  const int kWidth = 800;
  const int kHeight = 600;
  const float kAspectFix = (float)kHeight / (float)kWidth;
  const float kCardinalRadius = 0.55f;
  const float kNumberRadius = 0.57f;
  const float kLeftOffset = -0.92f;
  const float kRightOffset = 0.92f;
  
  // Colors
  const float R_YELLOW = 1.0f, G_YELLOW = 1.0f, B_YELLOW = 0.0f;
  const float R_WHITE = 1.0f, G_WHITE = 1.0f, B_WHITE = 1.0f;
  const float R_GREEN = 0.0f, G_GREEN = 1.0f, B_GREEN = 0.0f;
  const float R_GRAY = 0.5f, G_GRAY = 0.5f, B_GRAY = 0.5f;
  const float R_MAGENTA = 1.0f, G_MAGENTA = 0.0f, B_MAGENTA = 1.0f; // Perbaikan warna magenta
}

namespace DataConstants {
  // Wind data
  const float WIND_DIRECTION = 53.0f;
  const float WIND_SPEED = 180.0f;
  const float WIND_Y = 0.90f;

  // GPS data
  const char* GPS_STATUS = "GPS OK";
  const float GPS_Y = 0.80f;

  // IAS (Indicated Air Speed) data
  const float IAS_VALUE = 181.0f;
  const float IAS_Y = -0.10f;

  // Course data
  const float COURSE_COG = 45.0f;
  const float COURSE_GS = 181.0f;
  const float COURSE_Y_COG = 0.90f;
  const float COURSE_Y_GS = 0.80f;

  // Altitude data
  const float ALT_VALUE = -840.0f;
  const float ALT_Y = -0.10f;

  // Left waypoint data
  const float WP_LEFT_DISTANCE = 861.9f;
  const char* WP_LEFT_NAME = "EDAB";
  const char* WP_LEFT_RUNWAY = "BRUTZEM";
  const float WP_LEFT_APP_FREQ = 125.875f;
  const float WP_LEFT_INFO_FREQ = 120.605f;
  const float WP_LEFT_Y = -0.40f;

  // Right waypoint data
  const float WP_RIGHT_DISTANCE = 1000.0f;
  const char* WP_RIGHT_NAME = "EDD1";
  const char* WP_RIGHT_RUNWAY = "LSZH";
  const float WP_RIGHT_APP_FREQ = 119.120f;
  const float WP_RIGHT_INFO_FREQ = 134.000f;
  const float WP_RIGHT_Y = -0.40f;

  // Bug indicator data
  const float BUG_X = 0.0f;
  const float BUG_Y = -0.90f;
}

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
  glViewport(0, 0, w, h);
}

bool initializeFonts(TtfTextRenderer fonts[], const char* paths[], const float sizes[], int count) {
  for (int i = 0; i < count; ++i) {
    if (!fonts[i].init(paths[i], sizes[i])) {
      std::cerr << "Failed to init font: " << paths[i] << "\n";
      return false;
    }
  }
  return true;
}

void handleInput(GLFWwindow* window, float& heading_deg, float& bug_heading,
                 float& wp_left_bearing, float& wp_right_bearing,
                 CompasRenderer& compas, float delta_time) {
  // Heading control
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    heading_deg += 90.0f * delta_time;
    if (heading_deg >= 360.0f) heading_deg -= 360.0f;
    compas.setHeadingDeg(heading_deg);
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    heading_deg -= 90.0f * delta_time;
    if (heading_deg < 0.0f) heading_deg += 360.0f;
    compas.setHeadingDeg(heading_deg);
  }

  // Bug heading control
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    bug_heading += 90.0f * delta_time;
    if (bug_heading >= 360.0f) bug_heading -= 360.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    bug_heading -= 90.0f * delta_time;
    if (bug_heading < 0.0f) bug_heading += 360.0f;
  }

  // Waypoint left bearing control
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    wp_left_bearing += 90.0f * delta_time;
    if (wp_left_bearing >= 360.0f) wp_left_bearing -= 360.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    wp_left_bearing -= 90.0f * delta_time;
    if (wp_left_bearing < 0.0f) wp_left_bearing += 360.0f;
  }

  // Waypoint right bearing control
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    wp_right_bearing += 90.0f * delta_time;
    if (wp_right_bearing >= 360.0f) wp_right_bearing -= 360.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    wp_right_bearing -= 90.0f * delta_time;
    if (wp_right_bearing < 0.0f) wp_right_bearing += 360.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void renderCompass(CompasRenderer& compas, TtfTextRenderer& ttf_cardinal,
                   TtfTextRenderer& ttf_numbers, float heading_deg) {
  compas.drawRing();
  compas.drawTicks();
  compas.drawCardinalMarkers();
  compas.drawHeadingIndicator();
  compas.drawAircraftSymbol(kAspectFix);

  // Cardinal letters
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "N", 0.0f, kCardinalRadius, kAspectFix, heading_deg, R_YELLOW, G_YELLOW, B_YELLOW);
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "E", 90.0f, kCardinalRadius, kAspectFix, heading_deg, R_YELLOW, G_YELLOW, B_YELLOW);
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "S", 180.0f, kCardinalRadius, kAspectFix, heading_deg, R_YELLOW, G_YELLOW, B_YELLOW);
  HsiRenderer::drawTextAtBearingRadial(ttf_cardinal, "W", 270.0f, kCardinalRadius, kAspectFix, heading_deg, R_YELLOW, G_YELLOW, B_YELLOW);

  // Bearing numbers
  const int bearings[] = {30, 60, 120, 150, 210, 240, 300, 330};
  for (int bearing : bearings) {
    char label[16];
    snprintf(label, sizeof(label), "%d", bearing / 10);
    HsiRenderer::drawTextAtBearingRadial(ttf_numbers, label, (float)bearing, kNumberRadius, kAspectFix, heading_deg, R_WHITE, G_WHITE, B_WHITE);
  }
}

void renderHeadingDisplay(TtfTextRenderer& ttf_heading, TtfTextRenderer& ttf_label,
                          TtfTextRenderer& ttf_symbol, float heading_deg, Shader& shader) {
  shader.use();
  HsiRenderer::drawHeadingBox(0.0f, 0.82f, 0.18f, 0.10f);

  char heading_str[16];
  float display_heading = 360.0f - (int)heading_deg;
  if (display_heading >= 360.0f) display_heading -= 360.0f;
  snprintf(heading_str, sizeof(heading_str), "%03d", (int)display_heading);
  ttf_heading.drawTextCenteredNDC(heading_str, 0.0f, 0.87f, R_YELLOW, G_YELLOW, B_YELLOW); // Kuning

  ttf_label.drawTextCenteredNDC("HDG", -0.17f, 0.87f, R_WHITE, G_WHITE, B_WHITE);
  ttf_symbol.drawTextCenteredNDC("o", 0.11f, 0.86f, R_WHITE, G_WHITE, B_WHITE);
  ttf_label.drawTextCenteredNDC("M", 0.15f, 0.87f, R_WHITE, G_WHITE, B_WHITE);
}

int main() {
  if (!glfwInit()) {
    std::cerr << "GLFW init failed\n";
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "HSI (Horizontal Situation Indicator)", nullptr, nullptr);
  if (!window) {
    std::cerr << "Create window failed\n";
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowSizeLimits(window, kWidth, kHeight, kWidth, kHeight);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "GLAD load failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  // Initialize renderer
  CompasRenderer compas;
  if (!compas.init(kWidth, kHeight)) {
    std::cerr << "CompasRenderer init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  // Initialize fonts
  TtfTextRenderer fonts[7];
  const char* font_paths[] = {
    "../assets/fonts/DejaVuSans-Bold.ttf",
    "../assets/fonts/DejaVuSans-Bold.ttf",
    "../assets/fonts/DejaVuSans-Bold.ttf",
    "../assets/fonts/DejaVuSans-Bold.ttf",
    "../assets/fonts/DejaVuSans-Bold.ttf",
    "../assets/fonts/ArialMdm.ttf",
    "../assets/fonts/ArialMdm.ttf"
  };
  const float font_sizes[] = {56.0f, 40.0f, 36.0f, 32.0f, 20.0f, 36.0f, 34.0f};
  
  if (!initializeFonts(fonts, font_paths, font_sizes, 7)) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer& ttf_cardinal = fonts[0];
  TtfTextRenderer& ttf_numbers = fonts[1];
  TtfTextRenderer& ttf_heading = fonts[2];
  TtfTextRenderer& ttf_heading_label = fonts[3];
  TtfTextRenderer& ttf_heading_symbol = fonts[4];
  TtfTextRenderer& ttf_info = fonts[5];
  TtfTextRenderer& ttf_info_label = fonts[6];

  HsiUiRenderer ui_renderer(ttf_info, ttf_info_label);

  // Initialize data
  float heading_deg = 0.0f;
  float bug_heading = 0.0f;
  float wp_left_bearing = 347.0f;
  float wp_right_bearing = 324.0f;

  compas.setHeadingDeg(heading_deg);

  // ==================== DATA GROUPS INITIALIZATION ====================
  // Initialize data groups with constants for easy modification
  WindGroup wind = {
    DataConstants::WIND_DIRECTION,
    DataConstants::WIND_SPEED,
    kLeftOffset,
    DataConstants::WIND_Y,
    R_WHITE, G_WHITE, B_WHITE
  };

  GpsGroup gps = {
    DataConstants::GPS_STATUS,
    kLeftOffset,
    DataConstants::GPS_Y,
    R_WHITE, G_WHITE, B_WHITE
  };

  IasGroup ias = {
    DataConstants::IAS_VALUE,
    kLeftOffset,
    DataConstants::IAS_Y,
    R_GRAY, G_GRAY, B_GRAY,
    R_GRAY, G_GRAY, B_GRAY
  };

  CourseGroup course = {
    DataConstants::COURSE_COG,
    DataConstants::COURSE_GS,
    kRightOffset,
    DataConstants::COURSE_Y_COG,
    DataConstants::COURSE_Y_GS,
    R_WHITE, G_WHITE, B_WHITE
  };

  AltGroup alt = {
    DataConstants::ALT_VALUE,
    kRightOffset,
    DataConstants::ALT_Y,
    R_GRAY, G_GRAY, B_GRAY,
    R_GRAY, G_GRAY, B_GRAY
  };

  WaypointGroup wp_left = {
    wp_left_bearing,
    DataConstants::WP_LEFT_DISTANCE,
    DataConstants::WP_LEFT_NAME,
    DataConstants::WP_LEFT_RUNWAY,
    DataConstants::WP_LEFT_APP_FREQ,
    DataConstants::WP_LEFT_INFO_FREQ,
    kLeftOffset,
    DataConstants::WP_LEFT_Y,
    R_YELLOW, G_YELLOW, B_YELLOW
  };

  WaypointGroup wp_right = {
    wp_right_bearing,
    DataConstants::WP_RIGHT_DISTANCE,
    DataConstants::WP_RIGHT_NAME,
    DataConstants::WP_RIGHT_RUNWAY,
    DataConstants::WP_RIGHT_APP_FREQ,
    DataConstants::WP_RIGHT_INFO_FREQ,
    kRightOffset,
    DataConstants::WP_RIGHT_Y,
    R_GREEN, G_GREEN, B_GREEN
  };

  BugGroup bug = {
    bug_heading,
    DataConstants::BUG_X,
    DataConstants::BUG_Y,
    R_MAGENTA, G_MAGENTA, B_MAGENTA
  };

  // Setup shader
  Shader shader;
  const char* vs = R"(#version 330 core
    layout (location = 0) in vec2 aPos;
    void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
  )";
  const char* fs = R"(#version 330 core
    uniform vec3 uColor;
    out vec4 FragColor;
    void main() { FragColor = vec4(uColor, 1.0); }
  )";
  shader.build(vs, fs);

  glViewport(0, 0, kWidth, kHeight);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  double last_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    double current_time = glfwGetTime();
    float delta_time = (float)(current_time - last_time);
    last_time = current_time;

    handleInput(window, heading_deg, bug_heading, wp_left_bearing, wp_right_bearing, compas, delta_time);

    // Update data
    bug.value = bug_heading;
    wp_left.bearing = wp_left_bearing;
    wp_right.bearing = wp_right_bearing;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderCompass(compas, ttf_cardinal, ttf_numbers, heading_deg);
    renderHeadingDisplay(ttf_heading, ttf_heading_label, ttf_heading_symbol, heading_deg, shader);

    // Render UI elements
    ui_renderer.renderWindGroup(wind, kLeftOffset);
    ui_renderer.renderGpsGroup(gps, kLeftOffset);
    ui_renderer.renderIasGroup(ias, kLeftOffset);
    ui_renderer.renderCourseGroup(course, kRightOffset);
    ui_renderer.renderAltGroup(alt, kRightOffset);
    ui_renderer.renderWaypointLeft(wp_left, kLeftOffset);
    ui_renderer.renderWaypointRight(wp_right, kRightOffset);
    ui_renderer.renderBugGroup(bug);

    // Draw waypoint arrows
    shader.use();
    glUniform3f(glGetUniformLocation(shader.id(), "uColor"), R_YELLOW, G_YELLOW, B_YELLOW);
    compas.drawWaypointArrowSingle(wp_left_bearing, heading_deg, kAspectFix, 0.50f);
    
    glUniform3f(glGetUniformLocation(shader.id(), "uColor"), R_GREEN, G_GREEN, B_GREEN);
    compas.drawWaypointArrowDouble(wp_right_bearing, heading_deg, kAspectFix, 0.50f);

    glUniform3f(glGetUniformLocation(shader.id(), "uColor"), R_MAGENTA, G_MAGENTA, B_MAGENTA);
    compas.drawBugTriangle(bug_heading, heading_deg, kAspectFix, 0.73f);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}