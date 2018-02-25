#ifndef RASLITE_COMMON_H
#define RASLITE_COMMON_H
#include <cassert>
#include <cstdint>
#include "RasliteMath.h"
namespace rl {
	constexpr uint32_t VS_REGISTER_COUNT = 8;
	constexpr uint32_t PS_REGISTER_COUNT = 8;

	constexpr uint32_t SHADER_SAMPLER_COUNT = 16;
	constexpr uint32_t SHADER_CONSTANT_COUNT = 32;

	constexpr uint32_t VERTEX_CACHE_CAPACITY = 32;
	constexpr uint32_t VERTEX_STREAM_COUNT = 8;

    constexpr uint8_t RENDER_TARGET_COUNT = 8;

    constexpr uint8_t SIMULTANEOUS_RENDER_TARGET_COUNT = 8;
    constexpr uint8_t SAMPLER_STATE_COUNT = 8;
    constexpr uint8_t SHADER_INPUT_RESOURCE_COUNT = 128;

	enum class Format
	{//后缀: float,sint,uint,snorm,unorm,typeless,sRGB...
		R32_FLOAT,
		R32G32_FLOAT,
		R32G32B32_FLOAT,
		R32G32B32A32_FLOAT,

        R16_UINT,
        R16_SINT,
        R32_UINT,
        R32_SINT,

        INDEX16 = R16_UINT,
        INDEX32 = R32_UINT,
        FLOAT32 = R32_FLOAT,
        VECTOR2 = R32G32_FLOAT,
        VECTOR3 = R32G32B32_FLOAT,
        VECTOR4 = R32G32B32A32_FLOAT,
	};
	enum class PrimitiveTopology
	{
        LINE_LIST,
        LINE_STRIP,
		TRIANGLE_LIST,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
	};
    struct VertexStreamI
    {
        enum Enum: uint8_t
        {
            _0 = 0,
            _1,
            _2,
            _3,
            _4,
            _5,
            _6,
            _7,
       };
    };
    enum class CmpFunc
    {
        NEVER = 1,
        LESS = 2,
        EQUAL = 3,
        LESS_EQUAL = 4,
        GREATER = 5,
        NOT_EQUAL = 6,
        GREATER_EQUAL = 7,
        ALWAYS = 8
    };
	enum class FilterType: uint32_t
	{
        POINT,
        LINEAR,

        MIN_MAG_MIP_POINT                            = 0,
        MIN_MAG_POINT_MIP_LINEAR                     = 0x1,
        MIN_POINT_MAG_LINEAR_MIP_POINT               = 0x4,
        MIN_POINT_MAG_MIP_LINEAR                     = 0x5,
        MIN_LINEAR_MAG_MIP_POINT                     = 0x10,
        MIN_LINEAR_MAG_POINT_MIP_LINEAR              = 0x11,
        MIN_MAG_LINEAR_MIP_POINT                     = 0x14,
        MIN_MAG_MIP_LINEAR                           = 0x15,

        ANISOTROPIC                                  = 0x55,

        COMPARISON_MIN_MAG_MIP_POINT                 = 0x80,
        COMPARISON_MIN_MAG_POINT_MIP_LINEAR          = 0x81,
        COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT    = 0x84,
        COMPARISON_MIN_POINT_MAG_MIP_LINEAR          = 0x85,
        COMPARISON_MIN_LINEAR_MAG_MIP_POINT          = 0x90,
        COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR   = 0x91,
        COMPARISON_MIN_MAG_LINEAR_MIP_POINT          = 0x94,
        COMPARISON_MIN_MAG_MIP_LINEAR                = 0x95,
        COMPARISON_ANISOTROPIC                       = 0xd5,

        TEXT_1BIT                                    = 0x80000000
 
	};
    enum class AddressMode 
    {
        WRAP        = 1,
        MIRROR      = 2,
        CLAMP       = 3,
        BORDER      = 4,
        MIRROR_ONCE = 5
    };
	enum class ShaderRegisterType
	{
		UNUSED = 0,
		FLOAT32,
		VECTOR2,
		VECTOR3,
		VECTOR4,
	};
    struct ClippingPlaneType
    {
        enum Enum: uint32_t
        {
            LEFT = 0,
            RIGHT,
            TOP,
            BOTTOM,
            NEARx,
            FARx,

            USER0,
            USER1,
            USER2,
            USER3,

            MAX
        };
    };
    using ClippingPlaneTypeE = ClippingPlaneType::Enum;
    enum class CullMode
    {
        NONE,
        FRONT,
        BACK,
    };
    enum class FillMode
    {
        SOLID,
        WIRE_FRAME
    };

