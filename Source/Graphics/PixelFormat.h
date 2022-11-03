#ifndef GRAPHICS_PIXELFORMAT_H_

#include "Core/Types.h"

struct SDL_PixelFormat;

class VPixelFormat
{
public:
    u32 AlphaLoss, RedLoss, GreenLoss, BlueLoss;
    u32 AlphaShift, RedShift, GreenShift, BlueShift;
    u32 AlphaMask, RedMask, GreenMask, BlueMask;
    i32 BytesPerPixel, BitsPerPixel;

    SDL_PixelFormat* SDLPixelFormat;
    u32 SDLPixelFormatEnum;
};

extern VPixelFormat PixelFormat;

#define GRAPHICS_PIXELFORMAT_H_
#endif
