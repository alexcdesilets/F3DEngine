#pragma once

#include "vec3.h"

namespace vecSys {
    struct vec4 {
        double x = 0.0, y = 0.0, z = 0.0, w = 0.0;

        vec4() {}
        vec4(const double val): x(val), y(val), z(val), w(val) {}
        vec4(const double valX, const double valY, const double valZ, const double valW): x(valX), y(valY), z(valZ), w(valW) {}
        vec4(const std::vector<double>& vals) {
            size_t size = vals.size();
            if (size == 1) { x = vals[0]; }
            else if (size == 2) { x = vals[0]; y = vals[1]; }
            else if (size == 3) { x = vals[0]; y = vals[1]; z = vals[2]; }
            else if (size >= 4) { x = vals[0]; y = vals[1]; z = vals[2]; w = vals[3]; }
        }
        vec4(const vec2& xy, double valZ = 0.0, double valW = 0.0): x(xy.x), y(xy.y), z(valZ), w(valW) {}
        vec4(const vec3& xyz, double valW = 0.0): x(xyz.x), y(xyz.y), z(xyz.z), w(valW) {}
        vec4(const vec4& other): x(other.x), y(other.y), z(other.z), w(other.w) {}

        // --- Static Factory Methods ---
        static constexpr vec4 zero() { return {0.0, 0.0, 0.0, 0.0}; }
        static constexpr vec4 one() { return {1.0, 1.0, 1.0, 1.0}; }
        static constexpr vec4 unitX() { return {1.0, 0.0, 0.0, 0.0}; }
        static constexpr vec4 unitY() { return {0.0, 1.0, 0.0, 0.0}; }
        static constexpr vec4 unitZ() { return {0.0, 0.0, 1.0, 0.0}; }
        static constexpr vec4 unitW() { return {0.0, 0.0, 0.0, 1.0}; }
        static constexpr vec4 nUnitX() { return {-1.0, 0.0, 0.0, 0.0}; }
        static constexpr vec4 nUnitY() { return {0.0, -1.0, 0.0, 0.0}; }
        static constexpr vec4 nUnitZ() { return {0.0, 0.0, -1.0, 0.0}; }
        static constexpr vec4 nUnitW() { return {0.0, 0.0, 0.0, -1.0}; }
        static vec4 random(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) {
            return vec4().randomize(minVal, maxVal);
        }

        // --- Rounding methods ---
        vec4& ceil() { x = std::ceil(x); y = std::ceil(y); z = std::ceil(z); w = std::ceil(w); return *this; }
        [[nodiscard]] vec4 ceiled() const { return {std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w)}; }
        vec4& floor() { x = std::floor(x); y = std::floor(y); z = std::floor(z); w = std::floor(w); return *this; }
        [[nodiscard]] vec4 floored() const { return {std::floor(x), std::floor(y), std::floor(z), std::floor(w)}; }
        vec4& round() { x = std::round(x); y = std::round(y); z = std::round(z); w = std::round(w); return *this; }
        [[nodiscard]] vec4 rounded() const { return {std::round(x), std::round(y), std::round(z), std::round(w)}; }

        // --- Clamping methods ---
        vec4& clamp(double minValue, double maxValue) {
            x = std::clamp(x, minValue, maxValue);
            y = std::clamp(y, minValue, maxValue);
            z = std::clamp(z, minValue, maxValue);
            w = std::clamp(w, minValue, maxValue);
            return *this;
        }
        [[nodiscard]] vec4 clamped(double minValue, double maxValue) const {
            return {std::clamp(x, minValue, maxValue), std::clamp(y, minValue, maxValue), std::clamp(z, minValue, maxValue), std::clamp(w, minValue, maxValue)};
        }