    enum class StencilOp
    {
        KEEP = 1,
        ZERO = 2,
        REPLACE = 3,
        INCR_SAT = 4,
        DECR_SAT = 5,
        INVERT = 6,
        INCR = 7,
        DECR = 8
    };
    enum class BlendFactor
    {
        ZERO             = 1,
        ONE              = 2,
        SRC_COLOR        = 3,
        INV_SRC_COLOR    = 4,
        SRC_ALPHA        = 5,
        INV_SRC_ALPHA    = 6,
        DEST_ALPHA       = 7,
        INV_DEST_ALPHA   = 8,
        DEST_COLOR       = 9,
        INV_DEST_COLOR   = 10,
        SRC_ALPHA_SAT    = 11,
        BLEND_FACTOR     = 14,
        INV_BLEND_FACTOR = 15,
        SRC1_COLOR       = 16,
        INV_SRC1_COLOR   = 17,
        SRC1_ALPHA       = 18,
        INV_SRC1_ALPHA   = 19
    };
    enum class BlendOp
    {
        // src + dst
        ADD = 1,
        // dst - src
        SUBTRACT = 2,
        // src - dst
        REV_SUBTRACT = 3,
        MIN = 4,
        MAX = 5
    };
    struct DepthStencilOpDesc
    {
        StencilOp stencilFailOp      = StencilOp::KEEP;
        StencilOp stencilDepthFailOp = StencilOp::KEEP;
        StencilOp stencilPassOp      = StencilOp::KEEP;
        CmpFunc   stencilFunc        = CmpFunc::ALWAYS;
    };
    struct ColorWriteEnable
    {
        enum Enum: uint8_t
        {
            RED   = 1 << 0,
            GREEN = 1 << 1,
            BLUE  = 1 << 2,
            ALPHA = 1 << 3,
            COLOR = (RED | GREEN | BLUE),
            ALL   = (COLOR | ALPHA),
        };
    };
    struct Blend
    {
        bool        enabled       = false;

        BlendFactor srcBlend      = BlendFactor::ONE;
        BlendFactor dstBlend      = BlendFactor::ZERO;
        BlendOp     blendOp       = BlendOp::ADD;

        BlendFactor srcBlendAlpha = BlendFactor::ONE;
        BlendFactor dstBlendAlpha = BlendFactor::ZERO;
        BlendOp     blendOpAlpha  = BlendOp::ADD;

        uint8_t     writeMask     = ColorWriteEnable::ALL;
    };
}//ns rl
namespace rl
{
    template <typename T, size_t N = 0>
    struct LengthOf_t
    {
        static_assert(std::is_array<T>::value, "必须为数组");
        static constexpr size_t value = std::extent<T, N>::value;
    };
    template <typename ArrT>
    inline constexpr size_t lengthof() noexcept
    {
        return LengthOf_t<ArrT>::value;
    }
    template <typename T, size_t S>
    inline constexpr size_t lengthof(const T(&)[S]) noexcept
    {
        return S;
    };
    template <typename ArrT>
    inline constexpr size_t lengthof(const ArrT&) noexcept
    {
        static_assert(std::is_array<T>::value, "必须为数组");
        static_assert(std::rank<ArrT>::value == 1 "必须为1维数组");
        return lengthof<ArrT>();
    }
    inline uint32_t float_count(Format fmt)
    {
        switch(fmt)
        {
            case Format::R32_FLOAT:
                return 1;
            case Format::R32G32_FLOAT:
                return 2;
            case Format::R32G32B32_FLOAT:
                return 3;
            case Format::R32G32B32A32_FLOAT:
                return 4;
            default:
                assert(false && "Invalid Format!");
                return 0;
        }
    }
    inline uint32_t byte_count(Format fmt)
    {
        switch(fmt)
        {
        case Format::INDEX16:
            return 2;
        case Format::INDEX32:
            return 4;
        default:
            return float_count(fmt) * sizeof(float);
        }
    }
    inline ShaderRegisterType ToShaderRegisterType(Format t)
    {
        switch(t)
        {
            case Format::FLOAT32:
                return ShaderRegisterType::FLOAT32;
                break;
            case Format::VECTOR2:
                return ShaderRegisterType::VECTOR2;
                break;
            case Format::VECTOR3:
                return ShaderRegisterType::VECTOR3;
                break;
            case Format::VECTOR4:
                return ShaderRegisterType::VECTOR4;
                break;
            default:
                assert(false && "非法ElementType");
                return ShaderRegisterType::UNUSED;
        }
    }
    inline uint32_t vertex_count(PrimitiveTopology t, uint32_t nPrimitive)
    {
        switch(t)
        {
            case PrimitiveTopology::TRIANGLE_FAN:
                return nPrimitive + 2;
                break;
            case PrimitiveTopology::TRIANGLE_STRIP:
                return nPrimitive + 2;
                break;
            case PrimitiveTopology::TRIANGLE_LIST:
                return nPrimitive * 3;
                break;
            default:
                assert(false && "不支持!");
                return nPrimitive;
        }
    }
    inline uint32_t primitive_count(PrimitiveTopology t, uint32_t nVertex)
    {
        switch(t)
        {
            case PrimitiveTopology::TRIANGLE_FAN:
                return nVertex - 2;
                break;
            case PrimitiveTopology::TRIANGLE_STRIP:
                return nVertex - 2;
                break;
            case PrimitiveTopology::TRIANGLE_LIST:
                return nVertex / 3;
                break;
            default:
                assert(false);
        }
        return 0;
	}
}

#endif //RASLITE_COMMON_H