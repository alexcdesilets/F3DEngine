#pragma once

#include <vector>
#include <array>
#include <limits>
#include <numbers>
#include "vec3.h"
#include "vec2.h"
#include "line3.h"
#include "pline3.h"
#include "quadPLane3.h"
#include "triangleStrip3.h"

namespace vecSys {

    struct Mesh3 {
        enum class TriangleGenerationMode { Simple, Grid, TriangleStrip};
        struct collisionInfo {bool collisionFound = false; vec3 point = {INFINITY, INFINITY, INFINITY}, normal = {INFINITY, INFINITY, INFINITY}; vec2 UV = {INFINITY, INFINITY}; double rayDistanceSquared = INFINITY; int parentIndex = -1, triangleIndex = -1;};
        struct generationDetails {TriangleGenerationMode type; int uDivs, vDivs;};

        // --- Data Members ---
        std::vector<vec3> vertices;
        std::vector<std::array<int, 3>> triangles; // Triplets of indices into vertices
        std::vector<vec2> uvs; // Optional, 1-to-1 with vertices
        generationDetails lastGen;
        
        // --- Initialisers ---
        
        void init(const std::vector<vec3>& pts, TriangleGenerationMode mode, int uDivs = 0, int vDivs = 0) {
            lastGen = {mode, uDivs, vDivs};
            vertices = pts;
            switch (mode) {
                case TriangleGenerationMode::Simple:
                    autoGenerateTrianglesSimple();
                    autoGenerateUVsSimple();
                    break;
                case TriangleGenerationMode::Grid:
                    autoGenerateTrianglesFromGrid(uDivs, vDivs);
                    autoGenerateUVsByBoundingBox();
                    break;
                case TriangleGenerationMode::TriangleStrip:
                    autoGenerateTrianglesFromTriangleStrip();
                    autoGenerateUVsByBestFitPlane();
                    break;
            }
        }

        void repair(TriangleGenerationMode mode, int uDivs = 0, int vDivs = 0) {
            lastGen = {mode, uDivs, vDivs};
            switch (mode) {
                case TriangleGenerationMode::Simple:
                    autoGenerateTrianglesSimple();
                    autoGenerateUVsSimple();
                    break;
                case TriangleGenerationMode::Grid:
                    autoGenerateTrianglesFromGrid(uDivs, vDivs);
                    autoGenerateUVsByBoundingBox();
                    break;
                case TriangleGenerationMode::TriangleStrip:
                    autoGenerateTrianglesFromTriangleStrip();
                    autoGenerateUVsByBestFitPlane();
                    break;
            }
        }

        void repairUVsOnly() {
            switch (lastGen.type) {
                case TriangleGenerationMode::Simple:
                    autoGenerateUVsSimple();
                    break;
                case TriangleGenerationMode::Grid:
                    autoGenerateUVsByBoundingBox();
                    break;
                case TriangleGenerationMode::TriangleStrip:
                    autoGenerateUVsByBestFitPlane();
                    break;
            }
        }

        void autoGenerateTrianglesFromGrid(int uDivs, int vDivs) {
            triangles.clear();
            if (uDivs <= 0 || vDivs <= 0) return;
        
            for (int v = 0; v < vDivs; ++v) {
                for (int u = 0; u < uDivs; ++u) {
                    int i0 = v * (uDivs + 1) + u;
                    int i1 = i0 + 1;
                    int i2 = i0 + (uDivs + 1);
                    int i3 = i2 + 1;
        
                    // First triangle
                    triangles.push_back({i0, i2, i1});
                    // Second triangle
                    triangles.push_back({i1, i2, i3});
                }
            }
        }

        void autoGenerateTrianglesFromTriangleStrip() {
            triangles.clear();
            if (vertices.size() < 3) return;
        
            for (size_t i = 0; i + 2 < vertices.size(); ++i) {
                if (i % 2 == 0) {
                    triangles.push_back({static_cast<int>(i), static_cast<int>(i+1), static_cast<int>(i+2)});
                } else {
                    triangles.push_back({static_cast<int>(i+1), static_cast<int>(i), static_cast<int>(i+2)});
                }
            }
        }

        void autoGenerateTrianglesSimple() {
            triangles.clear();
            if (vertices.size() < 3) return;
        
            if (vertices.size() % 3 != 0) {
                // Optionally: throw warning or silently continue
                // (could choose to only process full groups of 3)
            }
        
            for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
                triangles.push_back({static_cast<int>(i), static_cast<int>(i+1), static_cast<int>(i+2)});
            }
        }

