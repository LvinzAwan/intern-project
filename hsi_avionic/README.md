# HSI Avionic Display System

A Horizontal Situation Indicator (HSI) is an aircraft navigation display that combines heading information, course guidance, and lateral deviation into a single, intuitive instrument to help pilots understand their position and direction relative to a planned route. The HSI developed in this project is a software-based prototype that focuses on the graphical visualization of these navigation concepts using OpenGL, including a rotating compass, course indication, CDI, and TO/FROM logic. At this stage, the system functions purely as a GUI prototype and has not yet been integrated with real aircraft sensors, avionics hardware, or live navigation data.

---

##  Visual Documentation

### HSI Main Display
(<img width="795" height="638" alt="HSI Main Display" src="https://github.com/user-attachments/assets/ea9a313f-96e8-438e-9aa6-b38f2d5819ac" />)

### Navigation Demo
https://github.com/user-attachments/assets/cced2431-4631-45ed-82a7-4efc48e0ec77

---

##  Keyboard Controls

| Key | Action | Range |
|-----|--------|-------|
| **↑ (Up Arrow)** | Increase bug heading | +1° per press |
| **↓ (Down Arrow)** | Decrease bug heading | -1° per press |
| **→ (Right Arrow)** | Increase heading (fast) | +1° per press |
| **← (Left Arrow)** | Decrease heading (fast) | -1° per press |
| **W** | Increase bug heading | +1° per press |
| **S** | Decrease bug heading | -1° per press |
| **A** | Decrease waypoint bearing | -1° per press |
| **D** | Increase waypoint bearing | +1° per press |
| **1** | Move perpendicular line (left/offset) | -0.1 unit per press |
| **2** | Move perpendicular line (right/offset) | +0.1 unit per press |
| **3** | Switch to/from flag | N/A |

---

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
---

## Installation

### 1. Clone Repository

```bash
git clone https://github.com/LvinzAwan/intern-project.git
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

##  Build & Compilation

### Standard Build Process

```bash
# Navigate to project directory
cd ~/intern-project/hsi_avionic

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Compile 
cmake --build .

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
cmake --build . 
```

---

##  Running the Application

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
##  Project Structure & Architecture

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

### Modules

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

### OpenGL Specifications

- **Shader Version:** GLSL 330 Core
- **Vertex Format:** 2D positions (float x, float y)
- **Primitives:** GL_LINE_LOOP, GL_LINES, GL_TRIANGLES, GL_LINE_STRIP
- **Line Widths:** 1.0 - 10.0 pixels (configurable per element)

---
