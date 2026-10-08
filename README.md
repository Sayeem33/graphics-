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

### 3. OpenGL 3.3 Texturing Pipeline & Surface Detailing
A modern, self-contained texturing architecture with hardware mipmapping and trilinear filtering, integrated directly into the Blinn-Phong lighting and fog shader:
- **Zero External DLL Overhead**: Built-in 24/32-bit BMP loader and procedural generator; works out-of-the-box on standard MinGW / GCC without third-party image libraries.
- **Auto Texture Initialization**: On first run, the system automatically synthesizes and writes rich, mathematically seamless procedural textures into `assets/textures/`:
  - `asphalt.bmp`: Dark bitumen roadway aggregate with mineral quartz speckling.
  - `grass.bmp`: Multi-frequency meadow turf with lush green blade variation.
  - `water.bmp`: Dynamic water caustics with sine wave harmonics; UVs scroll continuously in real time (`globalTime * (0.035, 0.015)`).
  - `stone.bmp`: Chiseled ashlar masonry brick blocks with mortar joints for river quays and tunnel portals.
  - `building.bmp`: Skyscraper architectural facade with steel mullion grids, illuminated interior offices, and tinted glass reflections.
  - `concrete.bmp`: Sidewalk and curb aggregate pavement grain.
- **Custom Texture Swapping**: Drop any standard 24-bit or 32-bit `.bmp` image into `assets/textures/<name>.bmp` to instantly customize any scene element.
- **Shader Modulation**: Diffuse textures modulate surface base colors (`mix(baseColor, baseColor * texSample.rgb, textureBlend)`), preserving specular highlights, point lights, spotlights, and distance fog.

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

### 🚗 Manual Car Driving & Game Controls (Active by Default)
| Key / Input | Action |
|---|---|
| `[↑]` or `[W]` | **Accelerate Forward** (Throttle up to ~94 km/h; drains fuel under load) |
| `[↓]` or `[S]` | **Brake / Reverse** (Active brakes while moving forward, reverse gear when stopped) |
| `[←]` or `[A]` | **Steer Left** (Smooth steering with front wheel rotation and dynamic turn rate) |
| `[→]` or `[D]` | **Steer Right** (Smooth steering with front wheel rotation and dynamic turn rate) |
| `[F]` | **Refuel at Gas Station** (Spends 1 Coin ➔ +30% Fuel when parked in pump bay) |
| `[M]` | Toggle between **Manual User Drive** and **Autonomous Path-Following** |
| `[R]` | **Reset Car** back to Garage starting line with full fuel tank |

### ⛽ Game Mechanics, HUD & Obstacle Collisions
1. **Clean Dedicated Game Dashboard**:
   - **Row 1 - Fuel Level**:
     - Dedicated horizontal fuel gauge bar with real-time percentage (`85%`).
     - **Stays Emerald Green** during regular driving and refueling.
     - Automatically turns **Bright Pulsing Red** with a `LOW FUEL!` warning when below 25%.
     - Refueling at a gas station restores it back to vibrant Green.
   - **Row 2 - Coin Count & Milestones**:
     - Visual 2D Golden Coin icon with metallic shine beside explicit digital count: `x 05`.
     - Live Checkpoint counter: `CHECKPOINT 2/5` with emerald completion pips.
   - **Row 3 - Speedometer & Transmission**:
     - Digital speedometer reading in `KM/H` with dynamic speed bar, drive mode (`[MANUAL]`), and gear (`[D]`/`[R]`).
   - **Row 4 - Contextual Action Prompts**:
     - Interactive prompts for refueling (`[F] PRESS F TO REFUEL`), collision warnings, and control hints.

2. **Obstacle & World Border Collision System**:
   - The car can freely drive outside the road on open grass meadows.
   - Whenever an obstacle or map perimeter is reached, the car **stops immediately** (`carSpeed = 0`):
     - **Trees**: Forest and nature corridor trunks ($< 1.85\text{m}$ collision radius).
     - **Buildings**: Garage workshop, City Skyscrapers A–D, convenience stores.
     - **Mountain & Tunnel Walls**: Tunnel portal abutments, cliff ridges, and interior curved vault walls.
     - **Bridge Railings & River**: Safety guardrails stop the car on the bridge deck; open river water prevents driving into the river channel.
     - **Gas Stations & Gantries**: Station mart structures, pump columns, price totems, and checkpoint pylons.
     - **World Borders**: Boundary perimeter stops the car at map edges.
   - Upon collision, an alert banner appears: `COLLISION: [OBSTACLE] - REVERSE TO CLEAR`. Pressing `[↓]` (Reverse) seamlessly backs the vehicle away to continue driving.

3. **Golden Road Coins**:
   - 30 3D golden coins with embossed star emblems hover and spin along alternating lanes.
   - Steer the vehicle to collect them ($< 2.3\text{m}$ proximity) for a vertical pop and score increase.
   - Coins respawn after 22 seconds for continuous gameplay.

