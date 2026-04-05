#pragma once
#include "base.h"
#include "vec3.h"
#include "vec2.h"
#include "pline3.h"
#include "line3.h"
#include "quadPLane3.h"
#include "triangleStrip3.h"

namespace vecSys {

    struct BezierSurface3 {
        struct collisionInfo {bool collisionFound = false; vec3 point = {infinity, infinity, infinity}, normal = {infinity, infinity, infinity}; vec2 UV = {infinity, infinity}; double rayDistanceSquared = infinity; int parentIndex = -1;};

        // 2D grid of control points [v][u]
        std::vector<std::vector<vec3>> controlPoints;

        // Constructors
        BezierSurface3() = default;
        BezierSurface3(const std::vector<std::vector<vec3>>& pts) : controlPoints(pts) {}

        size_t degreeU() const { return (controlPoints.empty()) ? 0 : (controlPoints[0].size() - 1); }
        size_t degreeV() const { return (controlPoints.size()  - 1); }

        bool valid() const {
            if (controlPoints.empty()) return false;
            size_t uSize = controlPoints[0].size();
            for (const auto& row : controlPoints) {
                if (row.size() != uSize) return false; // Must be a proper grid
            }
            return true;
        }

        // Evaluate point at (u,v) in [0,1] x [0,1]
        vec3 evaluate(double u, double v) const {
            if (!valid()) return { infinity, infinity, infinity };

            u = std::clamp(u, 0.0, 1.0);
            v = std::clamp(v, 0.0, 1.0);

            std::vector<vec3> tempRow;
            tempRow.reserve(controlPoints.size());

            // First interpolate along U for each V
            for (const auto& row : controlPoints) {
                tempRow.push_back(deCasteljau(row, u));
            }

            // Then interpolate along V
            return deCasteljau(tempRow, v);
        }

        // Normal at (u,v) via cross of partial derivatives
        vec3 normalAt(double u, double v, double tolerance = default_Tolerance) const {
            // Finite difference
            const double du = 1e-5;
            const double dv = 1e-5;

            vec3 p  = evaluate(u, v);
            vec3 pu = evaluate(std::clamp(u + du, 0.0, 1.0), v) - p;
            vec3 pv = evaluate(u, std::clamp(v + dv, 0.0, 1.0)) - p;

            vec3 n = pu.cross(pv);

            if (n.lengthSquared() < tolerance * tolerance) {
                return { 0.0, 0.0, 0.0 };
            }

            return n.normalize();
        }

        BezierSurface3& translate(const vec3& offset) {
            for (auto& row : controlPoints) {
                for (auto& p : row) {
                    p += offset;
                }
            }
            return *this;
        }

        BezierSurface3& scale(const vec3& pivot, double factor) {
            for (auto& row : controlPoints) {
                for (auto& p : row) {
                    p = pivot + (p - pivot) * factor;
                }
            }
            return *this;
        }

        BezierSurface3& deformPivot(const vec3& pivot, const vec3& axis, double rads) {
            for (auto& row : controlPoints) {
                for (auto& p : row) {
                    p.orbit(pivot, axis, rads);
                }
            }
            return *this;
        }

        BezierSurface3& rotate(const vec3& axis, double rads) {
            vec3 center = centerPoint();
            return deformPivot(center, axis, rads);
        }

        BezierSurface3& orbit(const vec3& orbitCenter, const vec3& axis, double rads) {
            vec3 center = centerPoint();
            vec3 translation = center.orbited(orbitCenter, axis, rads) - center;
            return translate(translation);
        }

        // --- Transformations (return new modified copy) ---

        BezierSurface3 translated(const vec3& offset) const {
            BezierSurface3 mod = *this;
            mod.translate(offset);
            return mod;
        }

        BezierSurface3 scaled(const vec3& pivot, double factor) const {
            BezierSurface3 mod = *this;
            mod.scale(pivot, factor);
            return mod;
        }

        BezierSurface3 deformedPivot(const vec3& pivot, const vec3& axis, double rads) const {
            BezierSurface3 mod = *this;
            mod.deformPivot(pivot, axis, rads);
            return mod;
        }

        BezierSurface3 rotated(const vec3& axis, double rads) const {
            BezierSurface3 mod = *this;
            mod.rotate(axis, rads);
            return mod;
        }

        BezierSurface3 orbited(const vec3& orbitCenter, const vec3& axis, double rads) const {
            BezierSurface3 mod = *this;
            mod.orbit(orbitCenter, axis, rads);
            return mod;
        }

        // --- Utility ---

        std::pair<vec3, vec3> boundingBox() const {
            if (controlPoints.empty() || controlPoints[0].empty()) {
                return { {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0} };
            }
        
            vec3 minPt = controlPoints[0][0];
            vec3 maxPt = controlPoints[0][0];
        
            for (const auto& row : controlPoints) {
                for (const auto& p : row) {
                    minPt = minPt.minComp(p);
                    maxPt = maxPt.maxComp(p);
                }
            }
        
            return { minPt, maxPt };
        }

