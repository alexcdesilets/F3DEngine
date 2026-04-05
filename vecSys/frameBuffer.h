#pragma once

#include <fstream>
#include "base.h"
#include "vecRGBA.h"
#include "vec2.h"

namespace vecSys {
    struct FrameBuffer {
        std::vector<vecRGBA> accumulationBuffer;
        std::vector<uint32_t> displayPixels;
        int width = 0, height = 0;

        void init(int wdth, int hght) {
            width = wdth;
            height = hght;
            accumulationBuffer.resize(width * height, vecRGBA::TransparentBlack());
            displayPixels.resize(width * height);
        }

        void blit(const FrameBuffer& src, int dstX, int dstY) {
            for (int y = 0; y < src.height; ++y) {
                for (int x = 0; x < src.width; ++x) {
                    int targetX = dstX + x;
                    int targetY = dstY + y;
                    if (targetX < 0 || targetX >= width || targetY < 0 || targetY >= height) continue;
                    setPixel(targetX, targetY, src.getPixel(x, y));
                }
            }
        }

        void bufferBlend(const FrameBuffer& other) {
            if (width != other.width || height != other.height || accumulationBuffer.size() != other.accumulationBuffer.size()) {
                throw std::runtime_error("FrameBuffer::bufferBlend - Mismatched buffer dimensions");
            }
        
            for (size_t i = 0; i < accumulationBuffer.size(); ++i) {
                accumulationBuffer[i] = other.accumulationBuffer[i].compositeOver(accumulationBuffer[i]);
            }
        }

        void clear(const vecRGBA& clearColor = vecRGBA::TransparentBlack()) {
            std::fill(accumulationBuffer.begin(), accumulationBuffer.end(), clearColor);
        }

        inline uint32_t convertVecToPixel(const vecRGBA& color) const {
            vecRGBA c = color.clamped();
            uint8_t r = static_cast<uint8_t>(c.r * 255.99);
            uint8_t g = static_cast<uint8_t>(c.g * 255.99);
            uint8_t b = static_cast<uint8_t>(c.b * 255.99);
            uint8_t a = static_cast<uint8_t>(c.a * 255.99);
            
            // OpenGL expects RGBA in memory (little-endian: least-significant byte is first)
            // Correct layout: 0xAABBGGRR
            return (r << 0) | (g << 8) | (b << 16) | (a << 24);
        }

        inline uint32_t convertVecToPixelARGB(const vecRGBA& color) const {
            vecRGBA c = color.clamped();
            uint8_t r = static_cast<uint8_t>(c.r * 255.99);
            uint8_t g = static_cast<uint8_t>(c.g * 255.99);
            uint8_t b = static_cast<uint8_t>(c.b * 255.99);
            uint8_t a = static_cast<uint8_t>(c.a * 255.99);
            return (a << 24) | (r << 16) | (g << 8) | b;
        }

        void drawHorizontalLine(int y, int xStart, int xEnd, const vecRGBA& color) {
            for (int x = xStart; x <= xEnd; ++x) {
                setPixel(x, y, color);
            }
        }

        void drawHorizontalLine(float y, float xStart, float xEnd, const vecRGBA& color) {
            int xMin = static_cast<int>(std::floor(xStart));
            int xMax = static_cast<int>(std::ceil(xEnd));
        
            for (int x = xMin; x < xMax; ++x) {
                float covX = overlap(xStart, xEnd, x, x + 1);
                float covY = overlap(y, y + 1, static_cast<int>(std::floor(y)), static_cast<int>(std::ceil(y)));
                float coverage = covX * covY;
                if (coverage > 0.0) {
                    vecRGBA c = color * coverage;
                    setPixel(x, static_cast<int>(std::floor(y)), c);
                }
            }
        }

