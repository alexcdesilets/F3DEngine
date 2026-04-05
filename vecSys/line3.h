#pragma once

#include "base.h"
#include "vec2.h"
#include "vec3.h"
#include "quadPLane3.h"
#include "triangleStrip3.h"

// Removed includes: vec.h, typeHelp.h, concepts, cstdlib

namespace vecSys {
    struct line3 {
        struct collisionInfo {bool collisionFound = false; vec3 point = {infinity, infinity, infinity}, normal = {infinity, infinity, infinity}; vec2 UV = {infinity, infinity}; double rayDistanceSquared = infinity; int parentIndex = -1;};

        // Data Member: Uses double-specialized vec3
        vec3 pnt1, pnt2;

        // Calculate point on the infinite line for a given X-coordinate
        vec3 atX(const const double xVal, const double tolerance = default_Tolerance) const {
            vec3 dir = direction();
            // Check if line is parallel to the YZ plane (dir.x is near zero)
            if (std::abs(dir.x) < tolerance) {
                // If parallel, check if the line's x-coordinate matches xVal (within tolerance)
                return (std::abs(pnt1.x - xVal) < tolerance)
                        ? pnt1 // Line lies on the plane x=xVal, return start point as one possibility
                        : vec3{infinity, infinity, infinity}; // Line is parallel and offset, no point has this x-coordinate
            }
            // Denominator dir.x guaranteed > tolerance, standard division safe.
            double t = (xVal - pnt1.x) / dir.x;
            return pnt1 + dir * t;
        }

        // Calculate point on the infinite line for a given Y-coordinate
        vec3 atY(const const double yVal, const double tolerance = default_Tolerance) const {
            vec3 dir = direction();
            // Check if line is parallel to the XZ plane (dir.y is near zero)
            if (std::abs(dir.y) < tolerance) {
                // If parallel, check if the line's y-coordinate matches yVal (within tolerance)
                return (std::abs(pnt1.y - yVal) < tolerance)
                        ? pnt1 // Line lies on the plane y=yVal, return start point
                        : vec3{infinity, infinity, infinity}; // Line is parallel and offset
            }
            // Denominator dir.y guaranteed > tolerance, standard division safe.
            double t = (yVal - pnt1.y) / dir.y;
            return pnt1 + dir * t;
        }

        // Calculate point on the infinite line for a given Z-coordinate
        vec3 atZ(const const double zVal, const double tolerance = default_Tolerance) const {
            vec3 dir = direction();
            // Check if line is parallel to the XY plane (dir.z is near zero)
            if (std::abs(dir.z) < tolerance) {
                // If parallel, check if the line's z-coordinate matches zVal (within tolerance)
                return (std::abs(pnt1.z - zVal) < tolerance)
                        ? pnt1 // Line lies on the plane z=zVal, return start point
                        : vec3{infinity, infinity, infinity}; // Line is parallel and offset
            }
            // Denominator dir.z guaranteed > tolerance, standard division safe.
            double t = (zVal - pnt1.z) / dir.z;
            return pnt1 + dir * t;
        }

        std::pair<vec3, vec3> boundingBox() const {
            vec3 minPt = pnt1.minComp(pnt2);
            vec3 maxPt = pnt1.maxComp(pnt2);
            return {minPt, maxPt};
        }

        vec3 centerPoint() const {
            return (pnt1 + pnt2) / 2.0; // Use double literal
        }

        vec3 closestPointOnLine(const vec3& point, const double tolerance = default_Tolerance) const {
            vec3 lineDir = direction();
            double lineLenSq = lineDir.lengthSquared();

            // Check if line has near-zero length using tolerance squared
            if (lineLenSq < tolerance * tolerance) {
                return pnt1; // Return start point if line has effectively no length
            }

            vec3 pointVec = point - pnt1;
            // Division creates scalar factor, denominator lineLenSq guaranteed > tolerance^2, standard division safe.
            double factor = pointVec.dot(lineDir) / lineLenSq;
            return pnt1 + lineDir * factor;
        }

        vec3 closestPointOnSegment(const vec3& point, const double tolerance = default_Tolerance) const {
            vec3 lineDir = direction();
            double lenSq = lineDir.lengthSquared();

            // Check for near-zero length segment
            if (lenSq < tolerance * tolerance) {
                return pnt1;
            }

            // Calculate projection parameter t
            // Denominator lenSq guaranteed > tolerance^2, standard division safe.
            double t = (point - pnt1).dot(lineDir) / lenSq;
            // Clamp t to [0, 1] to stay within the segment
            t = std::clamp(t, 0.0, 1.0);

            return pnt1 + lineDir * t;
        }

