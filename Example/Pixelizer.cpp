#include "Pixelizer.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>


Pixelizer::Pixelizer(SDL_Surface* surface)
    : m_surface(surface)
    , m_locked(false)
{
    assert(m_surface);
    m_mustLock = SDL_MUSTLOCK(surface);
}
uint32_t Pixelizer::getPixel(int x, int y)
{
    assert(!m_mustLock || m_locked);
    int bpp = m_surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)m_surface->pixels + y * m_surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(uint32_t *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}
void Pixelizer::putPixel(int x, int y, uint32_t pixel)
{
    assert(!m_mustLock || m_locked);
    int bpp = m_surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)m_surface->pixels + y * m_surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xFF;
            p[1] = (pixel >> 8)  & 0xFF;
            p[2] = pixel & 0xFF;
        }
        else
        {
            p[0] = pixel & 0xFF;
            p[1] = (pixel >> 8) & 0xFF;
            p[2] = (pixel >> 16) & 0xFF;
        }
        break;

    case 4:
        *(uint32_t *)p = pixel;
        break;
    }
}
void Pixelizer::putPixels(const ScreenCoordinates& coords, uint32_t color)
{
    this->lock();
    int bpp = m_surface->format->BytesPerPixel;
    switch(bpp) {
    case 1:
        {
            for(auto& coord : coords)
            {
                assert(0 <= coord.y&&coord.y < m_surface->h);
                assert(0 <= coord.x&&coord.x < m_surface->w);
                Uint8* p = (Uint8*)m_surface->pixels + coord.y * m_surface->pitch + coord.x * bpp;
                *p = color;
            }

        }
        break;

    case 2:
        {
            for(auto& coord : coords)
            {
                assert(0 <= coord.y&&coord.y < m_surface->h);
                assert(0 <= coord.x&&coord.x < m_surface->w);
                Uint8* p = (Uint8*)m_surface->pixels + coord.y * m_surface->pitch + coord.x * bpp;
                *(Uint16*)p = color;
            }

        }
        break;

    case 3:
        {
            for(auto& coord : coords)
            {
                assert(0 <= coord.y&&coord.y < m_surface->h);
                assert(0 <= coord.x&&coord.x < m_surface->w);
                Uint8* p = (Uint8 *)m_surface->pixels + coord.y * m_surface->pitch + coord.x * bpp;
                if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                {
                    p[0] = (color >> 16) & 0xFF;
                    p[1] = (color >> 8) & 0xFF;
                    p[2] = color & 0xFF;
                }
                else
                {
                    p[0] = color & 0xFF;
                    p[1] = (color >> 8) & 0xFF;
                    p[2] = (color >> 16) & 0xFF;
                }
            }

        }
        break;
    case 4:
        {
            for(auto& coord : coords)
            {
                assert(0 <= coord.y&&coord.y < m_surface->h);
                assert(0 <= coord.x&&coord.x < m_surface->w);
                Uint8 *p = (Uint8*)m_surface->pixels + coord.y * m_surface->pitch + coord.x * bpp;
                *(uint32_t*)p = color;
            }
        }
        break;
    }
    this->unlock();
}
void Pixelizer::lock()
{
    if(!m_mustLock)
        return;
    if(SDL_LockSurface(m_surface) < 0)
    {
        std::fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        std::exit(1);
    }
    m_locked = true;
}
void Pixelizer::unlock()
{
    if(!m_mustLock)
        return;
    SDL_UnlockSurface(m_surface);
    m_locked = false;
}
int Pixelizer::getWidth() const
{
    return m_surface->w;
}
int Pixelizer::getHeight() const
{
    return m_surface->h;
}
