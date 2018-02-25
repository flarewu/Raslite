
#define STB_IMAGE_IMPLEMENTATION  
#include "../Dependencies/stb_image.h"
#include "Example.h"
class CheckerBoardVS: public VertexShader
{
public:
    CheckerBoardVS()
    {
    }
private:
    virtual void execute(const VSRegisters& input, PSRegisters& output, SystemValue& sv) override
    {
        auto wvp = uniform<Mat4>(ShaderUniformI::WORLD_MATRIX) *
                   uniform<Mat4>(ShaderUniformI::VIEW_MATRIX)  *
                   uniform<Mat4>(ShaderUniformI::PROJECTION_MATRIX);
        sv.position = input[VSRegisterI::POSITION] * wvp;

        output[PSRegisterI::COLOR]   = input[VSRegisterI::COLOR];
        output[PSRegisterI::TEX_UV0] = input[VSRegisterI::TEX_UV0];
    }
};
class CheckerBoardPS: public PixelShader
{
    virtual bool execute(const PSRegisters& varyings, SystemValue& sv)
    {
        sv.targets[sv.targetIndex] = /*varyings[PSRegisterI::COLOR]*/ tex2D(0).sample(0,varyings[PSRegisterI::TEX_UV0].uv());
        return true;
    }
};
class CheckerBoard: public Renderable
{
    struct Vertex
    {
        Vec3 m_position;
        Vec4 m_color;
        Vec2 m_texUV;
    };
public:
    CheckerBoard(Example& ex, Float extent)
        : Renderable(ex)
        , m_extent(extent)
    {
        // 加载texture
        {
            int width, height, nChannel;
            auto mem = stbi_loadf(RLEXAMPLE_PATH("CheckerBoard.tga"), &width, &height, &nChannel,4);
            Texture2D::Desc desc;
            {
                desc.width         = width;
                desc.height        = height;
                desc.format        = Format::R32G32B32A32_FLOAT;
                desc.mem           = mem;
                desc.memPitch      = width * byte_count(desc.format);
                desc.memSlicePitch = desc.memPitch * height;
            }
            m_texture = std::make_unique<Texture2D>(desc);
            stbi_image_free(mem);
        }
        //
        InputElement inputElements[] =
        {
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::POSITION},
            {VertexStreamI::_0,Format::VECTOR4,VSRegisterI::COLOR},
            {VertexStreamI::_0,Format::VECTOR2,VSRegisterI::TEX_UV0},
        };
        m_ilayout = std::make_unique<InputLayout>(inputElements, lengthof(inputElements));
        m_context.ia.layout = m_ilayout.get();
        m_context.ia.topology = PrimitiveTopology::TRIANGLE_STRIP;

        const auto halfExtent = m_extent*Float(0.5);
        const Vec3 corner(-halfExtent, 0, -halfExtent);
        m_vertexCount = 4;
        m_vbuffer = std::make_unique<VertexBuffer>(sizeof(Vertex) *m_vertexCount);
        {
            Vertex vertex ={ corner, {0x80/255.0f, 0xc0/255.0f, 0xd0/255.0f, 0xb0/255.0f} ,{0,0} };

            auto vptr = m_vbuffer->getData<Vertex>();
            // 按CCW排列顶点
            //左下
            vptr->m_position = vertex.m_position;
            vptr->m_color    = vertex.m_color;
            vptr->m_texUV    = vertex.m_texUV;
            ++vptr;
            //右下
            vptr->m_position = vertex.m_position + Vec3(m_extent, 0, 0);
            vptr->m_color    = vertex.m_color;
            vptr->m_texUV    = vertex.m_texUV + Vec2(0,1);
            ++vptr;
            //左上
            vptr->m_position = vertex.m_position + Vec3(0, 0, m_extent);
            vptr->m_color    = vertex.m_color;
            vptr->m_texUV    = vertex.m_texUV + Vec2(1,0);
            ++vptr;
            //右上
            vptr->m_position = vertex.m_position + Vec3(m_extent, 0, m_extent);
            vptr->m_color    = vertex.m_color;
            vptr->m_texUV    = vertex.m_texUV + Vec2(1,1);
            ++vptr;

        }
        auto& vstrm = m_context.ia.vstreams[0];
        {
            vstrm.vbuffer = m_vbuffer.get();
            vstrm.offset = 0;
            vstrm.stride = sizeof(Vertex);
        }
        m_vshader = std::make_unique<CheckerBoardVS>();
        {
            m_vshader->uniform(ShaderUniformI::WORLD_MATRIX, Transform::translate(0, 0, 0));
            m_vshader->uniform(ShaderUniformI::PROJECTION_MATRIX, m_example.getProjection());
        }
        m_context.vs = m_vshader.get();

