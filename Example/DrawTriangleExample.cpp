#include "Example.h"

class TriangleVS: public VertexShader
{
public:
    TriangleVS()
    {
    }
private:
    virtual void execute(const VSRegisters& input, PSRegisters& output, SystemValue& sv) override
    {
        sv.position = input[VSRegisterI::POSITION] * uniform<Mat4>(ShaderUniformI::WVP_MATRIX);
        output[PSRegisterI::COLOR] = input[VSRegisterI::COLOR];//vertex color
    }
};
class TrianglePS: public PixelShader
{
    virtual bool execute(const PSRegisters& varyings, SystemValue& sv)
    {
        sv.targets[sv.targetIndex] = varyings[PSRegisterI::COLOR];
        return true;
    }
};
class DrawTriangleExample: public Example
{
    struct Vertex
    {
        Vec3 m_position;
        Vec3 m_color;
    };
public:
    DrawTriangleExample(uint32_t backbufferW, uint32_t backbufferH)
        : Example(backbufferW, backbufferH)
    {
        InputElement inputLements[] =
        {
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::POSITION},
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::COLOR},
        };
        m_ilayout = std::make_unique<InputLayout>(inputLements, lengthof(inputLements));
        m_context.ia.layout = m_ilayout.get();


        auto triCount = 1;

        m_vbuffer = std::make_unique<VertexBuffer>(sizeof(Vertex) * 3);
        {
            auto vptr = m_vbuffer->getData<Vertex>();

            vptr->m_position = Vec3(-48, -10, 80.2);
            //vptr->m_position = Vec3(-10, 0, 5);
            vptr->m_color = Vec3(1, 0, 0);
            ++vptr;

            vptr->m_position = Vec3(29, -15, 40.4);
            //vptr->m_position = Vec3(10, -0, 5);
            vptr->m_color = Vec3(0, 1, 0);
            ++vptr;

            vptr->m_position = Vec3(1, 0, 11.4);
            //vptr->m_position = Vec3(0, 10, 5);
            vptr->m_color = Vec3(0, 0, 1);
            ++vptr;
        }

        m_context.ia.vstreams[0].vbuffer = m_vbuffer.get();
        m_context.ia.vstreams[0].offset = 0;
        m_context.ia.vstreams[0].stride = sizeof(Vertex);

        m_vshader = std::make_unique<TriangleVS>();
        {
            auto wvp =
                Transform::LookAt().eye(Vec3::ZERO).at(Vec3(0, 0, 10)).up(Vec3::UNIT_Y).LH() *
                Transform::Project().width(2).height(2).zNear(1).zFar(1000).perspLH();

            m_vshader->uniform(ShaderUniformI::WVP_MATRIX, wvp);
        }
        m_context.vs = m_vshader.get();

        m_pshader = std::make_unique<TrianglePS>();
        m_context.ps = m_pshader.get();

        m_colorbuf = std::make_unique<Surface>(m_backbufferW, m_backbufferH, Format::R32G32B32_FLOAT);
        m_depthbuf = std::make_unique<Surface>(m_backbufferW, m_backbufferH, Format::R32_FLOAT);

        {
            m_viewport.topLeftX = 0;
            m_viewport.topLeftY = 0;
            m_viewport.width = m_backbufferW;
            m_viewport.height = m_backbufferH;
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

        m_pipeline = std::make_unique<Pipeline>();
    }
    virtual void onUpdate(Float deltaTime) override
    {
        m_depthbuf->clear(1.0f);
        m_colorbuf->clear(ColorValue::BLACK);
        m_pipeline->draw(m_context, 3, 0);
    }
};

std::unique_ptr<Example> DrawTriangleExampleCreate(uint32_t backbufferW, uint32_t backbufferH)
{
    return std::make_unique<DrawTriangleExample>(backbufferW, backbufferH);
}

