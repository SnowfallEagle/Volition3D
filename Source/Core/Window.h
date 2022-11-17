#pragma once

#include "Core/Platform.h"

#if VL_IMPL_SDL
# include "Core/Impl/Window_SDL.h"
#elif VL_IMPL_DDRAW
# include "Core/Impl/Window_DDraw.h"
#endif

extern VWindow Window;
