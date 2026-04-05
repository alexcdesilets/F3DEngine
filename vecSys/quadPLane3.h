#pragma once
#include <limits>     // For std::numeric_limits
#include <cmath>      // For std::abs, safe_sqrt, std::acos
#include <stdexcept>  // For potential exceptions
#include <numbers>
#include <vector>     // For getEdges
#include <array>      // For storing points
#include <numeric>    // For std::iota (optional)
#include <algorithm>  // For std::clamp, std::max, std::min, std::reverse
#include <tuple>      // For returning multiple values (optional)

#include "base.h"
#include "vec2.h"
#include "vec3.h"     // Needs double-specialized vec3
#include "line3.h"    // Needs double-specialized line3
#include "pline3.h"   // Needs pLine3 for overlapsAABB
// Include plane3 definition if available and needed for helper calculations,
// otherwise, we can perform calculations directly.
// #include "plane3.h"

// Forward declaration for QuadPlane3 overlaps check
// struct QuadPlane3; // Not needed as it's defined in this file

namespace vecSys {
    struct QuadPlane3 {
        // --- Data Members (POD Style) ---
        // Store points directly in a fixed-size array.
        std::array<vec3, 4> points; // p0, p1, p2, p3

        // --- Constructors ---

        // Default constructor: Creates a unit square on the XY plane at z=0 (CCW order)
        QuadPlane3() : points{vec3{0.0, 0.0, 0.0}, vec3{1.0, 0.0, 0.0}, vec3{1.0, 1.0, 0.0}, vec3{0.0, 1.0, 0.0}} {}

        // Constructor from four points
        QuadPlane3(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3) : points{p0, p1, p2, p3} {
            // Optionally add a check here or a separate method isCoplanar()
            // if strict planarity is required upon construction.
            // if (!isCoplanar()) { /* Throw or warn */ }
        }

        // --- Helper Methods (Calculated Properties) ---

        // Calculate the normal vector of the plane defined by the quad.
        // Assumes CCW winding (p0, p1, p2, p3). Uses p0, p1, p3 for calculation.
        // Returns a normalized vector. Returns {0,0,0} if points are degenerate.
        vec3 normal() const {
            // Assumes vec3::isZero and vec3::normalized handle tolerance internally if needed.
            vec3 v1 = points[1] - points[0]; vec3 v2 = points[3] - points[0];
            vec3 n = v1.cross(v2);
            if (!n.isZero()) { return n.normalize(); }
            // Handle degenerate case
            v1 = points[2] - points[1]; v2 = points[0] - points[1];
            n = v1.cross(v2);
            return n.isZero() ? vec3{0.0, 0.0, 0.0} : n.normalize();
        }


        // Calculate the center point (average of vertices)
        vec3 centerPoint() const {
            // Array always has 4 points
            vec3 sum = {0.0, 0.0, 0.0};
            sum += points[0];
            sum += points[1];
            sum += points[2];
            sum += points[3];
            return sum / 4.0;
        }

        // Calculate the area of the quadrilateral.
        // Assumes a convex or non-self-intersecting quad. Works by splitting into two triangles.
        double area() const {
             // Area = Area(p0, p1, p2) + Area(p0, p2, p3)
             // Triangle area = 0.5 * |(p1-p0) x (p2-p0)|
             vec3 n1 = (points[1] - points[0]).cross(points[2] - points[0]);
             vec3 n2 = (points[2] - points[0]).cross(points[3] - points[0]);
             // For a planar quad, normals n1 and n2 should point in the same direction (or be zero).
             // Adding their lengths works for convex quads.
             return 0.5 * (n1.length() + n2.length());
        }

        // Check if the four points are coplanar within a tolerance.
        bool isCoplanar(double tolerance = default_Tolerance) const {
            vec3 n = normal();
            if (n.isZero()) return true; // Degenerate case is technically coplanar
            // Check if the 4th point's distance to the plane defined by the first 3 is near zero.
            // Distance = normal · (p4 - p0)
            double dist = n.dot(points[3] - points[0]);
            return std::abs(dist) <= tolerance;
        }