        void drawCircle(int cx, int cy, int radius, const vecRGBA& color) {
            int x = radius;
            int y = 0;
            int err = 0;
        
            while (x >= y) {
                setPixel(cx + x, cy + y, color);
                setPixel(cx + y, cy + x, color);
                setPixel(cx - y, cy + x, color);
                setPixel(cx - x, cy + y, color);
                setPixel(cx - x, cy - y, color);
                setPixel(cx - y, cy - x, color);
                setPixel(cx + y, cy - x, color);
                setPixel(cx + x, cy - y, color);
        
                y += 1;
                err += 1 + 2 * y;
                if (2 * (err - x) + 1 > 0) {
                    x -= 1;
                    err += 1 - 2 * x;
                }
            }
        }

        void drawCircle(float cx, float cy, float radius, const vecRGBA& color) {
            int xMin = static_cast<int>(std::floor(cx - radius));
            int xMax = static_cast<int>(std::ceil(cx + radius));
            int yMin = static_cast<int>(std::floor(cy - radius));
            int yMax = static_cast<int>(std::ceil(cy + radius));
        
            float r2 = radius * radius;
            float outer2 = (radius + 1.0) * (radius + 1.0);
            float inner2 = std::max(0.0, (radius - 1.0) * (radius - 1.0));
        
            for (int y = yMin; y <= yMax; ++y) {
                for (int x = xMin; x <= xMax; ++x) {
                    float dx = x + 0.5 - cx;
                    float dy = y + 0.5 - cy;
                    float dist2 = dx * dx + dy * dy;
        
                    if (dist2 <= outer2 && dist2 >= inner2) {
                        float coverage = 1.0 - std::abs(dist2 - r2) / radius; // soft edge fade
                        coverage = std::clamp(coverage, 0.0f, 1.0f);
                        setPixel(x, y, color * coverage);
                    }
                }
            }
        }

        void drawFilledCircle(float cx, float cy, float radius, const vecRGBA& color) {
            int xMin = static_cast<int>(std::floor(cx - radius));
            int xMax = static_cast<int>(std::ceil(cx + radius));
            int yMin = static_cast<int>(std::floor(cy - radius));
            int yMax = static_cast<int>(std::ceil(cy + radius));
        
            float r2 = radius * radius;
        
            for (int y = yMin; y <= yMax; ++y) {
                for (int x = xMin; x <= xMax; ++x) {
                    float dx = x + 0.5 - cx;
                    float dy = y + 0.5 - cy;
                    float dist2 = dx * dx + dy * dy;
                    if (dist2 <= r2) {
                        float coverage = 1.0 - (dist2 / r2);  // fade at edge
                        coverage = std::clamp(coverage, 0.0f, 1.0f);
                        setPixel(x, y, color * coverage);
                    }
                }
            }
        }    

        void drawFilledCircle(int cx, int cy, int radius, const vecRGBA& color) {
            for (int y = -radius; y <= radius; ++y) {
                int dx = static_cast<int>(safe_sqrt(radius * radius - y * y));
                for (int x = -dx; x <= dx; ++x) {
                    setPixel(cx + x, cy + y, color);
                }
            }
        }

        void drawLine(int x0, int y0, int x1, int y1, const vecRGBA& color) {
            int dx = std::abs(x1 - x0);
            int dy = std::abs(y1 - y0);
            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;
            int err = dx - dy;
        
            while (true) {
                setPixel(x0, y0, color);
                if (x0 == x1 && y0 == y1) break;
                int e2 = 2 * err;
                if (e2 > -dy) { err -= dy; x0 += sx; }
                if (e2 < dx)  { err += dx; y0 += sy; }
            }
        }

