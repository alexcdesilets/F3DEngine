#pragma once

#include "base.h"

namespace vecSys {

    struct vec2 {
        double x = 0.0, y = 0.0;

        vec2() {}
        vec2(const double val): x(val), y(val) {}
        vec2(const double valX, const double valY): x(valX), y(valY) {}
        vec2(const std::vector<double>& vals) {
            size_t size = vals.size();
            if (size == 1) {x = vals[0]; y = 0.0;}
            else if (size >= 2) {x = vals[0]; y = vals[1];}
        }
        vec2(const vec2& other): x(other.x), y(other.y) {}

        // --- Static Factory Methods (from raylib-wrapper vec2) ---
        static constexpr vec2 zero() { return {0.0, 0.0}; }
        static constexpr vec2 one() { return {1.0, 1.0}; }
        static constexpr vec2 right() { return {1.0, 0.0}; }
        static constexpr vec2 left() { return {-1.0, 0.0}; }
        static constexpr vec2 up() { return {0.0, 1.0}; }
        static constexpr vec2 down() { return {0.0, -1.0}; }
        static constexpr vec2 unitX() { return {1.0, 0.0}; }
        static constexpr vec2 unitY() { return {0.0, 1.0}; }
        static constexpr vec2 nUnitX() { return -unitX(); }
        static constexpr vec2 nUnitY() { return -unitY(); }
        static vec2 random(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) {return vec2().randomize(minVal, maxVal);}

        static constexpr vec2 fromAngle(const double radians) { return {std::cos(radians), std::sin(radians)}; }

        // --- Angle methods (from raylib-wrapper vec2) ---
        double angle() const {  return std::atan2(y, x); }
        double angleTo(const vec2& other) const { vec2 tN = normalized(), oN = other.normalized(); return std::atan2(tN.cross(oN), tN.dot(oN)); }
        double angleDiff(const vec2& other) const { return wrapAngle(other.angle() - angle()); }
        
        vec2& setAngleLerped(const double targetAbsAngle, const double rate) {
            double current_angle = angle();
            double diff = targetAbsAngle - current_angle;
            while (diff > pi) diff -= tau;
            while (diff <= -pi) diff += tau;
            double lerped_relative_angle = std::lerp(0.0, diff, rate);
            double new_absolute_angle = current_angle + lerped_relative_angle;
            
            double len = length();
            if (len > 0.0) {
                x = len * std::cos(new_absolute_angle);
                y = len * std::sin(new_absolute_angle);
            } else {
                x = 0.0; y = 0.0;
            }
            return *this;
        }
        [[nodiscard]] vec2 angleSetLerped(const double targetAbsAngle, const double rate) const {
            vec2 copy = *this;
            return copy.setAngleLerped(targetAbsAngle, rate);
        }

        vec2& angleSnap(const double targetAngleRad) {
            double len = length();
            if (len > 0.0) {
                x = len * std::cos(targetAngleRad);
                y = len * std::sin(targetAngleRad);
            } else {
                x = 0.0; y = 0.0;
            }
            return *this;
        }
        [[nodiscard]] vec2 angleSnapped(const double targetAngleRad) const {
            vec2 copy = *this;
            return copy.angleSnap(targetAngleRad);
        }

        // --- Rounding methods ---
        vec2& ceil() { x = std::ceil(x); y = std::ceil(y); return *this; }
        [[nodiscard]] vec2 ceiled() const { return {std::ceil(x), std::ceil(y)}; }

        // --- Clamping methods ---
        vec2& clamp(double minValue, double maxValue) {
            x = std::clamp(x, minValue, maxValue);
            y = std::clamp(y, minValue, maxValue);
            return *this;
        }
        [[nodiscard]] vec2 clamped(double minValue, double maxValue) const {
            return {std::clamp(x, minValue, maxValue), std::clamp(y, minValue, maxValue)};
        }

        // Component-wise clamp to a vector range
        vec2& clamp(const vec2& minVec, const vec2& maxVec) { // This matches Vector2Clamp
            x = std::clamp(x, minVec.x, maxVec.x);
            y = std::clamp(y, minVec.y, maxVec.y);
            return *this;
        }
        [[nodiscard]] vec2 clamped(const vec2& minVec, const vec2& maxVec) const {
            return {std::clamp(x, minVec.x, maxVec.x), std::clamp(y, minVec.y, maxVec.y)};
        }

