#pragma once

#include "Core/Platform.h"

#if VL_IMPL_SDL
# include "Graphics/Impl/Surface_SDL.h"
#elif VL_IMPL_DDRAW
# include "Graphics/Impl/Surface_DDraw.h"
#endif
