#ifndef RASLITE_PIPELINE_H
#define RASLITE_PIPELINE_H
#include "RasliteShader.h"
namespace rl 
{
    /////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////
    struct VertexStream
    {
        const VertexBuffer* vbuffer = nullptr;
        uint32_t	        offset  = 0;
        uint32_t	        stride  = 0;
    };
	/////////////////////////////////////////////////////////////////
	//InputLayout
	/////////////////////////////////////////////////////////////////
	struct InputElement
	{
		uint8_t      streami;
		Format format;
		uint8_t      registeri;
	};
	class InputLayout
	{
	public:
		InputLayout(const InputElement* elements, uint32_t count);
	   ~InputLayout();

		uint32_t getElementNum() const;
		uint32_t getHighestStreamIndex() const;

		const InputElement* getElements() const;
		const InputElement& getElement(uint32_t idx) const;

        const VSRegisterTypes& getRegisterTypes() const;
	private:
		std::unique_ptr<InputElement[]> m_elements;
		uint32_t m_nElement = 0;
		uint32_t m_highestStreamIndex = 0;
        VSRegisterTypes m_registerTypes;
	};
	/////////////////////////////////////////////////////////////////
	//Context
	/////////////////////////////////////////////////////////////////
    struct Viewport
    {
        Float topLeftX;
        Float topLeftY;
        Float width;
        Float height;
        Float minDepth;
        Float maxDepth;
    };
	struct Context 
	{
		// Input Assembler Stage
		struct IA
		{
			const IndexBuffer* ibuffer = nullptr;
			VertexStream       vstreams[VERTEX_STREAM_COUNT];
			PrimitiveTopology  topology = PrimitiveTopology::TRIANGLE_LIST;
			const InputLayout* layout = nullptr;
		};
		// Output Merger Stage
		struct OM
		{
            //Render Targets;
            uint8_t  renderTargetCount = 0;
            Surface* renderTargets[SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
            Surface* depthStencil =  nullptr;
			//BlendFactor State
            bool     alphaToCoverageEnabled = false;
            bool     independentBlendEnabled= false; //false: 所有的RT都用blends[0]; true: 对RTi使用其对应的blends[i]
            Blend    blends[8];
            Float    blendFactor[4] = {1,1,1,1};
            uint32_t sampleMask = 0xFFFFFFFF;
			//DepthStencil State
            bool    depthEnabled = true;
            bool    depthWriteEnabled = true;
            CmpFunc depthCmpFunc = CmpFunc::LESS;
            bool    stencilEnabled = false;
            uint8_t stencilReadMask = 0xFF;
            uint8_t stencilWriteMask= 0xFF;
            DepthStencilOpDesc frontFace;
            DepthStencilOpDesc backFace;
            uint32_t stencilRef;

            ///////////////////////////////
			// 内部数据
            Float*   colorData = nullptr;
            uint32_t colorFloatCount;
            uint32_t colorBufferPitch;

            Float*   depthData = nullptr;
            uint32_t depthBufferPitch;
            uint32_t depthFloatCount;

            Mat4     viewportTransform = Mat4::IDENTITY;
		};
		// Rasterization Stage
		struct RS
		{
			//Scissor Rects
            const Rect* scissorRects;
            uint8_t     scissorRectCount;
			//Viewports
            const Viewport* viewports;
            uint8_t         viewportCount;
			//State
            FillMode fillMode           = FillMode::SOLID;
            CullMode cullMode           = CullMode::BACK;
            bool  frontCounterClockwise = false; // 是否顶点顺序为逆时针为front?
            int   depthBias             = 0;
            Float depthBiasClamp        = 0.0f;
            Float slopeScaledDepthBias  = 0.0f;
            bool depthClipEnabled       = true;
            bool scissorEnabled         = false;
            bool multisampleEnabled     = false;
            bool antialiasedLineEnabled = false;
		};
		IA  ia;
		RS  rs;
		OM  om;

		VertexShader* vs = nullptr;
		PixelShader*  ps = nullptr;
		//todo: GS HS DS CS
	};
	/////////////////////////////////////////////////////////////////
	//Pipeline
	/////////////////////////////////////////////////////////////////
	class Rasterizer;
	class OutputMerger;
	class Vertexer;
	class Pipeline
	{
	public:
         Pipeline();
        ~Pipeline();
		void draw(const Context& ctx,uint32_t vertexCount,uint32_t vertexStart);
        void drawIndexed(const Context& ctx, uint32_t indexCount, uint32_t indexStart, int32_t baseVertexIndex);
        
	private:
        Rasterizer* m_rasterizer;
        Vertexer*   m_vertexer;
	};
}//ns rl
///////////////////////////////////////////////////////////////////
//   inline
///////////////////////////////////////////////////////////////////
namespace rl
{
	inline uint32_t InputLayout::getElementNum() const
	{
		return m_nElement;
	}

	inline uint32_t InputLayout::getHighestStreamIndex() const
	{
		return m_highestStreamIndex;
	}

	inline const InputElement* InputLayout::getElements() const
	{
		return m_elements.get();
	}
	inline const InputElement& InputLayout::getElement(uint32_t idx) const
	{
		assert(idx < m_nElement);
		return m_elements[idx];
	}
    inline const VSRegisterTypes& InputLayout::getRegisterTypes() const
    {
        return m_registerTypes;
    }
}


#endif //RASLITE_PIPELINE_H