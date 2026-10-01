#pragma once
#ifndef TRACK_HPP
#define TRACK_HPP

#include "math3d.hpp"
#include <vector>
#include <string>

enum class EnvironmentZone {
    GARAGE = 0,
    CITY = 1,
    BRIDGE = 2,
    TUNNEL = 3,
    COUNTRYSIDE = 4
};

inline const char* getZoneName(EnvironmentZone zone) {
    switch (zone) {
        case EnvironmentZone::GARAGE: return "Garage (Departure/Arrival)";
        case EnvironmentZone::CITY: return "Metropolitan City Area";
        case EnvironmentZone::BRIDGE: return "Grand River Bridge";
        case EnvironmentZone::TUNNEL: return "Mountain Tunnel";
        case EnvironmentZone::COUNTRYSIDE: return "Scenic Countryside Highway";
        default: return "Roadway";
    }
}

struct Waypoint {
    m3d::Vec3 position;
    EnvironmentZone zone;
    float targetSpeed; // In m/s (e.g. 6.0 = 21.6 km/h, 22.0 = 79.2 km/h)
};

class Track {
public:
    std::vector<Waypoint> waypoints;
    std::vector<float> cumulativeDistances;
    float totalLength{0.0f};

    Track() {
        initWaypoints();
        computeDistances();
    }

    void initWaypoints() {
        // Closed loop track with realistic dimensions and smooth curvature
        // Coordinate axes: X = East/West, Y = Altitude (Up), Z = North/South
        waypoints = {
            // Zone 0: Garage Bay & Driveway (Start Point at building named GARAGE)
            { {-48.0f,  0.0f,  30.0f}, EnvironmentZone::GARAGE,      6.0f },
            { {-38.0f,  0.0f,  30.0f}, EnvironmentZone::GARAGE,      9.0f },
            { {-26.0f,  0.0f,  30.0f}, EnvironmentZone::GARAGE,     12.0f },

            // Zone 1: City Area (Streets, intersections, buildings)
            { {-15.0f,  0.0f,  30.0f}, EnvironmentZone::CITY,       14.0f },
            { { -2.0f,  0.0f,  30.0f}, EnvironmentZone::CITY,       14.0f },
            { { 12.0f,  0.0f,  26.0f}, EnvironmentZone::CITY,       12.0f },
            { { 22.0f,  0.0f,  15.0f}, EnvironmentZone::CITY,       13.0f },
            { { 22.0f,  0.0f,  -2.0f}, EnvironmentZone::CITY,       14.0f },

            // Approach to Bridge (Elevating over river)
            { { 20.0f,  1.5f, -16.0f}, EnvironmentZone::BRIDGE,     11.0f },
            { { 15.0f,  3.2f, -30.0f}, EnvironmentZone::BRIDGE,     10.0f },

            // Zone 2: Grand River Bridge (Elevated road deck at Y = 3.2m, water & boats underneath)
            { {  6.0f,  3.2f, -44.0f}, EnvironmentZone::BRIDGE,      9.5f },
            { {-10.0f,  3.2f, -47.0f}, EnvironmentZone::BRIDGE,      9.5f },
            { {-26.0f,  3.2f, -44.0f}, EnvironmentZone::BRIDGE,      9.5f },
            { {-36.0f,  3.2f, -33.0f}, EnvironmentZone::BRIDGE,     10.0f },

            // Approach to Mountain Tunnel (Descending to ground level on solid land)
            { {-43.0f,  1.5f, -26.0f}, EnvironmentZone::TUNNEL,     11.0f },

            // Zone 3: Mountain Tunnel (Through mountain terrain, interior lights)
            { {-45.0f,  0.0f, -18.0f}, EnvironmentZone::TUNNEL,     10.5f },
            { {-45.0f,  0.0f,  -6.0f}, EnvironmentZone::TUNNEL,     10.5f },
            { {-38.0f,  0.0f,   4.0f}, EnvironmentZone::TUNNEL,     10.5f },
            { {-26.0f,  0.0f,  10.0f}, EnvironmentZone::TUNNEL,     11.0f },

            // Zone 4: Countryside Highway (Open road, lush swaying trees, street lamps)
            { {-12.0f,  0.0f,  10.0f}, EnvironmentZone::COUNTRYSIDE, 20.0f },
            { {  5.0f,  0.0f,   6.0f}, EnvironmentZone::COUNTRYSIDE, 22.0f },
            { { 28.0f,  0.0f,  -5.0f}, EnvironmentZone::COUNTRYSIDE, 22.0f },
            { { 42.0f,  0.0f, -22.0f}, EnvironmentZone::COUNTRYSIDE, 20.0f },
            { { 45.0f,  0.0f, -45.0f}, EnvironmentZone::COUNTRYSIDE, 18.0f },
            { { 30.0f,  0.0f, -65.0f}, EnvironmentZone::COUNTRYSIDE, 18.0f },
            { {  5.0f,  0.0f, -70.0f}, EnvironmentZone::COUNTRYSIDE, 20.0f },
            { {-25.0f,  0.0f, -68.0f}, EnvironmentZone::COUNTRYSIDE, 20.0f },
            { {-48.0f,  0.0f, -55.0f}, EnvironmentZone::COUNTRYSIDE, 16.0f },
            { {-60.0f,  0.0f, -30.0f}, EnvironmentZone::COUNTRYSIDE, 14.0f },
            { {-60.0f,  0.0f, -10.0f}, EnvironmentZone::COUNTRYSIDE, 12.0f },
            { {-60.0f,  0.0f,  10.0f}, EnvironmentZone::GARAGE,      10.0f },
            { {-59.2f,  0.0f,  20.0f}, EnvironmentZone::GARAGE,       7.0f },
            { {-56.5f,  0.0f,  26.5f}, EnvironmentZone::GARAGE,       4.5f },
            { {-52.0f,  0.0f,  29.5f}, EnvironmentZone::GARAGE,       2.0f }
        };
    }

