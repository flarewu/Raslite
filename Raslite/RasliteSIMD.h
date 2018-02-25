#ifndef RASLITE_SIMD_H
#define RASLITE_SIMD_H

#if defined(_MSC_VER)
#define RL_FORCE_INLINE __forceinline
#else
#define RL_FORCE_INLINE inline __attribute__((always_inline))
#endif
namespace rl {

    template <size_t N>
    inline bool IsAligned(const void* p)
    {
        return (reinterpret_cast<uintptr_t>(p) & (N - 1)) == uintptr_t(0);
    }
}

//#define  RL_SIMD_REF

#if !defined(RL_SIMD_REF)

#if defined(__AVX__)
#include <immintrin.h> 
#define RL_SIMD_AVX
#define RL_SIMD_SSE4_2  
#endif

#if defined(__SSE4_2__) || defined(RL_SIMD_SSE4_2)
#include <nmmintrin.h>
#define RL_SIMD_SSE4_2
#define RL_SIMD_SSE4_1  
#endif

#if defined(__SSE4_1__) || defined(RL_SIMD_SSE4_1)
#include <smmintrin.h>
#define RL_SIMD_SSE4_1
#define RL_SIMD_SSSE3 
#endif

#if defined(__SSSE3__) || defined(RL_SIMD_SSSE3)
#include <tmmintrin.h>
#define RL_SIMD_SSSE3
#define RL_SIMD_SSE3 
#endif

#if defined(__SSE3__) || defined(RL_SIMD_SSE3)
#include <pmmintrin.h>
#define RL_SIMD_SSE3
#define RL_SIMD_SSE2
#endif
#if defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64) || (_M_IX86_FP>=2) || defined(RL_SIMD_SSE2)
#include <emmintrin.h>
#define RL_SIMD_SSE2
#define RL_SIMD_SSEx
#endif
#endif //RL_SIMD_REF

namespace rl
{
#if defined(RL_SIMD_SSEx)
    using SIMDFloat4_t = __m128;
    using SIMDInt4_t   = __m128i;
    // 用于做函数Parameter
    using SIMDFloat4P_t = const SIMDFloat4_t;
    using SIMDInt4P_t   = const SIMDInt4_t;
#else
#define RL_SIMD_REF
    struct SIMDFloat4_t
    {
        alignas(16) float x;
        float y;
        float z;
        float w;
    };
    struct SIMDInt4_t
    {
        alignas(16) int x;
        int y;
        int z;
        int w;
    };
    // 用于做函数Parameter
    using SIMDFloat4P_t = const SIMDFloat4_t&;
    using SIMDInt4P_t   = const SIMDInt4_t&;
#endif// RL_SIMD_SSEx
}//ns rl
namespace rl
{
    struct SIMDFloat4
    {
        // (x:0,y:0,z:0,w:0)
        static SIMDFloat4_t zero();
        // (x:1,y:1,z:1,w:1)
        static SIMDFloat4_t one();
        // (x:1,y:0,z:0,w:0)
        static SIMDFloat4_t unitX();
        // (x:0,y:1,z:0,w:0)
        static SIMDFloat4_t unitY();
        // (x:0,y:0,z:1,w:0)
        static SIMDFloat4_t unitZ();
        // (x:0,y:0,z:0,w:1)
        static SIMDFloat4_t unitW();

        // (x:x,y:y,z:y,w:w)
        static SIMDFloat4_t set(float x, float y, float z, float w);
        // (x:v,y:v,z:v,w:v)
        static SIMDFloat4_t set1(float v);
        // (x:v,y:0,z:0,w:0)
        static SIMDFloat4_t setX(float v);

        //(x:p[0],y:p[1],z:p[2],w:p[3])
        static SIMDFloat4_t load(const float* p);
        //(x:p[0],y:0,z:0,w:0)
        static SIMDFloat4_t loadX(const float* p);