        vec3 centerPoint() const {
            if (controlPoints.empty() || controlPoints[0].empty()) return {0.0, 0.0, 0.0};

            vec3 sum = {0.0, 0.0, 0.0};
            size_t count = 0;
            for (const auto& row : controlPoints) {
                for (const auto& p : row) {
                    sum += p;
                    ++count;
                }
            }
            return (count > 0) ? (sum / static_cast<double>(count)) : vec3{0.0, 0.0, 0.0};
        }
        

        // de Casteljau for a 1D vector of vec3
        vec3 deCasteljau(const std::vector<vec3>& pts, double t) const {
            if (pts.empty()) return { infinity, infinity, infinity };
            if (pts.size() == 1) return pts[0];

            std::vector<vec3> current = pts;
            while (current.size() > 1) {
                std::vector<vec3> next;
                next.reserve(current.size() - 1);
                for (size_t i = 0; i < current.size() - 1; ++i) {
                    next.push_back(current[i].lerped(current[i+1], t));
                }
                current = std::move(next);
            }
            return current[0];
        }

        collisionInfo intersectLine(const line3& ray, int uDivs = 10, int vDivs = 10, bool stretchedUV = true, double tolerance = default_Tolerance) const {
            collisionInfo result;
        
            TriangleStrip3 strip = toTriangleStrip3(uDivs, vDivs);
        
            auto hitInfo = strip.intersectStripForUV(ray, tolerance);
        
            if (hitInfo.hit) {
                result.collisionFound = true;
                result.point = hitInfo.point;
                result.rayDistanceSquared = (ray.pnt1 - hitInfo.point).lengthSquared();
                result.normal = strip.normal(static_cast<int>(hitInfo.triangleIndex));
        
                if (stretchedUV) {
                    result.UV = strip.calculatePlanarUV_Stretched(result.point, tolerance);
                } else {
                    result.UV = strip.calculateSurfaceUV(hitInfo.triangleIndex, hitInfo.u, hitInfo.v, tolerance);
                }
            }
        
            return result;
        }
        
        collisionInfo intersectSegment(const line3& segment, int uDivs = 10, int vDivs = 10, bool stretchedUV = true, double tolerance = default_Tolerance) const {
            collisionInfo result;
        
            TriangleStrip3 strip = toTriangleStrip3(uDivs, vDivs);
        
            auto hitInfo = strip.intersectSegmentForUV(segment, tolerance);
        
            if (hitInfo.hit) {
                result.collisionFound = true;
                result.point = hitInfo.point;
                result.rayDistanceSquared = (segment.pnt1 - hitInfo.point).lengthSquared();
                result.normal = strip.normal(static_cast<int>(hitInfo.triangleIndex));
        
                if (stretchedUV) {
                    result.UV = strip.calculatePlanarUV_Stretched(result.point, tolerance);
                } else {
                    result.UV = strip.calculateSurfaceUV(hitInfo.triangleIndex, hitInfo.u, hitInfo.v, tolerance);
                }
            }
        
            return result;
        }

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

        bool overlapsAABB(const TriangleStrip3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }

        bool overlapsAABB(const BezierSurface3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }

        bool overlapsAABB(const std::pair<vec3, vec3>& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other;
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }

        std::vector<std::vector<vec3>> sampleGrid(int uDivs, int vDivs) const {
            std::vector<std::vector<vec3>> grid;
            
            if (uDivs <= 0 || vDivs <= 0 || !valid()) {
                return grid; // Return empty grid for invalid cases
            }
        
            grid.resize(vDivs + 1); // vDivs + 1 rows
            for (int v = 0; v <= vDivs; ++v) {
                double vParam = static_cast<double>(v) / static_cast<double>(vDivs);
                grid[v].reserve(uDivs + 1);
                for (int u = 0; u <= uDivs; ++u) {
                    double uParam = static_cast<double>(u) / static_cast<double>(uDivs);
                    grid[v].push_back(evaluate(uParam, vParam));
                }
            }
            return grid;
        }

        TriangleStrip3 toTriangleStrip3(int uDivs, int vDivs) const {
            TriangleStrip3 strip;
        
            if (uDivs <= 0 || vDivs <= 0 || !valid()) {
                return strip; // Return empty if invalid
            }
        
            auto grid = sampleGrid(uDivs, vDivs);
        
            // Traverse the grid and generate the triangle strip
            for (int v = 0; v < vDivs; ++v) {
                if (v % 2 == 0) {
                    // Left to right for even rows
                    for (int u = 0; u <= uDivs; ++u) {
                        strip.points.push_back(grid[v][u]);
                        strip.points.push_back(grid[v+1][u]);
                    }
                } else {
                    // Right to left for odd rows (to maintain strip continuity)
                    for (int u = uDivs; u >= 0; --u) {
                        strip.points.push_back(grid[v][u]);
                        strip.points.push_back(grid[v+1][u]);
                    }
                }
            }
        
            return strip;
        }

    };

} // namespace vecSys