        vec4& clamp(const vec4& minVec, const vec4& maxVec) {
            x = std::clamp(x, minVec.x, maxVec.x);
            y = std::clamp(y, minVec.y, maxVec.y);
            z = std::clamp(z, minVec.z, maxVec.z);
            w = std::clamp(w, minVec.w, maxVec.w);
            return *this;
        }
        [[nodiscard]] vec4 clamped(const vec4& minVec, const vec4& maxVec) const {
            return {std::clamp(x, minVec.x, maxVec.x), std::clamp(y, minVec.y, maxVec.y), std::clamp(z, minVec.z, maxVec.z), std::clamp(w, minVec.w, maxVec.w)};
        }
        
        vec4& clampMagnitude(double minLen, double maxLen) {
            double currentLenSq = lengthSquared();
            if (currentLenSq == 0.0) {
                return *this;
            }
            double currentLen = safe_sqrt(currentLenSq);
            double scaleFactor = 1.0;
            if (currentLen < minLen && minLen > 0.0) {
                scaleFactor = minLen / currentLen;
            } else if (currentLen > maxLen && maxLen >= 0.0) {
                 if (currentLen > 0.0) {
                    scaleFactor = maxLen / currentLen;
                } else {
                    scaleFactor = 0.0;
                }
            }
            
            if (std::abs(scaleFactor - 1.0) > 0.0 && scaleFactor != 0.0) {
                 x *= scaleFactor; y *= scaleFactor; z *= scaleFactor; w *= scaleFactor;
            } else if (scaleFactor == 0.0 && maxLen == 0.0) {
                 x = 0.0; y = 0.0; z = 0.0; w = 0.0;
            }
            return *this;
        }
        [[nodiscard]] vec4 clampedMagnitude(double minLen, double maxLen) const {
            vec4 copy = *this;
            return copy.clampMagnitude(minLen, maxLen);
        }

        // --- Distance methods ---
        double distance(const vec4& other) const { return safe_sqrt(distanceSqr(other)); }
        double distanceSqr(const vec4& other) const { return (other - *this).lengthSquared(); }

        // --- Dot Product ---
        double dot(const vec4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }

        // --- Inversion ---
        vec4& invert() {
            x = safe_divide(1.0, x);
            y = safe_divide(1.0, y);
            z = safe_divide(1.0, z);
            w = safe_divide(1.0, w);
            return *this;
        }
        [[nodiscard]] vec4 inverted() const {
            return {safe_divide(1.0, x), safe_divide(1.0, y), safe_divide(1.0, z), safe_divide(1.0, w)};
        }