        //(x:p[0],y:p[1],z:p[2],w:p[3])
        static SIMDFloat4_t loadu(const float* p);
        //(x:p[0],y:p[0],z:p[0],w:p[0])
        static SIMDFloat4_t loadu1(const float* p);
        //(x:p[0],y:0,z:0,w:0)
        static SIMDFloat4_t loaduX(const float* p);
        //(x:p[0],y:p[1],z:0,w:0)
        static SIMDFloat4_t loaduXY(const float* p);
        //(x:p[0],y:p[1],z:p[2],w:0)
        static SIMDFloat4_t loaduXYZ(const float* p);
    };
    //////////////////////////////////////////////////////////////////
    namespace simd
    {
        float getX(SIMDFloat4P_t v);
		float getY(SIMDFloat4P_t v);
		float getZ(SIMDFloat4P_t v);
		float getW(SIMDFloat4P_t v);
		SIMDFloat4_t splat(SIMDFloat4P_t v, unsigned int imm);

        SIMDFloat4_t setX(SIMDFloat4P_t v, float f);
        SIMDFloat4_t setY(SIMDFloat4P_t v, float f);
        SIMDFloat4_t setZ(SIMDFloat4P_t v, float f);
        SIMDFloat4_t setW(SIMDFloat4P_t v, float f);
        // (p[0]:x, p[1]:y ,p[2]:z ,p[3]:w)
        void store(SIMDFloat4P_t v, float* p);
        // (p[0]:x)
        void storeX(SIMDFloat4P_t v, float* p);

        // (p[0]:x, p[1]:y ,p[2]:z ,p[3]:w)
        void storeu(SIMDFloat4P_t v, float* p);
        // (p[0]:x)
        void storeuX(SIMDFloat4P_t v, float* p);

        SIMDFloat4_t splatX(SIMDFloat4P_t v);
        SIMDFloat4_t splatY(SIMDFloat4P_t v);
        SIMDFloat4_t splatZ(SIMDFloat4P_t v);
        SIMDFloat4_t splatW(SIMDFloat4P_t v);
    }

