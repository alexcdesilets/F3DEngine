#pragma once

#include "globals.h"
#include "vecSys/vec2.h"
#include "vecSys/vec3.h"
#include "vecSys/vecRGBA.h"


struct Light {
    vecRGBA color = {0.5, 0.5, 0.5, 1.0};
    vec2 position = {0.0,0.0};
    double zElevation = 64.0;
    double radiance = 200.0;
    double pitch = 0.0, yaw = 0.0;
    double halfFOVH = pi, halfFOVV = pi;
    double cuttoffMinH = 0, cuttoffMaxH = 2*pi, cuttoffminV = 0, cuttoffMaxV = 2*pi;
    bool isON = true, flicker = false;

    bool illuminated(const vec2& obj, const double& objElevation) const {
        vec2 toSource = obj - position;
        double angleH = toSource.angle() + pi;
        if (cuttoffMinH <= angleH && cuttoffMaxH >= angleH) {
            double angleV = std::atan2(objElevation - zElevation, toSource.length()) + pi;
            if (cuttoffminV <=  angleV && cuttoffMaxV >= angleV) return true;
        }
        return false;
    }

    void move(const vec2& worldChange, const double& zChange) {
        position += worldChange;
        zElevation += zChange;
    }

    void lerpRadiance(const double& target, double rate = 1.0) {
        rate = std::clamp(rate, 0.0, 1.0);
        radiance += (target - radiance) * rate;
    }

    vecRGBA lightUp(const vec2& obj, const double& objElevation, float flickerVariance = 0.05f) const {
        vec2 toSource = obj - position;
        double zDiff = objElevation - zElevation;
        double distance = std::sqrt(toSource.x*toSource.x + toSource.y*toSource.y + zDiff*zDiff);
        if (flicker) {
            double flickerAmmount = (((double)rand() / RAND_MAX ) - 0.5) * flickerVariance;
            return color * flickerAmmount / std::clamp((distance*distance)/radiance, 0.0, 1.0);
        }
        return color / std::clamp((distance*distance)/radiance, 0.0, 1.0);
    }

    void complexOrbit(const vec3& orbitPoint, const vec3& axis, const double& rads) {
        vec3 complexPosition = {position.x, position.y, zElevation};
        complexPosition.orbit(orbitPoint, axis, rads);
        position = {complexPosition.x, complexPosition.y};
        zElevation = complexPosition.z;
    }

    void rotate(double horizontal, double vertical) {
        auto rotationWrap = [](double& val) -> void {
            if (val < - pi) val += 2*pi;
            else if (val > pi) val -= 2*pi;
        };
        pitch += vertical; rotationWrap(pitch);
        yaw += horizontal; rotationWrap(yaw);
        
        cuttoffMinH = yaw - halfFOVH + pi;
        cuttoffMaxH = yaw + halfFOVH + pi;
        cuttoffminV = pitch - halfFOVV + pi;
        cuttoffMaxV = pitch + halfFOVV + pi;
    }
};