        // Calculates closest points between two line segments.
        std::pair<vec3, vec3> closestPointsOnSegments(const line3& other, const double tolerance = default_Tolerance) const {
            vec3 u = pnt2 - pnt1;
            vec3 v = other.pnt2 - other.pnt1;
            vec3 w = pnt1 - other.pnt1;
            double a = u.dot(u); // length squared of segment 1
            double b = u.dot(v);
            double c = v.dot(v); // length squared of segment 2
            double d = u.dot(w);
            double e = v.dot(w);
            double D = a * c - b * b; // Denominator (always >= 0)

            double sc, sN, sD = D; // sc = sN / sD : parameter on segment 1
            double tc, tN, tD = D; // tc = tN / tD : parameter on segment 2

            // Compute the line parameters of the two closest points
            if (D < tolerance) { // Check if lines are almost parallel
                sN = 0.0;        // Force using endpoint P0 on segment 1
                sD = 1.0;
                tN = e;
                tD = c;
            } else {             // Get the closest points on the infinite lines
                sN = (b * e - c * d);
                tN = (a * e - b * d);
                // Clamp sN to [0, sD] (which corresponds to clamping sc to [0, 1])
                if (sN < 0.0) {  // sc < 0 => the s=0 edge is visible
                    sN = 0.0;
                    tN = e;
                    tD = c;
                } else if (sN > sD) { // sc > 1 => the s=1 edge is visible
                    sN = sD;
                    tN = e + b;
                    tD = c;
                }
            }

            // Clamp tN to [0, tD] (which corresponds to clamping tc to [0, 1])
            if (tN < 0.0) {      // tc < 0 => the t=0 edge is visible
                tN = 0.0;
                // Recalculate sN for this edge case
                if (-d < 0.0)
                    sN = 0.0;
                else if (-d > a)
                    sN = sD;
                else {
                    sN = -d;
                    sD = a;
                }
            } else if (tN > tD) { // tc > 1 => the t=1 edge is visible
                tN = tD;
                // Recalculate sN for this edge case
                if ((-d + b) < 0.0)
                    sN = 0;
                else if ((-d + b) > a)
                    sN = sD;
                else {
                    sN = (-d + b);
                    sD = a;
                }
            }

            // Calculate final parameters, dividing only if denominator is not near zero
            // Standard division safe here as D is checked against tolerance earlier for the parallel case.
            // For non-parallel case, D > tolerance. For parallel case, sD=1.0 and tD=c (lengthSq).
            // Need to check tD=c for near-zero length of the 'other' segment if parallel.
            sc = (std::abs(sN) < tolerance ? 0.0 : sN / sD);
            // Check tD before dividing
            tc = (std::abs(tN) < tolerance || std::abs(tD) < tolerance ? 0.0 : tN / tD);

            // Get the closest points on the segments
            vec3 pointOnThis = pnt1 + u * sc;
            vec3 pointOnOther = other.pnt1 + v * tc;

            return {pointOnThis, pointOnOther};
        }


        vec3 direction() const { return pnt2 - pnt1; }

        // Uses closestPointsOnSegments, which now takes tolerance
        double distanceToSegment(const line3& other, const double tolerance = default_Tolerance) const {
            auto [pt1, pt2] = closestPointsOnSegments(other, tolerance);
            return (pt1 - pt2).length();
        }

        double distanceToPoint(const vec3& point, const double tolerance = default_Tolerance) const {
            vec3 lineDir = pnt2 - pnt1;
            double len = lineDir.length();
            // Check for near-zero length
            if (len < tolerance) {
                return (point - pnt1).length(); // Line is effectively a point
            }
            vec3 p1ToPoint = point - pnt1;
            // Perpendicular distance = |Cross product length| / |Base length|
            // Denominator len guaranteed > tolerance, standard division safe.
            return (lineDir.cross(p1ToPoint)).length() / len;
        }

