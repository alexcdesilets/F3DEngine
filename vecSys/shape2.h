#pragma once

#include "base.h"
#include "vec2.h"         // Needs double-specialized vec2
#include "line2.h"        // Needs double-specialized line2
// #include "shape2.h" // Self-include removed

namespace vecSys {
    struct shape2 {

        /** shape2 is defined by a sequential list of 2d coordinate points (vertices)
         * - Vertices define edges in order: p[0]->p[1], p[1]->p[2], ..., p[n-1]->p[0].
         * - Assumes a simple polygon (edges do not intersect except at vertices).
         * - Minimum 3 vertices are required for a valid shape.
         */
        std::vector<vec2> points = {vec2{0.0, 0.0}, vec2{0.0, 10.0}, vec2{10.0, 0.0}}; // default triangle

        // --- Methods ---

        // Calculate signed area of the polygon (positive = CCW, negative = CW) using Shoelace formula
        double area() const {
            if (points.size() < 3) return 0.0;
            double result = 0.0;
            for (size_t i = 0; i < points.size(); ++i) {
                const vec2& p1 = points[i];
                const vec2& p2 = points[(i + 1) % points.size()]; // Wrap around for closing edge
                result += (p1.x * p2.y - p2.x * p1.y); // Cross product sum
            }
            return result * 0.5; // Divide by 2
        }

        // Calculate the average position of the vertices.
        // Note: This is the true centroid only for triangles. For general polygons,
        // the centroid (center of mass) calculation is more complex.
        vec2 centerPoint() const {
            if (points.size() < 3) {
                // Handle degenerate cases
                 if (points.empty()) return {0.0, 0.0}; // Origin for empty
                 if (points.size() == 1) return points[0]; // Single point
                 if (points.size() == 2) return (points[0] + points[1]) * 0.5; // Center of line segment
            }

            vec2 sum = {0.0, 0.0}; // Initialize sum
            for (const vec2& point: points) {
                 sum += point;
            }
            return sum / static_cast<double>(points.size()); // Use double division
        }

        // Calculate the geometric centroid (center of mass) of the polygon.
        // Assumes uniform density and a simple (non-self-intersecting) polygon.
        vec2 centroid(double tolerance = default_Tolerance) const {
            if (points.size() < 3) {
                 // Centroid is ill-defined for degenerate shapes, return average vertex position
                 return centerPoint();
            }

            double signedArea = area(); // Use the existing area function
            if (std::abs(signedArea) < tolerance) {
                 // Centroid is ill-defined for zero-area polygons, return average vertex position
                 return centerPoint();
            }

            vec2 centroidSum = {0.0, 0.0};
            for (size_t i = 0; i < points.size(); ++i) {
                const vec2& p1 = points[i];
                const vec2& p2 = points[(i + 1) % points.size()]; // Wrap around
                double crossTerm = (p1.x * p2.y - p2.x * p1.y); // Same term as in Shoelace formula
                centroidSum.x += (p1.x + p2.x) * crossTerm;
                centroidSum.y += (p1.y + p2.y) * crossTerm;
            }

            // The formula requires dividing by 6 * SignedArea
            double scale = 1.0 / (6.0 * signedArea);
            return centroidSum * scale;
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

        // Finds the closest point on the boundary (edges) of the shape to an external point
        vec2 closestPoint(const vec2& externalPoint) const {
            if (points.size() < 3) {
                 // Handle degenerate cases
                 if (points.empty()) return {infinity, infinity};
                 if (points.size() == 1) return points[0];
                 if (points.size() == 2) return line2(points[0], points[1]).projectOntoSegment(externalPoint); // Closest on line segment
            }

            vec2 closestPt = points[0]; // Initialize with first vertex
            double minDistSq = (externalPoint - points[0]).lengthSquared();

            // Check projections onto line segments forming the boundary
            for (const auto& line : toLines()) { // toLines() now creates closed segments
                vec2 ptOnSegment = line.projectOntoSegment(externalPoint);
                double distSq = (externalPoint - ptOnSegment).lengthSquared();
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    closestPt = ptOnSegment;
                }
            }
            // No need for explicit last point check because toLines() handles the closing segment.

            return closestPt;
        }

        // Checks if all vertices of the 'other' shape are inside 'this' shape.
        // Note: This is not a full geometric containment check (e.g., 'other' could
        // intersect 'this' without any vertices being outside).
        bool contains(const shape2& other, double tolerance = default_Tolerance) const {
            if (points.size() < 3 || other.points.size() < 3) return false; // Invalid shapes cannot contain

            for (const vec2& p : other.points) {
                if (!isInside(p, tolerance)) {
                    return false; // If any point of 'other' is outside 'this', not contained
                }
            }
            return true; // All points inside
        }

