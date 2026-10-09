#pragma once
#ifndef SCENE_HPP
#define SCENE_HPP

#include "math3d.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "track.hpp"
#include "texture.hpp"
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

// 3D Collectible Coin
struct Coin {
    m3d::Vec3 position;
    float baseHeight{0.0f};
    float laneOffset{0.0f};
    float rotationAngle{0.0f};
    bool isCollected{false};
    float respawnTimer{0.0f};
    float collectAnimTimer{0.0f};
    int id{0};
};

// Roadside Gas & Fuel Station
struct GasStation {
    m3d::Vec3 position;       // Canopy / lot center
    float rotationY{0.0f};    // Facing angle in radians
    std::string name;
    m3d::Vec3 pumpBayCenter;  // Where vehicle pulls up to refuel
    float refuelRadius{7.0f};
};

// Milestone Checkpoint Gate
struct Milestone {
    m3d::Vec3 position;
    m3d::Vec3 forward;
    float trackDistance{0.0f};
    std::string name;
    int index{1};
    bool isReached{false};
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

    // Central Texture Manager
    TextureManager textures;

    // Environmental entities
    std::vector<TreeInstance> trees;
    std::vector<StreetLamp> streetLamps;
    std::vector<LightSource> tunnelCeilingLights;
    std::vector<Boat> boats;

    // Game systems & Collectibles
    int coinsCollected{3};           // Starts with 3 seed coins for refueling
    float fuel{100.0f};              // 0.0 - 100.0 %
    const float maxFuel{100.0f};
    bool isNearGasStation{false};
    int activeStationIdx{-1};
    std::string activeStationName{""};
    std::string notificationMessage{""};
    float notificationTimer{0.0f};
    int currentMilestoneIdx{0};
    const int totalMilestones{5};
    int lapsCompleted{0};

    // Obstacle & World Border Collision State
    bool isCollidingWithObstacle{false};
    std::string activeObstacleName{""};
    float collisionAlertTimer{0.0f};

    std::vector<Coin> coins;
    std::vector<GasStation> gasStations;
    std::vector<Milestone> milestones;

    // Car state
    m3d::Vec3 carPos{-48.0f, 0.0f, 30.0f};
    m3d::Vec3 carForward{1.0f, 0.0f, 0.0f};
    m3d::Vec3 carUp{0.0f, 1.0f, 0.0f};
    m3d::Vec3 carRight{0.0f, 0.0f, 1.0f};
    float carYaw{0.0f};          // Heading angle in radians (0 = facing East +X)
    float carPitch{0.0f};
    float carDistance{0.0f};
    float carSpeed{0.0f};
    float wheelSpinAngle{0.0f};
    float wheelSteerAngle{0.0f};
    EnvironmentZone currentZone{EnvironmentZone::GARAGE};
    bool isJourneyComplete{false};
    bool isManualDrive{true};    // User manual control by default
    bool isBraking{false};
    bool isReversing{false};

    // Environmental state
    bool isNight{false};
    bool autoDayNight{false};
    float timeOfDay{0.3f}; // 0.0 - 1.0 (0.2-0.7 = day, 0.7-0.2 = night)
    bool headlightsActive{false};
    bool enableTreeShear{true};
    float globalTime{0.0f};

    // Point lights toggleable via [L] key
    bool pointLightsActive{true};

    SceneManager() = default;

    void triggerNotification(const std::string& msg, float duration = 2.5f) {
        notificationMessage = msg;
        notificationTimer = duration;
    }

    void togglePointLights() {
        pointLightsActive = !pointLightsActive;
        if (pointLightsActive) {
            triggerNotification("POINT LIGHTS: ON (15 ACTIVE FIXTURES)", 2.2f);
        } else {
            triggerNotification("POINT LIGHTS: OFF", 2.2f);
        }
    }

    // Refuel car at Gas Station using collected coins
    bool refuel() {
        if (!isNearGasStation) {
            triggerNotification("NOT AT GAS STATION! PULL INTO PUMP BAY", 2.2f);
            return false;
        }
        if (fuel >= 99.5f) {
            fuel = 100.0f;
            triggerNotification("FUEL TANK ALREADY FULL (100%)", 2.0f);
            return false;
        }
        if (coinsCollected < 1) {
            triggerNotification("NO COINS! COLLECT ROAD COINS TO REFUEL", 2.5f);
            return false;
        }

        // Spend 1 coin for +30% fuel
        coinsCollected -= 1;
        fuel = std::min(maxFuel, fuel + 30.0f);
        triggerNotification("REFUELED +30%! [FUEL: " + std::to_string(static_cast<int>(fuel)) + "%]", 2.5f);
        return true;
    }

    struct RoadSurfaceInfo {
        m3d::Vec3 splinePoint;
        float distToCenter{0.0f};
        float elevation{0.0f};
        float trackDistance{0.0f};
        EnvironmentZone zone{EnvironmentZone::GARAGE};
        bool isOnRoad{false}; // True if within road width/shoulder
    };