        // Distance from point to this line SEGMENT
        double distanceToPointSegment(const vec3& point, const double tolerance = default_Tolerance) const {
            vec3 lineDir = direction();
            double lineLenSq = lineDir.lengthSquared();

                // Handle near-zero length segment case
                if (lineLenSq < tolerance * tolerance) {
                    return (point - pnt1).length();
                }

            vec3 p0_to_point = point - pnt1;
            vec3 p1_to_point = point - pnt2;

            // Project point onto the line, get parameter t
            // Denominator lineLenSq guaranteed > tolerance^2, standard division safe.
            double t = p0_to_point.dot(lineDir) / lineLenSq;

            // Check parameter t against segment bounds [0, 1]
            if (t < 0.0) { // Closest point is pnt1
                return p0_to_point.length();
            }
            if (t > 1.0) { // Closest point is pnt2
                return p1_to_point.length();
            }
            // Closest point is the projection onto the segment
            // Use distance to infinite line (perpendicular distance)
            return distanceToPoint(point, tolerance);
        }

        // Uses closestPointsOnSegments, which now takes tolerance
        bool fastIntersectsSegment(const line3& other, const double tolerance = default_Tolerance) const {
            auto [pt1, pt2] = closestPointsOnSegments(other, tolerance);
            double distSq = (pt1 - pt2).lengthSquared();
            return distSq < tolerance * tolerance;
        }

        collisionInfo findCollision(const QuadPlane3& plane, bool infiniteLine = false, bool mapUV = true, bool uv_stretch = false, const double tolerance = default_Tolerance) {
            collisionInfo info;
            // Is the intersection even possible
            if (!plane.overlapsAABB(*this) && !infiniteLine) return info;

            info.point = infiniteLine ? plane.intersectLine(*this, tolerance): plane.intersectSegment(*this, tolerance);

            if (info.point.isInfinite()) return info;

            info.rayDistanceSquared = (info.point - pnt1).lengthSquared();
            info.collisionFound = true;
            info.normal = plane.normal();

            if (!mapUV) return info;

            info.UV = uv_stretch ? plane.getPlanarCoordsScaled(info.point, tolerance): plane.getPlanarCoordsOrtho(info.point, tolerance);

            return info;
        }

        collisionInfo findCollision(const TriangleStrip3& strip, bool infiniteLine = false, bool mapUV = true, bool flat = true, bool complexShape = false, bool uv_stretch = false, const double tolerance = default_Tolerance) const {
            collisionInfo info;
            // AABB Check
            if (!strip.overlapsAABB(*this) && !infiniteLine) { // Assuming strip has overlapsAABB(line3)
                return info; // No collision found
            }

            TriangleStrip3::IntersectionUVInfo intersectionResult; // Use the detailed struct

            // Perform intersection test (line or segment vs strip)
            if (infiniteLine) {
                intersectionResult = strip.intersectStripForUV(*this, tolerance);
            } else {
                intersectionResult = strip.intersectSegmentForUV(*this, tolerance);
            }

            // Check if a hit occurred
            if (!intersectionResult.hit) {
                return info; // No collision found
            }

            // Collision found
            info.collisionFound = true;
            info.point = intersectionResult.point;
            info.rayDistanceSquared = (info.point - pnt1).lengthSquared();
            info.normal = strip.normal(intersectionResult.triangleIndex);

            // Calculate UV if requested
            if (mapUV) {
                if (!flat && complexShape) { // Treat as complex 3D surface (use arc-length parameterization)
                    // Pass tolerance to calculateSurfaceUV
                    info.UV = strip.calculateSurfaceUV(intersectionResult.triangleIndex, intersectionResult.u, intersectionResult.v, tolerance);
                } else { // Treat as flat plane
                    if (flat) {
                        if (uv_stretch) {
                            // Pass tolerance to calculatePlanarUV_Stretched
                            info.UV = strip.calculatePlanarUV_Stretched(info.point, tolerance);
                        } else {
                            // Pass tolerance to calculatePlanarUV_Unstretched
                            info.UV = strip.calculatePlanarUV_Unstretched(info.point, tolerance);
                        }
                    }
                    else {
                        info.UV = strip.calculateSphereUV(info.point);
                    }
                }
            } else {
                 info.UV = vec2{infinity, infinity}; // Set UV to infinite if not mapped
            }

            return info;
        }

