#pragma once

#include <cmath>
#include <ctime>
#include "Core/Types.h"
#include "Core/Platform.h"

class VMath
{
public:
    static constexpr f32 Pi = 3.141592654f;
    static constexpr f32 Pi2 = 6.283185307f;
    static constexpr f32 PiDiv2 = 1.570796327f;
    static constexpr f32 PiDiv4 = 0.785398163f;
    static constexpr f32 PiInv = 0.318309886f;

    static constexpr f32 DegToRadConversion = Pi / 180.0f;
    static constexpr f32 RadToDegConversion = 180.0f / Pi;

    static constexpr f32 Epsilon4 = (f32)(1E-4);
    static constexpr f32 Epsilon5 = (f32)(1E-5);
    static constexpr f32 Epsilon6 = (f32)(1E-6);

    static constexpr i32f SinCosLookSize = 361;

public:
    f32 SinLook[SinCosLookSize]; // 0-360
    f32 CosLook[SinCosLookSize]; // 0-360

public:
    void StartUp()
    {
        // Set random seed
        std::srand((u32)std::time(nullptr));

        // Build look up tables
        for (i32f I = 0; I < SinCosLookSize; ++I)
        {
            f32 Rad = DegToRad((f32)I);

            SinLook[I] = sinf(Rad);
            CosLook[I] = cosf(Rad);
        }
    }
    void ShutDown()
    {
    }

    f32 FastSin(f32 Angle)
    {
        Angle = fmodf(Angle, 360);
        if (Angle < 0)
            Angle += 360;

        i32f I = (i32f)Angle;
        f32 Remainder = Angle - (f32)I;
        return SinLook[I] + Remainder * (SinLook[I+1] - SinLook[I]);
    }
    f32 FastCos(f32 Angle)
    {
        Angle = fmodf(Angle, 360);
        if (Angle < 0)
            Angle += 360;

        i32f I = (i32f)Angle;
        f32 Remainder = Angle - (f32)I;
        return CosLook[I] + Remainder * (CosLook[I+1] - CosLook[I]);
    }

    FINLINE static f32 DegToRad(f32 Deg)
    {
        return Deg * DegToRadConversion;
    }
    FINLINE static f32 RadToDeg(f32 Rad)
    {
        return Rad * RadToDegConversion;
    }

    FINLINE static i32 Random(i32 Range) // From 0 to "Range"-1
    {
        return std::rand() % Range;
    }
    FINLINE static i32 Random(i32 From, i32 To) // From "From" to "To"
    {
        return From + (std::rand() % (To - From + 1));
    }
};

extern VMath Math;

