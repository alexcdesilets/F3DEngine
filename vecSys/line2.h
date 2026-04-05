#pragma once

#include <limits>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <numbers>

#include "vec2.h"

namespace vecSys {
    struct line2 {
        struct collisionInfo {
            vec2 collisionPoint = {infinity, infinity}, 
                 fromP1 = {infinity, infinity}, 
                 rayOriginToCollisionPoint = {infinity, infinity};
            int segmentIndex = -1;
            bool collisionFound = false;
            double rayLength = -1;
        };

        // Data Member
        vec2 pnt1, pnt2;

        // Static factory function
        static line2 rayProjection(const vec2& startPoint, double length, double radians) {
            // Assuming vec2::fromAngle exists and is static
            return {startPoint, startPoint + (vec2::fromAngle(radians) * length)};
        }

        // Member Functions
        double angle() const { return direction().angle(); }

        double angleBetween(const line2& other) const { return other.angle() - angle(); }

        vec2 centerPoint() const { return (pnt1 + pnt2) * 0.5; }

        vec2 closestPointOnLine(const vec2& point, const double tolerance = default_Tolerance) const {
            vec2 lineDir = direction();
            double lineLenSq = lineDir.lengthSquared();

            // Use double default_Tolerance for comparison
            if (lineLenSq < tolerance * tolerance) {
                return pnt1; // Return start point if line has effectively no length
            }

            vec2 pointVec = point - pnt1;
            
            double factor = pointVec.dot(lineDir) / lineLenSq;
            return pnt1 + lineDir * factor;
        }

        vec2 direction() const { return pnt2 - pnt1; }

        // Corrected: Distance from point P to infinite line defined by A, B
        double distanceToPoint(const vec2& point, const double tolerance = default_Tolerance) const {
            vec2 lineDir = direction();
            double len = lineDir.length();
            if (len < tolerance) { // Check if line has length
                 return (point - pnt1).length(); // Distance to the single point
            }
            vec2 p1ToPoint = point - pnt1;
            // Perpendicular distance = |Area of Parallelogram| / |Base Length|
            return std::abs(lineDir.cross(p1ToPoint)) / len;
        }

        // Distance between two line SEGMENTS
        double distanceToSegment(const line2& other) const {
            // Check for intersection pnt1 (within tolerance)
            if (!intersection(other).isInfinite()) {
                return 0.0;
            }

            // If no intersection, check distances from endpoints to the other segment
            double dist1 = distanceToPointSegment(other.pnt1);
            double dist2 = distanceToPointSegment(other.pnt2);
            double dist3 = other.distanceToPointSegment(pnt1);
            double dist4 = other.distanceToPointSegment(pnt2);

            // Return the minimum of these four distances
            // Use std::min with initializer list
            return std::min({dist1, dist2, dist3, dist4});
        }

        // Distance from point to this line SEGMENT
        double distanceToPointSegment(const vec2& point, const double tolerance = default_Tolerance) const {
            vec2 lineDir = direction();
            double lineLenSq = lineDir.lengthSquared();

             // Handle zero-length segment case
             if (lineLenSq < tolerance * tolerance) {
                  return (point - pnt1).length();
             }

            vec2 p0_to_point = point - pnt1;
            vec2 p1_to_point = point - pnt2;

            // Project point onto the line, get parameter t
            // Division okay here, checked lineLenSq
            double t = p0_to_point.dot(lineDir) / lineLenSq;

            // If projection before start point (t < 0), closest is pnt1
            if (t < 0.0) {
                return p0_to_point.length();
            }
            // If projection after end point (t > 1), closest is pnt2
            if (t > 1.0) {
                return p1_to_point.length();
            }
            // Otherwise, projection is on the segment, use perpendicular distance to infinite line
            // Since t is between 0 and 1, the closest point is on the segment, which is the projection
             vec2 projection = pnt1 + lineDir * t;
             return (point - projection).length();
             // Or equivalently, use the distanceToPoint method (perpendicular distance)
             // return distanceToPoint(point); // This would be simpler if logic above is sound
        }

        // Find Intersection MetaData
        collisionInfo findIntersection(const line2& other, const int segmentIndex) const {
            collisionInfo closestCollision; // Assumes double-specialized collisionInfo
            closestCollision.collisionFound = false;
            closestCollision.collisionPoint = {infinity, infinity};

            // Find intersection point of the infinite lines
            vec2 intersectPoint = intersection(other);

            if (!intersectPoint.isInfinite()) {
                closestCollision.collisionPoint = intersectPoint;
                closestCollision.collisionFound = true;
                closestCollision.segmentIndex = segmentIndex;
                closestCollision.fromP1 = intersectPoint - other.pnt1;
                closestCollision.rayOriginToCollisionPoint = intersectPoint - pnt1;
                closestCollision.rayLength = closestCollision.rayOriginToCollisionPoint.length();
            }
            
            return closestCollision;
        }