        void drawLine(float x0, float y0, float x1, float y1, const vecRGBA& color) {
            auto ipart = [](float x) { return static_cast<int>(std::floor(x)); };
            auto fpart = [](float x) { return x - std::floor(x); };
            auto rfpart = [=](float x) { return 1.0 - fpart(x); };
        
            bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
            if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
            if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
        
            float dx = x1 - x0;
            float dy = y1 - y0;
            float gradient = (dx == 0.0) ? 1.0 : dy / dx;
        
            // handle first endpoint
            int xend = static_cast<int>(std::round(x0));
            float yend = y0 + gradient * (xend - x0);
            float xgap = rfpart(x0 + 0.5);
            int xpxl1 = xend;
            int ypxl1 = ipart(yend);
            if (steep) {
                setPixel(ypxl1, xpxl1, color * rfpart(yend) * xgap);
                setPixel(ypxl1 + 1, xpxl1, color * fpart(yend) * xgap);
            } else {
                setPixel(xpxl1, ypxl1, color * rfpart(yend) * xgap);
                setPixel(xpxl1, ypxl1 + 1, color * fpart(yend) * xgap);
            }
        
            float intery = yend + gradient;
        
            // handle second endpoint
            xend = static_cast<int>(std::round(x1));
            yend = y1 + gradient * (xend - x1);
            xgap = fpart(x1 + 0.5);
            int xpxl2 = xend;
            int ypxl2 = ipart(yend);
            if (steep) {
                setPixel(ypxl2, xpxl2, color * rfpart(yend) * xgap);
                setPixel(ypxl2 + 1, xpxl2, color * fpart(yend) * xgap);
            } else {
                setPixel(xpxl2, ypxl2, color * rfpart(yend) * xgap);
                setPixel(xpxl2, ypxl2 + 1, color * fpart(yend) * xgap);
            }
        
            // main loop
            if (steep) {
                for (int x = xpxl1 + 1; x < xpxl2; ++x) {
                    setPixel(ipart(intery), x, color * rfpart(intery));
                    setPixel(ipart(intery) + 1, x, color * fpart(intery));
                    intery += gradient;
                }
            } else {
                for (int x = xpxl1 + 1; x < xpxl2; ++x) {
                    setPixel(x, ipart(intery), color * rfpart(intery));
                    setPixel(x, ipart(intery) + 1, color * fpart(intery));
                    intery += gradient;
                }
            }
        }

        void drawRectOutline(int x1, int y1, int x2, int y2, const vecRGBA& color) {
            drawHorizontalLine(y1, x1, x2, color);
            drawHorizontalLine(y2, x1, x2, color);
            drawVerticalLine(x1, y1, y2, color);
            drawVerticalLine(x2, y1, y2, color);
        }

        void drawRectOutline(float x1, float y1, float x2, float y2, const vecRGBA& color) {
            drawHorizontalLine(y1, x1, x2, color);
            drawHorizontalLine(y2, x1, x2, color);
            drawVerticalLine(x1, y1, y2, color);
            drawVerticalLine(x2, y1, y2, color);
        }
        
        void drawVerticalLine(int x, int yStart, int yEnd, const vecRGBA& color) {
            for (int y = yStart; y <= yEnd; ++y) {
                setPixel(x, y, color);
            }
        }

        void drawVerticalLine(float x, float yStart, float yEnd, const vecRGBA& color) {
            int yMin = static_cast<int>(std::floor(yStart));
            int yMax = static_cast<int>(std::ceil(yEnd));
        
            for (int y = yMin; y < yMax; ++y) {
                float covY = overlap(yStart, yEnd, y, y + 1);
                float covX = overlap(x, x + 1, static_cast<int>(std::floor(x)), static_cast<int>(std::ceil(x)));
                float coverage = covX * covY;
                if (coverage > 0.0) {
                    vecRGBA c = color * coverage;
                    setPixel(static_cast<int>(std::floor(x)), y, c);
                }
            }
        }

