#include "Raslite.h"
#include <tuple>
#include <algorithm>
namespace rl
{
    struct VSInput
    {
        VSRegisters	registers;//用于存放顶点属性
    };
    struct VSOutput
    {
        PSRegisters registers;
        Vec4		position;
		void registerMul(Float factor)
		{
			for(auto& r : this->registers)
				r *= factor;
        }
		void setRegisterMul(const VSOutput& v, Float factor)
		{
			for(uint32_t i = 0; i < lengthof<PSRegisters>(); ++i)
				this->registers[i] = v.registers[i] * factor;
        }
    };
    struct Vertex
    {
        VSInput  input;
        VSOutput output;
    };
    class PipelineChild
    {
    public:
        void setContext(const Context* ctx)
        {
            m_context = ctx;
        }
    protected:
        const Context* m_context = nullptr;
    };
    ///////////////////////////////////////////////////////////////////////
	//InputLayout
	///////////////////////////////////////////////////////////////////////
	InputLayout::InputLayout(const InputElement* elements, uint32_t count)
		: m_nElement(count)
	{
		assert(elements && count > 0);
		for (uint32_t i = 0; i < m_nElement; ++i)
		{
			auto& curr = elements[i];
			assert(curr.streami   < VERTEX_STREAM_COUNT);
			assert(curr.registeri < VS_REGISTER_COUNT);
			if (curr.streami > m_highestStreamIndex)
				m_highestStreamIndex = curr.streami;
		}
		m_elements = std::make_unique<InputElement[]>(m_nElement);
		std::memcpy(m_elements.get(), elements, sizeof(InputElement) * m_nElement);
        //
        {
            for(uint32_t i = 0; i < VS_REGISTER_COUNT; ++i)
                m_registerTypes[i] = ShaderRegisterType::UNUSED;
            for(auto i = 0; i < m_nElement; ++i)
            {
                auto& elem = elements[i];
                m_registerTypes[elem.registeri] = ToShaderRegisterType(elem.format);
            }
        }
	}
	InputLayout::~InputLayout()
	{
	}
    ///////////////////////////////////////////////////////////////////////
    void lerp(const VSOutput& v0, const VSOutput& v1, Float factor, VSOutput& v2Out)
    {
        v2Out.position = v0.position.lerp(v1.position, factor);

        auto rOut = v2Out.registers; auto r0 = v0.registers, r1 = v1.registers;
		for(uint32_t i = 0; i < lengthof<PSRegisterTypes>(); ++i, ++rOut, ++r0, ++r1)
            *rOut = r0->lerp(*r1, factor);
    }
 }//ns rl
namespace rl
{
    /*
         E(X,Y) = aX + bY + c = 0;
         用于判Screen Point是否再Edge中
         坐标系(右手坐标系)(即: 逆时针三角形面积为正)
          ---------------->x
          |
         \|/y
    */
     struct EdgeEquation
     {
     public:
         EdgeEquation() {}
         EdgeEquation(const Vec2& v0, const Vec2& v1)
         {
             /*根据两点式求出直线方程 aX + bY + c = 0;
                     (y-y0)/(x-x0)  = (y1-y0)/(x1-x0)  
                  => (y-y0)*(x1-x0) = (y1-y0)*(x-x0)
                  => (x1-x0)*y -(x1-x0)y0 = (y1-y0)*x - (y1-y0)*x0
                  => (y1-y0)*x -(x1-x0)*y +(x1-x0)y0 -(y1-y0)*x0 = 0
             */
             auto tmp = v1 - v0;
             m_a =  tmp.y;
             m_b = -tmp.x;
             m_c =  tmp.x*v0.y - tmp.y*v0.x;
             //Top: 水平且x大->x小; Left: y小->y大
             m_topLeft =  v0.y < v1.y/*left*/ || (v1.y == v0.y && v0.x > v1.x)/*top*/;
         }
         // 求解E(X,Y) = aX + bY + c
		 Float evaluate(Float x, Float y) const
		 {
			 return m_a * x + m_b * y + m_c;
		 }
		 Float evaluate(int x, int y) const
		 {
			 return this->evaluate(x + 0.5f, y + 0.5f);
		 }
         // E(X + stepsz, Y) = a(X+stepsz) + bY + c = E(X,Y) + a*stepsz
         // 传入的的e为某个E(X,Y),获取(X+stepsz)的E值
         Float deltaX(Float stepsz = 1) const
         {
             return m_a * stepsz;
         }
         // E(X, Y+stepsz) = aX + b(Y+stepsz) + c = E(X,Y) + b*stepsz
         // 所以传入的的e为某个E(X,Y),然后获取下一个X(即X+1)的E值
         Float deltaY(Float stepsz = 1) const
         {
             return m_b * stepsz;
         }
         bool inside(Float val) const
         {
             return val > 0 || (val == 0 && m_topLeft);
         }
         bool inside(int x, int y) const
         {
             return this->inside(this->evaluate(x, y));
         }
     public:
         Float m_a, m_b, m_c;
         bool m_topLeft;
     };
     // 每个属性都有自己的Plane Attribute Equation
     // F = aX + bY + c;
     struct PlaneEquation
     {
     public:
         PlaneEquation() {};
         // 三个点的坐标分别为v0,v1,v2;
         // 三个点的属性分别为a0,a1,a2;
         PlaneEquation(const Vec2& v0,const Vec2& v1,const Vec2& v2,const Vec4& a0,const Vec4& a1,const Vec4& a2)
         {
             /*
                 a0 = Ax0 + By0 + C
                 a1 = Ax1 + By1 + C
                 a2 = Ax2 + By2 + C
             */
             auto sa = (v0.x - v1.x)*(v0.y - v2.y) - (v0.x - v2.x)*(v0.y-v1.y);
             assert(sa != 0);
             auto va = (a0 - a1)*(v0.y - v2.y) - (a0 - a2)*(v0.y - v1.y);
             m_a = va / sa;

             auto sb = (v0.y - v1.y)*(v0.x - v2.x) - (v0.y - v2.y)*(v0.x-v1.x);
             assert(sb != 0);
             auto vb = (a0 - a1)*(v0.x - v2.x) - (a0 - a2)*(v0.x - v1.x);
             m_b = vb / sb;

             m_c = a0 - m_a*v0.x - m_b*v0.y;
#ifdef _DEBUG
             auto c1 = a1 - m_a*v1.x - m_b*v1.y;
             auto c2 = a2 - m_a*v2.x - m_b*v2.y;
             assert(m_c.equal(c1, 0.01f));
             assert(m_c.equal(c2, 0.01f));
#endif
         }
         // E(X + stepsz, Y) = a(X+stepsz) + bY + c = E(X,Y) + a*stepsz
         // 传入的的e为某个E(X,Y),获取(X+stepsz)的E值
         Vec4 deltaX(Float stepsz = 1) const
         {
             return m_a * stepsz;
         }
         // E(X, Y+stepsz) = aX + b(Y+stepsz) + c = E(X,Y) + b*stepsz
         // 所以传入的的e为某个E(X,Y),然后获取下一个X(即X+1)的E值
         Vec4 deltaY(Float stepsz = 1) const
         {
             return m_b * stepsz;
         }
         Vec4 evaluate(Float x, Float y) const
         {
             return m_a * x + m_b * y + m_c;
         }
		 Vec4 evaluate(int x, int y) const
		 {
			 return this->evaluate(x + 0.5f, y + 0.5f);
		 }
     public:
         // F(X,Y) = aX + bY + c的系数
         Vec4 m_a, m_b, m_c;
     };
     struct TriangleEquation
     {
         EdgeEquation m_e01, m_e12, m_e20;
         Float m_area;
         PlaneEquation m_attributeEqns[lengthof<PSRegisters>()];
         //(nonlinearDepth,linearDepthInv)
         PlaneEquation m_depthEqn;
         bool inside(int x, int y) const
         {
             return m_e01.inside(x, y)
                 && m_e12.inside(x, y)
                 && m_e20.inside(x, y);
         }
         bool inside(Float edgeVal0, Float edgeVal1, Float edgeVal2) const
         {
             return m_e01.inside(edgeVal0)
                 && m_e12.inside(edgeVal1)
                 && m_e20.inside(edgeVal2);
         }
         bool inside(const Vec3& edgeVals) const
         {
             return this->inside(edgeVals[0], edgeVals[1], edgeVals[2]);
         }
         bool isDegenerate() const
         {
             return std::fabs(m_area) < 0.1f;
         }
     };
     struct PixelTraverser
     {
         static std::pair<Vec2i, Vec2i> calcBoundingBox(const Vec2i points[3])
         {
             auto boxMin = points[0], boxMax = points[0];
             for(auto i = 1; i < 3; ++i)
             {
                 auto& pt = points[i];
                 if(boxMin.x > pt.x) boxMin.x = pt.x;
                 if(boxMax.x < pt.x) boxMax.x = pt.x;
                 if(boxMin.y > pt.y) boxMin.y = pt.y;
                 if(boxMax.y < pt.y) boxMax.y = pt.y;
             }
             boxMin.x = std::max(boxMin.x, 0);
             boxMin.y = std::max(boxMin.y, 0);
             boxMax.x = std::max(boxMax.x, 0);
             boxMax.y = std::max(boxMax.y, 0);
             return std::make_pair(boxMin, boxMax);
         }