        void autoGenerateUVsByBestFitPlane() {
            uvs.clear();
            if (vertices.size() < 3) return;
        
            // Step 1: Compute centroid
            vec3 centroid = {0.0, 0.0, 0.0};
            for (const auto& v : vertices) {
                centroid += v;
            }
            centroid /= static_cast<double>(vertices.size());
        
            // Step 2: Compute covariance matrix
            double cov_xx = 0.0, cov_xy = 0.0, cov_xz = 0.0;
            double cov_yy = 0.0, cov_yz = 0.0, cov_zz = 0.0;
        
            for (const auto& v : vertices) {
                vec3 p = v - centroid;
                cov_xx += p.x * p.x;
                cov_xy += p.x * p.y;
                cov_xz += p.x * p.z;
                cov_yy += p.y * p.y;
                cov_yz += p.y * p.z;
                cov_zz += p.z * p.z;
            }
        
            // Step 3: Choose the axis of smallest variance as the normal
            // (fast, approximate PCA: avoid full eigen decomposition)
            double varX = cov_xx;
            double varY = cov_yy;
            double varZ = cov_zz;
        
            vec3 normal;
            if (varX <= varY && varX <= varZ) {
                normal = {1.0, 0.0, 0.0}; // x-axis has least spread
            } else if (varY <= varX && varY <= varZ) {
                normal = {0.0, 1.0, 0.0}; // y-axis has least spread
            } else {
                normal = {0.0, 0.0, 1.0}; // z-axis has least spread
            }
        
            // Step 4: Build UV axes
            // Pick an arbitrary vector not parallel to the normal
            vec3 up = (std::abs(normal.z) < 0.9) ? vec3{0,0,1} : vec3{0,1,0};
            vec3 uAxis = normal.cross(up).normalize();
            vec3 vAxis = normal.cross(uAxis).normalize();
        
            // Step 5: Project all vertices to UV space
            uvs.reserve(vertices.size());
            for (const auto& v : vertices) {
                vec3 rel = v - centroid;
                double u = rel.dot(uAxis);
                double vCoord = rel.dot(vAxis);
                uvs.emplace_back(u, vCoord);
            }


        
            // Step 6 (optional): Normalize UVs to 0-1
            vec2 minUV = uvs[0];
            vec2 maxUV = uvs[0];
            for (const auto& uv : uvs) {
                minUV.x = std::min(minUV.x, uv.x);
                minUV.y = std::min(minUV.y, uv.y);
                maxUV.x = std::max(maxUV.x, uv.x);
                maxUV.y = std::max(maxUV.y, uv.y);
            }
        
            vec2 dims = maxUV - minUV;
            if (dims.x < default_Tolerance) dims.x = 1.0;
            if (dims.y < default_Tolerance) dims.y = 1.0;
        
            for (auto& uv : uvs) {
                uv.x = (uv.x - minUV.x) / dims.x;
                uv.y = (uv.y - minUV.y) / dims.y;
            }
        }

        void autoGenerateUVsByBoundingBox() {
            uvs.clear();
            if (vertices.empty()) return;
        
            auto [minPt, maxPt] = boundingBox();
            vec3 dims = maxPt - minPt;
        
            // Avoid division by zero
            if (dims.x < default_Tolerance) dims.x = 1.0;
            if (dims.y < default_Tolerance) dims.y = 1.0;
            if (dims.z < default_Tolerance) dims.z = 1.0;
        
            uvs.reserve(vertices.size());
            for (const auto& v : vertices) {
                double u = (v.x - minPt.x) / dims.x;
                double vCoord = (v.z - minPt.z) / dims.z; // Default projection: X-Z plane
                uvs.emplace_back(u, vCoord);
            }
        }

        void autoGenerateUVsSimple() {
            uvs.clear();
            if (vertices.empty()) return;
        
            // Simple sequential UVs based on index (not perfect but consistent)
            uvs.reserve(vertices.size());
            for (size_t i = 0; i < vertices.size(); ++i) {
                double u = static_cast<double>(i % 10) / 9.0; // 0.0 -> 1.0 range horizontally
                double v = static_cast<double>(i / 10) / 9.0; // 0.0 -> 1.0 vertically
                uvs.emplace_back(u, v);
            }
        }