        // Clamp vector's magnitude (length)
        vec2& clampMagnitude(double minLen, double maxLen) {
            double currentLenSq = lengthSquared();
            if (currentLenSq == 0.0) {
                 // If minLen > 0 and vector is zero, behavior is ambiguous.
                 // For now, if it's zero and minLen is positive, it stays zero.
                return *this;
            }
            double currentLen = safe_sqrt(currentLenSq);
            double scaleFactor = 1.0;
            if (currentLen < minLen && minLen > 0) { // Ensure minLen is positive to scale up
                scaleFactor = minLen / currentLen;
            } else if (currentLen > maxLen && maxLen >= 0) { // Ensure maxLen is non-negative
                scaleFactor = maxLen / currentLen;
            }
            if (scaleFactor != 1.0) { // Avoid unnecessary multiplication
                 x *= scaleFactor;
                 y *= scaleFactor;
            }
            return *this;
        }
        [[nodiscard]] vec2 clampedMagnitude(double minLen, double maxLen) const {
            vec2 copy = *this;
            return copy.clampMagnitude(minLen, maxLen);
        }

        // --- Cross Product (2D returns scalar) ---
        double cross(const vec2& other) const { return (x * other.y) - (y * other.x); }

        // --- Distance methods ---
        double distance(const vec2& other) const { return safe_sqrt(distanceSqr(other)); }
        double distanceSqr(const vec2& other) const { return (other - *this).lengthSquared(); }

        // --- Dot Product ---
        double dot(const vec2& other) const { return x * other.x + y * other.y; }

        // --- Rounding methods ---
        vec2& floor() { x = std::floor(x); y = std::floor(y); return *this; }
        [[nodiscard]] vec2 floored() const { return {std::floor(x), std::floor(y)}; }

        // --- Inversion ---
        vec2& invert() {
            x = safe_divide(1.0, x);
            y = safe_divide(1.0, y);
            return *this;
        }
        [[nodiscard]] vec2 inverted() const {
            return {safe_divide(1.0, x), safe_divide(1.0, y)};
        }

        // --- Finite/Infinite/NaN Checks ---
        bool isFinite() const { return std::isfinite(x) && std::isfinite(y); }
        bool isInfinite() const { return std::isinf(x) || std::isinf(y); }
        bool isNaN() const { return std::isnan(x) || std::isnan(y); }
        bool isZero() const {return x == 0.0 && y == 0.0;}

        // --- Length methods ---
        double length() const {
            if (isInfinite()) return infinity;
            return std::hypot(x, y);
        }
        double lengthSquared() const {
            if (isInfinite()) return infinity;
            return x * x + y * y;
        }

        // --- Linear Interpolation ---
        vec2& lerp(const vec2& target, const double amount) { x = std::lerp(x, target.x, amount); y = std::lerp(y, target.y, amount); return *this; }
        vec2 lerped(const vec2& target, const double amount) const { return {std::lerp(x, target.x, amount), std::lerp(y, target.y, amount)}; }
        
        // --- lineAngle (static method from raylib-wrapper additions) ---
        static double lineAngle(const vec2& start, const vec2& end) { return std::atan2(end.y - start.y, end.x - start.x); }

        // --- Component-wise Max/Min ---
        // Renaming to maxComp/minComp to avoid conflict if a general 'max' is added later.
        vec2& setMax(const vec2& other) { x = std::max(x, other.x); y = std::max(y, other.y); return *this; }
        [[nodiscard]] vec2 max(const vec2& other) const { return {std::max(x, other.x), std::max(y, other.y)}; }
        vec2& setMin(const vec2& other) { x = std::min(x, other.x); y = std::min(y, other.y); return *this; }
        [[nodiscard]] vec2 min(const vec2& other) const { return {std::min(x, other.x), std::min(y, other.y)}; }

