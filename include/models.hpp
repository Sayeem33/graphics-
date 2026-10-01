#pragma once
#ifndef MODELS_HPP
#define MODELS_HPP

#include "math3d.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "scene.hpp"
#include <cmath>

class ModelRenderer {
public:
    static void setMaterial(const Shader& shader, const m3d::Vec3& diffuse, 
                            float specularIntensity = 0.4f, float shininess = 32.0f,
                            const m3d::Vec3& emissive = {0, 0, 0}) {
        shader.setVec3("material.diffuseColor", diffuse);
        shader.setVec3("material.specularColor", m3d::Vec3(specularIntensity));
        shader.setFloat("material.shininess", shininess);
        shader.setVec3("material.emissiveColor", emissive);
    }

    // -------------------------------------------------------------
    // 1. AUTONOMOUS CAR (Hierarchical 3D Model with Spinning/Steering Wheels)
    // -------------------------------------------------------------
    static void renderCar(const Shader& shader, SceneManager& scene) {
        // Base car model matrix: Translation along track + Rotation (Yaw/Pitch/Roll)
        m3d::Mat4 carBase = m3d::Mat4::identity();
        carBase(0, 0) = scene.carRight.x;   carBase(0, 1) = scene.carUp.x;   carBase(0, 2) = scene.carForward.x;   carBase(0, 3) = scene.carPos.x;
        carBase(1, 0) = scene.carRight.y;   carBase(1, 1) = scene.carUp.y;   carBase(1, 2) = scene.carForward.y;   carBase(1, 3) = scene.carPos.y + 0.38f;
        carBase(2, 0) = scene.carRight.z;   carBase(2, 1) = scene.carUp.z;   carBase(2, 2) = scene.carForward.z;   carBase(2, 3) = scene.carPos.z;

        // [A] Lower Chassis / Main Body (Metallic Sports Red)
        m3d::Vec3 carBodyColor(0.85f, 0.12f, 0.16f);
        setMaterial(shader, carBodyColor, 0.7f, 64.0f);
        {
            m3d::Mat4 m = carBase * m3d::Mat4::translate(0.0f, 0.22f, 0.0f) * m3d::Mat4::scale(1.7f, 0.45f, 3.8f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();
        }

        // [B] Front Hood Slope / Wedge (Translation + Scale)
        {
            m3d::Mat4 m = carBase * m3d::Mat4::translate(0.0f, 0.35f, 0.95f) * m3d::Mat4::scale(1.65f, 0.25f, 1.6f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();
        }

        // [C] Cabin / Greenhouse with Tinted Windshield Glass
        setMaterial(shader, m3d::Vec3(0.08f, 0.10f, 0.14f), 0.9f, 128.0f); // Tinted glossy glass
        {
            m3d::Mat4 m = carBase * m3d::Mat4::translate(0.0f, 0.65f, -0.3f) * m3d::Mat4::scale(1.4f, 0.52f, 2.0f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();
        }

        // [D] Cabin Roof Cap (Matches car body)
        setMaterial(shader, carBodyColor, 0.7f, 64.0f);
        {
            m3d::Mat4 m = carBase * m3d::Mat4::translate(0.0f, 0.93f, -0.35f) * m3d::Mat4::scale(1.35f, 0.08f, 1.8f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();
        }

        // [E] Front Bumper & Dark Grille
        setMaterial(shader, m3d::Vec3(0.12f, 0.12f, 0.14f), 0.2f, 16.0f);
        {
            m3d::Mat4 m = carBase * m3d::Mat4::translate(0.0f, 0.15f, 1.92f) * m3d::Mat4::scale(1.68f, 0.3f, 0.15f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();
        }

        // [F] Rear Diffuser & Aerodynamic Spoiler
        setMaterial(shader, m3d::Vec3(0.1f, 0.1f, 0.1f), 0.5f, 32.0f);
        {
            // Spoiler wing
            m3d::Mat4 mWing = carBase * m3d::Mat4::translate(0.0f, 0.75f, -1.9f) * m3d::Mat4::scale(1.6f, 0.06f, 0.35f);
            shader.setMat4("model", mWing);
            scene.cubeMesh.draw();

            // Left & Right spoiler struts
            m3d::Mat4 mStrutL = carBase * m3d::Mat4::translate(-0.55f, 0.55f, -1.88f) * m3d::Mat4::scale(0.06f, 0.35f, 0.1f);
            shader.setMat4("model", mStrutL);
            scene.cubeMesh.draw();

            m3d::Mat4 mStrutR = carBase * m3d::Mat4::translate(0.55f, 0.55f, -1.88f) * m3d::Mat4::scale(0.06f, 0.35f, 0.1f);
            shader.setMat4("model", mStrutR);
            scene.cubeMesh.draw();
        }

        // [G] Headlights (Emissive glowing capsules when active)
        m3d::Vec3 headlightEmissive = scene.headlightsActive ? m3d::Vec3(1.2f, 1.2f, 1.0f) : m3d::Vec3(0.1f, 0.1f, 0.1f);
        setMaterial(shader, m3d::Vec3(0.95f, 0.95f, 1.0f), 0.9f, 64.0f, headlightEmissive);
        {
            // Left headlight
            m3d::Mat4 mL = carBase * m3d::Mat4::translate(-0.62f, 0.28f, 1.91f) * m3d::Mat4::scale(0.32f, 0.14f, 0.08f);
            shader.setMat4("model", mL);
            scene.cubeMesh.draw();

            // Right headlight
            m3d::Mat4 mR = carBase * m3d::Mat4::translate(0.62f, 0.28f, 1.91f) * m3d::Mat4::scale(0.32f, 0.14f, 0.08f);
            shader.setMat4("model", mR);
            scene.cubeMesh.draw();
        }

        // [H] Taillights (Emissive red bars)
        m3d::Vec3 taillightEmissive = scene.isNight ? m3d::Vec3(1.0f, 0.05f, 0.05f) : m3d::Vec3(0.4f, 0.02f, 0.02f);
        setMaterial(shader, m3d::Vec3(0.8f, 0.05f, 0.05f), 0.5f, 32.0f, taillightEmissive);
        {
            m3d::Mat4 mL = carBase * m3d::Mat4::translate(-0.62f, 0.32f, -1.91f) * m3d::Mat4::scale(0.35f, 0.12f, 0.08f);
            shader.setMat4("model", mL);
            scene.cubeMesh.draw();

            m3d::Mat4 mR = carBase * m3d::Mat4::translate(0.62f, 0.32f, -1.91f) * m3d::Mat4::scale(0.35f, 0.12f, 0.08f);
            shader.setMat4("model", mR);
            scene.cubeMesh.draw();
        }

        // [I] Autonomous Sensor Dome (LiDAR puck on roof, spinning)
        setMaterial(shader, m3d::Vec3(0.15f, 0.15f, 0.18f), 0.5f, 32.0f);
        {
            float lidarRot = scene.globalTime * 720.0f; // Rapidly spins 720 deg/s
            m3d::Mat4 mLidar = carBase * m3d::Mat4::translate(0.0f, 1.05f, -0.2f) * 
                               m3d::Mat4::rotateY(m3d::radians(lidarRot)) * 
                               m3d::Mat4::scale(0.32f, 0.16f, 0.32f);
            shader.setMat4("model", mLidar);
            scene.cylinderMesh.draw();
        }

        // [J] 4 WHEELS (Demonstrating Rotation around local axle & Steering Angle)
        float wheelRadius = 0.38f;
        float wheelThickness = 0.26f;
        float wheelTrackX = 0.88f; // Half car width offset
        float frontWheelZ = 1.15f;
        float rearWheelZ = -1.15f;

        // Wheel positions: Front-Left, Front-Right, Rear-Left, Rear-Right
        struct WheelDef {
            float x, z;
            bool isFront;
        };
        WheelDef wheelDefs[4] = {
            { -wheelTrackX,  frontWheelZ, true  },
            {  wheelTrackX,  frontWheelZ, true  },
            { -wheelTrackX,  rearWheelZ,  false },
            {  wheelTrackX,  rearWheelZ,  false }
        };

        for (int i = 0; i < 4; ++i) {
            const auto& w = wheelDefs[i];

            // 1. Position wheel relative to car chassis
            m3d::Mat4 wMat = carBase * m3d::Mat4::translate(w.x, 0.0f, w.z);

            // 2. If front wheel, steer around Y axis
            if (w.isFront) {
                wMat = wMat * m3d::Mat4::rotateY(m3d::radians(scene.wheelSteerAngle));
            }

            // 3. Spin around X axis based on vehicle forward movement
            wMat = wMat * m3d::Mat4::rotateX(m3d::radians(scene.wheelSpinAngle));

            // Rotate cylinder from Y-axis to Z/X-axis for rolling orientation
            m3d::Mat4 orientMat = wMat * m3d::Mat4::rotateZ(m3d::radians(90.0f));

            // Dark Rubber Tire
            setMaterial(shader, m3d::Vec3(0.08f, 0.08f, 0.09f), 0.15f, 16.0f);
            m3d::Mat4 mRubber = orientMat * m3d::Mat4::scale(wheelRadius * 2.0f, wheelThickness, wheelRadius * 2.0f);
            shader.setMat4("model", mRubber);
            scene.cylinderMesh.draw();

            // Silver Alloy Rim / Hubcap
            setMaterial(shader, m3d::Vec3(0.75f, 0.75f, 0.80f), 0.85f, 64.0f);
            m3d::Mat4 mRim = orientMat * m3d::Mat4::scale(wheelRadius * 1.3f, wheelThickness * 1.05f, wheelRadius * 1.3f);
            shader.setMat4("model", mRim);
            scene.cylinderMesh.draw();
        }
    }

    // -------------------------------------------------------------
    // 2. GARAGE (Departure & Starting Location - Building named GARAGE)
    // -------------------------------------------------------------
    // 2. GARAGE (Departure & Starting Location - Building named GARAGE)
    // -------------------------------------------------------------
    static void renderGarage(const Shader& shader, SceneManager& scene) {
        m3d::Vec3 garagePos(-48.0f, 0.0f, 38.0f);

        // Concrete Forecourt / Parking Apron (Y = 0.01m, strictly below road at Y = 0.08m, ends at road edge Z = 32.8m)
        setMaterial(shader, m3d::Vec3(0.36f, 0.36f, 0.38f), 0.2f, 16.0f);
        {
            m3d::Mat4 m = m3d::Mat4::translate(garagePos.x, 0.01f, 33.6f) * 
                          m3d::Mat4::scale(22.0f, 0.02f, 1.6f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();
        }

        // Garage Main Building Structure (Charcoal stone / modern architectural brick)
        setMaterial(shader, m3d::Vec3(0.50f, 0.48f, 0.45f), 0.15f, 12.0f);
        {
            m3d::Mat4 mMain = m3d::Mat4::translate(garagePos.x, 3.0f, garagePos.z) * 
                              m3d::Mat4::scale(22.0f, 6.0f, 8.0f);
            shader.setMat4("model", mMain);
            scene.cubeMesh.draw();
        }

        // Metal Roof Fascia / Parapet Coping
        setMaterial(shader, m3d::Vec3(0.24f, 0.22f, 0.20f), 0.4f, 32.0f);
        {
            m3d::Mat4 mRoof = m3d::Mat4::translate(garagePos.x, 6.15f, garagePos.z) * 
                              m3d::Mat4::scale(22.8f, 0.45f, 8.8f);
            shader.setMat4("model", mRoof);
            scene.cubeMesh.draw();
        }

        // Dual Service Bay Roll-Up Doors facing South (toward road at Z = 30.0m)
        setMaterial(shader, m3d::Vec3(0.18f, 0.20f, 0.24f), 0.5f, 32.0f);
        {
            // Bay 1 (West)
            m3d::Mat4 mBay1 = m3d::Mat4::translate(garagePos.x - 5.5f, 2.1f, garagePos.z - 3.95f) * 
                              m3d::Mat4::scale(6.5f, 4.2f, 0.25f);
            shader.setMat4("model", mBay1);
            scene.cubeMesh.draw();

            // Bay 2 (East)
            m3d::Mat4 mBay2 = m3d::Mat4::translate(garagePos.x + 5.5f, 2.1f, garagePos.z - 3.95f) * 
                              m3d::Mat4::scale(6.5f, 4.2f, 0.25f);
            shader.setMat4("model", mBay2);
            scene.cubeMesh.draw();
        }

        // =============================================================
        // [A] PROMINENT 3D ILLUMINATED BUILDING SIGN: "G A R A G E"
        // =============================================================
        // Dark metal signboard plaque centered above garage doors
        setMaterial(shader, m3d::Vec3(0.10f, 0.10f, 0.12f), 0.3f, 16.0f);
        {
            m3d::Mat4 mPlaque = m3d::Mat4::translate(garagePos.x, 5.0f, garagePos.z - 4.10f) * 
                                m3d::Mat4::scale(12.0f, 1.45f, 0.18f);
            shader.setMat4("model", mPlaque);
            scene.cubeMesh.draw();
        }

        // Glowing 3D Block Letters: "G A R A G E"
        m3d::Vec3 signLight = scene.isNight ? m3d::Vec3(1.0f, 0.85f, 0.20f) : m3d::Vec3(0.35f, 0.30f, 0.08f);
        m3d::Vec3 signCol(0.96f, 0.82f, 0.16f); // Radiant golden amber
        setMaterial(shader, signCol, 0.85f, 64.0f, signLight);

        auto drawSegment = [&](float sx, float sy, float sz, float sw, float sh, float sd) {
            m3d::Mat4 sm = m3d::Mat4::translate(sx, sy, sz) * m3d::Mat4::scale(sw, sh, sd);
            shader.setMat4("model", sm);
            scene.cubeMesh.draw();
        };

        float lZ = garagePos.z - 4.22f;
        float lY = 5.0f;
        float bD = 0.12f; // letter bar depth
        float bT = 0.16f; // letter bar thickness

        // 1. 'G' at X = garagePos.x + 3.9f (Leftmost on screen when viewing front from road)
        {
            float cx = garagePos.x + 3.9f;
            drawSegment(cx + 0.30f, lY,         lZ, bT,    0.92f, bD); // Left spine
            drawSegment(cx,         lY + 0.38f, lZ, 0.65f, bT,    bD); // Top bar
            drawSegment(cx,         lY - 0.38f, lZ, 0.65f, bT,    bD); // Bot bar
            drawSegment(cx - 0.28f, lY - 0.15f, lZ, bT,    0.46f, bD); // Right lower vert
            drawSegment(cx - 0.14f, lY,         lZ, 0.38f, bT,    bD); // Mid inner horiz
        }

        // 2. 'A' at X = garagePos.x + 2.4f
        {
            float cx = garagePos.x + 2.4f;
            drawSegment(cx + 0.30f, lY,         lZ, bT,    0.92f, bD); // Left vert
            drawSegment(cx - 0.30f, lY,         lZ, bT,    0.92f, bD); // Right vert
            drawSegment(cx,         lY + 0.38f, lZ, 0.65f, bT,    bD); // Top horiz
            drawSegment(cx,         lY - 0.02f, lZ, 0.65f, bT,    bD); // Mid crossbar
        }

        // 3. 'R' at X = garagePos.x + 0.85f
        {
            float cx = garagePos.x + 0.85f;
            drawSegment(cx + 0.30f, lY,         lZ, bT,    0.92f, bD); // Left vert
            drawSegment(cx,         lY + 0.38f, lZ, 0.65f, bT,    bD); // Top horiz
            drawSegment(cx - 0.26f, lY + 0.20f, lZ, bT,    0.40f, bD); // Right upper vert
            drawSegment(cx,         lY + 0.02f, lZ, 0.65f, bT,    bD); // Mid horiz
            drawSegment(cx - 0.22f, lY - 0.24f, lZ, bT * 1.2f, 0.46f, bD); // Lower right leg
        }

        // 4. 'A' at X = garagePos.x - 0.85f
        {
            float cx = garagePos.x - 0.85f;
            drawSegment(cx + 0.30f, lY,         lZ, bT,    0.92f, bD); // Left vert
            drawSegment(cx - 0.30f, lY,         lZ, bT,    0.92f, bD); // Right vert
            drawSegment(cx,         lY + 0.38f, lZ, 0.65f, bT,    bD); // Top horiz
            drawSegment(cx,         lY - 0.02f, lZ, 0.65f, bT,    bD); // Mid crossbar
        }

        // 5. 'G' at X = garagePos.x - 2.4f
        {
            float cx = garagePos.x - 2.4f;
            drawSegment(cx + 0.30f, lY,         lZ, bT,    0.92f, bD); // Left spine
            drawSegment(cx,         lY + 0.38f, lZ, 0.65f, bT,    bD); // Top bar
            drawSegment(cx,         lY - 0.38f, lZ, 0.65f, bT,    bD); // Bot bar
            drawSegment(cx - 0.28f, lY - 0.15f, lZ, bT,    0.46f, bD); // Right lower vert
            drawSegment(cx - 0.14f, lY,         lZ, 0.38f, bT,    bD); // Mid inner horiz
        }

        // 6. 'E' at X = garagePos.x - 3.9f (Rightmost on screen)
        {
            float cx = garagePos.x - 3.9f;
            drawSegment(cx + 0.30f, lY,         lZ, bT,    0.92f, bD); // Left vert spine
            drawSegment(cx,         lY + 0.38f, lZ, 0.65f, bT,    bD); // Top bar
            drawSegment(cx + 0.05f, lY,         lZ, 0.52f, bT,    bD); // Mid bar
            drawSegment(cx,         lY - 0.38f, lZ, 0.65f, bT,    bD); // Bot bar
        }

        // Subtitle badge below "GARAGE"
        m3d::Vec3 subLight = scene.isNight ? m3d::Vec3(0.3f, 0.8f, 1.0f) : m3d::Vec3(0.1f, 0.2f, 0.3f);
        setMaterial(shader, m3d::Vec3(0.2f, 0.75f, 0.95f), 0.8f, 32.0f, subLight);
        {
            m3d::Mat4 mSub = m3d::Mat4::translate(garagePos.x, 4.35f, lZ) * 
                             m3d::Mat4::scale(9.0f, 0.20f, bD);
            shader.setMat4("model", mSub);
            scene.cubeMesh.draw();
        }

        // EV Rapid Charging Stations on the Forecourt
        for (int c = 0; c < 2; ++c) {
            float cx = garagePos.x - 7.5f + (float)c * 15.0f;
            float cz = garagePos.z - 4.8f;

            // Charger body
            setMaterial(shader, m3d::Vec3(0.22f, 0.24f, 0.28f), 0.3f, 16.0f);
            m3d::Mat4 mCharger = m3d::Mat4::translate(cx, 1.1f, cz) * 
                                 m3d::Mat4::scale(0.6f, 2.2f, 0.5f);
            shader.setMat4("model", mCharger);
            scene.cubeMesh.draw();

            // LED Status screen (Cyan)
            m3d::Vec3 evLight = scene.isNight ? m3d::Vec3(0.2f, 0.8f, 1.0f) : m3d::Vec3(0.1f, 0.3f, 0.4f);
            setMaterial(shader, m3d::Vec3(0.1f, 0.7f, 0.9f), 0.8f, 32.0f, evLight);
            m3d::Mat4 mScreen = m3d::Mat4::translate(cx, 1.55f, cz - 0.26f) * 
                                m3d::Mat4::scale(0.38f, 0.45f, 0.08f);
            shader.setMat4("model", mScreen);
            scene.cubeMesh.draw();
        }

        // Roadside Architectural Street Bollards on the forecourt edge
        for (int b = -1; b <= 1; ++b) {
            float bx = garagePos.x + (float)b * 7.5f;
            float bz = 33.2f;

            setMaterial(shader, m3d::Vec3(0.25f, 0.28f, 0.32f), 0.5f, 32.0f);
            m3d::Mat4 mPost = m3d::Mat4::translate(bx, 0.5f, bz) * 
                              m3d::Mat4::scale(0.25f, 1.0f, 0.25f);
            shader.setMat4("model", mPost);
            scene.cylinderMesh.draw();

            // Bollard LED Cap
            m3d::Vec3 capLight = scene.isNight ? m3d::Vec3(1.0f, 0.8f, 0.3f) : m3d::Vec3(0.2f, 0.2f, 0.1f);
            setMaterial(shader, m3d::Vec3(1.0f, 0.9f, 0.5f), 0.9f, 64.0f, capLight);
            m3d::Mat4 mCap = m3d::Mat4::translate(bx, 1.05f, bz) * 
                             m3d::Mat4::scale(0.30f, 0.12f, 0.30f);
            shader.setMat4("model", mCap);
            scene.cylinderMesh.draw();
        }
    }

    // -------------------------------------------------------------
    // 3. CITY BUILDINGS & SHEARED ARCHITECTURAL SKYSCRAPER
    // -------------------------------------------------------------
    static void renderCity(const Shader& shader, SceneManager& scene) {
        // [A] Traditional Corporate High-Rise (Blue tinted glass)
        setMaterial(shader, m3d::Vec3(0.20f, 0.35f, 0.50f), 0.8f, 64.0f);
        {
            m3d::Mat4 m = m3d::Mat4::translate(-12.0f, 12.0f, 45.0f) * m3d::Mat4::scale(10.0f, 24.0f, 10.0f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();

            // Roof Spire (Cone scaling)
            setMaterial(shader, m3d::Vec3(0.8f, 0.8f, 0.85f), 0.9f, 128.0f);
            m3d::Mat4 mSpire = m3d::Mat4::translate(-12.0f, 26.5f, 45.0f) * m3d::Mat4::scale(1.5f, 5.0f, 1.5f);
            shader.setMat4("model", mSpire);
            scene.coneMesh.draw();
        }

        // [B] Residential / Commercial Tower (Warm Concrete & Balconies)
        setMaterial(shader, m3d::Vec3(0.65f, 0.60f, 0.55f), 0.2f, 16.0f);
        {
            m3d::Mat4 m = m3d::Mat4::translate(5.0f, 9.0f, 42.0f) * m3d::Mat4::scale(12.0f, 18.0f, 8.0f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();
        }

        // [C] Corner Commercial Building with Rooftop AC units
        setMaterial(shader, m3d::Vec3(0.48f, 0.45f, 0.42f), 0.3f, 32.0f);
        {
            m3d::Mat4 m = m3d::Mat4::translate(28.0f, 7.0f, 34.0f) * m3d::Mat4::scale(9.0f, 14.0f, 9.0f);
            shader.setMat4("model", m);
            scene.cubeMesh.draw();

            // Rooftop mechanical unit
            setMaterial(shader, m3d::Vec3(0.3f, 0.3f, 0.3f), 0.4f, 32.0f);
            m3d::Mat4 mAC = m3d::Mat4::translate(28.0f, 14.8f, 34.0f) * m3d::Mat4::scale(3.5f, 1.6f, 3.5f);
            shader.setMat4("model", mAC);
            scene.cubeMesh.draw();
        }

        // [D] *** EXPLICIT SHEARING DEMONSTRATION ***
        // Modern Deconstructivist Skyscraper with X-Shear transformation!
        // Anchored in city center at (-22.0f, 0.0f, 48.0f), completely away from roadway!
        setMaterial(shader, m3d::Vec3(0.22f, 0.52f, 0.65f), 0.85f, 96.0f);
        {
            // M = Translate to location * Shear along X * Scale
            // Shear formula: x' = x + sy * y (sy = 0.22)
            m3d::Mat4 mShearedBuilding = 
                m3d::Mat4::translate(-22.0f, 0.0f, 48.0f) *
                m3d::Mat4::shearX(0.22f, 0.0f) * 
                m3d::Mat4::translate(0.0f, 10.0f, 0.0f) *
                m3d::Mat4::scale(8.0f, 20.0f, 8.0f);

            shader.setMat4("model", mShearedBuilding);
            scene.cubeMesh.draw();

            // Architectural accent bands on the sheared building
            setMaterial(shader, m3d::Vec3(0.9f, 0.75f, 0.2f), 0.7f, 64.0f);
            for (float h = 4.0f; h <= 18.0f; h += 4.5f) {
                m3d::Mat4 mBand = 
                    m3d::Mat4::translate(-22.0f, 0.0f, 48.0f) *
                    m3d::Mat4::shearX(0.22f, 0.0f) * 
                    m3d::Mat4::translate(0.0f, h, 0.0f) *
                    m3d::Mat4::scale(8.2f, 0.4f, 8.2f);
                shader.setMat4("model", mBand);
                scene.cubeMesh.draw();
            }
        }

        // [E] Elevated Sidewalks & Curbs along City Street
        setMaterial(shader, m3d::Vec3(0.42f, 0.42f, 0.44f), 0.15f, 8.0f);
        {
            // Left sidewalk block
            m3d::Mat4 mSideL = m3d::Mat4::translate(-8.0f, 0.1f, 34.5f) * m3d::Mat4::scale(24.0f, 0.2f, 2.5f);
            shader.setMat4("model", mSideL);
            scene.cubeMesh.draw();

            // Right sidewalk block
            m3d::Mat4 mSideR = m3d::Mat4::translate(-8.0f, 0.1f, 25.5f) * m3d::Mat4::scale(24.0f, 0.2f, 2.5f);
            shader.setMat4("model", mSideR);
            scene.cubeMesh.draw();
        }
    }

    // -------------------------------------------------------------
    // 4. GRAND RIVER BRIDGE (Detailed Arches, Piers, Deck & Railings)
    // -------------------------------------------------------------
    static void renderBridge(const Shader& shader, SceneManager& scene) {
        // [A] Massive Concrete Pier Columns (Strictly under bridge girder, tops at Y = 2.40m)
        setMaterial(shader, m3d::Vec3(0.48f, 0.48f, 0.50f), 0.15f, 16.0f);
        m3d::Vec3 pierPositions[4] = {
            { 10.0f, 0.0f, -39.0f },
            {  1.0f, 0.0f, -46.0f },
            {-12.0f, 0.0f, -46.5f },
            {-24.0f, 0.0f, -41.0f }
        };

        for (int i = 0; i < 4; ++i) {
            // Main pier column: rises from riverbed (Y = -2.0) to underside of deck bearing pad (Y = 2.40m)
            m3d::Mat4 mPier = m3d::Mat4::translate(pierPositions[i].x, 0.20f, pierPositions[i].z) * 
                              m3d::Mat4::scale(2.2f, 4.40f, 2.2f);
            shader.setMat4("model", mPier);
            scene.cylinderMesh.draw();

            // Pointed cutwater base at water level (deflects river current at Y = -1.20m)
            m3d::Mat4 mCutwater = m3d::Mat4::translate(pierPositions[i].x - 1.6f, -1.20f, pierPositions[i].z) * 
                                  m3d::Mat4::scale(1.8f, 1.6f, 2.0f);
            shader.setMat4("model", mCutwater);
            scene.cubeMesh.draw();

            // Pier top bearing pad (ends at Y = 2.52m, flush beneath girder bottom at Y = 2.525m; 0.73m below road!)
            m3d::Mat4 mPad = m3d::Mat4::translate(pierPositions[i].x, 2.46f, pierPositions[i].z) * 
                             m3d::Mat4::scale(4.2f, 0.12f, 3.2f);
            shader.setMat4("model", mPad);
            scene.cubeMesh.draw();
        }

        // [B] Solid Concrete Road Deck Box Girder & Safety Railings (Deck at Y = 3.20m)
        m3d::Vec3 railNodes[5] = {
            { 16.0f, 3.2f, -31.0f },
            {  6.0f, 3.2f, -44.0f },
            {-10.0f, 3.2f, -47.0f },
            {-26.0f, 3.2f, -44.0f },
            {-36.0f, 3.2f, -33.0f }
        };

        for (int i = 0; i < 4; ++i) {
            m3d::Vec3 p0 = railNodes[i];
            m3d::Vec3 p1 = railNodes[i + 1];
            m3d::Vec3 mid = (p0 + p1) * 0.5f;
            m3d::Vec3 diff = p1 - p0;
            float segLen = diff.length();
            float angleY = std::atan2(-diff.z, diff.x);

            // Structural concrete box girder beneath road deck (Y = 2.80m, thickness = 0.55m)
            setMaterial(shader, m3d::Vec3(0.34f, 0.34f, 0.37f), 0.2f, 16.0f);
            m3d::Mat4 mDeckGirder = m3d::Mat4::translate(mid.x, mid.y - 0.40f, mid.z) * 
                                    m3d::Mat4::rotateY(angleY) * 
                                    m3d::Mat4::scale(segLen * 1.02f, 0.55f, 6.4f);
            shader.setMat4("model", mDeckGirder);
            scene.cubeMesh.draw();

            // Safety railings placed at 3.25m from road center (clear of 2.8m roadway)
            setMaterial(shader, m3d::Vec3(0.88f, 0.88f, 0.90f), 0.7f, 64.0f);
            m3d::Vec3 leftOffset(-diff.z / segLen * 3.25f, 0.45f, diff.x / segLen * 3.25f);
            m3d::Mat4 mRailL = m3d::Mat4::translate(mid + leftOffset) * 
                               m3d::Mat4::rotateY(angleY) * 
                               m3d::Mat4::scale(segLen, 0.90f, 0.22f);
            shader.setMat4("model", mRailL);
            scene.cubeMesh.draw();

            m3d::Vec3 rightOffset(diff.z / segLen * 3.25f, 0.45f, -diff.x / segLen * 3.25f);
            m3d::Mat4 mRailR = m3d::Mat4::translate(mid + rightOffset) * 
                               m3d::Mat4::rotateY(angleY) * 
                               m3d::Mat4::scale(segLen, 0.90f, 0.22f);
            shader.setMat4("model", mRailR);
            scene.cubeMesh.draw();

            // Vertical stanchion posts at joints
            m3d::Mat4 mPostL = m3d::Mat4::translate(p0 + leftOffset) * m3d::Mat4::scale(0.32f, 0.98f, 0.32f);
            shader.setMat4("model", mPostL);
            scene.cubeMesh.draw();

            m3d::Mat4 mPostR = m3d::Mat4::translate(p0 + rightOffset) * m3d::Mat4::scale(0.32f, 0.98f, 0.32f);
            shader.setMat4("model", mPostR);
            scene.cubeMesh.draw();

            // Decorative parapet lantern atop outer stanchion (anchored on outer curb, zero lane intrusion)
            setMaterial(shader, m3d::Vec3(0.2f, 0.2f, 0.22f), 0.5f, 32.0f);
            m3d::Mat4 mLampPost = m3d::Mat4::translate(p0 + leftOffset * 1.04f + m3d::Vec3(0.0f, 0.65f, 0.0f)) *
                                  m3d::Mat4::scale(0.12f, 0.50f, 0.12f);
            shader.setMat4("model", mLampPost);
            scene.cylinderMesh.draw();

            m3d::Vec3 lampGlow = scene.isNight ? m3d::Vec3(1.4f, 1.1f, 0.7f) : m3d::Vec3(0.3f, 0.3f, 0.2f);
            setMaterial(shader, m3d::Vec3(1.0f, 0.95f, 0.7f), 0.9f, 64.0f, lampGlow);
            m3d::Mat4 mLampGlobe = m3d::Mat4::translate(p0 + leftOffset * 1.04f + m3d::Vec3(0.0f, 0.95f, 0.0f)) *
                                   m3d::Mat4::scale(0.32f, 0.32f, 0.32f);
            shader.setMat4("model", mLampGlobe);
            scene.sphereMesh.draw();
        }

        // [C] Overhead Steel Arch Trusses (Follows true bridge curve, anchored outside parapets at +-3.75m!)
        setMaterial(shader, m3d::Vec3(0.88f, 0.16f, 0.14f), 0.7f, 48.0f); // Iconic Crimson Bridge Arch
        {
            const int numArchSegs = 16;
            // Bridge span curve between node 3 and node 1 via node 2
            m3d::Vec3 n3 = railNodes[3]; // {-26, 3.2, -44}
            m3d::Vec3 n2 = railNodes[2]; // {-10, 3.2, -47}
            m3d::Vec3 n1 = railNodes[1]; // {  6, 3.2, -44}

            auto getBridgeArchPoint = [&](float t, m3d::Vec3& pCenter, m3d::Vec3& rNorm, float& archY) {
                if (t <= 0.5f) {
                    float f = t * 2.0f;
                    pCenter = n3 * (1.0f - f) + n2 * f;
                    m3d::Vec3 d = (n2 - n3).normalized();
                    rNorm = m3d::Vec3(-d.z, 0.0f, d.x).normalized();
                } else {
                    float f = (t - 0.5f) * 2.0f;
                    pCenter = n2 * (1.0f - f) + n1 * f;
                    m3d::Vec3 d = (n1 - n2).normalized();
                    rNorm = m3d::Vec3(-d.z, 0.0f, d.x).normalized();
                }
                archY = 3.0f + 5.5f * std::sin(t * m3d::PI);
            };

            const float archOffset = 3.75f; // Strictly outside 3.25m parapet!

            for (int s = 0; s < numArchSegs; ++s) {
                float t0 = (float)s / (float)numArchSegs;
                float t1 = (float)(s + 1) / (float)numArchSegs;

                m3d::Vec3 c0, c1, r0, r1;
                float y0, y1;
                getBridgeArchPoint(t0, c0, r0, y0);
                getBridgeArchPoint(t1, c1, r1, y1);

                // Left Arch Rib segment
                m3d::Vec3 pL0 = c0 - r0 * archOffset; pL0.y = y0;
                m3d::Vec3 pL1 = c1 - r1 * archOffset; pL1.y = y1;
                m3d::Vec3 diffL = pL1 - pL0;
                float lenL = diffL.length();
                float rotZL = std::atan2(diffL.y, std::sqrt(diffL.x * diffL.x + diffL.z * diffL.z));
                float rotYL = std::atan2(-diffL.z, diffL.x);

                m3d::Mat4 mSegL = m3d::Mat4::translate((pL0 + pL1) * 0.5f) *
                                  m3d::Mat4::rotateY(rotYL) *
                                  m3d::Mat4::rotateZ(rotZL) *
                                  m3d::Mat4::scale(lenL * 1.05f, 0.42f, 0.42f);
                shader.setMat4("model", mSegL);
                scene.cubeMesh.draw();

                // Right Arch Rib segment
                m3d::Vec3 pR0 = c0 + r0 * archOffset; pR0.y = y0;
                m3d::Vec3 pR1 = c1 + r1 * archOffset; pR1.y = y1;
                m3d::Vec3 diffR = pR1 - pR0;
                float lenR = diffR.length();
                float rotZR = std::atan2(diffR.y, std::sqrt(diffR.x * diffR.x + diffR.z * diffR.z));
                float rotYR = std::atan2(-diffR.z, diffR.x);

                m3d::Mat4 mSegR = m3d::Mat4::translate((pR0 + pR1) * 0.5f) *
                                  m3d::Mat4::rotateY(rotYR) *
                                  m3d::Mat4::rotateZ(rotZR) *
                                  m3d::Mat4::scale(lenR * 1.05f, 0.42f, 0.42f);
                shader.setMat4("model", mSegR);
                scene.cubeMesh.draw();

                // Overhead crossbeams connecting the two arches near top (y >= 7.0m)
                if (s % 3 == 0 && t0 > 0.20f && t0 < 0.80f) {
                    m3d::Vec3 crossMid = (pL0 + pR0) * 0.5f;
                    m3d::Vec3 crossDiff = pR0 - pL0;
                    float crossLen = crossDiff.length();
                    float crossAngleY = std::atan2(-crossDiff.z, crossDiff.x);
                    m3d::Mat4 mCross = m3d::Mat4::translate(crossMid) *
                                       m3d::Mat4::rotateY(crossAngleY) *
                                       m3d::Mat4::scale(crossLen, 0.32f, 0.32f);
                    shader.setMat4("model", mCross);
                    scene.cubeMesh.draw();
                }

                // Vertical steel hanger cables connecting arch rib to outer edge of deck
                if (s % 2 == 1 && t0 > 0.08f && t0 < 0.92f) {
                    setMaterial(shader, m3d::Vec3(0.75f, 0.75f, 0.8f), 0.8f, 64.0f);
                    float hangerH = y0 - 3.20f;
                    m3d::Mat4 mHangerL = m3d::Mat4::translate(pL0.x, 3.20f + hangerH * 0.5f, pL0.z) *
                                         m3d::Mat4::scale(0.06f, hangerH, 0.06f);
                    shader.setMat4("model", mHangerL);
                    scene.cylinderMesh.draw();

                    m3d::Mat4 mHangerR = m3d::Mat4::translate(pR0.x, 3.20f + hangerH * 0.5f, pR0.z) *
                                         m3d::Mat4::scale(0.06f, hangerH, 0.06f);
                    shader.setMat4("model", mHangerR);
                    scene.cylinderMesh.draw();
                    setMaterial(shader, m3d::Vec3(0.88f, 0.16f, 0.14f), 0.7f, 48.0f);
                }
            }
        }
    }

    // -------------------------------------------------------------
    // 5. RIVER & BOATS (Animated River Currents, Boats Bobbing & Sailing)
    // -------------------------------------------------------------
    static void renderRiverAndBoats(const Shader& shader, SceneManager& scene) {
        // [A] River Water Surface with Shimmering Specular
        m3d::Vec3 waterColor = scene.isNight ? m3d::Vec3(0.04f, 0.12f, 0.22f) : m3d::Vec3(0.10f, 0.44f, 0.72f);
        setMaterial(shader, waterColor, 0.98f, 160.0f);
        {
            glDisable(GL_CULL_FACE);
            m3d::Mat4 mRiver = m3d::Mat4::identity();
            shader.setMat4("model", mRiver);
            scene.riverMesh.draw();
            glEnable(GL_CULL_FACE);
        }

        // Stone Quay Embankment Retaining Walls along riverbanks (holding back meadows from river chasm)
        setMaterial(shader, m3d::Vec3(0.38f, 0.35f, 0.30f), 0.15f, 12.0f);
        {
            // South riverbank quay wall (Z = -34.2m, X from -36.0 to +20.0m strictly under bridge)
            m3d::Mat4 mBankS = m3d::Mat4::translate(-8.0f, -1.05f, -34.2f) * 
                               m3d::Mat4::scale(56.8f, 2.3f, 1.2f);
            shader.setMat4("model", mBankS);
            scene.cubeMesh.draw();

            // North riverbank quay wall (Z = -57.8m, X from -36.0 to +20.0m strictly under bridge)
            m3d::Mat4 mBankN = m3d::Mat4::translate(-8.0f, -1.05f, -57.8f) * 
                               m3d::Mat4::scale(56.8f, 2.3f, 1.2f);
            shader.setMat4("model", mBankN);
            scene.cubeMesh.draw();

            // West terminal seawall (at X = -36.0m, enclosing river west of bridge; solid land to the west!)
            m3d::Mat4 mBankW = m3d::Mat4::translate(-36.0f, -1.05f, -46.0f) * 
                               m3d::Mat4::scale(1.4f, 2.3f, 24.8f);
            shader.setMat4("model", mBankW);
            scene.cubeMesh.draw();

            // East terminal seawall (at X = +20.0m, enclosing river east of bridge; solid land to the east!)
            m3d::Mat4 mBankE = m3d::Mat4::translate(20.0f, -1.05f, -46.0f) * 
                               m3d::Mat4::scale(1.4f, 2.3f, 24.8f);
            shader.setMat4("model", mBankE);
            scene.cubeMesh.draw();
        }

        // [B] Boats Sailing on the River under the Bridge (River level Y = -1.20m)
        for (const auto& boat : scene.boats) {
            // Harmonic wave physics (pitch, roll, bobbing)
            float waveBob = std::sin(scene.globalTime * 2.2f + boat.basePosition.x * 0.25f) * 0.08f;
            float waveRoll = std::cos(scene.globalTime * 1.9f + boat.basePosition.x * 0.20f) * 0.06f;
            float wavePitch = std::sin(scene.globalTime * 1.6f + boat.basePosition.x * 0.15f) * 0.04f;

            m3d::Vec3 pos = boat.basePosition + m3d::Vec3(0.0f, waveBob, 0.0f);

            m3d::Mat4 boatBase = m3d::Mat4::translate(pos) * 
                                 m3d::Mat4::rotateY(boat.heading) *
                                 m3d::Mat4::rotateZ(waveRoll) *
                                 m3d::Mat4::rotateX(wavePitch);

            // -------------------------------------------------------------
            // Boat Type 1: Luxury Cabin Cruiser (Max height Y = +0.50m; girder underside is Y = 2.525m)
            // -------------------------------------------------------------
            if (boat.type == BoatType::CABIN_CRUISER) {
                // Pointed Marine Hull (Navy Blue waterline)
                setMaterial(shader, m3d::Vec3(0.12f, 0.22f, 0.42f), 0.7f, 32.0f);
                m3d::Mat4 mLowerHull = boatBase * m3d::Mat4::translate(0.0f, 0.08f, 0.0f) * 
                                       m3d::Mat4::scale(boat.length, 0.55f, 2.2f);
                shader.setMat4("model", mLowerHull);
                scene.boatHullMesh.draw();

                // Upper Topsides (Pristine Marine White)
                setMaterial(shader, m3d::Vec3(0.96f, 0.96f, 0.98f), 0.8f, 64.0f);
                m3d::Mat4 mUpperHull = boatBase * m3d::Mat4::translate(0.0f, 0.38f, 0.0f) * 
                                       m3d::Mat4::scale(boat.length * 0.96f, 0.40f, 2.1f);
                shader.setMat4("model", mUpperHull);
                scene.boatHullMesh.draw();

                // Teak Cockpit Deck
                setMaterial(shader, m3d::Vec3(0.65f, 0.45f, 0.25f), 0.2f, 16.0f);
                m3d::Mat4 mCockpit = boatBase * m3d::Mat4::translate(1.0f, 0.78f, 0.0f) * 
                                     m3d::Mat4::scale(boat.length * 0.35f, 0.08f, 1.8f);
                shader.setMat4("model", mCockpit);
                scene.cubeMesh.draw();

                // Enclosed Cabin with Tinted Wrap-around Glass
                setMaterial(shader, m3d::Vec3(0.15f, 0.20f, 0.25f), 0.95f, 128.0f);
                m3d::Mat4 mCabin = boatBase * m3d::Mat4::translate(-0.4f, 1.15f, 0.0f) * 
                                   m3d::Mat4::scale(boat.length * 0.48f, 0.65f, 1.6f);
                shader.setMat4("model", mCabin);
                scene.cubeMesh.draw();

                // Flying Bridge / Upper Sun Deck
                setMaterial(shader, m3d::Vec3(0.94f, 0.94f, 0.96f), 0.7f, 64.0f);
                m3d::Mat4 mFlyBridge = boatBase * m3d::Mat4::translate(-0.8f, 1.55f, 0.0f) * 
                                       m3d::Mat4::scale(boat.length * 0.35f, 0.30f, 1.4f);
                shader.setMat4("model", mFlyBridge);
                scene.cubeMesh.draw();

                // Radar Mast & Spinning Scanner Dome (top at Y = 2.05m above water, world Y = +0.85m; well under girder Y = 2.525m!)
                setMaterial(shader, m3d::Vec3(0.85f, 0.85f, 0.88f), 0.5f, 32.0f);
                m3d::Mat4 mMast = boatBase * m3d::Mat4::translate(-1.4f, 1.85f, 0.0f) * 
                                  m3d::Mat4::scale(0.12f, 0.45f, 0.12f);
                shader.setMat4("model", mMast);
                scene.cylinderMesh.draw();

                float radarRot = scene.globalTime * 360.0f;
                m3d::Mat4 mRadar = boatBase * m3d::Mat4::translate(-1.4f, 2.10f, 0.0f) * 
                                   m3d::Mat4::rotateY(m3d::radians(radarRot)) * 
                                   m3d::Mat4::scale(0.65f, 0.10f, 0.16f);
                shader.setMat4("model", mRadar);
                scene.cubeMesh.draw();

                // Foaming Twin Wake Trails behind cruiser
                setMaterial(shader, m3d::Vec3(0.92f, 0.96f, 1.0f), 0.4f, 16.0f);
                m3d::Mat4 mWakeL = boatBase * m3d::Mat4::translate(-boat.length * 0.85f, 0.03f, -0.6f) * 
                                   m3d::Mat4::scale(boat.length * 0.9f, 0.04f, 0.5f);
                shader.setMat4("model", mWakeL);
                scene.cubeMesh.draw();

                m3d::Mat4 mWakeR = boatBase * m3d::Mat4::translate(-boat.length * 0.85f, 0.03f, 0.6f) * 
                                   m3d::Mat4::scale(boat.length * 0.9f, 0.04f, 0.5f);
                shader.setMat4("model", mWakeR);
                scene.cubeMesh.draw();
            }
            // -------------------------------------------------------------
            // Boat Type 2: Classic Sailboat (Low-profile River Rig; mast top at Y = +1.30m world space, 1.22m under girder!)
            // -------------------------------------------------------------
            else if (boat.type == BoatType::SAILBOAT) {
                // Sleek Pointed Hull (Crimson Red / White)
                setMaterial(shader, boat.hullColor, 0.7f, 64.0f);
                m3d::Mat4 mHull = boatBase * m3d::Mat4::translate(0.0f, 0.10f, 0.0f) * 
                                  m3d::Mat4::scale(boat.length, 0.55f, 1.8f);
                shader.setMat4("model", mHull);
                scene.boatHullMesh.draw();

                // Teak Deck Trim
                setMaterial(shader, m3d::Vec3(0.68f, 0.48f, 0.28f), 0.2f, 16.0f);
                m3d::Mat4 mDeck = boatBase * m3d::Mat4::translate(0.0f, 0.65f, 0.0f) * 
                                  m3d::Mat4::scale(boat.length * 0.90f, 0.06f, 1.60f);
                shader.setMat4("model", mDeck);
                scene.cubeMesh.draw();

                // Aluminum River Rig Mast (Mast top at Y = 2.50m above water, world Y = +1.30m)
                setMaterial(shader, m3d::Vec3(0.88f, 0.88f, 0.92f), 0.8f, 64.0f);
                m3d::Mat4 mMast = boatBase * m3d::Mat4::translate(0.38f, 1.45f, 0.0f) * 
                                  m3d::Mat4::scale(0.12f, 2.1f, 0.12f);
                shader.setMat4("model", mMast);
                scene.cylinderMesh.draw();

                // Horizontal Boom Spar
                m3d::Mat4 mBoom = boatBase * m3d::Mat4::translate(-0.8f, 0.70f, 0.0f) * 
                                  m3d::Mat4::scale(2.2f, 0.08f, 0.08f);
                shader.setMat4("model", mBoom);
                scene.cylinderMesh.draw();

                // Rigging Shrouds (Steel stay cables)
                setMaterial(shader, m3d::Vec3(0.75f, 0.75f, 0.80f), 0.5f, 32.0f);
                m3d::Mat4 mStayL = boatBase * m3d::Mat4::translate(0.38f, 1.35f, -0.65f) * 
                                   m3d::Mat4::rotateX(m3d::radians(18.0f)) *
                                   m3d::Mat4::scale(0.03f, 1.9f, 0.03f);
                shader.setMat4("model", mStayL);
                scene.cylinderMesh.draw();

                m3d::Mat4 mStayR = boatBase * m3d::Mat4::translate(0.38f, 1.35f, 0.65f) * 
                                   m3d::Mat4::rotateX(m3d::radians(-18.0f)) *
                                   m3d::Mat4::scale(0.03f, 1.9f, 0.03f);
                shader.setMat4("model", mStayR);
                scene.cylinderMesh.draw();

                // Billowing Triangular Mainsail and Crimson Jib (scaled to river clearance rig)
                glDisable(GL_CULL_FACE);
                setMaterial(shader, m3d::Vec3(0.98f, 0.98f, 0.96f), 0.35f, 24.0f, m3d::Vec3(0.15f, 0.15f, 0.15f));
                shader.setMat4("model", boatBase * m3d::Mat4::translate(0.0f, 0.65f, 0.0f));
                scene.sailboatSailMesh.draw();
                glEnable(GL_CULL_FACE);
            }
            // -------------------------------------------------------------
            // Boat Type 3: River Patrol / Tugboat
            // -------------------------------------------------------------
            else {
                // High-visibility Royal Blue / Red Workboat Hull
                setMaterial(shader, boat.hullColor, 0.6f, 32.0f);
                m3d::Mat4 mHull = boatBase * m3d::Mat4::translate(0.0f, 0.10f, 0.0f) * 
                                  m3d::Mat4::scale(boat.length, 0.70f, 1.9f);
                shader.setMat4("model", mHull);
                scene.boatHullMesh.draw();

                // Black Rubber Bumper Gunwale
                setMaterial(shader, m3d::Vec3(0.15f, 0.15f, 0.15f), 0.2f, 8.0f);
                m3d::Mat4 mBumper = boatBase * m3d::Mat4::translate(0.0f, 0.78f, 0.0f) * 
                                    m3d::Mat4::scale(boat.length * 1.03f, 0.12f, 1.98f);
                shader.setMat4("model", mBumper);
                scene.cubeMesh.draw();

                // White Pilot Wheelhouse Cabin
                setMaterial(shader, m3d::Vec3(0.94f, 0.94f, 0.96f), 0.7f, 32.0f);
                m3d::Mat4 mCabin = boatBase * m3d::Mat4::translate(-0.2f, 1.30f, 0.0f) * 
                                   m3d::Mat4::scale(boat.length * 0.45f, 0.85f, 1.4f);
                shader.setMat4("model", mCabin);
                scene.cubeMesh.draw();

                // Searchlight atop wheelhouse
                m3d::Vec3 searchGlow = scene.isNight ? m3d::Vec3(1.5f, 1.5f, 1.2f) : m3d::Vec3(0.2f, 0.2f, 0.2f);
                setMaterial(shader, m3d::Vec3(0.95f, 0.95f, 0.95f), 0.9f, 64.0f, searchGlow);
                m3d::Mat4 mLight = boatBase * m3d::Mat4::translate(0.3f, 1.85f, 0.0f) * 
                                   m3d::Mat4::scale(0.3f, 0.3f, 0.3f);
                shader.setMat4("model", mLight);
                scene.sphereMesh.draw();

                // Bow foaming wake
                setMaterial(shader, m3d::Vec3(0.92f, 0.96f, 1.0f), 0.4f, 16.0f);
                m3d::Mat4 mWake = boatBase * m3d::Mat4::translate(-boat.length * 0.75f, 0.03f, 0.0f) * 
                                  m3d::Mat4::scale(boat.length * 0.9f, 0.04f, 1.2f);
                shader.setMat4("model", mWake);
                scene.cubeMesh.draw();
            }
        }
    }

    // -------------------------------------------------------------
    // 6. MOUNTAIN & TUNNEL (Matching Reference Images 1 & 2)
    // - Image 1: Rustic stone masonry arch portal with keystones & autumn foliage
    // - Image 2: Curved cylindrical concrete vault with dual LED strips & overhead signals
    // -------------------------------------------------------------
    static void renderTunnelAndMountain(const Shader& shader, SceneManager& scene) {
        // [A] Mountain Hillside (Rocky terrain masses framing and enclosing tunnel)
        shader.setFloat("sunMultiplier", 1.0f);
        setMaterial(shader, m3d::Vec3(0.38f, 0.36f, 0.32f), 0.15f, 12.0f);
        {

            // Right Rocky Cliff Ridge (East of tunnel road, well outside curve at Z <= -2.0m, X >= -18.0m)
            m3d::Mat4 mRidgeR = m3d::Mat4::translate(-24.0f, 5.0f, -10.0f) * 
                                m3d::Mat4::scale(12.0f, 10.0f, 16.0f);
            shader.setMat4("model", mRidgeR);
            scene.cubeMesh.draw();

            // Mountain Peak / Mass atop tunnel vault (Bottom at Y = 5.75m, strictly above vault roof at Y = 5.55m!)
            setMaterial(shader, m3d::Vec3(0.32f, 0.34f, 0.28f), 0.1f, 8.0f);
            m3d::Mat4 mRidgeTop = m3d::Mat4::translate(-40.0f, 9.5f, -4.0f) * 
                                  m3d::Mat4::scale(22.0f, 7.5f, 28.0f);
            shader.setMat4("model", mRidgeTop);
            scene.cubeMesh.draw();

            // Forested earthen embankment slope rising directly behind the stone cornice
            setMaterial(shader, m3d::Vec3(0.24f, 0.28f, 0.20f), 0.1f, 8.0f);
            m3d::Mat4 mSlope = m3d::Mat4::translate(-45.0f, 10.2f, -17.0f) * 
                               m3d::Mat4::rotateX(m3d::radians(-20.0f)) *
                               m3d::Mat4::scale(24.0f, 2.5f, 7.0f);
            shader.setMat4("model", mSlope);
            scene.cubeMesh.draw();
        }

        // =============================================================
        // [B] TUNNEL ENTRANCE PORTAL (MATCHING REFERENCE IMAGE 1)
        // Multi-course rustic ashlar stone wall with radiating voussoir keystones,
        // overhanging autumn foliage, climbing ivy, and roadside trees!
        // =============================================================
        m3d::Vec3 entryPos(-45.0f, 0.0f, -20.5f);

        // Solid dark mortar backing walls (eliminates light leaks between rustic stone courses)
        setMaterial(shader, m3d::Vec3(0.18f, 0.17f, 0.16f), 0.1f, 8.0f);
        {
            // Left backing wall (stops cleanly at X = -51.5m, well east of return road)
            m3d::Mat4 mBackL = m3d::Mat4::translate(entryPos.x - 4.2f, 4.6f, entryPos.z + 0.15f) * 
                               m3d::Mat4::scale(4.5f, 9.2f, 2.0f);
            shader.setMat4("model", mBackL);
            scene.cubeMesh.draw();

            // Right backing wall
            m3d::Mat4 mBackR = m3d::Mat4::translate(entryPos.x + 7.5f, 4.6f, entryPos.z + 0.15f) * 
                               m3d::Mat4::scale(8.5f, 9.2f, 2.0f);
            shader.setMat4("model", mBackR);
            scene.cubeMesh.draw();

            // Spandrel backing wall
            m3d::Mat4 mBackS = m3d::Mat4::translate(entryPos.x, 7.0f, entryPos.z + 0.15f) * 
                               m3d::Mat4::scale(7.5f, 4.8f, 2.0f);
            shader.setMat4("model", mBackS);
            scene.cubeMesh.draw();
        }

        // 1. Natural Rustic Stone Palette for Courses (Warm limestone, granite, charcoal slate, weathered brown)
        m3d::Vec3 stoneColors[7] = {
            { 0.38f, 0.36f, 0.32f }, // Weathered grey limestone
            { 0.28f, 0.27f, 0.25f }, // Deep charcoal slate
            { 0.44f, 0.41f, 0.36f }, // Medium warm ashlar fieldstone
            { 0.24f, 0.23f, 0.22f }, // Dark charcoal granite
            { 0.42f, 0.38f, 0.33f }, // Sandstone block
            { 0.33f, 0.32f, 0.29f }, // Earthy shale
            { 0.48f, 0.45f, 0.39f }  // Pale limestone
        };

        // 2. Ashlar Masonry Courses: Left Abutment Wall (X from -51.5 to -47.4, Y from 0 to 9.2)
        // 8 horizontal courses of 3 staggered stone blocks with varied relief
        for (int row = 0; row < 8; ++row) {
            float y = 0.58f + (float)row * 1.15f;
            float h = 1.12f;
            float widths[3] = { 1.45f, 1.35f, 1.40f };

            // Stagger starting offset per row
            float curX = entryPos.x - 6.5f + ((row % 2 == 1) ? 0.35f : -0.25f);

            for (int col = 0; col < 3; ++col) {
                float w = widths[(col + row) % 3];
                float blockX = curX + w * 0.5f;
                curX += w;

                m3d::Vec3 colr = stoneColors[(row * 3 + col * 2 + 1) % 7];
                setMaterial(shader, colr, 0.2f, 16.0f);

                float relief = (((row * 4 + col * 7) % 5) - 2) * 0.025f;
                m3d::Mat4 mBlock = m3d::Mat4::translate(blockX, y, entryPos.z + relief) * 
                                   m3d::Mat4::scale(w * 0.98f, h, 2.4f);
                shader.setMat4("model", mBlock);
                scene.cubeMesh.draw();
            }
        }

        // 3. Ashlar Masonry Courses: Right Abutment Wall (X from -41.4 to -33.5, Y from 0 to 9.2)
        for (int row = 0; row < 8; ++row) {
            float y = 0.58f + (float)row * 1.15f;
            float h = 1.12f;
            float widths[5] = { 1.45f, 1.75f, 1.35f, 1.80f, 1.55f };

            float curX = entryPos.x + 3.6f + ((row % 2 == 1) ? -0.45f : 0.35f);

            for (int col = 0; col < 5; ++col) {
                float w = widths[(col + row + 2) % 5];
                float blockX = curX + w * 0.5f;
                curX += w;

                m3d::Vec3 colr = stoneColors[(row * 2 + col * 3 + 3) % 7];
                setMaterial(shader, colr, 0.2f, 16.0f);

                float relief = (((row * 5 + col * 3) % 5) - 2) * 0.025f;
                m3d::Mat4 mBlock = m3d::Mat4::translate(blockX, y, entryPos.z + relief) * 
                                   m3d::Mat4::scale(w * 0.98f, h, 2.4f);
                shader.setMat4("model", mBlock);
                scene.cubeMesh.draw();
            }
        }

        // 4. Spandrel Wall Above Arch (Y from 4.8 to 9.2, X from -48.6 to -41.4)
        for (int row = 0; row < 4; ++row) {
            float y = 5.2f + (float)row * 1.05f;
            float h = 1.02f;
            float widths[4] = { 1.8f, 1.9f, 1.7f, 1.8f };
            float curX = entryPos.x - 3.6f + ((row % 2 == 1) ? 0.3f : -0.3f);

            for (int col = 0; col < 4; ++col) {
                float w = widths[col];
                float blockX = curX + w * 0.5f;
                curX += w;

                m3d::Vec3 colr = stoneColors[(row * 2 + col * 4 + 2) % 7];
                setMaterial(shader, colr, 0.2f, 16.0f);

                float relief = (((row * 3 + col * 5) % 5) - 2) * 0.02f;
                m3d::Mat4 mBlock = m3d::Mat4::translate(blockX, y, entryPos.z + relief) * 
                                   m3d::Mat4::scale(w * 0.96f, h, 2.3f);
                shader.setMat4("model", mBlock);
                scene.cubeMesh.draw();
            }
        }

        // 5. Radiating Stone Arch Ring & Voussoirs (Reference Image 1: Distinct stone wedge ring)
        const int numVoussoirs = 22;
        float archRadius = 4.25f; // Center radius of stone arch ring
        float ringThick = 1.15f;  // Radial thickness of arch stones

        for (int v = 0; v < numVoussoirs; ++v) {
            float angle = (float)v / (float)(numVoussoirs - 1) * m3d::PI;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float vx = entryPos.x + archRadius * cosA;
            float vy = entryPos.y + archRadius * sinA;
            float vz = entryPos.z - 0.16f; // Protrudes forward from wall face (relief!)

            // Subtle color variation across voussoirs
            m3d::Vec3 vCol = stoneColors[(v + 2) % 7] * (0.92f + 0.12f * std::sin((float)v));
            setMaterial(shader, vCol, 0.25f, 24.0f);

            float rotZ = angle - m3d::PI * 0.5f;
            m3d::Mat4 mVoussoir = m3d::Mat4::translate(vx, vy, vz) * 
                                  m3d::Mat4::rotateZ(rotZ) * 
                                  m3d::Mat4::scale(0.65f, ringThick, 0.45f);
            shader.setMat4("model", mVoussoir);
            scene.cubeMesh.draw();
        }

        // 6. Prominent Crown Keystone at Arch Apex (Reference Image 1: Protruding beveled keystone)
        {
            setMaterial(shader, m3d::Vec3(0.62f, 0.60f, 0.55f), 0.35f, 32.0f); // Bright limestone keystone
            m3d::Mat4 mKeystone = m3d::Mat4::translate(entryPos.x, entryPos.y + archRadius + 0.20f, entryPos.z - 0.28f) * 
                                  m3d::Mat4::scale(1.05f, 1.45f, 0.60f);
            shader.setMat4("model", mKeystone);
            scene.cubeMesh.draw();
        }

        // 7. Heavy Overhanging Parapet Cornice Coping (Reference Image 1: Continuous stone ledge)
        setMaterial(shader, m3d::Vec3(0.44f, 0.42f, 0.39f), 0.2f, 16.0f);
        {
            m3d::Mat4 mCornice = m3d::Mat4::translate(entryPos.x, 9.40f, entryPos.z - 0.32f) * 
                                 m3d::Mat4::scale(23.5f, 0.60f, 2.8f);
            shader.setMat4("model", mCornice);
            scene.cubeMesh.draw();
        }

        // 8. Lush Autumn Foliage Overhanging the Stone Entrance Portal (Reference Image 1)
        m3d::Vec3 autumnPalette[5] = {
            { 0.88f, 0.18f, 0.12f }, // Autumn crimson red
            { 0.94f, 0.52f, 0.10f }, // Glowing fiery amber
            { 0.92f, 0.72f, 0.15f }, // Golden yellow
            { 0.72f, 0.28f, 0.12f }, // Russet bronze
            { 0.22f, 0.46f, 0.18f }  // Deep forest green
        };

        // Dense carpet of autumn foliage cascading over the cornice & hanging down (Image 1)
        for (int f = 0; f < 34; ++f) {
            float fx = entryPos.x - 11.0f + (float)f * 0.66f;
            float fy = 9.65f + 0.50f * std::sin((float)f * 1.3f);
            float fz = entryPos.z - 0.70f + 0.35f * std::cos((float)f * 1.6f);
            m3d::Vec3 folCol = autumnPalette[f % 5];

            setMaterial(shader, folCol, 0.15f, 8.0f);
            m3d::Mat4 mFoliage = m3d::Mat4::translate(fx, fy, fz) * 
                                 m3d::Mat4::scale(1.6f, 1.3f, 1.4f);
            shader.setMat4("model", mFoliage);
            scene.sphereMesh.draw();

            // Hanging leaves draping down past cornice edge onto the stone face
            if (f % 2 == 0) {
                float hy = 8.85f + 0.35f * std::sin(f * 2.1f);
                float hz = entryPos.z - 0.58f;
                m3d::Mat4 mHang = m3d::Mat4::translate(fx, hy, hz) *
                                  m3d::Mat4::scale(0.85f, 0.95f, 0.8f);
                shader.setMat4("model", mHang);
                scene.sphereMesh.draw();
            }
        }

        // Ivy vines cascading down the corners of the stone wall (Image 1)
        for (int v = 0; v < 9; ++v) {
            float vy = 8.7f - (float)v * 0.75f;
            // Left vine
            m3d::Vec3 colL = autumnPalette[(v * 2) % 5];
            setMaterial(shader, colL, 0.15f, 8.0f);
            m3d::Mat4 mVineL = m3d::Mat4::translate(entryPos.x - 9.8f + 0.3f * std::sin(v * 1.8f), vy, entryPos.z - 0.4f) * 
                               m3d::Mat4::scale(0.95f, 0.85f, 0.7f);
            shader.setMat4("model", mVineL);
            scene.sphereMesh.draw();

            // Right vine
            m3d::Vec3 colR = autumnPalette[(v * 2 + 1) % 5];
            setMaterial(shader, colR, 0.15f, 8.0f);
            m3d::Mat4 mVineR = m3d::Mat4::translate(entryPos.x + 9.8f - 0.3f * std::sin(v * 1.8f), vy, entryPos.z - 0.4f) * 
                               m3d::Mat4::scale(0.95f, 0.85f, 0.7f);
            shader.setMat4("model", mVineR);
            scene.sphereMesh.draw();
        }

        // 9. Autumn Embankment & Flanking Deciduous Trees (Reference Image 1: Rich canopy towering over portal)
        struct AutumnTreePos {
            float xOffset;
            float yBase;
            float zOffset;
            float trunkH;
            float canopyR;
            m3d::Vec3 color;
        };

        AutumnTreePos autumnTrees[8] = {
            // Embankment Trees directly atop stone cornice (Image 1: dense foliage crowning the arch)
            { -6.5f,  9.6f, -0.6f, 3.4f, 4.4f, {0.92f, 0.20f, 0.12f} }, // Red maple atop left parapet
            { -2.5f,  9.8f,  0.5f, 4.0f, 4.6f, {0.96f, 0.56f, 0.10f} }, // Fiery amber oak atop mid-left
            {  2.5f,  9.8f,  0.5f, 3.8f, 4.5f, {0.94f, 0.76f, 0.12f} }, // Golden birch atop mid-right
            {  6.5f,  9.6f, -0.6f, 3.4f, 4.2f, {0.86f, 0.18f, 0.14f} }, // Crimson oak atop right parapet
            // Hillside trees framing portal from behind and on east flank (completely clear of return road!)
            {   4.5f, 10.2f,  1.5f, 4.2f, 4.8f, {0.94f, 0.48f, 0.10f} }, // Fiery maple atop mountain
            {  11.5f,  0.0f, -5.5f, 6.2f, 5.0f, {0.92f, 0.72f, 0.14f} }, // Golden birch (East flank)
            {   0.0f, 10.4f,  2.2f, 4.8f, 5.5f, {0.90f, 0.30f, 0.12f} }, // Tall autumn crown peak
            {   5.5f,  0.0f, -12.0f, 5.2f, 4.5f, {0.95f, 0.62f, 0.10f} }  // Roadside amber tree (East verge)
        };

        for (int t = 0; t < 8; ++t) {
            float tx = entryPos.x + autumnTrees[t].xOffset;
            float ty = autumnTrees[t].yBase;
            float tz = entryPos.z + autumnTrees[t].zOffset;

            // Tree trunk
            setMaterial(shader, m3d::Vec3(0.32f, 0.20f, 0.12f), 0.1f, 8.0f);
            m3d::Mat4 mTrunk = m3d::Mat4::translate(tx, ty + autumnTrees[t].trunkH * 0.5f, tz) * 
                               m3d::Mat4::scale(0.55f, autumnTrees[t].trunkH, 0.55f);
            shader.setMat4("model", mTrunk);
            scene.cylinderMesh.draw();

            // Multi-spherical foliage canopy
            setMaterial(shader, autumnTrees[t].color, 0.15f, 8.0f);
            m3d::Mat4 mCanopy1 = m3d::Mat4::translate(tx, ty + autumnTrees[t].trunkH + 1.2f, tz) * 
                                 m3d::Mat4::scale(autumnTrees[t].canopyR, autumnTrees[t].canopyR * 0.9f, autumnTrees[t].canopyR);
            shader.setMat4("model", mCanopy1);
            scene.sphereMesh.draw();

            // Upper crown sphere
            m3d::Vec3 topCol = autumnTrees[t].color * 1.1f;
            setMaterial(shader, topCol, 0.15f, 8.0f);
            m3d::Mat4 mCanopy2 = m3d::Mat4::translate(tx, ty + autumnTrees[t].trunkH + 2.6f, tz) * 
                                 m3d::Mat4::scale(autumnTrees[t].canopyR * 0.75f, autumnTrees[t].canopyR * 0.7f, autumnTrees[t].canopyR * 0.75f);
            shader.setMat4("model", mCanopy2);
            scene.sphereMesh.draw();
        }

        // 10. Fallen Autumn Leaves on the Right Roadside Verge (Reference Image 1: Crisp carpet of leaves on grass)
        for (int l = 0; l < 24; ++l) {
            float lz = entryPos.z - 2.0f - (float)l * 0.75f;
            float lx = entryPos.x + 3.2f + 0.8f * std::sin(l * 1.7f);
            m3d::Vec3 leafCol = autumnPalette[(l * 3) % 5];
            setMaterial(shader, leafCol, 0.1f, 4.0f);

            m3d::Mat4 mLeaf = m3d::Mat4::translate(lx, 0.05f, lz) * 
                              m3d::Mat4::scale(0.35f, 0.06f, 0.45f);
            shader.setMat4("model", mLeaf);
            scene.sphereMesh.draw();
        }

        // 11. Angled Stone Wingwalls Flanking Entrance Road (Image 1)
        setMaterial(shader, m3d::Vec3(0.44f, 0.41f, 0.38f), 0.15f, 12.0f);
        {
            // Left wingwall stops cleanly at X = -50.5m, safely clear of return road
            m3d::Mat4 mWingL = m3d::Mat4::translate(entryPos.x - 5.5f, 1.8f, entryPos.z - 3.2f) * 
                               m3d::Mat4::rotateY(m3d::radians(-20.0f)) * 
                               m3d::Mat4::scale(1.0f, 3.6f, 6.0f);
            shader.setMat4("model", mWingL);
            scene.cubeMesh.draw();

            m3d::Mat4 mWingR = m3d::Mat4::translate(entryPos.x + 8.2f, 1.8f, entryPos.z - 3.2f) * 
                               m3d::Mat4::rotateY(m3d::radians(24.0f)) * 
                               m3d::Mat4::scale(1.2f, 3.6f, 6.5f);
            shader.setMat4("model", mWingR);
            scene.cubeMesh.draw();
        }

        // =============================================================
        // [C] INSIDE THE TUNNEL (MATCHING REFERENCE IMAGE 2)
        // Curved concrete vault, continuous dual LED strips, overhead signals, barriers
        // Subterranean lighting isolation: zero outdoor sunlight inside!
        // =============================================================
        {
            m3d::Vec3 normalAmbient = scene.isNight ? m3d::Vec3(0.08f, 0.09f, 0.12f) : m3d::Vec3(0.42f, 0.44f, 0.48f);
            shader.setFloat("sunMultiplier", 0.0f);
            shader.setVec3("ambientGlobal", m3d::Vec3(0.08f, 0.09f, 0.10f));

            // 1. Procedural Curved Tunnel Vault Mesh (curves along track with sealed roadway floor!)
            glDisable(GL_CULL_FACE);
            setMaterial(shader, m3d::Vec3(1.0f, 1.0f, 1.0f), 0.25f, 24.0f);
            shader.setMat4("model", m3d::Mat4::identity());
            scene.tunnelVaultMesh.draw();

            // 2. Dual Continuous LED Overhead Light Strips (Image 2)
            m3d::Vec3 stripEmissive(2.8f, 2.8f, 3.0f);
            setMaterial(shader, m3d::Vec3(1.0f, 1.0f, 1.0f), 0.95f, 64.0f, stripEmissive);
            shader.setMat4("model", m3d::Mat4::identity());
            scene.tunnelCeilingLightMesh.draw();

            // 3. Side Barrier Curbs (New Jersey barriers)
            setMaterial(shader, m3d::Vec3(0.36f, 0.38f, 0.40f), 0.2f, 16.0f);
            shader.setMat4("model", m3d::Mat4::identity());
            scene.tunnelBarriersMesh.draw();
            glEnable(GL_CULL_FACE);

            // [D] Overhead Suspended Electronic Lane Signals (Image 2)
            // Green illuminated down arrows [ ↓ ] hanging above each lane!
            m3d::Vec3 signalGantryPositions[2] = {
                { -45.0f, 3.8f, -9.0f },
                { -39.0f, 3.8f,  3.0f }
            };
            float gantryRots[2] = { 0.0f, -22.0f };

            for (int g = 0; g < 2; ++g) {
                // Dark mounting crossbar
                setMaterial(shader, m3d::Vec3(0.12f, 0.12f, 0.14f), 0.4f, 32.0f);
                m3d::Mat4 mBar = m3d::Mat4::translate(signalGantryPositions[g]) * 
                                 m3d::Mat4::rotateY(m3d::radians(gantryRots[g])) *
                                 m3d::Mat4::scale(4.8f, 0.15f, 0.25f);
                shader.setMat4("model", mBar);
                scene.cubeMesh.draw();

                // Vertical hanger struts
                m3d::Mat4 mStrutL = m3d::Mat4::translate(signalGantryPositions[g]) * 
                                    m3d::Mat4::rotateY(m3d::radians(gantryRots[g])) *
                                    m3d::Mat4::translate(-1.8f, 0.45f, 0.0f) * 
                                    m3d::Mat4::scale(0.08f, 0.8f, 0.08f);
                shader.setMat4("model", mStrutL);
                scene.cubeMesh.draw();

                m3d::Mat4 mStrutR = m3d::Mat4::translate(signalGantryPositions[g]) * 
                                    m3d::Mat4::rotateY(m3d::radians(gantryRots[g])) *
                                    m3d::Mat4::translate(1.8f, 0.45f, 0.0f) * 
                                    m3d::Mat4::scale(0.08f, 0.8f, 0.08f);
                shader.setMat4("model", mStrutR);
                scene.cubeMesh.draw();

                // Green illuminated lane indicator panels (Left & Right lane: [ ↓ ])
                m3d::Vec3 greenGlow(0.1f, 2.4f, 0.4f);
                setMaterial(shader, m3d::Vec3(0.2f, 0.95f, 0.3f), 0.8f, 64.0f, greenGlow);

                m3d::Mat4 mArrowL = m3d::Mat4::translate(signalGantryPositions[g]) * 
                                    m3d::Mat4::rotateY(m3d::radians(gantryRots[g])) *
                                    m3d::Mat4::translate(-1.2f, -0.4f, 0.0f) * 
                                    m3d::Mat4::scale(0.7f, 0.55f, 0.12f);
                shader.setMat4("model", mArrowL);
                scene.cubeMesh.draw();

                m3d::Mat4 mArrowR = m3d::Mat4::translate(signalGantryPositions[g]) * 
                                    m3d::Mat4::rotateY(m3d::radians(gantryRots[g])) *
                                    m3d::Mat4::translate(1.2f, -0.4f, 0.0f) * 
                                    m3d::Mat4::scale(0.7f, 0.55f, 0.12f);
                shader.setMat4("model", mArrowR);
                scene.cubeMesh.draw();

                // Digital Speed Limit Circle Sign [ 80 ]
                setMaterial(shader, m3d::Vec3(0.9f, 0.1f, 0.1f), 0.6f, 32.0f, m3d::Vec3(1.8f, 0.1f, 0.1f));
                m3d::Mat4 mSpeedSign = m3d::Mat4::translate(signalGantryPositions[g]) * 
                                       m3d::Mat4::rotateY(m3d::radians(gantryRots[g])) *
                                       m3d::Mat4::translate(0.0f, -0.4f, 0.0f) * 
                                       m3d::Mat4::scale(0.65f, 0.65f, 0.08f);
                shader.setMat4("model", mSpeedSign);
                scene.cylinderMesh.draw();
            }

            // [E] Orange reflective safety markers along wall base (Image 2)
            m3d::Vec3 reflectorGlow(2.2f, 1.2f, 0.2f);
            setMaterial(shader, m3d::Vec3(1.0f, 0.60f, 0.1f), 0.8f, 64.0f, reflectorGlow);

            for (float rz = -18.0f; rz <= 6.0f; rz += 3.5f) {
                float frac = (rz + 18.0f) / 24.0f;
                float rx = (rz < -8.0f) ? -45.0f : (-45.0f + 15.0f * (frac - 0.4f) / 0.6f);

                m3d::Mat4 mRefL = m3d::Mat4::translate(rx - 2.85f, 0.45f, rz) * 
                                  m3d::Mat4::scale(0.12f, 0.38f, 0.12f);
                shader.setMat4("model", mRefL);
                scene.cubeMesh.draw();

                m3d::Mat4 mRefR = m3d::Mat4::translate(rx + 2.85f, 0.45f, rz) * 
                                  m3d::Mat4::scale(0.12f, 0.38f, 0.12f);
                shader.setMat4("model", mRefR);
                scene.cubeMesh.draw();
            }

            // Grand Open Exit Portal at countryside emergence (X = -20.0, Z = 10.0; clear width 7.2m, height 4.8m)
            m3d::Vec3 exitPos(-20.0f, 0.0f, 10.0f);
            setMaterial(shader, m3d::Vec3(0.44f, 0.42f, 0.39f), 0.2f, 16.0f);
            {
                // Left stone abutment (Z = 5.5m, clear of road)
                m3d::Mat4 mPylonL = m3d::Mat4::translate(exitPos.x, 2.5f, exitPos.z - 4.5f) * 
                                    m3d::Mat4::scale(2.2f, 5.0f, 1.8f);
                shader.setMat4("model", mPylonL);
                scene.cubeMesh.draw();

                // Right stone abutment (Z = 14.5m, clear of road)
                m3d::Mat4 mPylonR = m3d::Mat4::translate(exitPos.x, 2.5f, exitPos.z + 4.5f) * 
                                    m3d::Mat4::scale(2.2f, 5.0f, 1.8f);
                shader.setMat4("model", mPylonR);
                scene.cubeMesh.draw();

                // Overhead stone lintel arch beam (Y = 5.4m, clear height = 4.8m)
                m3d::Mat4 mLintel = m3d::Mat4::translate(exitPos.x, 5.4f, exitPos.z) * 
                                    m3d::Mat4::scale(2.4f, 1.2f, 10.8f);
                shader.setMat4("model", mLintel);
                scene.cubeMesh.draw();

                // Cornice coping
                setMaterial(shader, m3d::Vec3(0.52f, 0.50f, 0.46f), 0.25f, 24.0f);
                m3d::Mat4 mCornice = m3d::Mat4::translate(exitPos.x, 6.2f, exitPos.z) * 
                                     m3d::Mat4::scale(2.8f, 0.4f, 11.4f);
                shader.setMat4("model", mCornice);
                scene.cubeMesh.draw();

                // Mountain earth mass sloping up over the portal exit
                setMaterial(shader, m3d::Vec3(0.32f, 0.34f, 0.28f), 0.1f, 8.0f);
                m3d::Mat4 mHillExit = m3d::Mat4::translate(exitPos.x - 3.5f, 7.2f, exitPos.z) * 
                                      m3d::Mat4::scale(9.0f, 4.5f, 15.0f);
                shader.setMat4("model", mHillExit);
                scene.cubeMesh.draw();
            }

            // Restore daylight sun and ambient
            shader.setFloat("sunMultiplier", 1.0f);
            shader.setVec3("ambientGlobal", normalAmbient);
        }
    }

    // -------------------------------------------------------------
    // 7. ROADSIDE TREES (Dynamic Wind Shearing Demonstration!)
    // -------------------------------------------------------------
    static void renderTrees(const Shader& shader, SceneManager& scene) {
        for (const auto& tree : scene.trees) {
            // Dynamic wind shear transformation
            // Shear matrix shifts upper branches in X and Z based on height Y!
            float windFreq = 2.2f;
            float shearAmpX = scene.enableTreeShear ? 0.08f * std::sin(scene.globalTime * windFreq + tree.phase) : 0.0f;
            float shearAmpZ = scene.enableTreeShear ? 0.04f * std::cos(scene.globalTime * (windFreq * 0.8f) + tree.phase) : 0.0f;

            m3d::Mat4 treeBase = m3d::Mat4::translate(tree.position) * 
                                 m3d::Mat4::scale(tree.scale);

            // [A] Tree Trunk (Brown bark cylinder)
            setMaterial(shader, m3d::Vec3(0.32f, 0.20f, 0.12f), 0.1f, 8.0f);
            m3d::Mat4 mTrunk = treeBase * 
                               m3d::Mat4::translate(0.0f, 1.2f, 0.0f) * 
                               m3d::Mat4::scale(0.5f, 2.4f, 0.5f);
            shader.setMat4("model", mTrunk);
            scene.cylinderMesh.draw();

            // [B] Foliage Canopies (Subject to Wind Shear!)
            // We apply Mat4::shearX(shearAmpX, shearAmpZ) to simulate wind sway!
            m3d::Mat4 shearedFoliage = treeBase * 
                                      m3d::Mat4::translate(0.0f, 2.2f, 0.0f) * 
                                      m3d::Mat4::shearX(shearAmpX, shearAmpZ);

            setMaterial(shader, tree.foliageColor, 0.25f, 16.0f);

            // Bottom canopy cone
            m3d::Mat4 mCone1 = shearedFoliage * m3d::Mat4::translate(0.0f, 0.8f, 0.0f) * m3d::Mat4::scale(3.2f, 2.4f, 3.2f);
            shader.setMat4("model", mCone1);
            scene.coneMesh.draw();

            // Middle canopy cone
            m3d::Mat4 mCone2 = shearedFoliage * m3d::Mat4::translate(0.0f, 2.0f, 0.0f) * m3d::Mat4::scale(2.5f, 2.2f, 2.5f);
            shader.setMat4("model", mCone2);
            scene.coneMesh.draw();

            // Top canopy cone
            m3d::Mat4 mCone3 = shearedFoliage * m3d::Mat4::translate(0.0f, 3.1f, 0.0f) * m3d::Mat4::scale(1.8f, 1.8f, 1.8f);
            shader.setMat4("model", mCone3);
            scene.coneMesh.draw();
        }
    }

    // -------------------------------------------------------------
    // 8. STREET LAMPS & ROADSIDE PROPS
    // -------------------------------------------------------------
    static void renderStreetLamps(const Shader& shader, SceneManager& scene) {
        for (const auto& lamp : scene.streetLamps) {
            m3d::Mat4 lampBase = m3d::Mat4::translate(lamp.position) * 
                                 m3d::Mat4::rotateY(lamp.rotationY);

            // Vertical metallic post
            setMaterial(shader, m3d::Vec3(0.22f, 0.24f, 0.26f), 0.6f, 32.0f);
            m3d::Mat4 mPost = lampBase * m3d::Mat4::translate(0.0f, 2.6f, 0.0f) * 
                              m3d::Mat4::scale(0.18f, 5.2f, 0.18f);
            shader.setMat4("model", mPost);
            scene.cylinderMesh.draw();

            // Overhanging horizontal curved arm
            m3d::Mat4 mArm = lampBase * m3d::Mat4::translate(0.55f, 5.1f, 0.0f) * 
                             m3d::Mat4::scale(1.2f, 0.12f, 0.12f);
            shader.setMat4("model", mArm);
            scene.cubeMesh.draw();

            // Lamp Lantern Head Fixture
            m3d::Vec3 glowColor = scene.isNight ? m3d::Vec3(1.2f, 0.95f, 0.6f) : m3d::Vec3(0.2f, 0.2f, 0.2f);
            setMaterial(shader, m3d::Vec3(0.95f, 0.9f, 0.7f), 0.8f, 64.0f, glowColor);

            m3d::Mat4 mHead = lampBase * m3d::Mat4::translate(1.1f, 4.95f, 0.0f) * 
                              m3d::Mat4::scale(0.35f, 0.25f, 0.35f);
            shader.setMat4("model", mHead);
            scene.sphereMesh.draw();
        }
    }

    // -------------------------------------------------------------
    // 9. TERRAIN / GROUND PLANE (Carved River Valley between North & South Meadows)
    // -------------------------------------------------------------
    static void renderTerrain(const Shader& shader, SceneManager& scene) {
        // Lush green landscape meadow
        m3d::Vec3 grassCol = scene.isNight ? m3d::Vec3(0.06f, 0.12f, 0.08f) : m3d::Vec3(0.22f, 0.42f, 0.20f);
        setMaterial(shader, grassCol, 0.1f, 4.0f);
        {
            // South Meadow (City, Garage, Road, Tunnel): Z from -34.2m to +85.0m
            m3d::Mat4 mSouth = m3d::Mat4::translate(0.0f, -0.05f, 25.4f) * 
                               m3d::Mat4::scale(220.0f, 0.1f, 119.2f);
            shader.setMat4("model", mSouth);
            scene.cubeMesh.draw();

            // North Meadow (Countryside highway return): Z from -57.8m to -130.0m
            m3d::Mat4 mNorth = m3d::Mat4::translate(0.0f, -0.05f, -93.9f) * 
                               m3d::Mat4::scale(220.0f, 0.1f, 72.2f);
            shader.setMat4("model", mNorth);
            scene.cubeMesh.draw();

            // East solid land mass (from X = +20.0m to +120.0m; solid continuous green ground for countryside highway!)
            m3d::Mat4 mEast = m3d::Mat4::translate(70.0f, -0.05f, -46.0f) * 
                              m3d::Mat4::scale(100.0f, 0.1f, 23.6f);
            shader.setMat4("model", mEast);
            scene.cubeMesh.draw();

            // West solid land mass (from X = -36.0m to -120.0m; solid dry ground for countryside return road!)
            m3d::Mat4 mWest = m3d::Mat4::translate(-78.0f, -0.05f, -46.0f) * 
                              m3d::Mat4::scale(84.0f, 0.1f, 23.6f);
            shader.setMat4("model", mWest);
            scene.cubeMesh.draw();
        }

        // Riverbed beneath flowing river water: deep river silt & gravel at Y = -2.15m (strictly under bridge X from -36.0m to +20.0m)
        setMaterial(shader, m3d::Vec3(0.18f, 0.16f, 0.14f), 0.05f, 4.0f);
        {
            m3d::Mat4 mBed = m3d::Mat4::translate(-8.0f, -2.15f, -46.0f) * 
                             m3d::Mat4::scale(56.8f, 0.2f, 23.6f);
            shader.setMat4("model", mBed);
            scene.cubeMesh.draw();
        }
    }
};

#endif // MODELS_HPP
