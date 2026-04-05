#pragma once

#include "base.h"
#include "vec2.h"
#include "line2.h"
#include "shape2.h"

namespace vecSys {
    // using namespace std; // Keep if desired, but generally avoid in headers

    struct pLine2 {

        // Data Member: Uses the double-specialized vec2
        std::vector<vec2> points;

        // --- Methods ---

        void append(const pLine2& other) {
            if (other.points.empty()) return; // Nothing to append

            // Use double default_Tolerance for comparison
            constexpr double default_ToleranceSq = default_Tolerance * default_Tolerance; // Squared default_Tolerance for lengthSquared comparison

            if (!points.empty()) {
                // Check if last point of this matches first point of other
                if ((points.back() - other.points.front()).lengthSquared() < default_ToleranceSq) {
                    // If they match, skip the first point of 'other'
                    points.insert(points.end(), other.points.begin() + 1, other.points.end());
                } else {
                    // If they don't match, append all points
                    points.insert(points.end(), other.points.begin(), other.points.end());
                }
            } else {
                // If this polyline is empty, just copy all points
                points.assign(other.points.begin(), other.points.end()); // Use assign for empty initial vector
            }
        }

        vec2 centerPoint() const { // Made const
            if (points.empty()) {
                return {0.0, 0.0}; // Return origin for empty polyline
            }
            vec2 summation = {0.0, 0.0};
            for (const vec2& pnt : points) {
                summation += pnt;
            }
            return summation / points.size();
        }

        vec2 closestPointOnPolyline(const vec2& externalPoint) const {
            if (points.empty()) {
                 // Return NaN or a specific indicator? Or throw? Returning origin for now.
                 return {infinity, infinity};
            }
            if (points.size() == 1) {
                return points[0];
            }

            vec2 closestPt = points[0]; // Initialize with first point
            double minDistSq = (externalPoint - points[0]).lengthSquared();

            // Check projections onto line segments
            for (const auto& line : toLines()) {
                 // Assuming line2::projectOntoSegment is double-specialized
                 vec2 ptOnSegment = line.projectOntoSegment(externalPoint);
                 double distSq = (externalPoint - ptOnSegment).lengthSquared();
                 if (distSq < minDistSq) {
                     minDistSq = distSq;
                     closestPt = ptOnSegment;
                 }
            }

             // Check the last vertex explicitly (as per original logic, though potentially redundant)
             // This handles cases where the closest point IS the last vertex.
             double lastPtDistSq = (externalPoint - points.back()).lengthSquared();
             if (lastPtDistSq < minDistSq) {
                // No need to update minDistSq as we just need the point
                 closestPt = points.back();
             }

            return closestPt;
        }

        double distanceToPoint(const vec2& externalPoint) const {
            if (points.empty()) return infinity; // Distance is infinite if polyline is empty
            vec2 closest = closestPointOnPolyline(externalPoint);
             // Check if closestPoint returned NaN
             if (std::isnan(closest.x)) return infinity;
            return (externalPoint - closest).length();
        }

        vec2 directionAtProgress(double percent) const {
            if (points.size() < 2) return {1.0, 0.0}; // Default direction if not enough points
        
            percent = std::clamp(percent, 0.0, 1.0);
            double targetDistance = totalLength() * percent;
            double accumulated = 0.0;
        
            for (size_t i = 0; i < numSegments(); ++i) {
                double segmentLength = (points[i+1] - points[i]).length();
                if (accumulated + segmentLength >= targetDistance) {
                    return (points[i+1] - points[i]).normalize();
                }
                accumulated += segmentLength;
            }
        
            return (points.back() - points[points.size() - 2]).normalize(); // Last segment
        }

        std::pair<vec2, vec2> boundingBox() const {
            if (points.empty()) {
                // Return zero-size box at origin, or maybe NaN box?
                return {{0.0, 0.0}, {0.0, 0.0}};
            }

            vec2 minPt = points[0];
            vec2 maxPt = points[0];

            // Can skip the first point in the loop
            for (size_t i = 1; i < points.size(); ++i) {
                minPt = minPt.min(points[i]);
                maxPt = maxPt.max(points[i]);
            }
            return {minPt, maxPt};
        }