        m_pshader = std::make_unique<CheckerBoardPS>();
        {
            m_pshader->setInputResource(0, m_texture.get());
            SamplerState ss;
            {

            }
            m_pshader->setSamplerState(0, ss);
        }
        m_context.ps = m_pshader.get();
        {// viewport
            m_viewport.topLeftX = 0;
            m_viewport.topLeftY = 0;
            m_viewport.width    = m_example.getBackBufferW();
            m_viewport.height   = m_example.getBackBufferH();
            m_viewport.minDepth = 0.0f;
            m_viewport.maxDepth = 1.0f;
        };
        m_context.rs.viewportCount         = 1;
        m_context.rs.viewports             = &m_viewport;
        m_context.om.renderTargets[0]      = m_example.getRenderTarget();
        m_context.om.depthStencil          = m_example.getDepthStencil();
        m_context.rs.cullMode              = CullMode::BACK;
        m_context.rs.fillMode              = FillMode::SOLID;
        m_context.rs.frontCounterClockwise = true;
        m_context.om.depthEnabled          = true;

        m_context.om.blends[0].enabled   = true;
        m_context.om.blends[0].srcBlend  = BlendFactor::SRC_ALPHA;
        m_context.om.blends[0].dstBlend  = BlendFactor::INV_SRC_ALPHA;
        m_context.om.blends[0].blendOp   = BlendOp::ADD;
        m_context.om.blends[0].writeMask = ColorWriteEnable::ALL;


        m_context.rs.scissorEnabled        = false;;
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
    virtual void render(Float deltaTime) override
    {
        m_vshader->uniform(ShaderUniformI::VIEW_MATRIX, m_example.getView());
        m_example.getPipeline()->draw(m_context, m_vertexCount/*vertexCount*/, 0/*vertexStart*/);
    }
private:
    uint32_t m_vertexCount;
    Float    m_extent;
    std::unique_ptr<Texture> m_texture;
};
class CheckerBoardExample: public Example
{
    struct Vertex
    {
        Vec3 m_position;
        Vec4 m_color;
    };
public:
    CheckerBoardExample(uint32_t backbufferW, uint32_t backbufferH)
        : Example(backbufferW, backbufferH)
    {
        m_colorbuf = std::make_unique<Surface>(backbufferW, backbufferH, Format::R32G32B32_FLOAT);
        m_depthbuf = std::make_unique<Surface>(backbufferW, backbufferH, Format::R32_FLOAT);
        m_view = Transform::LookAt().eye(Vec3(0, 5, -10)).at(Vec3(0, 0, 20)).up(Vec3::unit_y()).LH();
        m_checker = std::make_unique<CheckerBoard>(*this, Float(100));
    }
private:
    virtual void onUpdate(Float deltaTime) override
    {
        m_depthbuf->clear(1.0f);
        m_colorbuf->clear(ColorValue(.33f, .333f, .315f, 0.f));
        m_checker->render(deltaTime);
    }
private:
    std::unique_ptr<Renderable> m_checker;
};

std::unique_ptr<Example> CheckerBoardExampleCreate(uint32_t backbufferW, uint32_t backbufferH)
{
    return std::make_unique <CheckerBoardExample>(backbufferW, backbufferH);
}