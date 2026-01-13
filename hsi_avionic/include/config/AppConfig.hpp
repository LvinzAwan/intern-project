#pragma once

//Window
namespace WindowConfig {
  constexpr int WIDTH = 800;
  constexpr int HEIGHT = 600;
  constexpr float ASPECT_FIX = (float)HEIGHT / (float)WIDTH;
  constexpr const char* TITLE = "HSI (Horizontal Situation Indicator)";
}

//Layout 
namespace DisplayLayout {
  constexpr float CARDINAL_RADIUS = 0.55f;   // N/E/S/W radius
  constexpr float NUMBER_RADIUS   = 0.57f;   // heading numbers radius

  constexpr float LEFT_OFFSET  = -0.95f;   
  constexpr float RIGHT_OFFSET =  0.95f;     
}

//Colors 
namespace ColorRGB {
  struct Color { float r, g, b; };

  constexpr Color YELLOW  {1.0f, 1.0f, 0.0f};
  constexpr Color WHITE   {1.0f, 1.0f, 1.0f};
  constexpr Color GREEN   {0.0f, 1.0f, 0.0f};
  constexpr Color GRAY    {0.5f, 0.5f, 0.5f};
  constexpr Color MAGENTA {1.0f, 0.0f, 1.0f};
}

//Marker dots 
namespace CircleConfig {
  constexpr float SPACING    = 0.15f;
  constexpr float RADIUS     = 0.025f;
  constexpr float OPACITY    = 0.7f;
  constexpr float LINE_WIDTH = 2.0f;
}

//CDI bar
namespace PerpLineConfig {
  constexpr float SPACING     = 0.15f;
  constexpr float LINE_LENGTH = 0.20f;
  constexpr float LINE_WIDTH  = 6.0f;

  constexpr float SENSITIVITY      = 0.008f;
  constexpr float MAX_OFFSET_RIGHT =  0.45f;
  constexpr float MAX_OFFSET_LEFT  = -0.45f;
}

//Stub data
namespace DataConfig {
  // Wind
  constexpr float WIND_DIRECTION = 53.0f;
  constexpr float WIND_SPEED     = 180.0f;
  constexpr float WIND_Y         = 0.90f;

  // GPS
  constexpr const char* GPS_STATUS = "GPS OK";
  constexpr float GPS_Y = 0.80f;

  // IAS
  constexpr float IAS_VALUE = 181.0f;
  constexpr float IAS_Y     = 0.0f;

  // Course / GS
  constexpr float COURSE_COG   = 45.0f;
  constexpr float COURSE_GS    = 181.0f;
  constexpr float COURSE_Y_COG = 0.90f;
  constexpr float COURSE_Y_GS  = 0.80f;

  // Altitude
  constexpr float ALT_VALUE = -840.0f;
  constexpr float ALT_Y     = 0.0f;

  // Left waypoint
  constexpr float       WP_LEFT_DISTANCE   = 861.9f;
  constexpr const char* WP_LEFT_NAME       = "EDAB";
  constexpr const char* WP_LEFT_RUNWAY     = "BRUTZEM";
  constexpr float       WP_LEFT_APP_FREQ   = 125.875f;
  constexpr float       WP_LEFT_INFO_FREQ  = 120.605f;
  constexpr float       WP_LEFT_Y          = -0.40f;

  // Right waypoint
  constexpr float       WP_RIGHT_DISTANCE  = 1000.0f;
  constexpr const char* WP_RIGHT_NAME      = "EDD1";
  constexpr const char* WP_RIGHT_RUNWAY    = "LSZH";
  constexpr float       WP_RIGHT_APP_FREQ  = 119.120f;
  constexpr float       WP_RIGHT_INFO_FREQ = 134.000f;
  constexpr float       WP_RIGHT_Y         = -0.40f;

  // Heading bug
  constexpr float BUG_X = 0.0f;
  constexpr float BUG_Y = -0.85f;

  // IAS frame
  constexpr float IAS_FRAME_X      = -0.95f;
  constexpr float IAS_FRAME_Y      =  0.00f;
  constexpr float IAS_FRAME_WIDTH  =  0.35f;
  constexpr float IAS_FRAME_HEIGHT =  0.15f;

  // ALT frame
  constexpr float ALT_FRAME_X      =  0.95f;
  constexpr float ALT_FRAME_Y      =  0.00f;
  constexpr float ALT_FRAME_WIDTH  =  0.35f;
  constexpr float ALT_FRAME_HEIGHT =  0.15f;

  // Heading box
  constexpr float HEADING_BOX_X      = 0.0f;
  constexpr float HEADING_BOX_Y      = 0.82f;
  constexpr float HEADING_BOX_WIDTH  = 0.20f;
  constexpr float HEADING_BOX_HEIGHT = 0.10f;
}

//Fonts
namespace FontConfig {
  constexpr int FONT_COUNT = 12;
  constexpr const char* PATHS[FONT_COUNT] = {
    "../assets/fonts/DejaVuSans-Bold.ttf", // CARDINAL
    "../assets/fonts/DejaVuSans-Bold.ttf", // NUMBERS
    "../assets/fonts/DejaVuSans-Bold.ttf", // HEADING_VALUE
    "../assets/fonts/DejaVuSans-Bold.ttf", // HEADING_LABEL
    "../assets/fonts/DejaVuSans-Bold.ttf", // HEADING_SYMBOL
    "../assets/fonts/ArialMdm.ttf",        // INFO_VALUE
    "../assets/fonts/ArialMdm.ttf",        // INFO_LABEL
    "../assets/fonts/DejaVuSans-Bold.ttf", // WAYPOINT_NAME
    "../assets/fonts/DejaVuSans-Bold.ttf", // WAYPOINT_BEARING
    "../assets/fonts/ArialMdm.ttf",        // WAYPOINT_INFO
    "../assets/fonts/DejaVuSans-Bold.ttf", // IAS_ALT_VALUE
    "../assets/fonts/DejaVuSans-Bold.ttf"  // IAS_ALT_LABEL
  };

  constexpr float SIZES[FONT_COUNT] = {
    56.0f, 40.0f, 52.0f, 38.0f, 26.0f, 54.0f,
    48.0f, 82.0f, 64.0f, 42.0f, 84.0f, 56.0f
  };

  enum FontIndex : int {
    CARDINAL = 0,
    NUMBERS,
    HEADING_VALUE,
    HEADING_LABEL,
    HEADING_SYMBOL,
    INFO_VALUE,
    INFO_LABEL,
    WAYPOINT_NAME,
    WAYPOINT_BEARING,
    WAYPOINT_INFO,
    IAS_ALT_VALUE,
    IAS_ALT_LABEL
  };
}