    RoadSurfaceInfo getRoadInfoAt(float x, float z, const Track& track) const {
        RoadSurfaceInfo info;
        int numWaypoints = static_cast<int>(track.waypoints.size());
        if (numWaypoints == 0) return info;

        // 1. High-density coarse search over spline segments (8 sub-samples per waypoint segment)
        float bestT = 0.0f;
        float minDistSq = 1e9f;
        const int samplesPerSeg = 8;
        const int totalSamples = numWaypoints * samplesPerSeg;

        for (int s = 0; s < totalSamples; ++s) {
            float t = static_cast<float>(s) / static_cast<float>(samplesPerSeg);
            m3d::Vec3 p = track.evaluateSpline(t);
            float dx = x - p.x;
            float dz = z - p.z;
            float d2 = dx * dx + dz * dz;
            if (d2 < minDistSq) {
                minDistSq = d2;
                bestT = t;
            }
        }

        // 2. High-precision Golden Section local refinement
        float tA = bestT - 0.25f;
        float tB = bestT + 0.25f;
        for (int iter = 0; iter < 8; ++iter) {
            float t1 = tA + (tB - tA) * 0.382f;
            float t2 = tA + (tB - tA) * 0.618f;

            m3d::Vec3 p1 = track.evaluateSpline(t1);
            m3d::Vec3 p2 = track.evaluateSpline(t2);

            float d1 = (x - p1.x) * (x - p1.x) + (z - p1.z) * (z - p1.z);
            float d2 = (x - p2.x) * (x - p2.x) + (z - p2.z) * (z - p2.z);

            if (d1 < d2) {
                tB = t2;
            } else {
                tA = t1;
            }
        }
        float optT = (tA + tB) * 0.5f;

        // 3. Exact point on the spline centerline & continuous track distance
        info.splinePoint = track.evaluateSpline(optT);
        info.trackDistance = track.tToDistance(optT);
        info.distToCenter = std::sqrt((x - info.splinePoint.x) * (x - info.splinePoint.x) +
                                      (z - info.splinePoint.z) * (z - info.splinePoint.z));

        // 4. Zone
        int segIdx = static_cast<int>(std::floor(optT));
        while (segIdx < 0) segIdx += numWaypoints;
        segIdx %= numWaypoints;
        info.zone = track.waypoints[segIdx].zone;

        // 5. Road half-width is 3.6m. With 4.2m margin, car anywhere on road/curb is safely on road!
        info.isOnRoad = (info.distToCenter <= 4.2f);

        // 6. Smooth continuous elevation
        float roadElev = info.splinePoint.y;
        if (roadElev > 0.02f) {
            if (info.distToCenter <= 4.2f) {
                info.elevation = roadElev;
            } else if (info.distToCenter < 7.0f) {
                float blend = (7.0f - info.distToCenter) / 2.8f;
                info.elevation = roadElev * (blend * blend * (3.0f - 2.0f * blend));
            } else {
                info.elevation = 0.0f;
            }
        } else {
            info.elevation = 0.0f;
        }

        return info;
    }

    // Continuously and smoothly sample road surface elevation and active zone
    float getRoadHeightAt(float x, float z, const Track& track, EnvironmentZone& outZone) const {
        RoadSurfaceInfo info = getRoadInfoAt(x, z, track);
        outZone = info.zone;
        return info.elevation;
    }

    // Helper: Push circle (x, z, r) out of AABB box [minX, maxX] x [minZ, maxZ]
    static bool pushOutOfBox(float& x, float& z, float minBx, float maxBx, float minBz, float maxBz, float r) {
        float cx = m3d::clamp(x, minBx, maxBx);
        float cz = m3d::clamp(z, minBz, maxBz);
        float dx = x - cx;
        float dz = z - cz;
        float d2 = dx * dx + dz * dz;
        if (d2 < r * r) {
            if (d2 > 0.0001f) {
                float d = std::sqrt(d2);
                float push = r - d;
                x += (dx / d) * push;
                z += (dz / d) * push;
            } else {
                float leftDist = std::abs(x - minBx);
                float rightDist = std::abs(maxBx - x);
                float botDist = std::abs(z - minBz);
                float topDist = std::abs(maxBz - z);
                float minVal = std::min({leftDist, rightDist, botDist, topDist});
                if (minVal == leftDist) x = minBx - r;
                else if (minVal == rightDist) x = maxBx + r;
                else if (minVal == botDist) z = minBz - r;
                else z = maxBz + r;
            }
            return true;
        }
        return false;
    }