        // --- Move Towards ---
        vec2& moveTowards(const vec2& target, double maxDistance) {
            if (maxDistance <= 0.0) return *this; // No movement if maxDistance is non-positive
            *this += (target - *this).normalize() * maxDistance;
            return *this;
        }
        [[nodiscard]] vec2 movedTowards(const vec2& target, double maxDistance) const {
            vec2 result = *this;
            return result.moveTowards(target, maxDistance);
        }

        // --- Normalization ---
        vec2& normalize() {
            if (isZero()) return *this;
            else if (isInfinite()) {
                x = safe_divide(x, infinity);
                y = safe_divide(y, infinity);
            }
            double len = length();
            x /= len;
            y /= len;
            return *this;
        }
        [[nodiscard]] vec2 normalized() const {
            vec2 result = *this;
            return result.normalize();
        }

        // --- Orbit and Rotation ---
        vec2& rotate(const double angleRad) { // Rotates BY angleRad
            double s = std::sin(angleRad);
            double c = std::cos(angleRad);
            double tempX = x * c - y * s;
            y = x * s + y * c;
            x = tempX;
            return *this;
        }
        [[nodiscard]] vec2 rotated(const double angleRad) const {
            vec2 result = *this;
            return result.rotate(angleRad);
        }

        vec2& rotateAngleLerped(const double angleDelta, const double rate) {
            double current_angle = angle();
            double newAngle = std::lerp(current_angle, current_angle + angleDelta, rate);
            double len = length(); // Preserve length
             if (len > 0.0) {
                x = len * std::cos(newAngle);
                y = len * std::sin(newAngle);
            } else {
                x = 0.0; y = 0.0;
            }
            return *this;
        }
        [[nodiscard]] vec2 angleRotatedLerped(const double angleDelta, const double rate) const {
            vec2 copy = *this;
            return copy.rotateAngleLerped(angleDelta, rate);
        }
        
        [[nodiscard]] vec2 opposite() const { return {-x, -y}; } // Same as negated() or unary minus

        vec2& orbit(const vec2& pivot, const double angleRad) { *this -= pivot; rotate(angleRad); return *this += pivot; }
        [[nodiscard]] vec2 orbited(const vec2& pivot, const double angleRad) const { vec2 result = *this; return result.orbit(pivot, angleRad); }

        // --- Perpendicular ---
        [[nodiscard]] vec2 perpendicularCW() const { return {y, -x}; }
        [[nodiscard]] vec2 perpendicularCCW() const { return {-y, x}; }

        // --- Randomization ---
        vec2& randomize(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) {
            static std::mt19937_64 rng{std::random_device{}()}; // Use 64-bit for double
            std::uniform_real_distribution<double> distribution(minVal, maxVal);
            x = distribution(rng);
            y = distribution(rng);
            return *this;
        }
        [[nodiscard]] vec2 randomized(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) const {
            vec2 result;
            return result.randomize(minVal, maxVal);
        }

        // --- Reflection/Refraction ---
        vec2& reflect(vec2 normal) { // Pass normal by value to modify (normalize) it
            normal.normalize();
            if (normal.isZero() || normal.isInfinite()) return *this; // Cannot reflect with invalid normal
            // Formula: R = V - 2 * N * dot(V, N)
            *this -= normal * (2.0 * dot(normal));
            return *this;
        }
        [[nodiscard]] vec2 reflected(vec2 normal) const { // Pass normal by value
            vec2 result = *this;
            return result.reflect(normal);
        }

        vec2& refract(vec2 normal, double indexOfRefractionRatio) {
            normal.normalize();
            if (normal.isZero() || normal.isInfinite()) return *this;

            double dotNI = dot(normal); 
            double k = 1.0 - indexOfRefractionRatio * indexOfRefractionRatio * (1.0 - dotNI * dotNI);

            if (k < 0.0) {
                x = 0.0; y = 0.0; 
            } else {
                *this = (*this * indexOfRefractionRatio) - normal * (indexOfRefractionRatio * dotNI + safe_sqrt(k));
            }
            return *this;
        }
        [[nodiscard]] vec2 refracted(vec2 normal, double indexOfRefractionRatio) const {
            vec2 result = *this;
            return result.refract(normal, indexOfRefractionRatio);
        }

