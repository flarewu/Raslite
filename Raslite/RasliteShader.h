#ifndef RASLITE_SHADER_H
#define RASLITE_SHADER_H
#include "RasliteData.h"
namespace rl {
    /////////////////////////////////////////////////////////////////
    //
    using ShaderRegister = Vec4;
    using VSRegisters = ShaderRegister[VS_REGISTER_COUNT];
    using PSRegisters = ShaderRegister[PS_REGISTER_COUNT];

    using VSRegisterTypes = ShaderRegisterType[VS_REGISTER_COUNT];
    using PSRegisterTypes = ShaderRegisterType[PS_REGISTER_COUNT];
    struct SamplerState
    {
        FilterType  filter        = FilterType::MIN_MAG_MIP_POINT;
        AddressMode addressU      = AddressMode::CLAMP;
        AddressMode addressV      = AddressMode::CLAMP;
        AddressMode addressW      = AddressMode::CLAMP;
        Float       mipLodBias    = Float(0);
        uint32_t    maxAnisotropy = 16;
        CmpFunc     cmpFunc       = CmpFunc::NEVER;
        ColorValue  borderColor   = ColorValue::BLACK;
        Float       minLod        = -FLT_MAX;
        Float       maxLod        = FLT_MAX;
    };
    /////////////////////////////////////////////////////////////////
    //Shader
    class Shader
    {
    protected:
        // Shader中使用的Texture Objects
        class Tex1D;       //todo: RWTex1D
        class Tex1DArray;  //todo: RWTex1DArray
        class Tex2D;       //todo: RWTex2D
        class Tex2DArray;  //todo: RWTex2DArray
        class Tex2DMS;
        class Tex2DMSArray;
        class Tex3D;       //todo: RWTex3D
        class Tex3DArray;
        class TexCube;
        class TexCubeArray;
    public:
        template <typename T> 
        void uniform(uint32_t idx, const T& val);
        void setSamplerState (uint8_t sloti, const SamplerState& ss);
        void setInputResource(uint8_t sloti, Texture* surf);
        Texture*            getInputResource(uint8_t sloti) const;
        const SamplerState& getSamplerState(uint8_t sloti)  const;
    protected:
        template <typename T> 
        const T& uniform(uint32_t idx) const;
        Tex2D tex2D(uint8_t sloti) const;
    public:// Write Uniform
        template<> void uniform<Float>(uint32_t idx, const Float& val);
        template<> void uniform<Vec2>(uint32_t idx, const Vec2&  val);
        template<> void uniform<Vec3>(uint32_t idx, const Vec3&  val);
        template<> void uniform<Vec4>(uint32_t idx, const Vec4&  val);
        template<> void uniform<Mat4>(uint32_t idx, const Mat4&  val);
    protected: // Read Uniform
        template<> const Float& uniform<Float>(uint32_t idx) const;
        template<> const Vec4&  uniform<Vec4> (uint32_t idx) const;
        template<> const Mat4&  uniform<Mat4> (uint32_t idx) const;
    private: 
        Float m_floats  [SHADER_CONSTANT_COUNT] = { Float(0) };
        Vec4  m_vectors [SHADER_CONSTANT_COUNT];
        Mat4  m_matrices[SHADER_CONSTANT_COUNT];

        SamplerState m_samplerStates [SAMPLER_STATE_COUNT];
        Texture*     m_resources[SHADER_INPUT_RESOURCE_COUNT] = { nullptr };
    };
    class Shader::Tex2D 
    {
    public:
        Tex2D(const Texture2D* texture, const SamplerState* states);

        Vec4 load(int location, int offset, int sampleIndex);

