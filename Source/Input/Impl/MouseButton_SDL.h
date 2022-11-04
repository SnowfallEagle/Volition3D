#ifndef INPUT_IMPL_MOUSEBUTTON_SDL_H_

#include "SDL_mouse.h"

namespace EMouseButton
{
    enum
    {
        Left = SDL_BUTTON_LMASK,
        Middle = SDL_BUTTON_MMASK,
        Right = SDL_BUTTON_RMASK
    };
}

#define INPUT_IMPL_MOUSEBUTTON_SDL_H_
#endif
