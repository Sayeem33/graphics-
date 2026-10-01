#pragma once
#ifndef SCENE_HPP
#define SCENE_HPP

#include "math3d.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "track.hpp"
#include <vector>
#include <cmath>

struct ObjectMaterial {
    m3d::Vec3 ambient{0.3f, 0.3f, 0.3f};
    m3d::Vec3 diffuse{0.8f, 0.8f, 0.8f};
    m3d::Vec3 specular{0.5f, 0.5f, 0.5f};
    float shininess{32.0f};
};

struct LightSource {
    m3d::Vec3 position;
    m3d::Vec3 color;
    float intensity{1.0f};
    float range{18.0f};
};

// Tree instance data
struct TreeInstance {
    m3d::Vec3 position;
    float scale;
    float phase;
    m3d::Vec3 foliageColor;
};

// Street lamp instance data
struct StreetLamp {
    m3d::Vec3 position;
    float rotationY;
    bool isBridgeLamp{false};
};

enum class BoatType {
    CABIN_CRUISER = 0,
    SAILBOAT = 1,
    RIVER_PATROL = 2
};

// Boat on the river
struct Boat {
    m3d::Vec3 basePosition;
    float speed;
    float heading; // in radians
    m3d::Vec3 hullColor;
    float length;
    BoatType type{BoatType::CABIN_CRUISER};
};

class SceneManager {
public:
    // Shared primitive meshes
    Mesh cubeMesh;
    Mesh cylinderMesh;
    Mesh coneMesh;
    Mesh sphereMesh;
    Mesh planeMesh;

    // Specialized architectural meshes
    Mesh tunnelVaultMesh;
    Mesh tunnelCeilingLightMesh;
    Mesh tunnelBarriersMesh;
    Mesh stonePortalMesh;
    Mesh sailboatSailMesh;
    Mesh boatHullMesh;

    // Procedural Road Mesh
    Mesh roadMesh;
    Mesh roadStripesMesh;
    Mesh riverMesh;

    // Environmental entities
    std::vector<TreeInstance> trees;
    std::vector<StreetLamp> streetLamps;
    std::vector<LightSource> tunnelCeilingLights;
    std::vector<Boat> boats;

    // Car state
    m3d::Vec3 carPos{0.0f, 0.0f, 0.0f};
    m3d::Vec3 carForward{0.0f, 0.0f, -1.0f};
    m3d::Vec3 carUp{0.0f, 1.0f, 0.0f};
    m3d::Vec3 carRight{1.0f, 0.0f, 0.0f};
    float carDistance{0.0f};
    float carSpeed{0.0f};
    float wheelSpinAngle{0.0f};
    float wheelSteerAngle{0.0f};
    EnvironmentZone currentZone{EnvironmentZone::GARAGE};
    bool isJourneyComplete{false};

    // Environmental state
    bool isNight{false};
    bool autoDayNight{false};
    float timeOfDay{0.3f}; // 0.0 - 1.0 (0.2-0.7 = day, 0.7-0.2 = night)
    bool headlightsActive{false};
    bool enableTreeShear{true};
    float globalTime{0.0f};

    SceneManager() = default;

    void init(const Track& track) {
        // Initialize primitives
        cubeMesh = GeometryGenerator::createCube();
        cylinderMesh = GeometryGenerator::createCylinder(24);
        coneMesh = GeometryGenerator::createCone(24);
        sphereMesh = GeometryGenerator::createSphere(16, 24);
        planeMesh = GeometryGenerator::createPlane();

        // Initialize rustic stone portal
        stonePortalMesh = GeometryGenerator::createStonePortalArch(3.8f, 5.0f, 14.0f, 8.5f, 2.5f, 18, m3d::Vec3(0.48f, 0.46f, 0.42f));

        buildRoadMeshes(track);
        buildCurvedTunnelMeshes(track);
        buildRiverMesh();
        buildSailboatMesh();
        buildBoatHullMesh();
        buildTrees(track);
        buildStreetLamps();
        buildTunnelLights();
        buildBoats();
    }

