#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "compas/CompasRenderer.hpp"

class InputHandler {
public:
  void processInput(GLFWwindow* window, float& heading_deg, float& bug_heading,
                    float& wp_left_bearing, float& wp_right_bearing,
                    CompasRenderer& compas, float delta_time);

private:
  bool key3_pressed_ = false;

  void handleHeadingAdjustment(GLFWwindow* window, float& heading_deg, 
                               CompasRenderer& compas, float delta_time);
  void handleBugHeadingAdjustment(GLFWwindow* window, float& bug_heading, float delta_time);
  void handleWaypointAdjustment(GLFWwindow* window, float& wp_left_bearing, 
                                float& wp_right_bearing, float delta_time);
  void handlePerpLineControl(GLFWwindow* window, CompasRenderer& compas);
  void handleToFromToggle(GLFWwindow* window, CompasRenderer& compas);
};