        Vec4 sample    (uint8_t samplerslot, const Vec2& location,                       const Vec2i& offset = Vec2i::zero());
        Vec4 sampleGrad(uint8_t samplerslot, const Vec2& location, Float ddx, Float ddy, const Vec2i& offset = Vec2i::zero());
        Vec4 sampleBias(uint8_t samplerslot, const Vec2& location, Float lod,            const Vec2i& offset = Vec2i::zero());
        Vec4 sampleCmp (uint8_t samplerslot, const Vec2& location, Float cmpValue,       const Vec2i& offset = Vec2i::zero());
    private:
        const Texture2D*    m_texture = nullptr;
        const SamplerState* m_states  = nullptr;
    };
    /////////////////////////////////////////////////////////////////
    class VertexShader: public Shader
    {
    public:
        struct SystemValue;
    public:
        VertexShader();
        virtual void execute(const VSRegisters& input, PSRegisters& output,SystemValue& sv) = 0;
    };
    struct VertexShader::SystemValue
    {
        Vec4     position;     // in clipspace

        uint32_t vertexID;
        uint32_t primtiveID;
        uint32_t instanceID;
    };
    /////////////////////////////////////////////////////////////////
    class PixelShader: public Shader
    {
    public:
        struct SystemValue;
    public:
        virtual bool execute(const PSRegisters& varyings, SystemValue& sv) = 0;
    };
    struct PixelShader::SystemValue
    {
        //Output
        Float depth;
        //Input:(xPixel+0.5,yPixel+0.5,NonlinearZ,1)
        Vec4 position;
        //Ouptut
        Vec4 targets[RENDER_TARGET_COUNT];
        //Input
        uint8_t targetIndex;
        //Input
        bool isFrontFace;
        //input&output
        uint32_t coverage;
        //input
        uint32_t sampleIndex;
    };
}//ns 
/////////////////////////////////////////////////////////////////
// 内联
/////////////////////////////////////////////////////////////////
namespace rl {
    /////////////////////////////////////////////////////////////////
    inline void Shader::setSamplerState(uint8_t sloti, const SamplerState& ss)
    {
        assert(sloti < SHADER_INPUT_RESOURCE_COUNT);
        m_samplerStates[sloti] = ss;
    }
    inline const SamplerState& Shader::getSamplerState(uint8_t sloti) const
    {
        assert(sloti < SAMPLER_STATE_COUNT);
        return m_samplerStates[sloti];
    }
    inline void Shader::setInputResource(uint8_t sloti, Texture* surf)
    {
        assert(sloti < SHADER_INPUT_RESOURCE_COUNT);
        m_resources[sloti] = surf;
    }
    inline Texture* Shader::getInputResource(uint8_t sloti) const
    {
        assert(sloti < SHADER_INPUT_RESOURCE_COUNT);
        return m_resources[sloti];
    }
    template<>
    inline void Shader::uniform<Float>(uint32_t idx, const Float& val)
    {
        m_floats[idx] = val;
    }
    template<>
    inline void Shader::uniform<Vec4>(uint32_t idx, const Vec4& val)
    {
        m_vectors[idx] = val;
    }
    template<> 
    inline void Shader::uniform<Vec3>(uint32_t idx, const Vec3& val)
    {
        m_vectors[idx] = val;
    }
    template<>
    inline void Shader::uniform<Vec2>(uint32_t idx, const Vec2& val)
    {
        m_vectors[idx] = val;
    }
    template<>
    inline void Shader::uniform<Mat4>(uint32_t idx, const Mat4& val)
    {
        m_matrices[idx] = val;
    }
    template<> 
    inline const Float& Shader::uniform<Float>(uint32_t idx) const
    {
        return m_floats[idx];
    }
    template<>
    inline const Vec4& Shader::uniform<Vec4>(uint32_t idx) const
    {
        return m_vectors[idx];
    }
    template<>
    inline const Mat4& Shader::uniform<Mat4>(uint32_t idx) const
    {
        return m_matrices[idx];
    }
    //
    inline Shader::Tex2D::Tex2D(const Texture2D* texture, const SamplerState* states)
        : m_texture(texture)
        , m_states(states)
    {
    }
    inline Shader::Tex2D Shader::tex2D(uint8_t texslot) const
    {
        return Tex2D(static_cast<const Texture2D*>(m_resources[texslot]), m_samplerStates);
    }
    /////////////////////////////////////////////////////////////////
    inline VertexShader::VertexShader()
    {
    }

}//
#endif //RASLITE_SHADER_H