         PixelTraverser(const TriangleEquation& tri, const Vec2i points[3])
             : m_triangleEqn(tri)
         {
             std::tie(m_boxMin, m_boxMax) = PixelTraverser::calcBoundingBox(points);
             m_pixelCoords = Vec2i(m_boxMin.x, m_boxMin.y);
             this->_evaluate();
         }
         uint32_t getPixelCount() const
         {
             return (m_boxMax.y - m_boxMin.y)*(m_boxMax.x - m_boxMin.x);
         }
         bool traverse()
         {
             if(m_pixelCoords.y > m_boxMax.y)
                 return false;
             if(m_forward)
             {
                 if(m_pixelCoords.x < m_boxMax.x)
                     this->_stepX_Forward();
                 else
                     this->_stepY();
             }
             else
             {
                 if(m_pixelCoords.x >= m_boxMin.x)
                     this->_stepX_Backward();
                 else
                     this->_stepY();
             }
             return true;
         }
         bool isInsideTriangle() const
         {
             return m_triangleEqn.inside(m_edgeValues);
         }
         const PSRegisters& getAtrributes() const
         {
             return m_attributes;
         }
         Float getNonlinearDepth() const
         {
             return m_depth.x;
         }
         Float getInverseLinearDepth() const
         {
             return m_depth.y;
         }
         const Vec2i& getPixelCoords() const
         {
             return m_pixelCoords;
         }
     private:
         void _evaluate()
         {
             auto pixelPos = Vec2(m_pixelCoords.x + Float(0.5), m_pixelCoords.y + Float(0.5));
             m_edgeValues  ={ m_triangleEqn.m_e01.evaluate(pixelPos.x,pixelPos.y),
                              m_triangleEqn.m_e12.evaluate(pixelPos.x,pixelPos.y),
                              m_triangleEqn.m_e20.evaluate(pixelPos.x,pixelPos.y) };
             for(uint32_t i = 0; i < lengthof<PSRegisters>(); ++i)
                 m_attributes[i] = m_triangleEqn.m_attributeEqns[i].evaluate(pixelPos.x, pixelPos.y);
             m_depth = m_triangleEqn.m_depthEqn.evaluate(pixelPos.x, pixelPos.y);
         }
         void _stepX_Forward()
         {
             ++m_pixelCoords.x;
             //this->_evaluate();
             //return;
             {
                 auto dx = Vec3{ m_triangleEqn.m_e01.deltaX(),
                                 m_triangleEqn.m_e12.deltaX(),
                                 m_triangleEqn.m_e20.deltaX() };
                 m_edgeValues += dx;
             }
             for(uint32_t i = 0; i < lengthof<PSRegisters>(); ++i)
                 m_attributes[i] += m_triangleEqn.m_attributeEqns[i].deltaX();

             m_depth += m_triangleEqn.m_depthEqn.deltaX();
         }
         void _stepX_Backward()
         {
             --m_pixelCoords.x;
             //this->_evaluate();
             //return;
             {
                 auto dx = Vec3{ m_triangleEqn.m_e01.deltaX(),
                                 m_triangleEqn.m_e12.deltaX(),
                                 m_triangleEqn.m_e20.deltaX()};
                 m_edgeValues -= dx;
             }
             for(uint32_t i = 0; i < lengthof<PSRegisters>(); ++i)
                 m_attributes[i] -= m_triangleEqn.m_attributeEqns[i].deltaX();

             m_depth -= m_triangleEqn.m_depthEqn.deltaX();
         }
         void _stepY()
         {
             ++m_pixelCoords.y;
             m_forward = !m_forward;
             //this->_evaluate();
             //return;
             m_edgeValues += { m_triangleEqn.m_e01.deltaY(),
                               m_triangleEqn.m_e12.deltaY(),
                               m_triangleEqn.m_e20.deltaY()};
             for(uint32_t i = 0; i < lengthof<PSRegisters>(); ++i)
                 m_attributes[i] += m_triangleEqn.m_attributeEqns[i].deltaY();
             m_depth += m_triangleEqn.m_depthEqn.deltaY();
         }
     private:
         const TriangleEquation& m_triangleEqn;
         Vec2i m_pixelCoords;