    // Resolve collision and push car out of any overlapping obstacle or map boundary.
    // Returns true if a collision occurred. Position (x, z) is updated to be strictly valid outside the obstacle.
    bool resolveCollision(float& x, float& z, const Track& track, std::string& hitName) const {
        const float carRadius = 0.85f;
        bool collided = false;

        // 1. World Boundaries (Playable Map Perimeter)
        const float minX = -84.0f, maxX = 84.0f;
        const float minZ = -108.0f, maxZ = 68.0f;
        if (x - carRadius < minX) { x = minX + carRadius; hitName = "WORLD BORDER"; collided = true; }
        if (x + carRadius > maxX) { x = maxX - carRadius; hitName = "WORLD BORDER"; collided = true; }
        if (z - carRadius < minZ) { z = minZ + carRadius; hitName = "WORLD BORDER"; collided = true; }
        if (z + carRadius > maxZ) { z = maxZ - carRadius; hitName = "WORLD BORDER"; collided = true; }

        // 2. Continuous Spline Road Information
        RoadSurfaceInfo roadInfo = getRoadInfoAt(x, z, track);

        // 3. Bridge Safety Railings (Covering elevated bridge deck and approach/exit ramps)
        // Railings are placed at 3.25m from centerline (inner face at 3.14m).
        // With car half-width ~0.90m, car center must not exceed 2.15m from centerline!
        bool isBridgeArea = (roadInfo.zone == EnvironmentZone::BRIDGE) || 
                            (roadInfo.splinePoint.y > 0.15f && x > -48.0f && x < 25.0f && z < -10.0f);
        if (isBridgeArea) {
            const float maxBridgeDist = 2.15f;
            if (roadInfo.distToCenter > maxBridgeDist && roadInfo.distToCenter > 0.001f) {
                float excess = roadInfo.distToCenter - maxBridgeDist;
                float dirX = (roadInfo.splinePoint.x - x) / roadInfo.distToCenter;
                float dirZ = (roadInfo.splinePoint.z - z) / roadInfo.distToCenter;
                x += dirX * excess;
                z += dirZ * excess;
                hitName = "BRIDGE RAILING";
                collided = true;
            }
            // While safely on the elevated bridge deck (Y > 0.8m), car cannot hit river or ground underneath
            if (roadInfo.splinePoint.y > 0.8f) {
                return collided;
            }
        }

        // 4. Mountain Tunnel Barriers & Walls (Both Left and Right walls along the curved tunnel)
        // Tunnel runs between Z = -20.5m (Entrance Portal WP 13) and Z = 11.0m (Exit Portal WP 16)
        bool isTunnelTrack = (roadInfo.zone == EnvironmentZone::TUNNEL) && 
                             (roadInfo.splinePoint.z >= -21.0f && roadInfo.splinePoint.z <= 11.5f && roadInfo.splinePoint.x <= -23.0f);
        if (isTunnelTrack) {
            if (roadInfo.distToCenter > 1.95f && roadInfo.distToCenter < 2.50f) {
                // INSIDE THE TUNNEL:
                // Concrete walls are at 3.95m; New Jersey barrier curb is at 2.75m.
                // Clamping car center to 1.95m prevents outer body (~0.90m) from ever penetrating barrier or wall!
                const float maxTunnelDist = 1.95f;
                float excess = roadInfo.distToCenter - maxTunnelDist;
                float dirX = (roadInfo.splinePoint.x - x) / roadInfo.distToCenter;
                float dirZ = (roadInfo.splinePoint.z - z) / roadInfo.distToCenter;
                x += dirX * excess;
                z += dirZ * excess;
                hitName = "TUNNEL WALL";
                collided = true;
            } else if (roadInfo.distToCenter >= 2.50f && roadInfo.distToCenter < 5.00f && x < -24.0f && z > -21.0f && z < 11.0f) {
                // OUTSIDE THE TUNNEL (Driving on mountain hillside / grass outside the curved vault):
                // Exterior tube has radius 4.2m. Clamping distance to >= 5.00m keeps car strictly outside the mountain!
                const float minMountainDist = 5.00f;
                float shortfall = minMountainDist - roadInfo.distToCenter;
                float pushX = (x - roadInfo.splinePoint.x) / roadInfo.distToCenter;
                float pushZ = (z - roadInfo.splinePoint.z) / roadInfo.distToCenter;
                x += pushX * shortfall;
                z += pushZ * shortfall;
                hitName = "MOUNTAIN RIDGE";
                collided = true;
            }
        }

        // 5. River Water Barrier (When not on the elevated bridge)
        // River flows between Z = -34.5 and Z = -57.0 from X = -36.0 to X = 20.0
        if (roadInfo.splinePoint.y < 0.8f && z >= -57.0f && z <= -34.5f && x >= -36.0f && x <= 20.0f) {
            if (z > -45.5f) {
                z = -34.0f;
            } else {
                z = -57.5f;
            }
            hitName = "RIVER WATER";
            collided = true;
        }

        // 6. Garage Main Building: [-59.5, -36.5] x [33.5, 42.5]
        if (pushOutOfBox(x, z, -59.5f, -36.5f, 33.5f, 42.5f, carRadius)) {
            hitName = "GARAGE BUILDING";
            collided = true;
        }

        // 7. Metropolitan City Buildings
        // Building A: [-17.5, -6.5] x [39.5, 50.5]
        if (pushOutOfBox(x, z, -17.5f, -6.5f, 39.5f, 50.5f, carRadius)) {
            hitName = "CITY BUILDING";
            collided = true;
        }
        // Building B: [-1.5, 11.5] x [37.5, 46.5]
        if (pushOutOfBox(x, z, -1.5f, 11.5f, 37.5f, 46.5f, carRadius)) {
            hitName = "CITY TOWER";
            collided = true;
        }
        // Building C: [23.0, 33.0] x [29.0, 39.0]
        if (pushOutOfBox(x, z, 23.0f, 33.0f, 29.0f, 39.0f, carRadius)) {
            hitName = "COMMERCIAL BUILDING";
            collided = true;
        }
        // Building D (Sheared Skyscraper): [-26.5, -17.5] x [43.5, 52.5]
        if (pushOutOfBox(x, z, -26.5f, -17.5f, 43.5f, 52.5f, carRadius)) {
            hitName = "SKYSCRAPER";
            collided = true;
        }

        // 8. Mountain Portal Masonry Walls & Cliff Ridges
        // Left portal abutment wall: [-54.0, -48.2] x [-23.5, -18.2]
        if (pushOutOfBox(x, z, -54.0f, -48.2f, -23.5f, -18.2f, carRadius)) {
            hitName = "TUNNEL PORTAL";
            collided = true;
        }
        // Right portal abutment wall: [-41.8, -32.5] x [-23.5, -18.2]
        if (pushOutOfBox(x, z, -41.8f, -32.5f, -23.5f, -18.2f, carRadius)) {
            hitName = "TUNNEL PORTAL";
            collided = true;
        }
        // Rocky Cliff East of Tunnel: [-32.0, -16.0] x [-16.0, 5.0]
        if (pushOutOfBox(x, z, -32.0f, -16.0f, -16.0f, 5.0f, carRadius)) {
            hitName = "MOUNTAIN CLIFF";
            collided = true;
        }
        // Rocky Mountain Ridge between tunnel and return highway: [-54.5, -48.2] x [-18.0, 6.0]
        if (pushOutOfBox(x, z, -54.5f, -48.2f, -18.0f, 6.0f, carRadius)) {
            hitName = "MOUNTAIN RIDGE";
            collided = true;
        }

        // 9. Gas Stations (Mart buildings, pump islands, and totem signs)
        // Station 1 (City): Mart [39.5, 47.5] x [0.0, 12.0]
        if (pushOutOfBox(x, z, 39.5f, 47.5f, 0.0f, 12.0f, carRadius)) {
            hitName = "GAS STATION MART";
            collided = true;
        }
        // Station 1 Pump Islands: [33.8, 36.2] x [1.2, 4.8] and [33.8, 36.2] x [7.2, 10.8]
        if (pushOutOfBox(x, z, 33.8f, 36.2f, 1.2f, 4.8f, carRadius) ||
            pushOutOfBox(x, z, 33.8f, 36.2f, 7.2f, 10.8f, carRadius)) {
            hitName = "GAS PUMP ISLAND";
            collided = true;
        }
        // Station 2 (Countryside): Mart [-1.0, 11.0] x [-98.5, -92.5]
        if (pushOutOfBox(x, z, -1.0f, 11.0f, -98.5f, -92.5f, carRadius)) {
            hitName = "GAS STATION MART";
            collided = true;
        }
        // Station 2 Pump Islands: [1.2, 2.8] x [-89.2, -84.8] and [7.2, 8.8] x [-89.2, -84.8]
        if (pushOutOfBox(x, z, 1.2f, 2.8f, -89.2f, -84.8f, carRadius) ||
            pushOutOfBox(x, z, 7.2f, 8.8f, -89.2f, -84.8f, carRadius)) {
            hitName = "GAS PUMP ISLAND";
            collided = true;
        }
        // Station 2 Roadside Price Totem: [-3.5, -1.5] x [-81.2, -78.8]
        if (pushOutOfBox(x, z, -3.5f, -1.5f, -81.2f, -78.8f, carRadius)) {
            hitName = "PRICE TOTEM SIGN";
            collided = true;
        }

        // 10. Trees (Trunks outside the road)
        const float treeMinDist = 0.80f + carRadius; // ~1.65m safe clearance
        for (const auto& tree : trees) {
            float dx = x - tree.position.x;
            float dz = z - tree.position.z;
            float d2 = dx * dx + dz * dz;
            if (d2 < treeMinDist * treeMinDist && d2 > 0.0001f) {
                float d = std::sqrt(d2);
                float push = treeMinDist - d;
                x += (dx / d) * push;
                z += (dz / d) * push;
                hitName = "TREE";
                collided = true;
            }
        }

        // 11. Street Lamps
        const float lampMinDist = 0.40f + carRadius; // ~1.25m safe clearance
        for (const auto& lamp : streetLamps) {
            float dx = x - lamp.position.x;
            float dz = z - lamp.position.z;
            float d2 = dx * dx + dz * dz;
            if (d2 < lampMinDist * lampMinDist && d2 > 0.0001f) {
                float d = std::sqrt(d2);
                float push = lampMinDist - d;
                x += (dx / d) * push;
                z += (dz / d) * push;
                hitName = "STREET LAMP";
                collided = true;
            }
        }

        // 12. Milestone Gantries
        const float pylonMinDist = 0.45f + carRadius;
        for (const auto& ms : milestones) {
            m3d::Vec3 right = m3d::cross(ms.forward, m3d::Vec3(0, 1, 0)).normalized();
            m3d::Vec3 pL = ms.position - right * 4.4f;
            m3d::Vec3 pR = ms.position + right * 4.4f;

            float dxL = x - pL.x, dzL = z - pL.z;
            float d2L = dxL * dxL + dzL * dzL;
            if (d2L < pylonMinDist * pylonMinDist && d2L > 0.0001f) {
                float d = std::sqrt(d2L);
                x += (dxL / d) * (pylonMinDist - d);
                z += (dzL / d) * (pylonMinDist - d);
                hitName = "CHECKPOINT PYLON";
                collided = true;
            }

            float dxR = x - pR.x, dzR = z - pR.z;
            float d2R = dxR * dxR + dzR * dzR;
            if (d2R < pylonMinDist * pylonMinDist && d2R > 0.0001f) {
                float d = std::sqrt(d2R);
                x += (dxR / d) * (pylonMinDist - d);
                z += (dzR / d) * (pylonMinDist - d);
                hitName = "CHECKPOINT PYLON";
                collided = true;
            }
        }

        return collided;
    }

