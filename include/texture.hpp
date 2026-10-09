#pragma once
#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glad/gl.h>
#include "math3d.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <algorithm>

// =================================================================
// 1. OpenGL 2D Texture Wrapper
// =================================================================
class Texture {
public:
    GLuint id{0};
    int width{0};
    int height{0};
    int channels{0};
    bool isValid{false};

    Texture() = default;

    ~Texture() {
        destroy();
    }

    // Move semantics (disable copy)
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& o) noexcept
        : id(o.id), width(o.width), height(o.height), channels(o.channels), isValid(o.isValid) {
        o.id = 0;
        o.isValid = false;
    }

    Texture& operator=(Texture&& o) noexcept {
        if (this != &o) {
            destroy();
            id = o.id;
            width = o.width;
            height = o.height;
            channels = o.channels;
            isValid = o.isValid;
            o.id = 0;
            o.isValid = false;
        }
        return *this;
    }

    void destroy() {
        if (id != 0) {
            glDeleteTextures(1, &id);
            id = 0;
        }
        isValid = false;
    }

    void bind(GLuint unit = 0) const {
        if (id != 0) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, id);
        }
    }

    void unbind(GLuint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Create OpenGL texture from raw RGB/RGBA byte buffer
    static Texture createFromBytes(int w, int h, int ch, const unsigned char* data,
                                  bool generateMipmaps = true, GLenum wrapMode = GL_REPEAT) {
        Texture tex;
        tex.width = w;
        tex.height = h;
        tex.channels = ch;

        glGenTextures(1, &tex.id);
        glBindTexture(GL_TEXTURE_2D, tex.id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

        if (generateMipmaps) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GLenum format = (ch == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);

        if (generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        tex.isValid = true;
        return tex;
    }

    // Load standard 24-bit or 32-bit BMP from file
    static Texture loadBMP(const std::string& filepath) {
        #pragma pack(push, 1)
        struct BMPHeader {
            uint16_t fileType;
            uint32_t fileSize;
            uint16_t reserved1;
            uint16_t reserved2;
            uint32_t offsetData;
            uint32_t sizeHeader;
            int32_t  width;
            int32_t  height;
            uint16_t planes;
            uint16_t bitCount;
            uint32_t compression;
            uint32_t sizeImage;
            int32_t  xPelsPerMeter;
            int32_t  yPelsPerMeter;
            uint32_t clrUsed;
            uint32_t clrImportant;
        };
        #pragma pack(pop)

        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            return Texture(); // invalid
        }

        BMPHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
        if (!file || header.fileType != 0x4D42 || (header.bitCount != 24 && header.bitCount != 32)) {
            return Texture();
        }

        int w = header.width;
        int h = std::abs(header.height);
        bool isTopDown = (header.height < 0);

        std::vector<unsigned char> rgb(w * h * 3);
        file.seekg(header.offsetData, std::ios::beg);

        int rowStride = ((w * (header.bitCount / 8) + 3) / 4) * 4;
        std::vector<unsigned char> rowBuffer(rowStride);

        for (int row = 0; row < h; ++row) {
            file.read(reinterpret_cast<char*>(rowBuffer.data()), rowStride);
            int dstRow = isTopDown ? (h - 1 - row) : row; // OpenGL is bottom-up

            for (int col = 0; col < w; ++col) {
                int srcIdx = col * (header.bitCount / 8);
                int dstIdx = (dstRow * w + col) * 3;
                rgb[dstIdx + 0] = rowBuffer[srcIdx + 2]; // R
                rgb[dstIdx + 1] = rowBuffer[srcIdx + 1]; // G
                rgb[dstIdx + 2] = rowBuffer[srcIdx + 0]; // B
            }
        }

        return createFromBytes(w, h, 3, rgb.data(), true, GL_REPEAT);
    }

    // Save RGB byte array to 24-bit BMP file
    static bool saveBMP(const std::string& filepath, int w, int h, const unsigned char* rgb) {
        #pragma pack(push, 1)
        struct BMPHeader {
            uint16_t fileType{0x4D42};
            uint32_t fileSize{0};
            uint16_t reserved1{0};
            uint16_t reserved2{0};
            uint32_t offsetData{54};
            uint32_t sizeHeader{40};
            int32_t  width{0};
            int32_t  height{0};
            uint16_t planes{1};
            uint16_t bitCount{24};
            uint32_t compression{0};
            uint32_t sizeImage{0};
            int32_t  xPelsPerMeter{2835};
            int32_t  yPelsPerMeter{2835};
            uint32_t clrUsed{0};
            uint32_t clrImportant{0};
        };
        #pragma pack(pop)

        int rowStride = ((w * 3 + 3) / 4) * 4;
        uint32_t dataSize = rowStride * h;

        BMPHeader header;
        header.fileSize = 54 + dataSize;
        header.width = w;
        header.height = h; // bottom-up
        header.sizeImage = dataSize;

        std::ofstream file(filepath, std::ios::binary);
        if (!file) return false;

        file.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));

        std::vector<unsigned char> rowBuffer(rowStride, 0);
        for (int row = 0; row < h; ++row) {
            for (int col = 0; col < w; ++col) {
                int srcIdx = (row * w + col) * 3;
                int dstIdx = col * 3;
                rowBuffer[dstIdx + 0] = rgb[srcIdx + 2]; // B
                rowBuffer[dstIdx + 1] = rgb[srcIdx + 1]; // G
                rowBuffer[dstIdx + 2] = rgb[srcIdx + 0]; // R
            }
            file.write(reinterpret_cast<const char*>(rowBuffer.data()), rowStride);
        }
        return true;
    }
};

