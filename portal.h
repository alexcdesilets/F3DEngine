#pragma once

#include <vector>

#include "vecSys/vec2.h"
#include "vecSys/line2.h"
#include "vecSys/shape2.h"
#include "texture.h"


struct Portal {
    const vec2* entrancePoint1;
    const vec2* entrancePoint2;
    const vec2* exitPoint1; 
    const vec2* exitPoint2;
    double portalBase, portalHeight;

    double teleportationAngle() const {return line2{*entrancePoint1, *entrancePoint2}.angleBetween(line2{*exitPoint1, *exitPoint2}); }

    struct teleportationData {
        vec2 location;
        double adjusmentAngle;
        double visualStrectch;
    };
    
    teleportationData teleportationLocation(const vec2& entranceLocation) const {
        line2 entrance = { *entrancePoint1, *entrancePoint2 };
        line2 exit = {*exitPoint1, *exitPoint2};
        double portalEntranceWitdth = entrance.length();
        double portalExitWitdth = exit.length();
        double proportionalAdjustment = (entranceLocation - entrance.pnt1).length();
        double adjustmentRatio = portalEntranceWitdth / portalExitWitdth;
        proportionalAdjustment *= adjustmentRatio;
        double tAng = teleportationAngle();
        vec2 exitLocation = vec2::fromAngle(tAng) * proportionalAdjustment;
        return {exitLocation, tAng, adjustmentRatio};
    }
};

