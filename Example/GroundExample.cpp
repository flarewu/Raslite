#include "Example.h"

class GridVS: public VertexShader
{
public:
    GridVS()
    {
    }
private:
    virtual void execute(const VSRegisters& input, PSRegisters& output, SystemValue& sv) override
    {
        auto wvp = uniform<Mat4>(ShaderUniformI::WORLD_MATRIX) *   
                   uniform<Mat4>(ShaderUniformI::VIEW_MATRIX)  *
                   uniform<Mat4>(ShaderUniformI::PROJECTION_MATRIX);
        sv.position = input[VSRegisterI::POSITION] * wvp;

        output[PSRegisterI::COLOR] = input[VSRegisterI::COLOR];
    }
};
class GridPS: public PixelShader
{
    virtual bool execute(const PSRegisters& varyings, SystemValue& sv)
    {
        sv.targets[sv.targetIndex] = varyings[PSRegisterI::COLOR];
        return true;
    }
};
class Grid: public Renderable
{
    struct Vertex
    {
        Vec3 m_position;
        Vec4 m_color;
    };
public:
    Grid(Example& ex,uint32_t cellCount,Float cellLength)
        : Renderable(ex)
        , m_cellCount(cellCount)
        , m_cellLength(cellLength)
    {
        InputElement inputElements[] =
        {
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::POSITION},
            {VertexStreamI::_0,Format::VECTOR4,VSRegisterI::COLOR},
        };
        m_ilayout = std::make_unique<InputLayout>(inputElements, lengthof(inputElements));
        m_context.ia.layout = m_ilayout.get();
        m_context.ia.topology = PrimitiveTopology::LINE_LIST;

        const auto extent     = m_cellCount*m_cellLength;
        const auto halfExtent = extent*Float(0.5);
        const Vec3 corner(-halfExtent, 0, -halfExtent);
        m_vertexCount = (m_cellCount+1)*2*2;
        m_vbuffer = std::make_unique<VertexBuffer>(sizeof(Vertex) *m_vertexCount);
        {
            auto vptr = m_vbuffer->getData<Vertex>();

            Vertex begVertex ={ corner,                    ColorValue::RED };
            Vertex endVertex ={ corner + Vec3(extent,0,0), ColorValue::RED };
            // 横轴: X
            for(int i = 0; i <= m_cellCount; ++i)
            {
                vptr->m_position = begVertex.m_position;
                vptr->m_color    = begVertex.m_color;
                ++vptr;
                vptr->m_position = endVertex.m_position;
                vptr->m_color    = endVertex.m_color;
                ++vptr;

                begVertex.m_position.z += m_cellLength;
                endVertex.m_position.z += m_cellLength;
            }
            // 纵轴: Z
            begVertex.m_position = corner;
            endVertex.m_position = corner + Vec3(0, 0, extent);
            for(int i = 0; i <= m_cellCount; ++i)
            {
                vptr->m_position = begVertex.m_position;
                vptr->m_color    = begVertex.m_color;
                ++vptr;
                vptr->m_position = endVertex.m_position;
                vptr->m_color    = endVertex.m_color;
                ++vptr;

                begVertex.m_position.x += m_cellLength;
                endVertex.m_position.x += m_cellLength;
            }
        }
        auto& vstrm = m_context.ia.vstreams[0];
        {
            vstrm.vbuffer = m_vbuffer.get();
            vstrm.offset = 0;
            vstrm.stride = sizeof(Vertex);
        }
        m_vshader = std::make_unique<GridVS>();
        {
            m_vshader->uniform(ShaderUniformI::WORLD_MATRIX, Transform::translate(0, 0, 0));
            m_vshader->uniform(ShaderUniformI::PROJECTION_MATRIX,m_example.getProjection());
        }
        m_context.vs = m_vshader.get();

        m_pshader = std::make_unique<GridPS>();
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
        m_context.rs.fillMode              = FillMode::WIRE_FRAME;
        m_context.rs.frontCounterClockwise = true;
        m_context.om.depthEnabled          = true;
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
    uint32_t m_indexCount;
    uint32_t m_vertexCount;

