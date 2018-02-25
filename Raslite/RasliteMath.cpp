#include "RasliteMath.h"
///////////////////////////////////////////////////////////////////////////////////////////
// Vec2/Vec3/Vec4
///////////////////////////////////////////////////////////////////////////////////////////
namespace rl {
    Vec2::Vec2(const Vec4 &rhs)
    {
        x = rhs.x; y = rhs.y;
    }

    Vec2::Vec2(const Vec3 &rhs)
    {
        x = rhs.x; y = rhs.y;
    }
    const Vec2 &Vec2::operator =(const Vec4 &rhs)
    {
        x = rhs.x; y = rhs.y;
        return *this;
    }

    const Vec2 &Vec2::operator =(const Vec3 &rhs)
    {
        x = rhs.x; y = rhs.y;
        return *this;
    }
    /////////////////////////////////////////////////////////////////////
    // Vec3
    /////////////////////////////////////////////////////////////////////
    const Vec3 Vec3::ZERO(Float(0), Float(0), Float(0));
    const Vec3 Vec3::ONE(Float(1), Float(1), Float(1));
    const Vec3 Vec3::UNIT_X(Float(1), Float(0), Float(0));
    const Vec3 Vec3::UNIT_Y(Float(0), Float(1), Float(0));
    const Vec3 Vec3::UNIT_Z(Float(0), Float(0), Float(1));

    Vec3::Vec3(const Vec4 &rhs)
    {
        x = rhs.x; y = rhs.y; z = rhs.z;
    }
    const Vec3& Vec3::operator=(const Vec4 &rhs)
    {
        x = rhs.x; y = rhs.y; z = rhs.z;
        return *this;
    }

    //const Vec3& Vec3::operator *= (const Mat4 &rhs)
    //{
    //	*this = (Vec4(x, y, z, 1) * rhs).homogenize();
    //	return *this;
    //}
    //
    //Vec3 Vec3::operator *(const Mat4 &rhs) const
    //{
    //    return Vec3(*this) *= rhs;
    //}
    /////////////////////////////////////////////////////////////////////
    // Vec4
    /////////////////////////////////////////////////////////////////////
    const Vec4 Vec4::ZERO(0, 0, 0, 0);
    const Vec4 Vec4::ONE(1, 1, 1, 1);
    const Vec4 Vec4::BLACK(0, 0, 0, 1);
    const Vec4 Vec4::WHITE(1, 1, 1, 1);
    const Vec4 Vec4::RED(1, 0, 0, 1);
    const Vec4 Vec4::GREEN(0, 1, 0, 1);
    const Vec4 Vec4::BLUE(0, 0, 1, 1);