        // Computes the convex hull of the shape's vertices using Graham Scan algorithm.
        shape2 convexHull(double tolerance = default_Tolerance) const {
            if (points.size() < 3) return *this; // Degenerate case: shape too small

            std::vector<vec2> sorted = points;

            // Step 1: Find the pivot with lowest Y (and lowest X to break ties)
            auto pivot_it = std::ranges::min_element(sorted, [](const vec2& a, const vec2& b) {
                return (a.y < b.y) || (a.y == b.y && a.x < b.x);
            });
            vec2 pivot = *pivot_it;
            // Swap pivot to the beginning
            std::iter_swap(sorted.begin(), pivot_it);

            // Step 2: Sort remaining points by polar angle relative to pivot
            std::sort(sorted.begin() + 1, sorted.end(), [&pivot, &tolerance](const vec2& a, const vec2& b) {
                double cross = (a - pivot).cross(b - pivot);
                if (std::abs(cross) < tolerance) { // Use a small default_Tolerance for collinearity check
                    // Points are collinear with pivot: closer one first
                    return (pivot - a).lengthSquared() < (pivot - b).lengthSquared();
                }
                return cross > 0; // Sort counter-clockwise
            });

            // Step 3: Graham scan - build the hull
            std::vector<vec2> hull;
            hull.reserve(sorted.size());

            for (const vec2& point : sorted) {
                // Remove points that make a non-left turn
                while (hull.size() >= 2) {
                    vec2& secondLast = hull[hull.size() - 2];
                    vec2& last = hull.back();
                    // Check the cross product: > 0 is a left turn (CCW)
                    if ((last - secondLast).cross(point - last) <= 0.0) { // If not a left turn (or collinear)
                        hull.pop_back(); // Remove last point
                    } else {
                        break; // Found a left turn, keep last point
                    }
                }
                hull.push_back(point);
            }

            return shape2{hull}; // Return new convex shape
        }

        // Decomposes the polygon into a set of convex triangles using the Ear Clipping algorithm.
        // Assumes the input is a simple polygon.
        std::vector<shape2> convexDecomposition(double tolerance = default_Tolerance) const {
            // Helper lambda to check if point p is inside the triangle defined by a, b, c using barycentric coordinates
             auto pointInTriangle = [&tolerance](const vec2& p, const vec2& a, const vec2& b, const vec2& c) {
                 vec2 v0 = c - a;
                 vec2 v1 = b - a;
                 vec2 v2 = p - a;

                 double dot00 = v0.dot(v0);
                 double dot01 = v0.dot(v1);
                 double dot02 = v0.dot(v2);
                 double dot11 = v1.dot(v1);
                 double dot12 = v1.dot(v2);

                 double denom = dot00 * dot11 - dot01 * dot01;
                 // Use a slightly larger default_Tolerance for floating point comparisons in geometry
                 if (std::abs(denom) < tolerance) return false; // Degenerate triangle

                 double invDenom = 1.0 / denom;
                 double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                 double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

                 // Check if the point is within the triangle boundaries (allowing for tolerance)
                 return (u >= -tolerance) && (v >= -tolerance) && (u + v <= 1.0 + tolerance);
             };


            std::vector<shape2> result;

            if (points.size() < 3) return result; // Cannot decompose degenerate shapes
            if (points.size() == 3) {
                 result.push_back(*this); // Already a triangle (convex)
                 return result;
            }

            shape2 working = *this;
            working.ensureCounterClockwise(); // Ear clipping assumes CCW winding

            // Keep track of indices to handle vertex removal correctly
            std::vector<size_t> indices(working.points.size());
            std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, 2, ...

            int iterations = 0; // Safety break for potential infinite loops
            int max_iterations = working.points.size() * 2; // Heuristic limit

            while (indices.size() >= 3 && iterations < max_iterations) {
                bool earFound = false;
                for (size_t i = 0; i < indices.size(); ++i) {
                    size_t prevIdx = indices[(i + indices.size() - 1) % indices.size()];
                    size_t currIdx = indices[i];
                    size_t nextIdx = indices[(i + 1) % indices.size()];

                    const vec2& prev = working.points[prevIdx];
                    const vec2& curr = working.points[currIdx];
                    const vec2& next = working.points[nextIdx];

                    // Check if the vertex is convex (forms a left turn in CCW polygon)
                    if ((next - curr).cross(prev - curr) >= -default_Tolerance) { // Allow for slight collinearity
                        bool validEar = true;
                        // Check if any other vertex lies inside the potential ear triangle
                        for (size_t j = 0; j < indices.size(); ++j) {
                            size_t testIdx = indices[j];
                            // Skip the vertices forming the ear itself
                            if (testIdx == prevIdx || testIdx == currIdx || testIdx == nextIdx) continue;

                            if (pointInTriangle(working.points[testIdx], prev, curr, next)) {
                                validEar = false;
                                break;
                            }
                        }

                        if (validEar) {
                            // Found an ear, clip it
                            result.push_back(shape2({prev, curr, next}));

                            // Remove the ear vertex index
                            indices.erase(indices.begin() + i);
                            earFound = true;
                            break; // Restart search after removing an ear
                        }
                    }
                }
                 iterations++;
                if (!earFound && indices.size() > 3) {
                     // Should not happen for simple polygons, but as a fallback:
                     // If no ear is found after checking all vertices, something is wrong
                     // or the polygon might be complex/self-intersecting.
                     // For now, just stop and return what we have. Consider logging an error.
                     // If the remaining shape is convex, add it.
                     // shape2 remaining_shape;
                     // for(size_t idx : indices) remaining_shape.points.push_back(working.points[idx]);
                     // if(remaining_shape.isConvex()) result.push_back(remaining_shape);
                     break;
                }
            }

            // The last remaining 3 indices should form the final triangle
            // (This check might be redundant if the loop correctly terminates)
            // if (indices.size() == 3) {
            //    result.push_back(shape2({working.points[indices[0]], working.points[indices[1]], working.points[indices[2]]}));
            // }

            return result;
        }