    uint32_t m_cellCount  = 100;
    Float    m_cellLength = Float(1);
};
class FloorVS: public VertexShader
{
public:
    FloorVS()
    {
    }
private:
    virtual void execute(const VSRegisters& input, PSRegisters& output, SystemValue& sv) override
    {
        auto wvp = uniform<Mat4>(ShaderUniformI::WORLD_MATRIX) *
                   uniform<Mat4>(ShaderUniformI::VIEW_MATRIX)  *
                   uniform<Mat4>(ShaderUniformI::PROJECTION_MATRIX);
        sv.position = input[VSRegisterI::POSITION] * wvp;

        output[PSRegisterI::COLOR] = input[VSRegisterI::COLOR];
    }
};
class FloorPS: public PixelShader
{
    virtual bool execute(const PSRegisters& varyings, SystemValue& sv)
    {
        sv.targets[sv.targetIndex] = varyings[PSRegisterI::COLOR];
        return true;
    }
};
class Floor: public Renderable
{
    struct Vertex
    {
        Vec3 m_position;
        Vec4 m_color;
    };
public:
    Floor(Example& ex,Float extent)
        : Renderable(ex)
        , m_extent(extent)
    {
        InputElement inputElements[] =
        {
            {VertexStreamI::_0,Format::VECTOR3,VSRegisterI::POSITION},
            {VertexStreamI::_0,Format::VECTOR4,VSRegisterI::COLOR},
        };
        m_ilayout = std::make_unique<InputLayout>(inputElements, lengthof(inputElements));
        m_context.ia.layout = m_ilayout.get();
        m_context.ia.topology = PrimitiveTopology::TRIANGLE_STRIP;

        const auto halfExtent = m_extent*Float(0.5);
        const Vec3 corner(-halfExtent, 0, -halfExtent);
        m_vertexCount = 4;
        m_vbuffer = std::make_unique<VertexBuffer>(sizeof(Vertex) *m_vertexCount);
        {
            Vertex vertex = {corner, {0x80/255.0f, 0xc0/255.0f, 0xd0/255.0f, 0xb0/255.0f}};

            auto vptr = m_vbuffer->getData<Vertex>();
            // 按CCW排列顶点
            //左下
            vptr->m_position = vertex.m_position;
            vptr->m_color    = vertex.m_color;
            ++vptr;
            //右下
            vptr->m_position = vertex.m_position + Vec3(m_extent, 0, 0);
            vptr->m_color    = vertex.m_color;
            ++vptr;
            //左上
            vptr->m_position = vertex.m_position + Vec3(0,0,m_extent);
            vptr->m_color    = vertex.m_color;
            ++vptr;
            //右上
            vptr->m_position = vertex.m_position + Vec3(m_extent, 0, m_extent);
            vptr->m_color    = vertex.m_color;
            ++vptr;
 
        }
        auto& vstrm = m_context.ia.vstreams[0];
        {
            vstrm.vbuffer = m_vbuffer.get();
            vstrm.offset = 0;
            vstrm.stride = sizeof(Vertex);
        }
        m_vshader = std::make_unique<FloorVS>();
        {
            m_vshader->uniform(ShaderUniformI::WORLD_MATRIX, Transform::translate(0, 0, 0));
            m_vshader->uniform(ShaderUniformI::PROJECTION_MATRIX, m_example.getProjection());
        }
        m_context.vs = m_vshader.get();

        m_pshader = std::make_unique<FloorPS>();
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
};
class GroundExample: public Example
{
    struct Vertex
    {
        Vec3 m_position;
        Vec4 m_color;
    };
public:
    GroundExample(uint32_t backbufferW, uint32_t backbufferH)
        : Example(backbufferW,backbufferH)
    {
        m_colorbuf = std::make_unique<Surface>(backbufferW, backbufferH, Format::R32G32B32_FLOAT);
        m_depthbuf = std::make_unique<Surface>(backbufferW, backbufferH, Format::R32_FLOAT);

        m_view = Transform::LookAt().eye(Vec3(0, 5, -10)).at(Vec3(0,0,20)).up(Vec3::unit_y()).LH();

        constexpr auto cellCount = 20;
        constexpr auto cellLength = 2.0f;
        m_grid  = std::make_unique<Grid>(*this,cellCount,cellLength);
        m_floor = std::make_unique<Floor>(*this,cellCount*cellLength);
    }
private:
    virtual void onUpdate(Float deltaTime) override
    {
        m_depthbuf->clear(1.0f);
        m_colorbuf->clear(ColorValue(.33f, .333f, .315f, 0.f));

        m_grid->render(deltaTime);
        m_floor->render(deltaTime);
    }
private:
    std::unique_ptr<Renderable> m_grid;
    std::unique_ptr<Renderable> m_floor;
};

std::unique_ptr<Example> GroundExampleCreate(uint32_t backbufferW, uint32_t backbufferH)
{
    return std::make_unique <GroundExample>(backbufferW, backbufferH);
}