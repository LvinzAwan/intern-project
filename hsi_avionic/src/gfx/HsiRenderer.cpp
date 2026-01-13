#include "gfx/HsiRenderer.hpp"
#include <cmath>
#include <glad/glad.h>
#include <vector> 

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

void HsiRenderer::drawHeadingBox(float x, float y, float width, float height, 
                                 float r, float g, float b) {
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
  
  glColor3f(r, g, b);
  glLineWidth(4.0f);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
  
  glBindVertexArray(0); 
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
}

void HsiRenderer::drawIasAltFrame(float x, float y, float width, float height,
                                  float r, float g, float b, bool is_left) {
  float x0 = (is_left) ? x : (x - width);
  float x1 = x0 + width;
  float y0 = y - height * 0.5f;
  float y1 = y + height * 0.5f;
  
  float pointer_size = 0.04f;
  float x_pointer = (is_left) ? (x1 + pointer_size) : (x0 - pointer_size);

  float rect_vertices[] = {
    x0, y0,
    x1, y0,
    x1, y1,
    x0, y1
  };
  
  float tri_vertices[] = {
    x_pointer, y,
    (is_left) ? x1 : x0, y0,
    (is_left) ? x1 : x0, y1
  };
  
  GLuint rect_VAO, rect_VBO;
  glGenVertexArrays(1, &rect_VAO);
  glGenBuffers(1, &rect_VBO);
  
  glBindVertexArray(rect_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, rect_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  glLineWidth(2.0f);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
  
  glBindVertexArray(0); 
  glDeleteBuffers(1, &rect_VBO);
  glDeleteVertexArrays(1, &rect_VAO);
  
  GLuint tri_VAO, tri_VBO;
  glGenVertexArrays(1, &tri_VAO);
  glGenBuffers(1, &tri_VBO);
  
  glBindVertexArray(tri_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, tri_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tri_vertices), tri_vertices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  glDrawArrays(GL_TRIANGLES, 0, 3);
  
  glBindVertexArray(0); 
  glDeleteBuffers(1, &tri_VBO);
  glDeleteVertexArrays(1, &tri_VAO);
  
  glLineWidth(1.0f);
}