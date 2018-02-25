#include "Example.h"
#include "../Dependencies/OBJ_Loader.h"

/////////////////////////////////////////////////////////////////////////
class HolographicVS: public VertexShader
{
    virtual void execute(const VSRegisters& input, PSRegisters& output, SystemValue& sv) override
    {
        auto wvp = this->uniform<Mat4>(ShaderUniformI::WORLD_MATRIX) *   
                   this->uniform<Mat4>(ShaderUniformI::VIEW_MATRIX)  *
                   this->uniform<Mat4>(ShaderUniformI::PROJECTION_MATRIX);
        sv.position = input[VSRegisterI::POSITION] * wvp;

        auto scale  = this->uniform<Vec4>(ShaderUniformI::SCALE).xyz();
        auto rot    = this->uniform<Mat4>(ShaderUniformI::WORLD_MATRIX) * Transform::scale(1.0f/scale.x, 1.0f/scale.y, 1.0f/scale.z);
        // World Normal
        output[PSRegisterI::NORMAL] = Transform::transformVector(input[VSRegisterI::NORMAL].xyz(), rot);
    }
};
class HolographicPS: public PixelShader
{
    virtual bool execute(const PSRegisters& varyings, SystemValue& sv)
    {
        auto view       = this->uniform<Vec4>(ShaderUniformI::VIEW_DIRECTION).xyz();
        auto innerAlpha = this->uniform<Float>(ShaderUniformI::INNER_ALPHA);
        auto albedo     = this->uniform<Vec4>(ShaderUniformI::ALBEDO).xyz();

        auto norm = varyings[PSRegisterI::NORMAL].xyz().normalize();

        //最边界的alpha为1, 最内部的alpha为innerAlpha,  border为插值因子;
        //border值越大说明越靠近边界
        auto border = 1 - std::abs(view.dot(norm));
        // 用border为插值因子: Alpha值从: 1(外围) 到 innerAlpha(内部)
        auto alpha  = border*1  + (1-border)* innerAlpha;
        sv.targets[sv.targetIndex].rgba(albedo,alpha);
        return true;
    }
};
/////////////////////////////////////////////////////////////////////////
class HolographicExample: public Example
{
    struct Vertex
    {
        Vec3 m_position;
        Vec3 m_normal;
        Vec3 m_color;
    };
public:
    HolographicExample(uint32_t backbufferW, uint32_t backbufferH)
        : Example(backbufferW,backbufferH)
    {
        InputElement inputElements[] =
        {
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::POSITION},
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::NORMAL},
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::COLOR},
        };
        m_ilayout = std::make_unique<InputLayout>(inputElements, lengthof(inputElements));
        m_context.ia.layout = m_ilayout.get();
        objl::Loader loader;
        auto loaded = loader.LoadFile(RLEXAMPLE_PATH("teapot.obj"));//african_head teapot pumpkin_tall_10k teddy
        m_scaling = Float(1);
        m_vbuffer = std::make_unique<VertexBuffer>(sizeof(Vertex) * loader.LoadedVertices.size());
        {
            auto vptr = m_vbuffer->getData<Vertex>();
            for(int i = 0; i < loader.LoadedVertices.size(); i++)
            {
                auto& p = loader.LoadedVertices[i].Position;
                auto& n = loader.LoadedVertices[i].Normal;
                vptr->m_position =  Vec3(p.X,p.Y,-p.Z);//obj在右手坐标系中，我们转换到左手坐标系中
                vptr->m_normal   =  Vec3(n.X,n.Y,-n.Z).normalize();
                switch(i % 3)
                {
                case 0:
                    vptr->m_color = Vec3(1, 0, 0);
                    break;
                case 1:
                    vptr->m_color = Vec3(0, 1, 0);
                    break;
                case 2:
                    vptr->m_color = Vec3(0, 0, 1);
                    break;
                }
                ++vptr;
            }
        }
        m_indexCount = loader.LoadedIndices.size();
        m_ibuffer = std::make_unique<IndexBuffer>(m_indexCount, Format::INDEX16);
        {
            auto iptr = m_ibuffer->getData<uint16_t>();
            for(int i = 0; i < loader.LoadedIndices.size(); i++)
                *iptr++ = loader.LoadedIndices[i];
        }
        m_context.ia.ibuffer = m_ibuffer.get();
        auto& vstrm = m_context.ia.vstreams[0];
        {
            vstrm.vbuffer = m_vbuffer.get();
            vstrm.offset = 0;
            vstrm.stride = sizeof(Vertex);
        }
        //m_camera->set(Vec3(2,4,2),Vec3(0, 0, 0),Vec3::UNIT_Y);
        m_vshader = std::make_unique<HolographicVS>();
        {
            m_vshader->uniform(ShaderUniformI::SCALE,             Vec4(m_scaling, m_scaling,m_scaling,0));
            m_vshader->uniform(ShaderUniformI::WORLD_MATRIX,      Transform::translate(0, -1, 4));
            m_vshader->uniform(ShaderUniformI::VIEW_MATRIX,      Transform::LookAt().eye(Vec3(2,4,2)).at(Vec3(0, 0, 0)).up(Vec3::UNIT_Y).LH());
            m_vshader->uniform(ShaderUniformI::PROJECTION_MATRIX, Transform::ProjectFov().aspect(Float(m_backbufferW)/m_backbufferH).yFov(PI/2).zNear(1).zFar(1000).LH());
           // m_vshader->uniform(ShaderUniformI::PROJECTION_MATRIX, Transform::Project().width(2).height(2).zNear(1).zFar(1000).orthoLH());
        }
        m_context.vs = m_vshader.get();

        m_pshader = std::make_unique<HolographicPS>();
        {
            m_pshader->uniform(ShaderUniformI::ALBEDO, ColorValue::BLUE);
            m_pshader->uniform(ShaderUniformI::INNER_ALPHA,0.01f);
            m_pshader->uniform(ShaderUniformI::VIEW_DIRECTION, (Vec3(2,4,2) - Vec3::ZERO).normalize());
        }
        m_context.ps = m_pshader.get();

        m_colorbuf = std::make_unique<Surface>(backbufferW, backbufferH, Format::R32G32B32_FLOAT);
        m_depthbuf = std::make_unique<Surface>(backbufferW, backbufferH, Format::R32_FLOAT);
        {
            m_viewport.topLeftX = 0;
            m_viewport.topLeftY = 0;
            m_viewport.width    = backbufferW;
            m_viewport.height   = backbufferH;
            m_viewport.minDepth = 0.0f;
            m_viewport.maxDepth = 1.0f;
        };
        m_context.rs.viewportCount = 1;
        m_context.rs.viewports = &m_viewport;
        m_context.om.renderTargets[0] = m_colorbuf.get();
        m_context.om.depthStencil = m_depthbuf.get();
        m_context.rs.cullMode = CullMode::BACK;
        m_context.rs.fillMode = FillMode::SOLID;
        m_context.rs.frontCounterClockwise = true;
        m_context.om.depthEnabled = true;

        m_context.om.blends[0].enabled   = true;
        m_context.om.blends[0].srcBlend  = BlendFactor::SRC_ALPHA;
        m_context.om.blends[0].dstBlend  = BlendFactor::INV_SRC_ALPHA;
        m_context.om.blends[0].blendOp   = BlendOp::ADD;
        m_context.om.blends[0].writeMask = ColorWriteEnable::ALL;

        m_context.rs.scissorEnabled = false;;
        if(m_context.rs.scissorEnabled)
        {
            m_scissor.top = 100;   // 0
            m_scissor.left = 100;   // 0
            m_scissor.bottom = 200;//backbufferH;
            m_scissor.right  = 200;//backbufferW;
            m_context.rs.scissorRects = &m_scissor;
            m_context.rs.scissorRectCount = 1;
        }
    }