        // Return double-specialized line2
        line2 getSegment(size_t index) const { // Use size_t for index
            if (points.size() < 2 || index >= points.size() - 1) {
                 throw std::out_of_range("Segment index out of range or polyline has < 2 points");
            }
            return {points[index], points[index+1]}; // Assuming line2 constructor {vec2, vec2}
        }

        void insertPoint(size_t index, const vec2& point) { // Use size_t for index
            if (index >= points.size()) {
                points.push_back(point); // Append if index is at or beyond the end
            } else {
                points.insert(points.begin() + index, point); // Insert before the element at index
            }
        }

        double moveAlong(double currentProgress, double distanceDelta, double default_Tolerance = default_Tolerance) const {
            if (points.size() < 2) return currentProgress;
        
            double total = totalLength();
            if (total < default_Tolerance) return currentProgress;
        
            double newDistance = std::clamp(currentProgress * total + distanceDelta, 0.0, total);
            return newDistance / total;
        }

        vec2 normalAtProgress(double percent) const {
            vec2 dir = directionAtProgress(percent);
            return dir.perpendicularCCW().normalize(); // Rotate 90 degrees CCW
        }

        size_t numSegments() const { // Return size_t
            return points.empty() ? 0 : points.size() - 1;
        }

        // Orbit the shape around an external point
        pLine2& orbit(const vec2& orbitCenter, const double& rads) {
            if (points.empty()) return *this;
            vec2 center = centerPoint();
            vec2 change = center.orbited(orbitCenter, rads) - center;
            for (vec2& pnt : points) {
                 pnt += change;
            }
            return *this;
        }

        pLine2 orbited(const vec2& point, const double& rads) const { // Made const
            pLine2 mod = *this;
            mod.orbit(point, rads); // Use the modified orbit logic
            return mod;
        }

        bool overlapsAABB(const line2& other) const {
            auto aabb = other.boundingBox();
            auto [minPt, maxPt] = boundingBox();
            return (minPt.x <= aabb.second.x && maxPt.x >= aabb.first.x &&
                    minPt.y <= aabb.second.y && maxPt.y >= aabb.first.y);
        }

        bool overlapsAABB(const pLine2& other) const {
            auto aabb = other.boundingBox();
            auto [minPt, maxPt] = boundingBox();
            return (minPt.x <= aabb.second.x && maxPt.x >= aabb.first.x &&
                    minPt.y <= aabb.second.y && maxPt.y >= aabb.first.y);
        }

        bool overlapsAABB(const shape2& other) const {
            auto aabb = other.boundingBox();
            auto [minPt, maxPt] = boundingBox();
            return (minPt.x <= aabb.second.x && maxPt.x >= aabb.first.x &&
                    minPt.y <= aabb.second.y && maxPt.y >= aabb.first.y);
        }

        bool overlapsAABB(const std::pair<vec2, vec2>& aabb) const {
            auto [minPt, maxPt] = boundingBox();
            return (minPt.x <= aabb.second.x && maxPt.x >= aabb.first.x &&
                    minPt.y <= aabb.second.y && maxPt.y >= aabb.first.y);
        }

        vec2 pointAtProgress(double percent) const {
            if (points.empty()) return {0.0, 0.0};
            if (points.size() == 1) return points[0];
            
            percent = std::clamp(percent, 0.0, 1.0);
            double targetDistance = totalLength() * percent;
            double accumulated = 0.0;
            
            for (size_t i = 0; i < numSegments(); ++i) {
                double segmentLength = (points[i+1] - points[i]).length();
                if (accumulated + segmentLength >= targetDistance) {
                    double localT = (targetDistance - accumulated) / segmentLength;
                    return points[i].lerped(points[i+1], localT);
                }
                accumulated += segmentLength;
            }
            
            return points.back();
        }
        