    void computeDistances() {
        int n = static_cast<int>(waypoints.size());
        cumulativeDistances.clear();
        cumulativeDistances.resize(n, 0.0f);
        totalLength = 0.0f;

        // Finely sample spline between waypoints to get accurate arc length
        for (int i = 0; i < n; ++i) {
            cumulativeDistances[i] = totalLength;

            // Approximate segment length with 16 samples
            m3d::Vec3 prevP = evaluateSpline(static_cast<float>(i));
            float segLen = 0.0f;
            for (int s = 1; s <= 16; ++s) {
                float t = static_cast<float>(i) + static_cast<float>(s) / 16.0f;
                m3d::Vec3 currP = evaluateSpline(t);
                segLen += m3d::distance(prevP, currP);
                prevP = currP;
            }
            totalLength += segLen;
        }
    }

    // Centripetal / Catmull-Rom Spline Interpolation
    m3d::Vec3 evaluateSpline(float t) const {
        int n = static_cast<int>(waypoints.size());
        while (t < 0.0f) t += n;
        while (t >= n) t -= n;

        int i1 = static_cast<int>(t);
        float frac = t - i1;

        int i0 = (i1 - 1 + n) % n;
        int i2 = (i1 + 1) % n;
        int i3 = (i1 + 2) % n;

        const m3d::Vec3& p0 = waypoints[i0].position;
        const m3d::Vec3& p1 = waypoints[i1].position;
        const m3d::Vec3& p2 = waypoints[i2].position;
        const m3d::Vec3& p3 = waypoints[i3].position;

        // Standard Catmull-Rom formula:
        // q(s) = 0.5 * ((2*p1) + (-p0 + p2)*s + (2*p0 - 5*p1 + 4*p2 - p3)*s^2 + (-p0 + 3*p1 - 3*p2 + p3)*s^3)
        float s = frac;
        float s2 = s * s;
        float s3 = s2 * s;

        m3d::Vec3 res = 0.5f * (
            (2.0f * p1) +
            (-p0 + p2) * s +
            (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * s2 +
            (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * s3
        );
        if (res.y < 0.0f) res.y = 0.0f;
        return res;
    }

    // Convert travel distance (meters) along loop to spline parameter t
    float distanceToT(float dist) const {
        while (dist < 0.0f) dist += totalLength;
        while (dist >= totalLength) dist -= totalLength;

        int n = static_cast<int>(waypoints.size());
        // Binary search for interval
        int low = 0, high = n - 1;
        while (low <= high) {
            int mid = (low + high) / 2;
            if (cumulativeDistances[mid] <= dist) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        int i = high;
        if (i < 0) i = 0;
        if (i >= n) i = n - 1;

        int nextIdx = (i + 1) % n;
        float d0 = cumulativeDistances[i];
        float d1 = (nextIdx == 0) ? totalLength : cumulativeDistances[nextIdx];
        float segDist = d1 - d0;
        if (segDist < 1e-4f) return static_cast<float>(i);

        float frac = (dist - d0) / segDist;
        return static_cast<float>(i) + frac;
    }

    // Sample position, forward tangent, up vector, environment zone, and target speed
    void sample(float distance, m3d::Vec3& outPos, m3d::Vec3& outForward, m3d::Vec3& outUp,
                EnvironmentZone& outZone, float& outTargetSpeed) const {
        float t = distanceToT(distance);
        outPos = evaluateSpline(t);

        // Compute smooth forward direction from small delta forward
        float deltaT = 0.02f;
        m3d::Vec3 nextP = evaluateSpline(t + deltaT);
        outForward = (nextP - outPos).normalized();

        // Standard world up
        outUp = m3d::Vec3(0.0f, 1.0f, 0.0f);

        // Zone and target speed interpolation
        int n = static_cast<int>(waypoints.size());
        int i = static_cast<int>(t) % n;
        int nextI = (i + 1) % n;
        float frac = t - std::floor(t);

        outZone = (frac < 0.5f) ? waypoints[i].zone : waypoints[nextI].zone;
        outTargetSpeed = waypoints[i].targetSpeed * (1.0f - frac) + waypoints[nextI].targetSpeed * frac;
    }
};

#endif // TRACK_HPP
