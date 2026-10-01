#pragma once
#ifndef SCREENSHOT_HPP
#define SCREENSHOT_HPP

#include <glad/gl.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdint>

class ScreenshotUtil {
public:
    static bool saveBMP(const char* filename, int width, int height) {
        std::vector<unsigned char> pixels(width * height * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

        // BMP Header structures
        #pragma pack(push, 1)
        struct BMPFileHeader {
            uint16_t fileType{0x4D42}; // 'BM'
            uint32_t fileSize{0};
            uint16_t reserved1{0};
            uint16_t reserved2{0};
            uint32_t offsetData{54};
        };

        struct BMPInfoHeader {
            uint32_t size{40};
            int32_t  width{0};
            int32_t  height{0};
            uint16_t planes{1};
            uint16_t bitCount{24};
            uint32_t compression{0};
            uint32_t sizeImage{0};
            int32_t  xPixelsPerMeter{0};
            int32_t  yPixelsPerMeter{0};
            uint32_t colorsUsed{0};
            uint32_t colorsImportant{0};
        };
        #pragma pack(pop)

        BMPFileHeader fileHeader;
        BMPInfoHeader infoHeader;

        uint32_t rowStride = ((width * 3 + 3) / 4) * 4;
        uint32_t dataSize = rowStride * height;

        fileHeader.fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + dataSize;
        infoHeader.width = width;
        infoHeader.height = height;
        infoHeader.sizeImage = dataSize;

        std::ofstream out(filename, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open screenshot file for writing: " << filename << std::endl;
            return false;
        }

        out.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
        out.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

        std::vector<unsigned char> rowBuffer(rowStride, 0);
        for (int y = 0; y < height; ++y) {
            const unsigned char* srcRow = &pixels[y * width * 3];
            std::copy(srcRow, srcRow + width * 3, rowBuffer.begin());
            out.write(reinterpret_cast<const char*>(rowBuffer.data()), rowStride);
        }

        out.close();
        std::cout << "[SCREENSHOT] Saved: " << filename << " (" << width << "x" << height << ")" << std::endl;
        return true;
    }
};

#endif // SCREENSHOT_HPP
