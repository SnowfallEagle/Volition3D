#ifndef INPUT_INPUT_H_

#include "Core/Platform.h"

#if VL_IMPL_SDL
# include "Input/Impl/Input_SDL.h"
#elif VL_IMPL_DDRAW
# include "Input/Impl/Input_DDRAW.h"
#endif

extern VInput Input;

#define INPUT_INPUT_H_
#endif