        std::vector<collisionInfo> findMultipleCollisions(const std::vector<QuadPlane3>& planes, bool infiniteLine = false, bool mapUV = true, bool uv_stretch = false, const double tolerance = default_Tolerance) {
            std::vector<collisionInfo> collisions;
            collisions.reserve(planes.size());
            for (size_t i = 0; i < planes.size(); ++i) {
                collisionInfo col = findCollision(planes[i], infiniteLine, mapUV, uv_stretch, tolerance);
                col.parentIndex = i;
                if (col.collisionFound) collisions.push_back(col);
            }
            collisions.shrink_to_fit();
            if (!collisions.empty()) {
                std::sort(collisions.begin(), collisions.end(), [](const collisionInfo& a, const collisionInfo& b) {
                    return a.rayDistanceSquared < b.rayDistanceSquared;
                });
            }
            return collisions;
        }

        std::vector<collisionInfo> findMultipleCollisions(const std::vector<TriangleStrip3>& strips, bool infiniteLine = false, bool mapUV = true, bool flat = true, bool complexShape = false, bool uv_stretch = false, const double tolerance = default_Tolerance) {
            std::vector<collisionInfo> collisions;
            collisions.reserve(strips.size());
            for (size_t i = 0; i < strips.size(); ++i) {
                collisionInfo col = findCollision(strips[i], infiniteLine, mapUV, flat, complexShape, uv_stretch, tolerance);
                col.parentIndex = i;
                if (col.collisionFound) collisions.push_back(col);
            }
            collisions.shrink_to_fit();
            if (!collisions.empty()) {
                std::sort(collisions.begin(), collisions.end(), [](const collisionInfo& a, const collisionInfo& b) {
                    return a.rayDistanceSquared < b.rayDistanceSquared;
                });
            }
            return collisions;
        }

        // Uses distanceToSegment, which now takes tolerance
        bool intersectsSegment(const line3& other, const double tolerance = default_Tolerance) const {
            return distanceToSegment(other, tolerance) < tolerance;
        }

        // Ray-Segment intersection check
        bool rayIntersectsSegment(const line3& segment, vec3& outHitPoint, double& outRayDistance, const double tolerance = default_Tolerance ) const {
            vec3 v = (pnt2 - pnt1).normalize(); // This is the ray direction
            vec3 u = segment.pnt2 - segment.pnt1; // Segment direction
            vec3 w = segment.pnt1 - pnt1; // std::Vector from ray origin to segment start

            double a = u.dot(u);         // segment length squared
            double b = u.dot(v);
            double c = v.dot(v);          // Should be 1.0 if normalized
            double d = u.dot(w);
            double e = v.dot(w);
            double D = a * c - b * b;     // Always non-negative

            double sc, tc; // sc: parameter on segment, tc: parameter on ray

            // Check if lines are almost parallel
            if (D < tolerance) {
                // Lines are parallel, check if collinear and overlapping
                // Simplified check: Assume no intersection for parallel case in this context
                return false; // Treat parallel as non-intersecting for simplicity here
            }

            // Calculate parameters for closest points on infinite lines
            // Denominator D guaranteed > tolerance, standard division safe.
            sc = (b * e - c * d) / D;
            tc = (a * e - b * d) / D;

            // Check if intersection point is within segment bounds [0, 1]
            // and if ray parameter is non-negative (ray goes forward)
            // Use tolerance for segment bounds check
            if (sc >= -tolerance && sc <= 1.0 + tolerance && tc >= -tolerance) { // Allow slightly negative ray dist too
                vec3 pointOnSegment = segment.pnt1 + u * sc;
                vec3 pointOnRay = pnt1 + v * tc;

                // Check if the closest points are coincident (within tolerance)
                if ((pointOnSegment - pointOnRay).lengthSquared() < tolerance * tolerance) {
                    outHitPoint = pointOnSegment; // Or average: (pointOnSegment + pointOnRay) * 0.5;
                    outRayDistance = tc;
                    return true;
                }
            }

            return false; // No intersection within bounds
        }


        double length() const { return direction().length(); }
        double lengthSquared() const { return direction().lengthSquared(); }

        // Relative std::vector calculations (methods renamed for clarity)
        vec3 fromStart(const vec3& point) const { return point - pnt1; }
        vec3 fromEnd(const vec3& point) const { return point - pnt2; }
        vec3 toStart(const vec3& point) const { return pnt1 - point; }
        vec3 toEnd(const vec3& point) const { return pnt2 - point; }

