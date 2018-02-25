#include "RasliteData.h"
namespace rl {
    IndexBuffer::IndexBuffer(uint32_t indexCount, Format fmt)
        : m_length(indexCount*byte_count(fmt))
        , m_format(fmt)
        , m_data(std::make_unique<uint8_t[]>(m_length))
    {
        assert(m_length > 0);
    }
    IndexBuffer::~IndexBuffer()
    {
    }
    uint32_t IndexBuffer::getVertexIndex(uint32_t idx) const
    {
        switch(m_format)
        {
            case Format::INDEX16:
            {
                assert(idx < (m_length >> 1));
                auto data = reinterpret_cast<const uint16_t*>(m_data.get());
                return data[idx];
            }
            case Format::INDEX32:
            {
                assert(idx < (m_length >> 2));
                auto data = reinterpret_cast<const uint32_t*>(m_data.get());
                return data[idx];
            }
            default:
                return std::numeric_limits<uint32_t>::max();
        }
    }
    ///////////////////////////////////////////////////////////
    //
    ///////////////////////////////////////////////////////////
    VertexBuffer::VertexBuffer(uint32_t lengthInBytes)
        : m_length(lengthInBytes)
        , m_data(std::make_unique<uint8_t[]>(m_length))
    {
        assert(m_length > 0);
    }
    VertexBuffer::~VertexBuffer()
    {
    }
    ///////////////////////////////////////////////////////////
    //Surface
    ///////////////////////////////////////////////////////////
    namespace detail {
        template <typename T>
        static inline void assign(Float* data, uint32_t width, const Rect& rect, const T& val)
        {
            auto bridge = width - rect.getWidth();
            auto ptr    = &reinterpret_cast<T*>(data)[rect.top * width + rect.left];
            for(auto y = rect.top; y < rect.bottom; ++y, ptr += bridge)
            {
                for(auto x = rect.left; x < rect.right; ++x, ++ptr)
                    *ptr = val;
            }
        }
        template <typename T>
        static inline const T& pointPixel(Float* data, uint32_t index)
        {
            return reinterpret_cast<T*>(data)[index];
        }
        template <typename T>
        static inline const T& pointPixel(Float* data, uint32_t xPixel, uint32_t yPixel, uint32_t width)
        {
            return pointPixel<T>(data, yPixel * width + xPixel);
        }
        template <typename T>
        static inline const T lerpPixel(Float* data, uint32_t width,
                                        uint32_t x1Pixel, uint32_t y1Pixel,
                                        uint32_t x2Pixel, uint32_t y2Pixel,
                                        Float factor)
        {
            auto& p1 = pointPixel<T>(data, x1Pixel, y1Pixel, width);
            auto& p2 = pointPixel<T>(data, x2Pixel, y2Pixel, width);
            return lerp(p1, p2, factor);
        }
        template <typename T>
        static inline const T bilerpPixel(Float* data, uint32_t width,
                                          uint32_t x1Pixel, uint32_t y1Pixel,
                                          uint32_t x2Pixel, uint32_t y2Pixel,
                                          Float factor1, Float factor2)
        {
            auto row1 = lerpPixel<T>(data, width, x1Pixel, y1Pixel, x2Pixel, y1Pixel, factor1);
            auto row2 = lerpPixel<T>(data, width, x1Pixel, y2Pixel, x2Pixel, y2Pixel, factor1);
            return lerp(row1, row2, factor2);
        }
    }//ns detail
    Surface::Surface(uint32_t width, uint32_t height, Format fmt)
        : m_width(width)
        , m_height(height)
        , m_allLocked(false)
        , m_lockedData(0)
        , m_format(fmt)
        , m_data(std::make_unique<Float[]>(m_width * m_height * float_count(m_format)))
    {
        assert(width > 0 && height > 0);
    }
    Surface::~Surface()
    {
    }

