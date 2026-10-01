#pragma once
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "math3d.hpp"

enum class CameraMode {
    CHASE = 1,        // Dynamic 3rd person follow behind car
    FIRST_PERSON = 2, // Cockpit / Hood driver view
    BIRDS_EYE = 3,    // High altitude overhead view
    ORBIT_FREE = 4,   // Free fly / orbit camera
    SCENIC_SIDE = 5   // Cinematic roadside camera tracking the car
};

class Camera {
public:
    CameraMode mode{CameraMode::CHASE};

    m3d::Vec3 position{0.0f, 10.0f, 20.0f};
    m3d::Vec3 forward{0.0f, 0.0f, -1.0f};
    m3d::Vec3 up{0.0f, 1.0f, 0.0f};
    m3d::Vec3 right{1.0f, 0.0f, 0.0f};

    // Free camera orientation (Euler angles in degrees)
    float yaw{-90.0f};
    float pitch{0.0f};
    float moveSpeed{25.0f};
    float mouseSensitivity{0.15f};

    // Smooth chase camera state
    m3d::Vec3 chasePosCurrent{0.0f, 5.0f, 10.0f};
    m3d::Vec3 chaseLookCurrent{0.0f, 1.0f, 0.0f};

    // Scenic viewpoints tailored to key architectural areas
    int currentScenicSpot{0};
    m3d::Vec3 scenicPositions[5] = {
        m3d::Vec3(-43.8f, 2.7f, -39.5f), // 1. Tunnel Entrance Portal (Reference Image 1: Stone masonry arch, autumn foliage, double yellow lines)
        m3d::Vec3(-44.6f, 1.35f, -13.0f), // 2. Inside Tunnel Vault (Reference Image 2: Curved vault, dual LED strips & green lane arrows)
        m3d::Vec3( -2.0f, 4.2f, -22.0f), // 3. Grand River Bridge Overlook (Steel arches, deck, river & boats)
        m3d::Vec3( -5.0f, 0.4f, -34.0f), // 4. River Level looking under Bridge (Water & Boats)
        m3d::Vec3(-18.0f, 8.5f,  46.0f)  // 5. City Skyline & Sheared Skyscraper
    };

    Camera() {
        updateVectors();
    }

    void setMode(CameraMode newMode) {
        mode = newMode;
    }

    void snapToCar(const m3d::Vec3& carPos, const m3d::Vec3& carForward, const m3d::Vec3& carUp) {
        float distanceBehind = 6.0f;
        float heightAbove = 2.4f;
        chasePosCurrent = carPos - carForward * distanceBehind + carUp * heightAbove;
        chaseLookCurrent = carPos + carForward * 3.0f + carUp * 0.8f;
        position = chasePosCurrent;
        forward = (chaseLookCurrent - position).normalized();
        right = m3d::cross(forward, m3d::Vec3(0, 1, 0)).normalized();
        up = m3d::cross(right, forward).normalized();
    }

    void nextScenicSpot() {
        currentScenicSpot = (currentScenicSpot + 1) % 5;
    }

    void update(float dt, const m3d::Vec3& carPos, const m3d::Vec3& carForward, const m3d::Vec3& carUp) {
        switch (mode) {
            case CameraMode::CHASE: {
                // Desired camera position: behind and slightly above the car
                float distanceBehind = 6.0f;
                float heightAbove = 2.4f;
                m3d::Vec3 desiredPos = carPos - carForward * distanceBehind + carUp * heightAbove;
                m3d::Vec3 desiredLook = carPos + carForward * 3.0f + carUp * 0.8f;

                // Smooth dampening
                float t = m3d::clamp(dt * 7.0f, 0.0f, 1.0f);
                chasePosCurrent = m3d::lerp(chasePosCurrent, desiredPos, t);
                chaseLookCurrent = m3d::lerp(chaseLookCurrent, desiredLook, t);

                position = chasePosCurrent;
                forward = (chaseLookCurrent - position).normalized();
                right = m3d::cross(forward, m3d::Vec3(0, 1, 0)).normalized();
                up = m3d::cross(right, forward).normalized();
                break;
            }

            case CameraMode::FIRST_PERSON: {
                // Driver cockpit / hood view
                position = carPos + carForward * 0.4f + carUp * 0.95f;
                forward = carForward;
                right = m3d::cross(forward, carUp).normalized();
                up = carUp;
                break;
            }

            case CameraMode::BIRDS_EYE: {
                // Overhead drone camera focused on the active car, looking down
                position = carPos + m3d::Vec3(0.0f, 55.0f, 1.0f);
                forward = (carPos - position).normalized();
                right = m3d::Vec3(1.0f, 0.0f, 0.0f);
                up = m3d::cross(right, forward).normalized();
                break;
            }

            case CameraMode::SCENIC_SIDE: {
                // Fixed roadside camera tracking the car or architectural vistas
                position = scenicPositions[currentScenicSpot];
                m3d::Vec3 targetLook = carPos + m3d::Vec3(0.0f, 0.8f, 0.0f);
                if (currentScenicSpot == 0) {
                    // Focused on Rustic Stone Tunnel Entrance Portal (matching Reference Image 1)
                    targetLook = m3d::Vec3(-45.0f, 3.4f, -20.5f);
                } else if (currentScenicSpot == 1) {
                    // Deep perspective looking down the curved glowing tunnel vault (matching Reference Image 2)
                    targetLook = m3d::Vec3(-43.6f, 1.45f, 1.0f);
                } else if (currentScenicSpot == 2) {
                    // Panoramic overlook framing crimson arches, river, boats, and car
                    targetLook = m3d::Vec3(-9.0f, 1.1f, -42.0f);
                }
                forward = (targetLook - position).normalized();
                right = m3d::cross(forward, m3d::Vec3(0, 1, 0)).normalized();
                up = m3d::cross(right, forward).normalized();
                break;
            }

            case CameraMode::ORBIT_FREE: {
                // Free camera handled via input
                break;
            }
        }
    }

    m3d::Mat4 getViewMatrix() const {
        return m3d::Mat4::lookAt(position, position + forward, up);
    }

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        if (mode != CameraMode::ORBIT_FREE) return;

        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch) {
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }

        updateVectors();
    }

    void processKeyboard(int direction, float dt) {
        if (mode != CameraMode::ORBIT_FREE) return;

        float velocity = moveSpeed * dt;
        // 0: Forward, 1: Backward, 2: Left, 3: Right, 4: Up, 5: Down
        if (direction == 0) position += forward * velocity;
        if (direction == 1) position -= forward * velocity;
        if (direction == 2) position -= right * velocity;
        if (direction == 3) position += right * velocity;
        if (direction == 4) position += up * velocity;
        if (direction == 5) position -= up * velocity;
    }

private:
    void updateVectors() {
        m3d::Vec3 front;
        float radYaw = m3d::radians(yaw);
        float radPitch = m3d::radians(pitch);

        front.x = std::cos(radYaw) * std::cos(radPitch);
        front.y = std::sin(radPitch);
        front.z = std::sin(radYaw) * std::cos(radPitch);
        forward = front.normalized();

        right = m3d::cross(forward, m3d::Vec3(0.0f, 1.0f, 0.0f)).normalized();
        up = m3d::cross(right, forward).normalized();
    }
};

#endif // CAMERA_HPP
