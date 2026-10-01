# 3D Autonomous Car Journey (OpenGL 3.3 Core Profile in C++)

A feature-complete, real-time 3D simulation of an **autonomous car journey** navigating through diverse dynamic environments: **Garage, Metropolitan City, Grand River Bridge, Mountain Tunnel, and Countryside Highway**.

Built strictly in **C++ (C++17)** with **OpenGL 3.3 Core Profile**, **GLFW**, and **GLAD**.

---

## 🌟 Key Features & Scene Elements

### 1. Autonomous Car
- **Hierarchical 3D Model**: Chassis, sports bodywork, tapered cabin with tinted windshield, front grille, rear aerodynamic wing, spinning autonomous LiDAR puck.
- **Physical Wheels**: 4 wheels with rubber tires and silver alloy spoke rims.
  - Wheels **spin around their local axle** proportionally to linear road speed ($d\theta = v \cdot dt / r$).
  - Front wheels **steer dynamically into road curves** based on spline curvature.
- **Twin Spotlights / Headlights**: Dual forward-facing spotlights with conical angular falloff illuminating the road surface ahead, plus glowing red taillights.

### 2. Environment Zones & Intelligent Speed Adjustment
The car continuously cruises along an interpolated closed-circuit 3D track, dynamically adapting its velocity based on safety and zone regulations:
1. **Garage Zone (Starting & Arrival)**:
   - 3D garage workshop with foundation pad, brick walls, pitched roof, and overhead door header.
   - Car initiates its journey here at a controlled starting speed (6–8 m/s).
2. **City Area**:
   - High-rise glass towers, commercial buildings with rooftop AC units, and pedestrian sidewalks with curbs.
   - Cruise speed: 12–14 m/s.
3. **Grand River Bridge**:
   - Elevated roadway with safety guardrails and iconic structural arch truss.
   - Anchored by 4 heavy concrete support pillars descending into the river valley.
   - **Water & River**: Flowing river beneath the bridge with shimmering specular reflections.
   - **Boats on the River**: Multiple 3D boats (cruiser yacht, sailboat, dinghy) sailing along the river with continuous wave bobbing and rocking rotation.
   - Speed slows down to bridge crossing speed (9.5 m/s).
4. **Mountain Tunnel**:
   - Rocky mountain terrain pierced by an arched tunnel.
   - Concrete portal entrance and curved interior ceiling.
   - **Tunnel Lights**: Ceiling fixtures automatically illuminate brightly when the car enters the tunnel or during nighttime.
   - Speed adjusts to safe tunnel speed (10.5 m/s).
5. **Countryside Highway**:
   - Wide open road flanked by roadside trees and meadows.
   - Car accelerates to high cruising speed (20–22 m/s).

---

## 📐 Geometric Transformations Demonstrated

| Transformation | Implementation in Code |
|---|---|
| **Translation** | Car moving along the 3D spline trajectory, boats sailing along the river channel, camera panning. |
| **Rotation** | Car yaw/pitch aligning with track curvature, front wheels turning with steering angle, wheels spinning around local axles, boats rocking with river waves ($pitch$ & $roll$), spinning LiDAR sensor. |
| **Scaling** | Skyscraper heights, tree canopy tiers, lamp posts, bridge pillar proportions, headlight cones. |
| **Shearing** | **1. Static Architecture**: Modern deconstructivist city skyscraper sheared along $X$ relative to height $Y$ (`Mat4::shearX(0.22f, 0.0f)`).<br>**2. Dynamic Wind Shearing**: Roadside pine and oak trees sway in real-time breeze using a time-varying shear matrix applied to foliage: `Mat4::shearX(amp * sin(t), amp * cos(t))`. |
| **Camera Control** | 5 distinct camera perspectives with smooth position and look-at damping. |
| **Lighting** | Blinn-Phong shading model with directional sunlight/moonlight, point lights for street/tunnel fixtures, and dual car headlight spotlights. |
| **Continuous Animation** | Continuous 60+ FPS delta-time physics loop with smooth acceleration and braking. |

---

## 🎮 Interactive Controls

| Key / Input | Action |
|---|---|
| `[1]` | **Chase Camera** (Dynamic 3rd-person follow behind the car) |
| `[2]` | **Cockpit Camera** (Driver hood / windshield view) |
| `[3]` | **Birds-Eye Camera** (Overhead drone view of the entire scene) |
| `[4]` | **Free Orbit Camera** (Free-fly navigation) |
| `[5]` | **Scenic Roadside Camera** (Tracks the car from roadside vantage points) |
| `[C]` | Cycle Scenic Camera Locations (Bridge overlook, City intersection, Tunnel ridge) |
| `[W] [A] [S] [D] [Q] [E]` | Move camera in Free Orbit mode (`[4]`) |
| `[Right Mouse Click]` | Toggle mouse cursor capture for free-look rotation |
| `[N]` | Toggle **Day / Night Mode** (triggers street lamps, tunnel lamps, headlights) |
| `[T]` | Toggle **Automatic Day/Night Cycle** (smooth continuous transition) |
| `[S]` | Toggle **Tree Wind Shearing** animation (demonstrates shear matrix) |
| `[H]` | Toggle **Car Headlights** manual override |
| `[SPACE]` | **Pause / Resume** car movement |
| `[R]` | **Reset Car** back to Garage starting point |
| `[ESC]` | Exit simulation |

---

## 📊 Live HUD & Telemetry
The application includes an on-screen HUD overlay and updates the window title in real-time with:
- **Current Zone Badge** (Garage, City, Bridge, Tunnel, Countryside)
- **Speedometer Bar** (Current speed in km/h with target speed indicator)
- **Day/Night & Headlight Status**
- **Active Camera Mode & Shearing Status**

---

## 🚀 How to Build and Run (C++)

### Option 1: 1-Click Batch Script
Double-click or run in terminal:
```cmd
run.bat
```

### Option 2: Using g++ directly in Terminal
```bash
g++ -std=c++17 -Wall -Wextra src/main.cpp src/glad.c -Iinclude -Llib -lglfw3 -lopengl32 -lgdi32 -o main.exe
./main.exe
```

### Option 3: Using Makefile
```bash
mingw32-make run
```
