#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "math3d.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "track.hpp"
#include "scene.hpp"
#include "models.hpp"
#include "hud.hpp"
#include "screenshot.hpp"

// Global Window State
int windowWidth = 1280;
int windowHeight = 720;
Camera camera;
Track track;
SceneManager scene;
HUD hud;

bool isPaused = false;
bool mouseCaptured = false;
double lastMouseX = 640.0;
double lastMouseY = 360.0;
bool firstMouse = true;

// Key state tracking for single-press toggles
bool keyStates[1024] = {false};

// ==========================================
// GLSL Shaders
// ==========================================

const char* sceneVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 VertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * aNormal);
    TexCoords = aTexCoords;
    VertexColor = aColor;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* sceneFragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexColor;

out vec4 FragColor;

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    vec3 emissiveColor;
};

struct DirLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
};

uniform Material material;
uniform DirLight dirLight;
uniform vec3 ambientGlobal;
uniform vec3 viewPos;

#define MAX_POINT_LIGHTS 16
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform bool spotlightsActive;
uniform SpotLight leftSpotlight;
uniform SpotLight rightSpotlight;

uniform vec3 skyColor;
uniform float sunMultiplier;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffColor) {
    vec3 lightDir = light.position - fragPos;
    float distance = length(lightDir);
    if (distance > light.radius) return vec3(0.0);
    lightDir = normalize(lightDir);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * diff * diffColor;

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specularColor;

    // Smooth Distance Attenuation
    float atten = clamp(1.0 - (distance / light.radius), 0.0, 1.0);
    atten = atten * atten * light.intensity;

    return (diffuse + specular) * atten;
}

vec3 calcSpotlight(SpotLight spot, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffColor) {
    vec3 lightDir = normalize(spot.position - fragPos);
    float theta = dot(lightDir, normalize(-spot.direction));
    float epsilon = spot.cutOff - spot.outerCutOff;
    float intensity = clamp((theta - spot.outerCutOff) / epsilon, 0.0, 1.0);

    if (intensity <= 0.0) return vec3(0.0);

    float distance = length(spot.position - fragPos);
    float atten = 1.0 / (1.0 + 0.04 * distance + 0.016 * distance * distance);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = spot.color * diff * diffColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = spot.color * spec * material.specularColor;

    return (diffuse + specular) * atten * intensity;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Base object diffuse color modulated by vertex color
    vec3 baseColor = material.diffuseColor * VertexColor;

    // 1. Ambient Lighting
    vec3 result = ambientGlobal * baseColor;

    // 2. Directional Sunlight / Moonlight (attenuated by sunMultiplier for subterranean tunnel)
    vec3 lightDir = normalize(-dirLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 dirDiffuse = dirLight.color * diff * baseColor * sunMultiplier;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 dirSpecular = dirLight.color * spec * material.specularColor * sunMultiplier;

    result += dirDiffuse + dirSpecular;

    // 3. Point Lights (Street lamps & Tunnel fixtures)
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; ++i) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor);
    }

    // 4. Car Twin Headlight Spotlights
    if (spotlightsActive) {
        result += calcSpotlight(leftSpotlight, norm, FragPos, viewDir, baseColor);
        result += calcSpotlight(rightSpotlight, norm, FragPos, viewDir, baseColor);
    }

    // 5. Emissive Glow (Car lights, lamp diffusers)
    result += material.emissiveColor;

    // 6. Subtle Distance Fog
    float distToCam = length(viewPos - FragPos);
    float fogDensity = 0.007;
    float fogFactor = clamp(1.0 - exp(-distToCam * fogDensity), 0.0, 1.0);
    result = mix(result, skyColor, fogFactor);

    FragColor = vec4(result, 1.0);
}
)";

const char* hudVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

out vec4 Color;
uniform mat4 projection;

void main() {
    Color = aColor;
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* hudFragmentShaderSource = R"(
#version 330 core
in vec4 Color;
out vec4 FragColor;

void main() {
    FragColor = Color;
}
)";