    const Vec4& Vec4::operator *= (const Mat4& rhs)
    {
        const auto xVal = rhs._11 * x + rhs._21 * y + rhs._31 * z + rhs._41 * w;
        const auto yVal = rhs._12 * x + rhs._22 * y + rhs._32 * z + rhs._42 * w;
        const auto zVal = rhs._13 * x + rhs._23 * y + rhs._33 * z + rhs._43 * w;
        const auto wVal = rhs._14 * x + rhs._24 * y + rhs._34 * z + rhs._44 * w;
        x = xVal; y = yVal; z = zVal; w = wVal;
        return *this;
    }
    Vec4 Vec4::operator *(const Mat4 &rhs) const
    {
        return Vec4(*this) *= rhs;
    }
}//ns rb
///////////////////////////////////////////////////////////////////////////////////////////
// Quat
///////////////////////////////////////////////////////////////////////////////////////////
namespace rl
{
    const Quat Quat::IDENTITY ={ 0,0,0,1 };
    Quat slerp(const Quat& lhs, const Quat& rhs, Float factor)
    {
        auto cosVal = lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
        Float to1[4];
        if(cosVal < 0.0f)
        {
            cosVal = -cosVal;
            to1[0] = -rhs.x;
            to1[1] = -rhs.y;
            to1[2] = -rhs.z;
            to1[3] = -rhs.w;
        }
        else
        {
            to1[0] = rhs.x;
            to1[1] = rhs.y;
            to1[2] = rhs.z;
            to1[3] = rhs.w;
        }

        const auto omega  = acosf(cosVal);
        const auto sinInv = 1.0f / sinf(omega);
        const auto scale0 = sinf((1.0f - factor) * omega) * sinInv;
        const auto scale1 = sinf(factor * omega) * sinInv;

        return
        {
            scale0 * rhs.x + scale1 * to1[0],
            scale0 * rhs.y + scale1 * to1[1],
            scale0 * rhs.z + scale1 * to1[2],
            scale0 * rhs.w + scale1 * to1[3],
        };
    }
}
///////////////////////////////////////////////////////////////////////////////////////////
// Mat4
///////////////////////////////////////////////////////////////////////////////////////////
namespace rl {
    const Mat4 Mat4::IDENTITY(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
    const Mat4 Mat4::ZERO(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0);
    const Mat4& Mat4::operator *=(const Mat4& rhs)
    {
        const auto f11 = _11 * rhs._11 + _12 * rhs._21 + _13 * rhs._31 + _14 * rhs._41;
        const auto f12 = _11 * rhs._12 + _12 * rhs._22 + _13 * rhs._32 + _14 * rhs._42;
        const auto f13 = _11 * rhs._13 + _12 * rhs._23 + _13 * rhs._33 + _14 * rhs._43;
        const auto f14 = _11 * rhs._14 + _12 * rhs._24 + _13 * rhs._34 + _14 * rhs._44;
        const auto f21 = _21 * rhs._11 + _22 * rhs._21 + _23 * rhs._31 + _24 * rhs._41;
        const auto f22 = _21 * rhs._12 + _22 * rhs._22 + _23 * rhs._32 + _24 * rhs._42;
        const auto f23 = _21 * rhs._13 + _22 * rhs._23 + _23 * rhs._33 + _24 * rhs._43;
        const auto f24 = _21 * rhs._14 + _22 * rhs._24 + _23 * rhs._34 + _24 * rhs._44;
        const auto f31 = _31 * rhs._11 + _32 * rhs._21 + _33 * rhs._31 + _34 * rhs._41;
        const auto f32 = _31 * rhs._12 + _32 * rhs._22 + _33 * rhs._32 + _34 * rhs._42;
        const auto f33 = _31 * rhs._13 + _32 * rhs._23 + _33 * rhs._33 + _34 * rhs._43;
        const auto f34 = _31 * rhs._14 + _32 * rhs._24 + _33 * rhs._34 + _34 * rhs._44;
        const auto f41 = _41 * rhs._11 + _42 * rhs._21 + _43 * rhs._31 + _44 * rhs._41;
        const auto f42 = _41 * rhs._12 + _42 * rhs._22 + _43 * rhs._32 + _44 * rhs._42;
        const auto f43 = _41 * rhs._13 + _42 * rhs._23 + _43 * rhs._33 + _44 * rhs._43;
        const auto f44 = _41 * rhs._14 + _42 * rhs._24 + _43 * rhs._34 + _44 * rhs._44;

        _11 = f11; _12 = f12; _13 = f13; _14 = f14;
        _21 = f21; _22 = f22; _23 = f23; _24 = f24;
        _31 = f31; _32 = f32; _33 = f33; _34 = f34;
        _41 = f41; _42 = f42; _43 = f43; _44 = f44;
        return *this;
    }
    static inline Float determinant2x2(const Float a, const Float b, const Float c, const Float d)
    {
        return a * d - b * c;
    }
    static Float determinant3x3(const Float a1, const Float a2, const Float a3,
                                const Float b1, const Float b2, const Float b3,
                                const Float c1, const Float c2, const Float c3)
    {
        return  a1 * determinant2x2(b2, b3, c2, c3)
              - b1 * determinant2x2(a2, a3, c2, c3)
              + c1 * determinant2x2(a2, a3, b2, b3);
    }
    static Float determinantMinor(const Mat4 &val, const uint32_t row, const uint32_t col)
    {
        Float mat[3][3];

        for(uint32_t r = 0, m = 0; r < 4; ++r)
        {
            if(r == row)
                continue;
            for(uint32_t c = 0, n = 0; c < 4; ++c)
            {
                if(c == col)
                    continue;
                mat[m][n] = val.m[r][c];
                ++n;
            }
            ++m;
        }
        return determinant3x3(mat[0][0], mat[0][1], mat[0][2], mat[1][0], mat[1][1], mat[1][2], mat[2][0], mat[2][1], mat[2][2]);
    }
    static Mat4 _adjoint(const Mat4& m)
    {
        Mat4 matReturn;
        for(uint32_t r = 0; r < 4; ++r)
        {
            for(uint32_t c = 0; c < 4; ++c)
                matReturn.m[c][r] = powf(-1.0f, (Float)(r + c)) * determinantMinor(m, r, c);
        }
        return matReturn;
    }
    Float Mat4::determinant() const
    {
        return _11 * determinant3x3(_22, _32, _42, _23, _33, _43, _24, _34, _44)
             - _12 * determinant3x3(_21, _31, _41, _23, _33, _43, _24, _34, _44)
             + _13 * determinant3x3(_21, _31, _41, _22, _32, _42, _24, _34, _44)
             - _14 * determinant3x3(_21, _31, _41, _22, _32, _42, _23, _33, _43);
    }
    Mat4& Mat4::inverse()
    {
        const auto d = this->determinant();
        if(fabsf(d) < FLT_EPSILON)
            return *this;
        *this = _adjoint(*this) / d;
        return *this;
    }
    Mat4& Mat4::transpose()
    {
        for(int i = 0; i < 3; ++i)
        {
            for(int r = i; r < 4; ++r)
                for(int c = i + 1; c < 4; ++c)
                    std::swap(this->m[r][c], this->m[c][r]);
        }
        return *this;
    }
    Mat4 Mat4::getTranspose() const
    {
        return Mat4(*this).transpose();
    }


}//ns rb
///////////////////////////////////////////////////////////////////////////////////////////
// Transform
///////////////////////////////////////////////////////////////////////////////////////////
namespace rl {

