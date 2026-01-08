#include <iostream>
#include <cmath>
#include <cstring> 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "compas/CompasRenderer.hpp"
#include "gfx/TtfTextRenderer.hpp"

static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
  glViewport(0, 0, w, h);
}

static void drawTextAtBearingRadial(TtfTextRenderer& ttf,
                                    const char* label,
                                    float bearing_deg,
                                    float radius,
                                    float aspect_fix,
                                    float heading_deg,
                                    float r, float g, float b) {
  float rotated_bearing = bearing_deg + heading_deg;
  float rotated_rad = rotated_bearing * 3.1415926535f / 180.0f;
  
  float x = std::sin(rotated_rad) * radius;
  float y = std::cos(rotated_rad) * radius;
  x *= aspect_fix;

  float text_rotation = -rotated_bearing;
  
  ttf.drawTextCenteredNDCRotated(label, x, y, text_rotation, r, g, b);
}

void drawHeadingBox(float x, float y, float width, float height) {
  float aspect_fix = 600.0f / 800.0f;
  
  float vertices[] = {
    x - width/2, y - height/2,
    x + width/2, y - height/2,
    x + width/2, y + height/2,
    x - width/2, y + height/2
  };
  
  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  glLineWidth(2.0f);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
  
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
}

// ==================== INFO GROUP STRUCTURE ====================
struct InfoGroup {
  const char* label;
  float value;
  float x;
  float y;
  float r, g, b;           // Color untuk value
  float label_r, label_g, label_b;  // Color untuk label
  TtfTextRenderer* label_font;
  TtfTextRenderer* value_font;
};

struct InfoDisplay {
  const char* title;
  float x;
  float y;
  float r, g, b;           // Color
  TtfTextRenderer* font;
};

struct WaypointGroup {
  float bearing;
  const char* name;
  float distance;
  const char* runway;
  const char* app_freq;
  const char* info_freq;
  
  float x;
  float y_start;
  
  float bearing_r, bearing_g, bearing_b;
  float name_r, name_g, name_b;
  float info_r, info_g, info_b;
  
  TtfTextRenderer* bearing_font;
  TtfTextRenderer* info_font;
};