// ==========================================
// Window & Input Callbacks
// ==========================================

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    if (width <= 0 || height <= 0) return;
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastMouseX);
    float yoffset = static_cast<float>(lastMouseY - ypos); // Reversed since y-coords go from bottom to top

    lastMouseX = xpos;
    lastMouseY = ypos;

    if (camera.mode == CameraMode::ORBIT_FREE && mouseCaptured) {
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouseCaptured = !mouseCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Camera Mode Switching
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) camera.setMode(CameraMode::CHASE);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) camera.setMode(CameraMode::FIRST_PERSON);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) camera.setMode(CameraMode::BIRDS_EYE);
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        camera.setMode(CameraMode::ORBIT_FREE);
        if (!mouseCaptured) {
            mouseCaptured = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) camera.setMode(CameraMode::SCENIC_SIDE);

    // Free camera navigation (WASDQE)
    if (camera.mode == CameraMode::ORBIT_FREE) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard(0, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard(1, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard(2, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard(3, dt);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.processKeyboard(4, dt);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.processKeyboard(5, dt);
    }

    // Toggle Day / Night mode [N]
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !keyStates[GLFW_KEY_N]) {
        scene.isNight = !scene.isNight;
        keyStates[GLFW_KEY_N] = true;
    } else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_N] = false;
    }

    // Toggle Auto Day/Night cycle [T]
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !keyStates[GLFW_KEY_T]) {
        scene.autoDayNight = !scene.autoDayNight;
        keyStates[GLFW_KEY_T] = true;
    } else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_T] = false;
    }

    // Pause / Resume car movement [SPACE]
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !keyStates[GLFW_KEY_SPACE]) {
        isPaused = !isPaused;
        keyStates[GLFW_KEY_SPACE] = true;
    } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_SPACE] = false;
    }

    // Toggle Tree Shearing animation [S] (when not in free camera)
    if (camera.mode != CameraMode::ORBIT_FREE) {
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !keyStates[GLFW_KEY_S]) {
            scene.enableTreeShear = !scene.enableTreeShear;
            keyStates[GLFW_KEY_S] = true;
        } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
            keyStates[GLFW_KEY_S] = false;
        }
    }

    // Toggle Headlights override [H]
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !keyStates[GLFW_KEY_H]) {
        scene.headlightsActive = !scene.headlightsActive;
        keyStates[GLFW_KEY_H] = true;
    } else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_H] = false;
    }

    // Cycle Scenic spot [C]
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !keyStates[GLFW_KEY_C]) {
        camera.nextScenicSpot();
        keyStates[GLFW_KEY_C] = true;
    } else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_C] = false;
    }

    // Reset back to Garage [R]
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !keyStates[GLFW_KEY_R]) {
        scene.carDistance = 0.0f;
        scene.carSpeed = 0.0f;
        scene.isJourneyComplete = false;
        keyStates[GLFW_KEY_R] = true;
    } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_R] = false;
    }

    // Capture Screenshot [P]
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyStates[GLFW_KEY_P]) {
        ScreenshotUtil::saveBMP("screenshot.bmp", windowWidth, windowHeight);
        keyStates[GLFW_KEY_P] = true;
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_P] = false;
    }
}

// ==========================================
// Main Function
// ==========================================