        // Calculate the Axis-Aligned Bounding Box
        std::pair<vec3, vec3> boundingBox() const {
            // Assumes points array is always size 4
            vec3 minPt = points[0];
            vec3 maxPt = points[0];
            minPt = minPt.minComp(points[1]);
            maxPt = maxPt.maxComp(points[1]);
            minPt = minPt.minComp(points[2]);
            maxPt = maxPt.maxComp(points[2]);
            minPt = minPt.minComp(points[3]);
            maxPt = maxPt.maxComp(points[3]);
            return {minPt, maxPt};
        }

        // --- Winding Order ---

        // Reverse the order of the vertices in-place.
        QuadPlane3& reverseWinding() {
            // Swap p1 and p3 to reverse winding
            std::swap(points[1], points[3]);
            return *this;
        }

        // Return a new QuadPlane3 with reversed winding order.
        QuadPlane3 reversedWinding() const {
            QuadPlane3 mod = *this;
            mod.reverseWinding();
            return mod;
        }

        // Check if the winding order (p0,p1,p2,p3) is clockwise relative to the calculated normal.
        // Note: This depends on the normal calculation method.
        bool isClockwise() const {
            // A simple check: If the normal calculated from (p1-p0) x (p2-p1)
            // points opposite to the primary normal calculated from (p1-p0) x (p3-p0).
            // This assumes a relatively well-behaved quad.
            vec3 n_primary = normal();
            if (n_primary.isZero()) return false; // Undefined for degenerate

            vec3 n_test = (points[1] - points[0]).cross(points[2] - points[1]);
            // If the dot product is negative, they point in opposite directions, suggesting CW order
            // relative to the primary normal calculation.
            return n_primary.dot(n_test) < 0.0;
        }

        // Ensures the winding order is counter-clockwise relative to the calculated normal.
        QuadPlane3& ensureCounterClockwise() {
            if (isClockwise()) {
                reverseWinding();
            }
            return *this;
        }

        // --- Intersection Methods ---

        // Helper: Check if point P (known to be on the plane) is inside the quad.
        // This version uses summation of angles. Robust for convex & concave quads (if simple).
        // Returns true if the sum of angles subtended by edges at P is close to 2*PI.
        bool isPointInQuad(const vec3& P, double tolerance = default_Tolerance) const {
            double angleSum = 0.0;
            double toleranceSq = tolerance * tolerance;
            double scaledpi_tolerance = tolerance * 10.0;

            for (size_t i = 0; i < 4; ++i) {
                vec3 v1 = points[i] - P;
                vec3 v2 = points[(i + 1) % 4] - P;
                double lenSq1 = v1.lengthSquared();
                double lenSq2 = v2.lengthSquared();

                if (lenSq1 < toleranceSq || lenSq2 < toleranceSq) return true; // Point near vertex

                double lengths_product = safe_sqrt(lenSq1 * lenSq2);
                if (lengths_product < tolerance) continue; // Avoid division by near-zero

                // Denominator (lengths_product) is guaranteed > tolerance, standard division is safe.
                double ratio = v1.dot(v2) / lengths_product;
                double cosTheta = std::clamp(ratio, -1.0, 1.0);
                angleSum += std::acos(cosTheta);
            }
            return std::abs(angleSum - (2.0 * pi)) < scaledpi_tolerance;
        }


        // Intersect a line segment with the finite quadrilateral plane.
        // Returns the intersection point if it exists and lies within the quad boundary.
        vec3 intersectSegment(const line3& segment, double tolerance = default_Tolerance) const {
            vec3 planeNormal = normal();
            if (planeNormal.isZero()) return vec3{infinity, infinity, infinity};

            vec3 lineDir = segment.direction();
            double n_dot_dir = planeNormal.dot(lineDir);

            if (std::abs(n_dot_dir) < tolerance) { // Parallel check
                return vec3{infinity, infinity, infinity}; // Treat parallel/coplanar as no single intersection
            }

            // Denominator (n_dot_dir) is guaranteed > tolerance, standard division is safe.
            double t = planeNormal.dot(points[0] - segment.pnt1) / n_dot_dir;

            if (t >= -tolerance && t <= 1.0 + tolerance) { // Check segment bounds
                vec3 planeIntersectionPoint = segment.pnt1 + lineDir * t;
                if (isPointInQuad(planeIntersectionPoint, tolerance)) { // Check quad bounds
                    return planeIntersectionPoint;
                }
            }
            return vec3{infinity, infinity, infinity}; // Outside segment or quad bounds
        }

