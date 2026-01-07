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

// Tambah fungsi baru untuk text alignment
void drawTextLeftAligned(TtfTextRenderer& ttf, const char* text, float x, float y, 
                        float r, float g, float b) {
  // Render text left-aligned (x adalah posisi kiri)
  // Asumsi: drawTextCenteredNDC menggunakan center, jadi kita offset
  ttf.drawTextCenteredNDC(text, x + 0.08f, y, r, g, b);  // Offset untuk left align
}

void drawTextRightAligned(TtfTextRenderer& ttf, const char* text, float x, float y, 
                         float r, float g, float b) {
  // Render text right-aligned (x adalah posisi kanan)
  ttf.drawTextCenteredNDC(text, x - 0.08f, y, r, g, b);  // Offset untuk right align
}

// Fungsi alignment terhadap FRAME (tepi layar) - PERBAIKAN
void drawTextLeftAlignedFrame(TtfTextRenderer& ttf, const char* text, float offset_from_left, float y, 
                              float r, float g, float b) {
  // offset_from_left: jarak dari tepi kiri (-1.0 ke kiri, 0 center)
  // Offset agar text bermula dari left_offset, bukan center
  // Estimasi: setiap karakter ~0.025 width (untuk text centered)
  int text_length = strlen(text);
  float text_width_estimate = text_length * 0.025f;
  
  // Posisi agar text dimulai dari offset_from_left
  float adjusted_x = offset_from_left + text_width_estimate;
  ttf.drawTextCenteredNDC(text, adjusted_x, y, r, g, b);
}

