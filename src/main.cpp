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

// Global Window State (Full HD 1920x1080)
int windowWidth = 1920;
int windowHeight = 1080;
Camera camera;
Track track;
SceneManager scene;
HUD hud;

bool isPaused = false;
bool mouseCaptured = false;
double lastMouseX = 960.0;
double lastMouseY = 540.0;
bool firstMouse = true;

// Key state tracking for single-press toggles
bool keyStates[1024] = {false};

// Real-time driving input flags (Arrows / WASD)
bool driveForward = false;
bool driveBackward = false;
bool driveLeft = false;
bool driveRight = false;

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
uniform vec2 uvScale = vec2(1.0, 1.0);
uniform vec2 uvOffset = vec2(0.0, 0.0);

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * aNormal);
    TexCoords = aTexCoords * uvScale + uvOffset;
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

// --- Material Definition for Complete Illumination Models ---
struct Material {
    vec3 ambient;       // Ambient reflectivity (Ka)
    vec3 diffuse;       // Diffuse reflectivity (Kd)
    vec3 specular;      // Specular reflectivity (Ks)
    float shininess;    // Shininess exponent (alpha)
    vec3 emissive;      // Self-illumination (Ke)
};

// --- Directional Light (Sun / Moon) ---
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// --- Point Light (Street Lamps, Tunnel Ceiling Fixtures, Gas Station Downlights) ---
struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float radius;
};

// --- Spot Light (Car Twin Conical Headlights) ---
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

uniform Material material;
uniform DirLight dirLight;
uniform vec3 ambientGlobal;
uniform vec3 viewPos;

uniform sampler2D diffuseTexture;
uniform bool useTexture = false;
uniform float textureBlend = 1.0;

#define MAX_POINT_LIGHTS 24
uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform bool spotlightsActive;
uniform SpotLight leftSpotlight;
uniform SpotLight rightSpotlight;

uniform vec3 skyColor;
uniform float sunMultiplier;

// =========================================================================
// Specular Calculation: Pure Blinn-Phong Halfway-Vector Model
// =========================================================================
float calcSpecularFactor(vec3 normal, vec3 lightDir, vec3 viewDir, float shininess) {
    // Blinn-Phong Shading Model: Halfway vector H = normalize(L + V)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float nDotH = max(dot(normal, halfwayDir), 0.0);
    return pow(nDotH, shininess);
}

// =========================================================================
// 1. Directional Light: Ambient + Diffuse + Specular
// =========================================================================
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 matAmb, vec3 matDiff, vec3 matSpec, float shininess) {
    vec3 lightDir = normalize(-light.direction);

    // Ambient
    vec3 ambient = light.ambient * matAmb;

    // Diffuse (Lambertian)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * matDiff;

    // Specular (Phong or Blinn-Phong)
    vec3 specular = vec3(0.0);
    if (diff > 0.0) {
        float specFactor = calcSpecularFactor(normal, lightDir, viewDir, shininess);
        specular = light.specular * specFactor * matSpec;
    }

    return (ambient + diffuse + specular) * sunMultiplier;
}

// =========================================================================
// 2. Point Light: Ambient + Diffuse + Specular with Distance Attenuation
// =========================================================================
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 matAmb, vec3 matDiff, vec3 matSpec, float shininess) {
    vec3 lightVec = light.position - fragPos;
    float distance = length(lightVec);
    if (distance > light.radius) return vec3(0.0);
    vec3 lightDir = lightVec / distance;

    // Distance Attenuation (Inverse-Square physics with soft radius cutoff)
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float radiusFactor = clamp(1.0 - (distance / light.radius), 0.0, 1.0);
    attenuation *= radiusFactor;

    // Ambient
    vec3 ambient = light.ambient * matAmb * attenuation;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * matDiff * attenuation;

    // Specular
    vec3 specular = vec3(0.0);
    if (diff > 0.0) {
        float specFactor = calcSpecularFactor(normal, lightDir, viewDir, shininess);
        specular = light.specular * specFactor * matSpec * attenuation;
    }

    return ambient + diffuse + specular;
}