        // --- Finite/Infinite/NaN Checks ---
        bool isFinite() const { return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) && std::isfinite(w); }
        bool isInfinite() const { return std::isinf(x) || std::isinf(y) || std::isinf(z) || std::isinf(w); }
        bool isNaN() const { return std::isnan(x) || std::isnan(y) || std::isnan(z) || std::isnan(w); }
        bool isZero() const { return x == 0.0 && y == 0.0 && z == 0.0 && w == 0.0; }

        // --- Length methods ---
        double length() const {
            if (isInfinite()) return infinity;
            // std::hypot for 4 arguments is not standard until C++17, but x*x+y*y+z*z+w*w is fine.
            return safe_sqrt(x*x + y*y + z*z + w*w);
        }
        double lengthSquared() const {
            if (isInfinite()) return infinity;
            return x*x + y*y + z*z + w*w;
        }

        // --- Linear Interpolation ---
        vec4& lerp(const vec4& target, const double amount) {
            x = std::lerp(x, target.x, amount);
            y = std::lerp(y, target.y, amount);
            z = std::lerp(z, target.z, amount);
            w = std::lerp(w, target.w, amount);
            return *this;
        }
        [[nodiscard]] vec4 lerped(const vec4& target, const double amount) const {
            return {std::lerp(x, target.x, amount), std::lerp(y, target.y, amount), std::lerp(z, target.z, amount), std::lerp(w, target.w, amount)};
        }

        // --- Component-wise Max/Min ---
        vec4& setMax(const vec4& other) { x = std::max(x, other.x); y = std::max(y, other.y); z = std::max(z, other.z); w = std::max(w, other.w); return *this; }
        [[nodiscard]] vec4 max(const vec4& other) const { return {std::max(x, other.x), std::max(y, other.y), std::max(z, other.z), std::max(w, other.w)}; }
        vec4& setMin(const vec4& other) { x = std::min(x, other.x); y = std::min(y, other.y); z = std::min(z, other.z); w = std::min(w, other.w); return *this; }
        [[nodiscard]] vec4 min(const vec4& other) const { return {std::min(x, other.x), std::min(y, other.y), std::min(z, other.z), std::min(w, other.w)}; }

        // --- Move Towards ---
        vec4& moveTowards(const vec4& target, double maxDistance) {
            if (maxDistance <= 0.0) return *this;
            vec4 diff = target - *this;
            double distSq = diff.lengthSquared();
            if (distSq == 0.0 || distSq <= maxDistance * maxDistance) {
                *this = target;
            } else {
                double dist = safe_sqrt(distSq); // dist will be > 0
                *this += (diff / dist) * maxDistance;
            }
            return *this;
        }
        [[nodiscard]] vec4 movedTowards(const vec4& target, double maxDistance) const {
            vec4 copy = *this;
            return copy.moveTowards(target, maxDistance);
        }

        // --- Normalization ---
        vec4& normalize() {
            if (isZero()) return *this;
            if (isInfinite()) {
                x = safe_divide(x, infinity);
                y = safe_divide(y, infinity);
                z = safe_divide(z, infinity);
                w = safe_divide(w, infinity);
                return *this; // Should result in a vector with 0s and +/-1s (or NaN if original was 0/inf)
            }
            double len = length();
            // if (len == 0.0) return *this; // Already handled by isZero()
            x = safe_divide(x,len); // Use safe_divide in case len is extremely small, though normalize usually implies non-zero.
            y = safe_divide(y,len);
            z = safe_divide(z,len);
            w = safe_divide(w,len);
            return *this;
        }
        [[nodiscard]] vec4 normalized() const {
            vec4 copy = *this;
            return copy.normalize();
        }
        
        [[nodiscard]] vec4 opposite() const { return {-x, -y, -z, -w}; }

        // --- Randomization ---
        vec4& randomize(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) {
            static std::mt19937_64 rng{std::random_device{}()};
            std::uniform_real_distribution<double> distribution(minVal, maxVal);
            x = distribution(rng); y = distribution(rng); z = distribution(rng); w = distribution(rng);
            return *this;
        }
        [[nodiscard]] vec4 randomized(double minVal = std::numeric_limits<double>::lowest(), double maxVal = std::numeric_limits<double>::max()) const {
            vec4 copy; return copy.randomize(minVal, maxVal);
        }
        
        // --- Reflection (assuming w is treated similarly to other components) ---
        // Normal is also a vec4; if it's a vec3 normal, conversion is needed.
        vec4& reflect(vec4 normal) { // Pass normal by value to modify (normalize) it
            normal.normalize();
            if (normal.isZero() || normal.isInfinite()) return *this;
            *this -= normal * (2.0 * dot(normal));
            return *this;
        }
        [[nodiscard]] vec4 reflected(vec4 normal) const {
            vec4 result = *this;
            return result.reflect(normal);
        }

        // --- Scaling ---
        vec4& scale(const double scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        [[nodiscard]] vec4 scaled(const double scalar) const { return {x * scalar, y * scalar, z * scalar, w * scalar}; }

        // --- Conversions ---
        std::array<double, 4> toArray() const { return {x, y, z, w}; }
        const double* data() const { return &x; }
        double* data() { return &x; }

        // --- Operators ---
        // Vector-Vector
        [[nodiscard]] vec4 operator+(const vec4& other) const { return {x + other.x, y + other.y, z + other.z, w + other.w}; }
        [[nodiscard]] vec4 operator-(const vec4& other) const { return {x - other.x, y - other.y, z - other.z, w - other.w}; }
        [[nodiscard]] vec4 operator*(const vec4& other) const { return {x * other.x, y * other.y, z * other.z, w * other.w}; } // Component-wise
        [[nodiscard]] vec4 operator/(const vec4& other) const { return {safe_divide(x, other.x), safe_divide(y, other.y), safe_divide(z, other.z), safe_divide(w, other.w)}; }

        // Unary minus
        [[nodiscard]] vec4 operator-() const { return {-x, -y, -z, -w}; }

        // Vector-Scalar
        [[nodiscard]] vec4 operator+(const double scalar) const { return {x + scalar, y + scalar, z + scalar, w + scalar}; }
        [[nodiscard]] vec4 operator-(const double scalar) const { return {x - scalar, y - scalar, z - scalar, w - scalar}; }
        [[nodiscard]] vec4 operator*(const double scalar) const { return {x * scalar, y * scalar, z * scalar, w * scalar}; }
        [[nodiscard]] vec4 operator/(const double scalar) const { return {safe_divide(x, scalar), safe_divide(y, scalar), safe_divide(z, scalar), safe_divide(w, scalar)}; }

        // Compound Assignment (Vector-Vector)
        vec4& operator+=(const vec4& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
        vec4& operator-=(const vec4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
        vec4& operator*=(const vec4& other) { x *= other.x; y *= other.y; z *= other.z; w *= other.w; return *this; }
        vec4& operator/=(const vec4& other) { x = safe_divide(x, other.x); y = safe_divide(y, other.y); z = safe_divide(z, other.z); w = safe_divide(w, other.w); return *this; }

        // Compound Assignment (Vector-Scalar)
        vec4& operator+=(const double scalar) { x += scalar; y += scalar; z += scalar; w += scalar; return *this; }
        vec4& operator-=(const double scalar) { x -= scalar; y -= scalar; z -= scalar; w -= scalar; return *this; }
        vec4& operator*=(const double scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        vec4& operator/=(const double scalar) { x = safe_divide(x, scalar); y = safe_divide(y, scalar); z = safe_divide(z, scalar); w = safe_divide(w, scalar); return *this; }

        bool operator==(const vec4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
        bool operator!=(const vec4& other) const { return !(*this == other); }

        // Comparison (based on length squared for efficiency)
        bool operator<(const vec4& other) const { return lengthSquared() < other.lengthSquared(); }
        bool operator<=(const vec4& other) const { return lengthSquared() <= other.lengthSquared(); }
        bool operator>(const vec4& other) const { return lengthSquared() > other.lengthSquared(); }
        bool operator>=(const vec4& other) const { return lengthSquared() >= other.lengthSquared(); }

        // --- String Conversion ---
        std::string toStr() const { return std::format("vec4[x:{}, y:{}, z:{}, w:{}]", x, y, z, w); }
    }; // End of struct vec4

    // --- Non-member operators for symmetry (scalar on left) ---
    [[nodiscard]] inline vec4 operator+(const double scalar, const vec4& v) { return v + scalar; }
    [[nodiscard]] inline vec4 operator-(const double scalar, const vec4& v) { return {scalar - v.x, scalar - v.y, scalar - v.z, scalar - v.w}; }
    [[nodiscard]] inline vec4 operator*(const double scalar, const vec4& v) { return v * scalar; }
    [[nodiscard]] inline vec4 operator/(const double scalar, const vec4& v) {
        return {safe_divide(scalar, v.x), safe_divide(scalar, v.y), safe_divide(scalar, v.z), safe_divide(scalar, v.w)};
    }

    // Stream operator
    inline std::ostream& operator<<(std::ostream& os, const vec4& v) {
        os << v.toStr();
        return os;
    }

} // End of namespace vecSys