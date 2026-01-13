# HSI Avionic Display System

Horizontal Situation Indicator (HSI) is a real-time aircraft navigation display system built with OpenGL, featuring compass visualization, waypoint navigation, and dynamic heading indicators.

**Status:** 🟢 Development Complete | Build: Stable | Version: 1.0.0

---

## 📋 Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Build & Compilation](#build--compilation)
- [Running the Application](#running-the-application)
- [Keyboard Controls](#keyboard-controls)
- [Project Structure](#project-structure)
- [Technical Documentation](#technical-documentation)
- [Screenshots & Demo](#screenshots--demo)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [License](#license)

---

## 📖 Overview

The HSI (Horizontal Situation Indicator) is an essential flight instrument that displays aircraft position and navigation information in a 360-degree compass view. This implementation provides:

- **Compass Ring** - Cardinal directions (N, E, S, W) with 360° scale
- **Aircraft Symbol** - Center indicator showing current aircraft heading
- **Bug Heading Indicator** - Adjustable target heading marker
- **Dual Waypoint Navigation** - Left and right waypoint tracking with bearing and distance
- **Wind Information** - Real-time wind direction and speed display
- **Flight Data** - IAS (Indicated Airspeed) and altitude readout

The system runs at 100+ Hz with smooth real-time updates and responsive input handling.

---

## ✨ Key Features

### Compass System
- ✅ Full 360° compass ring with 5° tick marks
- ✅ Cardinal direction markers (N, E, S, W) with triangular pointers
- ✅ Real-time heading rotation synchronized with input
- ✅ Smooth animation and rendering
- ✅ Multiple tick levels (cardinal, major, medium, minor)

### Navigation Display
- ✅ Dual independent waypoint system (Left/Right)
- ✅ Distance and bearing calculation for each waypoint
- ✅ TO/FROM flag indicator for active navigation
- ✅ Course tracking visualization
- ✅ Perpendicular line offset for approach tracking
- ✅ Double-arrow waypoint indicators

### Heading & Course Information
- ✅ Adjustable bug heading indicator
- ✅ Current heading readout
- ✅ Track and ground speed display
- ✅ Wind direction and magnitude visualization
- ✅ Real-time data refresh (100+ Hz)

### User Interface
- ✅ TTF font rendering for all text displays
- ✅ Color-coded information (Yellow = active, White = standard, Magenta = bugs)
- ✅ Responsive keyboard input handling
- ✅ Configurable window size and aspect ratio
- ✅ Hardware-accelerated OpenGL rendering

---

## 🖥️ System Requirements

### Minimum Specifications
| Component | Minimum | Recommended |
|-----------|---------|-------------|
| Operating System | Linux (Ubuntu 18.04+) | Ubuntu 20.04+ / Debian 11+ |
| Compiler | GCC 9 / Clang 10 | GCC 11+ / Clang 14+ |
| CMake | 3.10 | 3.20+ |
| OpenGL | 3.3 | 4.5+ |
| RAM | 512 MB | 2 GB+ |
| VRAM | 512 MB | 1 GB+ |

### Dependencies

**Required Libraries:**
- **GLFW 3.x** - Window management and input handling
- **OpenGL 3.3+** - Graphics API
- **FreeType 2.x** - Font rendering engine
- **GLM** - OpenGL Mathematics library
- **GLAD** - OpenGL loader (included)

**Development Tools:**
- CMake 3.10 or later
- GCC/Clang with C++17 support
- Make or Ninja build system
- pkg-config (for library detection)

### Linux Installation (Ubuntu/Debian)

```bash
# Update package manager
sudo apt update && sudo apt upgrade -y

# Install development tools
sudo apt install -y \
  build-essential \
  cmake \
  git \
  pkg-config \
  libfreetype6-dev \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxi-dev \
  libglvnd-dev \
  xorg-dev

# Optional: Install additional graphics tools
sudo apt install -y mesa-utils libglfw3-dev

# Verify OpenGL support
glxinfo | grep "OpenGL version"
```

### Fedora/RHEL Installation

```bash
sudo dnf install -y \
  gcc-c++ \
  cmake \
  git \
  pkgconfig \
  freetype-devel \
  libX11-devel \
  libXrandr-devel \
  libXinerama-devel \
  libXcursor-devel \
  libXi-devel \
  mesa-libGL-devel \
  glfw-devel
```

---

## 📦 Installation

### 1. Clone Repository

```bash
git clone https://github.com/lvinz/hsi_avionic.git
cd hsi_avionic
```

### 2. Project Structure

```
hsi_avionic/
├── CMakeLists.txt               # Build configuration
├── README.md                    # This file
├── LICENSE                      # MIT License
│
├── include/                     # Header files
│   ├── config/
│   │   ├── AppConfig.hpp       # Application configuration
│   │   └── ColorConfig.hpp     # Color definitions
│   ├── core/
│   │   ├── ApplicationState.hpp # Global state management
│   │   ├── InputHandler.hpp    # Input processing
│   │   └── RenderEngine.hpp    # Main rendering engine
│   ├── compas/
│   │   └── CompasRenderer.hpp  # Compass rendering
│   ├── gfx/
│   │   ├── Shader.hpp          # OpenGL shader wrapper
│   │   ├── TtfTextRenderer.hpp # Font rendering system
│   │   └── HsiRenderer.hpp     # HSI utility functions
│   ├── ui/
│   │   └── HsiUiRenderer.hpp   # UI overlay elements
│   └── data/
│       └── HsiData.hpp         # Data structures
│
├── src/                        # Implementation files
│   ├── main.cpp               # Entry point
│   ├── core/
│   │   ├── ApplicationState.cpp
│   │   ├── InputHandler.cpp
│   │   └── RenderEngine.cpp
│   ├── compas/
│   │   └── CompasRenderer.cpp
│   ├── gfx/
│   │   ├── Shader.cpp
│   │   ├── TtfTextRenderer.cpp
│   │   └── HsiRenderer.cpp
│   └── ui/
│       └── HsiUiRenderer.cpp
│
├── lib/                       # Third-party libraries
│   ├── glad/
│   └── stb/
│
├── assets/                    # Resources
│   └── fonts/
│       └── Arial.ttf
│
└── build/                     # Build output (auto-generated)
    ├── bin/
    │   └── hsi_avionic       # Executable
    └── CMakeFiles/
```

---

## 🔨 Build & Compilation

### Standard Build Process

```bash
# Navigate to project directory
cd ~/magang/intern-project/hsi_avionic

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Compile (using 4 parallel jobs)
cmake --build . --parallel 4

# Or using make directly
make -j4
```

### Build Variants

**Debug Build (with debugging symbols)**
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel 4
```

**Release Build (optimized)**
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel 4
```

**Verbose Compilation (to see all compiler flags)**
```bash
cmake --build . --verbose
```

### Build Output

```
build/
├── bin/hsi_avionic              # Executable program
├── CMakeFiles/                  # Build metadata
├── CMakeCache.txt               # CMake configuration cache
└── libhsi_avionic.a            # Static library (optional)
```

### Troubleshooting Build Issues

**Issue: "GLFW not found"**
```bash
sudo apt install libglfw3-dev
cd build && rm -rf * && cmake .. && cmake --build .
```

**Issue: "FreeType not found"**
```bash
sudo apt install libfreetype6-dev
cd build && rm -rf * && cmake .. && cmake --build .
```

**Issue: "OpenGL not found"**
```bash
sudo apt install libglvnd-dev
```

**Complete rebuild from scratch**
```bash
cd ~/magang/intern-project/hsi_avionic
rm -rf build
mkdir build && cd build
cmake ..
cmake --build . --parallel 4
```

---

## 🚀 Running the Application

### Start the Program

```bash
# From build directory
cd build
./hsi_avionic

# Or from project root
./build/hsi_avionic

# Or install and run from anywhere
sudo make install
hsi_avionic
```

### Expected Console Output

```
[HSI Avionic] Initializing application...
[HSI Avionic] Loading fonts from assets/fonts/
[HSI Avionic] Initializing OpenGL context...
[HSI Avionic] Starting render loop at 100+ Hz
[HSI Avionic] Ready for input
```

### Closing the Application

Press **ESC** key or close the window to exit gracefully.

---

## 🎮 Keyboard Controls

| Key | Action | Range |
|-----|--------|-------|
| **↑ (Up Arrow)** | Increase heading | +1° per press |
| **↓ (Down Arrow)** | Decrease heading | -1° per press |
| **→ (Right Arrow)** | Increase heading (fast) | +10° per press |
| **← (Left Arrow)** | Decrease heading (fast) | -10° per press |
| **W** | Increase bug heading | +1° per press |
| **S** | Decrease bug heading | -1° per press |
| **A** | Decrease waypoint bearing | -1° per press |
| **D** | Increase waypoint bearing | +1° per press |
| **Q** | Move perpendicular line (left/offset) | -0.1 unit per press |
| **E** | Move perpendicular line (right/offset) | +0.1 unit per press |
| **ESC** | Exit application | N/A |

### Control Examples

```
Current State Example:
┌─────────────────────────────────────┐
│ Heading:          180°              │
│ Bug Heading:      175°              │
│ Waypoint L:  090° / 25.5 NM        │
│ Waypoint R:  270° / 10.2 NM        │
│ Wind:        220° @ 15 knots       │
│ IAS:         120 knots             │
│ Altitude:    2,500 feet            │
└─────────────────────────────────────┘
```

**Example Control Sequence:**
1. Press **↑** 5 times → Heading changes from 180° to 185°
2. Press **W** 3 times → Bug heading changes from 175° to 178°
3. Press **D** 10 times → Right waypoint bearing increases by 10°
4. Press **Q** → Perpendicular line moves left

---

## 🏗️ Project Structure & Architecture

### System Architecture Diagram

```
┌────────────────────────────────────────┐
│         main.cpp (Entry Point)         │
└──────────────────┬─────────────────────┘
                   │
        ┌──────────┴──────────┐
        ▼                     ▼
   ┌──────────┐        ┌───────────────┐
   │ Input    │        │ Render        │
   │ Handler  │        │ Engine        │
   └──────────┘        └───────┬───────┘
        │                      │
        │      ┌───────────────┼───────────────┐
        │      ▼               ▼               ▼
        │  ┌──────────┐  ┌─────────────┐  ┌────────────┐
        │  │ Compass  │  │ HSI         │  │ HSI UI     │
        │  │ Renderer │  │ Renderer    │  │ Renderer   │
        │  └──────────┘  └─────────────┘  └────────────┘
        │      │               │               │
        └──────┼───────────────┼───────────────┘
               │               │
               └───────┬───────┘
                       ▼
            ┌──────────────────────┐
            │ TTF Text Renderer    │
            │ (FreeType / Shader)  │
            └──────────────────────┘
                       │
                       ▼
            ┌──────────────────────┐
            │   OpenGL / GLFW      │
            │  (Graphics & Input)  │
            └──────────────────────┘
```

### Core Modules

| Module | File | Responsibility |
|--------|------|-----------------|
| **CompasRenderer** | `src/compas/CompasRenderer.cpp` | Renders compass ring, ticks, markers, and aircraft symbol |
| **HsiRenderer** | `src/gfx/HsiRenderer.cpp` | Provides utility functions for rendering HSI elements |
| **TtfTextRenderer** | `src/gfx/TtfTextRenderer.cpp` | Handles font loading and text rendering using FreeType |
| **RenderEngine** | `src/core/RenderEngine.cpp` | Orchestrates overall rendering pipeline and data display |
| **InputHandler** | `src/core/InputHandler.cpp` | Processes keyboard input and updates application state |
| **ApplicationState** | `src/core/ApplicationState.hpp` | Manages global application data (heading, waypoints, wind, etc.) |
| **HsiUiRenderer** | `src/ui/HsiUiRenderer.cpp` | Renders informational overlays and UI elements |
| **Shader** | `src/gfx/Shader.cpp` | Wraps OpenGL shader compilation and linking |

### Data Flow

```
User Input (Keyboard)
        │
        ▼
┌──────────────────┐
│ InputHandler     │  ← Receives key events
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ ApplicationState │  ← Updates heading, bearing, etc.
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ RenderEngine     │  ← Reads state & renders scene
└────────┬─────────┘
         │
    ┌────┴────┬────────┬────────┐
    ▼         ▼        ▼        ▼
┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐
│Compass│ │HsiRend│ │HsiUI │ │Text  │
│Render │ │erer  │ │Render│ │Render│
└──────┘ └──────┘ └──────┘ └──────┘
    │         │        │        │
    └────┬────┴────┬───┴───┬────┘
         ▼         ▼       ▼
      OpenGL / GLFW / GPU
         │
         ▼
      Display (Screen)
```

---

## 📚 Technical Documentation

### Configuration Files

**AppConfig.hpp** - Window and display settings:
```cpp
namespace WindowConfig {
  constexpr int WIDTH = 800;
  constexpr int HEIGHT = 600;
  constexpr float ASPECT_FIX = HEIGHT / (float)WIDTH;
}

namespace DisplayLayout {
  constexpr float CARDINAL_RADIUS = 0.70f;
  constexpr float NUMBER_RADIUS = 0.60f;
}

namespace ColorRGB {
  const glm::vec3 YELLOW   = {1.0f, 1.0f, 0.0f};
  const glm::vec3 WHITE    = {1.0f, 1.0f, 1.0f};
  const glm::vec3 MAGENTA  = {1.0f, 0.0f, 1.0f};
}
```

### Key Classes

**CompasRenderer**
```cpp
class CompasRenderer {
public:
  bool init(int width, int height);
  void setHeadingDeg(float heading);
  
  // Geometry building
  void buildRingGeometry(float radius_ndc, int segments);
  void buildTicksGeometry(...);
  void buildCardinalMarkersGeometry(...);
  
  // Drawing functions
  void drawRing();
  void drawTicks();
  void drawCardinalMarkers();
  void drawHeadingIndicator();
  void drawAircraftSymbol(float aspect_fix);
  void drawBugTriangle(...);
  void drawWaypointArrowDouble(...);
};
```

**RenderEngine**
```cpp
class RenderEngine {
public:
  void renderFrame(CompasRenderer& compas,
                   TtfTextRenderer fonts[],
                   HsiUiRenderer& ui,
                   ApplicationState& state);
                   
private:
  void renderCompass(...);
  void renderHeadingDisplay(...);
  void renderNavigationOverlays(...);
};
```

### OpenGL Specifications

- **Shader Version:** GLSL 330 Core
- **Vertex Format:** 2D positions (float x, float y)
- **Primitives:** GL_LINE_LOOP, GL_LINES, GL_TRIANGLES, GL_LINE_STRIP
- **Line Widths:** 1.0 - 10.0 pixels (configurable per element)

### Coordinate System

```
Normalized Device Coordinates (NDC):
  Y
  │   1.0 ┌──────────┐
  │       │          │
  │       │  Display │
  │   0.0 │          │
  │       └──────────┘
  └───────────────────── X
 -1.0  0.0        1.0

Aspect Ratio Fix:
  aspect_fix = window_height / window_width
  Applied to X coordinates for square display
```

---

## 📷 Screenshots & Demo

### Visual Documentation

This section is ready for your screenshots and video demonstrations:

```markdown
#### Main Display
![HSI Main Display](assets/screenshots/01_main_display.png)
*Main compass display with heading indicator and waypoint markers*

#### Waypoint Navigation
![Waypoint Tracking](assets/screenshots/02_waypoint_tracking.png)
*Dual waypoint system with bearing and distance information*

#### Wind Information
![Wind Display](assets/screenshots/03_wind_info.png)
*Wind direction and speed overlay on the compass*

### Video Demonstrations

#### 1. Real-time Navigation (30 seconds)
[![Navigation Demo](assets/videos/thumbs/nav_demo.jpg)](assets/videos/01_navigation_demo.mp4)

#### 2. Heading Adjustment (20 seconds)
[![Heading Control](assets/videos/thumbs/heading_demo.jpg)](assets/videos/02_heading_control.mp4)

#### 3. Complete System Demo (1 minute)
[![Full Demo](assets/videos/thumbs/full_demo.jpg)](assets/videos/03_full_system.mp4)
```

**To add screenshots and videos:**

1. Create directories:
```bash
mkdir -p assets/screenshots
mkdir -p assets/videos/thumbs
```

2. Place images as PNG files in `assets/screenshots/`
3. Place videos as MP4 files in `assets/videos/`
4. Add references to README.md as shown above

---

## 🐛 Troubleshooting

### Build Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `error: cannot find -lglfw` | GLFW library not found | `sudo apt install libglfw3-dev` |
| `error: freetype.h: No such file` | FreeType not installed | `sudo apt install libfreetype6-dev` |
| `error: unknown type name 'GLint'` | OpenGL headers missing | `sudo apt install libglvnd-dev` |
| `error: 'glfw3.h' not found in any of the...` | GLFW include path wrong | Use `pkg-config` or `find /usr -name glfw3.h` |

### Runtime Issues

**Black Screen**
```bash
# Check OpenGL support
glxinfo | grep "OpenGL version"

# Should show OpenGL 3.3 or higher
# If not, update graphics drivers
```

**Keyboard Input Not Working**
```bash
# Ensure window has focus
# Rebuild with debug output
cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . && ./hsi_avionic
```

**Font Loading Error**
```bash
# Verify font file exists
ls -la assets/fonts/

# Check font path in source code
grep -r "fonts/" include/ src/
```

**Program Crashes on Startup**
```bash
# Run with debugging
gdb ./build/hsi_avionic
(gdb) run
(gdb) bt  # Print backtrace
```

### Performance Issues

| Problem | Cause | Solution |
|---------|-------|----------|
| Low FPS | Weak GPU or driver issue | Update GPU drivers: `sudo apt install --only-upgrade xserver-xorg-video-*` |
| Lag on input | High CPU usage | Check system: `htop`, close background apps |
| Memory leak | Unfreed OpenGL resources | Run with valgrind: `valgrind ./build/hsi_avionic` |

### Common Solutions

```bash
# Clean build if having issues
rm -rf build && mkdir build && cd build
cmake .. && cmake --build . --parallel 4

# Check all dependencies installed
pkg-config --list-all | grep -E "glfw|freetype|gl"

# Verify compiler version (C++17 required)
gcc --version
g++ --version
```

---

## 🔧 Development

### Adding New Features

1. **Header file:** Create or modify in `include/`
2. **Implementation:** Add to corresponding `src/` directory
3. **CMakeLists.txt:** Register new source files:
   ```cmake
   target_sources(hsi_avionic PRIVATE
     src/new_module/NewFile.cpp
   )
   ```
4. **Compile & test:** `cmake --build . --parallel 4`

### Code Style Guidelines

```cpp
// Naming conventions
class MyRenderer { };
void renderCompass() { }
float current_heading;

// Header guards
#pragma once

// Constants
constexpr float PI = 3.1415926535f;
const glm::vec3 COLOR_YELLOW = {1.0f, 1.0f, 0.0f};

// Include order
#include <standard_libs>
#include <third_party>
#include "local_headers"

// Const correctness
const float* getHeading() const;
void setHeading(float heading);
```

### Building Tests (if available)

```bash
cd build
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest --output-on-failure
```

---

## 📝 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

**MIT License Summary:**
- ✅ You can use this for commercial projects
- ✅ You can modify and distribute it
- ⚠️ Must include the original license and copyright notice
- ⚠️ No warranty or liability

---

## 🤝 Contributing

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/AmazingFeature`
3. Commit your changes: `git commit -m 'Add AmazingFeature'`
4. Push to the branch: `git push origin feature/AmazingFeature`
5. Open a Pull Request

---

## 📧 Support & Contact

For issues, questions, or suggestions:

- **GitHub Issues:** [Report bugs here](https://github.com/lvinz/hsi_avionic/issues)
- **Email:** your.email@example.com
- **Documentation Issues:** Create an issue with `[DOCS]` prefix

---

## 📚 References & Resources

### Official Documentation
- [GLFW Documentation](https://www.glfw.org/documentation.html)
- [OpenGL Tutorials](https://learnopengl.com/)
- [FreeType Documentation](https://freetype.org/documentation.html)
- [CMake Guide](https://cmake.org/cmake/help/latest/)
- [GLM Math Library](https://github.com/g-truc/glm)

### Learning Resources
- [Aviation - Horizontal Situation Indicator (HSI)](https://en.wikipedia.org/wiki/Horizontal_situation_indicator)
- [OpenGL 3.3+ Tutorial Series](https://learnopengl.com/)
- [C++ Modern Features](https://en.cppreference.com/)

### Related Projects
- [FlightGear Flight Simulator](https://www.flightgear.org/)
- [Open Cockpit](http://www.opencockpits.com/)

---

## 📊 Project Statistics

- **Language:** C++17
- **Graphics API:** OpenGL 3.3+
- **Build System:** CMake 3.10+
- **Lines of Code:** ~5,000+ LOC
- **Modules:** 8 core modules
- **Rendering Speed:** 100+ Hz
- **License:** MIT

---

## 🎯 Roadmap

### Current Version (1.0.0)
- ✅ Basic compass display
- ✅ Heading indicator
- ✅ Dual waypoint navigation
- ✅ Wind information display
- ✅ Flight data readout

### Future Enhancements (v1.1+)
- [ ] Multi-window support
- [ ] Configuration file system
- [ ] Data logging/recording
- [ ] Network data integration
- [ ] Customizable themes/colors
- [ ] VR support (experimental)

---

**Last Updated:** January 13, 2026  
**Version:** 1.0.0  
**Status:** Stable & Production Ready  
**Maintainer:** [Your Name]

---

### Quick Links
- [Installation Guide](#installation)
- [Controls Reference](#keyboard-controls)
- [Build Instructions](#build--compilation)
- [Troubleshooting](#troubleshooting)
- [License Information](#license)

**Happy Flying! ✈️**