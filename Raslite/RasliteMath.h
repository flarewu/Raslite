#ifndef RASLITE_MATH_H
#define RASLITE_MATH_H
#include <cstdint>
#include <cmath>
#include <cfloat>
#include <cassert>

#include <limits>
#include <utility>
#include <algorithm>
#pragma warning( disable: 4996 ) // Disable warning about _controlfp being deprecated.
namespace rl
{
	using Float = float;
	constexpr Float PI     = 3.141592654f;
	constexpr Float PI_INV = (1 / PI);
    constexpr Float PI_2   = PI * 2;

    //设置FPU RoundMode为TruncationMode和单精度
	inline void fpu_truncate()
	{
		_controlfp(_RC_DOWN + _PC_24, _MCW_RC | _MCW_PC);
	}
    // 重置FPU RoundMode到默认状态
	inline void fpu_reset()
	{
		_controlfp(_CW_DEFAULT, _MCW_RC | _MCW_PC);
	}
    // 使用fpu_truncate()模式，否则将会被被round到最近的整数
	inline int32_t float2int_fast(Float f)
	{
#ifdef _X86_
		static int32_t tmp;
		__asm
		{
			fld f
			fistp tmp
		}
		return tmp;
#else
        return  int32_t(f);
#endif

	}
	inline Float rad2deg(Float rad)
	{
		return 180.0f * rad * PI_INV;
	}
	inline Float deg2rad(Float deg)
	{
		return PI * deg / 180.0f;
	}
	template <typename T>
	inline T clamp(T v, T lower, T upper)
	{
		return v <= lower ? lower : (v >= upper ? upper : v);
	}
	inline Float saturate(Float rhs)
	{
		return clamp(rhs, 0.0f, 1.0f);
	}
	inline Float lerp(Float a, Float b, Float factor)
	{
		return a + (b - a) * factor;
	}
	inline Float abs(Float v)
	{
		return std::fabsf(v);
	}
	inline int32_t abs(int32_t v)
	{
		return std::abs(v);
	}
	inline bool equal(Float lhs, Float rhs, Float epsilon = FLT_EPSILON)
	{
		return abs(rhs - lhs) < epsilon;
	}
	inline Float floor(Float v)
	{
		return std::floor(v);
	}
	inline Float ceil(Float v)
	{
		return std::ceil(v);
	}
	// 返回整数
	inline int32_t floori(Float v)
	{
        return float2int_fast(v);
	}
	inline uint32_t floorui(Float v)
	{
		assert(v >= 0);
		return uint32_t(float2int_fast(v));
	}
	// 返回整数
	inline int32_t ceili(Float v)
	{
		return float2int_fast(ceil(v));
	}
	inline uint32_t ceilui(Float v)
	{
		assert(v >= -1);
		return uint32_t(float2int_fast(ceil(v)));
	}
    inline float maskOr(float val, uint32_t mask)
    {
        auto maskv = (*reinterpret_cast<uint32_t*>(&val) | mask);
        return *reinterpret_cast<float*>(&maskv);
    }
	
}//ns rb
namespace rl
{
	struct Mat4;
	struct Vec3;
	struct Vec4;
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Vec2
    //////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Vec2
	{
        union 
        {
            struct { Float x, y; };
            struct { Float u, v; };
        };

		Vec2();
		Vec2(const Vec2 &rhs);
		Vec2(const Float x, const Float y);
		Vec2(const Vec4 &rhs);
		Vec2(const Vec3 &rhs);

		const Vec2& operator =(const Vec2& rhs);
		const Vec2& operator =(const Vec4& rhs);
		const Vec2& operator =(const Vec3& rhs);
		const Vec2& operator =(const Float  rhs);

		operator Float*();
		operator const Float*() const;

		Vec2 operator +() const;
		Vec2 operator -() const;

		const Vec2& operator +=(const Vec2 &rhs);
		const Vec2& operator -=(const Vec2 &rhs);
		const Vec2& operator *=(const Vec2 &rhs);
        const Vec2& operator *=(Float rhs);
        const Vec2& operator /=(Float rhs);

        Vec2 operator +(const Vec2& rhs) const;
        Vec2 operator -(const Vec2& rhs) const;
        Vec2 operator *(const Vec2& rhs) const;
        Vec2 operator *(Float  rhs) const;
        Vec2 operator /(Float  rhs) const;

		Float length() const;
		Float lengthSq() const;

        Vec2& normalize();
        Float  dot(const Vec2& rhs) const;
        Vec2& setLerp(const Vec2& rhs, Float factor);
        Vec2  lerp(const Vec2& rhs, Float factor) const;
	};
    Float dot (const Vec2& lhs, const Vec2& rhs);
    Vec2  lerp(const Vec2& lhs, const Vec2& rhs, Float factor);
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Vec2i
    //////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Vec2i
	{
		union
		{
			struct { int x, y; };
            struct { int u, v; };
			int raw[2];
		};
        static Vec2i zero()
        {
            return { 0,0 };
        }
		Vec2i() : x(0), y(0) {};
		Vec2i(int xrhs, int yrhs) : x(xrhs), y(yrhs) {};
        Vec2i(const Vec2& rhs): x(int(rhs.x)), y(int(rhs.y)){}
		int&       operator[](int i);
		const int& operator[](int i) const;
	};
	Vec2i operator + (const Vec2i& lhs, const Vec2i& rhs);
	Vec2i operator - (const Vec2i& lhs, const Vec2i& rhs);
	Vec2i operator * (const Vec2i& lhs, Float rhs);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Vec3
    //////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Vec3
	{
		static const Vec3 ZERO;    //deprecated
		static const Vec3 ONE;     //deprecated
		static const Vec3 UNIT_X;  //deprecated
		static const Vec3 UNIT_Y;  //deprecated
		static const Vec3 UNIT_Z;  //deprecated

        static Vec3 zero()
        {
            return ZERO;
        }
        static Vec3 one()
        {
            return ONE;
        }
        static Vec3 unit_x()
        {
            return UNIT_X;
        }
        static Vec3 unit_y()
        {
            return UNIT_Y;
        }
        static Vec3 unit_z()
        {
            return UNIT_Z;
        }
		union
		{
			struct { Float x, y, z; };
			struct { Float r, g, b; };
            struct { Float u, v, w; };
		};

		Vec3();
		Vec3(const Vec3& rhs);
		Vec3(const Float xval, const Float yval, const Float zval);
		Vec3(const Vec4& rhs);
		Vec3(const Vec2& rhs);

		Vec3& set(Float x, Float y, Float z);
        Vec3& set(const Float* dst, uint32_t n);

        const Vec3& operator = (const Vec3& rhs);
        const Vec3& operator = (const Vec4& rhs);
        const Vec3& operator = (const Vec2& rhs);
		const Vec3& operator = (const Float  rhs);

		operator Float*();
		operator const Float*() const;

		Vec3 operator +() const;
		Vec3 operator -() const;

        const Vec3& operator +=(const Vec3& rhs);
        const Vec3& operator -=(const Vec3& rhs);
        const Vec3& operator *=(const Vec3& rhs);
        //const Vec3& operator *=(const Mat4& rhs);
        const Vec3& operator *=(const Float rhs);
        const Vec3& operator /=(const Float rhs);

		Vec3 operator +(const Vec3 &rhs) const;
		Vec3 operator -(const Vec3 &rhs) const;
		Vec3 operator *(const Vec3 &rhs) const;
		//Vec3 operator *(const Mat4 &rhs) const;
		Vec3 operator *(const Float rhs) const;
		Vec3 operator /(const Float rhs) const;

		Float length() const;
		Float lengthSq() const;

		Vec3& normalize();
		bool  normalized() const;
        Vec3  getNormalize() const;

		Vec3& clamp(const Float lower, const Float upper);
		Vec3& saturate();

        Float dot     (const Vec3& rhs) const;
		Vec3  cross   (const Vec3& rhs) const;
		Vec3& setCross(const Vec3& rhs);

        Vec3& setLerp(const Vec3& rhs, Float factor);
        Vec3  lerp   (const Vec3& rhs, Float factor) const;
	};
    Float dot  (const Vec3& lhs, const Vec3& rhs);
    Vec3  cross(const Vec3& lhs, const Vec3& rhs);
    Vec3  lerp (const Vec3& lhs, const Vec3& rhs, Float factor);
    Vec3  min  (const Vec3& lhs, const Vec3& rhs);
    Vec3  max  (const Vec3& lhs, const Vec3& rhs);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Vec4
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Vec4
	{
		static const Vec4 ZERO;      //deprecated
		static const Vec4 ONE;       //deprecated
                                     //deprecated
		static const Vec4 BLACK;     //deprecated
		static const Vec4 WHITE;     //deprecated
                                     //deprecated
        static const Vec4 RED;       //deprecated
        static const Vec4 GREEN;     //deprecated
        static const Vec4 BLUE;      //deprecated

        static Vec4 zero()
        {
            return ZERO;
        }
        static Vec4 one()
        {
            return ONE;
        }
        static Vec4 black()
        {
            return BLACK;
        }
        static Vec4 white()
        {
            return WHITE;
        }
        static Vec4 red()
        {
            return RED;
        }
        static Vec4 green()
        {
            return GREEN;
        }
        static Vec4 blue()
        {
            return BLUE;
        }

		union
		{
			struct { Float x, y, z, w; };
			struct { Float r, g, b, a; };
		};

		Vec4();
		Vec4(Float x);
		Vec4(Float x, Float, Float z, Float w);
        Vec4(const Vec4& rhs);
        Vec4(const Vec3& rhs, Float wval = Float(0));
        Vec4(const Vec2& rhs);
        Vec4(const Vec4& rhs, uint32_t n);
        Vec4(const Float* rhs, uint32_t n);
		const Vec4& operator =(const Vec4 &rhs);
		const Vec4& operator =(const Vec3 &rhs);
		const Vec4& operator =(const Vec2 &rhs);
		const Vec4& operator =(const Float rhs);

		operator Float*();
		operator const Float*() const;

		Vec4 operator +() const;
		Vec4 operator -() const;

		const Vec4& operator +=(const Vec4& rhs);
		const Vec4& operator -=(const Vec4& rhs);
		const Vec4& operator *=(const Vec4& rhs);
		const Vec4& operator *=(const Mat4& rhs);
		const Vec4& operator *=(const Float rhs);
		const Vec4& operator /=(const Float rhs);

		Vec4 operator +(const Vec4& rhs) const;
		Vec4 operator -(const Vec4& rhs) const;
		Vec4 operator *(const Vec4& rhs) const;
		Vec4 operator *(const Mat4& rhs) const; // Vec4为Row向量
		Vec4 operator *(const Float rhs) const;
		Vec4 operator /(const Float rhs) const;

		Float length() const;
		Float lengthSq() const;

		Float dot(const Vec4& rhs) const;

		Vec4& normalize();
		Vec4& homogenize();

		Vec4& clamp(const Float lower, const Float upper);
		Vec4& saturate();

		Vec4  lerp(const Vec4& rhs, Float factor) const;
		Vec4& setLerp(const Vec4& rhs, Float factor);
        Vec4& setZero();
		void  copyTo(Float* dst, uint32_t n) const;
        Vec4& set(const Float* dst, uint32_t n);
        bool equal(const Vec4& rhs, Float epsilon = FLT_EPSILON) const;
        //swizzle
        Vec2  xy () const;
        Vec2  zw () const;
        Vec3  xyz() const;
        Vec3  rgb() const;
        Vec2  uv () const;
        Vec3  uvw() const;
        Vec4& rgb (const Vec3& val);
        Vec4& rgba(const Vec3& color, Float alpha);

	};
	Vec4 lerp(const Vec4& lhs, const Vec4& rhs, Float factor);
    bool operator == (const Vec4& lhs, const Vec4& rhs);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Quat
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Quat
	{
	public:
		static const Quat IDENTITY;
	public:
		Float x, y, z, w;
    public:
		Quat();
		Quat(const Quat &rhs);
		Quat(const Float x, const Float y, const Float z, const Float w);

		const Quat &operator =(const Quat &rhs);

		operator Float*();
		operator const Float*() const;

		Quat operator +() const;
		Quat operator -() const;

		const Quat &operator +=(const Quat &rhs);
		const Quat &operator -=(const Quat &rhs);
		const Quat &operator *=(const Quat &qVal);
		const Quat &operator *=(const Float rhs);
		const Quat &operator /=(const Float rhs);

		Quat operator +(const Quat &rhs) const;
		Quat operator -(const Quat &rhs) const;
		Quat operator *(const Quat &rhs) const;
		Quat operator *(const Float rhs) const;
		Quat operator /(const Float rhs) const;

		Float length() const;
		Float lengthSq() const;

		Quat& normalize();

        Quat  slerp   (const Quat& rhs, Float factor) const;
        Quat& setSlerp(const Quat& rhs, Float factor);
	};
    Quat slerp(const Quat& lhs, const Quat& rhs, Float factor);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Mat4
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	struct Mat4
	{
	public:
		static const Mat4 IDENTITY;
		static const Mat4 ZERO;
	public:
		Mat4();
		Mat4(const Mat4& rhs);
		Mat4(Float m11, Float m12, Float m13, Float m14,
			 Float m21, Float m22, Float m23, Float m24,
			 Float m31, Float m32, Float m33, Float m34,
			 Float m41, Float m42, Float m43, Float m44);

		Mat4(const Vec4& row0, const Vec4& row1, const Vec4& row2, const Vec4& row3);
        operator       Float*();
        operator const Float*() const;

		Float operator  ()(const uint32_t r, const uint32_t c) const;
		Mat4  operator +() const;
		Mat4  operator -() const;


        const Mat4& operator = (const Mat4& rhs);
		const Mat4& operator +=(const Mat4& rhs);
		const Mat4& operator -=(const Mat4& rhs);
		const Mat4& operator *=(const Mat4& rhs);
		const Mat4& operator *=(const Float rhs);
		const Mat4& operator /=(const Float rhs);

		Float determinant() const;

        Mat4& transpose();
		Mat4  getTranspose() const;

        Mat4& negate();
        Mat4  getNegate() const;

        Mat4& inverse();
        Mat4  getInverse()   const;

        Vec4  getColumn(size_t c) const;
        Vec4  getRow(size_t r)    const;

	public:
		// 数学上为行矩阵，且按行存储
		union
		{
            struct { Float m[4][4]; };
			struct
			{
				Float _11, _12, _13, _14;
				Float _21, _22, _23, _24;
				Float _31, _32, _33, _34;
				Float _41, _42, _43, _44;
			};
		};
	};
	Mat4 operator +(const Mat4& lhs, const Mat4& rhs);
	Mat4 operator -(const Mat4& lhs, const Mat4& rhs);
	Mat4 operator *(const Mat4& lhs, const Mat4& rhs);
	Mat4 operator *(const Mat4& lhs, const Float rhs);
	Mat4 operator /(const Mat4& lhs, const Float rhs);