        // Find ALL intersections with a list of lines (segments), sorted by distance
        std::vector<collisionInfo> findMultipleIntersections(const std::vector<line2>& lines) const {
            std::vector<collisionInfo> foundCollisions;

            for (size_t i = 0; i < lines.size(); ++i) {
                const line2& otherLine = lines[i];
                vec2 intersectPoint = intersection(otherLine);

                if (!intersectPoint.isInfinite()) { // Check using vec2::isInfinite if available
                    collisionInfo colInfo; // Assumes double-specialized
                    colInfo.collisionFound = true;
                    colInfo.collisionPoint = intersectPoint;
                    colInfo.segmentIndex = static_cast<int>(i); // Cast size_t to int

                    // Populate remaining fields
                    colInfo.fromP1 = colInfo.collisionPoint - otherLine.pnt1;
                    colInfo.rayOriginToCollisionPoint = colInfo.collisionPoint - pnt1;
                    colInfo.rayLength = colInfo.rayOriginToCollisionPoint.length();

                    foundCollisions.push_back(colInfo);
                }
            }

            // Sort from furthest to nearest
            std::sort(foundCollisions.begin(), foundCollisions.end(), [](const collisionInfo& a, const collisionInfo& b) {return a.rayLength > b.rayLength;});

            return foundCollisions;
        }

        vec2 intersection(const line2& other, bool infiniteLine = false) const {
            if (!infiniteLine && !overlapsAABB(other)) return {infinity, infinity};
            // Do not reorder *this* ray
            line2 lineS = (other.pnt1.x <= other.pnt2.x) ? other : line2{other.pnt2, other.pnt1};
        
            vec2 dF = direction();       // Ray direction (this)
            vec2 dS = lineS.direction(); // Segment direction
        
            vec2 intPnt = { infinity, infinity };
        
            // General case: both lines are diagonal
            if (dF.x != 0.0 && dS.x != 0.0 && dF.y != 0.0 && dS.y != 0.0) {
                double mF = dF.y / dF.x;
                double mS = dS.y / dS.x;
        
                double mDelta = mF - mS;
                if (mDelta == 0.0) return { infinity, infinity }; // Parallel
        
                double bF = pnt1.y - mF * pnt1.x;
                double bS = lineS.pnt1.y - mS * lineS.pnt1.x;
        
                intPnt.x = safe_divide((bS - bF), mDelta);
                intPnt.y = mF * intPnt.x + bF;
            }
            // Special cases
            else if (dF.x == 0.0 && dS.x != 0.0) {
                intPnt.x = pnt1.x;
                double mS = dS.y / dS.x;
                double bS = lineS.pnt1.y - mS * lineS.pnt1.x;
                intPnt.y = mS * intPnt.x + bS;
            }
            else if (dS.x == 0.0 && dF.x != 0.0) {
                intPnt.x = lineS.pnt1.x;
                double mF = dF.y / dF.x;
                double bF = pnt1.y - mF * pnt1.x;
                intPnt.y = mF * intPnt.x + bF;
            }
            else if (dF.y == 0.0 && dS.y != 0.0) {
                intPnt.y = pnt1.y;
                double mS = dS.y / dS.x;
                intPnt.x = (intPnt.y - (lineS.pnt1.y - mS * lineS.pnt1.x)) / mS;
            }
            else if (dS.y == 0.0 && dF.y != 0.0) {
                intPnt.y = lineS.pnt1.y;
                double mF = dF.y / dF.x;
                intPnt.x = (intPnt.y - (pnt1.y - mF * pnt1.x)) / mF;
            }
        
            if (intPnt.isInfinite()) return { infinity, infinity };
            if (infiniteLine) return intPnt;
        
            // Final filtering: must lie on both segments
            if (!overlapsAABB(intPnt) || !other.overlapsAABB(intPnt)) return { infinity, infinity };
        
            return intPnt;
        }

        bool isParallel(const line2& other, const double tolerance = default_Tolerance) const {
            // Check if the cross product of direction vectors is close to zero
            return std::abs(direction().cross(other.direction())) < tolerance;
        }

        bool isCollinear(const line2& other, const double tolerance = default_Tolerance) const {
            // Check if parallel and if a point from the other line lies on this line
            return isParallel(other, tolerance) && pointOnLine(other.pnt1, tolerance);
        }

        double length() const { return direction().length(); }
        double lengthSquared() const { return direction().lengthSquared(); }

        // Relative vector calculations (methods renamed for clarity)
        vec2 FromStart(const vec2& point) const { return point - pnt1; }
        vec2 FromEnd(const vec2& point) const { return point - pnt2; }
        vec2 toStart(const vec2& point) const { return pnt1 - point; }
        vec2 toEnd(const vec2& point) const { return pnt2 - point; }

        vec2 normal() const {
            vec2 dir = direction().normalize();
            return {-dir.y, dir.x};
        }

        // Orbit line segment around an external point
        line2& orbit(const vec2& orbitCenter, const double radians) {
            vec2 center = (pnt1 + pnt2) / 2.0;
            vec2 change = center.orbited(orbitCenter, radians) - center;
            pnt1 += change;
            pnt2 += change;
            return *this;
        }