        // Ensures the shape's vertices are ordered counter-clockwise.
        shape2& ensureCounterClockwise() {
            if (isClockwise()) {
                reverseWinding();
            }
            return *this;
        }

        // Returns the line segment (edge) at the given index (wraps around)
        line2 getSegment(size_t index) const {
            size_t n = points.size();
            if (n < 3) { // Need at least 3 points for a shape segment
                throw std::out_of_range("Shape has fewer than 3 points, cannot get segment.");
            }
            if (index >= n) {
                 throw std::out_of_range("Segment index out of range");
            }
            // Connects points[index] to points[(index+1) % n]
            return {points[index], points[(index + 1) % n]};
        }

        // Inserts a point at the specified index.
        shape2& insertPoint(size_t index, const vec2& point) {
            if (index >= points.size()) {
                points.push_back(point); // Append if index is at or beyond the end
            } else {
                points.insert(points.begin() + index, point); // Insert before the element at index
            }
            return *this;
        }

        // Removes the point at the specified index, ensuring the shape remains valid (>= 3 points).
        shape2& removePoint(size_t index) {
            // Enforce minimum of 3 points for a valid shape
            if (points.size() > 3 && index < points.size()) {
                points.erase(points.begin() + index);
            }
            // Optionally handle cases where removal is not allowed or index is invalid
            // else if (index >= points.size()) { /* Throw or log error */ }
            // else { /* Log: Cannot remove point, would make shape invalid */ }
            return *this;
        }

        // Distance from external point to the closest point on the shape's boundary
        double distanceToPoint(const vec2& externalPoint) const {
             if (points.size() < 3) { // Handle degenerate shapes
                  if (points.empty()) return infinity;
                  if (points.size() == 1) return (externalPoint-points[0]).length();
                  if (points.size() == 2) return line2(points[0], points[1]).distanceToPointSegment(externalPoint);
             }
             vec2 closest = closestPoint(externalPoint);
             // Check if closestPoint returned NaN (e.g., from empty shape input)
             if (std::isnan(closest.x)) return infinity;
             return (externalPoint - closest).length();
        }

        // Number of segments (edges) in the closed shape, equals number of points
        size_t numSegments() const {
             // For a closed shape with N points, there are N segments
             // Return 0 if fewer than 3 points? Or points.size()? Current logic is consistent.
             return points.size();
        }

        // Check if shape points are ordered clockwise
        bool isClockwise() const {
            // Area is negative for clockwise winding
            return area() < 0.0;
        }

        // Check if shape points are ordered counterclockwise
        bool isCounterClockwise() const {
            // Area is positive for counter-clockwise winding
            return area() > 0.0;
        }

