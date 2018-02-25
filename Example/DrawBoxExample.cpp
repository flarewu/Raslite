#include "Example.h"

class BoxVS: public VertexShader
{
public:
    BoxVS()
    {
    }
private:
    virtual void execute(const VSRegisters& input, PSRegisters& output, SystemValue& sv) override
    {
        sv.position = input[VSRegisterI::POSITION] * uniform<Mat4>(ShaderUniformI::WVP_MATRIX);
        output[PSRegisterI::COLOR] = input[VSRegisterI::COLOR];//vertex color
    }
};
class BoxPS: public PixelShader
{
    virtual bool execute(const PSRegisters& varyings, SystemValue& sv)
    {
        sv.targets[sv.targetIndex] = varyings[PSRegisterI::COLOR];
        return true;
    }
};
class DrawBoxExample: public Example
{
    struct Vertex
    {
        Vec3 m_position;
       // Vec3 m_normal;
        Vec3 m_color;
    };
public:
    DrawBoxExample(uint32_t backbufferW, uint32_t backbufferH)
        : Example(backbufferW, backbufferH)
    {
        InputElement inputLements[] =
        {
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::POSITION},
          //  {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::NORMAL},
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::COLOR},
        };
        m_ilayout = std::make_unique<InputLayout>(inputLements, lengthof(inputLements));
        m_context.ia.layout = m_ilayout.get();


        auto triCount = 1;

        m_vbuffer = std::make_unique<VertexBuffer>(sizeof(Vertex) * 8);
        {
            auto vptr = m_vbuffer->getData<Vertex>();

            vptr->m_position = Vec3(-10, -10, -10);
            vptr->m_color    = Vec3(1, 0, 0);
            ++vptr;

            vptr->m_position = Vec3(+10, -10, -10);
            vptr->m_color    = Vec3(0, 1, 0);
            ++vptr;

            vptr->m_position = Vec3(+10, +10, -10);
            vptr->m_color    = Vec3(0, 0, 1);
            ++vptr;

            vptr->m_position = Vec3(-10, +10, -10);
            vptr->m_color = Vec3(0, 0, 1);
            ++vptr;
            ///////////////////////////////
            vptr->m_position = Vec3(-10, -10, +10);
            vptr->m_color = Vec3(1, 0, 0);
            ++vptr;

            vptr->m_position = Vec3(+10, -10, +10);
            vptr->m_color = Vec3(0, 1, 0);
            ++vptr;

            vptr->m_position = Vec3(+10, +10, +10);
            vptr->m_color = Vec3(0, 0, 1);
            ++vptr;

            vptr->m_position = Vec3(-10, +10, +10);
            vptr->m_color = Vec3(0, 0, 1);
            ++vptr;

        }
        m_context.ia.vstreams[0].vbuffer = m_vbuffer.get();
        m_context.ia.vstreams[0].offset = 0;
        m_context.ia.vstreams[0].stride = sizeof(Vertex);
        m_indexCount = 6 * 2 * 3;
        m_ibuffer = std::make_unique<IndexBuffer>(m_indexCount, Format::INDEX16);
        {
            auto iptr = m_ibuffer->getData<uint16_t>();
            //down: 0,1,2,3
            //0
            *iptr = 0, *(iptr + 1) = 1, *(iptr + 2) = 2;
            iptr += 3;
            //1
            *iptr = 0, *(iptr + 1) = 2, *(iptr + 2) = 3;
            iptr += 3;
            //up: 4,5,6,7
            //0
            *iptr = 4, *(iptr + 1) = 6, *(iptr + 2) = 5;
            iptr += 3;
            //1
            *iptr = 4, *(iptr + 1) = 7, *(iptr + 2) = 6;
            iptr += 3;
            //right: 1,5,6,2
            //0
            *iptr = 1, *(iptr + 1) = 5, *(iptr + 2) = 6;
            iptr += 3;
            //1
            *iptr = 1, *(iptr + 1) = 6, *(iptr + 2) = 2;
            iptr += 3;
            //left: 0,3,7,4
            //0
            *iptr = 0, *(iptr + 1) = 3, *(iptr + 2) = 7;
            iptr += 3;
            //1
            *iptr = 0, *(iptr + 1) = 7, *(iptr + 2) = 4;
            iptr += 3;
            //back: 2673
            //0
            *iptr = 2, *(iptr + 1) = 6, *(iptr + 2) = 7;
            iptr += 3;
            //1
            *iptr = 2, *(iptr + 1) = 7, *(iptr + 2) = 3;
            iptr += 3;
            //front: 0451
            //0
            *iptr = 0, *(iptr + 1) = 4, *(iptr + 2) = 6;
            iptr += 3;
            //1
            *iptr = 0, *(iptr + 1) = 6, *(iptr + 2) = 1;
            //iptr += 3;

        }
        m_context.ia.ibuffer = m_ibuffer.get();

        m_vshader = std::make_unique<BoxVS>();
        {
            auto wvp =
                Transform::LookAt().eye(Vec3(0,0,-30)).at(Vec3(0, 0, 10)).up(Vec3::UNIT_Y).LH() *
                Transform::ProjectFov().aspect(Float(m_backbufferW)/m_backbufferH).yFov(PI/2).zNear(1).zFar(1000).LH();
            m_vshader->uniform(ShaderUniformI::WVP_MATRIX, wvp);
        }
        m_context.vs = m_vshader.get();

        m_pshader = std::make_unique<BoxPS>();
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
        m_context.om.depthEnabled = true;
        m_pipeline = std::make_unique<Pipeline>();
    }
    virtual void onUpdate(Float deltaTime) override
    {
        m_depthbuf->clear(1.0f);
        m_colorbuf->clear(ColorValue(.33f, .333f, .315f, 0.f));
        if(!m_freezed)
            m_rad += PI / 90;
        auto wvp =
            Transform::rotateY(m_rad) *
            Transform::LookAt().eye(Vec3(20, 20, 20)).at(Vec3(0, 0, 0)).up(Vec3::UNIT_Y).LH() *
            Transform::ProjectFov().aspect(Float(m_backbufferW) / m_backbufferH).yFov(PI / 2).zNear(1).zFar(1000).LH();
       
        m_vshader->uniform(ShaderUniformI::WVP_MATRIX, wvp);

        m_pipeline->drawIndexed(m_context, m_indexCount, 0/*indexStart*/, 0/*baseVertexIndex*/);
    }

    uint32_t m_indexCount;
    Float m_rad = 0;
    Float m_scaling = 100;
};

std::unique_ptr<Example> DrawBoxExampleCreate(uint32_t backbufferW, uint32_t backbufferH)
{
    return std::make_unique<DrawBoxExample>(backbufferW, backbufferH);
}

