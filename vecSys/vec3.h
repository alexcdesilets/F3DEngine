#pragma once

#include "vec2.h"

namespace vecSys {
    struct vec3 {
        double x = 0.0, y = 0.0, z = 0.0;

        vec3() {}
        vec3(const double val): x(val), y(val), z(val) {} 
        vec3(const double valX, const double valY, const double valZ): x(valX), y(valY), z(valZ) {}
        vec3(const std::vector<double>& vals) {
            size_t size = vals.size();
            if (size == 1) { x = vals[0]; }
            else if (size == 2) { x = vals[0]; y = vals[1]; }
            else if (size >= 3) { x = vals[0]; y = vals[1]; z = vals[2]; }
        }
        vec3(const vec2& other): x(other.x), y(other.y) {}
        vec3(const vec2& other, const bool& swapYZ): x(other.x){ swapYZ ? z = other.y: y = other.y; }
        vec3(const vec3& other): x(other.x), y(other.y), z(other.z) {}
        vec3(const vec3& other, const bool& swapYZ): x(other.x) { if (swapYZ) {z = other.y; y = other.z;} else {y = other.y; z = other.z;} }

        // --- Static Factory Methods (from raylib-wrapper vec3) ---
        static constexpr vec3 zero() { return {0.0, 0.0, 0.0}; }
        static constexpr vec3 one() { return {1.0, 1.0, 1.0}; }
        static constexpr vec3 right() { return {1.0, 0.0, 0.0}; }
        static constexpr vec3 left() { return {-1.0, 0.0, 0.0}; }
        static constexpr vec3 up() { return {0.0, 1.0, 0.0}; }
        static constexpr vec3 down() { return {0.0, -1.0, 0.0}; }
        static constexpr vec3 forward() { return {0.0, 0.0, 1.0}; }
        static constexpr vec3 backward() { return {0.0, 0.0, -1.0};}
        static constexpr vec3 unitX() { return {1.0, 0.0, 0.0}; }
        static constexpr vec3 unitY() { return {0.0, 1.0, 0.0}; }
        static constexpr vec3 unitZ() { return {0.0, 0.0, 1.0}; }
        static constexpr vec3 nunitX() { return -unitX(); }
        static constexpr vec3 nunitY() { return -unitY(); }
        static constexpr vec3 nunitZ() { return -unitZ(); }
        static vec3 random(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) {return vec3().randomize(minVal, maxVal);}

        // --- Angle related methods (from raylib-wrapper vec3) ---
        double angleX() const { return std::atan2(y, z); }
        double angleY() const { return std::atan2(z, x); }
        double angleZ() const { return std::atan2(y, x); }
        double angleTo(const vec3& other) const { vec3 tN = normalized(), oN = other.normalized(); return std::atan2(tN.cross(oN).length(), tN.dot(oN)); }

        [[nodiscard]] vec3 getProjectedAngleDiffs(const vec3& other) const { return {other.angleX() - angleX(), other.angleY() - angleY(), other.angleZ() - angleZ()}; }

        // --- Rounding methods ---
        vec3& ceil() { x = std::ceil(x); y = std::ceil(y); z = std::ceil(z); return *this; }
        [[nodiscard]] vec3 ceilled() const { return {std::ceil(x), std::ceil(y), std::ceil(z)}; }
        vec3& floor() { x = std::floor(x); y = std::floor(y); z = std::floor(z); return *this; }
        [[nodiscard]] vec3 floored() const { return {std::floor(x), std::floor(y), std::floor(z)}; }
        vec3& round() { x = std::round(x); y = std::round(y); z = std::round(z); return *this; }
        [[nodiscard]] vec3 rounded() const { return {std::round(x), std::round(y), std::round(z)}; }

        // --- Clamping methods ---
        // This will be component-wise clamp for consistency, as Vector3ClampValue in raymath.c is component-wise.
        vec3& clamp(double minVal, double maxVal) {
            x = std::clamp(x, minVal, maxVal); // C++17
            y = std::clamp(y, minVal, maxVal);
            z = std::clamp(z, minVal, maxVal);
            return *this;
        }
        [[nodiscard]] vec3 clamped(double minVal, double maxVal) const {
            return {std::clamp(x, minVal, maxVal), std::clamp(y, minVal, maxVal), std::clamp(z, minVal, maxVal)};
        }