    void buildRoadMeshes(const Track& track) {
        std::vector<Vertex> roadVertices;
        std::vector<GLuint> roadIndices;

        std::vector<Vertex> stripeVertices;
        std::vector<GLuint> stripeIndices;

        const int numSamples = 600;
        const float roadHalfWidth = 2.8f;
        const float stripeHalfWidth = 0.10f;
        const float doubleGap = 0.22f; // spacing for double yellow lines

        m3d::Vec3 roadColor(0.20f, 0.20f, 0.22f);       // Dark asphalt
        m3d::Vec3 yellowColor(0.96f, 0.88f, 0.15f);     // Vibrant road yellow
        m3d::Vec3 whiteColor(0.96f, 0.96f, 0.96f);      // Crisp highway white
        m3d::Vec3 norm(0.0f, 1.0f, 0.0f);

        for (int i = 0; i <= numSamples; ++i) {
            float dist = (float)i / (float)numSamples * track.totalLength;
            m3d::Vec3 p, fwd, up;
            EnvironmentZone z;
            float dummySpeed;
            track.sample(dist, p, fwd, up, z, dummySpeed);

            m3d::Vec3 r = m3d::cross(fwd, up).normalized();

            // Elevate road comfortably above terrain meadow
            p.y += 0.08f;

            m3d::Vec3 leftEdge = p - r * roadHalfWidth;
            m3d::Vec3 rightEdge = p + r * roadHalfWidth;

            roadVertices.emplace_back(leftEdge, norm, m3d::Vec2(0.0f, (float)i * 0.2f), roadColor);
            roadVertices.emplace_back(rightEdge, norm, m3d::Vec2(1.0f, (float)i * 0.2f), roadColor);
        }

        // Connect road quads with Counter-Clockwise triangles (Right x Forward = +Up)
        for (int i = 0; i < numSamples; ++i) {
            GLuint i0 = i * 2;
            GLuint i1 = i * 2 + 1;
            GLuint i2 = (i + 1) * 2;
            GLuint i3 = (i + 1) * 2 + 1;

            // Quad: i0 (left), i1 (right), i3 (next right), i2 (next left)
            // Triangle 1: i0 -> i1 -> i2
            roadIndices.push_back(i0);
            roadIndices.push_back(i1);
            roadIndices.push_back(i2);

            // Triangle 2: i1 -> i3 -> i2
            roadIndices.push_back(i1);
            roadIndices.push_back(i3);
            roadIndices.push_back(i2);
        }

        // Helper lambda to add a CCW quad for striping
        auto addStripeQuad = [&](const m3d::Vec3& p0, const m3d::Vec3& p1, const m3d::Vec3& p2, const m3d::Vec3& p3, const m3d::Vec3& col) {
            GLuint base = static_cast<GLuint>(stripeVertices.size());
            stripeVertices.emplace_back(p0, norm, m3d::Vec2(0, 0), col);
            stripeVertices.emplace_back(p1, norm, m3d::Vec2(1, 0), col);
            stripeVertices.emplace_back(p2, norm, m3d::Vec2(1, 1), col);
            stripeVertices.emplace_back(p3, norm, m3d::Vec2(0, 1), col);

            stripeIndices.push_back(base + 0);
            stripeIndices.push_back(base + 1);
            stripeIndices.push_back(base + 2);

            stripeIndices.push_back(base + 0);
            stripeIndices.push_back(base + 2);
            stripeIndices.push_back(base + 3);
        };

        // Generate Road Striping based on environment zone
        for (int i = 0; i < numSamples; ++i) {
            float dist0 = (float)i / (float)numSamples * track.totalLength;
            float dist1 = (float)(i + 1) / (float)numSamples * track.totalLength;

            m3d::Vec3 p0, f0, u0, p1, f1, u1;
            EnvironmentZone z0, z1;
            float spd;
            track.sample(dist0, p0, f0, u0, z0, spd);
            track.sample(dist1, p1, f1, u1, z1, spd);

            p0.y += 0.085f;
            p1.y += 0.085f;

            m3d::Vec3 r0 = m3d::cross(f0, u0).normalized();
            m3d::Vec3 r1 = m3d::cross(f1, u1).normalized();

            // Start / Finish Line across the road right under the Garage Gantry at Waypoint 0
            if (i <= 2) {
                float lineWidth = roadHalfWidth - 0.2f;
                m3d::Vec3 sf0_a = p0 - r0 * lineWidth;
                m3d::Vec3 sf0_b = p0 + r0 * lineWidth;
                m3d::Vec3 sf1_b = p1 + r1 * lineWidth;
                m3d::Vec3 sf1_a = p1 - r1 * lineWidth;
                addStripeQuad(sf0_a, sf0_b, sf1_b, sf1_a, whiteColor);
            }

            // Approaching tunnel entrance: Double Yellow Lines (Reference Image 1)
            bool isTunnelApproach = (p0.x < -38.0f && p0.x > -50.0f && p0.z >= -34.0f && p0.z <= -19.0f);
            // Inside tunnel: White dashed center + white shoulder lines (Reference Image 2)
            bool isInsideTunnel = (z0 == EnvironmentZone::TUNNEL && !isTunnelApproach);

            if (isTunnelApproach) {
                // Double solid yellow line (Image 1)
                m3d::Vec3 l0_a = p0 - r0 * (doubleGap + stripeHalfWidth);
                m3d::Vec3 l0_b = p0 - r0 * (doubleGap - stripeHalfWidth);
                m3d::Vec3 l1_b = p1 - r1 * (doubleGap - stripeHalfWidth);
                m3d::Vec3 l1_a = p1 - r1 * (doubleGap + stripeHalfWidth);
                addStripeQuad(l0_a, l0_b, l1_b, l1_a, yellowColor);

                m3d::Vec3 r0_a = p0 + r0 * (doubleGap - stripeHalfWidth);
                m3d::Vec3 r0_b = p0 + r0 * (doubleGap + stripeHalfWidth);
                m3d::Vec3 r1_b = p1 + r1 * (doubleGap + stripeHalfWidth);
                m3d::Vec3 r1_a = p1 + r1 * (doubleGap - stripeHalfWidth);
                addStripeQuad(r0_a, r0_b, r1_b, r1_a, yellowColor);
            } else if (isInsideTunnel) {
                // White dashed center line (Image 2)
                if ((i / 3) % 2 == 0) {
                    m3d::Vec3 c0_a = p0 - r0 * stripeHalfWidth;
                    m3d::Vec3 c0_b = p0 + r0 * stripeHalfWidth;
                    m3d::Vec3 c1_b = p1 + r1 * stripeHalfWidth;
                    m3d::Vec3 c1_a = p1 - r1 * stripeHalfWidth;
                    addStripeQuad(c0_a, c0_b, c1_b, c1_a, whiteColor);
                }

                // Solid white edge lines flanking roadway shoulders (Image 2)
                float edgeOffset = roadHalfWidth - 0.35f;
                m3d::Vec3 le0_a = p0 - r0 * (edgeOffset + 0.08f);
                m3d::Vec3 le0_b = p0 - r0 * (edgeOffset - 0.08f);
                m3d::Vec3 le1_b = p1 - r1 * (edgeOffset - 0.08f);
                m3d::Vec3 le1_a = p1 - r1 * (edgeOffset + 0.08f);
                addStripeQuad(le0_a, le0_b, le1_b, le1_a, whiteColor);

                m3d::Vec3 re0_a = p0 + r0 * (edgeOffset - 0.08f);
                m3d::Vec3 re0_b = p0 + r0 * (edgeOffset + 0.08f);
                m3d::Vec3 re1_b = p1 + r1 * (edgeOffset + 0.08f);
                m3d::Vec3 re1_a = p1 + r1 * (edgeOffset - 0.08f);
                addStripeQuad(re0_a, re0_b, re1_b, re1_a, whiteColor);
            } else {
                // Standard yellow dashed center line
                if ((i / 4) % 2 == 0) {
                    m3d::Vec3 c0_a = p0 - r0 * stripeHalfWidth;
                    m3d::Vec3 c0_b = p0 + r0 * stripeHalfWidth;
                    m3d::Vec3 c1_b = p1 + r1 * stripeHalfWidth;
                    m3d::Vec3 c1_a = p1 - r1 * stripeHalfWidth;
                    addStripeQuad(c0_a, c0_b, c1_b, c1_a, yellowColor);
                }
            }
        }

        roadMesh.setup(roadVertices, roadIndices);
        roadStripesMesh.setup(stripeVertices, stripeIndices);
    }

