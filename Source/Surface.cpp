#include "Assert.h"
#include "Graphics.h"
#include "Surface.h"

void VSurface::Lock(u32*& OutVideoBuffer, i32& OutPitchInPixels)
{
	ASSERT(SDLSurface);

	if (SDL_MUSTLOCK(SDLSurface))
		SDL_LockSurface(SDLSurface);

	OutVideoBuffer = (u32*)SDLSurface->pixels;
	OutPitchInPixels = SDLSurface->pitch / Graphics.GetPixelFormat().BytesPerPixel;
}
