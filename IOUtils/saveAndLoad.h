#pragma once

#include "jsonUtils.h"

void saveLevelToFile(const Level& level, const string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    json j = level;
    outFile << j.dump(4);  // Pretty-printed with 4-space indentation
    outFile.close();
}

Level loadLevelFromFile(const string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

    json j;
    inFile >> j;
    inFile.close();

    return j.get<Level>();  // uses from_json(Level&, const json&)
}

inline void saveTextures(const string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    json j = textureFiles;
    outFile << j.dump(4);
    outFile.close();
}

inline void loadTextures(const string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

    json j;
    inFile >> j;
    inFile.close();

    textureFiles = j.get<vector<string>>();
    
    for (const string& fname: textureFiles) {
        textures[fname] = Texture::loadFromGIMPppmFile(fname);
    }
}