    // Reset car position, orientation, and game run states
    void resetCar(const Track& track) {
        carDistance = 0.0f;
        carSpeed = 0.0f;
        wheelSteerAngle = 0.0f;
        wheelSpinAngle = 0.0f;
        carYaw = 0.0f;
        carPitch = 0.0f;
        isJourneyComplete = false;
        isBraking = false;
        isReversing = false;

        // Reset collision state
        isCollidingWithObstacle = false;
        activeObstacleName = "";
        collisionAlertTimer = 0.0f;

        // Reset fuel and milestone state
        fuel = maxFuel;
        if (coinsCollected < 3) coinsCollected = 3; // Ensure 3 starting coins
        currentMilestoneIdx = 0;
        for (auto& m : milestones) m.isReached = false;
        for (auto& c : coins) {
            c.isCollected = false;
            c.position.y = c.baseHeight;
            c.respawnTimer = 0.0f;
            c.collectAnimTimer = 0.0f;
        }
        notificationMessage = "";
        notificationTimer = 0.0f;

        track.sample(0.0f, carPos, carForward, carUp, currentZone, carSpeed);
        carPos = m3d::Vec3(-48.0f, 0.0f, 30.0f);
        carForward = m3d::Vec3(1.0f, 0.0f, 0.0f);
        carUp = m3d::Vec3(0.0f, 1.0f, 0.0f);
        carRight = m3d::cross(carForward, carUp).normalized();
        carYaw = 0.0f;
    }

    void initGameEntities(const Track& track) {
        // [A] Initialize 2 Roadside Gas Stations
        gasStations.clear();

        // Station 1: City Gateway Gas Station (on East shoulder of City Road facing road westward)
        GasStation s1;
        s1.position = m3d::Vec3(35.0f, 0.0f, 6.0f);
        s1.rotationY = -m3d::PI * 0.5f; // Rotated to face west toward the city roadway
        s1.name = "City Gateway Gas Station";
        s1.pumpBayCenter = m3d::Vec3(31.5f, 0.0f, 6.0f);
        s1.refuelRadius = 9.0f;
        gasStations.push_back(s1);

        // Station 2: Countryside Highway Oasis (on North shoulder facing highway southward)
        GasStation s2;
        s2.position = m3d::Vec3(5.0f, 0.0f, -87.0f);
        s2.rotationY = 0.0f; // Facing south toward the countryside highway drivers!
        s2.name = "Countryside Highway Oasis";
        s2.pumpBayCenter = m3d::Vec3(5.0f, 0.0f, -83.5f);
        s2.refuelRadius = 9.0f;
        gasStations.push_back(s2);

        // [B] Initialize 5 Milestone Checkpoint Gates along the track
        milestones.clear();
        struct MSData { int wpIdx; const char* name; };
        MSData msData[5] = {
            { 4,  "Milestone 1: City Gateway" },
            { 8,  "Milestone 2: Grand River Bridge" },
            { 13, "Milestone 3: Mountain Tunnel Portal" },
            { 24, "Milestone 4: Countryside Speed Trap" },
            { 33, "Milestone 5: Garage Lap Finish" }
        };

        for (int i = 0; i < 5; ++i) {
            Milestone m;
            int idx = msData[i].wpIdx;
            m.index = i + 1;
            m.name = msData[i].name;
            m.trackDistance = track.cumulativeDistances[idx];

            // Evaluate continuous position and forward heading at milestone
            m3d::Vec3 p1 = track.evaluateSpline(static_cast<float>(idx));
            m3d::Vec3 p2 = track.evaluateSpline(static_cast<float>(idx) + 0.1f);
            m.position = p1;
            m.forward = (p2 - p1).normalized();
            m.isReached = false;
            milestones.push_back(m);
        }

        // [C] Initialize 30 Golden Collectible Coins along road spline
        coins.clear();
        const int numCoins = 30;
        for (int i = 0; i < numCoins; ++i) {
            float dist = ((float)i + 0.5f) / static_cast<float>(numCoins) * track.totalLength;
            m3d::Vec3 p, fwd, up; EnvironmentZone z; float spd;
            track.sample(dist, p, fwd, up, z, spd);

            m3d::Vec3 right = m3d::cross(fwd, up).normalized();
            // Alternate lanes: Left (-1.25m), Center (0.0m), Right (+1.25m)
            float laneOffset = ((i % 3) == 0 ? 0.0f : ((i % 3) == 1 ? -1.25f : 1.25f));

            Coin c;
            c.id = i;
            c.laneOffset = laneOffset;
            c.baseHeight = p.y + 0.65f;
            c.position = p + right * laneOffset + up * 0.65f;
            c.rotationAngle = static_cast<float>((i * 47) % 360);
            c.isCollected = false;
            c.respawnTimer = 0.0f;
            c.collectAnimTimer = 0.0f;
            coins.push_back(c);
        }
    }

