#pragma once

#include "base.h"
#include "vec3.h"     // Needs double-specialized vec3
#include "vec2.h"     // Needs double-specialized vec2 for UV return type
#include "line3.h"    // Needs double-specialized line3
#include "pline3.h"   // Needs pLine3 for overlapsAABB
#include "QuadPlane3.h" // Needs QuadPlane3 for overlapsAABB

namespace vecSys {

    // Represents a triangle strip defined by an ordered sequence of vertices.
    struct TriangleStrip3 {
        // --- Data Members (POD Style) ---
        std::vector<vec3> points;

        // --- Helper Members for Surface UV ---
        mutable std::vector<double> cache_cumulativeRailLengths; // Stores cumulative length along the even-indexed rail (p0->p2->p4...)
        mutable double cache_totalRailLength = -1.0;             // Total length of the even-indexed rail. -1 indicates dirty.
        mutable bool cache_railLengthsDirty = true;              // Flag to indicate if lengths need recalculation
        mutable std::vector<vec3> cache_normals;
        mutable bool cache_normalsDirty = true;

        // --- Constructors ---

        // Default constructor
        TriangleStrip3() = default;

        // Constructor from a list of points
        TriangleStrip3(const std::vector<vec3>& pts) : points(pts), cache_railLengthsDirty(true) {}

        // Constructor using initializer list
        TriangleStrip3(std::initializer_list<vec3> il) : points(il), cache_railLengthsDirty(true) {}

        // --- Calculated Properties ---

        // Calculate the center point (average of vertices)
        vec3 centerPoint() const {
            if (points.empty()) {
                return {0.0, 0.0, 0.0};
            }
            vec3 sum = {0.0, 0.0, 0.0};
            for (const auto& p : points) {
                sum += p;
            }
            // Ensure double division
            // Using standard division as points.size() cannot be zero here.
            return sum / static_cast<double>(points.size());
        }