        double progressAlongPolyline(const vec2& point) const {
            if (points.size() < 2) return 0.0;
        
            double bestDistanceSq = infinity;
            double bestProgress = 0.0;
            double accumulated = 0.0;
            double total = totalLength();
        
            for (size_t i = 0; i < numSegments(); ++i) {
                line2 seg{points[i], points[i+1]};
                vec2 closest = seg.projectOntoSegment(point);
                double distSq = (closest - point).lengthSquared();
        
                if (distSq < bestDistanceSq) {
                    bestDistanceSq = distSq;
        
                    double localProgress = (closest - points[i]).length() / (points[i+1] - points[i]).length();
                    bestProgress = (accumulated + (localProgress * (points[i+1] - points[i]).length())) / total;
                }
        
                accumulated += (points[i+1] - points[i]).length();
            }
        
            return std::clamp(bestProgress, 0.0, 1.0);
        }

        void removePoint(size_t index) { 
            if (index < points.size()) {
                points.erase(points.begin() + index);
            }
        }

        // Reverse the order of the points (flip winding)
        pLine2& reverseWinding() {
            std::reverse(points.begin(), points.end());
            return *this;
        }

        pLine2 reversedWinding () const {
            pLine2 mod = *this;
            return mod.reverseWinding();
        }

        // Rotate polyline around its center point
        void rotate(const double& rads) {
            if (points.size() < 2) return; // Nothing to rotate if 0 or 1 point
            vec2 cent = centerPoint();
            for (vec2& pnt : points) {
                // Translate to origin, rotate, translate back
                pnt.orbit(cent, rads); // vec2::orbit performs rotation around a pivot
            }
        }

        pLine2 rotated(const double& rads) const { // Made const
            pLine2 mod = *this;
            mod.rotate(rads);
            return mod;
        }

        pLine2& scale(double factor, vec2 pivot = {0.0, 0.0}) {
            for (vec2& pnt : points) {
                pnt = pivot + (pnt - pivot) * factor;
            }
            return *this;
        }
        
        // Immutable scaled version
        pLine2 scaled(double factor, vec2 pivot = {0.0, 0.0}) const {
            pLine2 mod = *this;
            return mod.scale(factor, pivot);
        }

        std::vector<double> segmentLengths() const {
            std::vector<double> lengths;
            if (points.size() < 2) return lengths;
            lengths.reserve(numSegments()); // Use numSegments()
            for (size_t i = 0; i < numSegments(); ++i) { // Loop up to numSegments()
                lengths.push_back((points[i+1] - points[i]).length());
            }
            return lengths;
        }

        bool selfIntersects(const double tolerance = default_Tolerance) const {
            if (points.size() < 4) return false; // Need at least 2 non-adjacent segments to intersect
            
            for (size_t i = 0; i < numSegments(); ++i) {
                line2 edge1 = getSegment(i);
                for (size_t j = i + 1; j < numSegments(); ++j) {
                    // Skip adjacent segments
                    if (j == i + 1) continue;
                    line2 edge2 = getSegment(j);
                    if (!edge1.intersection(edge2, tolerance).isInfinite()) {
                        return true; // Found intersection
                    }
                }
            }
            return false;
        }        

        pLine2& simplify(const double tolerance = default_Tolerance) {
            if (points.size() < 2) return *this; // Nothing to simplify
        
            std::vector<vec2> simplified;
            simplified.push_back(points.front());
        
            auto recursiveSimplify = [&](auto&& self, size_t startIdx, size_t endIdx) -> void {
                if (endIdx <= startIdx + 1) return;
        
                const vec2& start = points[startIdx];
                const vec2& end = points[endIdx];
        
                double maxDistSq = 0.0;
                size_t index = startIdx;
        
                for (size_t i = startIdx + 1; i < endIdx; ++i) {
                    double distSq = line2(start, end).distanceToPoint(points[i]);
                    if (distSq > maxDistSq) {
                        index = i;
                        maxDistSq = distSq;
                    }
                }
        
                if (safe_sqrt(maxDistSq) > tolerance) {
                    self(self, startIdx, index);
                    simplified.push_back(points[index]);
                    self(self, index, endIdx);
                }
            };
        
            recursiveSimplify(recursiveSimplify, 0, points.size() - 1);
            simplified.push_back(points.back());
        
            points = std::move(simplified);
            return *this;
        }