        void drawPolygon(const std::vector<vec2>& vertices, const vecRGBA& color) {
            if (vertices.size() < 3) return;
        
            // Find bounding box
            float minY = vertices[0].y, maxY = vertices[0].y;
            for (const auto& v : vertices) {
                minY = std::min(minY, (float) v.y);
                maxY = std::max(maxY, (float) v.y);
            }
        
            int yStart = static_cast<int>(std::floor(minY));
            int yEnd   = static_cast<int>(std::ceil(maxY));
        
            for (int y = yStart; y <= yEnd; ++y) {
                std::vector<float> nodeX;
        
                size_t j = vertices.size() - 1;
                for (size_t i = 0; i < vertices.size(); ++i) {
                    float yi = vertices[i].y;
                    float yj = vertices[j].y;
        
                    if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                        float xi = vertices[i].x;
                        float xj = vertices[j].x;
                        float x = xi + (y - yi) / (yj - yi) * (xj - xi);
                        nodeX.push_back(x);
                    }
                    j = i;
                }
        
                std::sort(nodeX.begin(), nodeX.end());
                for (size_t k = 0; k + 1 < nodeX.size(); k += 2) {
                    int xStart = static_cast<int>(std::floor(nodeX[k]));
                    int xEnd   = static_cast<int>(std::ceil(nodeX[k + 1]));
                    for (int x = xStart; x < xEnd; ++x) {
                        setPixel(x, y, color);
                    }
                }
            }
        }
        

        void fillRect(int x1, int y1, int x2, int y2, const vecRGBA& color) {
            int xMin = std::min(x1, x2);
            int xMax = std::max(x1, x2);
            int yMin = std::min(y1, y2);
            int yMax = std::max(y1, y2);
        
            for (int y = yMin; y <= yMax; ++y) {
                for (int x = xMin; x <= xMax; ++x) {
                    setPixel(x, y, color);
                }
            }
        }

        inline void fillRect(
            float x1, float y1,
            float x2, float y2,
            const vecRGBA& color
        ) {
            int xMin = static_cast<int>(std::floor(x1));
            int xMax = static_cast<int>(std::ceil(x2));
            int yMin = static_cast<int>(std::floor(y1));
            int yMax = static_cast<int>(std::ceil(y2));

            for (int y = yMin; y < yMax; ++y) {
                if (y < 0 || y >= height) continue;
                for (int x = xMin; x < xMax; ++x) {
                    if (x < 0 || x >= width) continue;

                    float covX = overlap(x1, x2, x, x + 1);
                    float covY = overlap(y1, y2, y, y + 1);
                    float coverage = covX * covY;

                    if (coverage <= 0.0) continue;

                    vecRGBA partialColor = color;
                    partialColor.r *= coverage;
                    partialColor.g *= coverage;
                    partialColor.b *= coverage;
                    partialColor.a *= coverage;

                    int idx = y * width + x;
                    accumulationBuffer[idx] = partialColor.compositeOver(accumulationBuffer[idx]);
                }
            }
        }