        // Orbit line segment around an external point
        line3& orbit(const vec3& orbitPoint, const vec3& axis, const const double rads) {
            vec3 center = (pnt1 + pnt2) / 2.0;
            vec3 change = center.orbited(orbitPoint, axis, rads) - center;
            pnt1 += change;
            pnt2 += change;
            return *this;
        }

        line3 orbited(const vec3& orbitPoint, const vec3& axis, const const double rads) const { // Made const
            line3 mod = *this;
            mod.orbit(orbitPoint, axis, rads);
            return mod;
        }

        double parameterAlongLine(const vec3& point, const double tolerance = default_Tolerance) const {
            vec3 lineDir = direction();
            double lenSq = lineDir.lengthSquared();
            // Check for near-zero length
            if (lenSq < tolerance * tolerance) return 0.0;

            // Denominator lenSq guaranteed > tolerance^2, standard division safe.
            return (point - pnt1).dot(lineDir) / lenSq;
        }

        // Get point along the line segment via linear interpolation
        vec3 pointAt(const const double percent) const { // Take percent by const ref
            // Clamp interpolation factor to [0, 1]
            double t = std::clamp(percent, 0.0, 1.0);
            // Assuming vec3::lerped exists
            return pnt1.lerped(pnt2, t);
        }

        // Check if a point lies on the infinite line defined by the segment
        bool pointOnLine(const vec3& point, const double tolerance = default_Tolerance) const {
            vec3 lineDir = pnt2 - pnt1;
            vec3 pointVec = point - pnt1;
            // Check if cross product length squared is near zero
            return (lineDir.cross(pointVec)).lengthSquared() < tolerance * tolerance;
        }

        // Check if a point lies on the line segment itself
        bool pointOnSegment(const vec3& point, const double tolerance = default_Tolerance) const {
            // Check if point is collinear first (efficient check)
            if (!pointOnLine(point, tolerance)) {
                return false;
            }

            // Check if point lies within the projection range [0, 1] on the line
            vec3 lineDir = direction();
            double lenSq = lineDir.lengthSquared();

            // Handle near-zero length segment: point must be close to pnt1
            if (lenSq < tolerance * tolerance) {
                    return (point - pnt1).lengthSquared() < tolerance * tolerance;
            }

            // Calculate parameter t for the projection
            // Denominator lenSq guaranteed > tolerance^2, standard division safe.
            double t = (point - pnt1).dot(lineDir) / lenSq;

            // Check if t is within [0, 1] with tolerance
            // A point slightly outside the segment due to FP error might still be considered "on"
            return (t >= -tolerance && t <= 1.0 + tolerance);
        }


        // Project an external point onto the line segment
        vec3 projectOntoSegment(const vec3& point, const double tolerance = default_Tolerance) const {
            vec3 lineDir = direction();
            double lineLenSq = lineDir.lengthSquared();

            // Handle near-zero length segment
            if (lineLenSq < tolerance * tolerance) {
                return pnt1;
            }

            vec3 pointVec = point - pnt1;
            // Calculate projection factor t
            // Denominator lineLenSq guaranteed > tolerance^2, standard division safe.
            double t = pointVec.dot(lineDir) / lineLenSq;

            // Clamp factor t to [0, 1] to stay within the segment
            t = std::clamp(t, 0.0, 1.0);

            // Calculate the projected point on the segment
            return pnt1 + lineDir * t;
        }

        vec3 reflectionDirection(const vec3& normal) const {
            return direction().reflect(normal);
        }

        line3& reflectAcrossPlane(const vec3& planePoint, vec3 planeNormal) {
            planeNormal.normalize(); // Ensure normal is unit length

            // Reflect both endpoints across the plane
            pnt1 = pnt1 - planeNormal * (2.0 * (pnt1 - planePoint).dot(planeNormal));
            pnt2 = pnt2 - planeNormal * (2.0 * (pnt2 - planePoint).dot(planeNormal));

            return *this;
        }

        line3 reflectedAcrossPlane(const vec3& planePoint, const vec3& planeNormal) const { // Made const
            line3 mod = *this;
            return mod.reflectAcrossPlane(planePoint, planeNormal);
        }

        // Rotate line segment around its center point
        line3& rotate(const vec3& axis, const const double rads, const double tolerance = default_Tolerance) {
            // Check for near-zero length before rotating
            if (lengthSquared() < tolerance * tolerance) return *this;
            vec3 cent = centerPoint();
            pnt1.orbit(cent, axis, rads);
            pnt2.orbit(cent, axis, rads);
            return *this;
        }

