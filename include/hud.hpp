#pragma once
#ifndef HUD_HPP
#define HUD_HPP

#include "math3d.hpp"
#include "shader.hpp"
#include "track.hpp"
#include <glad/gl.h>
#include <vector>

class HUD {
public:
    GLuint vao{0};
    GLuint vbo{0};

    HUD() = default;

    void init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // 2D Quad format: float x, y, r, g, b, a
        glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(float) * 6, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0); // Pos 2D
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1); // Color 4D
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    ~HUD() {
        if (vbo != 0) glDeleteBuffers(1, &vbo);
        if (vao != 0) glDeleteVertexArrays(1, &vao);
    }

    void drawQuad(std::vector<float>& buffer, float x, float y, float w, float h, 
                  float r, float g, float b, float a) {
        // Two triangles forming quad: (x, y) to (x+w, y+h)
        float x1 = x, y1 = y;
        float x2 = x + w, y2 = y + h;

        float quadVerts[] = {
            x1, y1, r, g, b, a,
            x2, y1, r, g, b, a,
            x2, y2, r, g, b, a,

            x1, y1, r, g, b, a,
            x2, y2, r, g, b, a,
            x1, y2, r, g, b, a,
        };

        buffer.insert(buffer.end(), std::begin(quadVerts), std::end(quadVerts));
    }

    void render(const Shader& hudShader, int screenWidth, int screenHeight,
                float speedMps, float targetSpeedMps, EnvironmentZone zone,
                bool isNight, int cameraMode, bool shearingActive, bool headlightsActive,
                bool isJourneyComplete = false) {
        std::vector<float> verts;
        verts.reserve(512);

        // Setup Orthographic projection matrix for HUD
        m3d::Mat4 ortho = m3d::Mat4::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
        hudShader.use();
        hudShader.setMat4("projection", ortho);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // ==========================================
        // 0. JOURNEY COMPLETION BANNER (Center Screen)
        // ==========================================
        if (isJourneyComplete) {
            float bannerW = 480.0f;
            float bannerH = 75.0f;
            float bannerX = ((float)screenWidth - bannerW) * 0.5f;
            float bannerY = 25.0f;

            // Background glassmorphism panel
            drawQuad(verts, bannerX, bannerY, bannerW, bannerH, 0.06f, 0.08f, 0.12f, 0.94f);
            // Emerald/gold top accent strip
            drawQuad(verts, bannerX, bannerY, bannerW, 3.5f, 0.2f, 0.95f, 0.45f, 1.0f);

            // Journey Completed Badge (Emerald green)
            drawQuad(verts, bannerX + 15.0f, bannerY + 12.0f, bannerW - 30.0f, 26.0f, 0.15f, 0.65f, 0.35f, 0.95f);

            // "[R] RESTART JOURNEY" Prompt pill
            drawQuad(verts, bannerX + 80.0f, bannerY + 45.0f, bannerW - 160.0f, 20.0f, 0.25f, 0.32f, 0.42f, 0.9f);
        }

        // ==========================================
        // 1. TOP STATUS BAR (Glassmorphism dark banner)
        // ==========================================
        drawQuad(verts, 20.0f, 20.0f, 380.0f, 115.0f, 0.08f, 0.10f, 0.14f, 0.88f);
        // Header accent line
        drawQuad(verts, 20.0f, 20.0f, 380.0f, 3.0f, 0.20f, 0.65f, 0.95f, 1.0f);

        // Zone Tag Badge
        float badgeR = 0.2f, badgeG = 0.6f, badgeB = 0.9f;
        if (isJourneyComplete)                    { badgeR = 0.15f; badgeG = 0.85f; badgeB = 0.35f; }
        else if (zone == EnvironmentZone::GARAGE) { badgeR = 0.9f; badgeG = 0.5f; badgeB = 0.1f; }
        else if (zone == EnvironmentZone::CITY)   { badgeR = 0.3f; badgeG = 0.7f; badgeB = 0.3f; }
        else if (zone == EnvironmentZone::BRIDGE) { badgeR = 0.2f; badgeG = 0.5f; badgeB = 0.95f; }
        else if (zone == EnvironmentZone::TUNNEL) { badgeR = 0.85f; badgeG = 0.3f; badgeB = 0.2f; }
        else if (zone == EnvironmentZone::COUNTRYSIDE) { badgeR = 0.2f; badgeG = 0.8f; badgeB = 0.5f; }

        drawQuad(verts, 30.0f, 32.0f, 120.0f, 22.0f, badgeR, badgeG, badgeB, 0.9f);

        // Day/Night indicator pill
        if (isNight) {
            drawQuad(verts, 160.0f, 32.0f, 75.0f, 22.0f, 0.15f, 0.2f, 0.45f, 0.9f); // Dark blue "NIGHT"
        } else {
            drawQuad(verts, 160.0f, 32.0f, 75.0f, 22.0f, 0.95f, 0.75f, 0.15f, 0.9f); // Warm sun "DAY"
        }

        // Headlights indicator pill
        if (headlightsActive) {
            drawQuad(verts, 245.0f, 32.0f, 85.0f, 22.0f, 0.2f, 0.8f, 0.4f, 0.85f);
        } else {
            drawQuad(verts, 245.0f, 32.0f, 85.0f, 22.0f, 0.3f, 0.3f, 0.35f, 0.85f);
        }

        // ==========================================
        // 2. SPEEDOMETER GAUGE BAR
        // ==========================================
        float speedKmh = speedMps * 3.6f;
        float maxSpeedKmh = 100.0f;
        float speedFrac = m3d::clamp(speedKmh / maxSpeedKmh, 0.0f, 1.0f);

        // Speedometer background track
        drawQuad(verts, 30.0f, 68.0f, 360.0f, 16.0f, 0.15f, 0.18f, 0.22f, 1.0f);

        // Target speed notch
        float targetKmh = targetSpeedMps * 3.6f;
        float targetFrac = m3d::clamp(targetKmh / maxSpeedKmh, 0.0f, 1.0f);
        drawQuad(verts, 30.0f + targetFrac * 360.0f - 2.0f, 64.0f, 4.0f, 24.0f, 1.0f, 0.9f, 0.2f, 1.0f);

        // Speedometer filled dynamic bar (Gradient from Cyan to Green to Orange)
        float barR = 0.2f + 0.7f * speedFrac;
        float barG = 0.8f - 0.3f * speedFrac;
        float barB = 0.9f - 0.7f * speedFrac;
        drawQuad(verts, 30.0f, 68.0f, speedFrac * 360.0f, 16.0f, barR, barG, barB, 1.0f);

        // Camera mode indicator badge
        float camR = 0.2f, camG = 0.5f, camB = 0.8f;
        if (cameraMode == 1) { camR = 0.2f; camG = 0.6f; camB = 0.9f; }      // Chase (Cyan-blue)
        else if (cameraMode == 2) { camR = 0.9f; camG = 0.4f; camB = 0.2f; } // Cockpit (Orange)
        else if (cameraMode == 3) { camR = 0.2f; camG = 0.7f; camB = 0.4f; } // Overhead (Green)
        else if (cameraMode == 4) { camR = 0.7f; camG = 0.3f; camB = 0.8f; } // Orbit (Purple)
        else if (cameraMode == 5) { camR = 0.85f; camG = 0.75f; camB = 0.2f; } // Scenic (Gold)
        drawQuad(verts, 30.0f, 96.0f, 170.0f, 24.0f, camR, camG, camB, 0.85f);
        // Shear status badge
        if (shearingActive) {
            drawQuad(verts, 210.0f, 96.0f, 180.0f, 24.0f, 0.5f, 0.2f, 0.7f, 0.85f);
        } else {
            drawQuad(verts, 210.0f, 96.0f, 180.0f, 24.0f, 0.25f, 0.25f, 0.28f, 0.85f);
        }

        // ==========================================
        // 3. CONTROLS GUIDE PANEL (Bottom-Right)
        // ==========================================
        float guideW = 340.0f;
        float guideH = 135.0f;
        float guideX = (float)screenWidth - guideW - 20.0f;
        float guideY = (float)screenHeight - guideH - 20.0f;

        drawQuad(verts, guideX, guideY, guideW, guideH, 0.08f, 0.10f, 0.14f, 0.88f);
        drawQuad(verts, guideX, guideY, guideW, 2.5f, 0.95f, 0.55f, 0.20f, 1.0f);

        // Control chips
        drawQuad(verts, guideX + 15.0f, guideY + 15.0f, 80.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);
        drawQuad(verts, guideX + 105.0f, guideY + 15.0f, 95.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);
        drawQuad(verts, guideX + 210.0f, guideY + 15.0f, 110.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);

        drawQuad(verts, guideX + 15.0f, guideY + 45.0f, 90.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);
        drawQuad(verts, guideX + 115.0f, guideY + 45.0f, 95.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);
        drawQuad(verts, guideX + 220.0f, guideY + 45.0f, 100.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);

        drawQuad(verts, guideX + 15.0f, guideY + 75.0f, 140.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);
        drawQuad(verts, guideX + 165.0f, guideY + 75.0f, 155.0f, 22.0f, 0.2f, 0.24f, 0.3f, 0.9f);

        drawQuad(verts, guideX + 15.0f, guideY + 105.0f, guideW - 30.0f, 18.0f, 0.15f, 0.18f, 0.22f, 0.8f);

        // Render vertices
        if (!verts.empty()) {
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(float), verts.data());
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size() / 6));
            glBindVertexArray(0);
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
};

#endif // HUD_HPP