    //////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////
    //struct SIMDInt4
    //{
    //    static SIMDInt4_t zero();
    //};
    //////////////////////////////////////////////////////////////////
    //namespace simd
    //{
    //    float getx(SIMDInt4P_t v);
    //}
}//ns rl
#ifdef RL_SIMD_SSEx
namespace rl
{
    //////////////////////////////////////////////////////////////////
    // Float4
    //////////////////////////////////////////////////////////////////

    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::zero()
    {
        return _mm_setzero_ps();
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::one()
    {
        return _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::unitX()
    {
        return _mm_set_ss(1.0f);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::unitY()
    {
        return _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::unitZ()
    {
        return _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::unitW()
    {
       return _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::set(float x, float y, float z, float w)
    {
        return _mm_set_ps(w, z, y, x);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::set1(float v)
    {
        return _mm_set1_ps(v);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::setX(float v)
    {
        return _mm_set_ss(v);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::load(const float* p)
    {
        assert(IsAligned<16>(p));
        return _mm_load_ps(p);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::loadX(const float* p)
    {
        assert(IsAligned<16>(p));
        return _mm_load_ss(p);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::loadu(const float* p)
    {
        assert(IsAligned<4>(p));
        return _mm_loadu_ps(p);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::loadu1(const float* p)
    {
        assert(IsAligned<4>(p));
        return _mm_load_ps1(p);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::loaduX(const float* p)
    {
        assert(IsAligned<4>(p));
        return _mm_load_ss(p);
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::loaduXY(const float* p)
    {
        assert(IsAligned<4>(p));
        //unpacklo([a: p0,0,0,0] ,[b: p1,0,0,0]) => [p0,p1,a1,b1] = [p0,p1,0,0]
        return _mm_unpacklo_ps(_mm_load_ss(p + 0), _mm_load_ss(p + 1));
    }
    RL_FORCE_INLINE SIMDFloat4_t SIMDFloat4::loaduXYZ(const float* p)
    {
        assert(IsAligned<4>(p));
        return _mm_movelh_ps(_mm_unpacklo_ps(_mm_load_ss(p + 0), _mm_load_ss(p + 1)), 
                             _mm_load_ss(p + 2));
    }
    //////////////////////////////////////////////////////////////////
	RL_FORCE_INLINE SIMDFloat4_t simd::splat(SIMDFloat4P_t v, unsigned int i)
	{
		return _mm_shuffle_ps(v,v,_MM_SHUFFLE(i,i,i,i));
	}
    RL_FORCE_INLINE float simd::getX(SIMDFloat4P_t v)
    {
        return _mm_cvtss_f32(v);
    }
    RL_FORCE_INLINE float simd::getY(SIMDFloat4P_t v)
    {
        return _mm_cvtss_f32(simd::splat(v,1));
    }
    RL_FORCE_INLINE float simd::getZ(SIMDFloat4P_t v)
    {
        // 首先把v:xyzw变成v:zwzw;   相当于: splatzw
        // 后在把v:zwxy变成z
        return _mm_cvtss_f32(_mm_movehl_ps(v, v));
    }
    RL_FORCE_INLINE float simd::getW(SIMDFloat4P_t v)
    {
        return _mm_cvtss_f32(simd::splat(v,3));
    }	

    RL_FORCE_INLINE SIMDFloat4_t simd::setX(SIMDFloat4P_t v, float f)
    {
        return _mm_move_ss(v, _mm_set_ss(f));
    }
    RL_FORCE_INLINE SIMDFloat4_t simd::setY(SIMDFloat4P_t v, float f)
    {
        const auto yxzw = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,2,0,1));
        const auto fxzw = _mm_move_ss(yxzw, _mm_set_ss(f));
        return _mm_shuffle_ps(fxzw, fxzw, _MM_SHUFFLE(3,2,0,1));
    }
    RL_FORCE_INLINE SIMDFloat4_t simd::setZ(SIMDFloat4P_t v, float f)
    {
        const auto zxyw = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 1, 0, 2));
        const auto fxyw = _mm_move_ss(zxyw, _mm_set_ss(f));
        return _mm_shuffle_ps(fxyw, fxyw, _MM_SHUFFLE(3, 1, 0, 2));
    }
    RL_FORCE_INLINE SIMDFloat4_t simd::setW(SIMDFloat4P_t v, float f)
    {
        const auto wyzx = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 2, 1, 3));
        const auto fyzx = _mm_move_ss(wyzx, _mm_set_ss(f));
        return _mm_shuffle_ps(fyzx, fyzx, _MM_SHUFFLE(0, 2, 1, 3));
    }

    RL_FORCE_INLINE void simd::store(SIMDFloat4P_t v, float* p)
    {
        assert(IsAligned<16>(p));
        _mm_store_ps(p, v);
    }
    RL_FORCE_INLINE void simd::storeX(SIMDFloat4P_t v, float* p)
    {
        assert(IsAligned<16>(p));
        _mm_store_ss(p, v);
    }
    RL_FORCE_INLINE void simd::storeu(SIMDFloat4P_t v, float* p)
    {
        assert(IsAligned<4>(p));
        _mm_storeu_ps(p, v);
    }
    RL_FORCE_INLINE void simd::storeuX(SIMDFloat4P_t v, float* p)
    {
        assert(IsAligned<4>(p));
        _mm_store_ss(p, v);
    }

    RL_FORCE_INLINE SIMDFloat4_t simd::splatX(SIMDFloat4P_t v)
    {
        return simd::splat(v, 0);
    }
    RL_FORCE_INLINE SIMDFloat4_t simd::splatY(SIMDFloat4P_t v) 
    {
        return simd::splat(v, 1);
    }
    RL_FORCE_INLINE SIMDFloat4_t simd::splatZ(SIMDFloat4P_t v) 
    {
        return simd::splat(v, 2);
    }
    RL_FORCE_INLINE SIMDFloat4_t simd::splatW(SIMDFloat4P_t v) 
    {
        return simd::splat(v, 3);
    }
    //////////////////////////////////////////////////////////////////
    // Int4
    //////////////////////////////////////////////////////////////////
}

#elif defined(RL_SIMD_REF)
#else
#error 未知 SIMD 指令集
#endif
#endif //RASLITE_SIMD_H