    void buildCurvedTunnelMeshes(const Track& track) {
        std::vector<Vertex> vaultVerts;
        std::vector<GLuint> vaultIndices;

        std::vector<Vertex> lightVerts;
        std::vector<GLuint> lightIndices;

        std::vector<Vertex> barrierVerts;
        std::vector<GLuint> barrierIndices;

        // Entrance portal: Z = -20.5m, X = -45.0m (5.6m past waypoint 14 approach)
        // Exit portal: Z = 10.0m, X = -20.0m (6.0m past waypoint 18)
        float startDist = track.cumulativeDistances[14] + 5.6f;
        float endDist = track.cumulativeDistances[18] + 6.0f;
        float tunnelLen = endDist - startDist;

        const int numRings = 70;
        const int archSegments = 16;
        const float radius = 3.95f;
        const float wallH = 1.35f;

        m3d::Vec3 vaultCol(0.40f, 0.42f, 0.44f);      // Modern smooth tunnel concrete (Image 2)
        m3d::Vec3 floorCol(0.20f, 0.20f, 0.22f);      // Dark asphalt pavement floor (Image 2)
        m3d::Vec3 lightCol(1.0f, 1.0f, 1.0f);          // Brilliant white LED strip (Image 2)
        m3d::Vec3 barrierCol(0.36f, 0.38f, 0.40f);    // New Jersey safety barrier (Image 2)

        // Build Rings along track curve
        for (int k = 0; k <= numRings; ++k) {
            float frac = (float)k / (float)numRings;
            float d = startDist + frac * tunnelLen;

            m3d::Vec3 p, fwd, up;
            EnvironmentZone z; float spd;
            track.sample(d, p, fwd, up, z, spd);

            m3d::Vec3 r = m3d::cross(fwd, up).normalized();
            m3d::Vec3 nUp = m3d::cross(r, fwd).normalized();

            // 1. Vault Closed Tube Profile (Left base -> Left wall -> Arch -> Right wall -> Right base -> Left base Floor)
            // 0: Left base
            vaultVerts.emplace_back(p - r * radius + nUp * 0.02f, r, m3d::Vec2(0.0f, frac * 12.0f), vaultCol);
            // 1: Left wall top
            vaultVerts.emplace_back(p - r * radius + nUp * wallH, r, m3d::Vec2(0.1f, frac * 12.0f), vaultCol);

            // 2 .. archSegments + 2: Arch Semicircle
            for (int j = 0; j <= archSegments; ++j) {
                float theta = m3d::PI - (float)j / (float)archSegments * m3d::PI;
                float cosT = std::cos(theta);
                float sinT = std::sin(theta);

                m3d::Vec3 pos = p + r * (radius * cosT) + nUp * (wallH + radius * sinT);
                m3d::Vec3 norm = (-r * cosT - nUp * sinT).normalized(); // Inward normal!
                float u = 0.1f + 0.8f * ((float)j / (float)archSegments);

                vaultVerts.emplace_back(pos, norm, m3d::Vec2(u, frac * 12.0f), vaultCol);
            }

            // archSegments + 3: Right wall top
            vaultVerts.emplace_back(p + r * radius + nUp * wallH, -r, m3d::Vec2(0.9f, frac * 12.0f), vaultCol);
            // archSegments + 4: Right base
            vaultVerts.emplace_back(p + r * radius + nUp * 0.04f, -r, m3d::Vec2(1.0f, frac * 12.0f), vaultCol);
            // archSegments + 5: Floor across to Left base (seals ground from outside meadow grass, normal = UP!)
            vaultVerts.emplace_back(p - r * radius + nUp * 0.04f, nUp, m3d::Vec2(0.0f, frac * 12.0f), floorCol);

            // 2. Dual Continuous LED strips along ceiling (Reference Image 2)
            float ledY = wallH + radius * 0.72f;
            float ledHalfW = 0.14f;
            m3d::Vec3 downNorm = -nUp;

            m3d::Vec3 ledL_a = p - r * (1.85f + ledHalfW) + nUp * ledY;
            m3d::Vec3 ledL_b = p - r * (1.85f - ledHalfW) + nUp * ledY;
            m3d::Vec3 ledR_a = p + r * (1.85f - ledHalfW) + nUp * ledY;
            m3d::Vec3 ledR_b = p + r * (1.85f + ledHalfW) + nUp * ledY;

            lightVerts.emplace_back(ledL_a, downNorm, m3d::Vec2(0, frac * 20.0f), lightCol);
            lightVerts.emplace_back(ledL_b, downNorm, m3d::Vec2(1, frac * 20.0f), lightCol);
            lightVerts.emplace_back(ledR_a, downNorm, m3d::Vec2(0, frac * 20.0f), lightCol);
            lightVerts.emplace_back(ledR_b, downNorm, m3d::Vec2(1, frac * 20.0f), lightCol);

            // 3. Side Barrier Curbs (New Jersey barriers, 8 points per ring)
            // Left Barrier: b0 outer base, b1 outer top, b2 curb top, b3 road toe
            barrierVerts.emplace_back(p - r * 3.40f + nUp * 0.04f, r, m3d::Vec2(0, frac * 10.0f), barrierCol);
            barrierVerts.emplace_back(p - r * 3.40f + nUp * 0.70f, r, m3d::Vec2(0, frac * 10.0f), barrierCol);
            barrierVerts.emplace_back(p - r * 2.95f + nUp * 0.70f, nUp, m3d::Vec2(1, frac * 10.0f), barrierCol);
            barrierVerts.emplace_back(p - r * 2.75f + nUp * 0.04f, r, m3d::Vec2(1, frac * 10.0f), barrierCol);

            // Right Barrier: b4 road toe, b5 curb top, b6 outer top, b7 outer base
            barrierVerts.emplace_back(p + r * 2.75f + nUp * 0.04f, -r, m3d::Vec2(1, frac * 10.0f), barrierCol);
            barrierVerts.emplace_back(p + r * 2.95f + nUp * 0.70f, nUp, m3d::Vec2(1, frac * 10.0f), barrierCol);
            barrierVerts.emplace_back(p + r * 3.40f + nUp * 0.70f, -r, m3d::Vec2(0, frac * 10.0f), barrierCol);
            barrierVerts.emplace_back(p + r * 3.40f + nUp * 0.04f, -r, m3d::Vec2(0, frac * 10.0f), barrierCol);
        }

        // Connect Vault rings with triangles
        int pointsPerRing = archSegments + 6;
        for (int k = 0; k < numRings; ++k) {
            int r0 = k * pointsPerRing;
            int r1 = (k + 1) * pointsPerRing;

            for (int p = 0; p < pointsPerRing - 1; ++p) {
                GLuint i0 = r0 + p;
                GLuint i1 = r0 + p + 1;
                GLuint i2 = r1 + p;
                GLuint i3 = r1 + p + 1;

                vaultIndices.push_back(i0); vaultIndices.push_back(i2); vaultIndices.push_back(i1);
                vaultIndices.push_back(i1); vaultIndices.push_back(i2); vaultIndices.push_back(i3);
                vaultIndices.push_back(i0); vaultIndices.push_back(i1); vaultIndices.push_back(i2);
                vaultIndices.push_back(i1); vaultIndices.push_back(i3); vaultIndices.push_back(i2);
            }
        }

        // Connect LED light strips
        for (int k = 0; k < numRings; ++k) {
            int b0 = k * 4;
            int b1 = (k + 1) * 4;

            // Left strip
            lightIndices.push_back(b0 + 0); lightIndices.push_back(b1 + 0); lightIndices.push_back(b0 + 1);
            lightIndices.push_back(b0 + 1); lightIndices.push_back(b1 + 0); lightIndices.push_back(b1 + 1);
            lightIndices.push_back(b0 + 0); lightIndices.push_back(b0 + 1); lightIndices.push_back(b1 + 0);
            lightIndices.push_back(b0 + 1); lightIndices.push_back(b1 + 1); lightIndices.push_back(b1 + 0);

            // Right strip
            lightIndices.push_back(b0 + 2); lightIndices.push_back(b1 + 2); lightIndices.push_back(b0 + 3);
            lightIndices.push_back(b0 + 3); lightIndices.push_back(b1 + 2); lightIndices.push_back(b1 + 3);
            lightIndices.push_back(b0 + 2); lightIndices.push_back(b0 + 3); lightIndices.push_back(b1 + 2);
            lightIndices.push_back(b0 + 3); lightIndices.push_back(b1 + 3); lightIndices.push_back(b1 + 2);
        }

        // Connect Barriers (8 vertices per ring: 3 quads for Left, 3 quads for Right)
        for (int k = 0; k < numRings; ++k) {
            int b0 = k * 8;
            int b1 = (k + 1) * 8;

            // Left barrier 3 segments (0->1 outer face, 1->2 top curb, 2->3 road slope)
            for (int q = 0; q < 3; ++q) {
                barrierIndices.push_back(b0 + q); barrierIndices.push_back(b1 + q); barrierIndices.push_back(b0 + q + 1);
                barrierIndices.push_back(b0 + q + 1); barrierIndices.push_back(b1 + q); barrierIndices.push_back(b1 + q + 1);
                barrierIndices.push_back(b0 + q); barrierIndices.push_back(b0 + q + 1); barrierIndices.push_back(b1 + q);
                barrierIndices.push_back(b0 + q + 1); barrierIndices.push_back(b1 + q + 1); barrierIndices.push_back(b1 + q);
            }
            // Right barrier 3 segments (4->5 road slope, 5->6 top curb, 6->7 outer face)
            for (int q = 4; q < 7; ++q) {
                barrierIndices.push_back(b0 + q); barrierIndices.push_back(b1 + q); barrierIndices.push_back(b0 + q + 1);
                barrierIndices.push_back(b0 + q + 1); barrierIndices.push_back(b1 + q); barrierIndices.push_back(b1 + q + 1);
                barrierIndices.push_back(b0 + q); barrierIndices.push_back(b0 + q + 1); barrierIndices.push_back(b1 + q);
                barrierIndices.push_back(b0 + q + 1); barrierIndices.push_back(b1 + q + 1); barrierIndices.push_back(b1 + q);
            }
        }

        tunnelVaultMesh.setup(vaultVerts, vaultIndices);
        tunnelCeilingLightMesh.setup(lightVerts, lightIndices);
        tunnelBarriersMesh.setup(barrierVerts, barrierIndices);
    }