4. **2 Roadside Gas Stations**:
   - **Station 1 (City Gateway)**: $X = 35.0\text{m}, Z = 6.0\text{m}$ (rotated $-90^\circ$, facing west toward city boulevard) on the east shoulder with full parking apron.
   - **Station 2 (Countryside Oasis)**: $X = 5.0\text{m}, Z = -87.0\text{m}$ (rotated $0^\circ$, facing south toward oncoming highway traffic) cleanly set back with ~9m clearance from the roadway.
   - Pulling into either station's pump bay allows pressing `[F]` to refuel using collected coins (+30% fuel per coin).

5. **5 Milestone Checkpoint Gates & 3D Waving Racing Flags**:
   - **Overhead LED Truss Gantries**: Span 9.2m across the roadway with dynamic LED signage, illuminated arrow chevrons, and checkered border trim.
   - **Multi-Harmonic 3D Waving Flags**: Stainless steel masts ($Y = 5.2\text{m}$ to $8.0\text{m}$) with golden ball finials and dynamic fluttering checkered cloth mounted firmly atop gantry towers:
     - **Milestone 1 (City Gateway)**: Electric Cyan & Bright White Checkered.
     - **Milestone 2 (Grand River Bridge Approach)**: Crimson Red & Bright White Checkered.
     - **Milestone 3 (Mountain Tunnel Entrance Portal)**: Neon Amber Gold & Charcoal Black Hazard Checkered framing the stone entrance archway.
     - **Milestone 4 (Countryside Highway)**: Vivid Emerald Green & Crisp White Checkered.
     - **Milestone 5 (Garage Lap Finish)**: Classic Grand Prix Black & White Checkered Flag.
   - **Crossed Checkered Racing Flags (🏁 X 🏁)**: Center golden shield with angled staves and twin fanning checkered flags atop Milestone 5.
   - **Checkered Track Decal Stripes**: 2-row black & white checkered checkpoint stripes painted directly onto the asphalt surface under each gantry.
   - **HUD Milestone Flag Tracker**: Row 2 features a 2D Checkered Racing Flag icon, "CHECK" tag, and 5 numbered indicator boxes (`1` to `5`) showing completed (green), active target (pulsing cyan), and upcoming (slate).
   - **Celebration Banner**: Passing a checkpoint triggers twin animated 2D checkered flags flanking the message: `* CHECKPOINT X/5! +2 COINS *`.
   - Passing each checkpoint awards **+2 bonus coins**.
   - Completing a full lap awards **+5 bonus coins** and **+25% bonus fuel**.

### 🎥 Camera Perspectives
| Key / Input | Action |
|---|---|
| `[1]` | **Chase Camera** (Smooth 3rd-person follow behind the car) |
| `[2]` | **Cockpit Camera** (First-person driver hood / windshield view) |
| `[3]` | **Birds-Eye Camera** (Overhead drone view focused on the car) |
| `[4]` | **Free Orbit Camera** (Free-fly navigation with WASDQE) |
| `[5]` | **Scenic Roadside Camera** (Cinematic vistas tracking the car) |
| `[C]` | Cycle Scenic Camera Locations (Tunnel entrance, Tunnel vault, Bridge arch, River boats, City skyline) |
| `[Right Mouse Click]` | Toggle mouse cursor capture for free-look in Free Cam (`[4]`) |

### 🌤️ Environment & Simulation Controls
| Key / Input | Action |
|---|---|
| `[N]` | Toggle **Day / Night Mode** (activates street lamps, tunnel LEDs, headlights) |
| `[T]` | Toggle **Automatic Day/Night Cycle** (smooth continuous 50s cycle) |
| `[K]` | Toggle **Tree Wind Shearing** animation (demonstrates shear matrix) |
| `[H]` | Toggle **Car Headlights** manual override |
| `[SPACE]` | **Pause / Resume** simulation |
| `[P]` | Capture High-Res **Screenshot** (`screenshot.bmp`) |
| `[ESC]` | Exit simulation |

---

## 📊 Live HUD & Telemetry
The application includes an on-screen HUD overlay and updates the window title in real-time with:
- **Status Badges**: Environment Zone, Drive Mode (`[D]` / `[R]`), Day/Night, Headlights
- **Speedometer Bar**: Current speed in km/h with color gradient (Cyan ➔ Green ➔ Orange)
- **Fuel Tank Gauge Bar**: Real-time fuel percentage with $25\%$, $50\%$, and $75\%$ notch dividers
- **Coin Counter**: Number of collected coins with indicator pips
- **Milestone Progress**: 5-segment checkpoint progress pips
- **Refuel Action Prompt**: Blinking lime prompt when parked inside a gas station bay
- **Top Notification Banner**: Popups for milestone crossing, coin pickups, and refueling alerts

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