// =================================================================
// 2. High-Fidelity Procedural Texture Synthesis
// =================================================================
class TextureGenerator {
private:
    static inline float hash21(float x, float y) {
        float n = std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
        return n - std::floor(n);
    }

    static inline float smoothNoise(float x, float y) {
        float ix = std::floor(x);
        float iy = std::floor(y);
        float fx = x - ix;
        float fy = y - iy;
        float ux = fx * fx * (3.0f - 2.0f * fx);
        float uy = fy * fy * (3.0f - 2.0f * fy);
        float a = hash21(ix, iy);
        float b = hash21(ix + 1.0f, iy);
        float c = hash21(ix, iy + 1.0f);
        float d = hash21(ix + 1.0f, iy + 1.0f);
        return a * (1.0f - ux) * (1.0f - uy) +
               b * ux * (1.0f - uy) +
               c * (1.0f - ux) * uy +
               d * ux * uy;
    }

    // Seamless periodic noise by sampling a 4D torus
    static inline float seamlessNoise(float u, float v, float freq) {
        float twoPi = 6.2831853f;
        float nx = std::cos(u * twoPi) * freq;
        float ny = std::sin(u * twoPi) * freq;
        float nz = std::cos(v * twoPi) * freq;
        float nw = std::sin(v * twoPi) * freq;
        return smoothNoise(nx + nz, ny + nw);
    }

public:
    // [A] Asphalt Road Texture: Speckled bitumen, fine stone granules, gravel aggregate
    static std::vector<unsigned char> generateAsphalt(int w = 256, int h = 256) {
        std::vector<unsigned char> data(w * h * 3);
        for (int y = 0; y < h; ++y) {
            float v = (float)y / (float)h;
            for (int x = 0; x < w; ++x) {
                float u = (float)x / (float)w;

                // Multi-octave seamless noise
                float n1 = seamlessNoise(u, v, 4.0f);
                float n2 = seamlessNoise(u, v, 12.0f);
                float n3 = seamlessNoise(u, v, 32.0f);
                float speckle = hash21((float)x, (float)y);

                float aggregate = n1 * 0.45f + n2 * 0.35f + n3 * 0.20f;
                // Base bitumen luminance
                float lum = 46.0f + aggregate * 28.0f;

                // Occasional light quartz flecks
                if (speckle > 0.88f) {
                    lum += (speckle - 0.88f) * 320.0f;
                } else if (speckle < 0.08f) {
                    lum -= 16.0f; // Tar pit dark spot
                }

                lum = std::max(18.0f, std::min(240.0f, lum));

                int idx = (y * w + x) * 3;
                data[idx + 0] = static_cast<unsigned char>(lum * 0.98f); // R
                data[idx + 1] = static_cast<unsigned char>(lum * 1.00f); // G
                data[idx + 2] = static_cast<unsigned char>(lum * 1.04f); // B (subtle cool slate)
            }
        }
        return data;
    }