	/////////////////////////////////////////////////////////////////////////////////////////
	// Transform
	/////////////////////////////////////////////////////////////////////////////////////////
    using AxisAngle = std::pair<Vec3, Float>;
	class Transform
	{
	public:
		static  Mat4 scale(Float x, Float y, Float z);
		static  Mat4 scale(const Vec3& s);

		static  Mat4 translate(Float x, Float y, Float z);
		static  Mat4 translate(const Vec3& t);

		static  Mat4 rotateX(Float radian);
		static  Mat4 rotateY(Float radian);
		static  Mat4 rotateZ(Float radian);

		static  Mat4 rotateEular(Float yawRadian, Float pitchRadian, Float rollRadian);
		static  Mat4 rotateEular(const Vec3& radianYPR);
		static  Mat4 rotateAxis (const Vec3& axis, Float radian);
        static  Mat4 rotateQuat (const Quat& q);

		static  Mat4 lookAtLH(const Vec3& eye, const Vec3& at, const Vec3& up); // WorldSpace -> ViewSpace
		static  Mat4 lookAtRH(const Vec3& eye, const Vec3& at, const Vec3& up); // WorldSpace -> ViewSpace

		static  Mat4 projectOrthoLH(Float width, Float height, Float zNear, Float zFar);
		static  Mat4 projectOrthoRH(Float width, Float height, Float zNear, Float zFar);
		static  Mat4 projectOrthoOffCenterLH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar);
		static  Mat4 projectOrthoOffCenterRH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar);


        static  Mat4 projectPerspLH(Float width, Float height, Float zNear, Float zFar);
        static  Mat4 projectPerspRH(Float width, Float height, Float zNear, Float zFar);
		static  Mat4 projectPerspFovLH(Float yFov, Float aspect, Float zNear, Float zFar);
		static  Mat4 projectPerspFovRH(Float yFov, Float aspect, Float zNear, Float zFar);
		static  Mat4 projectPerspOffCenterLH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar);
		static  Mat4 projectPerspOffCenterRH(Float left, Float right, Float bottom, Float top, Float zNear, Float zFar);

        static  Mat4 viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Float zNear, Float zFar);
    public:
        static  Vec3 transformVector(const Vec3& lhs, const Mat4& rhs);
        static  Vec3 getTranslation(const Mat4& m);
        static  Quat getRotation(const Mat4& m);
        static  AxisAngle getAxisAngle(const Quat& q);
    public:
        struct Project
        {
            Float m_width, m_height, m_zNear, m_zFar;

            Project& width(Float val)
            {
                m_width = val;
                return *this;
            }
            Project& height(Float val)
            {
                m_height = val;
                return *this;
            }
            Project& zNear(Float val)
            {
                m_zNear = val;
                return *this;
            }
            Project& zFar(Float val)
            {
                m_zFar = val;
                return *this;
            }
            Mat4 perspLH() const
            {
                return Transform::projectPerspLH(m_width, m_height, m_zNear, m_zFar);
            }
            Mat4 perspRH() const
            {
                return Transform::projectPerspRH(m_width, m_height, m_zNear, m_zFar);
            }
            Mat4 orthoLH() const
            {
                return Transform::projectOrthoLH(m_width, m_height, m_zNear, m_zFar);
            }
            Mat4 orthoRH() const
            {
                return Transform::projectOrthoRH(m_width, m_height, m_zNear, m_zFar);
            }
        };
        struct ProjectFov
        {
            Float m_aspect, m_yFov, m_zNear, m_zFar;

            ProjectFov& aspect(Float val)
            {
                m_aspect = val;
                return *this;
            }
            ProjectFov& yFov(Float val)
            {
                m_yFov = val;
                return *this;
            }
            ProjectFov& zNear(Float val)
            {
                m_zNear = val;
                return *this;
            }
            ProjectFov& zFar(Float val)
            {
                m_zFar = val;
                return *this;
            }
            Mat4 LH() const
            {
                return Transform::projectPerspFovLH(m_yFov, m_aspect, m_zNear, m_zFar);
            }
            Mat4 RH() const
            {
                return Transform::projectPerspFovRH(m_yFov, m_aspect, m_zNear, m_zFar);
            }
        };
        struct LookAt
        {
            Vec3 m_eye, m_at, m_up;

            LookAt& eye(const Vec3& val)
            {
                m_eye = val;
                return *this;
            }
            LookAt& at(const Vec3& val)
            {
                m_at = val;
                return *this;
            }
            LookAt& up(const Vec3& val)
            {
                m_up = val;
                return *this;
            }
            Mat4 LH() const
            {
                return Transform::lookAtLH(m_eye, m_at, m_up);
            }
            Mat4 RH() const
            {
                return Transform::lookAtRH(m_eye, m_at, m_up);
            }
        };
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	// Rect
	/*
	    在如下坐标系中
	    ---------------------->
		|
		|
		|
		|
		|
		|
	   \|/
	*/
	struct Rect
	{
	public:
		// 范围为: [Left -> Right ),[Bottom -> Top), 
		// 即为左闭右开,下闭上开
		uint32_t left, top;
		uint32_t right, bottom;
	public:
		Rect(uint32_t l, uint32_t t, uint32_t r, uint32_t b)
			: left(l), top(t), right(r), bottom(b)
		{}
		Rect() : Rect(0, 0, 0, 0) {};
		Rect(const Mat4& xfm)
		{
			this->set(xfm);
		}
		bool contains(const Rect& rhs) const
		{
			return (this->left <= rhs.left   && rhs.left   <  this->right)
				&& (this->left <  rhs.right  && rhs.right  <= this->right)
				&& (this->top  <= rhs.top    && rhs.top    <  this->bottom)
				&& (this->top  <  rhs.bottom && rhs.bottom <= this->bottom);
		}
		bool isNormal() const
		{
			return this->left < this->right && this->top < this->bottom;;
		}
		bool isValid() const
		{
			return this->left <= this->right && this->top <= this->bottom;
		}
		bool isVoid() const
		{
			assert(this->isValid());
			return this->left == this->right || this->top == this->bottom;
		}
		uint32_t getWidth() const
		{
			assert(this->isValid());
			return this->right - this->left;
		}
		uint32_t getHeight() const
		{
			assert(this->isValid());
			return this->bottom - this->top;
		}
		// xfm: viewport transformation;
		void set(const Mat4& xfm)
		{
			this->left   = uint32_t(xfm._41 - xfm._11);
			this->right  = uint32_t(xfm._41 + xfm._11);
			this->top    = uint32_t(xfm._42 + xfm._22);
			this->bottom = uint32_t(xfm._42 - xfm._22);
		}
		bool operator == (const Rect& rhs) const
		{
			return this->left   == rhs.left
				&& this->right  == rhs.right
				&& this->top    == rhs.top
				&& this->bottom == rhs.bottom;
		}
	};
    /////////////////////////////////////////////////////////////////////////////////////////
    //Plane: n*x + d = 0;
    struct Plane
    {
        Vec3 normal;
        Float d;

        Plane();
        Plane(const Plane& rhs);
        Plane(const Float a, const Float b, const Float c, const Float dd);
        Plane(const Vec3& n, const Float d);

        const Plane& operator =(const Plane& rhs);

        operator       Float*();
        operator const Float*() const;

        Plane operator +() const;
        Plane operator -() const;
        //non-homo dist
        Float distanceTo(const Vec3& rhs) const;
        //homo dist
        Float distanceTo(const Vec4& rhs) const;
        Plane& setNormal(const Vec3& val)
        {
            normal = val;
            return *this;
        }
        Plane& setD(Float val)
        {
            d = val;
            return *this;
        }
    };
}//ns rl
namespace rl 
{
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //  Vec2 inline
	inline Vec2::Vec2() {}
	inline Vec2::Vec2(const Vec2 &rhs)
		: x(rhs.x), y(rhs.y)
	{}
	inline Vec2::Vec2(const Float x, const Float y)
		: x(x), y(y)
	{}
	inline const Vec2 &Vec2::operator =(const Vec2 &rhs)
	{
		x = rhs.x; y = rhs.y;
		return *this;
	}