        // Intersect an infinite line with the finite quadrilateral plane.
        // Returns the intersection point if it exists and lies within the quad boundary.
        // Returns NaN vector otherwise (including if line lies within the plane).
        vec3 intersectLine(const line3& line, double tolerance = default_Tolerance) const {
            vec3 planeNormal = normal();
            if (planeNormal.isZero()) return vec3{infinity, infinity, infinity};

            vec3 lineDir = line.direction();
            double n_dot_dir = planeNormal.dot(lineDir);

            if (std::abs(n_dot_dir) < tolerance) { // Parallel check
                 return vec3{infinity, infinity, infinity};
            }

            // Denominator (n_dot_dir) is guaranteed > tolerance, standard division is safe.
            double t = planeNormal.dot(points[0] - line.pnt1) / n_dot_dir;

            vec3 planeIntersectionPoint = line.pnt1 + lineDir * t;

            if (isPointInQuad(planeIntersectionPoint, tolerance)) { // Check quad bounds
                return planeIntersectionPoint;
            }
            return vec3{infinity, infinity, infinity}; // Outside quad bounds
       }

        // --- AABB Overlap Methods ---

        // Check for AABB overlap with a line3
        bool overlapsAABB(const line3& other) const {
            auto [minA, maxA] = boundingBox();
            // Assuming line3 has a boundingBox() method
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }

        // Check for AABB overlap with a pLine3
        bool overlapsAABB(const pLine3& other) const {
            auto [minA, maxA] = boundingBox();
             // Assuming pLine3 has a getBoundingBox() method
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }

        // Check for AABB overlap with another QuadPlane3
        bool overlapsAABB(const QuadPlane3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }

        // --- Other Methods ---

        // Get the 4 edges of the quad as line3 segments
        std::vector<line3> getEdges() const {
            std::vector<line3> edges;
            edges.reserve(4);
            edges.emplace_back(points[0], points[1]);
            edges.emplace_back(points[1], points[2]);
            edges.emplace_back(points[2], points[3]);
            edges.emplace_back(points[3], points[0]); // Closing edge
            return edges;
        }

        // Map a 3D point known to be on the plane to 2D planar coordinates (UV-like).
        // This version scales coordinates relative to the edge lengths p0->p1 and p0->p3.
        // Useful for mapping a texture directly to the quad shape.
        // Returns {NaN, NaN} if the quad is degenerate or axes cannot be formed.
        vec2 getPlanarCoordsScaled(const vec3& pointOnPlane, double tolerance = default_Tolerance) const {
            const vec3& origin = points[0];
            vec3 uAxis = points[1] - origin;
            vec3 vAxis = points[3] - origin;

            double uLenSq = uAxis.lengthSquared();
            double vLenSq = vAxis.lengthSquared();

            // Check for degenerate axes using tolerance
            if (uLenSq < tolerance * tolerance || vLenSq < tolerance * tolerance) {
                return vec2{infinity, infinity};
            }

            vec3 vec = pointOnPlane - origin;
            // Denominators uLenSq/vLenSq guaranteed > tolerance^2, standard division safe.
            double uCoord = vec.dot(uAxis) / uLenSq;
            double vCoord = vec.dot(vAxis) / vLenSq;

            return {uCoord, vCoord};
        }