        // Check if the shape is convex (all internal angles <= 180 degrees)
        bool isConvex(double tolerance = 1e-6) const {
            if (points.size() < 4) return true; // Triangle (or less) is always convex

            bool gotPositive = false;
            bool gotNegative = false;

            for (size_t i = 0; i < points.size(); ++i) {
                const vec2& A = points[i];
                const vec2& B = points[(i + 1) % points.size()];
                const vec2& C = points[(i + 2) % points.size()];

                // Calculate the cross product of consecutive edge vectors (B-A) x (C-B)
                double cross = (B - A).cross(C - B);

                // Check the sign of the cross product (determines turn direction)
                if (cross > tolerance) gotPositive = true;
                else if (cross < -tolerance) gotNegative = true;

                // If we have both positive (left turns) and negative (right turns), it's concave
                if (gotPositive && gotNegative) return false;
            }
            // If all turns have the same sign (or are collinear), it's convex
            return true;
        }

        // Checks if a point is inside the polygon using the Ray Casting algorithm.
        bool isInside(const vec2& testPoint, double tolerance = default_Tolerance) const { // Use smaller tolerance for point checks
            if (points.size() < 3) return false;

            // 1. Boundary Check: Check if the point lies on any edge segment.
            for (size_t i = 0; i < points.size(); ++i) {
                // Use a slightly larger tolerance for segment check? Or keep consistent?
                if (getSegment(i).pointOnSegment(testPoint, tolerance)) {
                    return true; // Point is on the boundary, considered inside.
                }
            }

            // 2. Ray Casting: Cast a ray horizontally to the right (+X direction).
            int crossings = 0;
            size_t n = points.size();
            double tx = testPoint.x;
            double ty = testPoint.y;

            for (size_t i = 0; i < n; ++i) {
                const vec2& p1 = points[i];
                const vec2& p2 = points[(i + 1) % n];

                // Ensure p1.y <= p2.y for consistent handling
                const vec2& lower = (p1.y < p2.y) ? p1 : p2;
                const vec2& upper = (p1.y < p2.y) ? p2 : p1;

                // Skip segment if it's completely above, below, or to the left of the test point
                if (upper.y <= ty - tolerance || // Segment below ray
                    lower.y > ty + tolerance ||  // Segment above ray
                    std::max(lower.x, upper.x) < tx - tolerance) { // Segment entirely left of point
                    continue;
                }

                // Handle horizontal segments - already covered by boundary check if point is on them.
                // If ray is collinear with horizontal segment, standard algorithms often ignore or handle specially.
                // Here, boundary check suffices.
                if (std::abs(lower.y - upper.y) < tolerance) {
                     continue;
                }

                // Calculate intersection x-coordinate of the edge with the horizontal ray
                double x_intersect = (ty - lower.y) * (upper.x - lower.x) / (upper.y - lower.y) + lower.x;

                // Count crossing only if intersection is to the right (or exactly at the point)
                if (x_intersect >= tx - tolerance) {
                    // Handle vertex crossing: If ray passes through upper vertex, only count if the other vertex is below.
                    // This prevents double counting when ray hits a vertex shared by two edges.
                    if (std::abs(upper.y - ty) < tolerance) {
                         // Check the *next* segment's orientation relative to this one.
                         const vec2& p_next = points[(i + 2) % n];
                         // Only count if the edge following the vertex goes *upwards* relative to the ray
                         if (p_next.y > ty + tolerance) {
                              crossings++;
                         }
                    } else {
                         crossings++;
                    }
                }
            }

            return (crossings % 2) == 1; // Odd number of crossings means the point is inside
        }


        // Check if the point is inside the Axis-Aligned Bounding Box
        bool isInside_AABB(const vec2& point) const {
            // Check if points vector is populated before calculating bounding box
            if (points.empty()) return false;
            std::pair<vec2, vec2> minMax = boundingBox();
            return point.x >= minMax.first.x && point.x <= minMax.second.x &&
                   point.y >= minMax.first.y && point.y <= minMax.second.y;
        }

        // Modifies the shape to become its convex hull.
        shape2& makeConvex() {
            *this = convexHull(); // Replace points with hull points
            return *this;
        }

        // Move along the perimeter by distanceDelta (positive or negative), returning new progress [0, 1]
        double moveAlong(double currentProgress, double distanceDelta, double default_Tolerance = 1.0e-6) const {
            if (points.size() < 3) return currentProgress; // Cannot move along degenerate shape

            double total = totalLength();
            if (total < default_Tolerance) return currentProgress; // Avoid division by zero if perimeter is tiny

            // Calculate current distance and new distance, handling wrap-around
            double currentDistance = currentProgress * total;
            double newTotalDistance = currentDistance + distanceDelta;

            // Use fmod for wrap-around behavior
            newTotalDistance = std::fmod(newTotalDistance, total);
            if (newTotalDistance < 0.0) {
                newTotalDistance += total; // Ensure positive result from fmod
            }

            // Clamp just in case fmod result is exactly total due to precision
            double newProgress = std::clamp(newTotalDistance / total, 0.0, 1.0);
            return newProgress;
        }

