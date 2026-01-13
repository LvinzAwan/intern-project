#include "core/InputHandler.hpp"
#include "config/AppConfig.hpp"

void InputHandler::processInput(GLFWwindow* window, float& heading_deg, float& bug_heading,
                                 float& wp_left_bearing, float& wp_right_bearing,
                                 CompasRenderer& compas, float delta_time) {
  handleHeadingAdjustment(window, heading_deg, compas, delta_time);
  handleBugHeadingAdjustment(window, bug_heading, delta_time);
  handleWaypointAdjustment(window, wp_left_bearing, wp_right_bearing, delta_time);
  handlePerpLineControl(window, compas);
  handleToFromToggle(window, compas);

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void InputHandler::handleHeadingAdjustment(GLFWwindow* window, float& heading_deg,
                                           CompasRenderer& compas, float delta_time) {
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
}

void InputHandler::handleBugHeadingAdjustment(GLFWwindow* window, float& bug_heading, float delta_time) {
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    bug_heading += 90.0f * delta_time;
    if (bug_heading >= 360.0f) bug_heading -= 360.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    bug_heading -= 90.0f * delta_time;
    if (bug_heading < 0.0f) bug_heading += 360.0f;
  }
}

void InputHandler::handleWaypointAdjustment(GLFWwindow* window, float& wp_left_bearing,
                                            float& wp_right_bearing, float delta_time) {
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    wp_left_bearing += 90.0f * delta_time;
    if (wp_left_bearing >= 360.0f) wp_left_bearing -= 360.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    wp_left_bearing -= 90.0f * delta_time;
    if (wp_left_bearing < 0.0f) wp_left_bearing += 360.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    wp_right_bearing += 90.0f * delta_time;
    if (wp_right_bearing >= 360.0f) wp_right_bearing -= 360.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    wp_right_bearing -= 90.0f * delta_time;
    if (wp_right_bearing < 0.0f) wp_right_bearing += 360.0f;
  }
}

void InputHandler::handlePerpLineControl(GLFWwindow* window, CompasRenderer& compas) {
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
    float new_offset = compas.getPerpLineOffset() - PerpLineConfig::SENSITIVITY;
    if (new_offset >= PerpLineConfig::MAX_OFFSET_LEFT) {
      compas.updatePerpLineOffset(-PerpLineConfig::SENSITIVITY);
    }
  }
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
    float new_offset = compas.getPerpLineOffset() + PerpLineConfig::SENSITIVITY;
    if (new_offset <= PerpLineConfig::MAX_OFFSET_RIGHT) {
      compas.updatePerpLineOffset(PerpLineConfig::SENSITIVITY);
    }
  }
}

void InputHandler::handleToFromToggle(GLFWwindow* window, CompasRenderer& compas) {
  if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
    if (!key3_pressed_) {
      compas.toggleToFromFlag();
      key3_pressed_ = true;
    }
  } else {
    key3_pressed_ = false;
  }
}