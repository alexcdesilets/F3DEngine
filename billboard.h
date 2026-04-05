#pragma once

#include "vecSys/vec2.h"
#include "vecSys/vec3.h"
#include "vecSys/line2.h"
#include "texture.h"

struct Billboard {
    line2 base = {{0,0}, {1,0}};
    double verticalOffset = 0.0, height = 1.0;
    bool visible = false;
    string textureFile = "";

    void init(vec2 pos = {0,0}, double width = 64.0, double vOffset = 0.0, double hght = 64.0) {
        base = {{-width/2.0,0.0},{width/2.0,0.0}};
        base.translate(pos);
        verticalOffset = vOffset;
        height = hght;
    }

    void move(vec2 amnt, double verticalMovement = 0.0) {
        base.translate(amnt);
        verticalOffset += verticalMovement;
    }

    void complexOrbit(const vec3& orbitPoint, const vec3& axis, const double& rads) {
        vec2 positionXY = base.centerPoint();
        vec3 complexPosition = {positionXY.x, positionXY.y, verticalOffset};
        complexPosition.orbit(orbitPoint, axis, rads);
        vec2 newXY = {complexPosition.x, complexPosition.y};
        verticalOffset = complexPosition.z;
    }

    void pointAt(const vec2& position) {
        line2 centerLine = {base.centerPoint(), position};
        base.rotate(base.angleBetween(centerLine));
    }
};