        line3 rotated(const vec3& axis, const const double rads, const double tolerance = default_Tolerance) const { // Made const
            line3 mod = *this;
            // Pass tolerance to the in-place rotate
            mod.rotate(axis, rads, tolerance);
            return mod;
        }

        std::pair<vec3, vec3> tangentPlane(const const double percent) const {
            vec3 point = pointAt(percent);
            vec3 dir = direction().normalize();
            // Arbitrary std::vector not parallel to dir
            // Robust selection of arbitrary std::vector: find smallest component of dir
            vec3 arbitrary;
            if (std::abs(dir.x) <= std::abs(dir.y) && std::abs(dir.x) <= std::abs(dir.z)) {
                arbitrary = {1.0, 0.0, 0.0}; // Use X-axis
            } else if (std::abs(dir.y) <= std::abs(dir.x) && std::abs(dir.y) <= std::abs(dir.z)) {
                arbitrary = {0.0, 1.0, 0.0}; // Use Y-axis
            } else {
                arbitrary = {0.0, 0.0, 1.0}; // Use Z-axis
            }

            // Cross to get first axis (binormal)
            vec3 axis1 = dir.cross(arbitrary).normalize();
            // Cross again to get second axis (normal)
            vec3 axis2 = dir.cross(axis1).normalize(); // Note: order matters B x T = N or T x B = -N
            // Ensure axis2 is orthogonal to dir if axis1 was zero (shouldn't happen if dir != 0)
            if (axis1.isZero()) { // Fallback if dir was parallel to arbitrary
                    // Find another arbitrary std::vector... This case is unlikely with the robust selection.
                    arbitrary = (std::abs(dir.y) > 0.9) ? vec3{0,0,1} : vec3{0,1,0}; // Try another axis
                    axis1 = dir.cross(arbitrary).normalize();
                    axis2 = dir.cross(axis1).normalize();
            }

            // axis1 and axis2 form the plane at 'point'
            return {axis1, axis2};
        }


        // Uses distanceToSegment, which now takes tolerance
        bool thickRayHitsSegment(const line3& segment, double thickness, vec3& outHitPoint, const double tolerance = default_Tolerance) const {
            auto [ptOnRay, ptOnSegment] = closestPointsOnSegments(segment, tolerance);
            double dist = (ptOnRay - ptOnSegment).length();
            if (dist > thickness) return false;

            // Use tolerance in closestPointsOnSegments
            outHitPoint = ptOnSegment; // or average both points for center point
            return true;
        }

        // Translate the line segment
        line3& translate(const vec3& offset) {
            pnt1 += offset;
            pnt2 += offset;
            return *this;
        }

        line3 translated(const vec3& offset) const { // Added const version
            line3 mod = *this;
            mod.translate(offset);
            return mod;
        }

        double xAtYZ(const const double yVal, const const double zVal) const {
            vec3 dir = direction();
            double denom = std::hypot(dir.y, dir.z);
            return std::hypot(yVal, zVal) * safe_divide(dir.x, denom) + pnt1.x;
        }
        
        vec2 xyAtZ(const const double zVal) const {
            vec3 dir = direction();
            return vec2{dir.x, dir.y} * safe_divide(zVal - pnt1.z, dir.z) + vec2{pnt1.x, pnt1.y};
        }
        
        vec2 xzAtY(const const double yVal) const {
            vec3 dir = direction();
            return vec2{dir.x, dir.z} * safe_divide(yVal - pnt1.y, dir.y) + vec2{pnt1.x, pnt1.z};
        }
        
        double yAtXZ(const const double xVal, const const double zVal) const {
            vec3 dir = direction();
            double denom = std::hypot(dir.x, dir.z);
            return std::hypot(xVal, zVal) * safe_divide(dir.y, denom) + pnt1.y;
        }
        
        vec2 yzAtX(const const double xVal) const {
            vec3 dir = direction();
            return vec2{dir.y, dir.z} * safe_divide(xVal - pnt1.x, dir.x) + vec2{pnt1.y, pnt1.z};
        }
        
        double zAtXY(const const double xVal, const const double yVal) const {
            vec3 dir = direction();
            double denom = std::hypot(dir.x, dir.y);
            return std::hypot(xVal, yVal) * safe_divide(dir.z, denom) + pnt1.z;
        }        

    }; // End struct line3
}; // End namespace vecSys