    void buildSailboatMesh() {
        std::vector<Vertex> verts;
        std::vector<GLuint> indices;

        m3d::Vec3 whiteSail(0.98f, 0.98f, 0.96f);
        m3d::Vec3 redSail(0.92f, 0.18f, 0.15f);

        auto addDoubleTri = [&](const m3d::Vec3& p0, const m3d::Vec3& p1, const m3d::Vec3& p2, const m3d::Vec3& col) {
            m3d::Vec3 n = m3d::cross(p1 - p0, p2 - p0).normalized();
            GLuint b = static_cast<GLuint>(verts.size());
            verts.emplace_back(p0, n, m3d::Vec2(0, 0), col);
            verts.emplace_back(p1, n, m3d::Vec2(1, 0), col);
            verts.emplace_back(p2, n, m3d::Vec2(0.5f, 1.0f), col);
            indices.push_back(b + 0); indices.push_back(b + 1); indices.push_back(b + 2);
            indices.push_back(b + 0); indices.push_back(b + 2); indices.push_back(b + 1);
        };

        // 1. Triangular Mainsail (White, behind mast at X = 0.38, river clearance rig top at Y = 1.65m)
        m3d::Vec3 m_head(0.38f, 1.65f, 0.0f);
        m3d::Vec3 m_tack(0.38f, 0.05f, 0.0f);
        m3d::Vec3 m_clew(-1.8f, 0.05f, 0.0f);
        m3d::Vec3 m_midBelly(-0.7f, 0.85f, 0.12f); // billowing camber
        m3d::Vec3 m_midLuff(0.38f, 0.85f, 0.03f);
        m3d::Vec3 m_midFoot(-0.7f, 0.05f, 0.04f);

        addDoubleTri(m_head, m_midLuff, m_midBelly, whiteSail);
        addDoubleTri(m_midLuff, m_tack, m_midFoot, whiteSail);
        addDoubleTri(m_midLuff, m_midFoot, m_midBelly, whiteSail);
        addDoubleTri(m_midBelly, m_midFoot, m_clew, whiteSail);

        // 2. Triangular Jib (Red, forward from mast at X = 0.38 to bow at X = 2.0, top at Y = 1.35m)
        m3d::Vec3 j_head(0.38f, 1.35f, 0.0f);
        m3d::Vec3 j_tack(2.0f, -0.05f, 0.0f);
        m3d::Vec3 j_clew(0.55f, 0.08f, 0.0f);
        m3d::Vec3 j_midLuff(1.20f, 0.65f, 0.04f);
        m3d::Vec3 j_midBelly(1.00f, 0.55f, 0.10f); // billowing camber
        m3d::Vec3 j_midFoot(1.30f, 0.02f, 0.03f);

        addDoubleTri(j_head, j_midBelly, j_midLuff, redSail);
        addDoubleTri(j_midLuff, j_midFoot, j_tack, redSail);
        addDoubleTri(j_midLuff, j_midBelly, j_midFoot, redSail);
        addDoubleTri(j_midBelly, j_clew, j_midFoot, redSail);

        sailboatSailMesh.setup(verts, indices);
    }