        void initGrid(int uDivs, int vDivs, double spacing = 1.0) {
            vertices.clear();
            triangles.clear();
            uvs.clear();
        
            if (uDivs <= 0 || vDivs <= 0) return;
        
            for (int v = 0; v <= vDivs; ++v) {
                for (int u = 0; u <= uDivs; ++u) {
                    vertices.emplace_back(u * spacing, 0.0, v * spacing);
                    uvs.emplace_back(
                        static_cast<double>(u) / static_cast<double>(uDivs),
                        static_cast<double>(v) / static_cast<double>(vDivs)
                    );
                }
            }
        
            // Create triangles
            for (int v = 0; v < vDivs; ++v) {
                for (int u = 0; u < uDivs; ++u) {
                    int i0 = v * (uDivs + 1) + u;
                    int i1 = i0 + 1;
                    int i2 = i0 + (uDivs + 1);
                    int i3 = i2 + 1;
        
                    triangles.push_back({i0, i2, i1}); // First triangle
                    triangles.push_back({i1, i2, i3}); // Second triangle
                }
            }
        }

        // --- Basic Queries ---

        std::pair<vec3, vec3> boundingBox() const {
            if (vertices.empty()) return {vec3(0,0,0), vec3(0,0,0)};
            vec3 minPt = vertices[0];
            vec3 maxPt = vertices[0];
            for (const auto& v : vertices) {
                minPt = minPt.minComp(v);
                maxPt = maxPt.maxComp(v);
            }
            return {minPt, maxPt};
        }

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
        