        // Get the outward normal at a perimeter progress [0,1]. Assumes CCW winding.
        vec2 normalAtProgress(double percent) const {
            vec2 dir = directionAtProgress(percent);
            // Perpendicular gives 90-degree CCW rotation. For CCW shape, this points outwards.
            return dir.perpendicularCCW().normalize();
        }

        // Orbit the shape around an external point
        shape2& orbit(const vec2& orbitCenter, const double& rads) {
            if (points.empty()) return *this;
            vec2 center = centerPoint(); // Use average vertex position as pivot
            vec2 change = center.orbited(orbitCenter, rads) - center; // Calculate translation needed
            // Apply the same translation to all points
            for (vec2& pnt : points) {
                 pnt += change;
            }
            return *this;
        }

        // Returns a new shape orbited around an external point
        shape2 orbited(const vec2& point, const double& rads) const {
            shape2 mod = *this;
            mod.orbit(point, rads); // Call the in-place orbit
            return mod;
        }

        // Checks for overlap with another shape using the Separating Axis Theorem (SAT).
        // IMPORTANT: This method is only guaranteed to be correct for CONVEX shapes.
        // For non-convex shapes, it might return false negatives (no overlap reported when there is one).
        bool overlaps(const shape2& other, double tolerance = 1e-6) const {
            if (points.size() < 3 || other.points.size() < 3) return false; // Degenerate shapes can't overlap

            // Helper lambda to project shape points onto an axis and find min/max projection
            auto projectOntoAxis = [](const std::vector<vec2>& pts, const vec2& axis) {
                if (pts.empty()) return std::pair{infinity, -infinity}; // Handle empty case
                double minProj = pts[0].dot(axis);
                double maxProj = minProj;
                // Can skip first point if handled above
                for (size_t i = 1; i < pts.size(); ++i) {
                    double proj = pts[i].dot(axis);
                    if (proj < minProj) minProj = proj;
                    if (proj > maxProj) maxProj = proj;
                }
                return std::pair{minProj, maxProj};
            };

            // Helper lambda to check for separation along a set of axes
            auto checkAxes = [&](const std::vector<vec2>& axes) {
                for (vec2 axis : axes) {
                    axis.normalize(); // Use normalized axis
                    if(axis.isZero()) continue; // Skip zero axes if they occur

                    auto [minA, maxA] = projectOntoAxis(points, axis);
                    auto [minB, maxB] = projectOntoAxis(other.points, axis);

                    // Check for separation (gap between projections)
                    if (maxA < minB - tolerance || maxB < minA - tolerance) {
                        return false; // Gap found, shapes do not overlap
                    }
                }
                return true; // No gap found on any axis tested
            };

            // Collect all unique edge normals (potential separating axes) for both shapes
            std::vector<vec2> axes;
            axes.reserve(points.size() + other.points.size());
            for (const line2& edge : toLines()) {
                axes.push_back((edge.pnt2 - edge.pnt1).perpendicularCCW());
            }
            for (const line2& edge : other.toLines()) {
                axes.push_back((edge.pnt2 - edge.pnt1).perpendicularCCW());
            }
            // Optional: Remove duplicate/parallel axes for efficiency, though not strictly necessary

            // If no separating axis is found among the edge normals, the shapes overlap (for convex shapes)
            return checkAxes(axes);
        }

        // Check for AABB overlap with a line segment
        bool overlapsAABB(const line2& other) const { // Corrected typo: overLaps -> overlaps
             if (points.empty()) return false;
             auto [minA, maxA] = boundingBox();
             auto [minB, maxB] = other.boundingBox();

             return (minA.x <= maxB.x && maxA.x >= minB.x &&
                     minA.y <= maxB.y && maxA.y >= minB.y);
        }

        // Check for AABB overlap with another shape
        bool overlapsAABB(const shape2& other) const { // Corrected typo: overLaps -> overlaps
             if (points.empty() || other.points.empty()) return false;
             auto [minA, maxA] = boundingBox();
             auto [minB, maxB] = other.boundingBox();

             return (minA.x <= maxB.x && maxA.x >= minB.x &&
                     minA.y <= maxB.y && maxA.y >= minB.y);
        }