    void buildBoatHullMesh() {
        // Unit boat hull: Length X in [-0.5, +0.5], Width Z in [-0.5, +0.5], Height Y in [0, 1]
        // Bow point at X = +0.5, flat transom stern at X = -0.5
        std::vector<Vertex> verts;
        std::vector<GLuint> indices;
        m3d::Vec3 white(1.0f, 1.0f, 1.0f);

        // Deck vertices (Y = 1.0)
        m3d::Vec3 d_bow( 0.50f, 1.0f,  0.0f);
        m3d::Vec3 d_fwdR( 0.20f, 1.0f, -0.5f);
        m3d::Vec3 d_fwdL( 0.20f, 1.0f,  0.5f);
        m3d::Vec3 d_aftR(-0.50f, 1.0f, -0.5f);
        m3d::Vec3 d_aftL(-0.50f, 1.0f,  0.5f);

        // Keel / Bottom vertices (Y = 0.0)
        m3d::Vec3 k_bow( 0.45f, 0.0f,  0.0f);
        m3d::Vec3 k_fwdR( 0.18f, 0.0f, -0.38f);
        m3d::Vec3 k_fwdL( 0.18f, 0.0f,  0.38f);
        m3d::Vec3 k_aftR(-0.50f, 0.0f, -0.38f);
        m3d::Vec3 k_aftL(-0.50f, 0.0f,  0.38f);

        auto addQuad = [&](const m3d::Vec3& p0, const m3d::Vec3& p1, const m3d::Vec3& p2, const m3d::Vec3& p3) {
            m3d::Vec3 n = m3d::cross(p1 - p0, p2 - p0).normalized();
            GLuint b = static_cast<GLuint>(verts.size());
            verts.emplace_back(p0, n, m3d::Vec2(0, 0), white);
            verts.emplace_back(p1, n, m3d::Vec2(1, 0), white);
            verts.emplace_back(p2, n, m3d::Vec2(1, 1), white);
            verts.emplace_back(p3, n, m3d::Vec2(0, 1), white);
            indices.push_back(b + 0); indices.push_back(b + 1); indices.push_back(b + 2);
            indices.push_back(b + 0); indices.push_back(b + 2); indices.push_back(b + 3);
        };

        auto addTri = [&](const m3d::Vec3& p0, const m3d::Vec3& p1, const m3d::Vec3& p2) {
            m3d::Vec3 n = m3d::cross(p1 - p0, p2 - p0).normalized();
            GLuint b = static_cast<GLuint>(verts.size());
            verts.emplace_back(p0, n, m3d::Vec2(0, 0), white);
            verts.emplace_back(p1, n, m3d::Vec2(1, 0), white);
            verts.emplace_back(p2, n, m3d::Vec2(0.5f, 1), white);
            indices.push_back(b + 0); indices.push_back(b + 1); indices.push_back(b + 2);
        };

        // Starboard Bow Flank
        addQuad(d_bow, k_bow, k_fwdR, d_fwdR);
        // Port Bow Flank
        addQuad(d_fwdL, k_fwdL, k_bow, d_bow);
        // Starboard Mid-to-Aft
        addQuad(d_fwdR, k_fwdR, k_aftR, d_aftR);
        // Port Mid-to-Aft
        addQuad(d_aftL, k_aftL, k_fwdL, d_fwdL);
        // Transom Stern
        addQuad(d_aftR, k_aftR, k_aftL, d_aftL);
        // Bottom Keel
        addTri(k_bow, k_fwdR, k_fwdL);
        addQuad(k_fwdR, k_aftR, k_aftL, k_fwdL);
        // Deck Top
        addTri(d_bow, d_fwdL, d_fwdR);
        addQuad(d_fwdL, d_aftL, d_aftR, d_fwdR);

        boatHullMesh.setup(verts, indices);
    }