	inline const Vec2 &Vec2::operator =(const Float rhs)
	{
		x = rhs; y = 0.0f;
		return *this;
	}

	inline Vec2::operator Float*() { return &x; }
	inline Vec2::operator const Float*() const { return &x; }

	inline Vec2 Vec2::operator +() const { return *this; }
	inline Vec2 Vec2::operator -() const { return Vec2(-x, -y); }

	inline const Vec2 &Vec2::operator +=(const Vec2 &rhs)
	{
		x += rhs.x; y += rhs.y;
		return *this;
	}

	inline const Vec2 &Vec2::operator -=(const Vec2 &rhs)
	{
		x -= rhs.x; y -= rhs.y;
		return *this;
	}

	inline const Vec2 &Vec2::operator *=(const Vec2 &rhs)
	{
		x *= rhs.x; y *= rhs.y;
		return *this;
	}

	inline const Vec2 &Vec2::operator *=(const Float rhs)
	{
		x *= rhs; y *= rhs;
		return *this;
	}

	inline const Vec2 &Vec2::operator /=(const Float rhs)
	{
		const Float fInvVal = 1.0f / rhs;
		x *= fInvVal; y *= fInvVal;
		return *this;
	}

	inline Vec2 Vec2::operator +(const Vec2 &rhs) const
	{
		return Vec2(x + rhs.x, y + rhs.y);
	}

