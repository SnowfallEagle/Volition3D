#include "Core/Assert.h"
#include "Graphics/Graphics.h"
#include "Graphics/Surface.h"

VSurface* VSurface::Create(SDL_Surface* InSDLSurface)
{
    VSurface* Surface = new VSurface();
    Surface->SDLSurface = InSDLSurface;
    return Surface;
}

VSurface* VSurface::Load(const char* Path)
{
    SDL_Surface* Temp = SDL_LoadBMP(Path);
    ASSERT(Temp);

    SDL_Surface* Converted = SDL_ConvertSurface(
        Temp, Graphics.GetVideoSurface()->GetPlatformSurface()->format, 0
    );
    ASSERT(Converted);
    SDL_FreeSurface(Temp);

    return Create(Converted);
}

void VSurface::Destroy()
{
    if (SDLSurface)
        SDL_FreeSurface(SDLSurface);
}