    void buildRiverMesh() {
        // Deep recessed river channel beneath bridge at Y = -1.20m (1.2m below road/terrain)
        // Spans from X = -36.0m (western bank behind bridge) to X = +75.0m (eastern valley)
        std::vector<Vertex> riverVerts;
        std::vector<GLuint> riverIndices;

        m3d::Vec3 waterCol(0.12f, 0.45f, 0.72f);
        m3d::Vec3 norm(0.0f, 1.0f, 0.0f);

        float riverY = -1.20f;
        float riverZCenter = -46.0f;
        float riverHalfWidthZ = 11.5f; // River width = 23m (strictly Z in [-57.5, -34.5])
        float riverXMin = -36.0f;
        float riverXMax = 20.0f;

        riverVerts.emplace_back(m3d::Vec3(riverXMin, riverY, riverZCenter - riverHalfWidthZ), norm, m3d::Vec2(0, 0), waterCol);
        riverVerts.emplace_back(m3d::Vec3(riverXMax, riverY, riverZCenter - riverHalfWidthZ), norm, m3d::Vec2(10, 0), waterCol);
        riverVerts.emplace_back(m3d::Vec3(riverXMax, riverY, riverZCenter + riverHalfWidthZ), norm, m3d::Vec2(10, 3), waterCol);
        riverVerts.emplace_back(m3d::Vec3(riverXMin, riverY, riverZCenter + riverHalfWidthZ), norm, m3d::Vec2(0, 3), waterCol);

        // Counter-Clockwise indices for upward visibility
        riverIndices = { 0, 2, 1, 0, 3, 2 };
        riverMesh.setup(riverVerts, riverIndices);
    }

    void buildTrees(const Track& track) {
        trees.clear();

        // Strict clearance validator: zero trees allowed near road, river, city buildings or mountain
        auto isValidTreePos = [&](float tx, float tz) -> bool {
            // River valley exclusion strictly under bridge (river Z is -58 to -34, X in [-38, +22])
            if (tz >= -61.0f && tz <= -31.0f && tx >= -38.0f && tx <= 22.0f) return false;
            // Garage apron exclusion
            if (tx < -44.0f && tz > 22.0f) return false;
            // City center building block exclusion
            if (tx >= -25.0f && tx <= 32.0f && tz >= 22.0f && tz <= 52.0f) return false;
            // Mountain tunnel corridor exclusion
            if (tx >= -62.0f && tx <= -16.0f && tz >= -25.0f && tz <= 14.0f) return false;

            // Strict distance check to entire track spline
            for (float s = 0.0f; s < track.totalLength; s += 1.5f) {
                m3d::Vec3 p, f, u; EnvironmentZone z; float spd;
                track.sample(s, p, f, u, z, spd);
                float dx = tx - p.x;
                float dz = tz - p.z;
                if (dx * dx + dz * dz < 7.0f * 7.0f) { // 7.0m clearance from road center!
                    return false;
                }
            }
            return true;
        };

        // 1. Broad outer ring of countryside forest
        for (int i = 0; i < 90; ++i) {
            float angle = (float)i / 90.0f * m3d::TWO_PI;
            float radius = 56.0f + 14.0f * std::sin(angle * 4.0f);
            float tx = radius * std::cos(angle) + 4.0f;
            float tz = radius * std::sin(angle) - 26.0f;

            if (!isValidTreePos(tx, tz)) continue;

            float scale = 0.85f + 0.35f * std::sin(i * 1.7f);
            float phase = i * 0.45f;
            m3d::Vec3 col = (i % 2 == 0) ? m3d::Vec3(0.18f, 0.45f, 0.22f) : m3d::Vec3(0.12f, 0.38f, 0.16f);
            trees.push_back({ {tx, 0.0f, tz}, scale, phase, col });
        }

        // 2. Nature corridor clusters (validated with distance check)
        float candidatePoints[][2] = {
            { 12.0f, 15.0f }, { 18.0f, 6.0f }, { 32.0f, 8.0f }, { 38.0f, -8.0f },
            { 52.0f, -18.0f }, { 54.0f, -36.0f }, { 48.0f, -55.0f }, { 36.0f, -75.0f },
            { 20.0f, -76.0f }, { -4.0f, -78.0f }, { -24.0f, -76.0f }, { -44.0f, -68.0f },
            { -62.0f, -54.0f }, { -64.0f, -25.0f }, { -62.0f, -6.0f }, { -35.0f, 22.0f }
        };
        int numCandidates = sizeof(candidatePoints) / sizeof(candidatePoints[0]);

        for (int i = 0; i < numCandidates; ++i) {
            float offsets[][2] = { {6.0f, 6.0f}, {-6.0f, -6.0f}, {8.0f, -6.0f}, {-8.0f, 6.0f} };
            for (int k = 0; k < 4; ++k) {
                float tx = candidatePoints[i][0] + offsets[k][0];
                float tz = candidatePoints[i][1] + offsets[k][1];
                if (isValidTreePos(tx, tz)) {
                    float scale = 0.85f + 0.3f * std::sin(i * 2.1f + k);
                    float phase = (i * 4 + k) * 0.4f;
                    m3d::Vec3 col = (k % 2 == 0) ? m3d::Vec3(0.16f, 0.44f, 0.20f) : m3d::Vec3(0.11f, 0.36f, 0.15f);
                    trees.push_back({ {tx, 0.0f, tz}, scale, phase, col });
                }
            }
        }
    }

