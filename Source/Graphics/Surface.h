#ifndef GRAPHICS_SURFACE_H_

#include "Core/Platform.h"

#if VL_IMPL_SDL
# include "Graphics/Impl/Surface_SDL.h"
#else if VL_IMPL_DDRAW
# include "Graphics/Impl/Surface_DDraw.h"
#endif

#define GRAPHICS_SURFACE_H_
#endif