        pLine2 simplified(const double tolerance = default_Tolerance) const {
            pLine2 mod = *this;
            return mod.simplify(tolerance);
        }

        pLine2& smooth(int subdivisions = 5) {
            *this = smoothed(subdivisions);
            return *this;
        }

        pLine2 smoothed(int subdivisions = 5) const {
            if (points.size() < 2 || subdivisions <= 0) return *this;
        
            pLine2 smoothedPath;
            smoothedPath.points.reserve(points.size() * subdivisions);
        
            auto interpolate = [](const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3, double t) -> vec2 {
                double t2 = t * t;
                double t3 = t2 * t;
        
                return 0.5 * (
                    (2.0 * p1) +
                    (p0.opposite() + p2) * t +
                    (2.0*p0 - 5.0*p1 + 4.0*p2 - p3) * t2 +
                    (p0.opposite() + 3.0*p1 - 3.0*p2 + p3) * t3
                );
            };
        
            for (size_t i = 0; i < points.size() - 1; ++i) {
                vec2 p0 = (i == 0) ? points[i] : points[i-1];
                vec2 p1 = points[i];
                vec2 p2 = points[i+1];
                vec2 p3 = (i + 2 < points.size()) ? points[i+2] : points[i+1];
        
                for (int j = 0; j < subdivisions; ++j) {
                    double t = static_cast<double>(j) / subdivisions;
                    smoothedPath.points.push_back(interpolate(p0, p1, p2, p3, t));
                }
            }
        
            smoothedPath.points.push_back(points.back()); // Ensure last point is exact
        
            return smoothedPath;
        }        

        std::pair<std::vector<vec2>, std::vector<vec2>> splitAtProgress(double percent) const {
            std::vector<vec2> path1, path2;
            if (points.size() < 2) return {path1, path2}; // Return empty if too small
        
            percent = std::clamp(percent, 0.0, 1.0);
            double targetDistance = totalLength() * percent;
            double accumulated = 0.0;
        
            for (size_t i = 0; i < numSegments(); ++i) {
                const vec2& p1 = points[i];
                const vec2& p2 = points[i+1];
                double segmentLength = (p2 - p1).length();
        
                if (accumulated + segmentLength >= targetDistance) {
                    double localT = (targetDistance - accumulated) / segmentLength;
                    vec2 splitPoint = p1.lerped(p2, localT);
        
                    // Build path1
                    path1.insert(path1.end(), points.begin(), points.begin() + i + 1);
                    path1.push_back(splitPoint);
        
                    // Build path2
                    path2.push_back(splitPoint);
                    path2.insert(path2.end(), points.begin() + i + 1, points.end());
        
                    break;
                }
                accumulated += segmentLength;
            }
        
            return {path1, path2};
        }        

        // Return vector of double-specialized line2
        std::vector<line2> toLines() const { // Made const
            std::vector<line2> lines;
            if (points.size() < 2) return lines;
            lines.reserve(numSegments());
            for (size_t i = 0; i < numSegments(); ++i) { // Loop up to numSegments()
                lines.push_back({points[i], points[i+1]});
            }
            return lines;
        }

        double totalLength() const {
            if (points.size() < 2) return 0.0;
            double total = 0.0;
            for (size_t i = 0; i < numSegments(); ++i) { // Loop up to numSegments()
                total += (points[i+1] - points[i]).length();
            }
            return total;
        }

        void translate(const vec2& amnt) {
             for (vec2& point: points) {
                 point += amnt;
             }
        }

        pLine2 translated(const vec2& amnt) const { // Made const
            pLine2 mod = *this;
            mod.translate(amnt);
            return mod;
        }
    }; // End struct pLine2
}; // End namespace vecSys