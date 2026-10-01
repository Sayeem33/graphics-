#pragma once
#ifndef MESH_HPP
#define MESH_HPP

#include <glad/gl.h>
#include "math3d.hpp"
#include <vector>

struct Vertex {
    m3d::Vec3 position;
    m3d::Vec3 normal;
    m3d::Vec2 texCoords;
    m3d::Vec3 color{1.0f, 1.0f, 1.0f};

    Vertex() = default;
    Vertex(const m3d::Vec3& pos, const m3d::Vec3& norm, const m3d::Vec2& uv, const m3d::Vec3& col = {1.0f, 1.0f, 1.0f})
        : position(pos), normal(norm), texCoords(uv), color(col) {}
};

class Mesh {
public:
    GLuint vao{0};
    GLuint vbo{0};
    GLuint ebo{0};
    GLsizei indexCount{0};
    GLsizei vertexCount{0};
    bool hasIndices{false};

    Mesh() = default;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices = {}) {
        setup(vertices, indices);
    }

    ~Mesh() {
        destroy();
    }

    // Disable copy, allow move
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& o) noexcept 
        : vao(o.vao), vbo(o.vbo), ebo(o.ebo), indexCount(o.indexCount), 
          vertexCount(o.vertexCount), hasIndices(o.hasIndices) {
        o.vao = 0;
        o.vbo = 0;
        o.ebo = 0;
    }
    Mesh& operator=(Mesh&& o) noexcept {
        if (this != &o) {
            destroy();
            vao = o.vao;
            vbo = o.vbo;
            ebo = o.ebo;
            indexCount = o.indexCount;
            vertexCount = o.vertexCount;
            hasIndices = o.hasIndices;
            o.vao = 0;
            o.vbo = 0;
            o.ebo = 0;
        }
        return *this;
    }

    void destroy() {
        if (ebo != 0) { glDeleteBuffers(1, &ebo); ebo = 0; }
        if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
        if (vao != 0) { glDeleteVertexArrays(1, &vao); vao = 0; }
        indexCount = 0;
        vertexCount = 0;
        hasIndices = false;
    }

    void setup(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices = {}) {
        destroy();
        vertexCount = static_cast<GLsizei>(vertices.size());
        hasIndices = !indices.empty();
        indexCount = static_cast<GLsizei>(indices.size());

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        if (hasIndices) {
            glGenBuffers(1, &ebo);
        }

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        if (hasIndices) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        }

        // Layout 0: Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        // Layout 1: Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // Layout 2: TexCoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        // Layout 3: Color
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glBindVertexArray(0);
    }

    void draw() const {
        if (vao == 0) return;
        glBindVertexArray(vao);
        if (hasIndices) {
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        }
        glBindVertexArray(0);
    }
};

// ==========================================
// Procedural Geometry Generators
// ==========================================
class GeometryGenerator {
public:
    // Standard Unit Cube [-0.5, 0.5] with proper normals per face
    static Mesh createCube(m3d::Vec3 color = {1.0f, 1.0f, 1.0f}) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        struct Face {
            m3d::Vec3 normal;
            m3d::Vec3 c1, c2, c3, c4;
        };

        Face faces[6] = {
            // Front (+Z)
            {{ 0,  0,  1}, {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}},
            // Back (-Z)
            {{ 0,  0, -1}, { 0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}},
            // Left (-X)
            {{-1,  0,  0}, {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}},
            // Right (+X)
            {{ 1,  0,  0}, { 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}},
            // Top (+Y)
            {{ 0,  1,  0}, {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}},
            // Bottom (-Y)
            {{ 0, -1,  0}, {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}}
        };

        for (int i = 0; i < 6; ++i) {
            GLuint base = static_cast<GLuint>(vertices.size());
            vertices.emplace_back(faces[i].c1, faces[i].normal, m3d::Vec2(0.0f, 0.0f), color);
            vertices.emplace_back(faces[i].c2, faces[i].normal, m3d::Vec2(1.0f, 0.0f), color);
            vertices.emplace_back(faces[i].c3, faces[i].normal, m3d::Vec2(1.0f, 1.0f), color);
            vertices.emplace_back(faces[i].c4, faces[i].normal, m3d::Vec2(0.0f, 1.0f), color);

            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }

