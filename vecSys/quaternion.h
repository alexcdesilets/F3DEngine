#pragma once

#include "vec3.h"
#include "base.h"

namespace vecSys {

    struct quaternion {
        double x = 0.0, y = 0.0, z = 0.0, w = 1.0; // Default to identity quaternion

        // --- Constructors ---
        quaternion() {} // Defaults to identity (0,0,0,1)
        quaternion(const double valX, const double valY, const double valZ, const double valW)
            : x(valX), y(valY), z(valZ), w(valW) {}

        // Constructor from a vector part and a scalar part
        quaternion(const vec3& v, const double s) : x(v.x), y(v.y), z(v.z), w(s) {}

        // Constructor from std::vector<double>
        quaternion(const std::vector<double>& vals) {
            if (vals.size() >= 4) {
                x = vals[0]; y = vals[1]; z = vals[2]; w = vals[3];
            } else if (vals.size() == 3) { // Assume (x,y,z), w=0
                x = vals[0]; y = vals[1]; z = vals[2]; w = 0.0;
            } else if (vals.size() == 1) { // Assume w, (x,y,z) = 0
                w = vals[0]; x = 0.0; y = 0.0; z = 0.0;
            } else { // Default to identity
                x = 0.0; y = 0.0; z = 0.0; w = 1.0;
            }
        }
        
        quaternion(const quaternion& other) = default;


        // --- Static Factory Methods ---
        static constexpr quaternion identity() { return {0.0, 0.0, 0.0, 1.0}; }
        static constexpr quaternion zero() { return {0.0, 0.0, 0.0, 0.0}; } // Additive identity

        // From axis (unit vector) and angle (radians)
        static quaternion fromAxisAngle(vec3 axis, const double radians) {
            if (axis.lengthSquared() == 0.0) return identity();
            axis.normalize();
            double halfAngle = radians * 0.5;
            double s = std::sin(halfAngle);
            return {axis.x * s, axis.y * s, axis.z * s, std::cos(halfAngle)};
        }

        // From Euler angles (radians) - Roll (X), Pitch (Y), Yaw (Z) - ZYX order
        static quaternion fromEulerAngles(const double rollRad, const double pitchRad, const double yawRad) {
            double cr = std::cos(rollRad * 0.5);
            double sr = std::sin(rollRad * 0.5);
            double cp = std::cos(pitchRad * 0.5);
            double sp = std::sin(pitchRad * 0.5);
            double cy = std::cos(yawRad * 0.5);
            double sy = std::sin(yawRad * 0.5);

            return {
                cy * sr * cp - sy * cr * sp,
                cy * cr * sp + sy * sr * cp,
                sy * cr * cp - cy * sr * sp,
                cy * cr * cp + sy * sr * sp
            };
        }
        static quaternion fromEulerAngles(const vec3& eulerRad) {
            return fromEulerAngles(eulerRad.x, eulerRad.y, eulerRad.z);
        }


        // --- Core Quaternion Operations ---
        double lengthSquared() const { return x * x + y * y + z * z + w * w; }
        double length() const { return safe_sqrt(lengthSquared()); }