        vec3& clamp(const vec3& minVec, const vec3& maxVec) { // Component-wise vector clamp
            x = std::clamp(x, minVec.x, maxVec.x);
            y = std::clamp(y, minVec.y, maxVec.y);
            z = std::clamp(z, minVec.z, maxVec.z);
            return *this;
        }
        [[nodiscard]] vec3 clamped(const vec3& minVec, const vec3& maxVec) const {
            return {std::clamp(x, minVec.x, maxVec.x), std::clamp(y, minVec.y, maxVec.y), std::clamp(z, minVec.z, maxVec.z)};
        }

        vec3& clampMagnitude(double minLen, double maxLen) {
            double currentLenSq = lengthSquared();
            if (currentLenSq == 0.0) {
                // Handle zero vector: if minLen > 0, behavior is ambiguous.
                // For now, if it's zero and minLen is positive, it stays zero.
                return *this;
            }
            double currentLen = safe_sqrt(currentLenSq);
            double scaleFactor = 1.0;
            if (currentLen < minLen && minLen > 0.0) {
                scaleFactor = minLen / currentLen;
            } else if (currentLen > maxLen && maxLen >= 0.0) {
                 if (currentLen > 0.0) { // Avoid division by zero if currentLen is effectively zero
                    scaleFactor = maxLen / currentLen;
                } else {
                    scaleFactor = 0.0; // Cannot scale zero vector to maxLen unless maxLen is also 0
                }
            }
            
            if (std::abs(scaleFactor - 1.0) > 0.0 && scaleFactor != 0.0) {
                 x *= scaleFactor; y *= scaleFactor; z *= scaleFactor;
            } else if (scaleFactor == 0.0 && maxLen == 0.0) {
                 x = 0.0; y = 0.0; z = 0.0;
            }
            return *this;
        }
        [[nodiscard]] vec3 clampedMagnitude(double minLen, double maxLen) const {
            vec3 copy = *this;
            return copy.clampMagnitude(minLen, maxLen);
        }

        // --- Cross Product ---
        [[nodiscard]] vec3 cross(const vec3& other) const {
            return {y * other.z - z * other.y,
                    z * other.x - x * other.z,
                    x * other.y - y * other.x};
        }

        // --- Distance methods ---
        double distanceTo(const vec3& other) const { return (other-*this).length(); }
        double distanceToSquared(const vec3& other) const { return (other-*this).lengthSquared(); }

        // --- Dot Product ---
        double dot(const vec3& other) const { return x * other.x + y * other.y + z * other.z; }

        // --- Inversion ---
        vec3& invert() {
            x = safe_divide(1.0, x);
            y = safe_divide(1.0, y);
            z = safe_divide(1.0, z);
            return *this;
        }
        [[nodiscard]] vec3 inverted() const {
            return {safe_divide(1.0, x), safe_divide(1.0, y), safe_divide(1.0, z)};
        }