        // --- Rounding ---
        vec2& round() { x = std::round(x); y = std::round(y); return *this; }
        [[nodiscard]] vec2 rounded() const { return {std::round(x), std::round(y)}; }

        // --- Scaling --- (Covered by operators, but can be explicit)
        vec2& scale(const double scalar) { x *= scalar; y *= scalar; return *this; }
        [[nodiscard]] vec2 scaled(const double scalar) const { return {x * scalar, y * scalar}; }
        
        // --- Conversions (from raylib-wrapper additions) ---
        std::array<double, 2> toArray() const { return {x, y}; }
        const double* data() const { return &x; } // Pointer to first element
        double* data() { return &x; }


        // --- Operators ---
        // Vector-Vector
        [[nodiscard]] vec2 operator+(const vec2& other) const { return {x + other.x, y + other.y}; }
        [[nodiscard]] vec2 operator-(const vec2& other) const { return {x - other.x, y - other.y}; }
        [[nodiscard]] vec2 operator*(const vec2& other) const { return {x * other.x, y * other.y}; } // Component-wise
        [[nodiscard]] vec2 operator/(const vec2& other) const { return {safe_divide(x, other.x), safe_divide(y, other.y)}; }

        // Unary minus
        [[nodiscard]] vec2 operator-() const { return {-x, -y}; }

        // Vector-Scalar
        [[nodiscard]] vec2 operator+(const double scalar) const { return {x + scalar, y + scalar}; }
        [[nodiscard]] vec2 operator-(const double scalar) const { return {x - scalar, y - scalar}; }
        [[nodiscard]] vec2 operator*(const double scalar) const { return {x * scalar, y * scalar}; }
        [[nodiscard]] vec2 operator/(const double scalar) const { return {safe_divide(x, scalar), safe_divide(y, scalar)}; }

        // Compound Assignment (Vector-Vector)
        vec2& operator+=(const vec2& other) { x += other.x; y += other.y; return *this; }
        vec2& operator-=(const vec2& other) { x -= other.x; y -= other.y; return *this; }
        vec2& operator*=(const vec2& other) { x *= other.x; y *= other.y; return *this; }
        vec2& operator/=(const vec2& other) { x = safe_divide(x, other.x); y = safe_divide(y, other.y); return *this; }

        // Compound Assignment (Vector-Scalar)
        vec2& operator+=(const double scalar) { x += scalar; y += scalar; return *this; }
        vec2& operator-=(const double scalar) { x -= scalar; y -= scalar; return *this; }
        vec2& operator*=(const double scalar) { x *= scalar; y *= scalar; return *this; }
        vec2& operator/=(const double scalar) { x = safe_divide(x, scalar); y = safe_divide(y, scalar); return *this; }

        bool operator==(const vec2& other) const { return x == other.x && y == other.y; }
        bool operator!=(const vec2& other) const { return !(*this == other); }

        // Comparison (based on length squared for efficiency)
        bool operator<(const vec2& other) const { return lengthSquared() < other.lengthSquared(); }
        bool operator<=(const vec2& other) const { return lengthSquared() <= other.lengthSquared(); }
        bool operator>(const vec2& other) const { return lengthSquared() > other.lengthSquared(); }
        bool operator>=(const vec2& other) const { return lengthSquared() >= other.lengthSquared(); }

        // --- String Conversion ---
        std::string toStr() const { return std::format("vec2[x:{}, y:{}]", x, y); }
    }; // End of struct vec2

    // --- Non-member operators for symmetry (scalar on left) ---
    [[nodiscard]] inline vec2 operator+(const double scalar, const vec2& v) { return v + scalar; }
    [[nodiscard]] inline vec2 operator-(const double scalar, const vec2& v) { return {scalar - v.x, scalar - v.y}; }
    [[nodiscard]] inline vec2 operator*(const double scalar, const vec2& v) { return v * scalar; }
    [[nodiscard]] inline vec2 operator/(const double scalar, const vec2& v) {
        return {safe_divide(scalar, v.x), safe_divide(scalar, v.y)};
    }

    // Stream operator
    inline std::ostream& operator<<(std::ostream& os, const vec2& v) {
        os << v.toStr();
        return os;
    }

} // End of namespace vecSys