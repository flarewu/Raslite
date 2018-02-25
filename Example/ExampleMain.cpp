#include "Pixelizer.h"
#include "Example.h"


#include "../RasliteX/RasliteXTimer.h"
#include "../RasliteX/RasliteXInput.h"

using namespace rl;
using namespace rlx;
constexpr uint32_t _BACKBUFFER_W = 640*2, _BACKBUFFER_H = 480*2;
class Pixelizer;
typedef std::shared_ptr<Pixelizer> PixelizerPtr;
extern std::unique_ptr<Example> DrawTriangleExampleCreate(uint32_t backbufferW, uint32_t backbufferH);
extern std::unique_ptr<Example> DrawIndexedExampleCreate(uint32_t backbufferW, uint32_t backbufferH);
extern std::unique_ptr<Example> DrawBoxExampleCreate(uint32_t backbufferW, uint32_t backbufferH);
extern std::unique_ptr<Example> DrawHolographicExampleCreate(uint32_t backbufferW, uint32_t backbufferH);
extern std::unique_ptr<Example> CheckerBoardExampleCreate(uint32_t backbufferW, uint32_t backbufferH);
extern std::unique_ptr<Example> GroundExampleCreate(uint32_t backbufferW, uint32_t backbufferH);

std::unique_ptr<Example> ExampleCreate(int id)
{
    switch(id)
    {
    case 1:
        return DrawTriangleExampleCreate(_BACKBUFFER_W, _BACKBUFFER_H);
        break;
    case 2:
        return DrawIndexedExampleCreate(_BACKBUFFER_W, _BACKBUFFER_H);
        break;
    case 3:
        return DrawBoxExampleCreate(_BACKBUFFER_W, _BACKBUFFER_H);
        break;
    case 4:
        return DrawHolographicExampleCreate(_BACKBUFFER_W, _BACKBUFFER_H);
        break;
    case 5:
        return CheckerBoardExampleCreate(_BACKBUFFER_W, _BACKBUFFER_H);
        break;
    case 6:
        return GroundExampleCreate(_BACKBUFFER_W, _BACKBUFFER_H);
        break;
    default:
        return DrawBoxExampleCreate(_BACKBUFFER_W, _BACKBUFFER_H);
    }
}
void PresentSurface(Pixelizer* pxlzr, Surface* color)
{
    const auto nFloats = color->getFormatFloatCount();
    const auto width   = color->getWidth();
    const auto height  = color->getHeight();
    const auto pitch   = color->getWidth() * color->getFormatFloatCount();

    auto dataptr = color->lock();
    for(uint32_t h = 0; h < height; ++h)
    {
        for(uint32_t w = 0; w < width; ++w)
        {
            Float* colorData = dataptr + (h * pitch + w*nFloats);

            auto colorVal =
                (rl::clamp(rl::float2int_fast(colorData[0] * 255.0f), 0, 255) << 16) |  //r
                (rl::clamp(rl::float2int_fast(colorData[1] * 255.0f), 0, 255) << 8) |   //g
                (rl::clamp(rl::float2int_fast(colorData[2] * 255.0f), 0, 255));         //b
            pxlzr->putPixel(w, h, colorVal);
        }
    }
    color->unlock();
}
class InputSDL: public rlx::Input
{
public:
    virtual int32_t getMouseWheelV() const override 
    {
        return 0;
    }
    virtual int32_t getMouseWheelH() const override 
    {
        return 0;
    }
    virtual std::pair<uint32_t, uint32_t> getMousePosition() const 
    {
        return std::make_pair(0, 0);
    }
    void setKeyState(KeyCodeE kc,KeyState ks)
    {
        m_keyStates[kc] = ks;
    }
    void setMousePosition(int x, int y)
    {
        m_mousePos.x = x; m_mousePos.y = y;
    }
};
int main(int argc, char* argv[])
{
    std::cout <<"1: Draw Triangle" << std::endl;
    std::cout <<"2: Draw Indexed Mesh" << std::endl;
    std::cout <<"3: Draw Box" << std::endl;
    std::cout <<"4: Draw Holographic" << std::endl;
    std::cout <<"5: CheckerBoard" << std::endl;
    std::cout <<"8: Ground" << std::endl;

    auto example = ExampleCreate(0);
    auto input   = std::make_unique<InputSDL>();

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        std::exit(1);
    }
    std::atexit(SDL_Quit);
    auto window  = SDL_CreateWindow("Raslite Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _BACKBUFFER_W, _BACKBUFFER_H, 0);
    auto surface = SDL_GetWindowSurface(window);

    auto pxlzr = std::make_unique<Pixelizer>(surface);
    Timer::init();
    while(true)
    {
        Timer::update();
        input->reset();
        SDL_Event evt;
        while(SDL_PollEvent(&evt))
        {
            switch(evt.type)
            {
            case SDL_KEYDOWN:
                if(SDLK_0 < evt.key.keysym.sym&&evt.key.keysym.sym <= SDLK_9)
                    example = ExampleCreate(evt.key.keysym.sym - SDLK_0);
                break;
            }
            if(!example.get())
                break;
            switch(evt.type)
            {
            case SDL_QUIT:
                return(0);
            case SDL_KEYUP:
                switch(evt.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    return 0;
                case SDLK_LCTRL:
                    input->setKeyState(KeyCode::LCTRL, KeyState::RELEASED);
                    break;
                case SDLK_LSHIFT:
                    input->setKeyState(KeyCode::LSHIFT, KeyState::RELEASED);
                    break;
                case SDLK_d:
                    //toDisplayDepth = !toDisplayDepth;
                    ;
                }
                break;
            case SDL_KEYDOWN:
                switch(evt.key.keysym.sym)
                {
                case SDLK_LCTRL:
                    input->setKeyState(KeyCode::LCTRL, KeyState::PRESSED);
                    break;
                case SDLK_LSHIFT:
                    input->setKeyState(KeyCode::LSHIFT, KeyState::PRESSED);
                    break;
                case SDLK_d:
                    //toDisplayDepth = !toDisplayDepth;
                case SDLK_q:
                    switch(example->getFillMode())
                    {
                    case FillMode::WIRE_FRAME:
                        example->setFillMode(FillMode::SOLID);
                        break;
                    case FillMode::SOLID:
                        example->setFillMode(FillMode::WIRE_FRAME);
                        break;
                    default:
                        break;
                    }
                    break;
                case SDLK_SPACE:
                    example->setFreezed(!example->isFreezed());
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                input->setMousePosition(evt.motion.x, evt.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch(evt.button.button)
                {
                case SDL_BUTTON_LEFT:
                    input->setKeyState(KeyCode::LMB, KeyState::PRESSED);
                    break;
                case SDL_BUTTON_MIDDLE:
                    input->setKeyState(KeyCode::MMB, KeyState::PRESSED);
                    break;
                case SDL_BUTTON_RIGHT:
                    input->setKeyState(KeyCode::RMB, KeyState::PRESSED);
                    break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch(evt.button.button)
                {
                case SDL_BUTTON_LEFT:
                    input->setKeyState(KeyCode::LMB, KeyState::RELEASED);
                    break;
                case SDL_BUTTON_MIDDLE:
                    input->setKeyState(KeyCode::MMB, KeyState::RELEASED);
                    break;
                case SDL_BUTTON_RIGHT:
                    input->setKeyState(KeyCode::RMB, KeyState::RELEASED);
                    break;
                }
                break;
            case SDL_MOUSEWHEEL:
                break;

            }
        }//while
        if(example.get())
        {
            example->update(Timer::get().lastFrameTime*0.001f);
            PresentSurface(pxlzr.get(), example->getRenderTarget());
        }

        SDL_UpdateWindowSurface(window);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    Timer::deinit();

    return 0;

}
