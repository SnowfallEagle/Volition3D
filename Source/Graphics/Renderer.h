#pragma once

#include "Core/Platform.h"
#include "Graphics/RenderSpecification.h"

#if VL_IMPL_SDL
# include "Graphics/Impl/Renderer_SDL.h"
#endif

extern VRenderer Renderer;