        // Map a 3D point known to be on the plane to 2D planar coordinates (UV-like).
        // This version uses an orthonormal basis derived from p0, p1, p3.
        // The coordinates represent world-space distances along these axes.
        // Useful for mapping textures without stretching relative to world space.
        // Returns {NaN, NaN} if the quad is degenerate.
        vec2 getPlanarCoordsOrtho(const vec3& pointOnPlane, double tolerance = default_Tolerance) const {
            const vec3& origin = points[0];
            vec3 uAxis = points[1] - origin;

            double uLenSq = uAxis.lengthSquared();
            if (uLenSq < tolerance * tolerance) { // Check degenerate edge
                 return vec2{infinity, infinity}; // Use infinity return
            }
            // Denominator guaranteed > tolerance^2, standard division safe.
            vec3 uNorm = uAxis / safe_sqrt(uLenSq);

            vec3 n = normal();
            // Assumes vec3::isZero uses appropriate tolerance check
            if (n.isZero()) {
                 return vec2{infinity, infinity}; // Degenerate plane
            }

            vec3 vNorm = n.cross(uNorm).normalize();
            // Assumes vec3::isZero uses appropriate tolerance check
            if (vNorm.isZero()) {
                 // Fallback logic... (can be simplified if isZero check is robust)
                 vec3 altVAxis = points[3] - origin;
                 if(!altVAxis.isZero()) { vNorm = n.cross(altVAxis.normalized()).normalize(); }
                 if (vNorm.isZero()){ return vec2{infinity, infinity}; } // Still degenerate
            }

            vec3 vec = pointOnPlane - origin;
            double uCoord = vec.dot(uNorm);
            double vCoord = vec.dot(vNorm);

            return {uCoord, vCoord};
       }


        // --- Transformations (Modify in-place) ---

        QuadPlane3& translate(const vec3& offset) {
            points[0] += offset;
            points[1] += offset;
            points[2] += offset;
            points[3] += offset;
            return *this;
        }

        QuadPlane3& rotate(const vec3& pivot, const vec3& axis, double rads) {
            points[0].orbit(pivot, axis, rads);
            points[1].orbit(pivot, axis, rads);
            points[2].orbit(pivot, axis, rads);
            points[3].orbit(pivot, axis, rads);
            return *this;
        }

        // Rotate around the quad's center point
        QuadPlane3& rotateCenter(const vec3& axis, double rads) {
            vec3 center = centerPoint();
            return rotate(center, axis, rads);
        }

        QuadPlane3& orbit(const vec3& orbitCenter, const vec3& axis, double rads) {
            vec3 center = centerPoint();
            vec3 translation = center.orbited(orbitCenter, axis, rads) - center;
            return translate(translation);
        }

        QuadPlane3& scale(const vec3& pivot, double factor) {
            points[0] = pivot + (points[0] - pivot) * factor;
            points[1] = pivot + (points[1] - pivot) * factor;
            points[2] = pivot + (points[2] - pivot) * factor;
            points[3] = pivot + (points[3] - pivot) * factor;
            return *this;
        }

        // Scale around the quad's center point
        QuadPlane3& scaleCenter(double factor) {
            vec3 center = centerPoint();
            return scale(center, factor);
        }

        // --- Transformations (Return new object) ---

        QuadPlane3 translated(const vec3& offset) const {
            QuadPlane3 mod = *this;
            mod.translate(offset);
            return mod;
        }

        QuadPlane3 rotated(const vec3& pivot, const vec3& axis, double rads) const {
            QuadPlane3 mod = *this;
            mod.rotate(pivot, axis, rads);
            return mod;
        }

        QuadPlane3 rotatedCenter(const vec3& axis, double rads) const {
            QuadPlane3 mod = *this;
            mod.rotateCenter(axis, rads);
            return mod;
        }

        QuadPlane3 orbited(const vec3& orbitCenter, const vec3& axis, double rads) const {
            QuadPlane3 mod = *this;
            mod.orbit(orbitCenter, axis, rads);
            return mod;
        }

        QuadPlane3 scaled(const vec3& pivot, double factor) const {
            QuadPlane3 mod = *this;
            mod.scale(pivot, factor);
            return mod;
        }

        QuadPlane3 scaledCenter(double factor) const {
            QuadPlane3 mod = *this;
            mod.scaleCenter(factor);
            return mod;
        }

    }; // End struct QuadPlane3

} // End namespace vecSys