#ifndef RLEXAMPLE_H
#define RLEXAMPLE_H 
#include "../Raslite/Raslite.h"
#include "../RasliteX/RasliteXInput.h"
using namespace rl;
using namespace rlx;
#define RLEXAMPLE_PATH(fileName) "../../../../Asset/"fileName
struct ShaderUniformI
{
    enum EnumMat4: uint8_t
    {
        WORLD_MATRIX = 0,
        VIEW_MATRIX ,
        PROJECTION_MATRIX ,
        WVP_MATRIX,
    };
    enum EnumVec4: uint8_t
    {
        LIGHT_DIRECTION = 0, // world space
        LIGHT_INTENSITY,
        SCALE,
        VIEW_DIRECTION,
        ALBEDO,
        VEC4_0,
        VEC4_1,
        //.....
    };
    enum EnumFloat: uint8_t
    {
        INNER_ALPHA,

        

        FLOAT_0,
        //
    };
};
struct VSRegisterI
{
    enum Enum: uint8_t
    {
        POSITION = 0,// model space
        COLOR,
        NORMAL, // model space
        TEX_UV0,
    };
};
struct PSRegisterI
{
    enum Enum: uint8_t
    {
        COLOR = 0,
        NORMAL,
        TEX_UV0,
    };
};
constexpr Float _NEAR   = 1.0f;// 改成0.01f会有问题
constexpr Float _FAR    = 1000.f;
constexpr Float _FOV_Y  = PI / 3.f;

class Example
{
public:
    Example(uint32_t backbufferW, uint32_t backbufferH)
        : m_backbufferW(backbufferW), m_backbufferH(backbufferH)
    {
        m_pipeline = std::make_unique<Pipeline>();
        //m_camera   = std::make_unique<Camera>();
       m_projection = Transform::ProjectFov().aspect(1.f * m_backbufferW / backbufferH).yFov(_FOV_Y).zNear(_NEAR).zFar(_FAR).LH();
    }
    Pipeline* getPipeline() const
    {
        return m_pipeline.get();
    }
    //Camera* getCamera() const
    //{
    //    return m_camera.get();
    //}
    Surface* getRenderTarget(uint8_t idx = 0) const
    {
        return m_colorbuf.get();
    }
    Surface* getDepthStencil() const
    {
        return m_depthbuf.get();
    }
    uint32_t getBackBufferW() const
    {
        return m_backbufferW;
    }
    uint32_t getBackBufferH() const
    {
        return m_backbufferH;
    }
    Float getAspectRatio() const
    {
        return Float(m_backbufferW)/m_backbufferH;
    }
    void update(Float deltaTime)  
    {
        //m_camera->update(deltaTime);
        this->onUpdate(deltaTime);
    }
    virtual void setFillMode(FillMode fm) 
    {
        m_context.rs.fillMode = fm;
    }
    virtual FillMode getFillMode() const
    {
        return m_context.rs.fillMode;
    }
    void setFreezed(bool val)
    {
        m_freezed = val;
    }
    bool isFreezed() const
    {
        return m_freezed;
    }
    const Mat4& getProjection() const
    {
        return m_projection;
    }
    const Mat4& getView() const
    {
        return m_view;
    }
protected:
    virtual void onUpdate(Float deltaTime)
    {
    }
protected:
    bool m_freezed = false;
    uint32_t m_backbufferW, m_backbufferH;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<Surface>  m_colorbuf;
    std::unique_ptr<Surface>  m_depthbuf;

    Mat4 m_projection;
    Mat4 m_view;

    //std::unique_ptr<Camera>  m_camera;
    ///////////
    Viewport m_viewport;                     //deprecated
    Context  m_context;                      //deprecated
    Rect     m_scissor;                      //deprecated
                                             //deprecated
    std::unique_ptr<InputLayout>  m_ilayout; //deprecated
    std::unique_ptr<IndexBuffer>  m_ibuffer; //deprecated
    std::unique_ptr<VertexBuffer> m_vbuffer; //deprecated
    std::unique_ptr<PixelShader>  m_pshader; //deprecated
    std::unique_ptr<VertexShader> m_vshader; //deprecated
};

class Renderable
{
public:
    explicit Renderable(Example& ex): m_example(ex) {};
    virtual void render(Float deltaTime) {};
protected:
    Viewport m_viewport;
    Context  m_context;
    Rect     m_scissor;

    std::unique_ptr<InputLayout>  m_ilayout;
    std::unique_ptr<IndexBuffer>  m_ibuffer;
    std::unique_ptr<VertexBuffer> m_vbuffer;
    std::unique_ptr<PixelShader>  m_pshader;
    std::unique_ptr<VertexShader> m_vshader;

    Example& m_example;
};
#endif//RLEXAMPLE_H