        // Calculate the Axis-Aligned Bounding Box
        std::pair<vec3, vec3> boundingBox() const {
            if (points.empty()) {
                return {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
            }
            vec3 minPt = points[0];
            vec3 maxPt = points[0];
            // Assumes vec3::minComp and vec3::maxComp exist
            for (size_t i = 1; i < points.size(); ++i) {
                minPt = minPt.minComp(points[i]);
                maxPt = maxPt.maxComp(points[i]);
            }
            return {minPt, maxPt};
        }

        // Get the number of vertices in the strip
        size_t numVertices() const {
            return points.size();
        }

        // Get the number of triangles defined by the strip
        size_t numTriangles() const {
            return (points.size() >= 3) ? points.size() - 2 : 0;
        }

        // Get the vertices of the i-th triangle in the strip.
        // Handles the alternating winding order.
        // Returns false if index is out of bounds.
        bool getTriangle(size_t index, vec3& outP0, vec3& outP1, vec3& outP2) const {
            if (index >= numTriangles()) {
                return false; // Index out of bounds
            }

            // Determine vertices based on index parity for winding order
            if (index % 2 == 0) { // Even index: (i, i+1, i+2)
                outP0 = points[index];
                outP1 = points[index + 1];
                outP2 = points[index + 2];
            } else { // Odd index: (i+1, i, i+2) -> swapped order for consistent winding
                // Note: MT function needs vertices in a consistent order for barycentric calculation.
                // We pass (p_k+1, p_k, p_k+2) to MT for odd triangles.
                outP0 = points[index + 1]; // Middle vertex of previous tri
                outP1 = points[index];     // First vertex of previous tri
                outP2 = points[index + 2]; // New vertex
            }
            return true;
        }

        // Calculate the total surface area of all triangles in the strip.
        double totalArea() const {
            double total = 0.0;
            vec3 p0, p1, p2; // Temporary variables for getTriangle
            for (size_t i = 0; i < numTriangles(); ++i) {
                // Get the vertices in the order they define the triangle for area calc
                vec3 v0, v1, v2;
                 if (i % 2 == 0) { // Even triangle: (i, i+1, i+2)
                     v0 = points[i]; v1 = points[i+1]; v2 = points[i+2];
                 } else { // Odd triangle: (i+1, i, i+2)
                     // Area calculation uses magnitude of cross product, order matters for sign but not magnitude.
                     // Use (i+1, i, i+2) vertices for consistency with getTriangle's odd case logic if needed elsewhere.
                     v0 = points[i+1]; v1 = points[i]; v2 = points[i+2];
                 }
                // Assumes vec3::cross and vec3::length exist and handle precision.
                total += 0.5 * (v1 - v0).cross(v2 - v0).length();
            }
            return total;
        }

        // --- Point Manipulation ---
        // Mark rail lengths as dirty whenever points are modified.

        void append(const std::vector<vec3>& other_points) {
             points.insert(points.end(), other_points.begin(), other_points.end());
             cache_railLengthsDirty = true;
             cache_normalsDirty = true;
        }
        void append(const TriangleStrip3& other) {
             append(other.points); // cache_railLengthsDirty is set by the other append
        }

        void insertPoint(size_t index, const vec3& point) {
            if (index >= points.size()) {
                points.push_back(point);
            } else {
                points.insert(points.begin() + index, point);
            }
            cache_railLengthsDirty = true;
            cache_normalsDirty = true;
        }

        void removePoint(size_t index) {
            if (index < points.size()) {
                points.erase(points.begin() + index);
                cache_railLengthsDirty = true;
                cache_normalsDirty = true;
            }
        }

        void precomputeNormals() const {
            if (!cache_normalsDirty && cache_normals.size() == numTriangles()) return;
            cache_normals.clear();
            cache_normals.reserve(numTriangles());
            
            vec3 triP0, triP1, triP2;
            for (size_t i = 0; i < numTriangles(); ++i) {
                if (getTriangle(i, triP0, triP1, triP2)) {
                    vec3 n = (triP1 - triP0).cross(triP2 - triP0);
                    if (!n.isZero()) n = n.normalize();
                    cache_normals.push_back(n);
                }
            }
            cache_normalsDirty = false;
        }

        // --- Rail Length Calculation (for Surface UV) ---

        // Precomputes cumulative lengths along the even-indexed rail (p0, p2, p4...)
        // Should be called before using calculateSurfaceUV if points have changed.
        void precomputeRailLengths() const {
            // Only recompute if dirty or not computed yet
            if (!cache_railLengthsDirty && cache_totalRailLength >= 0.0) return;

            cache_cumulativeRailLengths.clear();
            cache_totalRailLength = 0.0;

            // Need at least 3 points (p0, p1, p2) for the first rail segment (p0->p2)
            if (points.size() < 3) {
                cache_railLengthsDirty = false; // Mark as computed (even if empty)
                return;
            }

            cache_cumulativeRailLengths.push_back(0.0); // Distance at p0 is 0

            // Iterate through even indices i = 0, 2, 4, ... up to points.size() - 3
            for (size_t i = 0; (i + 2) < points.size(); i += 2) {
                // Assumes vec3::length handles precision.
                double segmentLength = (points[i + 2] - points[i]).length();
                cache_totalRailLength += segmentLength;
                cache_cumulativeRailLengths.push_back(cache_totalRailLength); // Store cumulative length *at* vertex p_{i+2}
            }

            cache_railLengthsDirty = false; // Mark as computed
        }


        // Helper to get the precomputed cumulative distance along the rail up to vertex p_k
        // where k is the index of the vertex *starting* the rail segment (must be even).
        double getDistanceAlongRail(size_t k) const {
            if (cache_railLengthsDirty) {
                precomputeRailLengths();
            }
            // k must be an even index for this interpretation
            // No explicit check needed here as usage in calculateSurfaceUV is correct.

            size_t railIndex = k / 2; // Index into the cache_cumulativeRailLengths vector
            if (railIndex < cache_cumulativeRailLengths.size()) {
                return cache_cumulativeRailLengths[railIndex];
            }

            // Fallback: If index is out of bounds (should not happen with valid triangleIndex)
            // Return total length if it seems like the end, otherwise 0.
            return (railIndex == 0 && points.size() < 3) ? 0.0 : cache_totalRailLength;
        }

        // --- Intersection ---

        // Helper: Möller–Trumbore ray-triangle intersection algorithm.
        // Returns true if intersection occurs, false otherwise.
        // Outputs 't' (distance along ray), 'u', 'v' (barycentric coords relative to triP0, triP1, triP2).
        static bool rayTriangleIntersectMT(
            const vec3& rayOrigin, const vec3& rayDir,
            const vec3& triP0, const vec3& triP1, const vec3& triP2,
            double& outT, double& outU, double& outV,
            double tolerance = default_Tolerance) // Added tolerance parameter
        {
            vec3 edge1 = triP1 - triP0;
            vec3 edge2 = triP2 - triP0;
            vec3 h = rayDir.cross(edge2); // Assumes vec3::cross exists
            double a = edge1.dot(h);      // Assumes vec3::dot exists

            // Check if ray is parallel to triangle plane (within tolerance)
            if (std::abs(a) < tolerance)
                return false;

            // Denominator 'a' is guaranteed > tolerance, standard division is safe.
            double f = 1.0 / a;
            vec3 s = rayOrigin - triP0;
            outU = f * s.dot(h); // Weight for triP1

            // Check bounds with tolerance
            if (outU < -tolerance || outU > 1.0 + tolerance)
                return false;

            vec3 q = s.cross(edge1);
            outV = f * rayDir.dot(q); // Weight for triP2

            // Check bounds with tolerance
            if (outV < -tolerance || outU + outV > 1.0 + tolerance)
                return false;

            // At this stage we can compute t to find out where the intersection point is on the line.
            outT = f * edge2.dot(q);

            // Check if triangle is behind the ray origin (t < 0)
            // Allow slightly negative t for tolerance
            return (outT > -tolerance);
        }


        // Intersect a line segment with the triangle strip.
        // Returns the closest intersection point to the segment start, or infinity vector if no hit.
        vec3 intersectSegment(const line3& segment, double tolerance = default_Tolerance) const { // Added tolerance
            double min_t = infinity;
            vec3 closestHit = {infinity, infinity, infinity}; // Use infinity vector for "no hit" state
            bool hitFound = false;

            vec3 rayOrigin = segment.pnt1;
            vec3 rayDir = segment.direction();
            double segmentLen = rayDir.length();
            // Check for degenerate segment using tolerance
            if (segmentLen < tolerance) return vec3{infinity, infinity, infinity};

            // Denominator segmentLen guaranteed > tolerance, standard division is safe.
            rayDir /= segmentLen; // Normalize direction

            vec3 triP0, triP1, triP2;
            for (size_t i = 0; i < numTriangles(); ++i) {
                if (getTriangle(i, triP0, triP1, triP2)) {
                    double t, u, v;
                    // Pass tolerance to intersection test
                    if (rayTriangleIntersectMT(rayOrigin, rayDir, triP0, triP1, triP2, t, u, v, tolerance)) {
                        // Check if intersection distance 't' is within segment length (using tolerance)
                        if (t >= -tolerance && t <= segmentLen + tolerance) {
                             if (t < min_t) {
                                 min_t = t;
                                 closestHit = rayOrigin + rayDir * t;
                                 hitFound = true;
                             }
                        }
                    }
                }
            }
            return hitFound ? closestHit : vec3{infinity, infinity, infinity};
        }

        // Intersect an infinite line with the triangle strip.
        // Returns the closest intersection point to the line's origin, or infinity vector if no hit.
        vec3 intersectLine(const line3& line, double tolerance = default_Tolerance) const { // Added tolerance
            double min_t = infinity;
             vec3 closestHit = {infinity, infinity, infinity};
            bool hitFound = false;

            vec3 rayOrigin = line.pnt1;
            vec3 rayDir = line.direction().normalize(); // Assumes normalized handles internal tolerance
             // Assumes vec3::isZero handles internal tolerance
             if (rayDir.isZero()) return vec3{infinity, infinity, infinity}; // Degenerate line direction

            vec3 triP0, triP1, triP2;
            for (size_t i = 0; i < numTriangles(); ++i) {
                if (getTriangle(i, triP0, triP1, triP2)) {
                    double t, u, v;
                    // Pass tolerance to intersection test
                    if (rayTriangleIntersectMT(rayOrigin, rayDir, triP0, triP1, triP2, t, u, v, tolerance)) {
                         // t is already checked > -tolerance by rayTriangleIntersectMT
                         if (t < min_t) {
                             min_t = t;
                             closestHit = rayOrigin + rayDir * t;
                             hitFound = true;
                         }
                    }
                }
            }
             return hitFound ? closestHit : vec3{infinity, infinity, infinity};
        }

        // --- Intersection for UV ---
        // Structure to hold detailed intersection results
        struct IntersectionUVInfo {
            bool hit = false;
            vec3 point = {infinity, infinity, infinity};
            size_t triangleIndex = static_cast<size_t>(-1); // Index of the hit triangle
            double u = infinity; // Barycentric coordinate U (weight of triP1 in MT call)
            double v = infinity; // Barycentric coordinate V (weight of triP2 in MT call)
            double rayDist = infinity; // Distance along the ray/segment
        };

        // Intersects an infinite line ray with the strip, returning detailed info for UV mapping.
        IntersectionUVInfo intersectStripForUV(const line3& rayLine, double tolerance = default_Tolerance) const { // Added tolerance
            IntersectionUVInfo closestHit; // Defaults to no hit
            closestHit.rayDist = infinity;

            vec3 rayOrigin = rayLine.pnt1;
            vec3 rayDir = rayLine.direction().normalize(); // Assumes normalized handles tolerance
            // Assumes vec3::isZero handles tolerance
            if (rayDir.isZero()) return closestHit;

            vec3 triP0, triP1, triP2;
            for (size_t i = 0; i < numTriangles(); ++i) {
                if (getTriangle(i, triP0, triP1, triP2)) {
                    double t, u, v;
                    // Pass tolerance to intersection test
                    if (rayTriangleIntersectMT(rayOrigin, rayDir, triP0, triP1, triP2, t, u, v, tolerance)) {
                        // t is already checked > -tolerance
                        if (t < closestHit.rayDist) {
                            closestHit.hit = true;
                            closestHit.rayDist = t;
                            closestHit.point = rayOrigin + rayDir * t;
                            closestHit.triangleIndex = i;
                            closestHit.u = u; // Store barycentric U
                            closestHit.v = v; // Store barycentric V
                        }
                    }
                }
            }
            return closestHit;
        }

        // Intersects a line segment with the strip, returning detailed info for UV mapping.
        IntersectionUVInfo intersectSegmentForUV(const line3& segment, double tolerance = default_Tolerance) const { // Added tolerance
            IntersectionUVInfo closestHit; // Defaults to no hit
            closestHit.rayDist = infinity;

            vec3 rayOrigin = segment.pnt1;
            vec3 rayDir = segment.direction();
            double segmentLen = rayDir.length();
            // Check for degenerate segment using tolerance
            if (segmentLen < tolerance) return closestHit;

            // Denominator segmentLen guaranteed > tolerance, standard division safe.
            rayDir /= segmentLen; // Normalize

            vec3 triP0, triP1, triP2;
            for (size_t i = 0; i < numTriangles(); ++i) {
                if (getTriangle(i, triP0, triP1, triP2)) {
                    double t, u, v;
                    // Pass tolerance to intersection test
                    if (rayTriangleIntersectMT(rayOrigin, rayDir, triP0, triP1, triP2, t, u, v, tolerance)) {
                        // Check if hit is within segment bounds (using tolerance) and closer
                        if (t >= -tolerance && t <= segmentLen + tolerance && t < closestHit.rayDist) {
                            closestHit.hit = true;
                            closestHit.rayDist = t;
                            closestHit.point = rayOrigin + rayDir * t;
                            closestHit.triangleIndex = i;
                            closestHit.u = u; // Store barycentric U
                            closestHit.v = v; // Store barycentric V
                        }
                    }
                }
            }
            return closestHit;
        }

        vec3 normal(int triangleIndex) const {
            if (cache_normalsDirty) {
                precomputeNormals();
            }
            if (triangleIndex < cache_normals.size()) {
                return cache_normals[triangleIndex];
            }
            return vec3{infinity, infinity, infinity};
        }

        // --- AABB Overlap Methods --- (Exact comparison is fine for AABB)
        bool overlapsAABB(const line3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }
        bool overlapsAABB(const pLine3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }
        bool overlapsAABB(const QuadPlane3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }
        bool overlapsAABB(const TriangleStrip3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
         }

        // --- Transformations (Modify in-place) --- (Assume vec3 methods are robust)
        TriangleStrip3& translate(const vec3& offset) {
            for (vec3& p : points) { p += offset; }
            cache_railLengthsDirty = true;
            cache_normalsDirty = true;
            return *this;
        }
        TriangleStrip3& rotate(const vec3& pivot, const vec3& axis, double rads) {
            for (vec3& p : points) { p.orbit(pivot, axis, rads); }
             cache_railLengthsDirty = true;
             cache_normalsDirty = true;
             return *this;
        }
        TriangleStrip3& rotateCenter(const vec3& axis, double rads) {
            vec3 center = centerPoint();
            rotate(center, axis, rads);
            // rotate already sets dirty flag
            return *this;
        }
        TriangleStrip3& orbit(const vec3& orbitCenter, const vec3& axis, double rads) {
            vec3 center = centerPoint();
            vec3 translation = center.orbited(orbitCenter, axis, rads) - center;
            translate(translation);
            // translate already sets dirty flag
            return *this;
        }
        TriangleStrip3& scale(const vec3& pivot, double factor) {
             for (vec3& p : points) { p = pivot + (p - pivot) * factor; }
             cache_railLengthsDirty = true;
             cache_normalsDirty = true;
             return *this;
        }
        TriangleStrip3& scaleCenter(double factor) {
            vec3 center = centerPoint();
            scale(center, factor);
            // scale already sets dirty flag
            return *this;
        }

        // --- Transformations (Return new object) --- (Assume vec3 methods are robust)
        TriangleStrip3 translated(const vec3& offset) const {
            TriangleStrip3 mod = *this;
            mod.translate(offset);
            // Dirty flag is set by mod.translate
            return mod;
        }
        TriangleStrip3 rotated(const vec3& pivot, const vec3& axis, double rads) const {
             TriangleStrip3 mod = *this;
             mod.rotate(pivot, axis, rads);
             // Dirty flag is set by mod.rotate
             return mod;
        }
        TriangleStrip3 rotatedCenter(const vec3& axis, double rads) const {
            TriangleStrip3 mod = *this;
            mod.rotateCenter(axis, rads);
            // Dirty flag is set by mod.rotateCenter
            return mod;
        }
        TriangleStrip3 orbited(const vec3& orbitCenter, const vec3& axis, double rads) const {
             TriangleStrip3 mod = *this;
             mod.orbit(orbitCenter, axis, rads);
             // Dirty flag is set by mod.orbit
             return mod;
        }
        TriangleStrip3 scaled(const vec3& pivot, double factor) const {
             TriangleStrip3 mod = *this;
             mod.scale(pivot, factor);
             // Dirty flag is set by mod.scale
             return mod;
        }
        TriangleStrip3 scaledCenter(double factor) const {
            TriangleStrip3 mod = *this;
            mod.scaleCenter(factor);
            // Dirty flag is set by mod.scaleCenter
            return mod;
        }

        // --- UV Calculation Methods ---

        // Calculates spherical UV coordinates for a point.
        vec2 calculateSphereUV(vec3 pointOnSphere) const {
            pointOnSphere.normalize();
            // Assumes vec3::isZero handles tolerance
            if (pointOnSphere.isZero()) {
                 return vec2{infinity, infinity}; // Return infinite vec2
            }
            double x = pointOnSphere.x;
            double y = pointOnSphere.y;
            double z = pointOnSphere.z;
            double phi = std::atan2(z, x); // atan2 handles signs correctly
            double theta = std::acos(std::clamp(y, -1.0, 1.0)); // Clamp needed for acos domain
            // Denominators are constants, standard division is safe.
            double u = (phi + pi) / (2.0 * pi);
            double v = theta / pi;
            return vec2(u, v);
        }


        // Calculates planar UV coordinates stretched to the AABB.
        vec2 calculatePlanarUV_Stretched(const vec3& pointOnPlane, double tolerance = default_Tolerance) const { // Added tolerance
            if (points.size() < 3) {
                 return vec2{infinity, infinity};
            }
            auto [minPt, maxPt] = boundingBox();
            vec3 dims = maxPt - minPt;
            // Check for near-zero dimensions using tolerance
            if (std::abs(dims.x) < tolerance || std::abs(dims.y) < tolerance) {
                 return vec2{infinity, infinity};
            }
            vec3 relativePos = pointOnPlane - minPt;
            // Denominators dims.x/y guaranteed > tolerance, standard division safe.
            double u = relativePos.x / dims.x;
            double v = relativePos.y / dims.y;
            u = std::clamp(u, 0.0, 1.0);
            v = std::clamp(v, 0.0, 1.0);
            return vec2(u, v);
        }


        // Calculates planar UV coordinates using an orthonormal basis (world units).
        vec2 calculatePlanarUV_Unstretched(const vec3& pointOnPlane, double tolerance = default_Tolerance) const { // Added tolerance
            if (points.size() < 3) {
                 return vec2{infinity, infinity};
            }
            const vec3& origin = points[0];
            vec3 uAxis = points[1] - origin;
            vec3 vAxisTmp = points[2] - origin; // Used to find normal
            vec3 nNorm = uAxis.cross(vAxisTmp).normalize();
            double uLenSq = uAxis.lengthSquared();

            // Check for degenerate basis using tolerance squared
            if (uLenSq < tolerance * tolerance || nNorm.isZero()) {
                 return vec2{infinity, infinity};
            }
            // Denominator sqrt(uLenSq) guaranteed > tolerance, standard division safe.
            vec3 uNorm = uAxis / safe_sqrt(uLenSq);
            vec3 vNorm = nNorm.cross(uNorm).normalize(); // Assumes normalized handles zero vector case

             // Check if vNorm became zero (uAxis and nNorm were parallel)
             if (vNorm.isZero()) {
                  return vec2{infinity, infinity};
             }

            vec3 vec = pointOnPlane - origin;
            double uCoord = vec.dot(uNorm);
            double vCoord = vec.dot(vNorm);
            return vec2(uCoord, vCoord);
        }


        // Calculates surface UV coordinates using arc-length parameterization.
        vec2 calculateSurfaceUV(size_t triangleIndex, double barycentricU, double barycentricV, double tolerance = default_Tolerance) const { // Added tolerance
            if (cache_railLengthsDirty) {
                precomputeRailLengths(); // Ensure lengths are up-to-date
            }
            // Check if total rail length is near-zero
            if (cache_totalRailLength < tolerance) {
                return vec2{infinity, infinity};
            }
            // Check if the triangle index allows access to required points
            if (triangleIndex + 2 >= points.size()) {
                 return vec2{infinity, infinity};
            }

            // Calculate U coordinate (width)
            double U = (triangleIndex % 2 == 0)
                       ? barycentricU // Weight of p_{k+1} for even k
                       : (1.0 - barycentricU - barycentricV); // Weight of p_{k+1} for odd k

            // Calculate V coordinate (length)
            double distanceToSegmentStart = getDistanceAlongRail(triangleIndex);
            vec3 p_k = points[triangleIndex];
            vec3 p_k_plus_2 = points[triangleIndex + 2];
            double segmentRailLength = (p_k_plus_2 - p_k).length();

            // Interpolated distance along this rail segment.
            double v_dist_on_segment = barycentricV * segmentRailLength;
            // Handle near-zero length segment case
            if (segmentRailLength < tolerance) {
                 v_dist_on_segment = 0.0;
            }

            // Calculate final V coordinate
            // Denominator cache_totalRailLength guaranteed > tolerance, standard division safe.
            double V = (distanceToSegmentStart + v_dist_on_segment) / cache_totalRailLength;

            // Clamp results and return
            return vec2(std::clamp(U, 0.0, 1.0), std::clamp(V, 0.0, 1.0));
        }


    }; // End struct TriangleStrip3

} // End namespace vecSys