        line2 orbited(const vec2& point, const double radians) const { // Made const
            line2 mod = *this;
            mod.orbit(point, radians);
            return mod;
        }

        bool overlapsAABB(const vec2& other) const {
            auto bb = boundingBox();
            return bb.first.x <= other.x && other.x <= bb.second.x && bb.first.y <= other.y && other.y <= bb.second.y;
        }

        bool overlapsAABB(const line2& other) const {
            // Use vec2's component-wise min/max methods
            vec2 minA = pnt1.min(pnt2);
            vec2 maxA = pnt1.max(pnt2);
            vec2 minB = other.pnt1.min(other.pnt2);
            vec2 maxB = other.pnt1.max(other.pnt2);

            // Standard AABB overlap check
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y);
        }

        // Get point along the line segment via linear interpolation
        vec2 pointAt(const double percent) const { // Take percent by const ref
            // Clamp interpolation factor to [0, 1]
            double t = std::clamp(percent, 0.0, 1.0);
            // Assuming vec2::lerped exists
            return pnt1.lerped(pnt2, t);
        }

        // Check if a point lies on the infinite line defined by the segment
        bool pointOnLine(const vec2& point, const double tolerance = default_Tolerance) const {
            // Check if the area of the parallelogram formed by (P-A) and (B-A) is close to zero
            vec2 lineDir = direction();
            vec2 pointVec = point - pnt1;
            return std::abs(lineDir.cross(pointVec)) < tolerance;
        }

        // Check if a point lies on the line segment itself
        bool pointOnSegment(const vec2& point, const double tolerance = default_Tolerance) const {
            // Check if point is collinear pnt1 (efficient check)
            if (!pointOnLine(point, tolerance)) {
                return false;
            }
            // Then check if point is within the AABB of the segment endpoints
            vec2 minP = pnt1.min(pnt2);
            vec2 maxP = pnt1.max(pnt2);
            // Use tolerance in bounds check
            return (point.x >= minP.x - tolerance && point.x <= maxP.x + tolerance &&
                    point.y >= minP.y - tolerance && point.y <= maxP.y + tolerance);
        }

        // Project an external point onto the line segment
        vec2 projectOntoSegment(const vec2& point, const double tolerance = default_Tolerance) const {
            vec2 lineDir = direction();
            double lineLenSq = lineDir.lengthSquared();

            // Handle zero-length segment
            if (lineLenSq < tolerance * tolerance) {
                return pnt1;
            }

            vec2 pointVec = point - pnt1;
            // Calculate projection factor t
            // Division okay here, checked lineLenSq
            double t = pointVec.dot(lineDir) / lineLenSq;

            // Clamp factor t to [0, 1] to stay within the segment
            t = std::clamp(t, 0.0, 1.0);

            // Calculate the projected point on the segment
            return pnt1 + lineDir * t;
        }

        double reflectionAngle(const line2& other) const {
            vec2 dir = direction().normalize();
            vec2 surface_normal  = other.direction().perpendicularCCW().normalize();
            if (dir.dot(surface_normal) > 0.0) {
                surface_normal = surface_normal.opposite();
            }
            dir.reflect(surface_normal);
            return dir.angle();
        }

        // Rotate line segment around its center point
        line2& rotate(const double radians, const double tolerance = default_Tolerance) {
            if (lengthSquared() < tolerance * tolerance) return *this; // Don't rotate zero-length line
            vec2 cent = centerPoint();
            pnt1.orbit(cent, radians);
            pnt2.orbit(cent, radians);
            return *this;
        }

        line2 rotated(const double radians) const { // Made const
            line2 mod = *this;
            mod.rotate(radians);
            return mod;
        }

        // Determine which side of the line a point lies on
        double sideOfLine(const vec2& point) const {
            // Cross product (P - A) x (B - A)
            // Sign indicates the side: > 0 (e.g., left), < 0 (e.g., right), = 0 (on line)
            vec2 lineDir = direction();
            vec2 pointVec = point - pnt1;
            return lineDir.cross(pointVec);
        }

        std::pair<vec2, vec2> boundingBox() const {
            vec2 AA = pnt1.min(pnt2);
            vec2 BB = pnt1.max(pnt2);
            return {AA,BB};
        }

        vec2 toVec2() const { return {pnt2 - pnt1}; }

        // Translate the line segment
        line2& translate(const vec2& offset) {
            pnt1 += offset;
            pnt2 += offset;
            return *this;
        }

        // Translate the line segment
        line2 translated(const vec2& offset) {
            line2 mod = *this;
            mod.translate(offset);
            return mod;
        }

        double x_at_y(double yVal) const {
            vec2 dir = direction();
            return pnt1.x + (yVal - pnt1.y) * safe_divide(dir.x, dir.y);
        }
        
        // Y from X on infinite line
        double y_at_x(double xVal) const {
            vec2 dir = direction();
            return pnt1.y + (xVal - pnt1.x) * safe_divide(dir.y, dir.x);
        }

    };

}; // End namespace vecSys