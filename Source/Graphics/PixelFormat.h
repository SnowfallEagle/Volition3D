#ifndef PIXELFORMAT_H_

#include "Core/Types.h"

class VPixelFormat
{
public:
    u32 AlphaLoss, RedLoss, GreenLoss, BlueLoss;
    u32 AlphaShift, RedShift, GreenShift, BlueShift;
    u32 AlphaMask, RedMask, GreenMask, BlueMask;
    i32 BytesPerPixel, BitsPerPixel;
};

#define PIXELFORMAT_H_
#endif