    void init(const Track& track) {
        // Initialize textures (loads BMPs or generates procedural textures)
        textures.init();

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
        initGameEntities(track);
        resetCar(track);
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
            // Gas station 1 (City Gateway) lot exclusion
            if (tx >= 23.0f && tx <= 42.0f && tz >= -3.0f && tz <= 16.0f) return false;
            // Gas station 2 (Countryside Oasis) lot exclusion
            if (tx >= -8.0f && tx <= 20.0f && tz >= -89.0f && tz <= -69.0f) return false;

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

    void update(float dt, const Track& track, bool inputForward = false, bool inputBackward = false, bool inputLeft = false, bool inputRight = false) {
        globalTime += dt;

        // Auto Day/Night cycle
        if (autoDayNight) {
            timeOfDay += dt * 0.02f; // full cycle every 50 seconds
            if (timeOfDay >= 1.0f) timeOfDay -= 1.0f;
            isNight = (timeOfDay < 0.25f || timeOfDay > 0.75f);
        }

        // Headlights automatic activation (at night OR inside mountain tunnel)
        headlightsActive = isNight || (currentZone == EnvironmentZone::TUNNEL);

        if (isManualDrive) {
            // =========================================================
            // 1. MANUAL USER DRIVING MODE (Keyboard Arrow Keys / WASD)
            // =========================================================

            // [A] Responsive Steering Input (Smoothed for stable, comfortable control)
            float targetSteer = 0.0f;
            if (inputLeft)  targetSteer -= 19.0f; // degrees left (reduced from 28.0f)
            if (inputRight) targetSteer += 19.0f; // degrees right

            float steerSpeed = (targetSteer == 0.0f) ? 10.0f : 6.5f;
            wheelSteerAngle += (targetSteer - wheelSteerAngle) * std::min(1.0f, dt * steerSpeed);

            // [B] Throttle, Braking, and Reverse
            isBraking = false;
            isReversing = false;

            if (inputForward) {
                // Forward Acceleration (only if fuel > 0)
                const float maxForwardSpeed = 26.0f; // ~93.6 km/h
                const float accelRate = 12.0f;       // m/s^2
                if (carSpeed < -0.2f) {
                    // Braking while in reverse
                    carSpeed = std::min(0.0f, carSpeed + 24.0f * dt);
                    isBraking = true;
                } else if (fuel <= 0.0f) {
                    // Out of fuel: engine cuts out!
                    if (carSpeed > 0.0f) carSpeed = std::max(0.0f, carSpeed - 4.2f * dt);
                    if (notificationTimer <= 0.0f) {
                        triggerNotification("OUT OF FUEL! VISIT GAS STATION [F] OR RESET [R]", 0.6f);
                    }
                } else if (carSpeed < maxForwardSpeed) {
                    carSpeed = std::min(maxForwardSpeed, carSpeed + accelRate * dt);
                    // Consume fuel based on speed / throttle
                    float consumption = 1.3f + (carSpeed / maxForwardSpeed) * 1.5f;
                    fuel = std::max(0.0f, fuel - consumption * dt);
                }
            } else if (inputBackward) {
                // Braking or Reverse
                if (carSpeed > 0.4f) {
                    // Active braking while moving forward
                    const float brakeRate = 22.0f; // m/s^2
                    carSpeed = std::max(0.0f, carSpeed - brakeRate * dt);
                    isBraking = true;
                } else {
                    // Reverse drive (slow reverse allowed to maneuver even on low fuel)
                    const float maxReverseSpeed = -7.5f; // ~27 km/h in reverse
                    const float reverseAccel = 8.0f;     // m/s^2
                    if (fuel > 0.0f) {
                        carSpeed = std::max(maxReverseSpeed, carSpeed - reverseAccel * dt);
                        fuel = std::max(0.0f, fuel - 1.2f * dt);
                    } else {
                        // Emergency crawl in reverse if empty
                        carSpeed = std::max(-2.0f, carSpeed - 2.5f * dt);
                    }
                    isReversing = true;
                }
            } else {
                // Coasting / Rolling resistance
                if (carSpeed > 0.0f) {
                    carSpeed = std::max(0.0f, carSpeed - 3.8f * dt);
                } else if (carSpeed < 0.0f) {
                    carSpeed = std::min(0.0f, carSpeed + 4.8f * dt);
                }
                // Idle fuel drain
                if (fuel > 0.0f) {
                    fuel = std::max(0.0f, fuel - 0.12f * dt);
                }
            }

            // [C] Vehicle Kinematics & Ackermann Steering Turn Rate
            const float wheelbase = 2.30f; // Distance between front and rear axles
            if (std::abs(carSpeed) > 0.02f) {
                // Speed-sensitive steering damping for high-speed stability
                float speedRatio = std::min(1.0f, std::abs(carSpeed) / 26.0f);
                float effectiveSteerDeg = wheelSteerAngle * (1.0f - 0.40f * speedRatio);
                float steerRad = effectiveSteerDeg * m3d::DEG2RAD;

                // Turn rate: d(yaw)/dt = (v / L) * tan(steer)
                float yawRate = (carSpeed / wheelbase) * std::tan(steerRad) * 0.76f;
                carYaw += yawRate * dt;

                while (carYaw > m3d::PI)  carYaw -= 2.0f * m3d::PI;
                while (carYaw < -m3d::PI) carYaw += 2.0f * m3d::PI;
            }

            // [D] Planar Horizontal Driving Integration with Physical Obstacle & World Border Collision
            float cy = std::cos(carYaw);
            float sy = std::sin(carYaw);

            float deltaMove = carSpeed * dt;
            float candX = carPos.x + cy * deltaMove;
            float candZ = carPos.z + sy * deltaMove;

            std::string hitObs;
            bool hit = resolveCollision(candX, candZ, track, hitObs);

            // Car position is updated to the resolved, depenetrated position
            carPos.x = candX;
            carPos.z = candZ;

            if (hit) {
                isCollidingWithObstacle = true;
                activeObstacleName = hitObs;
                collisionAlertTimer = 1.0f;

                // Stop linear speed on impact
                carSpeed = 0.0f;
            } else {
                if (collisionAlertTimer > 0.0f) {
                    collisionAlertTimer -= dt;
                    if (collisionAlertTimer <= 0.0f) {
                        isCollidingWithObstacle = false;
                    }
                }
            }

            // [E] Continuous Smooth Surface Height & Suspension Integration
            // Samples exact C1 Catmull-Rom spline height without discrete jumps or staircases
            RoadSurfaceInfo rInfo = getRoadInfoAt(carPos.x, carPos.z, track);
            currentZone = rInfo.zone;
            carDistance = rInfo.trackDistance;
            float targetGroundY = rInfo.elevation;

            // Responsive spring-damper suspension (silky smooth vertical transitions without bouncing)
            float suspensionRate = 20.0f;
            carPos.y += (targetGroundY - carPos.y) * std::min(1.0f, dt * suspensionRate);

            // [F] Smooth Road Slope Pitch
            EnvironmentZone dummyZone;
            float frontTargetY = getRoadHeightAt(carPos.x + cy * 1.4f, carPos.z + sy * 1.4f, track, dummyZone);
            float rearTargetY  = getRoadHeightAt(carPos.x - cy * 1.4f, carPos.z - sy * 1.4f, track, dummyZone);
            float targetPitch = std::atan2(frontTargetY - rearTargetY, 2.8f);
            carPitch += (targetPitch - carPitch) * std::min(1.0f, dt * 10.0f);

            // [G] Update Orthonormal Orientation Basis with Pitch Tilt
            float cp = std::cos(carPitch);
            float sp = std::sin(carPitch);

            carForward = m3d::Vec3(cy * cp, sp, sy * cp).normalized();
            carRight = m3d::cross(carForward, m3d::Vec3(0.0f, 1.0f, 0.0f)).normalized();
            carUp = m3d::cross(carRight, carForward).normalized();

            // Wheel spin rotation proportional to travel distance (radius = 0.38m)
            const float wheelRadius = 0.38f;
            wheelSpinAngle += (carSpeed * dt / wheelRadius) * m3d::RAD2DEG;
            if (wheelSpinAngle > 360.0f)  wheelSpinAngle -= 360.0f;
            if (wheelSpinAngle < -360.0f) wheelSpinAngle += 360.0f;

        } else {
            // =========================================================
            // 2. AUTONOMOUS PATH-FOLLOWING MODE
            // =========================================================
            m3d::Vec3 nextP, nextFwd, nextUp;
            EnvironmentZone targetZone;
            float targetSpeed;
            track.sample(carDistance, carPos, carForward, carUp, targetZone, targetSpeed);
            currentZone = targetZone;

            if (!isJourneyComplete) {
                if (fuel > 0.0f) {
                    float consumption = 1.1f + (carSpeed / 20.0f) * 1.3f;
                    fuel = std::max(0.0f, fuel - consumption * dt);
                } else {
                    targetSpeed = 0.0f;
                }

                float speedRate = 4.5f; // m/s^2
                float distToEnd = track.totalLength - carDistance;
                if (distToEnd < 16.0f) {
                    targetSpeed = std::max(0.0f, (distToEnd / 16.0f) * 4.5f);
                }

                if (carSpeed < targetSpeed) {
                    carSpeed = std::min(targetSpeed, carSpeed + speedRate * dt);
                } else if (carSpeed > targetSpeed) {
                    carSpeed = std::max(targetSpeed, carSpeed - speedRate * 1.5f * dt);
                }

                float deltaDist = carSpeed * dt;
                carDistance += deltaDist;

                if (carDistance >= track.totalLength) {
                    carDistance = track.totalLength;
                    carSpeed = 0.0f;
                    wheelSteerAngle = 0.0f;
                    isJourneyComplete = true;
                }

                float wheelRadius = 0.38f;
                wheelSpinAngle += (deltaDist / wheelRadius) * m3d::RAD2DEG;
                if (wheelSpinAngle > 360.0f) wheelSpinAngle -= 360.0f;
            } else {
                carSpeed = 0.0f;
                wheelSteerAngle = 0.0f;
            }

            carRight = m3d::cross(carForward, carUp).normalized();
            carUp = m3d::cross(carRight, carForward).normalized();

            if (!isJourneyComplete) {
                m3d::Vec3 futureP, futureFwd, futureUp;
                EnvironmentZone fz; float fs;
                track.sample(carDistance + 3.0f, futureP, futureFwd, futureUp, fz, fs);
                float turnDot = m3d::dot(carRight, futureFwd);
                wheelSteerAngle = m3d::clamp(turnDot * 35.0f, -30.0f, 30.0f);
            } else {
                wheelSteerAngle = 0.0f;
            }
        }

        // Update boats movement strictly under the Grand River Bridge
        for (auto& boat : boats) {
            boat.basePosition.x += boat.speed * dt;
            if (boat.basePosition.x > 12.0f) boat.basePosition.x = -28.0f;
            if (boat.basePosition.x < -28.0f) boat.basePosition.x = 12.0f;
        }

        // -------------------------------------------------------------
        // Game Systems: Gas Stations, Coins, Milestones, Notifications
        // -------------------------------------------------------------

        // [A] Proximity detection to roadside Gas Stations
        isNearGasStation = false;
        activeStationIdx = -1;
        activeStationName = "";
        for (size_t i = 0; i < gasStations.size(); ++i) {
            float dx = carPos.x - gasStations[i].pumpBayCenter.x;
            float dz = carPos.z - gasStations[i].pumpBayCenter.z;
            float distSq = dx * dx + dz * dz;
            if (distSq < gasStations[i].refuelRadius * gasStations[i].refuelRadius) {
                isNearGasStation = true;
                activeStationIdx = static_cast<int>(i);
                activeStationName = gasStations[i].name;
                break;
            }
        }

        // [B] Golden Coin Collectibles (Hover Bobbing, 3D Spin, Collection & Respawn)
        for (auto& coin : coins) {
            coin.rotationAngle += 160.0f * dt;
            if (coin.rotationAngle >= 360.0f) coin.rotationAngle -= 360.0f;

            if (coin.isCollected) {
                if (coin.collectAnimTimer > 0.0f) {
                    coin.collectAnimTimer -= dt;
                    coin.position.y += dt * 3.8f; // Floats up with golden pop
                } else {
                    coin.respawnTimer -= dt;
                    if (coin.respawnTimer <= 0.0f) {
                        coin.isCollected = false;
                        coin.position.y = coin.baseHeight;
                    }
                }
            } else {
                coin.position.y = coin.baseHeight + 0.14f * std::sin(globalTime * 3.8f + coin.id);

                float dx = carPos.x - coin.position.x;
                float dy = (carPos.y + 0.38f) - coin.position.y;
                float dz = carPos.z - coin.position.z;
                float distSq = dx * dx + dy * dy + dz * dz;

                if (distSq < 2.3f * 2.3f) {
                    coin.isCollected = true;
                    coin.collectAnimTimer = 0.5f;
                    coin.respawnTimer = 22.0f; // Respawns after 22 seconds
                    coinsCollected++;
                    triggerNotification("+1 COIN! [TOTAL: " + std::to_string(coinsCollected) + "]", 1.6f);
                }
            }
        }

        // [C] Milestone Checkpoint Gates
        if (currentMilestoneIdx < static_cast<int>(milestones.size())) {
            const auto& ms = milestones[currentMilestoneIdx];
            float dx = carPos.x - ms.position.x;
            float dz = carPos.z - ms.position.z;
            if (dx * dx + dz * dz < 5.8f * 5.8f) {
                milestones[currentMilestoneIdx].isReached = true;
                if (currentMilestoneIdx == static_cast<int>(milestones.size()) - 1) {
                    lapsCompleted++;
                    coinsCollected += 5; // Lap bonus
                    fuel = std::min(maxFuel, fuel + 25.0f); // Bonus fuel on lap finish
                    triggerNotification("* LAP " + std::to_string(lapsCompleted) + " COMPLETE! +5 COINS *", 3.5f);
                    for (auto& m : milestones) m.isReached = false;
                    currentMilestoneIdx = 0;
                } else {
                    coinsCollected += 2; // Checkpoint bonus
                    currentMilestoneIdx++;
                    triggerNotification("* CHECKPOINT " + std::to_string(currentMilestoneIdx) + "/5! +2 COINS *", 2.8f);
                }
            }
        }

        // [D] HUD Notifications & Low Fuel Alert
        if (notificationTimer > 0.0f) {
            notificationTimer -= dt;
            if (notificationTimer <= 0.0f) {
                notificationMessage = "";
            }
        } else {
            if (fuel > 0.0f && fuel < 20.0f) {
                triggerNotification("WARNING: LOW FUEL (<20%)! VISIT A GAS STATION", 1.2f);
            }
        }
    }

    // Set uniform lighting data in shader
    void setupLighting(const Shader& shader, const m3d::Vec3& camPos) const {
        shader.use();
        shader.setVec3("viewPos", camPos);

        // 1. Sun / Moon Directional Light (DirLight: direction, ambient, diffuse, specular)
        m3d::Vec3 dirLightDir;
        m3d::Vec3 dirAmbient;
        m3d::Vec3 dirDiffuse;
        m3d::Vec3 dirSpecular;
        m3d::Vec3 ambientGlobal;

        if (!isNight) {
            // Daytime: warm golden sun angled towards +Z to bathe track in direct sunlight
            dirLightDir   = m3d::Vec3(-0.35f, -0.70f, 0.60f).normalized();
            dirAmbient    = m3d::Vec3(0.20f, 0.20f, 0.22f);
            dirDiffuse    = m3d::Vec3(1.10f, 1.05f, 0.95f) * 1.15f;
            dirSpecular   = m3d::Vec3(1.00f, 0.98f, 0.95f) * 1.20f;
            ambientGlobal = m3d::Vec3(0.42f, 0.44f, 0.48f);
        } else {
            // Nighttime: cool dark moonlight
            dirLightDir   = m3d::Vec3(0.30f, -0.70f, 0.50f).normalized();
            dirAmbient    = m3d::Vec3(0.04f, 0.05f, 0.08f);
            dirDiffuse    = m3d::Vec3(0.18f, 0.22f, 0.35f) * 0.70f;
            dirSpecular   = m3d::Vec3(0.25f, 0.30f, 0.45f);
            ambientGlobal = m3d::Vec3(0.08f, 0.09f, 0.14f);
        }

        shader.setVec3("dirLight.direction", dirLightDir);
        shader.setVec3("dirLight.ambient", dirAmbient);
        shader.setVec3("dirLight.diffuse", dirDiffuse);
        shader.setVec3("dirLight.specular", dirSpecular);
        shader.setVec3("dirLight.color", dirDiffuse); // Legacy compatibility
        shader.setVec3("ambientGlobal", ambientGlobal);

        // 2. Point Lights (Active Street Lamps + Active Tunnel Lights + Gas Station Canopies)
        int pointLightIdx = 0;
        const int MAX_POINT_LIGHTS = 24;

        if (pointLightsActive) {
            // [A] Street Lamps (Active at night: warm incandescent amber glow)
            if (isNight) {
                for (const auto& lamp : streetLamps) {
                    if (pointLightIdx >= MAX_POINT_LIGHTS) break;
                    std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";
                    m3d::Vec3 lampLightPos = lamp.position + m3d::Vec3(0.0f, 5.0f, 0.0f);
                    shader.setVec3(prefix + "position", lampLightPos);
                    shader.setVec3(prefix + "ambient", m3d::Vec3(0.06f, 0.05f, 0.03f));
                    shader.setVec3(prefix + "diffuse", m3d::Vec3(1.0f, 0.82f, 0.55f) * 2.2f);
                    shader.setVec3(prefix + "specular", m3d::Vec3(1.0f, 0.90f, 0.70f) * 1.5f);
                    shader.setFloat(prefix + "constant", 1.0f);
                    shader.setFloat(prefix + "linear", 0.09f);
                    shader.setFloat(prefix + "quadratic", 0.032f);
                    shader.setFloat(prefix + "radius", 20.0f);
                    // Backward-compatible properties
                    shader.setVec3(prefix + "color", m3d::Vec3(1.0f, 0.82f, 0.55f));
                    shader.setFloat(prefix + "intensity", 1.8f);
                    pointLightIdx++;
                }
            }

            // [B] Tunnel Ceiling Lights (Active 24/7 day & night: fluorescent cool white)
            for (const auto& tLight : tunnelCeilingLights) {
                if (pointLightIdx >= MAX_POINT_LIGHTS) break;
                std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";
                shader.setVec3(prefix + "position", tLight.position);
                shader.setVec3(prefix + "ambient", m3d::Vec3(0.05f, 0.05f, 0.06f));
                shader.setVec3(prefix + "diffuse", tLight.color * tLight.intensity * 2.2f);
                shader.setVec3(prefix + "specular", m3d::Vec3(1.0f, 1.0f, 1.0f) * 1.5f);
                shader.setFloat(prefix + "constant", 1.0f);
                shader.setFloat(prefix + "linear", 0.09f);
                shader.setFloat(prefix + "quadratic", 0.032f);
                shader.setFloat(prefix + "radius", tLight.range);
                // Backward-compatible properties
                shader.setVec3(prefix + "color", tLight.color);
                shader.setFloat(prefix + "intensity", tLight.intensity);
                pointLightIdx++;
            }

            // [C] Gas Station Canopy Downlights (Bright illuminated pump forecourts)
            for (const auto& st : gasStations) {
                if (pointLightIdx >= MAX_POINT_LIGHTS) break;
                std::string prefix = "pointLights[" + std::to_string(pointLightIdx) + "].";
                m3d::Vec3 canopyLightPos = st.pumpBayCenter + m3d::Vec3(0.0f, 4.4f, 0.0f);
                float boost = isNight ? 2.6f : 1.4f;
                shader.setVec3(prefix + "position", canopyLightPos);
                shader.setVec3(prefix + "ambient", m3d::Vec3(0.08f, 0.08f, 0.08f));
                shader.setVec3(prefix + "diffuse", m3d::Vec3(1.0f, 0.98f, 0.92f) * boost);
                shader.setVec3(prefix + "specular", m3d::Vec3(1.0f, 1.0f, 1.0f) * 1.8f);
                shader.setFloat(prefix + "constant", 1.0f);
                shader.setFloat(prefix + "linear", 0.07f);
                shader.setFloat(prefix + "quadratic", 0.024f);
                shader.setFloat(prefix + "radius", 18.0f);
                // Backward-compatible properties
                shader.setVec3(prefix + "color", m3d::Vec3(1.0f, 0.98f, 0.92f));
                shader.setFloat(prefix + "intensity", boost);
                pointLightIdx++;
            }
        }

        shader.setInt("numPointLights", pointLightIdx);

        // 3. Spotlights: Car Twin Conical Headlights
        if (headlightsActive) {
            shader.setBool("spotlightsActive", true);
            m3d::Vec3 leftHeadlightPos = carPos + carForward * 1.9f - carRight * 0.65f + carUp * 0.45f;
            m3d::Vec3 rightHeadlightPos = carPos + carForward * 1.9f + carRight * 0.65f + carUp * 0.45f;

            // Spotlights slightly angled down toward road surface
            m3d::Vec3 spotDir = (carForward - carUp * 0.12f).normalized();

            m3d::Vec3 spotAmb(0.02f, 0.02f, 0.02f);
            m3d::Vec3 spotDiff = m3d::Vec3(1.0f, 0.98f, 0.92f) * 3.2f;
            m3d::Vec3 spotSpec = m3d::Vec3(1.0f, 1.0f, 1.0f) * 2.5f;
            float cosInner = std::cos(m3d::radians(22.0f));
            float cosOuter = std::cos(m3d::radians(30.0f));

            // Left Spotlight
            shader.setVec3("leftSpotlight.position", leftHeadlightPos);
            shader.setVec3("leftSpotlight.direction", spotDir);
            shader.setVec3("leftSpotlight.ambient", spotAmb);
            shader.setVec3("leftSpotlight.diffuse", spotDiff);
            shader.setVec3("leftSpotlight.specular", spotSpec);
            shader.setFloat("leftSpotlight.constant", 1.0f);
            shader.setFloat("leftSpotlight.linear", 0.04f);
            shader.setFloat("leftSpotlight.quadratic", 0.016f);
            shader.setFloat("leftSpotlight.cutOff", cosInner);
            shader.setFloat("leftSpotlight.outerCutOff", cosOuter);
            shader.setVec3("leftSpotlight.color", spotDiff);

            // Right Spotlight
            shader.setVec3("rightSpotlight.position", rightHeadlightPos);
            shader.setVec3("rightSpotlight.direction", spotDir);
            shader.setVec3("rightSpotlight.ambient", spotAmb);
            shader.setVec3("rightSpotlight.diffuse", spotDiff);
            shader.setVec3("rightSpotlight.specular", spotSpec);
            shader.setFloat("rightSpotlight.constant", 1.0f);
            shader.setFloat("rightSpotlight.linear", 0.04f);
            shader.setFloat("rightSpotlight.quadratic", 0.016f);
            shader.setFloat("rightSpotlight.cutOff", cosInner);
            shader.setFloat("rightSpotlight.outerCutOff", cosOuter);
            shader.setVec3("rightSpotlight.color", spotDiff);
        } else {
            shader.setBool("spotlightsActive", false);
        }
    }
};

#endif // SCENE_HPP