        // Get the perimeter progress [0, 1] closest to an external point
        double perimeterProgressFromPoint(const vec2& p, double tolerance = default_Tolerance) const {
            if (points.size() < 3) return 0.0;

            vec2 closest = closestPoint(p); // Find closest point on boundary
            if (std::isnan(closest.x)) return 0.0; // Handle NaN case

            double accumulated = 0.0;
            double total = totalLength();
            if (total < tolerance) return 0.0; // Avoid division by zero

            for (size_t i = 0; i < numSegments(); ++i) {
                const vec2& p1 = points[i];
                const vec2& p2 = points[(i + 1) % points.size()];
                line2 segment{p1, p2};
                double segmentLen = (p2 - p1).length();

                // Check if the overall closest point lies on this segment
                if (segment.pointOnSegment(closest, tolerance)) {
                     if (segmentLen < tolerance) { // Handle zero-length segment
                          // Progress is just the accumulated length up to this point
                     } else {
                          // Calculate progress along this segment
                          double localDist = (closest - p1).length();
                          accumulated += localDist;
                     }
                     break; // Found the segment containing the closest point
                }
                accumulated += segmentLen; // Add full length of this segment
            }
            // Clamp accumulated distance before dividing by total length
            accumulated = std::clamp(accumulated, 0.0, total);
            return accumulated / total;
        }


        // Return a point on the shape's perimeter at the given progress [0, 1]
        vec2 pointAtProgress(double percent, double tolerance = default_Tolerance) const {
            if (points.empty()) return {0.0, 0.0};
            if (points.size() == 1) return points[0];
            // Handle size 2 case? Perimeter doesn't really make sense.
            if (points.size() == 2) return points[0].lerped(points[1], std::clamp(percent, 0.0, 1.0));

            percent = std::clamp(percent, 0.0, 1.0);
            // Handle edge case where percent is exactly 1.0 to avoid potential precision issues
            if (percent >= 1.0 - tolerance) return points[0]; // Return start point for 100% progress on closed loop

            double total = totalLength();
            if (total < tolerance) return points[0]; // Return start point if no length

            double targetDistance = total * percent;
            double accumulated = 0.0;

            for (size_t i = 0; i < numSegments(); ++i) { // Use numSegments() which is points.size()
                const vec2& p1 = points[i];
                const vec2& p2 = points[(i + 1) % points.size()];
                double segmentLength = (p2 - p1).length();

                if (segmentLength < tolerance) continue; // Skip zero-length segments

                // Check if target distance falls within this segment
                if (accumulated + segmentLength >= targetDistance - tolerance) {
                    // Calculate interpolation factor 't' for this segment
                    double remainingDist = targetDistance - accumulated;
                    double localT = remainingDist / segmentLength;
                    return p1.lerped(p2, std::clamp(localT, 0.0, 1.0)); // Clamp t just in case
                }
                accumulated += segmentLength;
            }

            // Fallback: Should ideally be unreachable if percent is clamped and totalLength > 0
            // but return the first point as a safe default for a closed loop.
            return points[0];
        }

        // Return the direction (unit vector) along the perimeter at a given progress [0, 1]
        vec2 directionAtProgress(double percent, double tolerance = default_Tolerance) const {
            if (points.size() < 3) return {1.0, 0.0}; // Default direction for degenerate shapes

            percent = std::clamp(percent, 0.0, 1.0);
             // Handle edge case for 1.0 progress -> direction of the last segment
             if (percent >= 1.0 - tolerance) {
                  return (points[0] - points.back()).normalize();
             }

            double total = totalLength();
             if (total < tolerance) return {1.0, 0.0}; // Default if no length

            double targetDistance = total * percent;
            double accumulated = 0.0;

            for (size_t i = 0; i < numSegments(); ++i) {
                const vec2& p1 = points[i];
                const vec2& p2 = points[(i + 1) % points.size()];
                double segmentLength = (p2 - p1).length();

                 if (segmentLength < tolerance) continue; // Skip zero-length segments

                // Check if target distance falls within this segment
                if (accumulated + segmentLength >= targetDistance - tolerance) {
                    return (p2 - p1).normalize(); // Return normalized direction of this segment
                }
                accumulated += segmentLength;
            }

            // Fallback: Should be unreachable, return direction of last segment
            return (points[0] - points.back()).normalize();
        }


        // Reverse the order of the points (flips winding order)
        shape2& reverseWinding() {
            std::reverse(points.begin(), points.end());
            return *this;
        }

        // Returns a new shape with reversed winding order
        shape2 reversedWinding () const {
            shape2 mod = *this;
            mod.reverseWinding(); // Call the in-place reverse
            return mod;
        }

        // Rotate the shape around its center point (average vertex position)
        void rotate(const double& rads) {
             if (points.size() < 2) return; // Cannot rotate single point or empty shape effectively
             vec2 cent = centerPoint(); // Use average vertex position as pivot
             // Assuming vec2::orbit performs rotation around a pivot
             for (vec2& pnt : points) {
                  pnt.orbit(cent, rads);
             }
        }