    void buildStreetLamps() {
        streetLamps.clear();
        // City street lamps (on sidewalk along Z = 34.5f)
        streetLamps.push_back({ {-15.0f, 0.0f, 34.5f}, 0.0f, false });
        streetLamps.push_back({ {-5.0f,  0.0f, 34.5f}, 0.0f, false });
        streetLamps.push_back({ { 8.0f,  0.0f, 30.5f}, 0.5f, false });
        streetLamps.push_back({ { 26.5f, 0.0f, 18.0f}, 1.57f, false });
        streetLamps.push_back({ { 26.5f, 0.0f,  2.0f}, 1.57f, false });

        // Countryside junction lamps
        streetLamps.push_back({ { 12.0f, 0.0f,  12.0f}, 0.0f, false });
        streetLamps.push_back({ { 34.0f, 0.0f, -2.5f}, 0.0f, false });
        streetLamps.push_back({ {-48.0f, 0.0f, 34.0f}, 0.0f, false }); // Near Garage entrance
    }

    void buildTunnelLights() {
        tunnelCeilingLights.clear();
        // Warm golden glow at tunnel entrance mouth (Reference Image 1)
        tunnelCeilingLights.push_back({ {-45.0f, 3.2f, -18.0f}, {1.0f, 0.78f, 0.35f}, 0.75f, 15.0f });

        // Dual rows of LED fixtures along upper curved ceiling of tunnel (matching Reference Image 2)
        for (int i = 1; i <= 6; ++i) {
            float frac = (float)i / 7.0f;
            float tz = -20.5f * (1.0f - frac) + 10.0f * frac;
            float tx = (tz < -8.0f) ? -45.0f : (-45.0f + 25.0f * (frac - 0.35f) / 0.65f);
            tunnelCeilingLights.push_back({ {tx, 4.0f, tz}, {0.92f, 0.96f, 1.0f}, 0.80f, 14.0f });
        }
    }

    void buildBoats() {
        boats.clear();
        // Boat 1: Luxury Cabin Cruiser sailing East on deep river at Y = -1.20m
        boats.push_back({ { 8.0f, -1.20f, -46.5f}, 2.4f, 0.0f, {0.95f, 0.95f, 0.98f}, 6.5f, BoatType::CABIN_CRUISER });
        // Boat 2: Classic Sailboat with low-profile river rig sailing West under main arch at Y = -1.20m
        boats.push_back({ {-6.0f, -1.20f, -45.5f}, -1.6f, m3d::PI, {0.88f, 0.22f, 0.20f}, 5.5f, BoatType::SAILBOAT });
        // Boat 3: River Patrol / Tug boat cruising near the west bank at Y = -1.20m
        boats.push_back({ {-22.0f, -1.20f, -46.0f}, 1.8f, 0.05f, {0.18f, 0.48f, 0.75f}, 4.5f, BoatType::RIVER_PATROL });
    }

    void update(float dt, const Track& track) {
        globalTime += dt;

        // Auto Day/Night cycle
        if (autoDayNight) {
            timeOfDay += dt * 0.02f; // full cycle every 50 seconds
            if (timeOfDay >= 1.0f) timeOfDay -= 1.0f;
            isNight = (timeOfDay < 0.25f || timeOfDay > 0.75f);
        }

        // Headlights automatic activation (at night OR inside mountain tunnel)
        headlightsActive = isNight || (currentZone == EnvironmentZone::TUNNEL);

        // Update car navigation physics along track
        m3d::Vec3 nextP, nextFwd, nextUp;
        EnvironmentZone targetZone;
        float targetSpeed;
        track.sample(carDistance, carPos, carForward, carUp, targetZone, targetSpeed);
        currentZone = targetZone;

        if (!isJourneyComplete) {
            // Smooth acceleration / deceleration toward target zone speed
            float speedRate = 4.5f; // m/s^2

            // If nearing completion of the journey (last 16 meters before finish line at garage):
            // decelerate smoothly down to zero for final parking stop!
            float distToEnd = track.totalLength - carDistance;
            if (distToEnd < 16.0f) {
                targetSpeed = std::max(0.0f, (distToEnd / 16.0f) * 4.5f);
            }

            if (carSpeed < targetSpeed) {
                carSpeed = std::min(targetSpeed, carSpeed + speedRate * dt);
            } else if (carSpeed > targetSpeed) {
                carSpeed = std::max(targetSpeed, carSpeed - speedRate * 1.5f * dt);
            }

            // Distance advance
            float deltaDist = carSpeed * dt;
            carDistance += deltaDist;

            // When reaching or completing full journey path at Garage:
            if (carDistance >= track.totalLength) {
                carDistance = track.totalLength;
                carSpeed = 0.0f;
                wheelSteerAngle = 0.0f;
                isJourneyComplete = true;
            }

            // Wheel spin rotation proportional to travel distance (radius = 0.38m)
            float wheelRadius = 0.38f;
            wheelSpinAngle += (deltaDist / wheelRadius) * m3d::RAD2DEG;
            if (wheelSpinAngle > 360.0f) wheelSpinAngle -= 360.0f;
        } else {
            // Car parked / stopped at Garage
            carSpeed = 0.0f;
            wheelSteerAngle = 0.0f;
        }

        // Car orthonormal basis
        carRight = m3d::cross(carForward, carUp).normalized();
        carUp = m3d::cross(carRight, carForward).normalized();

        // Calculate curvature for wheel steering angle
        if (!isJourneyComplete) {
            m3d::Vec3 futureP, futureFwd, futureUp;
            EnvironmentZone fz; float fs;
            track.sample(carDistance + 3.0f, futureP, futureFwd, futureUp, fz, fs);
            float turnDot = m3d::dot(carRight, futureFwd);
            wheelSteerAngle = m3d::clamp(turnDot * 35.0f, -30.0f, 30.0f); // degrees
        } else {
            wheelSteerAngle = 0.0f;
        }

        // Update boats movement strictly under the Grand River Bridge
        for (auto& boat : boats) {
            boat.basePosition.x += boat.speed * dt;
            // Wrap boat bounds in river channel strictly under bridge (between X = -28.0m and +12.0m)
            if (boat.basePosition.x > 12.0f) boat.basePosition.x = -28.0f;
            if (boat.basePosition.x < -28.0f) boat.basePosition.x = 12.0f;
        }
    }