         Vec3        m_edgeValues; //分别对应e01,e02,e03边方程的值
         PSRegisters m_attributes;
         Vec4        m_depth;      //(NonlinearDepth,1/LinearDepth)
         bool        m_forward = true;
         Vec2i       m_boxMin, m_boxMax;
     };
 }
//Pipeline Child
namespace rl
{
    ///////////////////////////////////////////////////////////////////////
	//Clipper
	///////////////////////////////////////////////////////////////////////
#define _RB_NEXT_GET(stage)  ((stage + 1) & 1) //0 -> 1 -> 0 -> 1->0....
#define _RB_NEXT_MOV(stage)  stage = _RB_NEXT_GET(stage)
    class Clipper: public PipelineChild
    {
    public:
        explicit Clipper()
            : m_nClipVertices(0) , m_stage(0)
        {
            std::memset(&m_clipVertices, 0, sizeof(m_clipVertices));
            std::memset(&m_pVisibleVertices, 0, sizeof(m_pVisibleVertices));
        }
        void reset()
        {
            m_clippingPlanes[ClippingPlaneType::LEFT]   = Plane().setNormal({ 1, 0, 0 }).setD(1);
            m_clippingPlanes[ClippingPlaneType::RIGHT]  = Plane().setNormal({-1, 0, 0 }).setD(1);
            m_clippingPlanes[ClippingPlaneType::TOP]    = Plane().setNormal({ 0,-1, 0 }).setD(1);
            m_clippingPlanes[ClippingPlaneType::BOTTOM] = Plane().setNormal({ 0, 1, 0 }).setD(1);
            m_clippingPlanes[ClippingPlaneType::NEARx]  = Plane().setNormal({ 0, 0, 1 }).setD(0);
            m_clippingPlanes[ClippingPlaneType::FARx]   = Plane().setNormal({ 0, 0,-1 }).setD(1);

            for(int i = 0; i < ClippingPlaneType::USER0; ++i)
                m_clippingPlanesEnabled[i] = true;
            for(int i = ClippingPlaneType::USER0; i < ClippingPlaneType::MAX; ++i)
                m_clippingPlanesEnabled[i] = false;

            m_stage = 0;
            m_nClipVertices = 0;
            m_nVisibleVertices = 0;
        }
        void setContext(const Context* ctx)
        {
            if(ctx)
            {
                if(ctx->rs.scissorEnabled)
                {
                    auto& rect = ctx->rs.scissorRects[0];
                    m_scissorPlanes[0] = Plane(1, 0, 0, -(Float)rect.left);
                    m_scissorPlanes[1] = Plane(-1, 0, 0, (Float)rect.right);
                    m_scissorPlanes[2] = Plane(0, 1, 0, -(Float)rect.top);
                    m_scissorPlanes[3] = Plane(0, -1, 0, (Float)rect.bottom);
                }
            }
            else 
                 this->reset();
            PipelineChild::setContext(ctx);
        }
        void planeClip(const VSOutput& v0, const VSOutput& v1)
        {// 裁剪Line
            this->reset();

            std::memcpy(&m_clipVertices[0], &v0, sizeof(VSOutput));
            std::memcpy(&m_clipVertices[1], &v1, sizeof(VSOutput));
            m_nClipVertices = 2;

            m_pVisibleVertices[m_stage][0] = &m_clipVertices[0];
            m_pVisibleVertices[m_stage][1] = &m_clipVertices[1];
            m_nVisibleVertices = 2;
            // Frustum planes clipping和User planes clipping
            for(uint32_t i = 0; i < ClippingPlaneType::MAX; ++i)
            {
                if(!m_clippingPlanesEnabled[i])
                    continue;
                this->_clipLineToPlane(m_clippingPlanes[i], true/*homo*/);
                if(m_nVisibleVertices < 2)
                    break;
                _RB_NEXT_MOV(m_stage);
            }
        }
        void planeClip(const VSOutput& v0, const VSOutput& v1, const VSOutput& v2)
        {//裁剪Triangle
            this->reset();

            std::memcpy(&m_clipVertices[0], &v0, sizeof(VSOutput));
            std::memcpy(&m_clipVertices[1], &v1, sizeof(VSOutput));
            std::memcpy(&m_clipVertices[2], &v2, sizeof(VSOutput));
            m_nClipVertices = 3;

            m_pVisibleVertices[m_stage][0] = &m_clipVertices[0];
            m_pVisibleVertices[m_stage][1] = &m_clipVertices[1];
            m_pVisibleVertices[m_stage][2] = &m_clipVertices[2];
            m_nVisibleVertices = 3;

            // 首先调用GS
            //auto gs = m_context->gs;
            //if(gs)
            //{
            //    if(!gs->execute(m_clipVertices[0].registers,
            //                    m_clipVertices[1].registers,
            //                    m_clipVertices[2].registers))
            //    {
            //        return;
            //    }
            //}
            // Frustum planes clipping和User planes clipping
            for(uint32_t i = 0; i < ClippingPlaneType::MAX; ++i)
            {
                if(!m_clippingPlanesEnabled[i])
                    continue;
                this->_clipTriangleToPlane(m_clippingPlanes[i], true/*homo*/);
                if(m_nVisibleVertices < 3)
                    break;
                _RB_NEXT_MOV(m_stage);
            }
        }
        void scissorClip()
        {
            for(uint32_t i = 0; i < 4; ++i, _RB_NEXT_MOV(m_stage))
            {
                this->_clipTriangleToPlane(m_scissorPlanes[i], false/*homo*/);
                if(m_nVisibleVertices < 3)
                    break;
            }
        }
        uint32_t getVisibleVertexCount() const
        {
            return m_nVisibleVertices;
        }
        VSOutput&  getVisibleVertex(uint32_t idx)
        {
            assert(idx < m_nVisibleVertices);
            return *m_pVisibleVertices[m_stage][idx];
        }
        VSOutput** getVisibleVertices()
        {
            return m_pVisibleVertices[m_stage];
        }
    private:
        // m_clipVertices开始为三个顶点，裁剪后产生的新的顶点和老的顶点数据都会存在此处
        // m_pVisibleVertices只是引用m_clipVertices中的可见顶点罢了;
        void _clipTriangleToPlane(const Plane& plane, bool homo)
        {
            auto src = m_pVisibleVertices[m_stage];               // 对于当前Plane的待裁剪的顶点
            auto dst = m_pVisibleVertices[_RB_NEXT_GET(m_stage)]; // 对于当前Plane的可见的顶点

            uint32_t nNonClippedVertices = 0;
            for(uint32_t i = 0, j = 1; i < m_nVisibleVertices; ++i, ++j)
            {
                if(j == m_nVisibleVertices)
                    j = 0;
                Float di, dj;
                {
                    if(homo)
                    {
                        di = plane.distanceTo(src[i]->position);
                        dj = plane.distanceTo(src[j]->position);
                    }
                    else
                    {
                        di = plane.distanceTo(*(Vec3*)&src[i]->position);
                        dj = plane.distanceTo(*(Vec3*)&src[j]->position);
                    }
                }
                if(di >= 0.0f)
                {// 顶点i在内部
                    dst[nNonClippedVertices++] = src[i];
                    if(dj < 0.0f)
                    {// 顶点j在外部
                        auto& result = m_clipVertices[m_nClipVertices];
                        lerp(/*m_context->vs->getOutputRegisterTypes(),*/ *src[i], *src[j], di / (di - dj), result);
                        dst[nNonClippedVertices++] = &result;
                        m_nClipVertices = (m_nClipVertices + 1) % 20;
                        assert(m_nClipVertices != 0 && "Clip Vertex Array太小了!");
                    }
                }
                else
                {//顶点i在外部
                    if(dj >= 0.0f)
                    {// 顶点j在内部
                        auto& result = m_clipVertices[m_nClipVertices];
                        lerp(/*m_context->vs->getOutputRegisterTypes(),*/ *src[j], *src[i], dj / (dj - di), result);
                        dst[nNonClippedVertices++] = &result;
                        m_nClipVertices = (m_nClipVertices + 1) % 20;
                        assert(m_nClipVertices != 0 && "Clip Vertex Array太小了!");
                    }
                }
            }//for
            m_nVisibleVertices = nNonClippedVertices;
        }
        void _clipLineToPlane(const Plane& plane, bool homo)
        {
            assert(m_nVisibleVertices == 2);
            auto src = m_pVisibleVertices[m_stage];               // 对于当前Plane的待裁剪的顶点
            auto dst = m_pVisibleVertices[_RB_NEXT_GET(m_stage)]; // 对于当前Plane的可见的顶点

            uint32_t nNonClippedVertices = 0;
            for(uint32_t i = 0, j = 1; i < m_nVisibleVertices - 1; ++i, ++j)
            {
                Float di, dj;
                {
                    if(homo)
                    {
                        di = plane.distanceTo(src[i]->position);
                        dj = plane.distanceTo(src[j]->position);
                    }
                    else
                    {
                        di = plane.distanceTo(*(Vec3*)&src[i]->position);
                        dj = plane.distanceTo(*(Vec3*)&src[j]->position);
                    }
                }
                if(di >= 0.0f)
                {// 顶点i在Plane内部
                    dst[nNonClippedVertices++] = src[i];
                    if(dj < 0.0f)
                    {// 顶点j在Plane外部
                        auto& result = m_clipVertices[m_nClipVertices];
                        lerp(/*m_context->vs->getOutputRegisterTypes(),*/ *src[i], *src[j], di / (di - dj), result);
                        dst[nNonClippedVertices++] = &result;
                        m_nClipVertices = (m_nClipVertices + 1) % 20;
                        assert(m_nClipVertices != 0 && "Clip Vertex Array太小了!");
                    }
                    else
                        dst[nNonClippedVertices++] = src[j];
                }
                else
                {//顶点i在Plane外部
                    if(dj >= 0.0f)
                    {// 顶点j在Plane内部
                        auto& result = m_clipVertices[m_nClipVertices];
                        lerp(/*m_context->vs->getOutputRegisterTypes(),*/ *src[j], *src[i], dj / (dj - di), result);
                        dst[nNonClippedVertices++] = &result; // new i
                        m_nClipVertices = (m_nClipVertices + 1) % 20;
                        assert(m_nClipVertices != 0 && "Clip Vertex Array太小了!");
                        dst[nNonClippedVertices++] = src[j];
                    }
                }
            }//for
            m_nVisibleVertices = nNonClippedVertices;
        }
    private:
        // 所有的顶点数据(包括因为裁剪，新增加的顶点)存储在此处
        VSOutput  m_clipVertices[20];
        // 所有的顶点的个数
        uint32_t  m_nClipVertices;
        // 每一次次裁剪都可能会有一些顶点不可见被裁掉)
        VSOutput* m_pVisibleVertices[2][20];
        // 当前可见的顶点个数
        uint32_t  m_nVisibleVertices;
        // 等于0或1,即: 分两个阶段
        uint32_t  m_stage;

