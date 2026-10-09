#pragma once
#ifndef HUD_HPP
#define HUD_HPP

#include "math3d.hpp"
#include "shader.hpp"
#include "track.hpp"
#include <glad/gl.h>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

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

        // Allocate vertex buffer (up to 16384 floats * 6 components = ~1600 quads for text & UI)
        glBufferData(GL_ARRAY_BUFFER, 16384 * sizeof(float) * 6, nullptr, GL_DYNAMIC_DRAW);

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

    void drawCircle(std::vector<float>& buffer, float cx, float cy, float radius,
                    float r, float g, float b, float a, int segs = 14) {
        const float twoPi = 6.28318530718f;
        for (int i = 0; i < segs; ++i) {
            float a1 = (float)i / (float)segs * twoPi;
            float a2 = (float)(i + 1) / (float)segs * twoPi;
            float x1 = cx + radius * std::cos(a1);
            float y1 = cy + radius * std::sin(a1);
            float x2 = cx + radius * std::cos(a2);
            float y2 = cy + radius * std::sin(a2);

            float triVerts[] = {
                cx, cy, r, g, b, a,
                x1, y1, r, g, b, a,
                x2, y2, r, g, b, a
            };
            buffer.insert(buffer.end(), std::begin(triVerts), std::end(triVerts));
        }
    }

    // -------------------------------------------------------------
    // Crisp 3x5 Vector/Bitmap Font System
    // Each glyph is 5 rows, each row has 3 bits (bits 2, 1, 0 from left to right)
    // -------------------------------------------------------------
    static uint8_t getGlyphRow(char c, int row) {
        if (row < 0 || row >= 5) return 0;
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');

        switch (c) {
            case '0': { const uint8_t r[5] = {7, 5, 5, 5, 7}; return r[row]; }
            case '1': { const uint8_t r[5] = {2, 6, 2, 2, 7}; return r[row]; }
            case '2': { const uint8_t r[5] = {7, 1, 7, 4, 7}; return r[row]; }
            case '3': { const uint8_t r[5] = {7, 1, 7, 1, 7}; return r[row]; }
            case '4': { const uint8_t r[5] = {5, 5, 7, 1, 1}; return r[row]; }
            case '5': { const uint8_t r[5] = {7, 4, 7, 1, 7}; return r[row]; }
            case '6': { const uint8_t r[5] = {7, 4, 7, 5, 7}; return r[row]; }
            case '7': { const uint8_t r[5] = {7, 1, 1, 1, 1}; return r[row]; }
            case '8': { const uint8_t r[5] = {7, 5, 7, 5, 7}; return r[row]; }
            case '9': { const uint8_t r[5] = {7, 5, 7, 1, 7}; return r[row]; }
            case 'A': { const uint8_t r[5] = {7, 5, 7, 5, 5}; return r[row]; }
            case 'B': { const uint8_t r[5] = {6, 5, 6, 5, 6}; return r[row]; }
            case 'C': { const uint8_t r[5] = {7, 4, 4, 4, 7}; return r[row]; }
            case 'D': { const uint8_t r[5] = {6, 5, 5, 5, 6}; return r[row]; }
            case 'E': { const uint8_t r[5] = {7, 4, 7, 4, 7}; return r[row]; }
            case 'F': { const uint8_t r[5] = {7, 4, 7, 4, 4}; return r[row]; }
            case 'G': { const uint8_t r[5] = {7, 4, 5, 5, 7}; return r[row]; }
            case 'H': { const uint8_t r[5] = {5, 5, 7, 5, 5}; return r[row]; }
            case 'I': { const uint8_t r[5] = {7, 2, 2, 2, 7}; return r[row]; }
            case 'J': { const uint8_t r[5] = {1, 1, 1, 5, 7}; return r[row]; }
            case 'K': { const uint8_t r[5] = {5, 5, 6, 5, 5}; return r[row]; }
            case 'L': { const uint8_t r[5] = {4, 4, 4, 4, 7}; return r[row]; }
            case 'M': { const uint8_t r[5] = {5, 7, 5, 5, 5}; return r[row]; }
            case 'N': { const uint8_t r[5] = {6, 5, 5, 5, 5}; return r[row]; }
            case 'O': { const uint8_t r[5] = {7, 5, 5, 5, 7}; return r[row]; }
            case 'P': { const uint8_t r[5] = {7, 5, 7, 4, 4}; return r[row]; }
            case 'Q': { const uint8_t r[5] = {7, 5, 5, 7, 1}; return r[row]; }
            case 'R': { const uint8_t r[5] = {7, 5, 7, 6, 5}; return r[row]; }
            case 'S': { const uint8_t r[5] = {7, 4, 7, 1, 7}; return r[row]; }
            case 'T': { const uint8_t r[5] = {7, 2, 2, 2, 2}; return r[row]; }
            case 'U': { const uint8_t r[5] = {5, 5, 5, 5, 7}; return r[row]; }
            case 'V': { const uint8_t r[5] = {5, 5, 5, 5, 2}; return r[row]; }
            case 'W': { const uint8_t r[5] = {5, 5, 5, 7, 5}; return r[row]; }
            case 'X': { const uint8_t r[5] = {5, 5, 2, 5, 5}; return r[row]; }
            case 'Y': { const uint8_t r[5] = {5, 5, 7, 2, 2}; return r[row]; }
            case 'Z': { const uint8_t r[5] = {7, 1, 2, 4, 7}; return r[row]; }
            case '%': { const uint8_t r[5] = {5, 1, 2, 4, 5}; return r[row]; }
            case ':': { const uint8_t r[5] = {0, 2, 0, 2, 0}; return r[row]; }
            case '/': { const uint8_t r[5] = {1, 1, 2, 4, 4}; return r[row]; }
            case '-': { const uint8_t r[5] = {0, 0, 7, 0, 0}; return r[row]; }
            case '+': { const uint8_t r[5] = {0, 2, 7, 2, 0}; return r[row]; }
            case '!': { const uint8_t r[5] = {2, 2, 2, 0, 2}; return r[row]; }
            case '.': { const uint8_t r[5] = {0, 0, 0, 0, 2}; return r[row]; }
            case '[': { const uint8_t r[5] = {6, 4, 4, 4, 6}; return r[row]; }
            case ']': { const uint8_t r[5] = {3, 1, 1, 1, 3}; return r[row]; }
            case '*': { const uint8_t r[5] = {5, 2, 7, 2, 5}; return r[row]; }
            default: return 0;
        }
    }

    void drawChar(std::vector<float>& buffer, float x, float y, char c, float pxSize,
                  float r, float g, float b, float a) {
        for (int row = 0; row < 5; ++row) {
            uint8_t bits = getGlyphRow(c, row);
            if (!bits) continue;
            for (int col = 0; col < 3; ++col) {
                if ((bits >> (2 - col)) & 1) {
                    drawQuad(buffer, x + (float)col * pxSize, y + (float)row * pxSize, pxSize, pxSize, r, g, b, a);
                }
            }
        }
    }

    void drawText(std::vector<float>& buffer, float x, float y, const std::string& text,
                  float pxSize, float r, float g, float b, float a) {
        float curX = x;
        float charW = 3.0f * pxSize;
        float charGap = 1.0f * pxSize;

        for (char c : text) {
            if (c == ' ') {
                curX += charW + charGap * 1.5f;
                continue;
            }
            drawChar(buffer, curX, y, c, pxSize, r, g, b, a);
            curX += charW + charGap;
        }
    }

    // -------------------------------------------------------------
    // Main HUD Render Pipeline
    // -------------------------------------------------------------
    void render(const Shader& hudShader, int screenWidth, int screenHeight,
                float speedMps, float targetSpeedMps, EnvironmentZone zone,
                bool isNight, int cameraMode, bool shearingActive, bool headlightsActive,
                bool isJourneyComplete = false, bool isManualDrive = true, bool isReversing = false,
                float fuel = 100.0f, int coins = 0, int currentMilestone = 0, int totalMilestones = 5,
                bool isNearStation = false, float notificationTimer = 0.0f, float globalTime = 0.0f,
                const std::string& notificationMsg = "", bool isColliding = false,
                const std::string& collisionObstacle = "") {
        std::vector<float> verts;
        verts.reserve(8192);
        (void)targetSpeedMps; (void)shearingActive; (void)cameraMode;
        (void)headlightsActive; (void)isJourneyComplete;

        // Orthographic 2D Projection
        m3d::Mat4 ortho = m3d::Mat4::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
        hudShader.use();
        hudShader.setMat4("projection", ortho);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // =========================================================
        // TOP FLOATING NOTIFICATION BANNER (When events trigger)
        // =========================================================
        if (notificationTimer > 0.0f && !notificationMsg.empty()) {
            float banW = 540.0f;
            float banH = 38.0f;
            float banX = (screenWidth > 1020) ? 520.0f : 20.0f;
            float banY = (screenWidth > 1020) ? 20.0f : 205.0f;

            bool isMilestoneEvent = (notificationMsg.find("CHECKPOINT") != std::string::npos ||
                                     notificationMsg.find("LAP") != std::string::npos ||
                                     notificationMsg.find("MILESTONE") != std::string::npos);

            // Background glass
            drawQuad(verts, banX, banY, banW, banH, 0.06f, 0.08f, 0.12f, 0.95f);
            if (isMilestoneEvent) {
                // Vibrant gold / emerald accent line
                float aPulse = 0.85f + 0.15f * std::sin(globalTime * 8.0f);
                drawQuad(verts, banX, banY, banW, 2.5f, 0.15f * aPulse, 0.95f * aPulse, 0.40f * aPulse, 1.0f);
            } else {
                drawQuad(verts, banX, banY, banW, 2.5f, 1.0f, 0.85f, 0.20f, 1.0f);
            }

            // Left icon: Checkered Racing Flag if milestone event
            if (isMilestoneEvent) {
                // Left 2D Checkered Racing Flag
                float lPoleX = banX + 10.0f;
                drawQuad(verts, lPoleX, banY + 7.0f, 2.0f, 24.0f, 0.88f, 0.92f, 0.98f, 1.0f);
                drawQuad(verts, lPoleX - 1.0f, banY + 5.5f, 4.0f, 3.5f, 1.0f, 0.82f, 0.18f, 1.0f);
                for (int cfc = 0; cfc < 3; ++cfc) {
                    for (int cfr = 0; cfr < 3; ++cfr) {
                        float cx = lPoleX + 2.0f + (float)cfc * 5.0f;
                        float cy = banY + 8.0f + (float)cfr * 5.0f;
                        bool sqW = ((cfc + cfr) % 2 == 0);
                        if (sqW) drawQuad(verts, cx, cy, 5.0f, 5.0f, 0.98f, 0.98f, 1.0f, 1.0f);
                        else     drawQuad(verts, cx, cy, 5.0f, 5.0f, 0.10f, 0.12f, 0.16f, 1.0f);
                    }
                }

                // Right matching 2D Checkered Racing Flag
                float rPoleX = banX + banW - 12.0f;
                drawQuad(verts, rPoleX, banY + 7.0f, 2.0f, 24.0f, 0.88f, 0.92f, 0.98f, 1.0f);
                drawQuad(verts, rPoleX - 1.0f, banY + 5.5f, 4.0f, 3.5f, 1.0f, 0.82f, 0.18f, 1.0f);
                for (int cfc = 0; cfc < 3; ++cfc) {
                    for (int cfr = 0; cfr < 3; ++cfr) {
                        float cx = rPoleX - 17.0f + (float)cfc * 5.0f;
                        float cy = banY + 8.0f + (float)cfr * 5.0f;
                        bool sqW = ((cfc + cfr) % 2 == 0);
                        if (sqW) drawQuad(verts, cx, cy, 5.0f, 5.0f, 0.98f, 0.98f, 1.0f, 1.0f);
                        else     drawQuad(verts, cx, cy, 5.0f, 5.0f, 0.10f, 0.12f, 0.16f, 1.0f);
                    }
                }
            } else {
                // Standard info icon
                drawQuad(verts, banX + 10.0f, banY + 7.0f, 24.0f, 24.0f, 1.0f, 0.85f, 0.15f, 0.95f);
                drawQuad(verts, banX + 18.0f, banY + 12.0f, 8.0f, 14.0f, 0.10f, 0.12f, 0.16f, 1.0f);
            }

            // Message text
            drawText(verts, banX + 36.0f, banY + 12.0f, notificationMsg, 2.2f, 1.0f, 0.95f, 0.85f, 1.0f);
        }

        // =========================================================
        // MAIN GAME DASHBOARD CARD (Top-Left)
        // Clean, structured, highly legible design with dedicated rows:
        // ROW 1: Fuel Level (Stays green, turns red when low, refuels to green)
        // ROW 2: Coin Count (Gold coin icon & explicit number) & Milestones
        // ROW 3: Speedometer & Telemetry
        // ROW 4: Contextual Interactive Action Prompt
        // =========================================================
        float dashX = 20.0f;
        float dashY = 20.0f;
        float dashW = 496.0f;
        float dashH = 176.0f;

        // 1. Sleek Glassmorphism Card Frame
        drawQuad(verts, dashX, dashY, dashW, dashH, 0.06f, 0.08f, 0.12f, 0.94f);
        // Border outline
        drawQuad(verts, dashX, dashY, dashW, 1.0f, 0.20f, 0.28f, 0.40f, 0.6f);
        drawQuad(verts, dashX, dashY + dashH - 1.0f, dashW, 1.0f, 0.20f, 0.28f, 0.40f, 0.6f);
        drawQuad(verts, dashX, dashY, 1.0f, dashH, 0.20f, 0.28f, 0.40f, 0.6f);
        drawQuad(verts, dashX + dashW - 1.0f, dashY, 1.0f, dashH, 0.20f, 0.28f, 0.40f, 0.6f);

        // Header Accent Stripe
        if (isColliding) {
            float cPulse = 0.6f + 0.4f * std::sin(globalTime * 12.0f);
            drawQuad(verts, dashX, dashY, dashW, 3.5f, 0.95f * cPulse, 0.15f, 0.15f, 1.0f);
        } else if (fuel <= 25.0f) {
            float fPulse = 0.6f + 0.4f * std::sin(globalTime * 8.0f);
            drawQuad(verts, dashX, dashY, dashW, 3.5f, 0.95f * fPulse, 0.20f, 0.15f, 1.0f);
        } else {
            drawQuad(verts, dashX, dashY, dashW, 3.5f, 0.15f, 0.85f, 0.95f, 1.0f);
        }

        // ---------------------------------------------------------
        // HEADER: TITLE & STATUS BADGES
        // ---------------------------------------------------------
        drawText(verts, dashX + 14.0f, dashY + 12.0f, "DASH", 2.0f, 0.92f, 0.96f, 1.0f, 1.0f);

        // Drive Mode Badge: [MANUAL] or [AUTOPILOT]
        if (isManualDrive) {
            drawQuad(verts, dashX + 80.0f, dashY + 9.0f, 62.0f, 18.0f, 0.10f, 0.45f, 0.70f, 0.95f);
            drawText(verts, dashX + 86.0f, dashY + 13.0f, "MANUAL", 1.6f, 1.0f, 1.0f, 1.0f, 1.0f);

            // Gear Indicator: [D] or [R]
            if (isReversing) {
                drawQuad(verts, dashX + 146.0f, dashY + 9.0f, 22.0f, 18.0f, 0.90f, 0.15f, 0.15f, 0.95f);
                drawText(verts, dashX + 152.0f, dashY + 13.0f, "R", 1.8f, 1.0f, 1.0f, 1.0f, 1.0f);
            } else {
                drawQuad(verts, dashX + 146.0f, dashY + 9.0f, 22.0f, 18.0f, 0.12f, 0.75f, 0.35f, 0.95f);
                drawText(verts, dashX + 152.0f, dashY + 13.0f, "D", 1.8f, 1.0f, 1.0f, 1.0f, 1.0f);
            }
        } else {
            drawQuad(verts, dashX + 80.0f, dashY + 9.0f, 88.0f, 18.0f, 0.50f, 0.20f, 0.75f, 0.95f);
            drawText(verts, dashX + 85.0f, dashY + 13.0f, "AUTOPILOT", 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);
        }

        // Zone Name Badge
        std::string zoneName = "GARAGE";
        m3d::Vec3 zoneCol(0.9f, 0.5f, 0.1f);
        if (zone == EnvironmentZone::CITY)             { zoneName = "CITY"; zoneCol = m3d::Vec3(0.2f, 0.7f, 0.3f); }
        else if (zone == EnvironmentZone::BRIDGE)      { zoneName = "BRIDGE"; zoneCol = m3d::Vec3(0.2f, 0.5f, 0.95f); }
        else if (zone == EnvironmentZone::TUNNEL)      { zoneName = "TUNNEL"; zoneCol = m3d::Vec3(0.85f, 0.3f, 0.2f); }
        else if (zone == EnvironmentZone::COUNTRYSIDE) { zoneName = "HIGHWAY"; zoneCol = m3d::Vec3(0.2f, 0.8f, 0.5f); }

        drawQuad(verts, dashX + 172.0f, dashY + 9.0f, 68.0f, 18.0f, zoneCol.x * 0.7f, zoneCol.y * 0.7f, zoneCol.z * 0.7f, 0.95f);
        drawText(verts, dashX + 177.0f, dashY + 13.0f, zoneName, 1.7f, 1.0f, 1.0f, 1.0f, 1.0f);

        // Day/Night Pill
        if (isNight) {
            drawQuad(verts, dashX + 244.0f, dashY + 9.0f, 54.0f, 18.0f, 0.15f, 0.20f, 0.45f, 0.95f);
            drawText(verts, dashX + 249.0f, dashY + 13.0f, "NIGHT", 1.6f, 0.85f, 0.90f, 1.0f, 1.0f);
        } else {
            drawQuad(verts, dashX + 244.0f, dashY + 9.0f, 54.0f, 18.0f, 0.85f, 0.65f, 0.12f, 0.95f);
            drawText(verts, dashX + 254.0f, dashY + 13.0f, "DAY", 1.6f, 0.12f, 0.14f, 0.18f, 1.0f);
        }

        // Shading Model Badge: Radiant Cyan Pill for Blinn-Phong
        drawQuad(verts, dashX + 302.0f, dashY + 9.0f, 108.0f, 18.0f, 0.10f, 0.52f, 0.85f, 0.95f);
        drawText(verts, dashX + 307.0f, dashY + 13.0f, "BLINN-PHONG", 1.4f, 1.0f, 1.0f, 1.0f, 1.0f);

        // ---------------------------------------------------------
        // ROW 1: FUEL LEVEL
        // User Requirement:
        // "show fuel level well. it will stay, and when low it will be red.
        //  and refuel will make it green. like other games. one row for fuel level."
        // ---------------------------------------------------------
        float row1Y = dashY + 38.0f;

        // [A] Fuel Pump Icon
        float pumpX = dashX + 16.0f;
        float pumpY = row1Y + 2.0f;
        // Color of icon reflects fuel status
        float statusR = 0.12f, statusG = 0.86f, statusB = 0.38f; // Stays Green normally!
        if (fuel <= 25.0f) {
            float p = 0.7f + 0.3f * std::sin(globalTime * 8.0f);
            statusR = 0.95f * p; statusG = 0.15f * p; statusB = 0.15f * p; // Red when low!
        }

        // Pump Cabinet
        drawQuad(verts, pumpX, pumpY, 13.0f, 18.0f, statusR, statusG, statusB, 1.0f);
        // Pump LCD screen inside
        drawQuad(verts, pumpX + 2.5f, pumpY + 3.0f, 8.0f, 5.0f, 0.08f, 0.10f, 0.14f, 1.0f);
        // Pump Base Pad
        drawQuad(verts, pumpX - 2.0f, pumpY + 18.0f, 17.0f, 3.0f, statusR, statusG, statusB, 1.0f);
        // Hanging nozzle on the right
        drawQuad(verts, pumpX + 14.5f, pumpY + 4.0f, 3.5f, 10.0f, statusR, statusG, statusB, 1.0f);
        drawQuad(verts, pumpX + 13.0f, pumpY + 13.0f, 5.0f, 3.0f, statusR, statusG, statusB, 1.0f);

        // [B] Label: "FUEL"
        drawText(verts, dashX + 44.0f, row1Y + 5.0f, "FUEL", 2.2f, 0.92f, 0.96f, 1.0f, 1.0f);

        // [C] Fuel Gauge Track & Fill Bar
        float gaugeX = dashX + 104.0f;
        float gaugeY = row1Y + 3.0f;
        float gaugeW = 236.0f;
        float gaugeH = 17.0f;

        // Dark track background with border
        drawQuad(verts, gaugeX, gaugeY, gaugeW, gaugeH, 0.11f, 0.13f, 0.17f, 1.0f);
        drawQuad(verts, gaugeX, gaugeY, gaugeW, 1.0f, 0.22f, 0.28f, 0.36f, 0.8f);
        drawQuad(verts, gaugeX, gaugeY + gaugeH - 1.0f, gaugeW, 1.0f, 0.22f, 0.28f, 0.36f, 0.8f);

        // Dynamic Fuel Fill
        float fuelFrac = m3d::clamp(fuel / 100.0f, 0.0f, 1.0f);
        float fillW = fuelFrac * gaugeW;
        if (fillW > 0.0f) {
            drawQuad(verts, gaugeX, gaugeY, fillW, gaugeH, statusR, statusG, statusB, 1.0f);
        }

        // Notch dividers at 25%, 50%, 75%
        for (float mark : {0.25f, 0.50f, 0.75f}) {
            drawQuad(verts, gaugeX + mark * gaugeW - 1.0f, gaugeY, 2.0f, gaugeH, 0.08f, 0.10f, 0.14f, 0.7f);
        }

        // [D] Digital Percentage Readout: e.g. "100%", "78%", "15%"
        int fuelInt = static_cast<int>(std::round(fuel));
        std::string fuelStr = std::to_string(fuelInt) + "%";
        if (fuel <= 25.0f) {
            // Flash "LOW!" if under 20%
            if (std::fmod(globalTime, 1.0f) < 0.5f) {
                fuelStr = "LOW!";
            }
            drawText(verts, dashX + 350.0f, row1Y + 5.0f, fuelStr, 2.2f, 0.98f, 0.25f, 0.25f, 1.0f);
        } else {
            drawText(verts, dashX + 350.0f, row1Y + 5.0f, fuelStr, 2.2f, 0.20f, 0.92f, 0.45f, 1.0f);
        }

        // ---------------------------------------------------------
        // ROW 2: COIN COUNT
        // User Requirement:
        // "then a coint ,and beside count for coin count. like this. do logical things."
        // ---------------------------------------------------------
        float row2Y = dashY + 69.0f;

        // [A] Beautiful 2D Golden Coin Icon
        float coinCX = dashX + 22.0f;
        float coinCY = row2Y + 11.0f;
        // Outer rich gold rim
        drawCircle(verts, coinCX, coinCY, 10.5f, 0.98f, 0.74f, 0.10f, 1.0f, 14);
        // Inner bright shiny gold core
        drawCircle(verts, coinCX, coinCY, 8.0f, 1.0f, 0.90f, 0.28f, 1.0f, 14);
        // Center embossed gold emblem ($ / star)
        drawQuad(verts, coinCX - 2.5f, coinCY - 4.5f, 5.0f, 9.0f, 0.82f, 0.55f, 0.06f, 1.0f);
        drawQuad(verts, coinCX - 4.0f, coinCY - 2.0f, 8.0f, 4.0f, 0.82f, 0.55f, 0.06f, 1.0f);
        // Metallic specular shine reflection
        drawQuad(verts, coinCX - 4.5f, coinCY - 5.5f, 3.5f, 3.5f, 1.0f, 1.0f, 0.85f, 0.95f);

        // [B] Label & Multiplication Sign: "COINS" and "x"
        drawText(verts, dashX + 44.0f, row2Y + 5.0f, "COINS", 2.2f, 1.0f, 0.82f, 0.15f, 1.0f);

        // Beside count: format 2-digit count e.g. "x 05" or "x 18"
        std::string coinStr = "x " + ((coins < 10) ? ("0" + std::to_string(coins)) : std::to_string(coins));
        drawText(verts, dashX + 112.0f, row2Y + 4.0f, coinStr, 2.6f, 1.0f, 0.95f, 0.35f, 1.0f);

        // [C] Milestone Checkpoint Tracker Pill
        float gatePillX = dashX + 205.0f;
        drawQuad(verts, gatePillX, row2Y + 2.0f, 209.0f, 20.0f, 0.12f, 0.16f, 0.24f, 0.95f);

        // 2D Checkered Racing Flag Icon
        float flagPoleX = gatePillX + 6.0f;
        float flagPoleY = row2Y + 4.0f;
        drawQuad(verts, flagPoleX, flagPoleY, 1.8f, 15.0f, 0.88f, 0.92f, 0.98f, 1.0f);
        drawQuad(verts, flagPoleX - 1.0f, flagPoleY - 1.0f, 3.8f, 3.0f, 1.0f, 0.82f, 0.18f, 1.0f);
        float fClothX = flagPoleX + 1.8f;
        float fClothY = flagPoleY + 1.0f;
        for (int fc = 0; fc < 3; ++fc) {
            for (int fr = 0; fr < 2; ++fr) {
                float fx = fClothX + (float)fc * 3.6f;
                float fy = fClothY + (float)fr * 4.5f;
                bool isSqWhite = ((fc + fr) % 2 == 0);
                if (isSqWhite) drawQuad(verts, fx, fy, 3.6f, 4.5f, 0.96f, 0.96f, 1.0f, 1.0f);
                else           drawQuad(verts, fx, fy, 3.6f, 4.5f, 0.10f, 0.12f, 0.16f, 1.0f);
            }
        }

        // Label: "CHECK"
        drawText(verts, gatePillX + 21.0f, row2Y + 6.0f, "CHECK", 1.8f, 0.75f, 0.85f, 0.95f, 1.0f);

        // Checkpoint indicator boxes (5 numbered segments)
        float pipStartX = gatePillX + 64.0f;
        for (int m = 0; m < totalMilestones; ++m) {
            float px = pipStartX + (float)m * 28.0f;
            float py = row2Y + 5.0f;
            if (m < currentMilestone) {
                // Completed: Emerald Green with dark numeral
                drawQuad(verts, px, py, 25.0f, 13.0f, 0.15f, 0.90f, 0.35f, 1.0f);
                drawChar(verts, px + 10.0f, py + 2.5f, '1' + m, 1.6f, 0.04f, 0.25f, 0.08f, 1.0f);
            } else if (m == currentMilestone) {
                // Target: Pulsing Electric Cyan with dark numeral
                float p = 0.70f + 0.30f * std::sin(globalTime * 8.0f);
                drawQuad(verts, px, py, 25.0f, 13.0f, 0.12f * p, 0.85f * p, 1.0f * p, 1.0f);
                drawChar(verts, px + 10.0f, py + 2.5f, '1' + m, 1.6f, 0.04f, 0.12f, 0.22f, 1.0f);
            } else {
                // Upcoming: Dark Slate with muted numeral
                drawQuad(verts, px, py, 25.0f, 13.0f, 0.18f, 0.22f, 0.30f, 0.85f);
                drawChar(verts, px + 10.0f, py + 2.5f, '1' + m, 1.6f, 0.50f, 0.58f, 0.68f, 0.9f);
            }
        }

        // ---------------------------------------------------------
        // ROW 3: SPEEDOMETER & GEAR
        // ---------------------------------------------------------
        float row3Y = dashY + 100.0f;

        // Speed Dial Icon
        float spdIconX = dashX + 16.0f;
        float spdIconY = row3Y + 3.0f;
        drawQuad(verts, spdIconX, spdIconY + 4.0f, 16.0f, 12.0f, 0.20f, 0.65f, 0.90f, 1.0f);
        drawQuad(verts, spdIconX + 4.0f, spdIconY, 8.0f, 4.0f, 0.20f, 0.65f, 0.90f, 1.0f);
        drawQuad(verts, spdIconX + 6.0f, spdIconY + 5.0f, 4.0f, 6.0f, 0.95f, 0.95f, 0.98f, 1.0f);

        // Label: "SPEED"
        drawText(verts, dashX + 44.0f, row3Y + 5.0f, "SPEED", 2.2f, 0.80f, 0.88f, 0.96f, 1.0f);

        // Speed Gauge Bar
        float spdGaugeX = dashX + 104.0f;
        float spdGaugeY = row3Y + 4.0f;
        float spdGaugeW = 200.0f;
        float spdGaugeH = 15.0f;

        drawQuad(verts, spdGaugeX, spdGaugeY, spdGaugeW, spdGaugeH, 0.11f, 0.13f, 0.17f, 1.0f);

        float speedKmh = std::abs(speedMps) * 3.6f;
        float speedFrac = m3d::clamp(speedKmh / 95.0f, 0.0f, 1.0f);
        float sR = 0.15f + 0.80f * speedFrac;
        float sG = 0.85f - 0.25f * speedFrac;
        float sB = 0.95f - 0.70f * speedFrac;
        drawQuad(verts, spdGaugeX, spdGaugeY, speedFrac * spdGaugeW, spdGaugeH, sR, sG, sB, 1.0f);

        // Digital Speed Number: e.g. "45 KM/H"
        int spdInt = static_cast<int>(std::round(speedKmh));
        std::string spdStr = std::to_string(spdInt) + " KM/H";
        drawText(verts, dashX + 314.0f, row3Y + 5.0f, spdStr, 2.2f, 0.95f, 0.98f, 1.0f, 1.0f);

        // ---------------------------------------------------------
        // ROW 4: CONTEXTUAL INTERACTIVE ACTION PROMPT & ALERTS
        // Shows clear actionable status:
        // - Collision hit & reverse instruction
        // - Gas station refuel prompt with [F]
        // - Low fuel alert
        // - Normal driving key guide
        // ---------------------------------------------------------
        float row4Y = dashY + 132.0f;
        float row4W = dashW - 28.0f;
        float row4H = 28.0f;
        float row4X = dashX + 14.0f;

        if (isColliding) {
            // Collision Alert: Bright flashing red banner
            float cPulse = 0.8f + 0.2f * std::sin(globalTime * 12.0f);
            drawQuad(verts, row4X, row4Y, row4W, row4H, 0.92f * cPulse, 0.12f, 0.15f, 0.96f);
            drawQuad(verts, row4X + 4.0f, row4Y + 4.0f, 20.0f, 20.0f, 1.0f, 1.0f, 1.0f, 0.95f);
            drawText(verts, row4X + 11.0f, row4Y + 7.0f, "!", 2.2f, 0.9f, 0.1f, 0.1f, 1.0f);

            std::string colMsg = "COLLISION: " + (collisionObstacle.empty() ? "OBSTACLE" : collisionObstacle) + " - REVERSE TO CLEAR";
            drawText(verts, row4X + 32.0f, row4Y + 9.0f, colMsg, 1.9f, 1.0f, 1.0f, 1.0f, 1.0f);

        } else if (isNearStation) {
            // Gas Station Refuel Available: Glowing green prompt
            float rPulse = 0.85f + 0.15f * std::sin(globalTime * 8.0f);
            drawQuad(verts, row4X, row4Y, row4W, row4H, 0.12f * rPulse, 0.78f * rPulse, 0.35f * rPulse, 0.96f);

            // [F] Key pill
            drawQuad(verts, row4X + 8.0f, row4Y + 4.0f, 24.0f, 20.0f, 0.08f, 0.10f, 0.14f, 0.95f);
            drawText(verts, row4X + 15.0f, row4Y + 7.0f, "F", 2.2f, 0.2f, 0.95f, 0.45f, 1.0f);

            drawText(verts, row4X + 40.0f, row4Y + 9.0f, "PRESS 'F' TO REFUEL (+30% / 1 COIN)", 1.9f, 1.0f, 1.0f, 1.0f, 1.0f);

        } else if (fuel <= 0.0f) {
            // Empty Tank Alert
            drawQuad(verts, row4X, row4Y, row4W, row4H, 0.90f, 0.15f, 0.15f, 0.95f);
            drawText(verts, row4X + 16.0f, row4Y + 9.0f, "OUT OF FUEL! VISIT GAS STATION OR RESET [R]", 1.8f, 1.0f, 1.0f, 1.0f, 1.0f);

        } else if (fuel <= 25.0f) {
            // Low Fuel Warning
            float lPulse = 0.8f + 0.2f * std::sin(globalTime * 8.0f);
            drawQuad(verts, row4X, row4Y, row4W, row4H, 0.95f * lPulse, 0.45f * lPulse, 0.10f, 0.95f);
            drawText(verts, row4X + 16.0f, row4Y + 9.0f, "WARNING: LOW FUEL (<25%)! VISIT A GAS STATION", 1.8f, 1.0f, 1.0f, 1.0f, 1.0f);

        } else {
            // Normal Driving Controls Prompt
            drawQuad(verts, row4X, row4Y, row4W, row4H, 0.12f, 0.15f, 0.20f, 0.95f);
            drawText(verts, row4X + 16.0f, row4Y + 9.0f, "DRIVE: [WASD] | LIGHTS: [L/H/N] | SHADING: [B] | REFUEL: [F]", 1.7f, 0.80f, 0.86f, 0.94f, 1.0f);
        }

        // =========================================================
        // Flush and Render Vertices
        // =========================================================
        if (!verts.empty()) {
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(float), verts.data());
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size() / 6));
            glBindVertexArray(0);
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
};

#endif // HUD_HPP
