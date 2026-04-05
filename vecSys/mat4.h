#pragma once

#include "base.h"
#include "vec2.h"       // For vec2 transformations
#include "vec3.h"
#include "vec4.h"
#include "quaternion.h" // For creating rotation matrices from quaternions and converting to quaternions


namespace vecSys {

    struct mat4 {
        // Matrix elements stored in row-major order:
        // m[0] m[1] m[2] m[3]
        // m[4] m[5] m[6] m[7]
        // m[8] m[9] m[10] m[11]
        // m[12] m[13] m[14] m[15]
        std::array<double, 16> m;

        // --- Constructors ---
        mat4() : m{ // Default constructor: Initializes to identity matrix
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        } {}

        mat4(double m00, double m01, double m02, double m03,
             double m10, double m11, double m12, double m13,
             double m20, double m21, double m22, double m23,
             double m30, double m31, double m32, double m33)
            : m{m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33} {}

        explicit mat4(const std::array<double, 16>& values) : m(values) {}

        explicit mat4(const std::vector<double>& values) {
            if (values.size() >= 16) {
                std::copy_n(values.begin(), 16, m.begin());
            } else {
                *this = identity();
            }
        }
        
        mat4(const vec4& row0, const vec4& row1, const vec4& row2, const vec4& row3)
            : m{row0.x, row0.y, row0.z, row0.w,
                row1.x, row1.y, row1.z, row1.w,
                row2.x, row2.y, row2.z, row2.w,
                row3.x, row3.y, row3.z, row3.w} {}

        // Construct from 4 column vectors
        static mat4 fromColumnVectors(const vec4& col0, const vec4& col1, const vec4& col2, const vec4& col3) {
            return mat4(
                col0.x, col1.x, col2.x, col3.x,
                col0.y, col1.y, col2.y, col3.y,
                col0.z, col1.z, col2.z, col3.z,
                col0.w, col1.w, col2.w, col3.w
            );
        }

        mat4(const mat4& other) = default;
        mat4(mat4&& other) noexcept = default;
        mat4& operator=(const mat4& other) = default;
        mat4& operator=(mat4&& other) noexcept = default;