    // [B] Grass / Meadow Turf: Vibrant organic lawn blades, subtle clover & earthy undertones
    static std::vector<unsigned char> generateGrass(int w = 256, int h = 256) {
        std::vector<unsigned char> data(w * h * 3);
        for (int y = 0; y < h; ++y) {
            float v = (float)y / (float)h;
            for (int x = 0; x < w; ++x) {
                float u = (float)x / (float)w;

                float patch = seamlessNoise(u, v, 3.0f);
                float blade1 = seamlessNoise(u, v, 16.0f);
                float blade2 = seamlessNoise(u, v, 40.0f);
                float fine = hash21((float)x * 1.5f, (float)y * 1.5f);

                float grassFactor = patch * 0.35f + blade1 * 0.40f + blade2 * 0.15f + fine * 0.10f;

                // Color gradient from warm olive to lush emerald
                float r = 42.0f + patch * 28.0f + fine * 12.0f;
                float g = 105.0f + grassFactor * 55.0f;
                float b = 32.0f + patch * 18.0f;

                int idx = (y * w + x) * 3;
                data[idx + 0] = static_cast<unsigned char>(std::min(255.0f, r));
                data[idx + 1] = static_cast<unsigned char>(std::min(255.0f, g));
                data[idx + 2] = static_cast<unsigned char>(std::min(255.0f, b));
            }
        }
        return data;
    }

    // [C] River Water Ripple Caustics: Fluid wave harmonics, specular shimmer
    static std::vector<unsigned char> generateRiverWater(int w = 256, int h = 256) {
        std::vector<unsigned char> data(w * h * 3);
        for (int y = 0; y < h; ++y) {
            float v = (float)y / (float)h;
            for (int x = 0; x < w; ++x) {
                float u = (float)x / (float)w;

                float w1 = seamlessNoise(u, v, 5.0f);
                float w2 = seamlessNoise(u + 0.3f, v + 0.2f, 9.0f);
                float w3 = seamlessNoise(u - 0.2f, v + 0.4f, 18.0f);

                float ripple = (w1 * 0.5f + w2 * 0.35f + w3 * 0.15f);
                // Sharp caustic highlight
                float caustic = std::pow(ripple, 2.4f) * 1.8f;

                float r = 24.0f + caustic * 80.0f;
                float g = 95.0f + caustic * 120.0f + ripple * 20.0f;
                float b = 175.0f + caustic * 75.0f;

                int idx = (y * w + x) * 3;
                data[idx + 0] = static_cast<unsigned char>(std::min(255.0f, r));
                data[idx + 1] = static_cast<unsigned char>(std::min(255.0f, g));
                data[idx + 2] = static_cast<unsigned char>(std::min(255.0f, b));
            }
        }
        return data;
    }

