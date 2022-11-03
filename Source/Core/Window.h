#ifndef CORE_WINDOW_H_

#include "Core/Platform.h"

#if VL_IMPL_SDL
# include "Core/Impl/Window_SDL.h"
#else if VL_IMPL_DDRAW
# include "Core/Impl/Window_DDraw.h"
#endif

extern VWindow Window;

#define CORE_WINDOW_H_
#endif