        // Returns a new shape rotated around its center point
        shape2 rotated(const double& rads) const {
             shape2 mod = *this;
             mod.rotate(rads); // Call the in-place rotate
             return mod;
        }

        // Checks if the shape self-intersects (edges cross each other). Assumes a simple polygon structure initially.
        bool selfIntersects(double tolerance = default_Tolerance) const { // Use smaller tolerance for intersection checks
            if (points.size() < 4) return false; // Triangle (or less) cannot self-intersect

            size_t n = points.size();
            for (size_t i = 0; i < n; ++i) {
                line2 edge1 = getSegment(i); // Includes wrap-around

                // Check against non-adjacent edges
                for (size_t j = i + 2; j < n; ++j) {
                     // Avoid checking adjacent edges (i, i+1) and the edge connecting back to start (i=0, j=n-1)
                     if (i == 0 && j == n - 1) continue;

                    line2 edge2 = getSegment(j);

                    // Use line2::intersection which checks segment bounds
                    if (!edge1.intersection(edge2, tolerance).isInfinite()) {
                        // Check if intersection is NOT just a shared vertex
                        vec2 intersectPt = edge1.intersection(edge2, tolerance);
                        constexpr double vertex_tolerance_sq = 1e-12; // Smaller tolerance for vertex check
                        bool shared_vertex = false;
                        for(const auto& p : points) {
                             if((intersectPt - p).lengthSquared() < vertex_tolerance_sq) {
                                  shared_vertex = true;
                                  break;
                             }
                        }
                        if (!shared_vertex) {
                           return true; // Found an intersection between non-adjacent edges not at a vertex
                        }
                    }
                }
            }
            return false; // No self-intersections found
        }

        // Simplifies the shape using the Ramer-Douglas-Peucker algorithm in-place.
        // Note: May result in fewer than 3 points if tolerance is large.
        // Note: For closed shapes, this might slightly break the exact closure if
        // start/end points are removed relative to each other. Consider post-processing if needed.
        shape2& simplify(double tolerance = default_Tolerance) {
            if (points.size() < 3) return *this; // Cannot simplify below 3 points

            // RDP works on open polylines. We apply it to points[0]...points[n-1]
            // and then handle the closure.
            std::vector<vec2> simplified_open;
            simplified_open.push_back(points.front()); // Start with the first point

            // Recursive helper lambda for RDP
            auto recursiveSimplify = [&](auto&& self, size_t startIdx, size_t endIdx) -> void {
                if (endIdx <= startIdx + 1) return; // Base case: segment is just two points

                const vec2& start = points[startIdx];
                const vec2& end = points[endIdx];
                line2 segment_line(start, end); // Line segment for distance calculation

                double maxDist = 0.0;
                size_t index = startIdx; // Index of the point furthest from the segment

                // Find the point furthest from the line segment [start, end]
                for (size_t i = startIdx + 1; i < endIdx; ++i) {
                    // Use distanceToPoint (perpendicular distance to infinite line) for RDP
                    double dist = segment_line.distanceToPoint(points[i]);
                    if (dist > maxDist) {
                        index = i;
                        maxDist = dist;
                    }
                }

                // If the max distance is greater than tolerance, recursively simplify
                if (maxDist > tolerance) {
                    self(self, startIdx, index); // Simplify first part
                    simplified_open.push_back(points[index]); // Add the splitting point
                    self(self, index, endIdx);   // Simplify second part
                }
                // Otherwise, the segment [start, end] approximates this section well enough
            };

            // Run RDP on the open sequence of points
            recursiveSimplify(recursiveSimplify, 0, points.size() - 1);
            simplified_open.push_back(points.back()); // Add the last point of the original sequence

            // Now handle the closure. Check if the new start/end points are too close.
            if (simplified_open.size() >= 2 && (simplified_open.back() - simplified_open.front()).lengthSquared() < tolerance * tolerance) {
                 simplified_open.pop_back(); // Remove duplicate last point if it's same as first
            }

            // Ensure we still have at least 3 points if possible
            if (simplified_open.size() < 3 && points.size() >=3) {
                 // Simplification reduced below 3 points, maybe return original or handle differently?
                 // For now, assign the simplified (potentially degenerate) points.
                 points = std::move(simplified_open);
            } else {
                 points = std::move(simplified_open);
            }

            return *this;
        }

        // Returns a new, simplified shape.
        shape2 simplified(double tolerance = default_Tolerance) const { // Corrected typo: tollerance -> tolerance
            shape2 mod = *this;
            return mod.simplify(tolerance); // Call the in-place simplify
        }