// =========================================================================
// 3. Spot Light: Ambient + Diffuse + Specular with Conical Falloff & Attenuation
// =========================================================================
vec3 calcSpotlight(SpotLight spot, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 matAmb, vec3 matDiff, vec3 matSpec, float shininess) {
    vec3 lightVec = spot.position - fragPos;
    float distance = length(lightVec);
    vec3 lightDir = normalize(lightVec);

    // Conical Beam Cutoff and Smooth Edge Falloff
    float theta = dot(lightDir, normalize(-spot.direction));
    float epsilon = spot.cutOff - spot.outerCutOff;
    float spotIntensity = clamp((theta - spot.outerCutOff) / epsilon, 0.0, 1.0);

    if (spotIntensity <= 0.0) return vec3(0.0);

    // Distance Attenuation
    float attenuation = 1.0 / (spot.constant + spot.linear * distance + spot.quadratic * (distance * distance));

    // Ambient
    vec3 ambient = spot.ambient * matAmb * attenuation;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = spot.diffuse * diff * matDiff * attenuation * spotIntensity;

    // Specular
    vec3 specular = vec3(0.0);
    if (diff > 0.0) {
        float specFactor = calcSpecularFactor(normal, lightDir, viewDir, shininess);
        specular = spot.specular * specFactor * matSpec * attenuation * spotIntensity;
    }

    return ambient + diffuse + specular;
}

