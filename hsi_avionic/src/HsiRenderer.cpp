#include "gfx/HsiRenderer.hpp"
#include <cmath>
#include <glad/glad.h>

void HsiRenderer::drawTextAtBearingRadial(TtfTextRenderer& ttf,
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

void HsiRenderer::drawHeadingBox(float x, float y, float width, float height) {
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