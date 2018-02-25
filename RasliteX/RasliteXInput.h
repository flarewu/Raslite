#ifndef RASLITEX_INPUT_H
#define RASLITEX_INPUT_H

#include "../Raslite/Raslite.h"

namespace rlx {
    using namespace rl;
    struct KeyCode
    {
        enum Enum
        {
            A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
            LCTRL,
            LSHIFT,

            LMB,
            MMB,
            RMB,

            MAX,
        };
    };
    enum class KeyState: uint8_t
    {
        NORMAL = 0,
        PRESSED,
        RELEASED,
    };
    using KeyCodeE = KeyCode::Enum;
    class Input
    {
    public:
        static Input& singleton()
        {
            assert(ms_singleton);
            return *ms_singleton;
        }
    public:
        Input()
        {
            assert(!ms_singleton);
            ms_singleton = this;
        }
        ~Input()
        {
            assert(ms_singleton);
            ms_singleton = nullptr;
        }
        bool isPressed(KeyCodeE kc) const
        {
            return m_keyStates[kc] == KeyState::PRESSED;
        }
        bool isReleased(KeyCodeE kc) const
        {
           return m_keyStates[kc] == KeyState::RELEASED;
        }
        void reset()
        {
            std::memset(m_keyStates, 0, sizeof(m_keyStates));
            m_mousePos = Vec2i(0, 0);
        }
        Vec2i   getMousePosition() const
        {
            return m_mousePos;
        }
        virtual int32_t getMouseWheelV()   const = 0; //Vertical
        virtual int32_t getMouseWheelH()   const = 0; //Horizontal

    private:
        static Input* ms_singleton;
    protected:
        KeyState m_keyStates[KeyCode::MAX] = { KeyState::NORMAL };
        Vec2i m_mousePos;
    };

}//ns rlx

#endif //RLX_INPUT_H