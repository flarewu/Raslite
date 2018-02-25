#ifndef RLEXAMPLE_PIXELER_H
#define RLEXAMPLE_PIXELER_H

#include <SDL.h>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <iostream>
#include <vector>

struct ScreenCoordinate
{
    ScreenCoordinate(int xx, int yy): x(xx), y(yy) {}
    int x, y;
};
using ScreenCoordinates = std::vector<ScreenCoordinate>;

class Pixelizer
{
public:
    explicit Pixelizer(SDL_Surface* surface);
    uint32_t getPixel(int x, int y);
    void putPixel(int x, int y, uint32_t pixel);
    void putPixels(const ScreenCoordinates& coords, uint32_t color);
    void lock();
    void unlock();
    int getWidth() const;
    int getHeight() const;
    SDL_Surface* getSurface() const {
        return m_surface;
    }
private:
    SDL_Surface* m_surface;
    bool m_mustLock;
    bool m_locked;
};
class Pixelizer;
typedef std::shared_ptr<Pixelizer> PixelizerPtr;



#endif //RLEXAMPLE_PIXELER_H
