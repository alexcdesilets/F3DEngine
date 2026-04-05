#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <format>
#include <sstream>
#include <iostream>
#include <fstream>
#include "vecSys/vecRGBA.h"

using namespace vecSys;

struct Texture {
    std::vector<vecRGBA> tx;
    unsigned int height, width;
    int colors = 3;

    static Texture loadFromGIMPppmFile(std::string fileName_ppm255, int COLORS = 3, int ignoreLines = 2) {
        auto panic = [](const std::string& message) {
            std::cerr << "Fatal Error: " << message << std::endl;
            std::exit(EXIT_FAILURE);
        };
    
        auto parseColorValue = [&](const std::string& buffer, const std::string& channelName, int pixelIndex) -> float {
            try {
                return std::stof(buffer) / 255.0;
            } catch (const std::invalid_argument&) {
                panic(std::format("Invalid value for {} channel (pixel {}): {}", channelName, pixelIndex, buffer));
                return 0.0; // unreachable, but silences compiler warning
            }
        };
    
        Texture loadedTexture;
        COLORS = std::clamp(COLORS, 3, 4);
        loadedTexture.colors = COLORS;
    
        std::ifstream file(fileName_ppm255);
        if (!file.is_open()) {
            panic("Could not open file: " + fileName_ppm255);
        }
    
        std::string buffer;
        for (unsigned int i = 0; i < ignoreLines; ++i) {
            if (!std::getline(file, buffer)) {
                panic("Unexpected end of file while skipping header lines: " + fileName_ppm255);
            }
        }
    
        if (!std::getline(file, buffer)) {
            panic("Unexpected end of file while reading resolution line: " + fileName_ppm255);
        }
    
        std::stringstream ss(buffer);
        ss >> loadedTexture.width >> loadedTexture.height;
    
        if (!loadedTexture.width || !loadedTexture.height) {
            panic("Invalid texture resolution in file: " + fileName_ppm255);
        }

        if (!std::getline(file, buffer)) {
            panic("Unexpected end of file while reading post-resolution line: " + fileName_ppm255);
        }
    
        loadedTexture.tx.reserve(loadedTexture.width * loadedTexture.height);
    
        for (unsigned int i = 0; i < loadedTexture.height * loadedTexture.width; ++i) {
            float r, g, b, a = 1.0f;
    
            if (!std::getline(file, buffer)) panic(std::format("Unexpected end of file at pixel {} (red): {}", i, fileName_ppm255));
            r = parseColorValue(buffer, "red", i);
    
            if (!std::getline(file, buffer)) panic(std::format("Unexpected end of file at pixel {} (green): {}", i, fileName_ppm255));
            g = parseColorValue(buffer, "green", i);
    
            if (!std::getline(file, buffer)) panic(std::format("Unexpected end of file at pixel {} (blue): {}", i, fileName_ppm255));
            b = parseColorValue(buffer, "blue", i);
    
            if (COLORS == 4) {
                if (!std::getline(file, buffer)) panic(std::format("Unexpected end of file at pixel {} (alpha): {}", i, fileName_ppm255));
                a = parseColorValue(buffer, "alpha", i);
            }
    
            loadedTexture.tx.push_back({r, g, b, a});
        }
    
        file.close();
        return loadedTexture;
    }

    vecRGBA get(int x, int y) const {
        x = (x % width + width) % width;  // wrap x
        y = (y % height + height) % height;  // wrap y
        int index = (y * width) + x;
        return tx[index];
    }

    vecRGBA getUV(unsigned int u, unsigned int v) const {
        v %= height - 1;
        return get(u,v);
    }

    std::vector<unsigned int> toRGBAUint8() const {
        std::vector<unsigned int> uvTex;
        for (unsigned int i = 0; i < tx.size(); ++i) {
            unsigned int index = i*4;
            uvTex.push_back(static_cast<unsigned int>(tx[i].r * 255.99));
            uvTex.push_back(static_cast<unsigned int>(tx[i].g * 255.99));
            uvTex.push_back(static_cast<unsigned int>(tx[i].b * 255.99));
            uvTex.push_back(static_cast<unsigned int>(tx[i].a * 255.99));
        }
        return uvTex;
    }
};

