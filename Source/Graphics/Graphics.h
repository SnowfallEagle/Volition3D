#ifndef GRAPHICS_GRAPHICS_H_

#include "Core/Platform.h"

#if VL_IMPL_SDL
# include "Graphics/Impl/Graphics_SDL.h"
#else if VL_IMPL_DDRAW
# include "Graphics/Impl/Graphics_DDraw.h"
#endif

extern VGraphics Graphics;

#define GRAPHICS_GRAPHICS_H_
#endif