        bool overlapsAABB(const Mesh3& other) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = other.boundingBox();
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }
        
        // Overlap against an explicit {min, max} AABB
        bool overlapsAABB(const std::pair<vec3, vec3>& box) const {
            auto [minA, maxA] = boundingBox();
            auto [minB, maxB] = box;
            return (minA.x <= maxB.x && maxA.x >= minB.x &&
                    minA.y <= maxB.y && maxA.y >= minB.y &&
                    minA.z <= maxB.z && maxA.z >= minB.z);
        }

        vec3 centerPoint() const {
            if (vertices.empty()) return vec3(0,0,0);
            vec3 sum(0,0,0);
            for (const auto& v : vertices) {
                sum += v;
            }
            return sum / static_cast<double>(vertices.size());
        }

        size_t numVertices() const { return vertices.size(); }
        size_t numTriangles() const { return triangles.size(); }

        // --- Transformations ---

        Mesh3& translate(const vec3& offset) {
            for (vec3& v : vertices) {
                v += offset;
            }
            return *this;
        }

        Mesh3 translated(const vec3& offset) const {
            Mesh3 mod = *this;
            return mod.translate(offset);
        }

        Mesh3& orbit(const vec3& point, const vec3& axis, const double& rads) {
            vec3 cent = centerPoint();
            vec3 mod = cent.orbited(point, axis, rads) - cent;
            for (vec3& v : vertices) {
                v += mod;
            }
            return *this;
        }

        Mesh3 orbited(const vec3& point, const vec3& axis, const double& rads) {
            Mesh3 mod = *this;
            return mod.orbit(point, axis, rads);
        }

        Mesh3& scale(double factor, const vec3& pivot = vec3(0,0,0)) {
            for (vec3& v : vertices) {
                v = pivot + (v - pivot) * factor;
            }
            repairUVsOnly();
            return *this;
        }

        Mesh3 scaled(double factor, const vec3& pivot = vec3(0,0,0)) const {
            Mesh3 mod = *this;
            return mod.scale(factor, pivot);
        }

        Mesh3& rotate(const vec3& axis, double rads) {
            vec3 center = centerPoint();
            for (vec3& v : vertices) {
                v.orbit(center, axis, rads);
            }
            repairUVsOnly();
            return *this;
        }

        Mesh3 rotated(const vec3& axis, double rads) const {
            Mesh3 mod = *this;
            return mod.rotate(axis, rads);
        }

        // --- Surface Queries ---

        collisionInfo intersectLine(const line3& ray, bool localUV = false, double tolerance = default_Tolerance) const {
            collisionInfo info;
        
            double closestT = INFINITY;
            int closestTri = -1;
            double bestU = 0.0, bestV = 0.0;
        
            for (size_t i = 0; i < triangles.size(); ++i) {
                const auto& tri = triangles[i];
                const vec3& p0 = vertices[tri[0]];
                const vec3& p1 = vertices[tri[1]];
                const vec3& p2 = vertices[tri[2]];
        
                double t, u, v;
                if (TriangleStrip3::rayTriangleIntersectMT(ray.pnt1, ray.direction(), p0, p1, p2, t, u, v, tolerance)) {
                    if (t > -tolerance && t < closestT) {
                        closestT = t;
                        closestTri = static_cast<int>(i);
                        bestU = u;
                        bestV = v;
                    }
                }
            }
        
            if (closestTri != -1) {
                info.collisionFound = true;
                vec3 hitPoint = ray.pnt1 + ray.direction().normalize() * closestT;
                info.point = hitPoint;
                info.rayDistanceSquared = (ray.pnt1 - hitPoint).lengthSquared();
                info.normal = computeFaceNormals()[closestTri];
                info.triangleIndex = closestTri;
        
                if (localUV) {
                    info.UV = vec2(bestU, bestV); // Raw barycentric UV
                } else {
                    info.UV = barycentricUV(closestTri, bestU, bestV); // Global interpolated UV
                }
            }
        
            return info;
        }

        collisionInfo intersectSegment(const line3& segment, double tolerance = default_Tolerance, bool localUV = false) const {
            collisionInfo result;
        
            double segmentLen = (segment.pnt2 - segment.pnt1).length();
            if (segmentLen < tolerance) {
                return result; // Degenerate segment
            }
        
            vec3 rayDir = (segment.pnt2 - segment.pnt1) / segmentLen;
        
            double closestT = INFINITY;
            vec3 hitPoint;
            int bestTriIndex = -1;
            double bestU = INFINITY, bestV = INFINITY;
        
            for (size_t i = 0; i < triangles.size(); ++i) {
                const auto& tri = triangles[i];
                const vec3& p0 = vertices[tri[0]];
                const vec3& p1 = vertices[tri[1]];
                const vec3& p2 = vertices[tri[2]];
        
                double t, u, v;
                if (TriangleStrip3::rayTriangleIntersectMT(segment.pnt1, rayDir, p0, p1, p2, t, u, v, tolerance)) {
                    if (t > -tolerance && t <= segmentLen + tolerance && t < closestT) {
                        closestT = t;
                        hitPoint = segment.pnt1 + rayDir * t;
                        bestTriIndex = static_cast<int>(i);
                        bestU = u;
                        bestV = v;
                    }
                }
            }
        
            if (bestTriIndex != -1) {
                result.collisionFound = true;
                result.point = hitPoint;
                result.triangleIndex = bestTriIndex;
                result.rayDistanceSquared = (segment.pnt1 - hitPoint).lengthSquared();
        
                const auto& tri = triangles[bestTriIndex];
                const vec3& p0 = vertices[tri[0]];
                const vec3& p1 = vertices[tri[1]];
                const vec3& p2 = vertices[tri[2]];
                result.normal = (p1 - p0).cross(p2 - p0).normalize();
        
                if (localUV) {
                    result.UV = vec2(bestU, bestV); // Local UV: pure barycentric
                } else {
                    result.UV = barycentricUV(bestTriIndex, bestU, bestV); // Global interpolated UV
                }
            }
        
            return result;
        }

        // Optional: Generate surface normals
        std::vector<vec3> computeFaceNormals() const {
            std::vector<vec3> normals;
            normals.reserve(triangles.size());
            for (const auto& tri : triangles) {
                const vec3& p0 = vertices[tri[0]];
                const vec3& p1 = vertices[tri[1]];
                const vec3& p2 = vertices[tri[2]];
                vec3 n = (p1 - p0).cross(p2 - p0);
                normals.push_back(n.isZero() ? vec3(0,0,0) : n.normalize());
            }
            return normals;
        }

        vec2 barycentricUV(int triIndex, double u, double v) const {
            if (triIndex < 0 || triIndex >= static_cast<int>(triangles.size()) || uvs.size() != vertices.size())
                return { INFINITY, INFINITY };
        
            const auto& tri = triangles[triIndex];
            const vec2& uv0 = uvs[tri[0]];
            const vec2& uv1 = uvs[tri[1]];
            const vec2& uv2 = uvs[tri[2]];
        
            double w = 1.0 - u - v;
            return uv0 * w + uv1 * u + uv2 * v;
        }

    };

} // namespace vecSys