private:
    virtual void onUpdate(Float deltaTime) override
    {
        m_depthbuf->clear(1.0f);
        m_colorbuf->clear(ColorValue(.33f, .333f, .315f, 0.f));
        if(!m_freezed)
            m_rad += 2*PI / 90;

        m_xltZ += 0.3*m_signed;
        if(m_xltZ > 10)
            m_signed = -1;
        if(m_xltZ < 5)
            m_signed = 1;
        m_xltZ = 6;
        m_vshader->uniform(ShaderUniformI::WORLD_MATRIX,Transform::rotateY(m_rad) * Transform::translate(0, 0, 0));
        m_pipeline->drawIndexed(m_context, m_indexCount, 0/*indexStart*/, 0/*baseVertexIndex*/);

        m_vshader->uniform(ShaderUniformI::WORLD_MATRIX, Transform::rotateY(m_rad) * Transform::translate(0, 0, -5));
        m_pipeline->drawIndexed(m_context, m_indexCount, 0/*indexStart*/, 0/*baseVertexIndex*/);
    }

    uint32_t m_indexCount;
    Float m_rad = 0;
    Float m_scaling = 1;
    Float m_xltZ = 5;
    Float m_signed = 1;
};

std::unique_ptr<Example> DrawHolographicExampleCreate(uint32_t backbufferW, uint32_t backbufferH)
{
    return std::make_unique <HolographicExample>(backbufferW, backbufferH);
}