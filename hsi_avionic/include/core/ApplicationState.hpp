#pragma once

#include "data/HsiData.hpp"

struct ApplicationState {
  //Navigation data
  float heading_deg = 0.0f;
  float bug_heading = 0.0f;
  float wp_left_bearing = 347.0f;
  float wp_right_bearing = 324.0f;

  //Data groups
  WindGroup wind;
  GpsGroup gps;
  IasGroup ias;
  CourseGroup course;
  AltGroup alt;
  WaypointGroup wp_left;
  WaypointGroup wp_right;
  BugGroup bug;

  void updateFromHeading() {
    bug.value = bug_heading;
    wp_left.bearing = wp_left_bearing;
    wp_right.bearing = wp_right_bearing;
  }
};