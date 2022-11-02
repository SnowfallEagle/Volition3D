#ifndef INPUT_H_

#include "SDL.h"
#include "Types.h"
#include "Platform.h"
#include "Input/KeyCode.h"
#include "Input/MouseButton.h"

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

    FINLINE b32 IsKeyDown(u32 Key) const
    {
        return KeyState[SDL_GetScancodeFromKey(Key)];
    }

    FINLINE b32 IsMouseDown(u32 Button) const
    {
        return MouseState & Button;
    }

    FINLINE i32 GetMouseX() const
    {
        return MousePosX;
    }

    FINLINE i32 GetMouseY() const
    {
        return MousePosY;
    }
};

extern VInput Input;

#define INPUT_H_
#endif