    // [D] Stone Masonry / Tunnel Rock: Chiseled granite blocks with mortar channels
    static std::vector<unsigned char> generateStoneMasonry(int w = 256, int h = 256) {
        std::vector<unsigned char> data(w * h * 3);
        int numRows = 8;
        int rowH = h / numRows;

        for (int y = 0; y < h; ++y) {
            int rowIdx = y / rowH;
            int inRowY = y % rowH;
            float v = (float)y / (float)h;

            // Stagger every other row by half-block
            int xOffset = (rowIdx % 2 == 1) ? (w / 8) : 0;

            for (int x = 0; x < w; ++x) {
                float u = (float)x / (float)w;
                int shiftedX = (x + xOffset) % w;
                int blockW = w / 4;
                int inBlockX = shiftedX % blockW;

                // Mortar lines (2 pixels margin)
                bool isMortar = (inRowY < 3 || inRowY > rowH - 3 || inBlockX < 3 || inBlockX > blockW - 3);

                float grain = seamlessNoise(u, v, 14.0f);
                float fine = hash21((float)x, (float)y) * 20.0f;

                float r, g, b;
                if (isMortar) {
                    // Dark cement mortar
                    r = 55.0f + grain * 15.0f;
                    g = 53.0f + grain * 15.0f;
                    b = 50.0f + grain * 15.0f;
                } else {
                    // Dressed stone block face with per-block tint variation
                    float blockTint = hash21((float)(shiftedX / blockW), (float)rowIdx);
                    float baseLum = 110.0f + blockTint * 35.0f + grain * 30.0f + fine;
                    r = baseLum * 1.02f;
                    g = baseLum * 0.98f;
                    b = baseLum * 0.94f; // Warm granite stone
                }

                int idx = (y * w + x) * 3;
                data[idx + 0] = static_cast<unsigned char>(std::min(255.0f, r));
                data[idx + 1] = static_cast<unsigned char>(std::min(255.0f, g));
                data[idx + 2] = static_cast<unsigned char>(std::min(255.0f, b));
            }
        }
        return data;
    }

    // [E] Building Facade / Glass Windows: Modern architectural steel grid & illuminated office panes
    static std::vector<unsigned char> generateBuildingFacade(int w = 256, int h = 256) {
        std::vector<unsigned char> data(w * h * 3);
        int floorH = 32;
        int winW = 24;

        for (int y = 0; y < h; ++y) {
            int floorIdx = y / floorH;
            int inFloorY = y % floorH;

            for (int x = 0; x < w; ++x) {
                int colIdx = x / winW;
                int inWinX = x % winW;

                // Frame pillars & spandrel beams (3px frame)
                bool isFrame = (inFloorY < 4 || inFloorY > floorH - 4 || inWinX < 3 || inWinX > winW - 3);

                float r, g, b;
                if (isFrame) {
                    // Dark matte charcoal/gunmetal steel frame
                    r = 45.0f; g = 48.0f; b = 54.0f;
                } else {
                    // Window pane: deterministic lit vs reflective glass
                    float litRand = hash21((float)colIdx * 3.1f, (float)floorIdx * 7.7f);
                    float glassGrain = hash21((float)x, (float)y) * 8.0f;

                    if (litRand > 0.65f) {
                        // Warm interior office lights
                        r = 235.0f + glassGrain;
                        g = 205.0f + glassGrain;
                        b = 135.0f + glassGrain;
                    } else {
                        // Glossy sky-reflective architectural glass
                        r = 38.0f + glassGrain;
                        g = 68.0f + glassGrain;
                        b = 98.0f + glassGrain;
                    }
                }

                int idx = (y * w + x) * 3;
                data[idx + 0] = static_cast<unsigned char>(std::min(255.0f, r));
                data[idx + 1] = static_cast<unsigned char>(std::min(255.0f, g));
                data[idx + 2] = static_cast<unsigned char>(std::min(255.0f, b));
            }
        }
        return data;
    }

    // [F] Concrete Sidewalk / Curb: Urban brushed concrete with subtle sand grain
    static std::vector<unsigned char> generateConcrete(int w = 256, int h = 256) {
        std::vector<unsigned char> data(w * h * 3);
        for (int y = 0; y < h; ++y) {
            float v = (float)y / (float)h;
            for (int x = 0; x < w; ++x) {
                float u = (float)x / (float)w;

                float n1 = seamlessNoise(u, v, 6.0f);
                float n2 = seamlessNoise(u, v, 24.0f);
                float sand = hash21((float)x, (float)y) * 16.0f;

                float lum = 155.0f + n1 * 25.0f + n2 * 15.0f + sand;

                int idx = (y * w + x) * 3;
                data[idx + 0] = static_cast<unsigned char>(std::min(255.0f, lum * 0.99f));
                data[idx + 1] = static_cast<unsigned char>(std::min(255.0f, lum * 1.00f));
                data[idx + 2] = static_cast<unsigned char>(std::min(255.0f, lum * 0.98f));
            }
        }
        return data;
    }

