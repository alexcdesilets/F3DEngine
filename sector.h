#pragma once

#include <vector>

#include "vecSys/base.h"
#include "vecSys/vec2.h"
#include "vecSys/line2.h"
#include "vecSys/shape2.h"
#include "texture.h"

using namespace vecSys;
using std::vector;

struct Sector {
    static constexpr double defaultWallHeight = 64.0;

    shape2 outline;
    double rotation = 0.0;
    double baseHeight = 16.0;
    double floatingHeight = 0;
    struct Wall {
        bool isBarrier = true, isVisibleWall = true;
        double wallHeight = defaultWallHeight * 2;
        string textureFile = "";
    };
    vector<Wall> walls;
    string floorTextureFile = "";
    string bottomTextureFile = "";
    string baseWallTextureFile = "";
    struct previousFrameAdjustments {
        vec2 motion = {0.0,0.0}, orbitalPoint = {0.0,0.0};
        double baseHeightChange = 0.0;
        double rotationRads = 0.0, orbitRads = 0.0;
        bool moved = false, rotated = false, orbited = false;

        void reset() {
            motion = {0.0, 0.0}; orbitalPoint = {0.0,0.0};
            baseHeightChange = 0.0; rotationRads = 0.0; orbitRads = 0.0;
            moved = false; rotated = false; orbited = false;
        }
    };
    previousFrameAdjustments lastFrame = {{0.0,0.0}, {0.0,0.0}, 0.0, 0.0 , 0.0, false, false, false};

    void setShape(const shape2& definition) {
        if (outline.numSegments() != 0) {
            outline.points.clear();
            walls.clear();
        }
        outline = definition;
        walls.resize(outline.numSegments());
    }

    int nextIndex(int i) const { return (i + 1) % outline.points.size(); }

    void rotate(const double& rads) {
        vec2 cen = outline.centerPoint();
        for (vec2& p: outline.points) {
            p.orbit(cen, rads);
        }
        lastFrame.rotated = true;
        lastFrame.rotationRads = rads;
        rotation = wrapAngle(rotation + rads);
    }

    void move(const vec2& ammount, double heightChange = 0.0) {
        outline.translate(ammount);
        if (heightChange != 0) {
            floatingHeight += heightChange;
        }
        lastFrame.moved = true;
        lastFrame.motion = ammount;
        lastFrame.baseHeightChange = heightChange;
    }

    void orbit(const vec2& orbtiPoint, const double& rads) {
        outline.orbit(orbtiPoint, rads);
        lastFrame.orbited = true;
        lastFrame.orbitalPoint = orbtiPoint;
        lastFrame.orbitRads = rads;
    }

    bool isInside(const vec2& checkPoint) {
        return outline.isInside(checkPoint);
    }

    int wallCount() const {
        return outline.numSegments();
    }
};