    // Set uniform lighting data in shader
    void setupLighting(const Shader& shader, const m3d::Vec3& camPos) const {
        shader.use();
        shader.setVec3("viewPos", camPos);

        // Sun / Moon Directional Light
        m3d::Vec3 dirLightColor;
        m3d::Vec3 dirLightDir;
        m3d::Vec3 ambientColor;

        if (!isNight) {
            // Daytime: warm bright golden sun angled towards +Z to bathe tunnel portal and bridge in direct sunlight
            dirLightDir = m3d::Vec3(-0.35f, -0.70f, 0.60f).normalized();
            dirLightColor = m3d::Vec3(1.05f, 0.98f, 0.90f) * 1.15f;
            ambientColor = m3d::Vec3(0.42f, 0.44f, 0.48f);
        } else {
            // Nighttime: cool dark moonlight
            dirLightDir = m3d::Vec3(0.3f, -0.7f, 0.5f).normalized();
            dirLightColor = m3d::Vec3(0.18f, 0.22f, 0.35f) * 0.6f;
            ambientColor = m3d::Vec3(0.08f, 0.09f, 0.14f);
        }

        shader.setVec3("dirLight.direction", dirLightDir);
        shader.setVec3("dirLight.color", dirLightColor);
        shader.setVec3("ambientGlobal", ambientColor);

        // Point lights (Active Street Lamps + Active Tunnel Lights)
        int pointLightIdx = 0;
        const int MAX_POINT_LIGHTS = 16;

        // If night, street lamps light up
        if (isNight) {
            for (const auto& lamp : streetLamps) {
                if (pointLightIdx >= MAX_POINT_LIGHTS) break;
                std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";
                m3d::Vec3 lampLightPos = lamp.position + m3d::Vec3(0.0f, 5.0f, 0.0f);
                shader.setVec3(prefix + "position", lampLightPos);
                // Warm incandescent amber street light glow
                shader.setVec3(prefix + "color", m3d::Vec3(1.0f, 0.82f, 0.55f));
                shader.setFloat(prefix + "intensity", 1.8f);
                shader.setFloat(prefix + "radius", 18.0f);
                pointLightIdx++;
            }
        }

        // Tunnel lights: highway tunnels are always illuminated (day & night)
        bool tunnelLightsOn = true;
        if (tunnelLightsOn) {
            for (const auto& tLight : tunnelCeilingLights) {
                if (pointLightIdx >= MAX_POINT_LIGHTS) break;
                std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";
                shader.setVec3(prefix + "position", tLight.position);
                shader.setVec3(prefix + "color", tLight.color);
                shader.setFloat(prefix + "intensity", tLight.intensity);
                shader.setFloat(prefix + "radius", tLight.range);
                pointLightIdx++;
            }
        }
        shader.setInt("numPointLights", pointLightIdx);

        // Spotlights: Car Twin Headlights
        if (headlightsActive) {
            shader.setBool("spotlightsActive", true);
            m3d::Vec3 leftHeadlightPos = carPos + carForward * 1.9f - carRight * 0.65f + carUp * 0.45f;
            m3d::Vec3 rightHeadlightPos = carPos + carForward * 1.9f + carRight * 0.65f + carUp * 0.45f;

            // Spotlights slightly angled down toward road surface
            m3d::Vec3 spotDir = (carForward - carUp * 0.12f).normalized();

            shader.setVec3("leftSpotlight.position", leftHeadlightPos);
            shader.setVec3("leftSpotlight.direction", spotDir);
            shader.setVec3("leftSpotlight.color", m3d::Vec3(1.0f, 0.98f, 0.92f) * 2.5f);
            shader.setFloat("leftSpotlight.cutOff", std::cos(m3d::radians(22.0f)));
            shader.setFloat("leftSpotlight.outerCutOff", std::cos(m3d::radians(30.0f)));

            shader.setVec3("rightSpotlight.position", rightHeadlightPos);
            shader.setVec3("rightSpotlight.direction", spotDir);
            shader.setVec3("rightSpotlight.color", m3d::Vec3(1.0f, 0.98f, 0.92f) * 2.5f);
            shader.setFloat("rightSpotlight.cutOff", std::cos(m3d::radians(22.0f)));
            shader.setFloat("rightSpotlight.outerCutOff", std::cos(m3d::radians(30.0f)));
        } else {
            shader.setBool("spotlightsActive", false);
        }
    }
};

#endif // SCENE_HPP