        // --- Finite/Infinite/NaN Checks ---
        bool isFinite() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z); }
        bool isInfinite() const { return std::isinf(x) || std::isinf(y) || std::isinf(z); }
        bool isNaN() const { return std::isnan(x) || std::isnan(y) || std::isnan(z); }
        bool isZero() const { return x == 0.0 && y == 0.0 && z == 0.0; }

        // --- Length methods ---
        double length() const {
            if (isInfinite()) return infinity;
            return safe_sqrt(x*x + y*y + z*z); // std::hypot(x,y,z) in C++17
        }
        double lengthSquared() const {
            if (isInfinite()) return infinity;
            return x*x + y*y + z*z;
        }

        // --- Linear Interpolation ---
        vec3& lerp(const vec3& target, const double amount) {
            x = std::lerp(x, target.x, amount); // C++20
            y = std::lerp(y, target.y, amount);
            z = std::lerp(z, target.z, amount);
            // Pre-C++20: x = x + amount * (target.x - x); ...
            return *this;
        }
        [[nodiscard]] vec3 lerped(const vec3& target, const double amount) const {
            return {std::lerp(x, target.x, amount), std::lerp(y, target.y, amount), std::lerp(z, target.z, amount)};
        }

        // --- Projection and Rejection ---
        vec3& projectOn(const vec3& other) {
            double otherLenSq = other.lengthSquared();
            if (otherLenSq == 0.0 || std::isinf(otherLenSq)) {
                x = y = z = 0.0; // Projecting onto zero or infinite vector copys in zero
                return *this;
            }
            double scale = dot(other) / otherLenSq; // Standard division
            *this = other * scale; // Use operator*(vec3, double)
            return *this;
        }
        [[nodiscard]] vec3 projectedOn(const vec3& other) const {
            vec3 copy = *this;
            return copy.projectOn(other);
        }

        vec3& rejectFrom(const vec3& other) {
            *this -= projectedOn(other); // Uses projectOn and operator-=
            return *this;
        }
        [[nodiscard]] vec3 rejectedFrom(const vec3& other) const {
            return *this - projectedOn(other); // Uses projectOn and operator-
        }

        // --- Component-wise Max/Min ---
        vec3& max(const vec3& other) { x = std::max(x, other.x); y = std::max(y, other.y); z = std::max(z, other.z); return *this; }
        [[nodiscard]] vec3 maxComp(const vec3& other) const { return {std::max(x, other.x), std::max(y, other.y), std::max(z, other.z)}; }
        vec3& min(const vec3& other) { x = std::min(x, other.x); y = std::min(y, other.y); z = std::min(z, other.z); return *this; }
        [[nodiscard]] vec3 minComp(const vec3& other) const { return {std::min(x, other.x), std::min(y, other.y), std::min(z, other.z)}; }

        // --- Move Towards ---
        vec3& moveTowards(const vec3& target, double maxDistance) {
            if (maxDistance <= 0.0) return *this;
            vec3 diff = target - *this;
            double distSq = diff.lengthSquared();
            if (distSq == 0.0 || distSq <= maxDistance * maxDistance) {
                *this = target;
            } else {
                double dist = safe_sqrt(distSq);
                *this += (diff / dist) * maxDistance;
            }
            return *this;
        }
        [[nodiscard]] vec3 movedTowards(const vec3& target, double maxDistance) const {
            vec3 copy = *this;
            return copy.moveTowards(target, maxDistance);
        }

        // --- Normalization ---
        vec3& normalize() {
            if (isZero()) return *this;
            else if (isInfinite()) {
                x = safe_divide(x, infinity);
                y = safe_divide(y, infinity);
                z = safe_divide(z, infinity);
            }
            double len = length();
            x /= len;
            y /= len;
            z /= len;
            return *this;
        }
        [[nodiscard]] vec3 normalized() const {
            vec3 copy = *this;
            return copy.normalize();
        }

        // --- Orbit and Rotation ---
        vec3& rotateByAxisAngle(const vec3& axis, double radians) {
            vec3 normAxis = axis.normalized();
            if (normAxis.lengthSquared() == 0.0) return *this;

            double s = std::sin(radians / 2.0);
            double c = std::cos(radians / 2.0);

            double qx = normAxis.x * s;
            double qy = normAxis.y * s;
            double qz = normAxis.z * s;
            double qw = c;

            vec3 q_vec = {qx, qy, qz};
            vec3 v = *this; // Current vector
            
            vec3 wv = q_vec.cross(v);
            vec3 wwv = q_vec.cross(wv);

            wv *= (2.0 * qw);
            wwv *= 2.0;

            *this = v + wv + wwv;
            return *this;
        }
        [[nodiscard]] vec3 rotatedByAxisAngle(const vec3& axis, double radians) const {
            vec3 copy = *this;
            return copy.rotateByAxisAngle(axis, radians);
        }

        // Axial rotations (from raylib-wrapper vec3)
        vec3& rotateX(const double radians) {
            double currentAngleInYZ = std::atan2(y, z);
            double magInYZ = std::hypot(y, z);
            double newAngleInYZ = currentAngleInYZ + radians;
            y = magInYZ * std::sin(newAngleInYZ);
            z = magInYZ * std::cos(newAngleInYZ);
            return *this;
        }
        [[nodiscard]] vec3 rotatedX(const double radians) const { vec3 copy = *this; return copy.rotateX(radians); }

        vec3& rotateLerpX(const double angleDelta, const double rate) {
            double currentAngleInYZ = std::atan2(y, z);
            double targetAngleInYZ = currentAngleInYZ + angleDelta;
            double diff = wrapAngle(targetAngleInYZ - currentAngleInYZ);
            double lerped_offset = std::lerp(0.0, diff, rate);
            double newAngleInYZ = currentAngleInYZ + lerped_offset;
            double magInYZ = std::hypot(y, z);
            if (magInYZ > 0.0) {
                y = magInYZ * std::sin(newAngleInYZ);
                z = magInYZ * std::cos(newAngleInYZ);
            } // else if mag is zero, stays zero
            return *this;
        }
        [[nodiscard]] vec3 rotatedLerpX(const double angleDelta, const double rate) const { vec3 copy = *this; return copy.rotateLerpX(angleDelta, rate); }

        vec3& rotateY(const double radians) {
            double currentAngleInXZ = std::atan2(z, x);
            double magInXZ = std::hypot(x, z);
            double newAngleInXZ = currentAngleInXZ + radians;
            x = magInXZ * std::cos(newAngleInXZ);
            z = magInXZ * std::sin(newAngleInXZ);
            return *this;
        }
        [[nodiscard]] vec3 rotatedY(const double radians) const { vec3 copy = *this; return copy.rotateY(radians); }

        vec3& rotateLerpY(const double angleDelta, const double rate) {
            double currentAngleInXZ = std::atan2(z, x);
            double targetAngleInXZ = currentAngleInXZ + angleDelta;
            double diff = wrapAngle(targetAngleInXZ - currentAngleInXZ);
            double lerped_offset = std::lerp(0.0, diff, rate);
            double newAngleInXZ = currentAngleInXZ + lerped_offset;
            double magInXZ = std::hypot(x, z);
             if (magInXZ > 0.0) {
                x = magInXZ * std::cos(newAngleInXZ);
                z = magInXZ * std::sin(newAngleInXZ);
            }
            return *this;
        }
        [[nodiscard]] vec3 rotatedLerpY(const double angleDelta, const double rate) const { vec3 copy = *this; return copy.rotateLerpY(angleDelta, rate); }

        vec3& rotateZ(const double radians) {
            double currentAngleInXY = std::atan2(y, x);
            double magInXY = std::hypot(x, y);
            double newAngleInXY = currentAngleInXY + radians;
            x = magInXY * std::cos(newAngleInXY);
            y = magInXY * std::sin(newAngleInXY);
            return *this;
        }
        [[nodiscard]] vec3 rotatedZ(const double radians) const { vec3 copy = *this; return copy.rotateZ(radians); }

        vec3& rotateLerpZ(const double angleDelta, const double rate) {
            double currentAngleInXY = std::atan2(y, x);
            double targetAngleInXY = currentAngleInXY + angleDelta;
            double diff = wrapAngle(targetAngleInXY - currentAngleInXY);
            double lerped_offset = std::lerp(0.0, diff, rate);
            double newAngleInXY = currentAngleInXY + lerped_offset;
            double magInXY = std::hypot(x, y);
            if (magInXY > 0.0) {
                x = magInXY * std::cos(newAngleInXY);
                y = magInXY * std::sin(newAngleInXY);
            }
            return *this;
        }
        [[nodiscard]] vec3 rotatedLerpZ(const double angleDelta, const double rate) const { vec3 copy = *this; return copy.rotateLerpZ(angleDelta, rate); }

        [[nodiscard]] vec3 opposite() const { return {-x, -y, -z}; }

        vec3& orbit(const vec3& point, const vec3& axis, double radians) {
            *this -= point; rotateByAxisAngle(axis, radians); return *this += point;
        }
        [[nodiscard]] vec3 orbited(const vec3& point, const vec3& axis, double radians) const {
            vec3 copy = *this - point; copy.rotateByAxisAngle(axis, radians); return copy + point;
        }

        // --- Perpendicular ---
        [[nodiscard]] vec3 perpendicular() const {
            double min_abs_comp = std::abs(x);
            vec3 cardinal_axis = {1.0, 0.0, 0.0};

            if (std::abs(y) < min_abs_comp) {
                min_abs_comp = std::abs(y);
                cardinal_axis = {0.0, 1.0, 0.0};
            }
            if (std::abs(z) < min_abs_comp) {
                cardinal_axis = {0.0, 0.0, 1.0};
            }
            return cross(cardinal_axis);
        }

        [[nodiscard]] vec3 perpendicular(const vec3& other) const {
            vec3 perp = this->cross(other);
            if (perp.lengthSquared() == 0.0) {
                // Fall back to arbitrary perpendicular (same as your existing method)
                return this->perpendicular();
            }
            return perp.normalized();
        }

        // --- Randomization ---
        vec3& randomize(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) {
            static std::mt19937_64 rng{std::random_device{}()};
            std::uniform_real_distribution<double> distribution(minVal, maxVal);
            x = distribution(rng); y = distribution(rng); z = distribution(rng);
            return *this;
        }
        [[nodiscard]] vec3 randomized(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) const {
            vec3 copy; return copy.randomize(minVal, maxVal);
        }

        // --- Reflection/Refraction ---
        vec3& reflect(vec3 normal) {
            normal.normalize();
            if (normal.lengthSquared() == 0.0) return *this;
            *this -= normal * (2.0 * dot(normal));
            return *this;
        }
        [[nodiscard]] vec3 reflected(vec3 normal) const {
            vec3 copy = *this; return copy.reflect(normal);
        }
        vec3& refract(vec3 normal, double indexOfRefractionRatio_n1_over_n2) {
            normal.normalize();
            if (normal.lengthSquared() == 0.0) return *this;
            double ni = dot(normal);
            double eta = indexOfRefractionRatio_n1_over_n2;
            double k = 1.0 - eta * eta * (1.0 - ni * ni);
            if (k < 0.0) { x = 0.0; y = 0.0; z = 0.0; } // Total internal reflection
            else { *this = (*this * eta) - normal * (eta * ni + safe_sqrt(k)); }
            return *this;
        }
        vec3 refracted(vec3 normal, double indexOfRefractionRatio_n1_over_n2) const {
            vec3 copy = *this; return copy.refract(normal, indexOfRefractionRatio_n1_over_n2);
        }

        // --- Scaling ---
        vec3& scale(const double scalar) { return *this *= scalar; }
        [[nodiscard]] vec3 scaled(const double scalar) const { return *this * scalar; }

        vec3& swapYZ() { *this = {x, z, y}; return *this; }
        [[nodiscard]] vec3 swappedYZ() {return {x, z, y};}

        // --- Conversions ---
        std::array<double, 3> toArray() const { return {x, y, z}; }
        const double* data() const { return &x; }
        double* data() { return &x; }

        // --- Static Utility Methods (from raylib-wrapper vec3 and raymath.h) ---
        static void orthoNormalize(vec3& v1, vec3& v2) {
            // Manual implementation of Vector3OrthoNormalize (Gram-Schmidt)
            v1.normalize();
            vec3 cross_v1_v2 = v1.cross(v2).normalize();
            v2 = cross_v1_v2.cross(v1); // v2 is now orthogonal to v1 and cross_v1_v2
            v2.normalize();
        }

        static vec3 barycenter(const vec3& p, const vec3& a, const vec3& b, const vec3& c) {
            // Manual implementation of Vector3Barycenter
            vec3 v0 = b - a;
            vec3 v1 = c - a;
            vec3 v2 = p - a;
            double d00 = v0.dot(v0);
            double d01 = v0.dot(v1);
            double d11 = v1.dot(v1);
            double d20 = v2.dot(v0);
            double d21 = v2.dot(v1);
            double denom = d00 * d11 - d01 * d01;
            if (std::abs(denom) == 0.0) return vec3::zero(); // Degenerate triangle

            double v_coord = (d11 * d20 - d01 * d21) / denom;
            double w_coord = (d00 * d21 - d01 * d20) / denom;
            double u_coord = 1.0 - v_coord - w_coord;
            return {u_coord, v_coord, w_coord}; // (u,v,w) stored in a vec3
        }

        static vec3 cubicHermiteSpline(const vec3& p0, const vec3& t0, const vec3& p1, const vec3& t1, double amount) {
            double t = amount;
            double t2 = t * t;
            double t3 = t2 * t;

            double h00 = 2*t3 - 3*t2 + 1;
            double h10 = t3 - 2*t2 + t;
            double h01 = -2*t3 + 3*t2;
            double h11 = t3 - t2;

            return (p0 * h00) + (t0 * h10) + (p1 * h01) + (t1 * h11);
        }

        // --- Operators ---
        [[nodiscard]] vec3 operator+(const vec3& other) const { return {x + other.x, y + other.y, z + other.z}; }
        [[nodiscard]] vec3 operator-(const vec3& other) const { return {x - other.x, y - other.y, z - other.z}; }
        [[nodiscard]] vec3 operator*(const vec3& other) const { return {x * other.x, y * other.y, z * other.z}; }
        [[nodiscard]] vec3 operator/(const vec3& other) const { return {safe_divide(x, other.x), safe_divide(y, other.y), safe_divide(z, other.z)}; }

        [[nodiscard]] vec3 operator+(const vec2& other) const { return {x + other.x, y + other.y, z}; }
        [[nodiscard]] vec3 operator-(const vec2& other) const { return {x - other.x, y - other.y, z}; }
        [[nodiscard]] vec3 operator*(const vec2& other) const { return {x * other.x, y * other.y, z}; }
        [[nodiscard]] vec3 operator/(const vec2& other) const { return {safe_divide(x, other.x), safe_divide(y, other.y), z}; }

        [[nodiscard]] vec3 operator-() const { return {-x, -y, -z}; }
        [[nodiscard]] vec3 operator+(const double scalar) const { return {x + scalar, y + scalar, z + scalar}; }
        [[nodiscard]] vec3 operator-(const double scalar) const { return {x - scalar, y - scalar, z - scalar}; }
        [[nodiscard]] vec3 operator*(const double scalar) const { return {x * scalar, y * scalar, z * scalar}; }
        [[nodiscard]] vec3 operator/(const double scalar) const { return {safe_divide(x, scalar), safe_divide(y, scalar), safe_divide(z, scalar)}; }

        vec3& operator+=(const vec3& other) { x+=other.x; y+=other.y; z+=other.z; return *this; }
        vec3& operator-=(const vec3& other) { x-=other.x; y-=other.y; z-=other.z; return *this; }
        vec3& operator*=(const vec3& other) { x*=other.x; y*=other.y; z*=other.z; return *this; }
        vec3& operator/=(const vec3& other) { x=safe_divide(x,other.x); y=safe_divide(y,other.y); z=safe_divide(z,other.z); return *this; }

        vec3& operator+=(const vec2& other) { x+=other.x; y+=other.y; return *this; }
        vec3& operator-=(const vec2& other) { x-=other.x; y-=other.y; return *this; }
        vec3& operator*=(const vec2& other) { x*=other.x; y*=other.y; return *this; }
        vec3& operator/=(const vec2& other) { x=safe_divide(x,other.x); y=safe_divide(y,other.y); return *this; }

        vec3& operator+=(const double scalar) { x+=scalar; y+=scalar; z+=scalar; return *this; }
        vec3& operator-=(const double scalar) { x-=scalar; y-=scalar; z-=scalar; return *this; }
        vec3& operator*=(const double scalar) { x*=scalar; y*=scalar; z*=scalar; return *this; }
        vec3& operator/=(const double scalar) { x=safe_divide(x,scalar); y=safe_divide(y,scalar); z=safe_divide(z,scalar); return *this; }

        bool operator==(const vec3& other) const { return x == other.x && y == other.y && z == other.z; }
        bool operator!=(const vec3& other) const { return !(*this == other); }
        bool operator<(const vec3& other) const { return lengthSquared() < other.lengthSquared(); }
        bool operator<=(const vec3& other) const { return lengthSquared() <= other.lengthSquared(); }
        bool operator>(const vec3& other) const { return lengthSquared() > other.lengthSquared(); }
        bool operator>=(const vec3& other) const { return lengthSquared() >= other.lengthSquared(); }

        // --- String Conversion ---
        std::string toStr() const { return std::format("vec3[x:{},y:{},z:{}]", x, y, z); }
    };

    // --- Non-member operators for symmetry (scalar on left) ---
    [[nodiscard]] inline vec3 operator+(const double scalar, const vec3& v) { return v + scalar; }
    [[nodiscard]] inline vec3 operator-(const double scalar, const vec3& v) { return {scalar - v.x, scalar - v.y, scalar - v.z}; }
    [[nodiscard]] inline vec3 operator*(const double scalar, const vec3& v) { return v * scalar; }
    [[nodiscard]] inline vec3 operator/(const double scalar, const vec3& v) { return {safe_divide(scalar, v.x), safe_divide(scalar, v.y), safe_divide(scalar, v.z)}; }

    [[nodiscard]] inline vec3 operator+(const vec2& v2, const vec3& v3) { return {v2.x + v3.x, v2.y + v3.y, v3.z}; }
    [[nodiscard]] inline vec3 operator-(const vec2& v2, const vec3& v3) { return {v2.x - v3.x, v2.y - v3.y, v3.z}; }
    [[nodiscard]] inline vec3 operator*(const vec2& v2, const vec3& v3) { return {v2.x * v3.x, v2.y * v3.y, v3.z}; }
    [[nodiscard]] inline vec3 operator/(const vec2& v2, const vec3& v3) { return {safe_divide(v2.x, v3.x), safe_divide(v2.y, v3.y), v3.z}; }

    inline std::ostream& operator<<(std::ostream& os, const vec3& v) {
        os << v.toStr();
        return os;
    }

} // End of namespace vecSys