int main(int argc, char** argv) {
    bool captureDemos = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--capture-demos") {
            captureDemos = true;
        }
    }

    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x Multisampling MSAA

    // 2. Create Window
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, 
        "3D Autonomous Car Journey - OpenGL 3.3 Core Profile", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 3. Initialize GLAD
    int version = gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    if (!version) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "===================================================\n";
    std::cout << "  3D OpenGL Autonomous Car Journey Simulation\n";
    std::cout << "  OpenGL Version: " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << "\n";
    std::cout << "  GPU / Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "===================================================\n";
    std::cout << "  Controls:\n";
    std::cout << "  [1] Chase Camera      [2] Cockpit View     [3] Birds-Eye Cam\n";
    std::cout << "  [4] Free Fly Cam      [5] Scenic Cam       [C] Cycle Scenic Spot\n";
    std::cout << "  [N] Day/Night Mode    [T] Auto Day/Night   [SPACE] Pause/Resume\n";
    std::cout << "  [S] Toggle Tree Wind Shearing              [H] Toggle Headlights\n";
    std::cout << "  [R] Reset to Garage   [Right Click] Toggle Mouse in Free Cam\n";
    std::cout << "===================================================\n";

    // OpenGL Global Configuration
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);

    // 4. Initialize Shaders
    Shader sceneShader(sceneVertexShaderSource, sceneFragmentShaderSource);
    Shader hudShader(hudVertexShaderSource, hudFragmentShaderSource);

    // 5. Initialize Track, Scene, and HUD
    scene.init(track);
    hud.init();

    // Timing
    float lastFrameTime = static_cast<float>(glfwGetTime());
    float titleTimer = 0.0f;

    // 6. Main Render Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Cap delta time to prevent large steps when dragging window
        if (dt > 0.1f) dt = 0.1f;

        // Automated demonstration frame setup if requested via --capture-demos
        static int demoFrame = 0;
        if (captureDemos) {
            demoFrame++;
            if (demoFrame <= 10) {
                // View 1: Start Point at building named GARAGE (Verifying building, 3D GARAGE sign, start gantry)
                camera.setMode(CameraMode::CHASE);
                scene.carDistance = 0.5f;
            } else if (demoFrame <= 20) {
                // View 2: Bridge road deck under arch (Verifying arches, river, boats)
                camera.setMode(CameraMode::CHASE);
                scene.carDistance = track.cumulativeDistances[10] + 6.0f;
            } else if (demoFrame <= 30) {
                // View 3: Inside mountain tunnel (Verifying curved vault, dual LED strips, overhead signals)
                camera.setMode(CameraMode::CHASE);
                scene.carDistance = track.cumulativeDistances[15] + 8.5f;
            } else if (demoFrame <= 40) {
                // View 4: Emergence into countryside (Verifying open highway, wind-sheared trees)
                camera.setMode(CameraMode::CHASE);
                scene.carDistance = track.cumulativeDistances[18] + 6.0f;
            } else if (demoFrame <= 50) {
                // View 5: Western return road (Verifying clean open road, no tree on road, no cliff block)
                camera.setMode(CameraMode::CHASE);
                scene.carDistance = track.cumulativeDistances[28] + 6.0f;
            } else if (demoFrame <= 60) {
                // View 6: Return road smooth curve approaching Garage (Verifying ZERO zigzag, ZERO z-fighting)
                camera.setMode(CameraMode::CHASE);
                scene.carDistance = track.cumulativeDistances[31] + 2.0f;
            } else if (demoFrame <= 70) {
                // View 7: Journey Completed - Car parked at Garage finish line (Verifying final stop & HUD banner)
                camera.setMode(CameraMode::CHASE);
                scene.carDistance = track.totalLength;
                scene.isJourneyComplete = true;
                scene.carSpeed = 0.0f;
            } else if (demoFrame <= 80) {
                // View 8: Bridge and River panoramic view
                camera.setMode(CameraMode::SCENIC_SIDE);
                camera.currentScenicSpot = 2;
                scene.carDistance = track.cumulativeDistances[10];
            }

            // Immediately sample car position along track and snap chase camera!
            EnvironmentZone z; float spd;
            track.sample(scene.carDistance, scene.carPos, scene.carForward, scene.carUp, z, spd);
            scene.currentZone = z;
            scene.carRight = m3d::cross(scene.carForward, scene.carUp).normalized();
            if (camera.mode == CameraMode::CHASE) {
                camera.snapToCar(scene.carPos, scene.carForward, scene.carUp);
            }
        }

        // Process Keyboard & Mouse
        processInput(window, dt);

        // Update Car Physics and Environment
        if (!isPaused) {
            scene.update(dt, track);
        }

        // Update Camera
        camera.update(dt, scene.carPos, scene.carForward, scene.carUp);

        // Sky Clear Color (interpolates between day sky and starry night)
        m3d::Vec3 skyColorDay(0.42f, 0.68f, 0.92f);
        m3d::Vec3 skyColorNight(0.025f, 0.035f, 0.065f);
        m3d::Vec3 currentSky = scene.isNight ? skyColorNight : skyColorDay;

        glClearColor(currentSky.x, currentSky.y, currentSky.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute 3D Projection & View Matrices
        float aspect = (float)windowWidth / (float)windowHeight;
        m3d::Mat4 projection = m3d::Mat4::perspective(m3d::radians(55.0f), aspect, 0.1f, 350.0f);
        m3d::Mat4 view = camera.getViewMatrix();

        // Setup 3D Scene Shader
        sceneShader.use();
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("view", view);
        sceneShader.setVec3("skyColor", currentSky);
        sceneShader.setFloat("sunMultiplier", 1.0f);

        // Setup Scene Lighting (Directional Sun/Moon, Point Lights, Car Headlights)
        scene.setupLighting(sceneShader, camera.position);

        // -------------------------------------------------------------
        // Render 3D Environment Components
        // -------------------------------------------------------------

        // 1. Terrain Meadow Ground
        ModelRenderer::renderTerrain(sceneShader, scene);

        // 2. Asphalt Road Ribbon & Dashed Markings
        {
            glDisable(GL_CULL_FACE);
            ModelRenderer::setMaterial(sceneShader, m3d::Vec3(0.20f, 0.20f, 0.22f), 0.2f, 16.0f);
            sceneShader.setMat4("model", m3d::Mat4::identity());
            scene.roadMesh.draw();

            // Bright road markings (yellow and white generated in mesh)
            ModelRenderer::setMaterial(sceneShader, m3d::Vec3(1.0f, 1.0f, 1.0f), 0.4f, 32.0f);
            scene.roadStripesMesh.draw();
            glEnable(GL_CULL_FACE);
        }

        // 3. Garage (Starting & Arrival Base)
        ModelRenderer::renderGarage(sceneShader, scene);

        // 4. Metropolitan City Area & Sheared Skyscraper
        ModelRenderer::renderCity(sceneShader, scene);

        // 5. Grand River Bridge (Deck, Pillars, Arches)
        ModelRenderer::renderBridge(sceneShader, scene);

        // 6. River & Sailing Boats (Wave Bobbing & Rocking)
        ModelRenderer::renderRiverAndBoats(sceneShader, scene);

        // 7. Mountain Hillside & Arched Dark Tunnel
        ModelRenderer::renderTunnelAndMountain(sceneShader, scene);

        // 8. Roadside Trees (Dynamic Shearing Demonstration!)
        ModelRenderer::renderTrees(sceneShader, scene);

        // 9. Street Lamps (Illuminated at Night)
        ModelRenderer::renderStreetLamps(sceneShader, scene);

        // 10. Autonomous Car (Main 3D Object with Spinning & Steering Wheels)
        ModelRenderer::renderCar(sceneShader, scene);

        // -------------------------------------------------------------
        // Render 2D Telemetry HUD Overlay
        // -------------------------------------------------------------
        float dummyTargetSpeed = 15.0f;
        m3d::Vec3 dummyP, dummyF, dummyU;
        EnvironmentZone dummyZ;
        track.sample(scene.carDistance, dummyP, dummyF, dummyU, dummyZ, dummyTargetSpeed);

        hud.render(hudShader, windowWidth, windowHeight,
                   scene.carSpeed, dummyTargetSpeed, scene.currentZone,
                   scene.isNight, static_cast<int>(camera.mode),
                   scene.enableTreeShear, scene.headlightsActive,
                   scene.isJourneyComplete);

        // -------------------------------------------------------------
        // Update Window Title with Live Telemetry
        // -------------------------------------------------------------
        titleTimer += dt;
        if (titleTimer >= 0.15f) {
            titleTimer = 0.0f;

            std::stringstream ss;
            ss << std::fixed << std::setprecision(1);
            if (scene.isJourneyComplete) {
                ss << "[JOURNEY COMPLETED - PARKED AT GARAGE] Press [R] to Restart | ";
            } else {
                ss << "[AUTONOMOUS JOURNEY] Zone: " << getZoneName(scene.currentZone)
                   << " | Speed: " << (scene.carSpeed * 3.6f) << " km/h (Limit: " << (dummyTargetSpeed * 3.6f) << " km/h) | ";
            }

            ss << "Mode: " << (scene.isNight ? "NIGHT" : "DAY")
               << " | Headlights: " << (scene.headlightsActive ? "ON" : "OFF")
               << " | Shearing: " << (scene.enableTreeShear ? "ACTIVE" : "OFF")
               << " | Camera: ";

            switch (camera.mode) {
                case CameraMode::CHASE: ss << "Chase [1]"; break;
                case CameraMode::FIRST_PERSON: ss << "Cockpit [2]"; break;
                case CameraMode::BIRDS_EYE: ss << "Overhead [3]"; break;
                case CameraMode::ORBIT_FREE: ss << "Free Orbit [4]"; break;
                case CameraMode::SCENIC_SIDE: ss << "Scenic Spot " << (camera.currentScenicSpot + 1) << " [5]"; break;
            }

            if (isPaused) ss << " [PAUSED]";

            glfwSetWindowTitle(window, ss.str().c_str());
        }

        // Swap buffers and poll input events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Automated demonstration frame capture if requested via --capture-demos
        if (captureDemos) {
            if (demoFrame == 10) {
                ScreenshotUtil::saveBMP("demo_garage_start_point.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 20) {
                ScreenshotUtil::saveBMP("demo_bridge_deck.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 30) {
                ScreenshotUtil::saveBMP("demo_tunnel_inside.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 40) {
                ScreenshotUtil::saveBMP("demo_road_countryside_nohouse.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 50) {
                ScreenshotUtil::saveBMP("demo_return_road_mountain_portal_clean.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 60) {
                ScreenshotUtil::saveBMP("demo_return_road_smooth_curve.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 70) {
                ScreenshotUtil::saveBMP("demo_journey_completed_stop.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 80) {
                ScreenshotUtil::saveBMP("demo_bridge_boats_scenic.bmp", windowWidth, windowHeight);
                glfwSetWindowShouldClose(window, true);
            }
        }
    }

    // 7. Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