        // Uniformly scale the shape around a pivot point (defaults to origin)
        shape2& scale(double factor, vec2 pivot = {0.0, 0.0}) {
            for (vec2& pnt : points) {
                // Vector from pivot to point, scale it, add back pivot
                pnt = pivot + (pnt - pivot) * factor;
            }
            return *this;
        }

        // Returns a new shape scaled around a pivot point
        shape2 scaled(double factor, vec2 pivot = {0.0, 0.0}) const {
            shape2 mod = *this;
            mod.scale(factor, pivot); // Call the in-place scale
            return mod;
        }

        // Get lengths of all segments (edges)
        std::vector<double> segmentLengths() const {
            std::vector<double> lengths;
            if (points.size() < 3) return lengths; // Need 3 points for valid segments
            lengths.reserve(points.size());
            for (size_t i = 0; i < points.size(); ++i) {
                lengths.push_back(getSegment(i).length()); // getSegment handles wrap-around
            }
            return lengths;
        }

        // Splits the shape's perimeter into two open paths at a given progress [0, 1]
        std::pair<std::vector<vec2>, std::vector<vec2>> splitAtProgress(double percent, double tolerance = default_Tolerance) const {
            std::vector<vec2> path1, path2;
            if (points.size() < 3) return {path1, path2}; // Cannot split degenerate shape

            percent = std::clamp(percent, 0.0, 1.0);
            double totalLen = totalLength();
             if (totalLen < tolerance) return {points, {}}; // Handle zero length case

            double targetDistance = totalLen * percent;
            double accumulated = 0.0;
            vec2 splitPoint = points[0]; // Default split point if percent is 0 or 1

            size_t splitSegmentIndex = points.size(); // Index of segment where split occurs

            for (size_t i = 0; i < numSegments(); ++i) { // numSegments() == points.size()
                const vec2& p1 = points[i];
                const vec2& p2 = points[(i + 1) % points.size()];
                double segmentLength = (p2 - p1).length();

                if (segmentLength < tolerance) continue; // Skip zero-length segments

                // Check if target distance falls within this segment
                if (accumulated + segmentLength >= targetDistance - tolerance) {
                    double remainingDist = targetDistance - accumulated;
                    double localT = remainingDist / segmentLength;
                    splitPoint = p1.lerped(p2, std::clamp(localT, 0.0, 1.0));
                    splitSegmentIndex = i;
                    break; // Found the split point and segment
                }
                accumulated += segmentLength;
            }

            // Build path1: from start up to split point
            path1.push_back(points[0]); // Always start with the first point
            for(size_t i = 0; i <= splitSegmentIndex; ++i) {
                 path1.push_back(points[(i + 1) % points.size()]);
            }
            // Replace the last point added (which is p2 of the split segment) with the actual split point
            if (!path1.empty()) path1.back() = splitPoint;
            else path1.push_back(splitPoint); // Should not happen if size >= 3


            // Build path2: from split point to end (which wraps back to start)
            path2.push_back(splitPoint);
            for(size_t i = splitSegmentIndex + 1; i < points.size(); ++i) {
                 path2.push_back(points[i]);
            }
             // Add the first point to close the loop conceptually for path2
             if (!path2.empty() && path2.back() != points[0]) { // Avoid duplicate if split was exactly at vertex 0
                  path2.push_back(points[0]);
             }


            return {path1, path2};
        }


        // Get all line segments (edges) forming the shape boundary
        std::vector<line2> toLines() const {
            std::vector<line2> lines;
            if (points.size() < 3) return lines; // Need 3 points for valid segments
            lines.reserve(points.size());
            for (size_t i = 0; i < points.size(); ++i) {
                lines.push_back(getSegment(i)); // getSegment handles wrapping
            }
            return lines;
        }

        // Calculate the perimeter (total length of the boundary)
        double totalLength() const {
            if (points.size() < 3) return 0.0;
            double total = 0.0;
             // Sum lengths from getSegment for accuracy, includes closing segment
             for (size_t i = 0; i < points.size(); ++i) {
                  total += getSegment(i).length();
             }
             // Alternative using std::accumulate if segmentLengths() is available and efficient
             // auto lengths = segmentLengths();
             // return std::accumulate(lengths.begin(), lengths.end(), 0.0);
            return total;
        }

        // Translate the shape by a given vector offset
        void translate(const vec2& amnt) {
             for (vec2& point: points) {
                  point += amnt;
             }
        }

        // Returns a new shape translated by a given vector offset
        shape2 translated(const vec2& amnt) const {
            shape2 mod = *this;
            mod.translate(amnt); // Call the in-place translate
            return mod;
        }
    }; // End struct shape2
}; // End namespace vecSys