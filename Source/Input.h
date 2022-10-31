#ifndef INPUT_H_

#include "SDL.h"
#include "Types.h"

namespace EKeyCode
{
    enum
    {

    };
}

namespace EMouseButton
{
    enum
    {
        Left = SDL_BUTTON_LMASK,
        Middle = SDL_BUTTON_MMASK,
        Right = SDL_BUTTON_RMASK
    };
}

class VInput
{
    const u8* KeyState = nullptr;
    u32 MouseState = 0;
    i32 MousePosX = 0;
    i32 MousePosY = 0;

public:
    void StartUp()
    {
        KeyState = SDL_GetKeyboardState(nullptr);
    }
    void ShutDown()
    {

    }

    void HandleEvents()
    {
        MouseState = SDL_GetMouseState(&MousePosX, &MousePosY);
    }

    b32 IsKeyDown(u32 Key) const
    {
        return KeyState[SDL_GetScancodeFromKey(Key)];
    }

    b32 IsMouseDown(u32 Button) const
    {
        return MouseState & Button;
    }

    i32 GetMouseX() const
    {
        return MousePosX;
    }

    i32 GetMouseY() const
    {
        return MousePosY;
    }
};

extern VInput Input;

#define INPUT_H_
#endif