	inline Vec2 Vec2::operator -(const Vec2 &rhs) const
	{
		return Vec2(x - rhs.x, y - rhs.y);
	}

	inline Vec2 Vec2::operator *(const Vec2 &rhs) const
	{
		return Vec2(x * rhs.x, y * rhs.y);
	}

	inline Vec2 Vec2::operator *(const Float rhs) const
	{
		return Vec2(x * rhs, y * rhs);
	}

	inline Vec2 Vec2::operator /(const Float rhs) const
	{
		const Float fInv = 1.0f / rhs;
		return Vec2(x * fInv, y * fInv);
	}

	inline Float Vec2::length() const
	{
		return sqrtf(x * x + y * y);
	}

	inline Float Vec2::lengthSq() const
	{
		return x * x + y * y;
	}
	inline Vec2& Vec2::normalize()
	{
		const Float fLength = length();
		//if( fLength >= FLT_EPSILON )
		{
			const Float fInvLength = 1.0f / fLength;
			x *= fInvLength; y *= fInvLength;
		}
		return *this;
	}
    inline Float Vec2::dot(const Vec2& rhs) const
    {
        return this->x * rhs.x + this->y * rhs.y;
    }
    inline Vec2& Vec2::setLerp(const Vec2& rhs, Float factor)
    {
        this->x += (rhs.x - this->x) * factor;
        this->y += (rhs.y - this->y) * factor;
        return *this;
    }
    inline Vec2 Vec2::lerp(const Vec2& rhs, Float factor) const
    {
        return  Vec2(*this).setLerp(rhs, factor);
    }
    //---------------------------------------------------------------------
    inline Float dot(const Vec2& lhs, const Vec2& rhs)
    {
        return lhs.dot(rhs);
    }
    inline Vec2 lerp(const Vec2& lhs, const Vec2& rhs, Float factor)
    {
        return lhs.lerp(rhs, factor);
    }
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Vec2i inline
	inline int& Vec2i::operator [](int i)
	{
		assert(0 <= i && i < 2);
		return this->raw[i];
	}
	inline const int& Vec2i::operator[](int i) const
	{
		assert(0 <= i && i < 2);
		return this->raw[i];
	}
	inline Vec2i operator + (const Vec2i& lhs, const Vec2i& rhs)
	{
		return { lhs.x + rhs.x, lhs.y + rhs.y };
	}
	inline Vec2i operator -(const Vec2i& lhs, const Vec2i& rhs)
	{
		return { lhs.x - rhs.x, lhs.y - rhs.y };
	}
	inline Vec2i operator *(const Vec2i& lhs, Float rhs)
	{
		return { (int)(lhs.x * rhs), (int)(lhs.y * rhs) };
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	inline Vec3::Vec3() {}
	inline Vec3::Vec3(const Vec3 &rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z)
	{}
	inline Vec3::Vec3(const Float x, const Float y, const Float z)
		: x(x), y(y), z(z)
	{}


	inline Vec3::Vec3(const struct Vec2 &rhs)
	{
		x = rhs.x; y = rhs.y;
		z = 0.0f;
	}
	inline Vec3& Vec3::set(Float x, Float y, Float z)
	{
		this->x = x; this->y = y; this->z = z;
        return *this;
	}
    inline Vec3& Vec3::set(const Float* dst, uint32_t n)
    {
        assert(1 <= n && n <= 3);
        switch(n)
        {
        case 3: z = dst[2];
        case 2: y = dst[1];
        case 1: x = dst[0];
        }
        return *this;
    }
	inline const Vec3 &Vec3::operator =(const Vec3 &rhs)
	{
		x = rhs.x; y = rhs.y; z = rhs.z;
		return *this;
	}
	inline const Vec3 &Vec3::operator =(const Vec2 &rhs)
	{
		x = rhs.x; y = rhs.y; z = 0.0f;
		return *this;
	}
	inline const Vec3 &Vec3::operator =(const Float rhs)
	{
		x = rhs; y = 0.0f; z = 0.0f;
		return *this;
	}

	inline Vec3::operator Float*()
	{
		return &x;
	}
	inline Vec3::operator const Float*() const
	{
		return &x;
	}

	inline Vec3 Vec3::operator +() const
	{
		return *this;
	}
	inline Vec3 Vec3::operator -() const
	{
		return Vec3(-x, -y, -z);
	}

	inline const Vec3 &Vec3::operator +=(const Vec3 &rhs)
	{
		x += rhs.x; y += rhs.y; z += rhs.z;
		return *this;
	}

	inline const Vec3 &Vec3::operator -=(const Vec3 &rhs)
	{
		x -= rhs.x; y -= rhs.y; z -= rhs.z;
		return *this;
	}
	inline const Vec3 &Vec3::operator *=(const Vec3 &rhs)
	{
		x *= rhs.x; y *= rhs.y; z *= rhs.z;
		return *this;
	}


	inline const Vec3 &Vec3::operator *=(const Float rhs)
	{
		x *= rhs; y *= rhs; z *= rhs;
		return *this;
	}

	inline const Vec3 &Vec3::operator /=(const Float rhs)
	{
		const Float fInvVal = 1.0f / rhs;
		x *= fInvVal; y *= fInvVal; z *= fInvVal;
		return *this;
	}

	inline Vec3 Vec3::operator +(const Vec3 &rhs) const
	{
		return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
	}
	inline Vec3 Vec3::operator -(const Vec3 &rhs) const
	{
		return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
	}
	inline Vec3 Vec3::operator *(const Vec3 &rhs) const
	{
		return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
	}
	inline Vec3 Vec3::operator *(const Float rhs) const
	{
		return Vec3(x * rhs, y * rhs, z * rhs);
	}

	inline Vec3 Vec3::operator /(const Float rhs) const
	{
		const Float fInv = 1.0f / rhs;
		return Vec3(x * fInv, y * fInv, z * fInv);
	}

	inline Float Vec3::length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	inline Float Vec3::lengthSq() const
	{
		return x * x + y * y + z * z;
	}

	inline Vec3& Vec3::normalize()
	{
		const Float fLength = length();
		//if( fLength >= FLT_EPSILON )
		{
			const Float fInvLength = 1.0f / fLength;
			x *= fInvLength; y *= fInvLength; z *= fInvLength;
		}
		return *this;
	}
	inline bool Vec3::normalized() const
	{
		return std::abs(this->length() - Float(1)) < std::numeric_limits<Float>::epsilon();
	}
    inline Vec3 Vec3::getNormalize() const
    {
        return Vec3(*this).normalize();
    }
	inline Vec3 &Vec3::clamp(const Float lower, const Float upper)
	{
		x = rl::clamp(x, lower, upper);
		y = rl::clamp(y, lower, upper);
		z = rl::clamp(z, lower, upper);
		return *this;
	}

	inline Vec3 &Vec3::saturate()
	{
		return this->clamp(0, 1);
	}

	inline Vec3& Vec3::setCross(const Vec3& rhs)
	{
		auto x = this->y*rhs.z - this->z*rhs.y;
		auto y = this->z*rhs.x - this->x*rhs.z;
		auto z = this->x*rhs.y - this->y*rhs.x;
		this->set(x, y, z);
		return *this;
	}
	inline Vec3 Vec3::cross(const Vec3& rhs) const
	{
		return Vec3(*this).setCross(rhs);
	}
	inline Float Vec3::dot(const Vec3& rhs) const
	{
		return this->x * rhs.x + this->y * rhs.y + this->z * rhs.z;
	}
    inline Vec3& Vec3::setLerp(const Vec3& rhs, Float factor)
    {
        this->x += (rhs.x - this->x) * factor;
        this->y += (rhs.y - this->y) * factor;
        this->z += (rhs.z - this->z) * factor;
        return *this;
    }
    inline Vec3 Vec3::lerp(const Vec3& rhs, Float factor) const
    {
        return Vec3(*this).setLerp(rhs, factor);
    }
    //-------------
    inline Float dot(const Vec3& lhs, const Vec3& rhs)
    {
        return lhs.dot(rhs);
    }
    inline Vec3 cross(const Vec3& lhs, const Vec3& rhs)
    {
        return lhs.cross(rhs);
    }
    inline Vec3 lerp(const Vec3& lhs, const Vec3& rhs, Float factor)
    {
        return lhs.lerp(rhs, factor);
    }
    inline Vec3 min(const Vec3& lhs, const Vec3& rhs)
    {
        return Vec3(std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z));
    }
    inline Vec3 max(const Vec3& lhs, const Vec3& rhs)
    {
        return Vec3(std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z));
    }
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//Vec4
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	inline Vec4::Vec4() {}
	inline Vec4::Vec4(const Vec4 &rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
	{}
	inline Vec4::Vec4(const Float x, const Float y, const Float z, const Float w)
		: x(x), y(y), z(z), w(w)
	{}
	inline Vec4::Vec4(Float x) : Vec4(x, Float(0), Float(0), Float(0))
	{

	}
	inline Vec4::Vec4(const Vec3& rhs, Float wval)
	{
		x = rhs.x; y = rhs.y;
		z = rhs.z; w = wval;
	}

	inline Vec4::Vec4(const Vec2 &rhs)
	{
		x = rhs.x; y = rhs.y;
		z = 0.0f;  w = 0.0f;
	}
	inline Vec4::Vec4(const Vec4 &rhs, uint32_t n)
		: Vec4(0, 0, 0, 1)
	{
		assert(1 <= n && n <= 4);
		switch (n)
		{
		case 4: w = rhs.w;
		case 3: z = rhs.z;
		case 2: y = rhs.y;
		case 1: x = rhs.x;
		}
	}
	inline Vec4::Vec4(const Float *rhs, uint32_t n)
		: Vec4(0, 0, 0, 1)
	{
		assert(1 <= n && n <= 4);
		switch (n)
		{
		case 4: w = rhs[3];
		case 3: z = rhs[2];
		case 2: y = rhs[1];
		case 1: x = rhs[0];
		}
	}
	inline const Vec4& Vec4::operator =(const Vec4& rhs)
	{
		x = rhs.x; y = rhs.y;
		z = rhs.z; w = rhs.w;
		return *this;
	}

	inline const Vec4& Vec4::operator =(const Vec3& rhs)
	{
		x = rhs.x; y = rhs.y;
		z = rhs.z; w = 0.0f;
		return *this;
	}
	inline const Vec4& Vec4::operator = (const Vec2& rhs)
	{
		x = rhs.x; y = rhs.y;
		z = 0.0f; w = 0.0f;
		return *this;
	}
	inline const Vec4& Vec4::operator =(const Float rhs)
	{
		x = rhs; y = 0.0f;
		z = 0.0f; w = 0.0f;
		return *this;
	}
	inline Vec4::operator Float*()
	{
		return &x;
	}
	inline Vec4::operator const Float*() const
	{
		return &x;
	}
	inline Vec4 Vec4::operator +() const
	{
		return *this;
	}
	inline Vec4 Vec4::operator -() const
	{
		return Vec4(-x, -y, -z, -w);
	}

	inline const Vec4 &Vec4::operator +=(const Vec4 &rhs)
	{
		x += rhs.x; y += rhs.y;
		z += rhs.z; w += rhs.w;
		return *this;
	}

	inline const Vec4 &Vec4::operator -=(const Vec4 &rhs)
	{
		x -= rhs.x; y -= rhs.y;
		z -= rhs.z; w -= rhs.w;
		return *this;
	}

	inline const Vec4 &Vec4::operator *=(const Vec4 &rhs)
	{
		x *= rhs.x; y *= rhs.y;
		z *= rhs.z; w *= rhs.w;
		return *this;
	}

	inline const Vec4 &Vec4::operator *=(const Float rhs)
	{
		x *= rhs; y *= rhs;
		z *= rhs; w *= rhs;
		return *this;
	}

	inline const Vec4 &Vec4::operator /=(const Float rhs)
	{
		const Float inv = 1.0f / rhs;
		x *= inv; y *= inv;
		z *= inv; w *= inv;
		return *this;
	}

	inline Vec4 Vec4::operator +(const Vec4 &rhs) const
	{
		return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	inline Vec4 Vec4::operator -(const Vec4 &rhs) const
	{
		return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	inline Vec4 Vec4::operator *(const Vec4 &rhs) const
	{
		return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
	}

	inline Vec4 Vec4::operator *(const Float rhs) const
	{
		return Vec4(x * rhs, y * rhs, z * rhs, w * rhs);
	}

	inline Vec4 Vec4::operator /(const Float rhs) const
	{
		const Float fInv = 1.0f / rhs;
		return Vec4(x * fInv, y * fInv, z * fInv, w * fInv);
	}

	inline Float Vec4::length() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	inline Float Vec4::lengthSq() const
	{
		return x * x + y * y + z * z + w * w;
	}

	inline Vec4 &Vec4::normalize()
	{
		const Float fLength = length();
		//if( fLength >= FLT_EPSILON )
		{
			const Float fInvLength = 1.0f / fLength;
			x *= fInvLength; y *= fInvLength;
			z *= fInvLength; w *= fInvLength;
		}
		return *this;
	}

	inline Vec4 &Vec4::homogenize()
	{
		//if( w != 1.0f && fabsf( w ) >= FLT_EPSILON )
		{
			const Float fInvW = 1.0f / w;
			x *= fInvW; y *= fInvW;
			z *= fInvW; w = 1.0f;
		}
		return *this;
	}

	inline Vec4& Vec4::clamp(const Float lower, const Float upper)
	{
		x = rl::clamp(x, lower, upper);
		y = rl::clamp(y, lower, upper);
		z = rl::clamp(z, lower, upper);
		w = rl::clamp(w, lower, upper);
		return *this;
	}
	inline void Vec4::copyTo(Float* dst, uint32_t n) const
	{
		assert(1 <= n && n <= 4);
		switch (n)
		{
		case 4: dst[3] = this->w;
		case 3: dst[2] = this->z;
		case 2: dst[1] = this->y;
		case 1: dst[0] = this->x;
		}
        //assert(this->x > 0.00001f);
	}
    inline Vec4& Vec4::set(const Float* dst, uint32_t n)
    {
        assert(1 <= n && n <= 4);
        switch(n)
        {
        case 4: this->w = dst[3];
        case 3: this->z = dst[2];
        case 2: this->y = dst[1];
        case 1: this->x = dst[0];
        }
        return *this;
    }
    inline bool Vec4::equal(const Vec4& rhs, Float epsilon) const
    {
        return rl::equal(this->x, rhs.x, epsilon)
            && rl::equal(this->y, rhs.y, epsilon)
            && rl::equal(this->z, rhs.z, epsilon)
            && rl::equal(this->w, rhs.w, epsilon);
    }
	inline Vec4 &Vec4::saturate()
	{
		return this->clamp(0, 1);
	}

	inline Vec4& Vec4::setLerp(const Vec4& rhs, Float factor)
	{
		(*this) += (rhs - (*this))*factor;
        return *this;
	}
	inline Vec4 Vec4::lerp(const Vec4& rhs, Float factor) const
	{
		auto lhs(*this);
		lhs.setLerp(rhs, factor);
		return lhs;
	}
    inline Vec4& Vec4::setZero()
    {
        *this = Vec4::ZERO;
        return *this;
    }
	inline Float Vec4::dot(const Vec4& rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
	}
    inline Vec3 Vec4::xyz() const
    {
        return Vec3(x, y, z);
    }
    inline Vec2 Vec4::xy() const
    {
        return Vec2(x, y);
    }
    inline Vec2 Vec4::zw() const
    {
        return Vec2(z, w);
    }
    inline Vec3 Vec4::rgb() const
    {
        return this->xyz();
    }
    inline Vec4& Vec4::rgb(const Vec3& val)
    {
        this->r = val.r;
        this->g = val.g;
        this->b = val.b;
        return *this;
    }
    inline Vec4& Vec4::rgba(const Vec3& color, Float alpha)
    {
        this->r = color.r;
        this->g = color.g;
        this->b = color.b;
        this->a = alpha;
        return *this;
    }
    inline Vec2  Vec4::uv() const
    {
        return this->xy();
    }
    inline Vec3  Vec4::uvw() const
    {
        return this->xyz();
    }
	///////////////////////////////////////////////////////
	inline Vec4 lerp(const Vec4& lhs, const Vec4& rhs, Float factor)
	{
		return lhs.lerp(rhs, factor);
	}
    inline bool operator == (const Vec4& lhs, const Vec4& rhs)
    {
        return lhs.equal(rhs, FLT_EPSILON*10);
    }
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//Quat
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	inline Quat::Quat() {}
	inline Quat::Quat(const Quat &rhs)
		: x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
	{}
	inline Quat::Quat(const Float x, const Float y, const Float z, const Float w)
		: x(x), y(y), z(z), w(w)
	{}

	inline const Quat &Quat::operator =(const Quat &rhs)
	{
		x = rhs.x; y = rhs.y;
		z = rhs.z; w = rhs.w;
		return *this;
	}

	inline Quat::operator Float*() { return &x; }
	inline Quat::operator const Float*() const { return &x; }

	inline Quat Quat::operator +() const { return *this; }
	inline Quat Quat::operator -() const { return Quat(-x, -y, -z, w); }

	inline const Quat &Quat::operator +=(const Quat &rhs)
	{
		x += rhs.x; y += rhs.y;
		z += rhs.z; w += rhs.w;
		return *this;
	}

	inline const Quat &Quat::operator -=(const Quat &rhs)
	{
		x -= rhs.x; y -= rhs.y;
		z -= rhs.z; w -= rhs.w;
		return *this;
	}

	inline const Quat &Quat::operator *=(const Quat &qVal)
	{
		Quat qResult;
		qResult.x = w * qVal.x + x * qVal.w + y * qVal.z - z * qVal.y;
		qResult.y = w * qVal.y - x * qVal.z + y * qVal.w + z * qVal.x;
		qResult.z = w * qVal.z + x * qVal.y - y * qVal.x + z * qVal.w;
		qResult.w = w * qVal.w - x * qVal.x - y * qVal.y - z * qVal.z;
		*this = qResult;

		return *this;
	}

	inline const Quat &Quat::operator *=(const Float rhs)
	{
		x *= rhs; y *= rhs;
		z *= rhs; w *= rhs;
		return *this;
	}

	inline const Quat &Quat::operator /=(const Float rhs)
	{
		const Float fInvVal = 1.0f / rhs;
		x *= fInvVal; y *= fInvVal;
		z *= fInvVal; w *= fInvVal;
		return *this;
	}

	inline Quat Quat::operator +(const Quat &rhs) const
	{
		return Quat(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	inline Quat Quat::operator -(const Quat &rhs) const
	{
		return Quat(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	inline Quat Quat::operator *(const Quat &qVal) const
	{
		Quat qResult;
		qResult.x = w * qVal.x + x * qVal.w + y * qVal.z - z * qVal.y;
		qResult.y = w * qVal.y - x * qVal.z + y * qVal.w + z * qVal.x;
		qResult.z = w * qVal.z + x * qVal.y - y * qVal.x + z * qVal.w;
		qResult.w = w * qVal.w - x * qVal.x - y * qVal.y - z * qVal.z;
		return qResult;
	}

	inline Quat Quat::operator *(const Float rhs) const
	{
		return Quat(x * rhs, y * rhs, z * rhs, w * rhs);
	}

	inline Quat Quat::operator /(const Float rhs) const
	{
		const Float fInv = 1.0f / rhs;
		return Quat(x * fInv, y * fInv, z * fInv, w * fInv);
	}

	inline Float Quat::length() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	inline Float Quat::lengthSq() const
	{
		return x * x + y * y + z * z + w * w;
	}

	inline Quat &Quat::normalize()
	{
		const auto len = this->length();
		//if( fLength >= FLT_EPSILON )
		{
			const auto invLen = 1.0f / len;
			x *= invLen; y *= invLen;
			z *= invLen; w *= invLen;
		}
		return *this;
	}
	 //////////////////////////////////////////////////////////////////////////////////////////////////////
	 //Mat4
	 //////////////////////////////////////////////////////////////////////////////////////////////////////
	inline Mat4::Mat4() {}
	inline Mat4::Mat4(const Mat4& rhs)
		: _11(rhs._11), _12(rhs._12), _13(rhs._13), _14(rhs._14)
		, _21(rhs._21), _22(rhs._22), _23(rhs._23), _24(rhs._24)
		, _31(rhs._31), _32(rhs._32), _33(rhs._33), _34(rhs._34)
		, _41(rhs._41), _42(rhs._42), _43(rhs._43), _44(rhs._44)
	{}
	inline Mat4::Mat4(Float m11, Float m12, Float m13, Float m14,
		              Float m21, Float m22, Float m23, Float m24,
		              Float m31, Float m32, Float m33, Float m34,
		              Float m41, Float m42, Float m43, Float m44)
		: _11(m11), _12(m12), _13(m13), _14(m14),
		  _21(m21), _22(m22), _23(m23), _24(m24),
		  _31(m31), _32(m32), _33(m33), _34(m34),
		  _41(m41), _42(m42), _43(m43), _44(m44)
	{}
	inline Mat4::Mat4(const Vec4& row0, const Vec4& row1, const Vec4& row2, const Vec4& row3)
	{
		_11 = row0.x; _12 = row0.y; _13 = row0.z; _14 = row0.w;
		_21 = row1.x; _22 = row1.y; _23 = row1.z; _24 = row1.w;
		_31 = row2.x; _32 = row2.y; _33 = row2.z; _34 = row2.w;
		_41 = row3.x; _42 = row3.y; _43 = row3.z; _44 = row3.w;
	}
	inline const Mat4& Mat4::operator =(const Mat4 &rhs)
	{
		_11 = rhs._11; _12 = rhs._12; _13 = rhs._13; _14 = rhs._14;
		_21 = rhs._21; _22 = rhs._22; _23 = rhs._23; _24 = rhs._24;
		_31 = rhs._31; _32 = rhs._32; _33 = rhs._33; _34 = rhs._34;
		_41 = rhs._41; _42 = rhs._42; _43 = rhs._43; _44 = rhs._44;
		return *this;
	}

	inline Mat4::operator Float*()
	{
		return &_11;
	}
	inline Mat4::operator const Float*() const
	{
		return &_11;
	}
	inline Float Mat4::operator()(const uint32_t r, const uint32_t c) const
	{
		return m[r][c];
	}

	inline Mat4 Mat4::operator +() const
	{
		return *this;
	}
	inline const Mat4& Mat4::operator +=(const Mat4& rhs)
	{
		_11 += rhs._11; _12 += rhs._12; _13 += rhs._13; _14 += rhs._14;
		_21 += rhs._21; _22 += rhs._22; _23 += rhs._23; _24 += rhs._24;
		_31 += rhs._31; _32 += rhs._32; _33 += rhs._33; _34 += rhs._34;
		_41 += rhs._41; _42 += rhs._42; _43 += rhs._43; _44 += rhs._44;
		return *this;
	}

	inline const Mat4& Mat4::operator -=(const Mat4& rhs)
	{
		_11 -= rhs._11; _12 -= rhs._12; _13 -= rhs._13; _14 -= rhs._14;
		_21 -= rhs._21; _22 -= rhs._22; _23 -= rhs._23; _24 -= rhs._24;
		_31 -= rhs._31; _32 -= rhs._32; _33 -= rhs._33; _34 -= rhs._34;
		_41 -= rhs._41; _42 -= rhs._42; _43 -= rhs._43; _44 -= rhs._44;
		return *this;
	}

	inline const Mat4& Mat4::operator *=(const Float rhs)
	{
		_11 *= rhs; _12 *= rhs; _13 *= rhs; _14 *= rhs;
		_21 *= rhs; _22 *= rhs; _23 *= rhs; _24 *= rhs;
		_31 *= rhs; _32 *= rhs; _33 *= rhs; _34 *= rhs;
		_41 *= rhs; _42 *= rhs; _43 *= rhs; _44 *= rhs;
		return *this;
	}

	inline const Mat4& Mat4::operator /=(const Float rhs)
	{
        assert(rhs != Float(0));
        *this *= (1.0f/rhs);
		return *this;
	}
    inline Vec4 Mat4::getColumn(size_t c) const
    {
        assert(c < 4);
        return Vec4(m[0][c], m[1][c], m[2][c], m[3][c]);
    }
    inline Vec4 Mat4::getRow(size_t r) const
    {
        assert(r < 4);
        return Vec4(m[r][0], m[r][1], m[r][2], m[r][3]);
    }
    inline Mat4  Mat4::getInverse() const
    {
        return Mat4(*this).inverse();
    }
    inline Mat4& Mat4::negate()
    {
        _11 = -_11; _12 = -_12; _13 = -_13; _14 = -_14;
        _21 = -_21; _22 = -_22; _23 = -_23; _24 = -_24;
        _31 = -_31; _32 = -_32; _33 = -_33; _34 = -_34;
        _41 = -_41; _42 = -_42; _43 = -_43; _44 = -_44;
        return *this;
    }
    inline Mat4 Mat4::getNegate() const
    {
        return Mat4(*this).negate();
    }
    inline Mat4 Mat4::operator -() const
    {
        return this->getNegate();
    }
	//////////////////////////////////////////////////////////////////////////////
	inline Mat4 operator +(const Mat4& lhs, const Mat4& rhs)
	{
		return Mat4(lhs) += rhs;
	}

	inline Mat4 operator -(const Mat4& lhs, const Mat4 &rhs)
	{
		return Mat4(lhs) -= rhs;
	}

	inline Mat4 operator * (const Mat4& lhs, const Float rhs)
	{
		return Mat4(lhs) *= rhs;
	}

	inline Mat4 operator /(const Mat4& lhs, const Float rhs)
	{
		return Mat4(lhs) /= rhs;
	}
	inline Mat4 operator *(const Mat4 &lhs, const Mat4 &rhs)
	{
		return Mat4(lhs) *= rhs;
	}
    //////////////////////////////////////////////////////////////////////////////////////
    //
    inline Plane::Plane() {}
    inline Plane::Plane(const Plane &rhs)
        : normal(rhs.normal), d(rhs.d)
    {}
    inline Plane::Plane(const Float a, const Float b, const Float c, const Float dd)
        : d(dd)
        , normal(a, b, c)
    {
    }
    inline Plane::Plane(const Vec3& n, const Float dd)
        : normal(n), d(dd)
    {}

    inline const Plane& Plane::operator =(const Plane &rhs)
    {
        normal = rhs.normal;
        d = rhs.d;
        return *this;
    }

    inline Plane::operator Float*()
    {
        return &normal.x;
    }
    inline Plane::operator const Float*() const
    {
        return &normal.x;
    }
    inline Plane Plane::operator +() const
    {
        return *this;
    }
    inline Plane Plane::operator -() const
    {
        return Plane(-normal, d);
    }

    // Homo: (x,y,z,w), Non-Homo: (X,Y,Z) 
    // 它们的关系是: X = x/w, Y = y/w, Z = z/w
    // non-homo dist = Nx*X + Ny*Y + Nz*Z + d
    // homo     dist = Nx*x + Ny*y + Nz*z + d*w
    // non_homo dist和homo dist的关系:
    // non-homo dist = homo_dist/w

    inline Float Plane::distanceTo(const Vec3& rhs) const
    {//non-homo dist
        return this->normal.dot(rhs) + d;
    }
    inline Float Plane::distanceTo(const Vec4& rhs) const
    {//homo dist
        return this->normal.dot(rhs) + d * rhs.w;
    }
}//ns rb

#endif //RASLITE_MATH_H