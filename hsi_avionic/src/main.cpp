#include <iostream>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "compas/CompasRenderer.hpp"
#include "gfx/TextRenderer.hpp"
#include "gfx/TtfTextRenderer.hpp"



static void framebuffer_size_callback(GLFWwindow*, int w, int h) {
  glViewport(0, 0, w, h);
}

static void rotate2d(float& x, float& y, float rad) {
  float cx = x, cy = y;
  float c = std::cos(rad);
  float s = std::sin(rad);
  x = cx * c - cy * s;
  y = cx * s + cy * c;
}

static void drawCardinalTTF(TtfTextRenderer& ttf,
                            const char* label,
                            float deg_card,
                            float heading_deg,
                            float radius,
                            float aspect_fix,
                            float r, float g, float b) {
  float a = deg_card * 3.1415926535f / 180.0f;

  // 1) titik di lingkaran (belum di-aspect)
  float x = std::cos(a) * radius;
  float y = std::sin(a) * radius;

  // 2) card muter kebalikan heading (rotate dulu)
  float h = (-heading_deg) * 3.1415926535f / 180.0f;
  rotate2d(x, y, h);

  // 3) baru koreksi aspect (biar lingkaran tidak oval di 800x600)
  x *= aspect_fix;

  // 4) teks tetap tegak
  ttf.drawTextCenteredNDC(label, x, y, r, g, b);
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

 TextRenderer text;
  if (!text.init()) {
     std::cerr << "TextRenderer init failed\n";
     glfwDestroyWindow(window);
     glfwTerminate();
     return 1;
 }

 TtfTextRenderer ttf;
 if (!ttf.init("../assets/fonts/DejaVuSans-Bold.ttf", 56.0f)) {
    std::cerr << "TTF init failed\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
 }

  float heading_deg = 70.0f;
  compas.setHeadingDeg(heading_deg);

  glViewport(0, 0, kWidth, kHeight);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    compas.setHeadingDeg(heading_deg);
    compas.drawRing();  
    compas.drawTicks();
    compas.drawCardinalMarkers();

   float aspect_fix = (float)kHeight / (float)kWidth;
   float text_radius = 0.54f;
   float R = 1.0f, G = 1.0f, B = 0.2f;

   // tidak pakai heading di sini, karena card sudah ikut heading
   drawCardinalTTF(ttf, "N",  90.0f, 0.0f, text_radius, aspect_fix, R, G, B);
   drawCardinalTTF(ttf, "E",   0.0f, 0.0f, text_radius, aspect_fix, R, G, B);
   drawCardinalTTF(ttf, "S", -90.0f, 0.0f, text_radius, aspect_fix, R, G, B);
   drawCardinalTTF(ttf, "W", 180.0f, 0.0f, text_radius, aspect_fix, R, G, B);

   glfwSwapBuffers(window);
   glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
