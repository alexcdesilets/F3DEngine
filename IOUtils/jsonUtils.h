#pragma once

#include <nlohmann/json.hpp>
#include "../globals.h"

using json = nlohmann::json;

namespace vecSys {
    inline void to_json(json& j, const vec2& val) {
        j = json{{"x", val.x}, {"y", val.y}};
    }

    inline void from_json(const json& j, vec2& val) {
        j.at("x").get_to(val.x);
        j.at("y").get_to(val.y);
        // using namespace std;
        // cout << format("vec2: x {} y {}", val.x, val.y) << endl;
    }

    inline void to_json(json& j, const vec3& val) {
        j = json{{"x", val.x}, {"y", val.y}, {"z", val.z}};
    }

    inline void from_json(const json& j, vec3& val) {
        j.at("x").get_to(val.x);
        j.at("y").get_to(val.y);
        j.at("z").get_to(val.z);
        // using namespace std;
        // cout << format("vec3: x {} y {} z {}", val.x, val.y, val.z) << endl;
    }

    inline void to_json(json& j, const line2& val) {
        j = json{{"pnt1", val.pnt1}, {"pnt2", val.pnt2}};
    }

    inline void from_json(const json& j, line2& val) {
        j.at("pnt1").get_to(val.pnt1);
        j.at("pnt2").get_to(val.pnt2);
        // using namespace std;
        // cout << format("Line2: pnt1 {},{} pnt2 {},{}", val.pnt1.x, val.pnt1.y , val.pnt2.x, val.pnt1.y) << endl;
    }

    inline void to_json(json& j, const shape2& val) {
        j = json{{"points", val.points}};
    }

    inline void from_json(const json& j, shape2& val) {
        j.at("points").get_to(val.points);
        // using namespace std;
        // cout << "Shape2: " << endl;
        // for (const vec2& v: val.points){
        //     cout << format("vec2: pnt1 {} pnt2 {}", v.x, v.y) << endl;
        // }
    }
};

// --- Sector ---
inline void to_json(json& j, const Sector::Wall& val) {
    j = {
        {"isBarrier", val.isBarrier},
        {"isVisibleWall", val.isVisibleWall},
        {"wallHeight", val.wallHeight},
        {"textureFile", val.textureFile}
    };
}

inline void from_json(const json& j, Sector::Wall& val){
    j.at("isBarrier").get_to(val.isBarrier);
    j.at("isVisibleWall").get_to(val.isVisibleWall);
    j.at("wallHeight").get_to(val.wallHeight);
    j.at("textureFile").get_to(val.textureFile);
    // std::cout << std::format("sector::wall texture: {}", val.textureFile) << std::endl;
}

inline void to_json(json& j, const Sector& val) {
    j = {
        {"outline", val.outline},
        {"rotation", val.rotation},
        {"baseHeight", val.baseHeight},
        {"floatingHeight", val.floatingHeight},
        {"walls", val.walls},
        {"floorTextureFile", val.floorTextureFile},
        {"bottomTextureFile", val.bottomTextureFile},
        {"baseWallTextureFile", val.baseWallTextureFile}
    };
}

inline void from_json(const json& j, Sector& val) {
    j.at("outline").get_to(val.outline);
    j.at("rotation").get_to(val.rotation);
    j.at("baseHeight").get_to(val.baseHeight);
    j.at("floatingHeight").get_to(val.floatingHeight);
    j.at("walls").get_to(val.walls);
    j.at("floorTextureFile").get_to(val.floorTextureFile);
    j.at("bottomTextureFile").get_to(val.bottomTextureFile);
    j.at("baseWallTextureFile").get_to(val.baseWallTextureFile);
    // std::cout << std::format("sector floorTextureFile: {}", val.floorTextureFile) << std::endl;
    // std::cout << std::format("sector bottomTextureFile: {}", val.bottomTextureFile) << std::endl;
    // std::cout << std::format("sector baseWallTextureFile: {}", val.baseWallTextureFile) << std::endl;
}

// --- Billboard ---

inline void to_json(json& j, const Billboard& val) {
    j = {
        {"baseLine", val.base},
        {"verticalOffset", val.verticalOffset},
        {"Height", val.height},
        {"isVisible", val.visible},
        {"TextureFile", val.textureFile}
    };
}

inline void from_json(const json& j, Billboard& val) {
    j.at("baseLine").get_to(val.base);
    j.at("verticalOffset").get_to(val.verticalOffset);
    j.at("Height").get_to(val.height);
    j.at("isVisible").get_to(val.visible);
    j.at("TextureFile").get_to(val.textureFile);
    // std::cout << std::format("billboard texture: {}", val.textureFile) << std::endl;
}

// --- Level ---

inline void to_json(json& j, const Level& val) {
    j = {
        {"sectorList", val.sectorList},
        {"billboardList", val.billboardList}
    };
}

inline void from_json(const json& j, Level& val) {
    j.at("sectorList").get_to(val.sectorList);
    j.at("billboardList").get_to(val.billboardList);
}