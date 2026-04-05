#pragma once

// Keep necessary includes for dependent types (assuming they are double-specialized)
#include "base.h"
#include "vec3.h"
#include "line3.h"

// Removed includes: vec.h, typeHelp.h, concepts

namespace vecSys {
    // using namespace std; // Keep if desired, but generally avoid in headers

    struct pLine3 {
        // Data Member: Uses the double-specialized vec3
        std::vector<vec3> points;

        // --- Methods ---

        // Appends another polyline, joining if endpoints are close within tolerance.
        void append(const pLine3& other, const double tolerance = default_Tolerance) {
            if (other.points.empty()) return; // Nothing to append

            // Use tolerance squared for lengthSquared comparison
            const double toleranceSq = tolerance * tolerance;

            if (!points.empty()) {
                // Check if last point of this matches first point of other (within tolerance)
                if ((points.back() - other.points.front()).lengthSquared() < toleranceSq) {
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

        std::pair<vec3, vec3> boundingBox() const {
            if (points.empty()) {
                return {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}; // Fallback for empty
            }

            vec3 minPt = points[0];
            vec3 maxPt = points[0];

            // Can skip first point in loop
            for (size_t i = 1; i < points.size(); ++i) {
                 const vec3& p = points[i];
                 minPt = minPt.minComp(p);
                 maxPt = maxPt.maxComp(p);
            }

            return {minPt, maxPt};
        }

        vec3 centerPoint() const { // Made const
            if (points.empty()) {
                return {0.0, 0.0, 0.0}; // Return origin for empty polyline
            }
            vec3 summation = {0.0, 0.0, 0.0};
            for (const vec3& pnt : points) {
                summation += pnt;
            }
            // Ensure division by size is done using double
            // Standard division safe as points.size() > 0 here.
            return summation / static_cast<double>(points.size());
        }

        // Finds the point on the polyline (segments) closest to an external point.
        vec3 closestPointOnPolyline(const vec3& externalPoint, const double tolerance = default_Tolerance) const {
            if (points.empty()) {
                 // Return infinite vector for invalid input/state
                 return {infinity, infinity, infinity};
            }
            if (points.size() == 1) {
                return points[0];
            }

            vec3 closestPt = points[0]; // Initialize with first point
            double minDistSq = (externalPoint - points[0]).lengthSquared();

            // Check projections onto line segments
            // Assuming line3 constructor and projectOntoSegment are available and correct
            for (size_t i = 0; i < numSegments(); ++i) {
                line3 current_segment = getSegment(i);
                // Pass tolerance to projectOntoSegment
                vec3 ptOnSegment = current_segment.projectOntoSegment(externalPoint, tolerance);
                double distSq = (externalPoint - ptOnSegment).lengthSquared();
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    closestPt = ptOnSegment;
                }
            }

             // Check the last vertex explicitly. projectOntoSegment clamps to the segment,
             // so the closest point might be a vertex that isn't the closest projection point.
             double lastPtDistSq = (externalPoint - points.back()).lengthSquared();
             if (lastPtDistSq < minDistSq) {
                  // Update minimum distance and closest point if last vertex is closer
                  // minDistSq = lastPtDistSq; // Not strictly needed if only point is returned
                  closestPt = points.back();
             }

            return closestPt;
        }

        // Calculates the minimum distance from an external point to the polyline.
        double distanceToPoint(const vec3& externalPoint, const double tolerance = default_Tolerance) const {
            if (points.empty()) return infinity; // Distance is infinite if polyline is empty
            // Pass tolerance down to closestPointOnPolyline
            vec3 closest = closestPointOnPolyline(externalPoint, tolerance);
             // Check if closestPoint returned infinite vector
             if (closest.isInfinite()) return infinity;
            return (externalPoint - closest).length();
        }

        // Calculates the normalized direction vector at a specific progress along the polyline.
        vec3 directionAtProgress(double percent, const double tolerance = default_Tolerance) const {
            if (points.size() < 2) return {1.0, 0.0, 0.0}; // Default direction if not enough points

            percent = std::clamp(percent, 0.0, 1.0);
            double totalLen = totalLength();
            // Handle near-zero length case
            if (totalLen < tolerance) {
                 // If there are points but zero length, return direction of first segment (or default)
                 if (points.size() >= 2) return (points[1] - points[0]).normalize();
                 else return {1.0, 0.0, 0.0};
            }

            double targetDistance = totalLen * percent;
            double accumulated = 0.0;

            for (size_t i = 0; i < numSegments(); ++i) {
                const vec3& p1 = points[i];
                const vec3& p2 = points[i+1];
                double segmentLength = (p2 - p1).length();

                // Skip near-zero length segments
                if (segmentLength < tolerance) continue;

                // Check if target distance falls within this segment (allowing for tolerance)
                if (accumulated + segmentLength >= targetDistance - tolerance) {
                    // Assumes vec3::normalized handles zero vector case internally if needed
                    return (p2 - p1).normalize(); // Return normalized direction of this segment
                }
                accumulated += segmentLength;
            }

            // Fallback for percent ~ 1.0, return direction of the last segment
            // Ensure there is a last segment (at least 2 points)
            if (points.size() >= 2) {
                 return (points.back() - points[points.size()-2]).normalize();
            } else {
                 return {1.0, 0.0, 0.0}; // Should be unreachable if initial check passes
            }
        }


        // Return a tangent-normal-binormal frame at a progress
        // Uses a more robust method to find the initial normal vector.
        std::tuple<vec3, vec3, vec3> frameAtProgress(double percent, const double tolerance = default_Tolerance) const {
            // Pass tolerance to directionAtProgress
            vec3 tangent = directionAtProgress(percent, tolerance);

            // Handle zero tangent case (e.g., from zero-length polyline)
            // Assuming vec3::isZero uses its own appropriate tolerance or exact check
            if (tangent.isZero()) {
                 // Return a default orthogonal frame (e.g., standard basis)
                 return {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
            }

            vec3 normal, binormal;

            // Find a vector not collinear with the tangent.
            // Pick the axis vector most orthogonal to the tangent by finding
            // the component with the smallest absolute value.
            vec3 axis;
            // Using exact comparison <= is fine here for selection logic
            if (std::abs(tangent.x) <= std::abs(tangent.y) && std::abs(tangent.x) <= std::abs(tangent.z)) {
                axis = {1.0, 0.0, 0.0}; // X-axis is most orthogonal
            } else if (std::abs(tangent.y) <= std::abs(tangent.x) && std::abs(tangent.y) <= std::abs(tangent.z)) {
                axis = {0.0, 1.0, 0.0}; // Y-axis is most orthogonal
            } else {
                axis = {0.0, 0.0, 1.0}; // Z-axis is most orthogonal
            }

            // Calculate binormal first using the chosen axis
            binormal = tangent.cross(axis).normalize();
            // Recalculate normal using tangent and binormal for guaranteed orthogonality
            normal = binormal.cross(tangent).normalize(); // Note order: B x T = N

            // Fallback if binormal became zero (tangent was parallel to chosen axis)
            // Assumes vec3::isZero handles tolerance
            if (binormal.isZero()) {
                 // Try a different axis (this should be rare with the selection logic)
                 axis = {0.0, 1.0, 0.0}; // Try Y-axis
                 // Use tolerance to check for near-parallelism
                 if (std::abs(tangent.dot(axis)) > 1.0 - tolerance) { // Check if parallel to Y
                      axis = {0.0, 0.0, 1.0}; // Try Z-axis
                 }
                 binormal = tangent.cross(axis).normalize();
                 normal = binormal.cross(tangent).normalize();
                 // If still degenerate, return default frame
                 if(binormal.isZero() || normal.isZero()){
                      return {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
                 }
            }

            return {tangent, normal, binormal};
        }


        // Return double-specialized line3 segment at index
        line3 getSegment(size_t index) const { // Use size_t for index
            if (points.size() < 2 || index >= points.size() - 1) {
                 throw std::out_of_range("Segment index out of range or polyline has < 2 points");
            }
            return {points[index], points[index+1]}; // Assuming line3 constructor {vec3, vec3}
        }

        void insertPoint(size_t index, const vec3& point) { // Use size_t for index
            if (index >= points.size()) {
                points.push_back(point); // Append if index is at or beyond the end
            } else {
                points.insert(points.begin() + index, point); // Insert before the element at index
            }
        }

        // Moves progress along the polyline by a distance delta.
        double moveAlong(double currentProgress, double distanceDelta, const double tolerance = default_Tolerance) const {
            if (points.size() < 2) return currentProgress;

            double total = totalLength();
            // Avoid division by zero/NaN if total length is negligible
            if (total < tolerance) return currentProgress;

            // Calculate new distance and clamp to [0, total]
            double currentDistance = currentProgress * total;
            double newTotalDistance = currentDistance + distanceDelta;
            double clampedDistance = std::clamp(newTotalDistance, 0.0, total);

            // Return new progress percentage using standard division (total guaranteed > tolerance)
            return clampedDistance / total;
        }


        void removePoint(size_t index) {
            if (index < points.size()) {
                points.erase(points.begin() + index);
            }
             // Consider adding a check if points.size() becomes < 2 and handling that case if needed.
        }

        size_t numSegments() const { // Return size_t
            return points.empty() ? 0 : points.size() - 1;
        }

        // Orbit the shape around an external point using a rotation axis
        pLine3& orbit(const vec3& orbitPoint, const vec3& axis, const double& rads) {
            if (points.empty()) return *this;
            vec3 center = centerPoint(); // Use average vertex position as pivot
            vec3 change = center.orbited(orbitPoint, axis ,rads) - center; // Calculate translation
            for (vec3& pnt : points) {
                 pnt += change; // Apply translation to all points
            }
            return *this;
        }

        // Returns a new polyline orbited around an external point
        pLine3 orbited(const vec3& orbitPoint, const vec3& axis, const double& rads) const {
            pLine3 mod = *this;
            mod.orbit(orbitPoint, axis ,rads); // Call the in-place orbit
            return mod;
        }

        // AABB overlap with a line3 (exact comparison is fine)
        bool overlapsAABB(const line3& other) const {
             if (points.empty()) return false;
             auto [minA, maxA] = boundingBox();
             auto [minB, maxB] = other.boundingBox(); // Assuming line3 has boundingBox
             return (minA.x <= maxB.x && maxA.x >= minB.x &&
                     minA.y <= maxB.y && maxA.y >= minB.y &&
                     minA.z <= maxB.z && maxA.z >= minB.z);
        }

        // AABB overlap with another pLine3 (exact comparison is fine)
        bool overlapsAABB(const pLine3& other) const {
             if (points.empty() || other.points.empty()) return false;
             auto [minA, maxA] = boundingBox();
             auto [minB, maxB] = other.boundingBox();
             return (minA.x <= maxB.x && maxA.x >= minB.x &&
                     minA.y <= maxB.y && maxA.y >= minB.y &&
                     minA.z <= maxB.z && maxA.z >= minB.z);
        }

        // AABB overlap with a raw {vec3, vec3} representing min/max corners (exact comparison is fine)
        bool overlapsAABB(const std::pair<vec3, vec3>& box) const {
             if (points.empty()) return false;
             auto [minA, maxA] = boundingBox();
             const auto& [minB, maxB] = box; // Deconstruct the input box
             return (minA.x <= maxB.x && maxA.x >= minB.x &&
                     minA.y <= maxB.y && maxA.y >= minB.y &&
                     minA.z <= maxB.z && maxA.z >= minB.z);
        }

        // Calculates the point at a specific progress along the polyline.
        vec3 pointAtProgress(double percent, const double tolerance = default_Tolerance) const {
            if (points.empty()) return {0.0, 0.0, 0.0};
            if (points.size() == 1) return points[0];

            percent = std::clamp(percent, 0.0, 1.0);
            double totalLen = totalLength();
            // Handle near-zero length case
            if (totalLen < tolerance) return points[0];

            double targetDistance = totalLen * percent;
            double accumulated = 0.0;

            for (size_t i = 0; i < numSegments(); ++i) {
                const vec3& p1 = points[i];
                const vec3& p2 = points[i+1];
                double segmentLength = (p2 - p1).length();

                // Skip near-zero length segments
                if (segmentLength < tolerance) continue;

                // Check if target distance falls within this segment (allowing for tolerance)
                if (accumulated + segmentLength >= targetDistance - tolerance) {
                    double remainingDist = targetDistance - accumulated;
                    // Denominator segmentLength guaranteed > tolerance, standard division safe.
                    double localT = std::clamp(remainingDist / segmentLength, 0.0, 1.0);
                    return p1.lerped(p2, localT); // Assumes vec3::lerped exists
                }
                accumulated += segmentLength;
            }

            // Fallback for percent ~ 1.0 (should ideally be caught by the loop)
            return points.back();
        }

        // Calculates the progress along the polyline closest to an external point.
        double progressAlongPolyline(const vec3& point, const double tolerance = default_Tolerance) const {
            if (points.size() < 2) return 0.0;

            double bestDistanceSq = infinity;
            double bestProgress = 0.0;
            double accumulatedLength = 0.0;
            double total = totalLength();

            // Handle near-zero total length
            if (total < tolerance) return 0.0;

            for (size_t i = 0; i < numSegments(); ++i) {
                line3 seg = getSegment(i);
                // Pass tolerance to projectOntoSegment
                vec3 closestOnSegment = seg.projectOntoSegment(point, tolerance);
                double distSq = (closestOnSegment - point).lengthSquared();

                double segmentLen = seg.length();

                // Check if this segment contains a point closer than previously found
                if (distSq < bestDistanceSq) {
                    bestDistanceSq = distSq;

                    // Calculate the distance along the *current* segment to the closest point
                    double localDist = 0.0;
                    if (segmentLen > tolerance) { // Avoid issues with zero-length segment
                         localDist = (closestOnSegment - points[i]).length();
                    }
                    // Calculate overall progress up to the closest point on this segment
                    // Denominator total guaranteed > tolerance, standard division safe.
                    bestProgress = (accumulatedLength + localDist) / total;
                }
                // Accumulate length only if segment has significant length
                if(segmentLen >= tolerance) {
                    accumulatedLength += segmentLen;
                }
            }

            // Clamp final progress due to potential precision errors
            return std::clamp(bestProgress, 0.0, 1.0);
        }


        // Reverse the order of the points
        pLine3& reverseWinding() {
            std::reverse(points.begin(), points.end());
            return *this;
        }

        // Return a new pLine3 with reversed winding (immutable version)
        pLine3 reversedWinding() const {
            pLine3 mod = *this;
            return mod.reverseWinding(); // Call in-place reverse on the copy
        }

        // Rotate polyline around its center point using a rotation axis
        void rotate(const vec3& axis, const double& rads) {
            if (points.size() < 2) return; // Nothing to rotate if 0 or 1 point
            vec3 cent = centerPoint(); // Use average vertex position as pivot
            for (vec3& pnt : points) {
                // Use vec3::orbit to rotate pnt around cent
                pnt.orbit(cent, axis, rads);
            }
        }

        // Returns a new polyline rotated around its center point
        pLine3 rotated(const vec3& axis, const double& rads) const {
            pLine3 mod = *this;
            mod.rotate(axis, rads); // Call the in-place rotate
            return mod;
        }

        // Uniformly scale the polyline around a pivot point
        pLine3& scale(double factor, vec3 pivot = {0.0, 0.0, 0.0}) {
            for (vec3& pnt : points) {
                pnt = pivot + (pnt - pivot) * factor;
            }
            return *this;
        }

        // Return a new pLine3 uniformly scaled (immutable version)
        pLine3 scaled(double factor, vec3 pivot = {0.0, 0.0, 0.0}) const {
            pLine3 mod = *this;
            return mod.scale(factor, pivot); // Call the in-place scale
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

        // Simplify (Ramer–Douglas–Peucker 3D)
        pLine3& simplify(const double tolerance = default_Tolerance) { // Default tolerance used here
            if (points.size() < 3) return *this; // Cannot simplify below 3 points effectively

            std::vector<vec3> simplified;
            simplified.push_back(points.front()); // Always keep the first point

            // Recursive helper lambda
            auto recursiveSimplify = [&](auto&& self, size_t start, size_t end) -> void {
                if (end <= start + 1) return; // Base case: only two points left

                const vec3& A = points[start];
                const vec3& B = points[end];
                line3 segment_line{A, B}; // Line segment for distance calculation

                double maxDistSq = 0.0; // Use squared distance for efficiency
                size_t index = start; // Index of the point furthest from the segment

                // Find the point with the maximum perpendicular distance to the line segment
                for (size_t i = start + 1; i < end; ++i) {
                    // Use squared distance to avoid sqrt
                    // Need distance to INFINITE line for RDP
                    // Pass tolerance to distanceToPoint
                    double distSq = segment_line.distanceToPoint(points[i], tolerance);
                    distSq *= distSq; // Square the result
                    if (distSq > maxDistSq) {
                        maxDistSq = distSq;
                        index = i;
                    }
                }

                // If the max distance exceeds tolerance, recursively simplify
                if (maxDistSq > tolerance * tolerance) { // Compare squared distance with tolerance^2
                    self(self, start, index);         // Simplify first part
                    simplified.push_back(points[index]); // Add the splitting point
                    self(self, index, end);           // Simplify second part
                }
                // Otherwise, the segment [start, end] is a good enough approximation
            };

            // Start the recursive simplification
            recursiveSimplify(recursiveSimplify, 0, points.size() - 1);
            simplified.push_back(points.back()); // Always keep the last point

            points = std::move(simplified); // Replace original points
            return *this;
        }


        // Returns a new simplified polyline (immutable version)
        pLine3 simplified(const double tolerance = default_Tolerance) const { // Default tolerance used here
            pLine3 copy = *this;
            // Pass tolerance to the in-place simplify
            return copy.simplify(tolerance);
        }

        // Catmull-Rom smoothing in 3D
        pLine3& smooth(int subdivisions = 5) {
            *this = smoothed(subdivisions); // Assign result of immutable version
            return *this;
        }

        // Returns a new smoothed polyline using Catmull-Rom interpolation
        pLine3 smoothed(int subdivisions = 5) const {
            if (points.size() < 2 || subdivisions <= 0) return *this; // Return self if not enough points or no subdivisions

            pLine3 smoothedPath;
            // Estimate required capacity
            smoothedPath.points.reserve((points.size() - 1) * subdivisions + 1);

            // Catmull-Rom interpolation function (works component-wise for vec3)
            auto interpolate = [](const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3, double t) -> vec3 {
                double t2 = t * t;
                double t3 = t2 * t;
                // Using vec3 operators assumed to be defined (opposite, +, *)
                return (p1 * 2.0) +
                       (p0.opposite() + p2) * t +
                       (p0*2.0 - p1*5.0 + p2*4.0 - p3) * t2 +
                       (p0.opposite() + p1*3.0 - p2*3.0 + p3) * t3
                       * 0.5; // Multiply the whole expression by 0.5
            };

            // Add the first point explicitly only if the polyline isn't degenerate
             if (!points.empty()){
                smoothedPath.points.push_back(points.front());
             }

            // Interpolate between segments
            for (size_t i = 0; i < points.size() - 1; ++i) {
                // Determine control points P0, P1, P2, P3 for the current segment (P1 to P2)
                // Handle endpoints by duplicating the first/last point
                const vec3& p0 = (i == 0) ? points[i] : points[i-1]; // Duplicate first point if at start
                const vec3& p1 = points[i];
                const vec3& p2 = points[i+1];
                const vec3& p3 = (i + 2 < points.size()) ? points[i+2] : points[i+1]; // Duplicate last point if at end

                // Generate subdivided points along the curve segment (excluding the start point p1)
                for (int j = 1; j <= subdivisions; ++j) { // Start j from 1 to include the endpoint p2
                    double t = static_cast<double>(j) / subdivisions;
                    smoothedPath.points.push_back(interpolate(p0, p1, p2, p3, t));
                }
            }
            // The last point (points.back()) should be added automatically by the last interpolation step where t=1.
            // Redundant check removed for clarity, ensure interpolate(..., 1.0) yields exactly p2.


            return smoothedPath;
        }


        // Split pLine3 into two at a progress value [0, 1]
        std::pair<std::vector<vec3>, std::vector<vec3>> splitAtProgress(double percent, const double tolerance = default_Tolerance) const {
            std::vector<vec3> path1, path2;
            if (points.size() < 2) return {path1, path2}; // Cannot split if fewer than 2 points

            percent = std::clamp(percent, 0.0, 1.0);
            double totalLen = totalLength();

            // Handle near-zero length case
            if (totalLen < tolerance) {
                 // If zero length, split depends on percent. Arbitrarily put all in path1 if <= 0.5
                 if (percent <= 0.5) {
                     path1 = points; // Copy all points
                 } else {
                     path2 = points; // Copy all points
                 }
                 return {path1, path2};
            }

            double targetDist = totalLen * percent;
            double accumulated = 0.0;
            vec3 splitPoint = points[0]; // Default if percent is 0
            size_t splitSegmentIndex = points.size(); // Sentinel value

            for (size_t i = 0; i < numSegments(); ++i) {
                const vec3& A = points[i];
                const vec3& B = points[i+1];
                double segLen = (B - A).length();

                // Skip near-zero length segments
                if (segLen < tolerance) continue;

                // Check if target distance falls within this segment (use tolerance)
                if (accumulated + segLen >= targetDist - tolerance) {
                    double remainingDist = targetDist - accumulated;
                    // Denominator segLen guaranteed > tolerance, standard division safe.
                    double localT = std::clamp(remainingDist / segLen, 0.0, 1.0);
                    splitPoint = A.lerped(B, localT); // Assumes vec3::lerped exists
                    splitSegmentIndex = i;
                    break; // Found the split point
                }
                accumulated += segLen;
            }

             // If splitSegmentIndex remains sentinel, it implies percent was ~1.0
             if (splitSegmentIndex == points.size()) {
                  splitPoint = points.back();
                  // If only 1 segment, splitSegmentIndex should be 0
                  splitSegmentIndex = (points.size() > 1) ? points.size() - 2 : 0;
             }


            // Build path1: from start up to split point
            path1.insert(path1.end(), points.begin(), points.begin() + splitSegmentIndex + 1); // Include start point of split segment
            // Avoid adding splitPoint if it's identical to the last point added
            if (path1.empty() || (path1.back() - splitPoint).lengthSquared() > tolerance * tolerance) {
                 path1.push_back(splitPoint);
            }


            // Build path2: from split point to the end
            path2.push_back(splitPoint); // Start with the split point
            // Add remaining points if the split didn't happen exactly at the last point
            if (splitSegmentIndex + 1 < points.size()) {
                 path2.insert(path2.end(), points.begin() + splitSegmentIndex + 1, points.end());
            }


            return {path1, path2};
        }

        // Return vector of double-specialized line3 segments
        std::vector<line3> toLines() const {
            std::vector<line3> lines;
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

        void translate(const vec3& amnt) {
             for (vec3& point: points) {
                  point += amnt;
             }
        }

        pLine3 translated(const vec3& amnt) const {
            pLine3 mod = *this;
            mod.translate(amnt); // Call the in-place translate
            return mod;
        }
    }; // End struct pLine3
}; // End namespace vecSys