int main() {
  if (!glfwInit()) {
    std::cerr << "GLFW init failed\n";
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  const int kWidth = 800;
  const int kHeight = 600;

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

  CompasRenderer compas;
  if (!compas.init(kWidth, kHeight)) {
    std::cerr << "CompasRenderer init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_cardinal;
  if (!ttf_cardinal.init("../assets/fonts/DejaVuSans-Bold.ttf", 56.0f)) {
    std::cerr << "TTF cardinal init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_numbers;
  if (!ttf_numbers.init("../assets/fonts/DejaVuSans-Bold.ttf", 40.0f)) {
    std::cerr << "TTF numbers init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_heading;
  if (!ttf_heading.init("../assets/fonts/DejaVuSans-Bold.ttf", 36.0f)) {
    std::cerr << "TTF heading init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_heading_label;
  if (!ttf_heading_label.init("../assets/fonts/DejaVuSans-Bold.ttf", 32.0f)) {
    std::cerr << "TTF heading label init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_heading_label_simbol;
  if (!ttf_heading_label_simbol.init("../assets/fonts/DejaVuSans-Bold.ttf", 20.0f)) {
    std::cerr << "TTF heading label simbol init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_info;
  if (!ttf_info.init("../assets/fonts/ArialMdm.ttf", 36.0f)) {
    std::cerr << "TTF info init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_info_label;
  if (!ttf_info_label.init("../assets/fonts/ArialMdm.ttf", 34.0f)) {
    std::cerr << "TTF info label init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  // ==================== VARIABEL DATA ====================
  // Wind data
  float wind_speed = 180;
  float wind_direction = 53;
  
  // IAS (Indicated Air Speed)
  float ias_speed = 181;
  
  // COG (Course Over Ground) & GS (Ground Speed)
  float cog_heading = 45;
  float gs_speed = 181;
  
  // ALT (Altitude)
  float altitude = -840;
  
  // GPS Status
  const char* gps_status = "GPS OK";
  
  // WAYPOINT LEFT (EDAB)
  float wp_left_bearing = 347;
  float wp_left_distance = 861.9f;
  const char* wp_left_name = "EDAB";
  const char* wp_left_runway = "BRUTZEM";
  float wp_left_app_freq = 125.875f;
  float wp_left_info_freq = 120.605f;
  
  // WAYPOINT RIGHT (EDD1)
  float wp_right_bearing = 324;
  float wp_right_distance = 1000;
  const char* wp_right_name = "EDD1";
  const char* wp_right_runway = "LSZH";
  float wp_right_app_freq = 119.120f;
  float wp_right_info_freq = 134.000f;
  
  // Bug heading
  // float bug_heading = 349;
   float bug_heading = 0;

  float heading_deg = 0.0f;
  compas.setHeadingDeg(heading_deg);

  glViewport(0, 0, kWidth, kHeight);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  double last_time = glfwGetTime();
  float aspect_fix = (float)kHeight / (float)kWidth;
  float cardinal_radius = 0.55f;
  float number_radius = 0.57f;

  float R_yellow = 1.0f, G_yellow = 1.0f, B_yellow = 0.0f;
  float R_white = 1.0f, G_white = 1.0f, B_white = 1.0f;
  float R_green = 0.0f, G_green = 1.0f, B_green = 0.0f;
  float R_gray = 0.5f, G_gray = 0.5f, B_gray = 0.5f;
  float R_magenta = 1.0f, G_magenta = 0.5f, B_magenta = 1.0f;

  // ==================== KONFIGURASI INFO GROUPS ====================
  // LEFT SIDE - WIND (White, Left Aligned)
  struct {
    float direction, speed;
    float x, y;
    float r, g, b;
  } wind_group = {
    wind_direction, wind_speed,
    -0.70f, 0.90f,  // Jarak dari tepi kiri: -0.95f (bisa diubah)
    R_white, G_white, B_white
  };

  // LEFT SIDE - GPS (White, Left Aligned)
  struct {
    const char* status;
    float x, y;
    float r, g, b;
  } gps_group = {
    gps_status,
    -0.70f, 0.80f,  // Jarak dari tepi kiri: -0.95f
    R_white, G_white, B_white
  };

  // LEFT SIDE - IAS (Gray label, Yellow value)
  struct {
    float value;
    float x, y;
    float label_r, label_g, label_b;
    float value_r, value_g, value_b;
  } ias_group = {
    ias_speed,
    -0.70f, -0.10f,  // Jarak dari tepi kiri: -0.95f
    R_gray, G_gray, B_gray,
    R_gray, G_gray, B_gray
  };

  // RIGHT SIDE - COG/GS (White)
  struct {
    float cog_value, gs_value;
    float x, y_cog, y_gs;
    float r, g, b;
  } course_group = {
    cog_heading, gs_speed,
    0.70f, 0.90f, 0.80f,  // Jarak dari tepi kanan: 0.95f
    R_white, G_white, B_white
  };

  // RIGHT SIDE - ALT (Gray label, Green value)
  struct {
    float value;
    float x, y;
    float label_r, label_g, label_b;
    float value_r, value_g, value_b;
  } alt_group = {
    altitude,
    0.70f, -0.10,  // Jarak dari tepi kanan: 0.95f
    R_gray, G_gray, B_gray,
    R_gray, G_gray, B_gray
  };

  // LEFT SIDE - WAYPOINT EDAB (Yellow)
  struct {
    float bearing, distance;
    const char* name, *runway;
    float app_freq, info_freq;  // Ubah dari const char* menjadi float
    float x, y_start;
    float r, g, b;
  } wp_left_group = {
    wp_left_bearing, wp_left_distance,
    wp_left_name, wp_left_runway, wp_left_app_freq, wp_left_info_freq,
    -0.70f, -0.40f,
    R_yellow, G_yellow, B_yellow
  };

  // RIGHT SIDE - WAYPOINT EDD1 (Green)
  struct {
    float bearing, distance;
    const char* name, *runway;
    float app_freq, info_freq;  // Ubah dari const char* menjadi float
    float x, y_start;
    float r, g, b;
  } wp_right_group = {
    wp_right_bearing, wp_right_distance,
    wp_right_name, wp_right_runway, wp_right_app_freq, wp_right_info_freq,
    0.70f, -0.40f,
    R_green, G_green, 0.0f
  };

  // BUG HEADING (Magenta)
  struct {
    float value;
    float x, y;
    float r, g, b;
  } bug_group = {
    bug_heading,
    0.0f, -0.90f,  // Center alignment
    R_magenta, G_magenta, B_magenta
  };

  // ==================== DEKLARASI BUFFER STRING (SEBELUM LOOP) ====================
  char wind_str[32];
  char ias_str[32];
  char cog_str[32], gs_str[32];
  char alt_str[32];
  char wp_left_bearing_str[32];
  char wp_left_dist_str[32];
  char wp_right_bearing_str[32];
  char wp_right_dist_str[32];
  char bug_str[32];

  // ==================== KONFIGURASI FRAME ALIGNMENT ====================
  // LEFT SIDE OFFSET (jarak dari tepi kiri, -1.0 adalah paling kiri)
  float left_offset = -0.92f;
  
  // RIGHT SIDE OFFSET (jarak dari tepi kanan, 1.0 adalah paling kanan)
  float right_offset = 0.92f;

  // ==================== SETUP SHADER (SEBELUM LOOP) ====================
  Shader shader;
  const char* vs = R"(#version 330 core
    layout (location = 0) in vec2 aPos;
    uniform vec3 uColor;
    void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
  )";
  const char* fs = R"(#version 330 core
    uniform vec3 uColor;
    out vec4 FragColor;
    void main() { FragColor = vec4(uColor, 1.0); }
  )";
  shader.build(vs, fs);

  while (!glfwWindowShouldClose(window)) {
    double current_time = glfwGetTime();
    float delta_time = (float)(current_time - last_time);
    last_time = current_time;

    // Kontrol heading pesawat (LEFT/RIGHT)
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

    // Kontrol bug heading (UP/DOWN)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
      bug_heading += 90.0f * delta_time;
      if (bug_heading >= 360.0f) bug_heading -= 360.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      bug_heading -= 90.0f * delta_time;
      if (bug_heading < 0.0f) bug_heading += 360.0f;
    }

    // Kontrol waypoint LEFT bearing (A/D) - TAMBAHAN BARU
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      wp_left_bearing += 90.0f * delta_time;
      if (wp_left_bearing >= 360.0f) wp_left_bearing -= 360.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      wp_left_bearing -= 90.0f * delta_time;
      if (wp_left_bearing < 0.0f) wp_left_bearing += 360.0f;
    }

    // Kontrol waypoint RIGHT bearing (W/S) - TAMBAHAN BARU
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

    // UPDATE BUG GROUP VALUE
    bug_group.value = bug_heading;

    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    compas.drawRing();
    compas.drawTicks();
    compas.drawCardinalMarkers();
    compas.drawHeadingIndicator();

    // TAMBAHAN: Gambar simbol pesawat di center (SELALU MENGHADAP ATAS)
    compas.drawAircraftSymbol(aspect_fix);

    // Gambar huruf cardinal
    drawTextAtBearingRadial(ttf_cardinal, "N",   0.0f, cardinal_radius, aspect_fix, heading_deg, R_yellow, G_yellow, B_yellow);
    drawTextAtBearingRadial(ttf_cardinal, "E",  90.0f, cardinal_radius, aspect_fix, heading_deg, R_yellow, G_yellow, B_yellow);
    drawTextAtBearingRadial(ttf_cardinal, "S", 180.0f, cardinal_radius, aspect_fix, heading_deg, R_yellow, G_yellow, B_yellow);
    drawTextAtBearingRadial(ttf_cardinal, "W", 270.0f, cardinal_radius, aspect_fix, heading_deg, R_yellow, G_yellow, B_yellow);

    // Gambar angka bearing
    const int bearings[] = {30, 60, 120, 150, 210, 240, 300, 330};
    for (int bearing : bearings) {
      char label[16];
      snprintf(label, sizeof(label), "%d", bearing / 10);
      drawTextAtBearingRadial(ttf_numbers, label, (float)bearing, number_radius, aspect_fix, heading_deg, R_white, G_white, B_white);
    }

    // Gambar box untuk heading indicator
    shader.use();
    drawHeadingBox(0.0f, 0.82f, 0.18f, 0.10f);

    // Gambar heading value di dalam box
    char heading_str[16];
    float display_heading = 360.0f - (int)heading_deg;
    if (display_heading >= 360.0f) display_heading -= 360.0f;
    snprintf(heading_str, sizeof(heading_str), "%03d", (int)display_heading);
    ttf_heading.drawTextCenteredNDC(heading_str, 0.0f, 0.87f, R_yellow, G_yellow, B_yellow);

    // Gambar label "HDG" di kiri
    ttf_heading_label.drawTextCenteredNDC("HDG", -0.17f, 0.87f, R_white, G_white, B_white);

    // Gambar simbol "°" di kanan atas (lebih kecil)
    ttf_heading_label_simbol.drawTextCenteredNDC("o", 0.11f, 0.86f, R_white, G_white, B_white);

    // Gambar huruf "M" di bawahnya
    ttf_heading_label.drawTextCenteredNDC("M", 0.15f, 0.87f, R_white, G_white, B_white);

    // ==================== RENDER INFO GROUPS ====================
    // WIND (Left aligned)
    snprintf(wind_str, sizeof(wind_str), "WIND %03.0f/%.0f", wind_group.direction, wind_group.speed);
    ttf_info_label.drawTextLeftAligned(wind_str, left_offset, wind_group.y, 
                                       wind_group.r, wind_group.g, wind_group.b);

    // GPS (Left aligned)
    ttf_info_label.drawTextLeftAligned(gps_group.status, left_offset, gps_group.y, 
                                       gps_group.r, gps_group.g, gps_group.b);

    // IAS GROUP (Left aligned)
    ttf_info_label.drawTextLeftAligned("IAS", left_offset, ias_group.y + 0.10f,
                                       ias_group.label_r, ias_group.label_g, ias_group.label_b);
    snprintf(ias_str, sizeof(ias_str), "%.0f", ias_group.value);
    ttf_info.drawTextLeftAligned(ias_str, left_offset, ias_group.y,
                                 ias_group.value_r, ias_group.value_g, ias_group.value_b);

    // COG/GS GROUP (Right aligned)
    snprintf(cog_str, sizeof(cog_str), "COG  %.0f", course_group.cog_value);
    snprintf(gs_str, sizeof(gs_str), "GS  %.0f", course_group.gs_value);
    ttf_info_label.drawTextRightAligned(cog_str, right_offset, course_group.y_cog,
                                        course_group.r, course_group.g, course_group.b);
    ttf_info_label.drawTextRightAligned(gs_str, right_offset, course_group.y_gs,
                                        course_group.r, course_group.g, course_group.b);

    // ALT GROUP (Right aligned)
    ttf_info_label.drawTextRightAligned("ALT", right_offset, alt_group.y + 0.10f,
                                        alt_group.label_r, alt_group.label_g, alt_group.label_b);
    snprintf(alt_str, sizeof(alt_str), "%.0f", alt_group.value);
    ttf_info.drawTextRightAligned(alt_str, right_offset, alt_group.y,
                                  alt_group.value_r, alt_group.value_g, alt_group.value_b);

    // WAYPOINT LEFT (EDAB) - Left aligned
    snprintf(wp_left_bearing_str, sizeof(wp_left_bearing_str), "%.0f deg", wp_left_bearing);
    ttf_info.drawTextLeftAligned(wp_left_bearing_str, left_offset, wp_left_group.y_start,
                                 wp_left_group.r, wp_left_group.g, wp_left_group.b);
    ttf_info.drawTextLeftAligned(wp_left_name, left_offset, wp_left_group.y_start - 0.10f,
                                 wp_left_group.r, wp_left_group.g, wp_left_group.b);
    snprintf(wp_left_dist_str, sizeof(wp_left_dist_str), "%.1f km", wp_left_distance);
    ttf_info_label.drawTextLeftAligned(wp_left_dist_str, left_offset, wp_left_group.y_start - 0.20f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);
    ttf_info_label.drawTextLeftAligned(wp_left_runway, left_offset, wp_left_group.y_start - 0.30f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);
    
    // APP FREQ
    char wp_left_app_freq_str[32];
    snprintf(wp_left_app_freq_str, sizeof(wp_left_app_freq_str), "(APP)%.3f", wp_left_app_freq);
    ttf_info_label.drawTextLeftAligned(wp_left_app_freq_str, left_offset, wp_left_group.y_start - 0.40f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);
    
    // INFO FREQ
    char wp_left_info_freq_str[32];
    snprintf(wp_left_info_freq_str, sizeof(wp_left_info_freq_str), "(INF)%.3f", wp_left_info_freq);
    ttf_info_label.drawTextLeftAligned(wp_left_info_freq_str, left_offset, wp_left_group.y_start - 0.50f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);

    // WAYPOINT RIGHT (EDD1) - Right aligned
    snprintf(wp_right_bearing_str, sizeof(wp_right_bearing_str), "%.0f deg", wp_right_bearing);
    ttf_info.drawTextRightAligned(wp_right_bearing_str, right_offset, wp_right_group.y_start,
                                  wp_right_group.r, wp_right_group.g, wp_right_group.b);
    ttf_info.drawTextRightAligned(wp_right_name, right_offset, wp_right_group.y_start - 0.10f,
                                  wp_right_group.r, wp_right_group.g, wp_right_group.b);
    snprintf(wp_right_dist_str, sizeof(wp_right_dist_str), ">%.0f km", wp_right_distance);
    ttf_info_label.drawTextRightAligned(wp_right_dist_str, right_offset, wp_right_group.y_start - 0.20f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);
    ttf_info_label.drawTextRightAligned(wp_right_runway, right_offset, wp_right_group.y_start - 0.30f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);
    
    // APP FREQ
    char wp_right_app_freq_str[32];
    snprintf(wp_right_app_freq_str, sizeof(wp_right_app_freq_str), "(ACC)%.3f", wp_right_app_freq);
    ttf_info_label.drawTextRightAligned(wp_right_app_freq_str, right_offset, wp_right_group.y_start - 0.40f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);
    
    // INFO FREQ
    char wp_right_info_freq_str[32];
    snprintf(wp_right_info_freq_str, sizeof(wp_right_info_freq_str), "(ACC)%.3f", wp_right_info_freq);
    ttf_info_label.drawTextRightAligned(wp_right_info_freq_str, right_offset, wp_right_group.y_start - 0.50f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);
    
    // BUG HEADING (Center, Magenta)
    snprintf(bug_str, sizeof(bug_str), "BUG  %.0f deg", bug_group.value);
    ttf_info.drawTextCenteredNDC(bug_str, bug_group.x, bug_group.y,
                                 bug_group.r, bug_group.g, bug_group.b);

    // GAMBAR WAYPOINT ARROWS
    shader.use();
    
    // WAYPOINT LEFT (EDAB) - Yellow arrow SINGLE
    glUniform3f(glGetUniformLocation(shader.id(), "uColor"), 
                1.0f, 1.0f, 0.0f);  // Yellow
    compas.drawWaypointArrowSingle(wp_left_bearing, heading_deg, aspect_fix, 0.50f);
    
    // WAYPOINT RIGHT (EDD1) - Green arrow DOUBLE
    glUniform3f(glGetUniformLocation(shader.id(), "uColor"), 
                0.0f, 1.0f, 0.0f);  // Green
    compas.drawWaypointArrowDouble(wp_right_bearing, heading_deg, aspect_fix, 0.50f);

    // GAMBAR BUG TRIANGLE 
    shader.use();
    glUniform3f(glGetUniformLocation(shader.id(), "uColor"), 
                1.0f, 0.0f, 1.0f);
    compas.drawBugTriangle(bug_heading, heading_deg, aspect_fix, 0.73f);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