        return Mesh(vertices, indices);
    }

    // Cylinder along Y-axis, centered at origin, height = 1, radius = 0.5
    static Mesh createCylinder(int segments = 24, m3d::Vec3 color = {1.0f, 1.0f, 1.0f}) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        float halfH = 0.5f;
        float radius = 0.5f;

        // Side vertices
        for (int i = 0; i <= segments; ++i) {
            float theta = (float)i / (float)segments * m3d::TWO_PI;
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);

            m3d::Vec3 norm(cosT, 0.0f, sinT);
            float u = (float)i / (float)segments;

            vertices.emplace_back(m3d::Vec3(radius * cosT, -halfH, radius * sinT), norm, m3d::Vec2(u, 0.0f), color);
            vertices.emplace_back(m3d::Vec3(radius * cosT,  halfH, radius * sinT), norm, m3d::Vec2(u, 1.0f), color);
        }

        for (int i = 0; i < segments; ++i) {
            GLuint i0 = i * 2;
            GLuint i1 = i * 2 + 1;
            GLuint i2 = (i + 1) * 2;
            GLuint i3 = (i + 1) * 2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }

        // Top cap (+Y)
        GLuint topCenterIdx = static_cast<GLuint>(vertices.size());
        vertices.emplace_back(m3d::Vec3(0.0f, halfH, 0.0f), m3d::Vec3(0.0f, 1.0f, 0.0f), m3d::Vec2(0.5f, 0.5f), color);
        GLuint topStartIdx = static_cast<GLuint>(vertices.size());

        for (int i = 0; i <= segments; ++i) {
            float theta = (float)i / (float)segments * m3d::TWO_PI;
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);
            vertices.emplace_back(m3d::Vec3(radius * cosT, halfH, radius * sinT), m3d::Vec3(0.0f, 1.0f, 0.0f),
                                  m3d::Vec2(0.5f + 0.5f * cosT, 0.5f + 0.5f * sinT), color);
        }

        for (int i = 0; i < segments; ++i) {
            indices.push_back(topCenterIdx);
            indices.push_back(topStartIdx + i);
            indices.push_back(topStartIdx + i + 1);
        }

        // Bottom cap (-Y)
        GLuint botCenterIdx = static_cast<GLuint>(vertices.size());
        vertices.emplace_back(m3d::Vec3(0.0f, -halfH, 0.0f), m3d::Vec3(0.0f, -1.0f, 0.0f), m3d::Vec2(0.5f, 0.5f), color);
        GLuint botStartIdx = static_cast<GLuint>(vertices.size());

        for (int i = 0; i <= segments; ++i) {
            float theta = (float)i / (float)segments * m3d::TWO_PI;
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);
            vertices.emplace_back(m3d::Vec3(radius * cosT, -halfH, radius * sinT), m3d::Vec3(0.0f, -1.0f, 0.0f),
                                  m3d::Vec2(0.5f + 0.5f * cosT, 0.5f + 0.5f * sinT), color);
        }

        for (int i = 0; i < segments; ++i) {
            indices.push_back(botCenterIdx);
            indices.push_back(botStartIdx + i + 1);
            indices.push_back(botStartIdx + i);
        }

        return Mesh(vertices, indices);
    }

    // Cone along Y-axis, centered at origin, height = 1, base radius = 0.5
    static Mesh createCone(int segments = 24, m3d::Vec3 color = {1.0f, 1.0f, 1.0f}) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        float halfH = 0.5f;
        float radius = 0.5f;

        // Apex vertex
        m3d::Vec3 apex(0.0f, halfH, 0.0f);

        for (int i = 0; i < segments; ++i) {
            float t0 = (float)i / (float)segments * m3d::TWO_PI;
            float t1 = (float)(i + 1) / (float)segments * m3d::TWO_PI;

            m3d::Vec3 p0(radius * std::cos(t0), -halfH, radius * std::sin(t0));
            m3d::Vec3 p1(radius * std::cos(t1), -halfH, radius * std::sin(t1));

            // Face normal
            m3d::Vec3 edge1 = p0 - apex;
            m3d::Vec3 edge2 = p1 - apex;
            m3d::Vec3 norm = m3d::cross(edge1, edge2).normalized();

            GLuint base = static_cast<GLuint>(vertices.size());
            vertices.emplace_back(apex, norm, m3d::Vec2(0.5f, 1.0f), color);
            vertices.emplace_back(p0, norm, m3d::Vec2(0.0f, 0.0f), color);
            vertices.emplace_back(p1, norm, m3d::Vec2(1.0f, 0.0f), color);

            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
        }

        // Base circle (-Y)
        GLuint centerIdx = static_cast<GLuint>(vertices.size());
        vertices.emplace_back(m3d::Vec3(0.0f, -halfH, 0.0f), m3d::Vec3(0.0f, -1.0f, 0.0f), m3d::Vec2(0.5f, 0.5f), color);
        GLuint startIdx = static_cast<GLuint>(vertices.size());

        for (int i = 0; i <= segments; ++i) {
            float theta = (float)i / (float)segments * m3d::TWO_PI;
            vertices.emplace_back(m3d::Vec3(radius * std::cos(theta), -halfH, radius * std::sin(theta)),
                                  m3d::Vec3(0.0f, -1.0f, 0.0f), m3d::Vec2(0.5f, 0.5f), color);
        }

        for (int i = 0; i < segments; ++i) {
            indices.push_back(centerIdx);
            indices.push_back(startIdx + i + 1);
            indices.push_back(startIdx + i);
        }

        return Mesh(vertices, indices);
    }

    // Sphere with radius = 0.5
    static Mesh createSphere(int rings = 16, int sectors = 24, m3d::Vec3 color = {1.0f, 1.0f, 1.0f}) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        float radius = 0.5f;

        for (int r = 0; r <= rings; ++r) {
            float phi = (float)r / (float)rings * m3d::PI; // 0 to PI
            float sinP = std::sin(phi);
            float cosP = std::cos(phi);

            for (int s = 0; s <= sectors; ++s) {
                float theta = (float)s / (float)sectors * m3d::TWO_PI; // 0 to 2PI
                float sinT = std::sin(theta);
                float cosT = std::cos(theta);

                m3d::Vec3 pos(radius * sinP * cosT, radius * cosP, radius * sinP * sinT);
                m3d::Vec3 norm = pos.normalized();
                m3d::Vec2 uv((float)s / (float)sectors, (float)r / (float)rings);

                vertices.emplace_back(pos, norm, uv, color);
            }
        }

        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < sectors; ++s) {
                GLuint i0 = r * (sectors + 1) + s;
                GLuint i1 = (r + 1) * (sectors + 1) + s;
                GLuint i2 = (r + 1) * (sectors + 1) + (s + 1);
                GLuint i3 = r * (sectors + 1) + (s + 1);

                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);

                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }

        return Mesh(vertices, indices);
    }

    // Horizontal Plane in XZ, centered at origin, size 1x1
    static Mesh createPlane(m3d::Vec3 color = {1.0f, 1.0f, 1.0f}) {
        std::vector<Vertex> vertices = {
            { {-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, color },
            { { 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, color },
            { { 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, color },
            { {-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, color }
        };

        std::vector<GLuint> indices = {
            0, 1, 2,
            0, 2, 3
        };

        return Mesh(vertices, indices);
    }

    // =========================================================================
    // Curved Horseshoe Tunnel Vault with Inward-Facing Normals
    // (Matches Reference Image 2: Smooth cylindrical concrete vault with ceiling arc)
    // =========================================================================
    static Mesh createTunnelVault(float radius, float wallHeight, float length, 
                                  int radialSegments = 24, int lengthSegments = 30, 
                                  m3d::Vec3 color = {0.35f, 0.36f, 0.38f}) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // Profile points in 2D (X, Y): Left vertical wall -> Semicircular arch -> Right vertical wall
        // Total profile points = 1 (left base) + radialSegments + 1 (right base)
        struct ProfilePoint {
            float x, y;
            m3d::Vec3 normal; // Inward facing normal
            float u;
        };

        std::vector<ProfilePoint> profile;
        profile.push_back({ -radius, 0.0f, m3d::Vec3(1.0f, 0.0f, 0.0f), 0.0f }); // Left base

        // Semicircular vaulted arch from angle PI (left) down to 0 (right)
        for (int i = 0; i <= radialSegments; ++i) {
            float angle = m3d::PI - (float)i / (float)radialSegments * m3d::PI;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            float px = radius * cosA;
            float py = wallHeight + radius * sinA;

            // Inward normal points toward center of arch
            m3d::Vec3 norm(-cosA, -sinA, 0.0f);
            float u = (float)(i + 1) / (float)(radialSegments + 2);
            profile.push_back({ px, py, norm, u });
        }

        profile.push_back({ radius, 0.0f, m3d::Vec3(-1.0f, 0.0f, 0.0f), 1.0f }); // Right base

        int numPointsPerRing = static_cast<int>(profile.size());

        // Generate rings along length (Z axis from 0 to -length)
        for (int z = 0; z <= lengthSegments; ++z) {
            float pz = -((float)z / (float)lengthSegments) * length;
            float v = (float)z / (float)lengthSegments * 10.0f; // repeated UV along tunnel

            for (int p = 0; p < numPointsPerRing; ++p) {
                const auto& pt = profile[p];
                m3d::Vec3 pos(pt.x, pt.y, pz);
                vertices.emplace_back(pos, pt.normal, m3d::Vec2(pt.u, v), color);
            }
        }

        // Connect rings with inward-facing triangles
        for (int z = 0; z < lengthSegments; ++z) {
            int ring0 = z * numPointsPerRing;
            int ring1 = (z + 1) * numPointsPerRing;

            for (int p = 0; p < numPointsPerRing - 1; ++p) {
                GLuint i0 = ring0 + p;
                GLuint i1 = ring0 + p + 1;
                GLuint i2 = ring1 + p;
                GLuint i3 = ring1 + p + 1;

                // Counter-clockwise for inward visibility
                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }

        return Mesh(vertices, indices);
    }

    // =========================================================================
    // Rustic Stone Masonry Portal Arch
    // (Matches Reference Image 1: Radiating stone voussoirs / keystones & stone facade)
    // =========================================================================
    static Mesh createStonePortalArch(float innerRadius, float outerRadius, float wallWidth, 
                                      float wallHeight, float depth, int archSegments = 16, 
                                      m3d::Vec3 stoneColor = {0.45f, 0.44f, 0.42f}) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        m3d::Vec3 frontNorm(0.0f, 0.0f, 1.0f);
        m3d::Vec3 backNorm(0.0f, 0.0f, -1.0f);

        // 1. Radiating Arch Keystone / Voussoir Ring (Front face)
        for (int i = 0; i < archSegments; ++i) {
            float a0 = (float)i / (float)archSegments * m3d::PI;
            float a1 = (float)(i + 1) / (float)archSegments * m3d::PI;

            // Keystone wedge vertex positions
            m3d::Vec3 pInner0(innerRadius * std::cos(a0), innerRadius * std::sin(a0), depth * 0.5f);
            m3d::Vec3 pOuter0(outerRadius * std::cos(a0), outerRadius * std::sin(a0), depth * 0.5f);
            m3d::Vec3 pInner1(innerRadius * std::cos(a1), innerRadius * std::sin(a1), depth * 0.5f);
            m3d::Vec3 pOuter1(outerRadius * std::cos(a1), outerRadius * std::sin(a1), depth * 0.5f);

            // Keystone subtle shading variation (center keystone is slightly larger/highlighted)
            float shade = 0.85f + 0.15f * std::sin((float)i / archSegments * m3d::PI);
            m3d::Vec3 kCol = stoneColor * shade;

            GLuint base = static_cast<GLuint>(vertices.size());
            vertices.emplace_back(pInner0, frontNorm, m3d::Vec2(0, 0), kCol);
            vertices.emplace_back(pOuter0, frontNorm, m3d::Vec2(0, 1), kCol);
            vertices.emplace_back(pOuter1, frontNorm, m3d::Vec2(1, 1), kCol);
            vertices.emplace_back(pInner1, frontNorm, m3d::Vec2(1, 0), kCol);

            // Keystone front face (Counter-Clockwise)
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 1);

            indices.push_back(base + 0);
            indices.push_back(base + 3);
            indices.push_back(base + 2);

            // Intrados (inner curved arch barrel surface)
            m3d::Vec3 inNorm0(-std::cos(a0), -std::sin(a0), 0.0f);
            m3d::Vec3 inNorm1(-std::cos(a1), -std::sin(a1), 0.0f);

            m3d::Vec3 pInner0Back(pInner0.x, pInner0.y, -depth * 0.5f);
            m3d::Vec3 pInner1Back(pInner1.x, pInner1.y, -depth * 0.5f);

            GLuint inBase = static_cast<GLuint>(vertices.size());
            vertices.emplace_back(pInner0, inNorm0, m3d::Vec2(0, 0), stoneColor * 0.7f);
            vertices.emplace_back(pInner1, inNorm1, m3d::Vec2(1, 0), stoneColor * 0.7f);
            vertices.emplace_back(pInner1Back, inNorm1, m3d::Vec2(1, 1), stoneColor * 0.7f);
            vertices.emplace_back(pInner0Back, inNorm0, m3d::Vec2(0, 1), stoneColor * 0.7f);

            indices.push_back(inBase + 0);
            indices.push_back(inBase + 2);
            indices.push_back(inBase + 1);

            indices.push_back(inBase + 0);
            indices.push_back(inBase + 3);
            indices.push_back(inBase + 2);
        }

        // 2. Surrounding Heavy Stone Wall Blocks (Counter-Clockwise)
        float halfW = wallWidth * 0.5f;
        float h = wallHeight;

        // Left masonry flank
        {
            m3d::Vec3 p0(-halfW, 0.0f, depth * 0.5f);
            m3d::Vec3 p1(-outerRadius, 0.0f, depth * 0.5f);
            m3d::Vec3 p2(-outerRadius, h, depth * 0.5f);
            m3d::Vec3 p3(-halfW, h, depth * 0.5f);

            GLuint base = static_cast<GLuint>(vertices.size());
            vertices.emplace_back(p0, frontNorm, m3d::Vec2(0, 0), stoneColor);
            vertices.emplace_back(p1, frontNorm, m3d::Vec2(1, 0), stoneColor);
            vertices.emplace_back(p2, frontNorm, m3d::Vec2(1, 1), stoneColor);
            vertices.emplace_back(p3, frontNorm, m3d::Vec2(0, 1), stoneColor);

            indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
            indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
        }

        // Right masonry flank
        {
            m3d::Vec3 p0(outerRadius, 0.0f, depth * 0.5f);
            m3d::Vec3 p1(halfW, 0.0f, depth * 0.5f);
            m3d::Vec3 p2(halfW, h, depth * 0.5f);
            m3d::Vec3 p3(outerRadius, h, depth * 0.5f);

            GLuint base = static_cast<GLuint>(vertices.size());
            vertices.emplace_back(p0, frontNorm, m3d::Vec2(0, 0), stoneColor);
            vertices.emplace_back(p1, frontNorm, m3d::Vec2(1, 0), stoneColor);
            vertices.emplace_back(p2, frontNorm, m3d::Vec2(1, 1), stoneColor);
            vertices.emplace_back(p3, frontNorm, m3d::Vec2(0, 1), stoneColor);

            indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
            indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
        }

        // Top masonry spandrel & parapet wall above the arch
        {
            m3d::Vec3 p0(-halfW, outerRadius, depth * 0.5f);
            m3d::Vec3 p1(halfW, outerRadius, depth * 0.5f);
            m3d::Vec3 p2(halfW, h, depth * 0.5f);
            m3d::Vec3 p3(-halfW, h, depth * 0.5f);

            GLuint base = static_cast<GLuint>(vertices.size());
            vertices.emplace_back(p0, frontNorm, m3d::Vec2(0, 0), stoneColor * 0.95f);
            vertices.emplace_back(p1, frontNorm, m3d::Vec2(1, 0), stoneColor * 0.95f);
            vertices.emplace_back(p2, frontNorm, m3d::Vec2(1, 1), stoneColor * 0.95f);
            vertices.emplace_back(p3, frontNorm, m3d::Vec2(0, 1), stoneColor * 0.95f);

            indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 1);
            indices.push_back(base + 0); indices.push_back(base + 3); indices.push_back(base + 2);
        }

        return Mesh(vertices, indices);
    }
};

#endif // MESH_HPP