        quaternion& normalize() {
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
        [[nodiscard]] quaternion normalized() const {
            quaternion q = *this;
            return q.normalize();
        }

        quaternion& conjugate() { x = -x; y = -y; z = -z; return *this; }
        [[nodiscard]] quaternion conjugated() const { return {-x, -y, -z, w}; }

        quaternion& invert() {
            double lenSq = lengthSquared();
            if (lenSq == 0.0) { // Or handle as error
                x = 0; y = 0; z = 0; w = 0; // or NAN
                return *this;
            }
            double lenSqInv = safe_divide(1.0, lenSq);
            conjugate(); // q*
            x *= lenSqInv; y *= lenSqInv; z *= lenSqInv; w *= lenSqInv;
            return *this;
        }
        [[nodiscard]] quaternion inversed() const {
            quaternion q = *this;
            return q.invert();
        }

        double dot(const quaternion& other) const {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        // --- State Checks ---
        bool isZero(double tolerance = default_Tolerance) const {
            return (std::abs(x) < tolerance && std::abs(y) < tolerance && std::abs(z) < tolerance && std::abs(w) < tolerance);
        }
        bool isIdentity(double tolerance = default_Tolerance) const {
            return (std::abs(x) < tolerance && std::abs(y) < tolerance && std::abs(z) < tolerance && std::abs(w - 1.0) < tolerance);
        }
        bool isNormalized(double tolerance = default_Tolerance) const {
            return std::abs(lengthSquared() - 1.0) < tolerance;
        }
        bool isInfinite() {return std::isinf(x) || std::isinf(y) || std::isinf(z) || std::isinf(w);}


        // --- Interpolation ---
        // Nlerp - Normalized Linear Interpolation (faster, good for small steps)
        quaternion& nlerp(quaternion target, const double amount, bool shortestPath = true) {
            double d = dot(target);
            if (shortestPath && d < 0.0) { // Ensure we take the shortest path
                target.x = -target.x; target.y = -target.y; target.z = -target.z; target.w = -target.w;
                d = -d;
            }
            x = std::lerp(x, target.x, amount);
            y = std::lerp(y, target.y, amount);
            z = std::lerp(z, target.z, amount);
            w = std::lerp(w, target.w, amount);
            return normalize();
        }
        [[nodiscard]] quaternion nlerped(const quaternion& target, const double amount, bool shortestPath = true) const {
            quaternion q = *this;
            return q.nlerp(target, amount, shortestPath);
        }
        
        // Slerp - Spherical Linear Interpolation
        quaternion& slerp(quaternion target, const double amount, bool shortestPath = true) {
            double cosTheta = dot(target);

            // If shortestPath is true and dot is negative, negate one quaternion to take the shorter path
            if (shortestPath && cosTheta < 0.0) {
                target.x = -target.x; target.y = -target.y; target.z = -target.z; target.w = -target.w;
                cosTheta = -cosTheta;
            }

            if (cosTheta > 1.0 - default_Tolerance) { // Quaternions are very close, use Nlerp
                return nlerp(target, amount, false); // shortestPath already handled
            } else {
                double angle = std::acos(cosTheta); // theta
                double sinThetaInv = safe_divide(1.0, std::sin(angle));
                double scale0 = std::sin((1.0 - amount) * angle) * sinThetaInv;
                double scale1 = std::sin(amount * angle) * sinThetaInv;
                
                x = scale0 * x + scale1 * target.x;
                y = scale0 * y + scale1 * target.y;
                z = scale0 * z + scale1 * target.z;
                w = scale0 * w + scale1 * target.w;
            }
            return *this; // Result of slerp is already normalized if inputs are.
        }
        [[nodiscard]] quaternion slerped(const quaternion& target, const double amount, bool shortestPath = true) const {
            quaternion q = *this;
            return q.slerp(target, amount, shortestPath);
        }


        // --- Transformations ---
        // Rotate a 3D vector by this quaternion
        [[nodiscard]] vec3 transform(const vec3& v) const {
            // q * v * q^-1  (where v is pure quaternion (x,y,z,0))
            // Assumes this quaternion is normalized.
            // If not, use q_norm = this->normalized();
            
            // vec3 u = {x, y, z}; // Vector part of quaternion
            // double s = w;      // Scalar part of quaternion
            // return u * (2.0 * u.dot(v)) + v * (s*s - u.dot(u)) + u.cross(v) * (2.0 * s);
            // Above is an optimized version. Standard way:
            quaternion p = {v.x, v.y, v.z, 0.0};
            quaternion q_conj = conjugated(); // If normalized, this is also the inverse
            quaternion result_q = (*this * p) * q_conj;
            return {result_q.x, result_q.y, result_q.z};
        }

        // --- Conversions ---
        // To axis (unit vector) and angle (radians)
        void toAxisAngle(vec3& outAxis, double outRadians) const {
            quaternion q_norm = normalized(); // Ensure quaternion is normalized
            
            if (std::abs(q_norm.w) > 1.0 - default_Tolerance) { // Handle identity or near-identity
                outRadians = 0.0;
                outAxis = vec3::unitX(); // Default axis, can be anything as angle is 0
                return;
            }
            
            outRadians = 2.0 * std::acos(q_norm.w);
            double s = safe_sqrt(1.0 - q_norm.w * q_norm.w); // sin(angle/2)
            if (s < default_Tolerance) { // Should not happen if w is not +/-1
                outAxis = {q_norm.x, q_norm.y, q_norm.z}; // Or set to a default axis
            } else {
                outAxis = {q_norm.x / s, q_norm.y / s, q_norm.z / s};
            }
            outAxis.normalize(); // Ensure unit vector due to potential precision issues
        }

        // To Euler angles (radians) - Roll (X), Pitch (Y), Yaw (Z) - ZYX order
        [[nodiscard]] vec3 toEulerAngles() const {
            vec3 angles;
            quaternion q = normalized(); // Work with a normalized quaternion

            // Roll (x-axis rotation)
            double sinr_cosp = 2.0 * (q.w * q.x + q.y * q.z);
            double cosr_cosp = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
            angles.x = std::atan2(sinr_cosp, cosr_cosp);

            // Pitch (y-axis rotation)
            double sinp = 2.0 * (q.w * q.y - q.z * q.x);
            if (std::abs(sinp) >= 1.0)
                angles.y = std::copysign(pi / 2.0, sinp); // Use 90 degrees if out of range
            else
                angles.y = std::asin(sinp);

            // Yaw (z-axis rotation)
            double siny_cosp = 2.0 * (q.w * q.z + q.x * q.y);
            double cosy_cosp = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
            angles.z = std::atan2(siny_cosp, cosy_cosp);

            return angles;
        }
        
        // --- Operators ---
        // Quaternion-Quaternion Multiplication (Hamilton Product)
        [[nodiscard]] quaternion operator*(const quaternion& other) const {
            return {
                w * other.x + x * other.w + y * other.z - z * other.y, // x
                w * other.y - x * other.z + y * other.w + z * other.x, // y
                w * other.z + x * other.y - y * other.x + z * other.w, // z
                w * other.w - x * other.x - y * other.y - z * other.z  // w
            };
        }
        quaternion& operator*=(const quaternion& other) {
            *this = *this * other;
            return *this;
        }

        // Quaternion-Scalar Multiplication
        [[nodiscard]] quaternion operator*(const double scalar) const {
            return {x * scalar, y * scalar, z * scalar, w * scalar};
        }
        quaternion& operator*=(const double scalar) {
            x *= scalar; y *= scalar; z *= scalar; w *= scalar;
            return *this;
        }

        // Quaternion-Scalar Division
        [[nodiscard]] quaternion operator/(const double scalar) const {
            return {safe_divide(x, scalar), safe_divide(y, scalar), safe_divide(z, scalar), safe_divide(w, scalar)};
        }
        quaternion& operator/=(const double scalar) {
            x = safe_divide(x, scalar); y = safe_divide(y, scalar);
            z = safe_divide(z, scalar); w = safe_divide(w, scalar);
            return *this;
        }

        // Quaternion Addition (less common for rotations, but defined)
        [[nodiscard]] quaternion operator+(const quaternion& other) const {
            return {x + other.x, y + other.y, z + other.z, w + other.w};
        }
        quaternion& operator+=(const quaternion& other) {
            x += other.x; y += other.y; z += other.z; w += other.w;
            return *this;
        }

        // Quaternion Subtraction
        [[nodiscard]] quaternion operator-(const quaternion& other) const {
            return {x - other.x, y - other.y, z - other.z, w - other.w};
        }
        quaternion& operator-=(const quaternion& other) {
            x -= other.x; y -= other.y; z -= other.z; w -= other.w;
            return *this;
        }
        
        // Unary minus
        [[nodiscard]] quaternion operator-() const {
            return {-x, -y, -z, -w};
        }

        // Equality
        bool operator==(const quaternion& other) const {
            // Could use a tolerance for floating point comparison
            return (x == other.x && y == other.y && z == other.z && w == other.w);
        }
        bool operator!=(const quaternion& other) const {
            return !(*this == other);
        }
        
        // --- String Conversion ---
        std::string toStr() const {
            return std::format("quat[x:{}, y:{}, z:{}, w:{}]", x, y, z, w);
        }

        // --- Other Type Manipulators ---
        [[nodiscard]] inline vec2 rotateVec2(const vec2& v) {
            vec3 v3d(v.x, v.y, 0.0);
            vec3 rotated_v3d = transform(v3d);
            return vec2(rotated_v3d.x, rotated_v3d.y);
        }

        [[nodiscard]] inline vec2 inverseRotateVec2(const vec2& v) {
            vec3 v3d(v.x, v.y, 0.0);
            // For a unit quaternion, inverse is conjugate
            vec3 rotated_v3d = conjugated().transform(v3d);
            return vec2(rotated_v3d.x, rotated_v3d.y);
        }

        [[nodiscard]] inline vec3 rotateVec3(const vec3& v) {
            return transform(v);
        }

        [[nodiscard]] inline vec3 rotateVec2ToVec3(const vec2& v) {
            vec3 v3d(v.x, v.y, 0.0);
            return transform(v3d);
        }

        [[nodiscard]] inline vec3 InverseRotateVec3(const vec3& v) {
            // For a unit quaternion (normalized), the inverse is its conjugate.
            return conjugated().transform(v);
        }
    }; // End of struct quaternion


    // --- Non-member operators for symmetry (scalar on left) ---
    [[nodiscard]] inline quaternion operator*(const double scalar, const quaternion& q) {
        return q * scalar;
    }

    // Stream operator
    inline std::ostream& operator<<(std::ostream& os, const quaternion& q) {
        os << q.toStr();
        return os;
    }
} // End of namespace vecSys