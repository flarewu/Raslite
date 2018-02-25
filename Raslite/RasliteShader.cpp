#include "RasliteShader.h"

namespace rl {
    class Sampler
    {
    private:
        static Float mapTextureCoordinate(AddressMode am, Float x)
        {
            assert(x >= 0);
            switch(am)
            {
            case AddressMode::WRAP:
                return x - floori(x);
                break;
            case AddressMode::CLAMP:
                return saturate(x);
                break;
            default:
                assert(false);
            }
            return Float(0);
        }
    public:
        static Vec4 load(const Texture2D& tex,uint32_t location, int offset, int sampleIndex)
        {
            return Vec4::zero();
        }
        static Vec4 sample(const Texture2D& tex,const SamplerState& ss, const Vec2& location, const Vec2i& offset)
        {
            // 把Normalized Texture Coordinate转换为Texel Coordinate
            auto texelU = Sampler::mapTextureCoordinate(ss.addressU, location.u) * tex.getWidth();
            auto texelV = Sampler::mapTextureCoordinate(ss.addressV, location.v) * tex.getHeight();
            //
            switch(ss.filter)
            {
            case FilterType::MIN_MAG_MIP_POINT:
                {
                    auto addrU = clamp<uint32_t>(floorui(texelU) + offset.u, 0, tex.getWidth()  - 1);
                    auto addrV = clamp<uint32_t>(floorui(texelV) + offset.v, 0, tex.getHeight() - 1);
                    return tex.getMipSurface(0)->getElement(addrU, addrV);
                }
                break;
            case FilterType::MIN_MAG_POINT_MIP_LINEAR:
                assert(false&&"待实现");
                break;
            case FilterType::MIN_POINT_MAG_LINEAR_MIP_POINT:
                assert(false&&"待实现");
                break;
            case FilterType::MIN_POINT_MAG_MIP_LINEAR:
                assert(false&&"待实现");
                break;
            case FilterType::MIN_LINEAR_MAG_MIP_POINT:
                assert(false&&"待实现");
                break;
            case FilterType::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
                assert(false&&"待实现");
                break;
            case FilterType::MIN_MAG_LINEAR_MIP_POINT:
                assert(false&&"待实现");
                break;
            case FilterType::MIN_MAG_MIP_LINEAR:
                assert(false&&"待实现");
                break;
            case FilterType::ANISOTROPIC:
                assert(false&&"待实现");
                break;
            }

            return Vec4::zero();
        }
        static Vec4 sampleGrad(const Texture2D& tex,const SamplerState& ss, const Vec2& location, Float ddx, Float ddy, const Vec2i& offset)
        {
            return Vec4::zero();
        }
        static Vec4 sampleBias(const Texture2D& tex,const SamplerState& ss, const Vec2& location, Float lod, const Vec2i& offset)
        {
            return Vec4::zero();
        }
        static Vec4 sampleCmp(const Texture2D& tex,const SamplerState& ss, const Vec2& location, Float cmpValue, const Vec2i& offset)
        {
            return Vec4::zero();
        }
    };
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    Vec4 Shader::Tex2D::load(int location, int offset, int sampleIndex)
    {
        return Sampler::load(*m_texture, location, offset, sampleIndex);
    }
    Vec4 Shader::Tex2D::sample(uint8_t samplerslot, const Vec2& location, const Vec2i& offset)
    {
        return Sampler::sample(*m_texture, m_states[samplerslot],location, offset);
    }
    Vec4 Shader::Tex2D::sampleGrad(uint8_t samplerslot, const Vec2& location, Float ddx, Float ddy, const Vec2i& offset)
    {
        return Sampler::sampleGrad(*m_texture, m_states[samplerslot],location, ddx,ddy,offset);
    }
    Vec4 Shader::Tex2D::sampleBias(uint8_t samplerslot, const Vec2& location, Float lod, const Vec2i& offset)
    {
        return Sampler::sampleBias(*m_texture, m_states[samplerslot],location, lod,offset);
    }
    Vec4 Shader::Tex2D::sampleCmp(uint8_t samplerslot, const Vec2& location, Float cmpValue, const Vec2i& offset)
    {
        return Sampler::sampleCmp(*m_texture, m_states[samplerslot],location, cmpValue,offset);
    }


}// ns rl