        void flushToDisplay(std::vector<uint32_t>& outPixels) const {
            outPixels.resize(accumulationBuffer.size());
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int srcIndex = (height - 1 - y) * width + x;
                    int dstIndex = y * width + x;
                    outPixels[dstIndex] = convertVecToPixel(accumulationBuffer[srcIndex]);
                }
            }
        }

        vecRGBA getPixel(int x, int y) const {
            if (x < 0 || x >= width || y < 0 || y >= height) return vecRGBA::TransparentBlack();
            return accumulationBuffer[y * width + x];
        }
        
        void loadFromBMP(const std::string& filePath) {
            std::ifstream file(filePath, std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("FrameBuffer::loadFromBMP - Failed to open file: " + filePath);
            }
        
            uint8_t header[54];
            file.read(reinterpret_cast<char*>(header), 54);
            if (header[0] != 'B' || header[1] != 'M') {
                throw std::runtime_error("FrameBuffer::loadFromBMP - Not a valid BMP file: " + filePath);
            }
        
            int fileWidth = *reinterpret_cast<int32_t*>(&header[18]);
            int fileHeight = *reinterpret_cast<int32_t*>(&header[22]);
            int bitDepth = *reinterpret_cast<uint16_t*>(&header[28]);
        
            if (bitDepth != 32) {
                throw std::runtime_error("FrameBuffer::loadFromBMP - Only 32-bit BMPs are supported.");
            }
        
            int rowSize = fileWidth * 4;
            int paddedRowSize = ((rowSize + 3) / 4) * 4;
            int dataOffset = *reinterpret_cast<int32_t*>(&header[10]);
        
            file.seekg(dataOffset, std::ios::beg);
        
            init(fileWidth, fileHeight); // Resize framebuffer to match BMP
        
            std::vector<uint8_t> rowData(paddedRowSize);
        
            for (int y = height - 1; y >= 0; --y) { // BMP is stored bottom-up
                file.read(reinterpret_cast<char*>(rowData.data()), paddedRowSize);
                for (int x = 0; x < width; ++x) {
                    int idx = x * 4;
                    float b = rowData[idx + 0] / 255.0;
                    float g = rowData[idx + 1] / 255.0;
                    float r = rowData[idx + 2] / 255.0;
                    float a = rowData[idx + 3] / 255.0;
        
                    accumulationBuffer[y * width + x] = vecRGBA{r, g, b, a};
                }
            }
        
            file.close();
        }    

        inline float overlap(float a1, float a2, float b1, float b2) const {
            return std::max(0.0f, std::min(a2, b2) - std::max(a1, b1));
        }

        void setPixel(int x, int y, const vecRGBA& color) {
            if (x < 0 || x >= width || y < 0 || y >= height) return;
            accumulationBuffer[y * width + x] = color;
        }

        void updatePixels() {
            flushToDisplay(displayPixels);
        }

        void writeToBMP(const std::string& filePath) const {
            std::ofstream file(filePath, std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error("FrameBuffer::writeToBMP - Failed to open file: " + filePath);
            }
        
            const int bytesPerPixel = 4;
            const int rowSize = width * bytesPerPixel;
            const int paddedRowSize = ((rowSize + 3) / 4) * 4; // BMP rows are aligned to 4 bytes
            const int dataSize = paddedRowSize * height;
            const int fileSize = 54 + dataSize;
        
            // --- BMP HEADER ---
            uint8_t header[54] = {0};
        
            // Bitmap signature
            header[0] = 'B';
            header[1] = 'M';
        
            // File size
            *reinterpret_cast<uint32_t*>(&header[2]) = fileSize;
        
            // Data offset
            *reinterpret_cast<uint32_t*>(&header[10]) = 54;
        
            // DIB Header size
            *reinterpret_cast<uint32_t*>(&header[14]) = 40;
        
            // Width and height
            *reinterpret_cast<int32_t*>(&header[18]) = width;
            *reinterpret_cast<int32_t*>(&header[22]) = height;
        
            // Planes and bit count
            *reinterpret_cast<uint16_t*>(&header[26]) = 1;
            *reinterpret_cast<uint16_t*>(&header[28]) = 32;
        
            file.write(reinterpret_cast<char*>(header), sizeof(header));
        
            // --- PIXEL DATA ---
            for (int y = height - 1; y >= 0; --y) { // BMP is bottom-up
                for (int x = 0; x < width; ++x) {
                    const vecRGBA& color = accumulationBuffer[y * width + x].clamped();
                    uint8_t r = static_cast<uint8_t>(color.r * 255.99);
                    uint8_t g = static_cast<uint8_t>(color.g * 255.99);
                    uint8_t b = static_cast<uint8_t>(color.b * 255.99);
                    uint8_t a = 255; // Could also use color.a * 255.99 if transparency mattered
        
                    file.put(b);
                    file.put(g);
                    file.put(r);
                    file.put(a);
                }
        
                // Padding if needed (rare for 32-bit aligned rows, but safe)
                int padding = paddedRowSize - rowSize;
                for (int i = 0; i < padding; ++i) file.put(0);
            }
        
            file.close();
        }
    };
};