        // --- Static Factory Methods ---
        static constexpr mat4 identity() {
            return mat4(
                1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
        }
        static constexpr mat4 zero() {
            return mat4(
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        }
        static mat4 createTranslation(const vec3& t) {
            return mat4(
                1.0, 0.0, 0.0, t.x,
                0.0, 1.0, 0.0, t.y,
                0.0, 0.0, 1.0, t.z,
                0.0, 0.0, 0.0, 1.0);
        }
        static mat4 createTranslation(double tx, double ty, double tz) { return createTranslation(vec3(tx, ty, tz)); }
        static mat4 createScale(const vec3& s) {
            return mat4(
                s.x, 0.0, 0.0, 0.0,
                0.0, s.y, 0.0, 0.0,
                0.0, 0.0, s.z, 0.0,
                0.0, 0.0, 0.0, 1.0);
        }
        static mat4 createScale(double sx, double sy, double sz) { return createScale(vec3(sx, sy, sz)); }
        static mat4 createScale(double s) { return createScale(vec3(s, s, s)); }
        static mat4 createRotationX(double radians) {
            double c = std::cos(radians), s = std::sin(radians);
            return mat4(1.0,0.0,0.0,0.0, 0.0,c,-s,0.0, 0.0,s,c,0.0, 0.0,0.0,0.0,1.0);
        }
        static mat4 createRotationY(double radians) {
            double c = std::cos(radians), s = std::sin(radians);
            return mat4(c,0.0,s,0.0, 0.0,1.0,0.0,0.0, -s,0.0,c,0.0, 0.0,0.0,0.0,1.0);
        }
        static mat4 createRotationZ(double radians) {
            double c = std::cos(radians), s = std::sin(radians);
            return mat4(c,-s,0.0,0.0, s,c,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0);
        }
        static mat4 createRotationAxisAngle(vec3 axis, double radians) {
            axis.normalize();
            double c=std::cos(radians), s=std::sin(radians), t=1.0-c;
            double x=axis.x, y=axis.y, z=axis.z;
            return mat4(
                t*x*x+c,   t*x*y-s*z, t*x*z+s*y, 0.0,
                t*x*y+s*z, t*y*y+c,   t*y*z-s*x, 0.0,
                t*x*z-s*y, t*y*z+s*x, t*z*z+c,   0.0,
                0.0,       0.0,       0.0,       1.0);
        }
        static mat4 createFromQuaternion(quaternion q) {
            q.normalize();
            double x=q.x, y=q.y, z=q.z, w=q.w;
            double xx=x*x, yy=y*y, zz=z*z;
            double xy=x*y, xz=x*z, yz=y*z;
            double wx=w*x, wy=w*y, wz=w*z;
            return mat4(
                1-2*(yy+zz), 2*(xy-wz),   2*(xz+wy), 0.0,
                2*(xy+wz),   1-2*(xx+zz), 2*(yz-wx), 0.0,
                2*(xz-wy),   2*(yz+wx),   1-2*(xx+yy), 0.0,
                0.0,         0.0,         0.0,         1.0);
        }
        static mat4 createLookAt(const vec3& eye, const vec3& target, vec3 up) {
            vec3 zAxis = (eye - target).normalize();
            vec3 xAxis = up.cross(zAxis).normalize();
            vec3 yAxis = zAxis.cross(xAxis);
            return mat4(
                xAxis.x, xAxis.y, xAxis.z, -xAxis.dot(eye),
                yAxis.x, yAxis.y, yAxis.z, -yAxis.dot(eye),
                zAxis.x, zAxis.y, zAxis.z, -zAxis.dot(eye),
                0.0,     0.0,     0.0,      1.0);
        }
        static mat4 createPerspectiveFieldOfView(double fovYRad, double aspectRatio, double zNear, double zFar) {
            if (zNear <= 0.0 || zFar <= 0.0 || zNear >= zFar || aspectRatio <= 0.0 || fovYRad <= 0.0) {
                // Invalid parameters, return identity or throw
                return identity();
            }
            double tanHalfFovY = std::tan(fovYRad * 0.5);
            mat4 result = zero();
            result.m[0]  = 1.0 / (aspectRatio * tanHalfFovY);
            result.m[5]  = 1.0 / tanHalfFovY;
            result.m[10] = -(zFar + zNear) / (zFar - zNear);
            result.m[11] = -1.0;
            result.m[14] = -(2.0 * zFar * zNear) / (zFar - zNear);
            return result;
        }
        static mat4 createOrthographic(double left, double right, double bottom, double top, double zNear, double zFar) {
            if (left == right || bottom == top || zNear == zFar) {
                 // Invalid parameters, return identity or throw
                return identity();
            }
            mat4 result = identity();
            result.m[0]  = 2.0 / (right - left);
            result.m[5]  = 2.0 / (top - bottom);
            result.m[10] = -2.0 / (zFar - zNear); // Standard GL version, maps z to [-1, 1]
            result.m[3]  = -(right + left) / (right - left);
            result.m[7]  = -(top + bottom) / (top - bottom);
            result.m[11] = -(zFar + zNear) / (zFar - zNear);
            return result;
        }

        // --- Element Access ---
        [[nodiscard]] double operator()(int row, int col) const {
            if (row<0||row>=4||col<0||col>=4) throw std::out_of_range("mat4 access out of range");
            return m[row * 4 + col];
        }
        double& operator()(int row, int col) {
            if (row<0||row>=4||col<0||col>=4) throw std::out_of_range("mat4 access out of range");
            return m[row * 4 + col];
        }
        [[nodiscard]] vec4 getRow(int r) const { return vec4(m[r*4],m[r*4+1],m[r*4+2],m[r*4+3]); }
        [[nodiscard]] vec4 getCol(int c) const { return vec4(m[c],m[c+4],m[c+8],m[c+12]); }
        void setRow(int r, const vec4& v) { m[r*4]=v.x; m[r*4+1]=v.y; m[r*4+2]=v.z; m[r*4+3]=v.w; }
        void setCol(int c, const vec4& v) { m[c]=v.x; m[c+4]=v.y; m[c+8]=v.z; m[c+12]=v.w; }

        // --- Core Matrix Operations ---
        mat4& transpose() {
            std::swap(m[1],m[4]); std::swap(m[2],m[8]); std::swap(m[3],m[12]);
            std::swap(m[6],m[9]); std::swap(m[7],m[13]); std::swap(m[11],m[14]);
            return *this;
        }
        [[nodiscard]] mat4 transposed() const { mat4 r=*this; return r.transpose(); }

        [[nodiscard]] double determinant() const {
            return m[0] * (m[5]*(m[10]*m[15]-m[11]*m[14]) - m[6]*(m[9]*m[15]-m[11]*m[13]) + m[7]*(m[9]*m[14]-m[10]*m[13]))
                 - m[1] * (m[4]*(m[10]*m[15]-m[11]*m[14]) - m[6]*(m[8]*m[15]-m[11]*m[12]) + m[7]*(m[8]*m[14]-m[10]*m[12]))
                 + m[2] * (m[4]*(m[9]*m[15]-m[11]*m[13]) - m[5]*(m[8]*m[15]-m[11]*m[12]) + m[7]*(m[8]*m[11]-m[9]*m[12]))
                 - m[3] * (m[4]*(m[9]*m[14]-m[10]*m[13]) - m[5]*(m[8]*m[14]-m[10]*m[12]) + m[6]*(m[8]*m[11]-m[9]*m[12]));
        }

        mat4& invert() {
            mat4 adj; // Adjugate matrix
            adj.m[0]  =  (m[5]*(m[10]*m[15]-m[11]*m[14]) - m[6]*(m[9]*m[15]-m[11]*m[13]) + m[7]*(m[9]*m[14]-m[10]*m[13]));
            adj.m[1]  = -(m[1]*(m[10]*m[15]-m[11]*m[14]) - m[2]*(m[9]*m[15]-m[11]*m[13]) + m[3]*(m[9]*m[14]-m[10]*m[13]));
            adj.m[2]  =  (m[1]*(m[6]*m[15]-m[7]*m[14])  - m[2]*(m[5]*m[15]-m[7]*m[13])  + m[3]*(m[5]*m[14]-m[6]*m[13]));
            adj.m[3]  = -(m[1]*(m[6]*m[11]-m[7]*m[10])  - m[2]*(m[5]*m[11]-m[7]*m[9])   + m[3]*(m[5]*m[10]-m[6]*m[9]));
            adj.m[4]  = -(m[4]*(m[10]*m[15]-m[11]*m[14]) - m[6]*(m[8]*m[15]-m[11]*m[12]) + m[7]*(m[8]*m[14]-m[10]*m[12]));
            adj.m[5]  =  (m[0]*(m[10]*m[15]-m[11]*m[14]) - m[2]*(m[8]*m[15]-m[11]*m[12]) + m[3]*(m[8]*m[14]-m[10]*m[12]));
            adj.m[6]  = -(m[0]*(m[6]*m[15]-m[7]*m[14])  - m[2]*(m[4]*m[15]-m[7]*m[12])  + m[3]*(m[4]*m[14]-m[6]*m[12]));
            adj.m[7]  =  (m[0]*(m[6]*m[11]-m[7]*m[10])  - m[2]*(m[4]*m[11]-m[7]*m[8])   + m[3]*(m[4]*m[10]-m[6]*m[8]));
            adj.m[8]  =  (m[4]*(m[9]*m[15]-m[11]*m[13]) - m[5]*(m[8]*m[15]-m[11]*m[12]) + m[7]*(m[8]*m[11]-m[9]*m[12]));
            adj.m[9]  = -(m[0]*(m[9]*m[15]-m[11]*m[13]) - m[1]*(m[8]*m[15]-m[11]*m[12]) + m[3]*(m[8]*m[11]-m[9]*m[12]));
            adj.m[10] =  (m[0]*(m[5]*m[15]-m[7]*m[13])  - m[1]*(m[4]*m[15]-m[7]*m[12])  + m[3]*(m[4]*m[11]-m[5]*m[12]));
            adj.m[11] = -(m[0]*(m[5]*m[11]-m[7]*m[9])   - m[1]*(m[4]*m[11]-m[7]*m[8])   + m[3]*(m[4]*m[9]-m[5]*m[8]));
            adj.m[12] = -(m[4]*(m[9]*m[14]-m[10]*m[13]) - m[5]*(m[8]*m[14]-m[10]*m[12]) + m[6]*(m[8]*m[11]-m[9]*m[12]));
            adj.m[13] =  (m[0]*(m[9]*m[14]-m[10]*m[13]) - m[1]*(m[8]*m[14]-m[10]*m[12]) + m[2]*(m[8]*m[11]-m[9]*m[12]));
            adj.m[14] = -(m[0]*(m[5]*m[14]-m[6]*m[13])  - m[1]*(m[4]*m[14]-m[6]*m[12])  + m[2]*(m[4]*m[9]-m[5]*m[12]));
            adj.m[15] =  (m[0]*(m[5]*m[10]-m[6]*m[9])   - m[1]*(m[4]*m[10]-m[6]*m[8])   + m[2]*(m[4]*m[5]-m[5]*m[8]));

            mat4 A = *this; // original matrix
            mat4 C; // Cofactor matrix (not adjugate yet)
            C.m[0] =  (A.m[5]*(A.m[10]*A.m[15]-A.m[11]*A.m[14]) - A.m[6]*(A.m[9]*A.m[15]-A.m[11]*A.m[13]) + A.m[7]*(A.m[9]*A.m[14]-A.m[10]*A.m[13]));
            C.m[1] = -(A.m[4]*(A.m[10]*A.m[15]-A.m[11]*A.m[14]) - A.m[6]*(A.m[8]*A.m[15]-A.m[11]*A.m[12]) + A.m[7]*(A.m[8]*A.m[14]-A.m[10]*A.m[12]));
            C.m[2] =  (A.m[4]*(A.m[9]*A.m[15]-A.m[11]*A.m[13]) - A.m[5]*(A.m[8]*A.m[15]-A.m[11]*A.m[12]) + A.m[7]*(A.m[8]*A.m[11]-A.m[9]*A.m[12]));
            C.m[3] = -(A.m[4]*(A.m[9]*A.m[14]-A.m[10]*A.m[13]) - A.m[5]*(A.m[8]*A.m[14]-A.m[10]*A.m[12]) + A.m[6]*(A.m[8]*A.m[11]-A.m[9]*A.m[12]));
            C.m[4] = -(A.m[1]*(A.m[10]*A.m[15]-A.m[11]*A.m[14]) - A.m[2]*(A.m[9]*A.m[15]-A.m[11]*A.m[13]) + A.m[3]*(A.m[9]*A.m[14]-A.m[10]*A.m[13]));
            C.m[5] =  (A.m[0]*(A.m[10]*A.m[15]-A.m[11]*A.m[14]) - A.m[2]*(A.m[8]*A.m[15]-A.m[11]*A.m[12]) + A.m[3]*(A.m[8]*A.m[14]-A.m[10]*A.m[12]));
            C.m[6] = -(A.m[0]*(A.m[9]*A.m[15]-A.m[11]*A.m[13]) - A.m[1]*(A.m[8]*A.m[15]-A.m[11]*A.m[12]) + A.m[3]*(A.m[8]*A.m[11]-A.m[9]*A.m[12]));
            C.m[7] =  (A.m[0]*(A.m[9]*A.m[14]-A.m[10]*A.m[13]) - A.m[1]*(A.m[8]*A.m[14]-A.m[10]*A.m[12]) + A.m[2]*(A.m[8]*A.m[11]-A.m[9]*A.m[12]));
            C.m[8] =   (A.m[1]*(A.m[6]*A.m[15]-A.m[7]*A.m[14]) - A.m[2]*(A.m[5]*A.m[15]-A.m[7]*A.m[13]) + A.m[3]*(A.m[5]*A.m[14]-A.m[6]*A.m[13]));
            C.m[9] =  -(A.m[0]*(A.m[6]*A.m[15]-A.m[7]*A.m[14]) - A.m[2]*(A.m[4]*A.m[15]-A.m[7]*A.m[12]) + A.m[3]*(A.m[4]*A.m[14]-A.m[6]*A.m[12]));
            C.m[10] =  (A.m[0]*(A.m[5]*A.m[15]-A.m[7]*A.m[13]) - A.m[1]*(A.m[4]*A.m[15]-A.m[7]*A.m[12]) + A.m[3]*(A.m[4]*A.m[6]-A.m[5]*A.m[12]));
            C.m[10] =  (A.m[0]*(A.m[5]*A.m[15]-A.m[7]*A.m[13]) - A.m[1]*(A.m[4]*A.m[15]-A.m[7]*A.m[12]) + A.m[3]*(A.m[4]*A.m[5]-A.m[13]*A.m[0]));
            C.m[10] = (A.m[0]*(A.m[5]*A.m[15]-A.m[7]*A.m[13]) - A.m[1]*(A.m[4]*A.m[15]-A.m[7]*A.m[12]) + A.m[3]*(A.m[4]*A.m[5]-A.m[5]*A.m[12]));
            C.m[10] =  (A.m[0]*(A.m[5]*A.m[15] - A.m[7]*A.m[13]) - A.m[1]*(A.m[4]*A.m[15] - A.m[7]*A.m[12]) + A.m[3]*(A.m[4]*A.m[13] - A.m[5]*A.m[12]));
            C.m[11] = -(A.m[0]*(A.m[5]*A.m[14]-A.m[6]*A.m[13]) - A.m[1]*(A.m[4]*A.m[14]-A.m[6]*A.m[12]) + A.m[2]*(A.m[4]*A.m[13]-A.m[5]*A.m[12]));
            C.m[12] = -(A.m[1]*(A.m[6]*A.m[11]-A.m[7]*A.m[10]) - A.m[2]*(A.m[5]*A.m[11]-A.m[7]*A.m[9])  + A.m[3]*(A.m[5]*A.m[10]-A.m[6]*A.m[9]));
            C.m[13] =  (A.m[0]*(A.m[6]*A.m[11]-A.m[7]*A.m[10]) - A.m[2]*(A.m[4]*A.m[11]-A.m[7]*A.m[8])  + A.m[3]*(A.m[4]*A.m[10]-A.m[6]*A.m[8]));
            C.m[14] = -(A.m[0]*(A.m[5]*A.m[11]-A.m[7]*A.m[9])  - A.m[1]*(A.m[4]*A.m[11]-A.m[7]*A.m[8])  + A.m[3]*(A.m[4]*A.m[5]-A.m[5]*A.m[8]));
            C.m[14] = -(A.m[0]*(A.m[5]*A.m[11]-A.m[7]*A.m[9]) - A.m[1]*(A.m[4]*A.m[11]-A.m[7]*A.m[8]) + A.m[3]*(A.m[4]*A.m[9]-A.m[5]*A.m[8]));


            C.m[15] =  (A.m[0]*(A.m[5]*A.m[10]-A.m[6]*A.m[9])  - A.m[1]*(A.m[4]*A.m[10]-A.m[6]*A.m[8])  + A.m[2]*(A.m[4]*A.m[9]-A.m[5]*A.m[8]));

            double det = A.m[0]*C.m[0] + A.m[1]*C.m[1] + A.m[2]*C.m[2] + A.m[3]*C.m[3]; // Determinant using first row of A and first row of C (cofactors of first row)

            if (std::abs(det) < default_Tolerance) {
                *this = zero(); // Singular matrix
                return *this;
            }
            double invDet = 1.0 / det;
            mat4 adjugate = C.transposed(); // Adjugate is transpose of cofactor matrix
            *this = adjugate * invDet;
            return *this;
        }
        [[nodiscard]] mat4 inversed() const { mat4 r=*this; return r.invert(); }

        // --- Decomposition ---
        [[nodiscard]] vec3 getTranslation() const {
            return vec3(m[3], m[7], m[11]);
        }
        [[nodiscard]] vec3 getScale() const {
            // Assumes no shear. Gets scale from the length of basis vectors.
            double sx = vec3(m[0], m[4], m[8]).length();  // Length of first column vector (X basis)
            double sy = vec3(m[1], m[5], m[9]).length();  // Length of second column vector (Y basis)
            double sz = vec3(m[2], m[6], m[10]).length(); // Length of third column vector (Z basis)
            // Determine sign of scale by checking determinant of upper 3x3
            mat3 upper3x3(m[0],m[1],m[2], m[4],m[5],m[6], m[8],m[9],m[10]); // Need a mat3 determinant here
            // For simplicity, if original matrix determinant is negative, assume one scale is negative.
            // This is a simplification. Proper decomposition is more involved with shear.
            if (determinant() < 0.0) {
                sx = -sx; // Or another axis, this is ambiguous without more context
            }
            return vec3(sx, sy, sz);
        }

        // --- Conversions to other types ---
        [[nodiscard]] quaternion toQuaternion() const {
            // Assumes the upper 3x3 is a pure rotation matrix (or rotation + uniform scale)
            // Normalizes the basis vectors to remove scale for quaternion extraction
            vec3 s = getScale(); // Get scale first
            if (std::abs(s.x) < default_Tolerance || std::abs(s.y) < default_Tolerance || std::abs(s.z) < default_Tolerance) {
                 return quaternion::identity(); // Avoid division by zero if scale is zero
            }

            mat4 rotMatNoScale(
                m[0]/s.x, m[1]/s.y, m[2]/s.z, 0.0,
                m[4]/s.x, m[5]/s.y, m[6]/s.z, 0.0,
                m[8]/s.x, m[9]/s.y, m[10]/s.z, 0.0,
                0.0,      0.0,      0.0,      1.0
            );
            
            // Using euclideanspace.com algorithm for matrix to quaternion
            double trace = rotMatNoScale.m[0] + rotMatNoScale.m[5] + rotMatNoScale.m[10]; // m00 + m11 + m22
            double qw, qx, qy, qz;

            if (trace > 0.0) {
                double S = safe_sqrt(trace + 1.0) * 2.0; // S = 4*qw
                qw = 0.25 * S;
                qx = (rotMatNoScale.m[6] - rotMatNoScale.m[9]) / S;  // (m21 - m12) / S ; m[6]=m12, m[9]=m21 in this row-major context for elements m_rc.
                                                                     // m[9] is m21 (el at row2,col1), m[6] is m12 (el at row1,col2)
                                                                     // For formula qx = (m21-m12)/(4qw) = (m[9]-m[6])/S
                qx = (rotMatNoScale.m[9] - rotMatNoScale.m[6]) / S;
                qy = (rotMatNoScale.m[2] - rotMatNoScale.m[8]) / S;  // (m02 - m20) / S = (m[2]-m[8])/S
                qz = (rotMatNoScale.m[4] - rotMatNoScale.m[1]) / S;  // (m10 - m01) / S = (m[4]-m[1])/S
            } else if ((rotMatNoScale.m[0] > rotMatNoScale.m[5]) && (rotMatNoScale.m[0] > rotMatNoScale.m[10])) { // m00 is largest
                double S = safe_sqrt(1.0 + rotMatNoScale.m[0] - rotMatNoScale.m[5] - rotMatNoScale.m[10]) * 2.0; // S = 4*qx
                qw = (rotMatNoScale.m[9] - rotMatNoScale.m[6]) / S; // (m21 - m12) / S
                qx = 0.25 * S;
                qy = (rotMatNoScale.m[1] + rotMatNoScale.m[4]) / S; // (m01 + m10) / S
                qz = (rotMatNoScale.m[2] + rotMatNoScale.m[8]) / S; // (m02 + m20) / S
            } else if (rotMatNoScale.m[5] > rotMatNoScale.m[10]) { // m11 is largest
                double S = safe_sqrt(1.0 + rotMatNoScale.m[5] - rotMatNoScale.m[0] - rotMatNoScale.m[10]) * 2.0; // S = 4*qy
                qw = (rotMatNoScale.m[2] - rotMatNoScale.m[8]) / S; // (m02 - m20) / S
                qx = (rotMatNoScale.m[1] + rotMatNoScale.m[4]) / S; // (m01 + m10) / S
                qy = 0.25 * S;
                qz = (rotMatNoScale.m[6] + rotMatNoScale.m[9]) / S; // (m12 + m21) / S
            } else { // m22 is largest
                double S = safe_sqrt(1.0 + rotMatNoScale.m[10] - rotMatNoScale.m[0] - rotMatNoScale.m[5]) * 2.0; // S = 4*qz
                qw = (rotMatNoScale.m[4] - rotMatNoScale.m[1]) / S; // (m10 - m01) / S
                qx = (rotMatNoScale.m[2] + rotMatNoScale.m[8]) / S; // (m02 + m20) / S
                qy = (rotMatNoScale.m[6] + rotMatNoScale.m[9]) / S; // (m12 + m21) / S
                qz = 0.25 * S;
            }
            return quaternion(qx, qy, qz, qw).normalize(); // Ensure normalized
        }

        [[nodiscard]] vec3 toEulerAngles() const { // ZYX rotation order (Yaw, Pitch, Roll)
             // First, get a quaternion from the matrix
            quaternion q = toQuaternion(); // Assumes upper 3x3 is rotation or scaled rotation
            return q.toEulerAngles(); // Use quaternion's method
        }
        
        // --- Transformations ---
        [[nodiscard]] vec4 operator*(const vec4& v) const {
            return vec4(
                m[0]*v.x + m[1]*v.y + m[2]*v.z + m[3]*v.w,
                m[4]*v.x + m[5]*v.y + m[6]*v.z + m[7]*v.w,
                m[8]*v.x + m[9]*v.y + m[10]*v.z + m[11]*v.w,
                m[12]*v.x + m[13]*v.y + m[14]*v.z + m[15]*v.w);
        }
        [[nodiscard]] vec3 transformPoint(const vec3& p) const {
            vec4 r = *this * vec4(p.x,p.y,p.z,1.0);
            if (std::abs(r.w)>default_Tolerance && std::abs(r.w-1.0)>default_Tolerance) { // Perspective divide if w is not 0 or 1
                double invW = 1.0/r.w; return vec3(r.x*invW, r.y*invW, r.z*invW);
            }
            return vec3(r.x, r.y, r.z);
        }
        [[nodiscard]] vec3 transformDirection(const vec3& d) const {
            vec4 r = *this * vec4(d.x,d.y,d.z,0.0);
            return vec3(r.x, r.y, r.z);
        }
        [[nodiscard]] vec2 transformPoint(const vec2& p, double z_coord = 0.0) const {
            vec3 p3d = transformPoint(vec3(p.x, p.y, z_coord));
            return vec2(p3d.x, p3d.y);
        }
        [[nodiscard]] vec2 transformDirection(const vec2& d) const {
            vec3 d3d = transformDirection(vec3(d.x, d.y, 0.0));
            return vec2(d3d.x, d3d.y);
        }

        // --- Operators ---
        [[nodiscard]] mat4 operator*(const mat4& o) const {
            mat4 r=zero();
            for(int i=0;i<4;++i) for(int j=0;j<4;++j) for(int k=0;k<4;++k)
                r.m[i*4+j] += m[i*4+k] * o.m[k*4+j];
            return r;
        }
        mat4& operator*=(const mat4& o) { *this = *this * o; return *this; }
        [[nodiscard]] mat4 operator*(double s) const { mat4 r; for(int i=0;i<16;++i)r.m[i]=m[i]*s; return r; }
        mat4& operator*=(double s) { for(int i=0;i<16;++i)m[i]*=s; return *this; }
        [[nodiscard]] mat4 operator/(double s) const { mat4 r; double invS=safe_divide(1.0,s); for(int i=0;i<16;++i)r.m[i]=m[i]*invS; return r; }
        mat4& operator/=(double s) { double invS=safe_divide(1.0,s); for(int i=0;i<16;++i)m[i]*=invS; return *this; }
        [[nodiscard]] mat4 operator+(const mat4& o) const { mat4 r; for(int i=0;i<16;++i)r.m[i]=m[i]+o.m[i]; return r; }
        mat4& operator+=(const mat4& o) { for(int i=0;i<16;++i)m[i]+=o.m[i]; return *this; }
        [[nodiscard]] mat4 operator-(const mat4& o) const { mat4 r; for(int i=0;i<16;++i)r.m[i]=m[i]-o.m[i]; return r; }
        mat4& operator-=(const mat4& o) { for(int i=0;i<16;++i)m[i]-=o.m[i]; return *this; }
        [[nodiscard]] mat4 operator-() const { mat4 r; for(int i=0;i<16;++i)r.m[i]=-m[i]; return r; }
        bool operator==(const mat4& o) const {
            for(int i=0;i<16;++i) if(std::abs(m[i]-o.m[i])>default_Tolerance) return false;
            return true;
        }
        bool operator!=(const mat4& o) const { return !(*this==o); }
        
        // --- String Conversion ---
        std::string toStr() const {
            return std::format(
                "mat4[\n"
                "  {:.3f}, {:.3f}, {:.3f}, {:.3f},\n"
                "  {:.3f}, {:.3f}, {:.3f}, {:.3f},\n"
                "  {:.3f}, {:.3f}, {:.3f}, {:.3f},\n"
                "  {:.3f}, {:.3f}, {:.3f}, {:.3f}\n]",
                m[0],m[1],m[2],m[3], m[4],m[5],m[6],m[7],
                m[8],m[9],m[10],m[11], m[12],m[13],m[14],m[15]);
        }
        const double* data() const { return m.data(); }
        double* data() { return m.data(); }

    private:
        // Helper struct for mat3 operations needed for determinant/inverse if not part of public API
        struct mat3 {
            double val[9];
            mat3(double v0,double v1,double v2, double v3,double v4,double v5, double v6,double v7,double v8)
                : val{v0,v1,v2,v3,v4,v5,v6,v7,v8} {}
            double determinant() const {
                return val[0]*(val[4]*val[8]-val[5]*val[7]) - val[1]*(val[3]*val[8]-val[5]*val[6]) + val[2]*(val[3]*val[7]-val[4]*val[6]);
            }
        };
}; // End of struct mat4

    [[nodiscard]] inline mat4 operator*(double s, const mat4& matrix) { return matrix * s; }
    inline std::ostream& operator<<(std::ostream& os, const mat4& matrix) { os << matrix.toStr(); return os; }

} // End of namespace vecSys