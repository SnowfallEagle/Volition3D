#include "Core/Assert.h"
#include "Graphics/Graphics.h"
#include "Graphics/Surface.h"

void VSurface::Lock(u32*& OutVideoBuffer, i32& OutPitchInPixels)
{
	ASSERT(SDLSurface);

	if (!SDLSurface->locked && SDL_MUSTLOCK(SDLSurface))
		SDL_LockSurface(SDLSurface);

	OutVideoBuffer = (u32*)SDLSurface->pixels;
	OutPitchInPixels = SDLSurface->pitch / Graphics.GetPixelFormat().BytesPerPixel;
}