// =========================================================================
// Main Fragment Shader: Phong Illumination Assembly + Distance Fog
// =========================================================================
void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Base surface color modulated by vertex colors and procedural textures
    vec3 matDiff = material.diffuse * VertexColor;
    if (useTexture) {
        vec4 texSample = texture(diffuseTexture, TexCoords);
        matDiff = mix(matDiff, texSample.rgb * material.diffuse, textureBlend);
    }
    vec3 matAmb = material.ambient * (useTexture ? matDiff : VertexColor);
    vec3 matSpec = material.specular;

    // 1. Ambient Lighting (Global ambient illumination preventing black voids)
    vec3 result = ambientGlobal * matAmb;

    // 2. Directional Sunlight / Moonlight (Ambient, Diffuse, Specular via Phong)
    result += calcDirLight(dirLight, norm, viewDir, matAmb, matDiff, matSpec, material.shininess);

    // 3. Point Lights (Street lamps, tunnel fixtures, gas station downlights)
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; ++i) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, matAmb, matDiff, matSpec, material.shininess);
    }

    // 4. Spotlights (Car twin front headlights with forward conical road projection)
    if (spotlightsActive) {
        result += calcSpotlight(leftSpotlight, norm, FragPos, viewDir, matAmb, matDiff, matSpec, material.shininess);
        result += calcSpotlight(rightSpotlight, norm, FragPos, viewDir, matAmb, matDiff, matSpec, material.shininess);
    }

    // 5. Emissive Lighting (Self-illumination independent of scene lights)
    result += material.emissive;

    // 6. Atmospheric Distance Fog
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

    // Car Manual Driving Inputs (Up/Down/Left/Right arrow keys & W/S/A/D)
    driveForward = (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
    driveBackward = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
    driveLeft = (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
    driveRight = (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);

    if (camera.mode != CameraMode::ORBIT_FREE) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) driveForward = true;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) driveBackward = true;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) driveLeft = true;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) driveRight = true;
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

    // Toggle Manual Driving / Autonomous Drive [M]
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !keyStates[GLFW_KEY_M]) {
        scene.isManualDrive = !scene.isManualDrive;
        keyStates[GLFW_KEY_M] = true;
    } else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_M] = false;
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

    // Pause / Resume movement [SPACE]
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !keyStates[GLFW_KEY_SPACE]) {
        isPaused = !isPaused;
        keyStates[GLFW_KEY_SPACE] = true;
    } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_SPACE] = false;
    }

    // Toggle Tree Shearing animation [K]
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !keyStates[GLFW_KEY_K]) {
        scene.enableTreeShear = !scene.enableTreeShear;
        keyStates[GLFW_KEY_K] = true;
    } else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_K] = false;
    }

    // Toggle Headlights override [H]
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !keyStates[GLFW_KEY_H]) {
        scene.headlightsActive = !scene.headlightsActive;
        keyStates[GLFW_KEY_H] = true;
    } else if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_H] = false;
    }

    // Toggle Point Lights (Street lamps, tunnel ceiling lights, canopy lights) [L]
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !keyStates[GLFW_KEY_L]) {
        scene.togglePointLights();
        keyStates[GLFW_KEY_L] = true;
    } else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_L] = false;
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
        scene.resetCar(track);
        keyStates[GLFW_KEY_R] = true;
    } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_R] = false;
    }

    // Refuel at Gas Station [F] (1 coin = +30% fuel)
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !keyStates[GLFW_KEY_F]) {
        scene.refuel();
        keyStates[GLFW_KEY_F] = true;
    } else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        keyStates[GLFW_KEY_F] = false;
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
    std::cout << "  Manual Driving Controls (Arrow Keys / WASD):\n";
    std::cout << "  [UP / W] Accelerate Forward     [DOWN / S] Brake / Reverse\n";
    std::cout << "  [LEFT / A] Steer Left           [RIGHT / D] Steer Right\n";
    std::cout << "  [M] Toggle Manual / Auto Drive  [R] Reset Car to Garage Start\n";
    std::cout << "  [F] Refuel at Gas Station (1 Coin = +30% Fuel)\n";
    std::cout << "  ---------------------------------------------------\n";
    std::cout << "  HOW TO OBSERVE THE 5 LIGHTINGS & BLINN-PHONG SHADING:\n";
    std::cout << "  Shading Model: Pure Blinn-Phong Halfway-Vector Illumination\n";
    std::cout << "  [N] Toggle Day / Night (Directional Sun vs. Ambient Moonlight)\n";
    std::cout << "  [H] Toggle Headlights (Twin Forward Spotlights with Cutoffs)\n";
    std::cout << "  [L] Toggle Point Lights (15 Active Street & Tunnel Fixtures)\n";
    std::cout << "  Emissive Lights: Golden Coins, Gas Station Neon, Taillights\n";
    std::cout << "  ---------------------------------------------------\n";
    std::cout << "  Camera Perspectives:\n";
    std::cout << "  [1] Chase Cam (Follow Car)      [2] Cockpit Driver View\n";
    std::cout << "  [3] Birds-Eye Cam               [4] Free Orbit Cam\n";
    std::cout << "  [5] Scenic Cam   [C] Cycle Scenic Spots (Tunnel/Bridge/Gas Station)\n";
    std::cout << "  Environment Extras:\n";
    std::cout << "  [T] Auto Day/Night   [K] Wind Shearing   [SPACE] Pause   [P] Screenshot\n";
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

    sceneShader.use();
    sceneShader.setInt("diffuseTexture", 0);
    sceneShader.setBool("useTexture", false);
    sceneShader.setFloat("textureBlend", 1.0f);
    sceneShader.setVec2("uvScale", m3d::Vec2(1.0f, 1.0f));
    sceneShader.setVec2("uvOffset", m3d::Vec2(0.0f, 0.0f));

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
            scene.isManualDrive = false;
            demoFrame++;
            float spd = 0.0f;
            if (demoFrame <= 15) {
                // View 1: Blinn-Phong Shading on Car Body & Road in Daytime Sunlight
                camera.setMode(CameraMode::CHASE);
                scene.isNight = false;
                scene.headlightsActive = false;
                scene.carDistance = 14.0f;
                scene.coinsCollected = 3;
                scene.fuel = 88.0f;
                scene.currentMilestoneIdx = 0;
                scene.triggerNotification("BLINN-PHONG SHADING: HALFWAY VECTOR (N * H)", 4.0f);
                track.sample(scene.carDistance, scene.carPos, scene.carForward, scene.carUp, scene.currentZone, spd);
                scene.carRight = m3d::cross(scene.carForward, scene.carUp).normalized();
                camera.snapToCar(scene.carPos, scene.carForward, scene.carUp);
            } else if (demoFrame <= 30) {
                // View 2: Nighttime Twin Headlight Conical Spotlights & Road Illumination
                camera.setMode(CameraMode::CHASE);
                scene.isNight = true;
                scene.headlightsActive = true;
                scene.carDistance = 16.0f;
                scene.coinsCollected = 3;
                scene.fuel = 85.0f;
                scene.triggerNotification("SPOTLIGHTS: TWIN CONICAL BEAMS (22-30 DEG)", 4.0f);
                track.sample(scene.carDistance, scene.carPos, scene.carForward, scene.carUp, scene.currentZone, spd);
                scene.carRight = m3d::cross(scene.carForward, scene.carUp).normalized();
                camera.snapToCar(scene.carPos, scene.carForward, scene.carUp);
            } else if (demoFrame <= 45) {
                // View 3: Point Lights & Emissive LED Strips inside Tunnel Vault
                camera.setMode(CameraMode::SCENIC_SIDE);
                camera.currentScenicSpot = 1; // Inside Tunnel Vault
                scene.isNight = false;
                scene.headlightsActive = true;
                scene.carDistance = track.cumulativeDistances[14];
                scene.triggerNotification("POINT LIGHTS: FLUORESCENT TUNNEL FIXTURES", 4.0f);
                track.sample(scene.carDistance, scene.carPos, scene.carForward, scene.carUp, scene.currentZone, spd);
            } else if (demoFrame <= 60) {
                // View 4: Gas Station Canopy Downlights & Neon Emissive Signage at Night
                camera.setMode(CameraMode::CHASE);
                scene.isNight = true;
                scene.headlightsActive = true;
                scene.carPos = m3d::Vec3(25.5f, 0.0f, 7.5f);
                scene.carForward = m3d::Vec3(0.85f, 0.0f, -0.52f).normalized();
                scene.carRight = m3d::cross(scene.carForward, m3d::Vec3(0.0f, 1.0f, 0.0f)).normalized();
                scene.carUp = m3d::Vec3(0.0f, 1.0f, 0.0f);
                scene.isNearGasStation = true;
                scene.activeStationName = "City Gateway Gas Station";
                scene.coinsCollected = 5;
                scene.fuel = 62.0f;
                scene.triggerNotification("POINT LIGHTS: GAS STATION CANOPY DOWNLIGHTS", 4.0f);
                camera.snapToCar(scene.carPos, scene.carForward, scene.carUp);
            } else if (demoFrame <= 75) {
                // View 5: Grand River Bridge Crossing & Water Specular Highlights
                camera.setMode(CameraMode::SCENIC_SIDE);
                camera.currentScenicSpot = 0; // River Bridge
                scene.isNight = false;
                scene.headlightsActive = false;
                scene.carDistance = track.cumulativeDistances[8];
                scene.coinsCollected = 4;
                scene.fuel = 75.0f;
                scene.triggerNotification("BLINN-PHONG SPECULAR ON RIVER & ARCH BRIDGE", 4.0f);
                track.sample(scene.carDistance, scene.carPos, scene.carForward, scene.carUp, scene.currentZone, spd);
            } else if (demoFrame <= 90) {
                // View 6: Golden Coin Collectibles with Emissive Glow and Specular Highlights
                camera.setMode(CameraMode::CHASE);
                scene.isNight = false;
                scene.headlightsActive = false;
                scene.carDistance = track.cumulativeDistances[24] - 14.0f;
                scene.coinsCollected = 8;
                scene.fuel = 78.0f;
                scene.triggerNotification("EMISSIVE GLOW & GOLD SPECULAR HIGHLIGHTS", 4.0f);
                track.sample(scene.carDistance, scene.carPos, scene.carForward, scene.carUp, scene.currentZone, spd);
                scene.carRight = m3d::cross(scene.carForward, scene.carUp).normalized();
                camera.snapToCar(scene.carPos, scene.carForward, scene.carUp);
            }
        }

        // Process Keyboard & Mouse
        processInput(window, dt);

        // Update Car Physics and Environment
        if (!isPaused) {
            scene.update(dt, track, driveForward, driveBackward, driveLeft, driveRight);
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
            sceneShader.setBool("useTexture", true);
            sceneShader.setFloat("textureBlend", 0.85f);
            sceneShader.setVec2("uvScale", m3d::Vec2(1.0f, 75.0f));
            sceneShader.setVec2("uvOffset", m3d::Vec2(0.0f, 0.0f));
            scene.textures.bind("asphalt", 0);
            ModelRenderer::setMaterial(sceneShader, m3d::Vec3(0.35f, 0.35f, 0.38f), 0.2f, 16.0f);
            sceneShader.setMat4("model", m3d::Mat4::identity());
            scene.roadMesh.draw();
            scene.textures.unbind(0);
            sceneShader.setBool("useTexture", false);
            sceneShader.setVec2("uvScale", m3d::Vec2(1.0f, 1.0f));

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

        // 10. Roadside Fuel Stations (Canopies, Dual Pumps, Convenience Stores, Price Totems)
        ModelRenderer::renderGasStations(sceneShader, scene);

        // 11. Milestone Checkpoint Gates (Overhead Gantries with LED chevrons)
        ModelRenderer::renderMilestoneGates(sceneShader, scene);

        // 12. Golden Coin Collectibles (Arcade 3D Spin, Hover Bob, Pop-Flash on Collect)
        ModelRenderer::renderCoins(sceneShader, scene);

        // 13. Autonomous Car (Main 3D Object with Spinning & Steering Wheels)
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
                   scene.isJourneyComplete, scene.isManualDrive, scene.isReversing,
                   scene.fuel, scene.coinsCollected, scene.currentMilestoneIdx, scene.totalMilestones,
                   scene.isNearGasStation, scene.notificationTimer, scene.globalTime,
                   scene.notificationMessage, scene.isCollidingWithObstacle, scene.activeObstacleName);

        // -------------------------------------------------------------
        // Update Window Title with Live Telemetry
        // -------------------------------------------------------------
        titleTimer += dt;
        if (titleTimer >= 0.15f) {
            titleTimer = 0.0f;

            std::stringstream ss;
            ss << std::fixed << std::setprecision(1);
            if (scene.isManualDrive) {
                ss << "[GAME] Coins: " << scene.coinsCollected << " | "
                   << "Fuel: " << static_cast<int>(scene.fuel) << "% | "
                   << "Gate: " << (scene.currentMilestoneIdx + 1) << "/" << scene.totalMilestones << " | "
                   << "Speed: " << (std::abs(scene.carSpeed) * 3.6f) << " km/h | ";
                if (scene.isNearGasStation) {
                    ss << "AT " << scene.activeStationName << " -> PRESS [F] TO REFUEL (+30%/COIN) | ";
                } else if (scene.fuel <= 0.0f) {
                    ss << "[OUT OF FUEL] COASTING - PRESS [R] TO TOW/RESET | ";
                } else if (scene.fuel < 20.0f) {
                    ss << "[LOW FUEL] VISIT GAS STATION TO REFUEL | ";
                }
            } else if (scene.isJourneyComplete) {
                ss << "[AUTONOMOUS COMPLETE - AT GARAGE] Press [R] to Restart | ";
            } else {
                ss << "[AUTONOMOUS] Coins: " << scene.coinsCollected << " | "
                   << "Fuel: " << static_cast<int>(scene.fuel) << "% | "
                   << "Zone: " << getZoneName(scene.currentZone)
                   << " | Speed: " << (scene.carSpeed * 3.6f) << " km/h | ";
            }

            ss << "Mode: " << (scene.isNight ? "NIGHT" : "DAY") << " [N]"
               << " | Shading: BLINN-PHONG"
               << " | Headlights: " << (scene.headlightsActive ? "ON" : "OFF") << " [H]"
               << " | PointLights: " << (scene.pointLightsActive ? "ON" : "OFF") << " [L]"
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
            if (demoFrame == 15) {
                ScreenshotUtil::saveBMP("lighting_phong_daytime_car.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 30) {
                ScreenshotUtil::saveBMP("lighting_phong_night_headlights_spot.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 45) {
                ScreenshotUtil::saveBMP("lighting_point_lights_tunnel.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 60) {
                ScreenshotUtil::saveBMP("lighting_gas_station_canopy_point.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 75) {
                ScreenshotUtil::saveBMP("lighting_bridge_scenic.bmp", windowWidth, windowHeight);
            } else if (demoFrame == 90) {
                ScreenshotUtil::saveBMP("lighting_coins_emissive_shine.bmp", windowWidth, windowHeight);
                glfwSetWindowShouldClose(window, true);
            }
        }
    }

    // 7. Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