    void Surface::clear(const ColorValue& colorVal, const Rect *rect/*= nullptr*/)
    {
        Rect clearingRect(0, 0, m_width, m_height);
        {
            assert(!(rect && rect->isNormal()));
            if(rect)
                clearingRect = *rect;
            assert(this->getRect().contains(clearingRect));
        }
        auto data = this->lock();
        switch(m_format)
        {
        case Format::R32_FLOAT:
            detail::assign(data, m_width, clearingRect, Float(colorVal.r));
            break;
        case Format::R32G32_FLOAT:
            detail::assign(data, m_width, clearingRect, Vec2(colorVal));
            break;
        case Format::R32G32B32_FLOAT:
            detail::assign(data, m_width, clearingRect, Vec3(colorVal));
            break;
        case Format::R32G32B32A32_FLOAT:
            detail::assign(data, m_width, clearingRect, colorVal);
            break;
        default:
            assert(false && "不支持!");
        }
        this->unlock();
    }
    Float* Surface::lock(const Rect *rect /*= nullptr*/)
    {
        assert(!m_allLocked && !m_lockedData && "已经Locked!");
        if(!rect)
        {
            m_allLocked = true;
            return m_data.get();
        }
        assert(rect->isNormal() && "非法Rect!");
        assert(this->getRect().contains(*rect) && "超出范围!");

        m_lockedRect = *rect;

        const auto w = m_lockedRect.getWidth(),
                   h = m_lockedRect.getHeight(),
              nFloat = this->getFormatFloatCount();

        m_lockedData = std::make_unique<Float[]>(w * h * nFloat);

        auto dst = m_lockedData.get();
        for(auto y = m_lockedRect.top; y < m_lockedRect.bottom; ++y)
        {
            auto src = &m_data[(y * m_width + m_lockedRect.left) * nFloat];
            std::memcpy(dst, src, sizeof(Float) * nFloat * w);
            dst += nFloat * w;
        }
        return m_lockedData.get();
    }
    void Surface::unlock()
    {
        assert(m_allLocked || m_lockedData);
        if(m_allLocked)
        {
            m_allLocked = false;
            return;
        }
        const auto w = m_lockedRect.getWidth(),
              nFloat = this->getFormatFloatCount();
        auto src = m_lockedData.get();
        for(auto y = m_lockedRect.top; y < m_lockedRect.bottom; ++y)
        {
            auto dst = &m_data[(y * m_width + m_lockedRect.left) * nFloat];
            std::memcpy(dst, src, sizeof(Float) * nFloat * w);
            src += nFloat * w;
        }
        m_lockedData = nullptr;
    }
    ColorValue Surface::getElement(uint32_t x, uint32_t y) const
    {
        switch(m_format)
        {
        case Format::R32_FLOAT:
            {
                auto& p = detail::pointPixel<Float>(m_data.get(), x, y, m_width);
                return ColorValue(p, 0, 0, 1);
            }
            break;
        case Format::R32G32_FLOAT:
            {
                auto& p = detail::pointPixel<Vec2>(m_data.get(), x, y, m_width);
                return ColorValue(p.x, p.y, 0, 1);
            }
            break;
        case Format::R32G32B32_FLOAT:
            {
                auto& p = detail::pointPixel<Vec3>(m_data.get(), x, y, m_width);
                return ColorValue(p.x, p.y, p.z, 1);
            }
            break;
        case Format::R32G32B32A32_FLOAT:
            {
                return detail::pointPixel<ColorValue>(m_data.get(), x, y, m_width);
            }
            break;
        default:
            assert(false);
            break;
        }
        return ColorValue::BLACK;
    }
    ColorValue Surface::getElement(uint32_t index)  const
    {
        switch(m_format)
        {
        case Format::R32_FLOAT:
            {
                auto& p = detail::pointPixel<Float>(m_data.get(),index);
                return ColorValue(p, 0, 0, 1);
            }
            break;
        case Format::R32G32_FLOAT:
            {
                auto& p = detail::pointPixel<Vec2>(m_data.get(),index);
                return ColorValue(p.x, p.y, 0, 1);
            }
            break;
        case Format::R32G32B32_FLOAT:
            {
                auto& p = detail::pointPixel<Vec3>(m_data.get(),index);
                return ColorValue(p.x, p.y, p.z, 1);
            }
            break;
        case Format::R32G32B32A32_FLOAT:
            {
                return detail::pointPixel<ColorValue>(m_data.get(),index);
            }
            break;
        default:
            assert(false);
            break;
        }
        return ColorValue::BLACK;
    }
    const ColorValue Surface::samplePoint(Float u, Float v) const
    {
        //取pixel时: Ceil(u*width - 1)更合理一些？
        const auto xPixel = floorui(u * (m_width - 1)),
                   yPixel = floorui(v * (m_height - 1));
        return this->getElement(xPixel, yPixel);

    }
    const ColorValue Surface::sampleLinear(Float u, Float v) const
    {
        const auto x = u * (m_width - 1), y = v * (m_height - 1);
        const auto x1Pixel = floorui(x), y1Pixel = floorui(y);

        uint32_t x2Pixel = x1Pixel + 1, y2Pixel = y1Pixel + 1;
        if(x2Pixel >= m_width)
            x2Pixel = m_width - 1;
        if(y2Pixel >= m_height)
            y2Pixel = m_height - 1;

        const Float factors[2] = { x - x1Pixel, y - y1Pixel };

        switch(m_format)
        {
            case Format::R32_FLOAT:
            {
                auto p = detail::bilerpPixel<Float>(m_data.get(), m_width, x1Pixel, y1Pixel, x2Pixel, y2Pixel, factors[0], factors[1]);
                return ColorValue(p, 0, 0, 1);
            }
            break;
            case Format::R32G32_FLOAT:
            {
                auto p = detail::bilerpPixel<Vec2>(m_data.get(), m_width, x1Pixel, y1Pixel, x2Pixel, y2Pixel, factors[0], factors[1]);
                return ColorValue(p.x, p.y, 0, 1);

            }
            break;
            case Format::R32G32B32_FLOAT:
            {
                auto p = detail::bilerpPixel<Vec3>(m_data.get(), m_width, x1Pixel, y1Pixel, x2Pixel, y2Pixel, factors[0], factors[1]);
                return ColorValue(p.x, p.y, p.z, 1);
            }
            break;
            case Format::R32G32B32A32_FLOAT:
            {
                return detail::bilerpPixel<Vec4>(m_data.get(), m_width, x1Pixel, y1Pixel, x2Pixel, y2Pixel, factors[0], factors[1]);
            }
            break;
            default:
                assert(false);
                break;
        }
        return ColorValue::BLACK;
    }
    void Surface::copyTo(const Rect *srcRect, Surface *dstSurface, const Rect *destRect, FilterType filterType)
    {
        assert(dstSurface);
        assert(filterType == FilterType::POINT && filterType == FilterType::LINEAR);

        Rect sRect(0, 0, m_width, m_height);
        {
            assert(!(srcRect&&srcRect->isNormal()));
            if(srcRect)
                sRect = *srcRect;
            assert(this->getRect().contains(sRect));
        }
        Rect dRest(0, 0, dstSurface->getWidth(), dstSurface->getHeight());
        {
            assert(!(destRect&&destRect->isNormal()));
            if(destRect)
                dRest = *destRect;
            assert(dstSurface->getRect().contains(dRest));
        }
        auto pDestData = dstSurface->lock(destRect);
        const auto iDestFloats = dstSurface->getFormatFloatCount(),
            iDestWidth = dRest.getWidth(),
            iDestHeight = dRest.getHeight();

        if(    !srcRect
            && !destRect
            && iDestFloats == this->getFormatFloatCount()
            && iDestWidth == m_width
            && iDestHeight == m_height)
        {
            std::memcpy(pDestData, m_data.get(), sizeof(Float) * iDestFloats * iDestWidth * iDestHeight);
            dstSurface->unlock();
            return;
        }

        const Float ustep = 1.0f / (m_width - 1);
        const Float vstep = 1.0f / (m_height - 1);

        Float vsrc = sRect.top * vstep;
        for(uint32_t y = 0; y < iDestHeight; ++y, vsrc += vstep)
        {
            Float usrc = sRect.left * ustep;
            for(uint32_t x = 0; x < iDestWidth; ++x, usrc += ustep, pDestData += iDestFloats)
            {
                auto vSrcColor =
                    filterType == FilterType::LINEAR ? this->sampleLinear(usrc, vsrc) : this->samplePoint(usrc, vsrc);
                vSrcColor.copyTo(pDestData, iDestFloats);
            }
        }
        dstSurface->unlock();
    }
    //
    //
    //
    namespace detail
    {
        template <typename T>
        static void filterTexel(const Float* src, uint32_t width, uint32_t height, Float* dstOut)
        {
            auto srcTexel = reinterpret_cast<const T*>(src);
            auto dstTexel = reinterpret_cast<T*>(dstOut);
            for(uint32_t y = 0; y < height; y += 2)
            {
                const uint32_t rows[2] = { y * width, (y + 1) * width };
                for(uint32_t x = 0; x < width; x += 2, ++dstTexel)
                {
                    *dstTexel =
                        (
                            srcTexel[rows[0] + x] +
                            srcTexel[rows[0] + x + 1] +
                            srcTexel[rows[1] + x] +
                            srcTexel[rows[1] + x + 1]
                         ) * Float(0.25);
                }
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////
    //
    ///////////////////////////////////////////////////////////////////////////////////
    Texture2D::Texture2D(const Desc& desc)
        : m_mipLevel(0)
        , m_surfaces(nullptr)
        , m_widthSq(desc.width * desc.width)
        , m_heightSq(desc.height* desc.height)
    {
        assert(desc.width > 0 && desc.height > 0);
        assert(Format::R32_FLOAT <= desc.format && desc.format <= Format::R32G32B32A32_FLOAT);

        auto mipLevels = desc.mipLevels;
        if(desc.mipLevels == 0)
        {// 如果等于0,那么就自动产生所有的mipmap
            auto w = desc.width, h = desc.height;
            do
            {
                ++mipLevels;
                w >>= 1; h >>= 1;
            }
            while(w && h);
        }
        m_surfaces = new Surface*[mipLevels];


        std::memset(m_surfaces, 0, sizeof(Surface*) * mipLevels);
        {
            auto mip = m_surfaces;
            auto w = desc.width, h = desc.height;
            do
            {
                *mip = new Surface(w, h, desc.format);
                ++m_mipLevel; ++mip;
                if(--mipLevels == 0)
                    break;
                w >>= 1; h >>= 1;
            } while(w && h);
        }
        if(desc.mem)
        {
            auto dst = reinterpret_cast<uint8_t*>(m_surfaces[0]->lock());
            const auto byteWidth = m_surfaces[0]->getFormatByteCount() * desc.width;
            for(size_t h = 0; h < desc.height; ++h)
                std::memcpy(dst + h * byteWidth, reinterpret_cast<const uint8_t*>(desc.mem) + h * desc.memPitch, byteWidth);
            m_surfaces[0]->unlock();
            this->generateMips();
        }
    }
    Texture2D::~Texture2D()
    {
        for(size_t i = 0; i < m_mipLevel; ++i)
            delete (m_surfaces[i]);
        delete[] m_surfaces;
    }
    void Texture2D::clear(uint32_t mipLevel, const Vec4& clr, const Rect *rect)
    {
        assert(mipLevel < m_mipLevel);
        assert(m_surfaces[mipLevel]);
        m_surfaces[mipLevel]->clear(clr, rect);
    }
    void Texture2D::generateMips(uint32_t baseLevel)
    {
        assert(baseLevel + 1 < m_mipLevel);
        const auto fmt = this->getFormat();
        for(uint32_t lvl = baseLevel + 1; lvl < m_mipLevel; ++lvl)
        {
            auto src = reinterpret_cast<const Float*>(this->lock(lvl - 1));
            auto dst = reinterpret_cast<      Float*>(this->lock(lvl));

            const auto height = this->getHeight(lvl - 1);
            const auto width  = this->getWidth (lvl - 1);
            switch(fmt)
            {
            case Format::R32_FLOAT:
                detail::filterTexel<Float>(src, width, height, dst);
                break;

            case Format::R32G32_FLOAT:
                detail::filterTexel<Vec2>(src, width, height, dst);
                break;

            case Format::R32G32B32_FLOAT:
                detail::filterTexel<Vec3>(src, width, height, dst);
                break;

            case Format::R32G32B32A32_FLOAT:
                detail::filterTexel<Vec4>(src, width, height, dst);
                break;
            default:
                assert(false);
                break;
            }
            this->unlock(lvl);
            this->unlock(lvl - 1);
        }
    }
    void* Texture2D::lock(uint32_t mipLevel, const Rect *rect)
    {
        assert(mipLevel < m_mipLevel);
        assert(m_surfaces[mipLevel]);
        return m_surfaces[mipLevel]->lock(rect);
    }
    void Texture2D::unlock(uint32_t mipLevel)
    {
        assert(mipLevel < m_mipLevel);
        assert(m_surfaces[mipLevel]);
        m_surfaces[mipLevel]->unlock();
    }
    Surface* Texture2D::getMipSurface(uint32_t mipLevel) const
    {
        assert(mipLevel < m_mipLevel);
        assert(m_surfaces[mipLevel]);
        return m_surfaces[mipLevel];
    }
    ColorValue Texture2D::sample(Float u, Float v, Float w/*未使用*/, const Vec4 *xGradient, const Vec4 *yGradient, const uint32_t* samplerStates)
    {
        return ColorValue::BLACK;
    }
    Format Texture2D::getFormat() const
    {
        return m_surfaces[0]->getFormat();
    }
    uint32_t Texture2D::getWidth(uint32_t mipLevel) const
    {
        assert(mipLevel < m_mipLevel);
        return m_surfaces[mipLevel]->getWidth();
    }
    uint32_t Texture2D::getHeight(uint32_t mipLevel) const
    {
        assert(mipLevel < m_mipLevel);
        return m_surfaces[mipLevel]->getHeight();
    }
}//ns rb
