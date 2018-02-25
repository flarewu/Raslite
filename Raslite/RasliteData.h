#ifndef RASLITE_DATA_H
#define RASLITE_DATA_H

#include "RasliteCommon.h"
#include <limits>
#include <memory>
#include <cassert>
namespace rl {
    using ColorValue =  Vec4;
	///////////////////////////////////////////////////////////
	//IndexBuffer
	///////////////////////////////////////////////////////////
	class IndexBuffer
	{
	public:
		IndexBuffer(uint32_t indexCount, Format fmt);
		virtual ~IndexBuffer();
	public:
        template <typename T>
		T* getData(uint32_t offsetInCount = 0) const;
		uint32_t  getLength() const;
		Format    getFormat() const;
		uint32_t  getVertexIndex(uint32_t idx) const;
	private:
		uint32_t m_length = 0;
		Format	 m_format;
		std::unique_ptr<uint8_t[]> m_data;
	};
	///////////////////////////////////////////////////////////
	//VertexBuffer
	///////////////////////////////////////////////////////////
	class VertexBuffer
	{
	public:
		VertexBuffer(uint32_t lengthInBytes);
		~VertexBuffer();
	public:
        template <typename T>
		T* getData(uint32_t offsetInCount = 0) const;
		// in bytes
		uint32_t getLength() const;
	private:
		// in bytes
		uint32_t m_length = 0;
		std::unique_ptr<uint8_t[]> m_data;
	};
	///////////////////////////////////////////////////////////
	// Surface
	///////////////////////////////////////////////////////////
	class Surface
	{
	public:
		Surface(uint32_t width, uint32_t height, Format fmt);
	   ~Surface();

		const ColorValue samplePoint(Float u, Float v) const;
		const ColorValue sampleLinear(Float u, Float v) const;
		void clear(const ColorValue& val, const Rect* rect = nullptr);
		void copyTo(const Rect *srcRect, Surface *dstSurface, const Rect *destRect, FilterType filterType);

		Float* lock(const Rect *rect = nullptr);
		void   unlock();

		Format   getFormat()           const;
		uint32_t getFormatFloatCount() const;
        uint32_t getFormatByteCount()  const;
		uint32_t getWidth()            const;
		uint32_t getHeight()           const;

        ColorValue getElement(uint32_t x, uint32_t y) const;
        ColorValue getElement(uint32_t index)         const;

		const Rect getRect() const;
	private:

		Format	 m_format;
		uint32_t m_width;
		uint32_t m_height;

		bool	m_allLocked;
		Rect	m_lockedRect;
		std::unique_ptr<Float[]> m_lockedData;
		// m_width * m_height个element
		std::unique_ptr<Float[]> m_data;
	};
    ///////////////////////////////////////////////////////////
    // Texture
    ///////////////////////////////////////////////////////////
    class Texture
    {
    public:
        explicit Texture() {};
        virtual ~Texture() {};

    public:
        virtual ColorValue sample(Float u, Float v, Float w, const Vec4 *xGradient, const Vec4 *yGradient, const uint32_t* samplerStates) = 0;
    };
    ///////////////////////////////////////////////////////////
    // Texture2D
    ///////////////////////////////////////////////////////////
    //todo: Texture1D, Texture3D
    class Texture2D: public Texture
    {
    public:
        struct Desc
        {
            uint32_t    width;
            uint32_t    height;
            uint32_t    mipLevels = 0; //1: 用于multisampled texture; 0: 会自动产生所有的mipmaps
            uint32_t    arraySize = 1;
            Format      format;
            uint32_t    sampleCount;
            uint32_t    sampleQuality;

            const void* mem = nullptr;
            uint32_t    memPitch;
            uint32_t    memSlicePitch;

            //D3D11_USAGE  Usage;
            //uint32_t     BindFlags;
            //uint32_t     CPUAccessFlags;
            //uint32_t     MiscFlags;
        };
    public:
        Texture2D(const Desc& desc);
        virtual ~Texture2D();
        virtual ColorValue sample(Float u, Float v, Float w, const Vec4 *xGradient, const Vec4 *yGradient, const uint32_t* samplerStates) override;
    public:
        // 以baseLevel为基准开始产生所有的mip
        void  generateMips(uint32_t baseLevel = 0);
        void  clear(uint32_t mipLevel, const ColorValue& colorVal, const Rect* rect = nullptr);
        void* lock(uint32_t mipLevel, const Rect* rect = nullptr);
        void  unlock(uint32_t mipLevel);

        Surface* getMipSurface(uint32_t mipLevel)     const;
        uint32_t getMipLevel  ()                      const;
        Format   getFormat    ()                      const;
        uint32_t getWidth     (uint32_t mipLevel = 0) const;
        uint32_t getHeight    (uint32_t mipLevel = 0) const;
    private:
        // mip level的总个数: [0, m_mipLevel);
        uint32_t  m_mipLevel; //0: 表示全部mipmap; 1: 表示可以multisample
        uint32_t  m_widthSq, m_heightSq;
        Surface** m_surfaces = nullptr;
    };
}//ns rl
 // inline 
namespace rl 
{
	///////////////////////////////////////////////////////////////////////////////
	// IndexBuffer 
	///////////////////////////////////////////////////////////////////////////////
    template <typename T>
	inline T* IndexBuffer::getData(uint32_t offsetInCount) const
	{
		assert(offsetInCount * sizeof(T) < m_length);
        static_assert(std::is_unsigned_v<T> && (sizeof(T) == 2 || sizeof(T) == 4));
		return reinterpret_cast<T*>(m_data.get()) + offsetInCount;
	}
	inline uint32_t IndexBuffer::getLength() const
	{
		return m_length;
	}
	inline Format IndexBuffer::getFormat() const
	{
		return m_format;
	}
	///////////////////////////////////////////////////////////////////////////////
	// VertexBuffer
	///////////////////////////////////////////////////////////////////////////////
	inline uint32_t VertexBuffer::getLength() const
	{
		return m_length;
	}
    template <typename T>
	inline T* VertexBuffer::getData(uint32_t offsetInCount) const
	{
		assert(offsetInCount * sizeof(T) < m_length);
		return reinterpret_cast<T*>(m_data.get()) + offsetInCount;
	}
	////////////////////////////////////////////////////////////////////////
	// Surface
	////////////////////////////////////////////////////////////////////////
	inline const Rect Surface::getRect() const
	{
		return Rect(0, 0, m_width, m_height);
	}
	inline Format Surface::getFormat() const
	{
		return m_format;
	}

	inline uint32_t Surface::getWidth() const
	{
		return m_width;
	}

	inline uint32_t Surface::getHeight() const
	{
		return m_height;
	}
	inline uint32_t Surface::getFormatFloatCount() const
	{
		return float_count(m_format);
	}
    inline uint32_t Surface::getFormatByteCount()  const
    {
        return this->getFormatFloatCount() * sizeof(Float);
    }
    ////////////////////////////////////////////////////////////////////////
	// Surface
	////////////////////////////////////////////////////////////////////////
    inline uint32_t Texture2D::getMipLevel() const
    {
        return m_mipLevel;
    }
}//ns rl


#endif //RL_DATA_FLOW_H