void drawTextRightAlignedFrame(TtfTextRenderer& ttf, const char* text, float offset_from_right, float y, 
                               float r, float g, float b) {
  // offset_from_right: jarak dari tepi kanan (1.0 ke kanan, 0 center)
  // Offset agar text berakhir di right_offset
  int text_length = strlen(text);
  float text_width_estimate = text_length * 0.025f;
  
  // Posisi agar text berakhir di offset_from_right
  float adjusted_x = offset_from_right - text_width_estimate;
  ttf.drawTextCenteredNDC(text, adjusted_x, y, r, g, b);
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
  if (!ttf_cardinal.init("../assets/fonts/DejaVuSans-Bold.ttf", 40.0f)) {
    std::cerr << "TTF cardinal init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  TtfTextRenderer ttf_numbers;
  if (!ttf_numbers.init("../assets/fonts/DejaVuSans-Bold.ttf", 28.0f)) {
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
  
  // Waypoint 1 (EDAB) - LEFT
  struct Waypoint {
    float bearing;
    const char* name;
    float distance;
    const char* runway;
    const char* app_freq;
    const char* info_freq;
  };
  
  Waypoint wp_left = {347, "EDAB", 861.9f, "BRUTZEM", "(APP)125.875", "(INF)120.605"};
  Waypoint wp_right = {324, "EDD1", 1000, "LSZH", "(ACC)119.120", "(ACC)134.000"};
  
  // Bug heading
  float bug_heading = 349;

  float heading_deg = 0.0f;
  compas.setHeadingDeg(heading_deg);

  glViewport(0, 0, kWidth, kHeight);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  double last_time = glfwGetTime();
  float aspect_fix = (float)kHeight / (float)kWidth;
  float cardinal_radius = 0.56f;
  float number_radius = 0.60f;

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
    const char* name, *runway, *app_freq, *info_freq;
    float x, y_start;
    float r, g, b;
  } wp_left_group = {
    wp_left.bearing, wp_left.distance,
    wp_left.name, wp_left.runway, wp_left.app_freq, wp_left.info_freq,
    -0.70f, -0.40f,  // Jarak dari tepi kiri: -0.95f
    R_yellow, G_yellow, B_yellow
  };

  // RIGHT SIDE - WAYPOINT EDD1 (Green)
  struct {
    float bearing, distance;
    const char* name, *runway, *app_freq, *info_freq;
    float x, y_start;
    float r, g, b;
  } wp_right_group = {
    wp_right.bearing, wp_right.distance,
    wp_right.name, wp_right.runway, wp_right.app_freq, wp_right.info_freq,
    0.70f, -0.40f,  // Jarak dari tepi kanan: 0.95f
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

  while (!glfwWindowShouldClose(window)) {
    double current_time = glfwGetTime();
    float delta_time = (float)(current_time - last_time);
    last_time = current_time;

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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    compas.drawRing();
    compas.drawTicks();
    compas.drawCardinalMarkers();
    compas.drawHeadingIndicator();

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
    Shader shader;
    const char* vs = R"(#version 330 core
      layout (location = 0) in vec2 aPos;
      void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
    )";
    const char* fs = R"(#version 330 core
      out vec4 FragColor;
      void main() { FragColor = vec4(1.0, 1.0, 1.0, 1.0); }
    )";
    shader.build(vs, fs);
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
    snprintf(wp_left_bearing_str, sizeof(wp_left_bearing_str), "%.0f", wp_left_group.bearing);
    ttf_info.drawTextLeftAligned(wp_left_bearing_str, left_offset, wp_left_group.y_start,
                                 wp_left_group.r, wp_left_group.g, wp_left_group.b);
    ttf_info.drawTextLeftAligned(wp_left_group.name, left_offset, wp_left_group.y_start - 0.10f,
                                 wp_left_group.r, wp_left_group.g, wp_left_group.b);
    snprintf(wp_left_dist_str, sizeof(wp_left_dist_str), "%.1f km", wp_left_group.distance);
    ttf_info_label.drawTextLeftAligned(wp_left_dist_str, left_offset, wp_left_group.y_start - 0.20f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);
    ttf_info_label.drawTextLeftAligned(wp_left_group.runway, left_offset, wp_left_group.y_start - 0.30f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);
    ttf_info_label.drawTextLeftAligned(wp_left_group.app_freq, left_offset, wp_left_group.y_start - 0.40f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);
    ttf_info_label.drawTextLeftAligned(wp_left_group.info_freq, left_offset, wp_left_group.y_start - 0.50f,
                                       wp_left_group.r, wp_left_group.g, wp_left_group.b);

    // WAYPOINT RIGHT (EDD1) - Right aligned
    snprintf(wp_right_bearing_str, sizeof(wp_right_bearing_str), "%.0f", wp_right_group.bearing);
    ttf_info.drawTextRightAligned(wp_right_bearing_str, right_offset, wp_right_group.y_start,
                                  wp_right_group.r, wp_right_group.g, wp_right_group.b);
    ttf_info.drawTextRightAligned(wp_right_group.name, right_offset, wp_right_group.y_start - 0.10f,
                                  wp_right_group.r, wp_right_group.g, wp_right_group.b);
    snprintf(wp_right_dist_str, sizeof(wp_right_dist_str), ">%.0f km", wp_right_group.distance);
    ttf_info_label.drawTextRightAligned(wp_right_dist_str, right_offset, wp_right_group.y_start - 0.20f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);
    ttf_info_label.drawTextRightAligned(wp_right_group.runway, right_offset, wp_right_group.y_start - 0.30f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);
    ttf_info_label.drawTextRightAligned(wp_right_group.app_freq, right_offset, wp_right_group.y_start - 0.40f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);
    ttf_info_label.drawTextRightAligned(wp_right_group.info_freq, right_offset, wp_right_group.y_start - 0.50f,
                                        wp_right_group.r, wp_right_group.g, wp_right_group.b);

    // BUG HEADING (Center, Magenta)
    snprintf(bug_str, sizeof(bug_str), "BUG  %.0f", bug_group.value);
    ttf_info.drawTextCenteredNDC(bug_str, bug_group.x, bug_group.y,
                                 bug_group.r, bug_group.g, bug_group.b);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