    // [G] Carbon Fiber / Composite Weave: 2x2 diagonal twill weave pattern with subtle anisotropic sheen
    static std::vector<unsigned char> generateCarbonFiber(int w = 512, int h = 512) {
        std::vector<unsigned char> data(w * h * 3);
        int twillSize = 16;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                // 2x2 twill diagonal pattern
                int cellX = (x / (twillSize / 4)) % 4;
                int cellY = (y / (twillSize / 4)) % 4;
                bool isHorizontal = ((cellX + cellY) % 4 < 2);

                float microNoise = hash21((float)x, (float)y) * 12.0f;
                float threadPattern = isHorizontal ? std::sin((float)y * 0.8f) : std::sin((float)x * 0.8f);
                float lum = 28.0f + (isHorizontal ? 18.0f : 0.0f) + threadPattern * 10.0f + microNoise;
                lum = std::max(12.0f, std::min(75.0f, lum));

                int idx = (y * w + x) * 3;
                data[idx + 0] = static_cast<unsigned char>(lum * 0.95f);
                data[idx + 1] = static_cast<unsigned char>(lum * 0.98f);
                data[idx + 2] = static_cast<unsigned char>(lum * 1.05f); // Subtle carbon blue tint
            }
        }
        return data;
    }
};

// =================================================================
// 3. Central Texture Manager
// =================================================================
class TextureManager {
public:
    std::unordered_map<std::string, Texture> textures;
    bool isInitialized{false};

    void init() {
        if (isInitialized) return;

        struct TexDef {
            std::string name;
            std::string filename;
            std::vector<unsigned char> (*generator)(int, int);
            int w;
            int h;
        };

        TexDef defs[] = {
            { "asphalt",  "assets/textures/asphalt.bmp",  TextureGenerator::generateAsphalt,        512, 512 },
            { "grass",    "assets/textures/grass.bmp",    TextureGenerator::generateGrass,          512, 512 },
            { "water",    "assets/textures/water.bmp",    TextureGenerator::generateRiverWater,     512, 512 },
            { "stone",    "assets/textures/stone.bmp",    TextureGenerator::generateStoneMasonry,   512, 512 },
            { "building", "assets/textures/building.bmp", TextureGenerator::generateBuildingFacade, 512, 512 },
            { "concrete", "assets/textures/concrete.bmp", TextureGenerator::generateConcrete,       512, 512 },
            { "carbon",   "assets/textures/carbon.bmp",   TextureGenerator::generateCarbonFiber,    512, 512 }
        };

        for (const auto& def : defs) {
            Texture tex = Texture::loadBMP(def.filename);
            if (!tex.isValid) {
                // Generate procedural texture data
                std::vector<unsigned char> data = def.generator(def.w, def.h);
                // Save to file for user inspection / editing
                Texture::saveBMP(def.filename, def.w, def.h, data.data());
                // Create OpenGL texture
                tex = Texture::createFromBytes(def.w, def.h, 3, data.data(), true, GL_REPEAT);
            }
            textures[def.name] = std::move(tex);
        }

        isInitialized = true;
    }

    void bind(const std::string& name, GLuint unit = 0) const {
        auto it = textures.find(name);
        if (it != textures.end() && it->second.isValid) {
            it->second.bind(unit);
        }
    }

    void unbind(GLuint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool has(const std::string& name) const {
        auto it = textures.find(name);
        return (it != textures.end() && it->second.isValid);
    }
};

#endif // TEXTURE_HPP