    Mat4 Transform::scale(Float x, Float y, Float z)
    {
        Mat4 m(Mat4::IDENTITY);
        {
            m._11 = x;
            m._22 = y;
            m._33 = z;
        }
        return m;
    }
    Mat4 Transform::scale(const Vec3& s)
    {
        return Transform::scale(s.x, s.y, s.z);
    }
    Mat4 Transform::translate(Float x, Float y, Float z)
    {
        Mat4 m(Mat4::IDENTITY);
        {
            m._41 = x;
            m._42 = y;
            m._43 = z;
        }
        return m;
    }
    Mat4 Transform::translate(const Vec3& v)
    {
        return Transform::translate(v.x, v.y, v.z);
    }
    Mat4 Transform::rotateX(Float radian)
    {
        auto sin = std::sin(radian), cos = std::cos(radian);
        Mat4 m(Mat4::IDENTITY);
        {
            m._22 =  cos; m._23 = sin;
            m._32 = -sin; m._33 = cos;
        }
        return m;
    }
    Mat4 Transform::rotateY(Float radian)
    {
        auto sin = std::sin(radian), cos = std::cos(radian);
        Mat4 m(Mat4::IDENTITY);
        {
            m._11 = cos;  m._13 = -sin;
            m._31 = sin;  m._33 =  cos;
        }
        return m;
    }
    Mat4 Transform::rotateZ(Float radian)
    {
        auto sin = std::sin(radian), cos = std::cos(radian);
        Mat4 m(Mat4::IDENTITY);
        {
            m._11 = cos; m._12 = sin;
            m._21 = -sin; m._22 = cos;
        }
        return m;
    }
    Mat4 Transform::rotateEular(Float yawRadian, Float pitchRadian, Float rollRadian)
    {
        auto yaw   = Transform::rotateY(yawRadian),
             pitch = Transform::rotateX(pitchRadian),
             roll  = Transform::rotateZ(rollRadian);
        return roll * pitch * yaw;
    }
    Mat4 Transform::rotateEular(const Vec3& radianYPR)
    {
        return Transform::rotateEular(radianYPR.x, radianYPR.y, radianYPR.z);
    }
    Mat4 Transform::rotateAxis(const Vec3& axis, Float radian)
    {
        assert(axis.normalized());
        const auto sin = std::sin(radian), cos = std::cos(radian);
        const auto cosInv = 1.0f - cos;
        Mat4 m(Mat4::IDENTITY);
        {
            m._11 = cosInv * axis.x * axis.x + cos;
            m._12 = cosInv * axis.x * axis.y - axis.z * sin;
            m._13 = cosInv * axis.x * axis.z + axis.y * sin;
            m._14 = 0.0f;

            m._21 = cosInv * axis.x * axis.y + axis.z * sin;
            m._22 = cosInv * axis.y * axis.y + cos;
            m._23 = cosInv * axis.y * axis.z - axis.x * sin;
            m._24 = 0.0f;

            m._31 = cosInv * axis.x * axis.z - axis.y * sin;
            m._32 = cosInv * axis.y * axis.z + axis.x * sin;
            m._33 = cosInv * axis.z * axis.z + cos;
            m._34 = 0.0f;
        }
        return m;
    }
    Mat4 Transform::rotateQuat(const Quat& q)
    {
        const Float sqX = q.x * q.x;
        const Float sqY = q.y * q.y;
        const Float sqZ = q.z * q.z;
        const Float sqW = q.w * q.w;
        Mat4 m(Mat4::IDENTITY);
        {
            m._11 = sqW + sqX - sqY - sqZ;
            m._12 = 2.0f * (q.x * q.y - q.w * q.z);
            m._13 = 2.0f * (q.x * q.z + q.w * q.y);
            m._14 = 0.0f;

            m._21 = 2.0f * (q.x * q.y + q.w * q.z);
            m._22 = sqW - sqX + sqY - sqZ;
            m._23 = 2.0f * (q.y * q.z - q.w * q.x);
            m._24 = 0.0f;

            m._31 = 2.0f * (q.x * q.z - q.w * q.y);
            m._32 = 2.0f * (q.y * q.z + q.w * q.x);
            m._33 = sqW - sqX - sqY + sqZ;
            m._34 = 0.0f;

            m._41 = 0.0f; m._42 = 0.0f; m._43 = 0.0f;
            m._44 = sqW + sqX + sqY + sqZ;
        }
        return m;
    }
    Mat4 Transform::lookAtLH(const Vec3& eye, const Vec3& at, const Vec3& up)
    {
        const auto zAxis = (at - eye).normalize();
        const auto xAxis = up.cross(zAxis).normalize();
        const auto yAxis = zAxis.cross(xAxis).normalize();
        Mat4 m(Mat4::IDENTITY);
        {
            m._11 = xAxis.x; m._12 = yAxis.x; m._13 = zAxis.x;
            m._21 = xAxis.y; m._22 = yAxis.y; m._23 = zAxis.y;
            m._31 = xAxis.z; m._32 = yAxis.z; m._33 = zAxis.z;

            m._41 = -xAxis.dot(eye);
            m._42 = -yAxis.dot(eye);
            m._43 = -zAxis.dot(eye);
        }
        return m;
    }
    Mat4 Transform::lookAtRH(const Vec3& eye, const Vec3& at, const Vec3& up)
    {
        return Transform::lookAtLH(at, eye, up);
    }
    Mat4 Transform::projectOrthoOffCenterLH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar)
    {
        const auto invLR = 1.0f / (left - right);
        const auto invBT = 1.0f / (bottom - top);
        const auto invNF = 1.0f / (zNear - zFar);

        Mat4 m(Mat4::IDENTITY);

        m._11 = -2.0f * invLR;      m._12 = 0.0f;               m._13 = 0.0f;         m._14 = 0.0f;
        m._21 = 0.0f;               m._22 = -2.0f * invBT;      m._23 = 0.0f;         m._24 = 0.0f;
        m._31 = 0.0f;               m._32 = 0.0f;               m._33 = -1.0f*invNF;  m._34 = 0.0f;
        m._41 = (left+right)*invLR; m._42 = (bottom+top)*invBT; m._43 = zNear*invNF;  m._44 = 1.0f;

        return m;
    }
    Mat4 Transform::projectOrthoOffCenterRH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar)
    {
        Mat4 m(Mat4::IDENTITY);

        m._11 = 2.0f / (right - left); m._12 = 0.0f; m._13 = 0.0f; m._14 = 0.0f;
        m._21 = 0.0f; m._22 = 2.0f / (top - bottom); m._23 = 0.0f; m._24 = 0.0f;
        m._31 = 0.0f; m._32 = 0.0f; m._33 = 1.0f / (zNear - zFar); m._34 = 0.0f;
        m._41 = (left + right) / (left - right); m._42 = (bottom + top) / (bottom - top); m._43 = zNear / (zNear - zFar); m._44 = 1.0f;
        return m;
    }
    Mat4 Transform::projectOrthoLH(Float width, Float height, Float zNear, Float zFar)
    {
        return Transform::projectOrthoOffCenterLH(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, zNear, zFar);
    }
    Mat4 Transform::projectOrthoRH(Float width, Float height, Float zNear, Float zFar)
    {
        return Transform::projectOrthoOffCenterRH(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, zNear, zFar);
    }
    Mat4 Transform::projectPerspLH(Float width, Float height, Float zNear, Float zFar)
    {
        const auto _11 = 2.0f * zNear / width,
            _22 = 2.0f * zNear / height,
            _33 = zFar / (zFar - zNear),
            _43 = -zNear * _33;

        return Mat4(_11, 0.0f, 0.0f, 0.0f,
            0.0f, _22, 0.0f, 0.0f,
            0.0f, 0.0f, _33, 1.0f,
            0.0f, 0.0f, _43, 0.0f);
    }
    Mat4 Transform::projectPerspRH(Float width, Float height, Float zNear, Float zFar)
    {
        const auto _11 = 2.0f * zNear / width,
            _22 = 2.0f * zNear / height,
            _33 = -zFar / (zFar - zNear),
            _43 = zNear * _33;
        return Mat4(_11, 0.0f, 0.0f, 0.0f,
            0.0f, _22, 0.0f, 0.0f,
            0.0f, 0.0f, _33, -1.0f,
            0.0f, 0.0f, _43, 0.0f);
    }
    Mat4 Transform::projectPerspFovLH(Float yFov, Float aspect, Float zNear, Float zFar)
    {
        auto h = std::tanf(yFov * 0.5f) * 2;
        auto w = aspect * h;
        return Transform::projectPerspLH(w, h, zNear, zFar);
    }
    Mat4 Transform::projectPerspFovRH(Float yFov, Float aspect, Float zNear, Float zFar)
    {
        Mat4 m(Mat4::IDENTITY);
        const auto height = 1.0f / tanf(yFov * 0.5f);
        const auto width = height / aspect;

        m._11 = width; m._12 = 0.0f;   m._13 = 0.0f;                          m._14 =  0.0f;
        m._21 = 0.0f;  m._22 = height; m._23 = 0.0f;                          m._24 =  0.0f;
        m._31 = 0.0f;  m._32 = 0.0f;   m._33 = zFar / (zNear - zFar);         m._34 = -1.0f;
        m._41 = 0.0f;  m._42 = 0.0f;   m._43 = zNear * zFar / (zNear - zFar); m._44 =  0.0f;

        return m;
    }
    Mat4 Transform::projectPerspOffCenterLH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar)
    {
        Mat4 m(Mat4::IDENTITY);
        m._11 = 2.0f * zNear / (right - left); m._12 = 0.0f; m._13 = 0.0f; m._14 = 0.0f;
        m._21 = 0.0f; m._22 = 2.0f * zNear / (top - bottom); m._23 = 0.0f; m._24 = 0.0f;
        m._31 = (left + right) / (left - right); m._32 = (bottom + top) / (bottom - top); m._33 = zFar / (zFar - zNear); m._34 = 1.0f;
        m._41 = 0.0f; m._42 = 0.0f; m._43 = zNear * zFar / (zNear - zFar); m._44 = 0.0f;
        return m;
    }
    Mat4 Transform::projectPerspOffCenterRH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar)
    {
        Mat4 m(Mat4::IDENTITY);
        m._11 = 2.0f * zNear / (right - left); m._12 = 0.0f; m._13 = 0.0f; m._14 = 0.0f;
        m._21 = 0.0f; m._22 = 2.0f * zNear / (top - bottom); m._23 = 0.0f; m._24 = 0.0f;
        m._31 = (left + right) / (left - right); m._32 = (bottom + top) / (bottom - top); m._33 = zFar / (zNear - zFar); m._34 = -1.0f;
        m._41 = 0.0f; m._42 = 0.0f; m._43 = zNear * zFar / (zNear - zFar); m._44 = 0.0f;
        return m;
    }
    Mat4 Transform::viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Float zNear, Float zFar)
    {
        const auto wHalf = (Float)width * 0.5f, hHalf = (Float)height * 0.5f;
        return Mat4(
            wHalf, 0.0f, 0.0f, 0.0f,
            0.0f, -hHalf, 0.0f, 0.0f,
            0.0f, 0.0f, zFar - zNear, 0.0f,
            (Float)x + wHalf, (Float)y + hHalf, zNear, 1.0f
        );
    }
    //////////////
    Vec3 Transform::transformVector(const Vec3& lhs, const Mat4& rhs)
    {
        return Vec4(lhs.x, lhs.y, lhs.z, 0) * rhs;
    }
    Vec3 Transform::getTranslation(const Mat4& m)
    {
        return m.getRow(3).xyz();
    }
    Quat Transform::getRotation(const Mat4& m)
    {
        Quat retval;
        const Float diagVal = m._11 + m._22 + m._33;
        if(diagVal > 0.0f)
        {
            Float s = sqrtf(diagVal + 1.0f);
            retval.w = s / 2.0f;
            s = 0.5f / s;
            retval.x = (m._32 - m._23) * s;
            retval.y = (m._13 - m._31) * s;
            retval.z = (m._21 - m._12) * s;
        }
        else
        {
            const uint32_t iNext[3] ={ 1, 2, 0 };

            Float q[4];
            uint32_t i = 0;
            if(m._22 > m._11)
                i = 1;
            if(m._33 > m(i, i))
                i = 2;
            uint32_t j = iNext[i];
            uint32_t k = iNext[j];

            Float s = sqrtf(m(i, i) - (m(j, j) + m(k, k)) + 1.0f);
            q[i] = s * 0.5f;

            if(s >= FLT_EPSILON)
                s = 0.5f / s;

            q[3] = (m(k, j) - m(j, k)) * s;
            q[j] = (m(j, i) + m(i, j)) * s;
            q[k] = (m(k, i) + m(i, k)) * s;

            retval.x = q[0];
            retval.y = q[1];
            retval.z = q[2];
            retval.w = q[3];
        }

        return retval;
    }
    AxisAngle Transform::getAxisAngle(const Quat& q)
    {
        AxisAngle axisAngle;
        const Float scl = std::sqrtf(q.x * q.x + q.y * q.y + q.z * q.z);
        if(scl >= FLT_EPSILON)
        {
            const Float sclInv = 1.0f / scl;
            axisAngle.first.x = q.x * sclInv;
            axisAngle.first.y = q.y * sclInv;
            axisAngle.first.z = q.z * sclInv;
        }
        else
        {
            axisAngle.first.x = 0.0f;
            axisAngle.first.y = 1.0f;
            axisAngle.first.z = 0.0f;
        }

        axisAngle.second = 2.0f * std::acosf(q.w);
        return axisAngle;
    }
}//ns rb