        Plane m_clippingPlanes       [ClippingPlaneType::MAX];
        bool  m_clippingPlanesEnabled[ClippingPlaneType::MAX];
        Plane m_scissorPlanes[4];

    };
	///////////////////////////////////////////////////////////////////////
	//Vertexer
	///////////////////////////////////////////////////////////////////////
	// 顶点处理器: 缓存已经处理的顶点,负责调用Vertex Shader
	class Vertexer: public PipelineChild
	{
	public:
		struct Entry
		{
			// 在Vertex Buffer中的索引
			uint32_t index;
			Vertex   vertex;
			uint32_t fetches;
		};
	private:
		uint32_t m_count;
		uint32_t m_fetchs;
		Entry m_entries[VERTEX_CACHE_CAPACITY];

	public:
		Vertexer() : m_count(0), m_fetchs(0)
		{
			std::memset(&m_entries, 0, sizeof(m_entries));
		}
		void reset()
		{
			m_count = 0;
			m_fetchs = 0;
		}
        void setContext(const Context* ctx)
        {
            if(!ctx)
                this->reset();
            PipelineChild::setContext(ctx);
        }
        Entry* fetch(uint32_t vertexi)
        {
            auto curr = &m_entries[0];
            auto dest = curr;
            for(uint32_t i = 0; i < m_count; ++i, ++curr)
            {
                if(curr->index == vertexi)
                {//命中cache
                    curr->fetches = m_fetchs++;
                    return curr;
                }
                //使用fetches次数最少的
                if(curr->fetches < dest->fetches)
                    dest = curr;
            }
            if(m_count < VERTEX_CACHE_CAPACITY)
                dest = &m_entries[m_count++];

            dest->index   = vertexi;
            dest->fetches = m_fetchs++;
            // 执行vertex shader
            {
                auto& vsi = dest->vertex.input;
                auto& vso = dest->vertex.output;

                this->assembleVertex(vertexi, vsi);
                VertexShader::SystemValue sv;
                {

                }
                m_context->vs->execute(vsi.registers, vso.registers, sv);
                vso.position = sv.position;
            }
            return dest;
        }
        void assembleVertex(uint32_t vertexi, VSInput& vsiOut)
        {
            const uint8_t* dataptrs[VERTEX_STREAM_COUNT] = { nullptr };
            auto layout = m_context->ia.layout;
            const auto highestIndex = layout->getHighestStreamIndex();
            for(uint32_t i = 0; i <= highestIndex; ++i)
            {
                auto& current = m_context->ia.vstreams[i];
                if(!current.vbuffer)
                    continue;
                auto offset = current.offset + vertexi * current.stride;
                assert(offset < current.vbuffer->getLength());
                dataptrs[i] = current.vbuffer->getData<uint8_t>(offset);
            }
            for(uint32_t i = 0; i < layout->getElementNum(); ++i)
            {
                auto& elem  = layout->getElement(i);
                auto& attri = vsiOut.registers[elem.registeri];
                auto  data  = (Float*)dataptrs[elem.streami];
                switch(elem.format)
                {
                case Format::FLOAT32:
                    dataptrs[elem.streami] += sizeof(Float);
                    attri = ShaderRegister(data[0], 0, 0, 1);
                    break;
                case Format::VECTOR2:
                    dataptrs[elem.streami] += 2 * sizeof(Float);
                    attri = ShaderRegister(data[0], data[1], 0, 1);
                    break;
                case Format::VECTOR3:
                    dataptrs[elem.streami] += 3 * sizeof(Float);
                    attri = ShaderRegister(data[0], data[1], data[2], 1);
                    break;
                case Format::VECTOR4:
                    dataptrs[elem.streami] += 4 * sizeof(Float);
                    attri = ShaderRegister(data[0], data[1], data[2], data[3]);
                    break;
                default:
                    break;
                }
            }
        }
    };
    ///////////////////////////////////////////////////////////////////////
	//Rasterizer
	///////////////////////////////////////////////////////////////////////
    class Rasterizer: public PipelineChild
    {
    public:
        //返回true: 说明剔除了，否则false
        bool cull(const VSOutput& v0, const VSOutput& v1, const VSOutput& v2)
        {
            const auto cm = m_context->rs.cullMode;
            if(cm == CullMode::NONE)
                return false;
            const auto frontCCW = m_context->rs.frontCounterClockwise;
            // > 0 则逆时针, <0 则顺时针
            auto edgeVal = Float(0);
            {
                auto tmp = v1.position.xy() - v0.position.xy();
                auto a   =  tmp.y;
                auto b   = -tmp.x;
                auto c   =  tmp.x*v0.position.y - tmp.y*v0.position.x;
                edgeVal = a * v2.position.x + b * v2.position.y + c;
            }
            if(edgeVal == 0.0f)
                return true;
            auto ccw = edgeVal > 0.0f;

            auto front = frontCCW && ccw || !frontCCW && !ccw;
            return cm == CullMode::FRONT ? front : !front;
        }
        // 输入vso在homo clip space中
        //    vso.position.xyz
        //    vso.position.w为linear z
        // 返回vso值为如下: 
        //     vso.position.xy在raster space中
        //     vso.position.z为nonlinear_z(scaled)
        //     vso.position.w为1/linear_z
        //     vso.shaderRegsiters都被乘以了1/linear_z(即 a/linear_z)在raster space下才能线性插值
        void transformToViewport(VSOutput& vso)
        {
            // vso.position现在为 homogeneous space中,w为linear depth
            if(vso.position.w < FLT_EPSILON)
                return;
            // 转换到NDC Space
            const auto invW = 1.0f / vso.position.w;
            vso.position.x *= invW;
            vso.position.y *= invW;
            vso.position.z *= invW;
            vso.position.w  = 1.0f;
            // xy转换到Raster space
            vso.position.x = clamp(vso.position.x, -0.99999f, 0.99999f);
            vso.position.y = clamp(vso.position.y, -0.99999f, 0.99999f);
            vso.position.z = clamp(vso.position.z,  0.0f, 0.99999f);
            vso.position *= m_context->om.viewportTransform;
            // position.w中存放 1/linear_z(用于恢复linear registers)
            vso.position.w = invW;
            // 将所有的属性都乘以1/linear_z,因为属性a除以linear_z(即 a/linear_z)在screen space下才能线性插值
            vso.registerMul(invW);
        }
    public:
        Rasterizer()
        {
            m_clipper = std::make_unique<Clipper>();
        }
        void scheduleLine(const Vertex& v0, const Vertex& v1)
        {
            //auto vs0 = v0.output, vs1 = v1.output;
            //this->transformToViewport(vs0);
            //this->transformToViewport(vs1);
            //const Vec2i points[2] ={ vs0.position.xy(),vs1.position.xy() };
            //this->rasterizeLine(points[0].x, points[0].y, points[1].x, points[1].y, Vec4::WHITE);
            //return;

            // 在Homogeneous space中裁剪(这样才能把External Triangles给裁掉)
            m_clipper->planeClip(v0.output, v1.output);
            const auto n = m_clipper->getVisibleVertexCount();
            if(n < 2)
                return;
            assert(n == 2);
            auto vertices = m_clipper->getVisibleVertices();
            for(uint32_t i = 0; i < n; ++i)
                this->transformToViewport(*vertices[i]);
            // scissor testing
            if(m_context->rs.scissorEnabled)
            {
                m_clipper->scissorClip();
                vertices = m_clipper->getVisibleVertices();
            }
            for(uint32_t i = 0; i < n; i += 2)
            {
                const Vec2i points[2] = { vertices[i]->position.xy(),vertices[i + 1]->position.xy()};
                this->rasterizeLine(points[0].x, points[0].y, points[1].x, points[1].y, Vec4::WHITE);
            }
        }
        void scheduleTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
        {
            if(false && m_context->rs.fillMode == FillMode::WIRE_FRAME)
            {
                auto vs0 = v0.output, vs1 = v1.output, vs2 = v2.output;
                this->transformToViewport(vs0);
                this->transformToViewport(vs1);
                this->transformToViewport(vs2);

                if(this->cull(vs0, vs1, vs2))
                    return;
                const Vec2i points[3] ={ vs0.position.xy(),vs1.position.xy(),vs2.position.xy() };

                this->rasterizeLine(points[0].x, points[0].y, points[1].x, points[1].y, Vec4::WHITE);
                this->rasterizeLine(points[1].x, points[1].y, points[2].x, points[2].y, Vec4::WHITE);
                this->rasterizeLine(points[2].x, points[2].y, points[0].x, points[0].y, Vec4::WHITE);
                return;
            }

            // 在Homogeneous space中裁剪(这样才能把External Triangles给裁掉)
            m_clipper->planeClip(v0.output, v1.output, v2.output);
            const auto n = m_clipper->getVisibleVertexCount();
            if(n < 3)
                return;
            auto vertices = m_clipper->getVisibleVertices();
            // 先转换一个三角形到Raster空间用于backface culling
            for(uint32_t i = 0; i < 3; ++i)
                this->transformToViewport(*vertices[i]);
            if(this->cull(*vertices[0], *vertices[1], *vertices[2]))
                return;
            for(uint32_t i = 3; i < n; ++i)
               this->transformToViewport(*vertices[i]);
            // scissor testing
            if(m_context->rs.scissorEnabled)
            {
                m_clipper->scissorClip();
                vertices = m_clipper->getVisibleVertices();
            }
            // 执行halfspace算法来光栅化三角形
            for(uint32_t i = 1; i < n - 1; ++i)
                this->rasterizeTriangle(*vertices[0], *vertices[i], *vertices[i + 1]);
        }
        void setContext(const Context* ctx) 
        {
            if(ctx)
            {
                auto mutCtx = const_cast<Context*>(ctx);

                auto& vp = ctx->rs.viewports[0];
                mutCtx->om.viewportTransform =
                    Transform::viewport(vp.topLeftX, vp.topLeftY, vp.width, vp.height, vp.minDepth, vp.maxDepth);

                auto color = ctx->om.renderTargets[0];
                mutCtx->om.colorData = color->lock();
                mutCtx->om.colorFloatCount = color->getFormatFloatCount();
                mutCtx->om.colorBufferPitch = color->getWidth() * color->getFormatFloatCount();

                auto depth = ctx->om.depthStencil;
                mutCtx->om.depthData = depth->lock();
                mutCtx->om.depthBufferPitch = depth->getWidth();
                mutCtx->om.depthFloatCount = depth->getFormatFloatCount();
            }
            else
            {
                auto color = m_context->om.renderTargets[0];
                color->unlock();
                auto depth = m_context->om.depthStencil;
                depth->unlock();
            }
            PipelineChild::setContext(ctx);
            m_clipper->setContext(ctx);
        }
    private:
        void setColor(int x, int y, const Vec4& val)
        {
            Float* colorData = m_context->om.colorData + (y * m_context->om.colorBufferPitch + x * m_context->om.colorFloatCount);
            val.copyTo(colorData, m_context->om.colorFloatCount);
        }
        // 浮点数版本的Bresenham划线算法
        //*.steep问题:  swap来解决
        //*.x的方向问题: swap来解决
        //*.y的方向问题: 更改正负号来解决
        void rasterizeLine(int x0, int y0, int x1, int y1, const Vec4& color)
        {
            if(x0 == x1)
            {
                if(y0 <= y1)
                {
                    for(int y = y0; y <= y1; ++y)
                        setColor(x0, y, color);
                }
                else
                {
                    for(int y = y1; y <= y0; ++y)
                        setColor(x0, y, color);
                }
                return;
            }
            if(y0 == y1)
            {
                if(x0 <= x1)
                {
                    for(int x = x0; x <= x1; ++x)
                        setColor(x, y0, color);
                }
                else
                {
                    for(int x = x1; x <= x0; ++x)
                        setColor(x, y0, color);
                }
                return;
            }
            auto steep = std::abs(x1 - x0) < std::abs(y1 - y0);//steep问题:swap来解决
            if(steep)
            {
                std::swap(x0, y0);
                std::swap(x1, y1);
            }
            if(x0 > x1)//x的方向问题:swap来解决
            {
                std::swap(x0, x1);
                std::swap(y0, y1);
            }
            float slope = std::abs(float(y1 - y0) / (x1 - x0));
            assert(0 <= slope&&slope <= 1);
            const float yIncrement = (y1 > y0) ? 1 : -1;//y的方向问题:swap来解决
            float err = 0.0f;
            for(int x = x0, y = y0; x <= x1; ++x)
            {
                if(steep)
                    setColor(y, x, color);
                else
                    setColor(x, y, color);
                assert(-0.5f <= err&&err <= 0.5f);
                err += slope;
                if(err > 0.5f)
                {
                    y += yIncrement;
                    err -= 1.0f;
                }
            }
        }
        void rasterizeTriangle(const VSOutput& v0, const VSOutput& v1, const VSOutput& v2)
        {
            TriangleEquation triEqn;
            this->_initTriangleEquation(v0, v1, v2, triEqn);
            if(triEqn.isDegenerate())
                return;//退化的三角形
            const Vec2i points[3] = { v0.position.xy(),v1.position.xy(),v2.position.xy() };
            if(m_context->rs.fillMode == FillMode::WIRE_FRAME)
            {
                this->rasterizeLine(points[0].x, points[0].y,points[1].x, points[1].y,Vec4::WHITE);
                this->rasterizeLine(points[1].x, points[1].y,points[2].x, points[2].y,Vec4::WHITE);
                this->rasterizeLine(points[2].x, points[2].y,points[0].x, points[0].y,Vec4::WHITE);
                return;
            }
            PixelTraverser traverser(triEqn,points);
			auto renderedCount = 0;
            // 使用Edge Test判断是否为inside
            while(traverser.traverse())
            {
                auto x = traverser.getPixelCoords().x, y = traverser.getPixelCoords().y;
                if(!traverser.isInsideTriangle())
                {
                    //Float* c = m_context->om.colorData + (y * m_context->om.colorBufferPitch + x * m_context->om.colorFloatCount);
                    //ColorValue::BLUE.copyTo(c, m_context->om.colorFloatCount);
                    continue;
                }
                PixelShader::SystemValue sv;
                {
                    sv.depth       = traverser.getNonlinearDepth();
                    sv.position    = {x + 0.5f,y + 0.5f,sv.depth ,1.0f };
                    sv.targetIndex = 0;
                }
                PSRegisters linearAttributes;
                for(int i = 0; i < lengthof(linearAttributes); ++i)
                    linearAttributes[i] = traverser.getAtrributes()[i] / traverser.getInverseLinearDepth();
                m_context->ps->execute(linearAttributes, sv);

                Float* depthData = m_context->om.depthData + (y * m_context->om.depthBufferPitch + x * m_context->om.depthFloatCount);
                Float* colorData = m_context->om.colorData + (y * m_context->om.colorBufferPitch + x * m_context->om.colorFloatCount);

                bool depthPassed = true;
                if(m_context->om.depthEnabled)
                    depthPassed = _doDepthTest(m_context->om.depthCmpFunc, sv.depth, *depthData);
                if(depthPassed)
                {
                    if(m_context->om.depthWriteEnabled)
                        *depthData = sv.depth;
                    const Blend* blend = nullptr;
                    {
                        //false: 所有的RT都用blends[0]; true: 对RTi使用其对应的blends[i]
                        if(m_context->om.independentBlendEnabled)
                            blend = &m_context->om.blends[sv.targetIndex];
                        else
                            blend = &m_context->om.blends[0];
                    }
                    if(blend && blend->enabled)
                    {
                        // SRC_blendfactor(Current) blendop DST_blendfactor(Backbuffer)
                        const auto& srcRGBA = sv.targets[sv.targetIndex];
                        const auto  dstRGBA = Vec4().set(colorData, m_context->om.colorFloatCount);

                        auto srcFactor = srcRGBA, dstFactor = dstRGBA;
                        {
                            srcFactor.rgb(_resolveBlendColor(blend->srcBlend, srcRGBA, dstRGBA, m_context->om.blendFactor));
                            dstFactor.rgb(_resolveBlendColor(blend->dstBlend, srcRGBA, dstRGBA, m_context->om.blendFactor));

                            srcFactor.a = _resolveBlendAlpha(blend->srcBlendAlpha, srcRGBA.a, dstRGBA.a, m_context->om.blendFactor[3]);
                            dstFactor.a = _resolveBlendAlpha(blend->dstBlendAlpha, srcRGBA.a, dstRGBA.a, m_context->om.blendFactor[3]);
                        }
                        auto color = Vec4::ZERO;
                        {
                            color.rgb(_blendOperationColor(blend->blendOp, srcRGBA.rgb()*srcFactor.rgb(), dstRGBA.rgb()*dstFactor.rgb()));
                            color.a = _blendOperationAlpha(blend->blendOpAlpha, srcRGBA.a*srcFactor.a, dstFactor.a*dstFactor.a);
                        }
                        if(blend->writeMask&ColorWriteEnable::ALL)
                            color.copyTo(colorData, m_context->om.colorFloatCount);
                    }
                    else
                        sv.targets[sv.targetIndex].copyTo(colorData, m_context->om.colorFloatCount);
                    ++renderedCount;
                }
            }
			auto ratio = Float(renderedCount) / traverser.getPixelCount();
        }
    private:
        static bool _doDepthTest(CmpFunc cmp,Float src ,Float dst)
        {
            switch(cmp)
            {
            case CmpFunc::NEVER:
                return false;
                break;
            case CmpFunc::LESS:
                return src < dst;
                break;
            case CmpFunc::EQUAL:
                return src == dst;
                break;
            case CmpFunc::LESS_EQUAL:
                return src <= dst;
                break;
            case CmpFunc::GREATER:
                return src > dst;
                break;
            case CmpFunc::NOT_EQUAL:
                return src != dst;
                break;
            case  CmpFunc::GREATER_EQUAL:
                return src >= dst;
                break;
            case CmpFunc::ALWAYS:
                return true;
                break;
            default:
                assert(false);
                break;
            }
            return false;
        }
        static Float _blendOperationAlpha(BlendOp op, Float src, Float dst)
        {
            auto alpha = Float(0);
            switch(op)
            {
            case BlendOp::ADD:
                alpha = src + dst;
                break;
            case BlendOp::SUBTRACT:
                alpha = dst - src;
                break;
            case BlendOp::REV_SUBTRACT:
                alpha = src - dst;
                break;
            case BlendOp::MIN:
                alpha = std::min(src, dst);
                break;
            case BlendOp::MAX:
                alpha = std::max(src, dst);
                break;
            }
            return alpha;
        }
        static Vec3 _blendOperationColor(BlendOp op, const Vec3& src, const Vec3& dst)
        {
            auto color = Vec3::ZERO;
            switch(op)
            {
            case BlendOp::ADD:
                color = src + dst;
                break;
            case BlendOp::SUBTRACT:
                color = dst - src;
                break;
            case BlendOp::REV_SUBTRACT:
                color = src - dst;
                break;
            case BlendOp::MIN:
                color.set( std::min(src.r, dst.r),
                           std::min(src.g, dst.g),
                           std::min(src.b, dst.b) );
                break;
            case BlendOp::MAX:
                color.set( std::max(src.r, dst.r),
                           std::max(src.g, dst.g),
                           std::max(src.b, dst.b));
                break;
            }
            return color;
        }
        static Vec3 _resolveBlendColor(BlendFactor blend, const Vec4& src, const Vec4& dst,const Float* factors)
        {
            auto result = Vec3::ZERO;
            switch(blend)
            {
            case BlendFactor::ZERO:
                result = Vec3::ZERO;
                break;
            case BlendFactor::ONE:
                result = Vec3::ONE;
                break;
            case BlendFactor::SRC_COLOR:
                result = src.rgb();
                break;
            case BlendFactor::INV_SRC_COLOR:
                result = Vec3::ONE - src.rgb();
                break;
            case BlendFactor::DEST_COLOR:
                result = dst.rgb();
                break;
            case BlendFactor::INV_DEST_COLOR:
                result = Vec3::ONE - dst.rgb();
                break;
            case BlendFactor::BLEND_FACTOR:
                result.set(factors, 3);
                break;
            case BlendFactor::INV_BLEND_FACTOR:
                result = Vec3::ONE - Vec3().set(factors, 3);
                break;
            case BlendFactor::SRC_ALPHA_SAT:
                result = Vec3(saturate(src.a),saturate(src.a),saturate(src.a));
                break;
            case BlendFactor::SRC_ALPHA:
                result = Vec3(src.a,src.a,src.a);
                break;
            case BlendFactor::INV_SRC_ALPHA:
                result = Vec3(1 - src.a, 1 - src.a, 1 - src.a);
                break;
            case BlendFactor::DEST_ALPHA:
                result = Vec3(dst.a, dst.a,dst.a);
                break;
            case BlendFactor::INV_DEST_ALPHA:
                result = Vec3(1 - dst.a, 1 - dst.a, 1 - dst.a);
                break;
            case BlendFactor::SRC1_COLOR:
            case BlendFactor::INV_SRC1_COLOR:
            case BlendFactor::SRC1_ALPHA:
            case BlendFactor::INV_SRC1_ALPHA:
                assert(false && "未实现dual source color blending");
                break;
            default:
                break;
            }
            return result;
        }
        static Float _resolveBlendAlpha(BlendFactor blend, Float src, Float dst,Float factor)
        {
            auto result = Float(0);
            switch(blend)
            {
            case BlendFactor::ZERO:
                result = Float(0);
                break;
            case BlendFactor::ONE:
                result = Float(1);
                break;
            case BlendFactor::BLEND_FACTOR:
                result = factor;
                break;
            case BlendFactor::INV_BLEND_FACTOR:
                result = Float(1) - factor;
                break;
            case BlendFactor::SRC_ALPHA_SAT:
                result = clamp(src, Float(0), Float(1));
                break;
            case BlendFactor::SRC_ALPHA:
                result = src;
                break;
            case BlendFactor::INV_SRC_ALPHA:
                result = 1 - src;
                break;
            case BlendFactor::DEST_ALPHA:
                result = dst;
                break;
            case BlendFactor::INV_DEST_ALPHA:
                result = 1 - dst;
                break;
            case BlendFactor::SRC_COLOR:
            case BlendFactor::INV_SRC_COLOR:
            case BlendFactor::DEST_COLOR:
            case BlendFactor::INV_DEST_COLOR:
                assert(false && "alpha值不能为color");
                break;
            case BlendFactor::SRC1_COLOR:
            case BlendFactor::INV_SRC1_COLOR:
            case BlendFactor::SRC1_ALPHA:
            case BlendFactor::INV_SRC1_ALPHA:
                assert(false && "未实现");
                break;
            default:
                break;
            }
            return result;
        }
        void _initTriangleEquation(const VSOutput& vs0, const VSOutput& vs1, const VSOutput& vs2,TriangleEquation& eqnOut)
        {
            const Vec2 v0(vs0.position), v1(vs1.position), v2(vs2.position);
            // 计算面积
            {
                const auto e01 = v1 - v0;
                const auto e02 = v2 - v0;
                eqnOut.m_area = (e01.x * e02.y - e01.y*e02.x)*0.5f;
            }
            if(eqnOut.isDegenerate())
                return;
            // 计算Edge Eqn
            {
                eqnOut.m_e01 = EdgeEquation(v0, v1);
                eqnOut.m_e12 = EdgeEquation(v1, v2);
                eqnOut.m_e20 = EdgeEquation(v2, v0);
            }
            // 计算属性Eqn
            {
                for(uint32_t i = 0; i < lengthof<PSRegisters>(); ++i)
                    eqnOut.m_attributeEqns[i] = PlaneEquation(v0,v1,v2,vs0.registers[i],vs1.registers[i],vs2.registers[i]);
            }
            // 计算Depth Eqn
            {
                eqnOut.m_depthEqn = PlaneEquation(v0,v1,v2,vs0.position.zw(),vs1.position.zw(),vs2.position.zw());
            }
        }
    private:
        std::unique_ptr<Clipper> m_clipper;
    };

}//ns rl
//Pipeline
namespace rl
{
    Pipeline::Pipeline()
    {
        m_rasterizer = new Rasterizer();
        m_vertexer = new Vertexer();
    }
    Pipeline::~Pipeline()
    {
        delete m_rasterizer;
        delete m_vertexer;
    }
	///////////////////////////////////////////////////////////////////////////
	void Pipeline::draw(const Context& ctx, uint32_t vertexCount, uint32_t vertexStart)
	{
        m_rasterizer->setContext(&ctx);
        m_vertexer->setContext(&ctx);
        switch(ctx.ia.topology)
        {
        case PrimitiveTopology::TRIANGLE_LIST:
            {
                uint32_t indices[3] = { vertexStart, vertexStart + 1, vertexStart + 2 };
                auto triCount = vertexCount / 3;
                while(triCount--)
                {
                    auto entry0 = m_vertexer->fetch(indices[0]);
                    auto entry1 = m_vertexer->fetch(indices[1]);
                    auto entry2 = m_vertexer->fetch(indices[2]);
                    m_rasterizer->scheduleTriangle(entry0->vertex, entry1->vertex, entry2->vertex);
                    indices[0] += 3; indices[1] += 3; indices[2] += 3;
                }
            }
        case PrimitiveTopology::TRIANGLE_STRIP:
            {//0,1,2,3,4,5.. ==>(0,1,2),(1,3,2); (2,3,4), (2,5,4); ....
                assert(vertexCount >= 3);
                uint32_t indices[3] ={ vertexStart, vertexStart + 1, vertexStart + 2 };
                auto triCount = vertexCount - 2;
                auto flip = true;
                while(triCount--)
                {
                    auto entry0 = m_vertexer->fetch(indices[0]);
                    auto entry1 = m_vertexer->fetch(indices[1]);
                    auto entry2 = m_vertexer->fetch(indices[2]);
                    m_rasterizer->scheduleTriangle(entry0->vertex, entry1->vertex, entry2->vertex);
                    if(flip)
                        indices[0] = indices[1], indices[1] = indices[2] + 1;
                    else
                        indices[0] = indices[2], indices[1] = indices[0] + 1, indices[2] = indices[0] + 2;
                    flip = !flip;
                }
            }
            break;
        case PrimitiveTopology::TRIANGLE_FAN:
            {//0,1,2,3,4,5.. ==>(0,1,2),(0,2,3), (0,3,4),(0,4,5) ....

            }
            break;
        case PrimitiveTopology::LINE_LIST:
            {
                uint32_t indices[2] = { vertexStart, vertexStart + 1};
                auto lineCount = vertexCount / 2;
                while(lineCount--)
                {
                    auto entry0 = m_vertexer->fetch(indices[0]);
                    auto entry1 = m_vertexer->fetch(indices[1]);
                    m_rasterizer->scheduleLine(entry0->vertex, entry1->vertex);
                    indices[0] += 2; indices[1] += 2;
                }
            }
            break;
        case PrimitiveTopology::LINE_STRIP:
            break;
        }
        m_rasterizer->setContext(nullptr);
        m_vertexer->setContext(nullptr);
    }
    void Pipeline::drawIndexed(const Context& ctx, uint32_t indexCount, uint32_t indexStart, int32_t baseVertexIndex)
    {
        m_rasterizer->setContext(&ctx);
        m_vertexer->setContext(&ctx);
        auto ibuffer = ctx.ia.ibuffer;
        assert(ibuffer);
        switch(ctx.ia.topology)
        {
        case PrimitiveTopology::TRIANGLE_LIST:
            {
                uint32_t indices[3] = { indexStart + 0, indexStart + 1,indexStart + 2 };
                auto triCount = indexCount / 3;
                while(triCount--)
                {
                    auto entry0 = m_vertexer->fetch(baseVertexIndex + ibuffer->getVertexIndex(indices[0]));
                    auto entry1 = m_vertexer->fetch(baseVertexIndex + ibuffer->getVertexIndex(indices[1]));
                    auto entry2 = m_vertexer->fetch(baseVertexIndex + ibuffer->getVertexIndex(indices[2]));
                    m_rasterizer->scheduleTriangle(entry0->vertex, entry1->vertex, entry2->vertex);

                    indices[0] += 3; indices[1] += 3; indices[2] += 3;
                }
            }
            break;
        }
        m_rasterizer->setContext(nullptr);
        m_vertexer->setContext(nullptr);
    }
}