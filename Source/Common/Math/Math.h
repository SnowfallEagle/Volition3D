#pragma once

#include <cmath>
#include <ctime>
#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"

namespace Volition
{

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

    static constexpr f32 Epsilon3 = (f32)(1E-3);
    static constexpr f32 Epsilon4 = (f32)(1E-4);
    static constexpr f32 Epsilon5 = (f32)(1E-5);
    static constexpr f32 Epsilon6 = (f32)(1E-6);

    static constexpr i32f SinCosLookSize = 361;

public:
    /**
        Tables are static because we want to
        have static FastSin/Cos funcs and don't
        pass extra argument (this) when we call them
    */
    inline static f32 SinLook[SinCosLookSize]; // 0-360
    inline static f32 CosLook[SinCosLookSize]; // 0-360

public:
    void StartUp()
    {
        // Set random seed
        std::srand((u32)std::time(nullptr));

        // Build look up tables
        for (i32f I = 0; I < SinCosLookSize; ++I)
        {
            const f32 Rad = DegToRad((f32)I);

            SinLook[I] = std::sinf(Rad);
            CosLook[I] = std::cosf(Rad);
        }
    }

    void ShutDown()
    {}

    VLN_FINLINE static f32 Abs(f32 X)
    {
        return std::fabsf(X);
    }
    VLN_FINLINE static i32 Abs(i32 X)
    {
        return std::abs(X);
    }

    VLN_FINLINE static f32 Floor(f32 X)
    {
        return std::floorf(X);
    }
    VLN_FINLINE static f32 Ceil(f32 X)
    {
        return std::ceilf(X);
    }

    VLN_FINLINE static b32 IsEqualFloat(f32 A, f32 B)
    {
        return Abs(A - B) < Epsilon3;
    }

    static f32 FastDist2D(f32 X, f32 Y) // 3.5% Error
    {
        // Absolute integer values
        const i32 IX = (i32)Abs(X);
        const i32 IY = (i32)Abs(Y);

        const i32 Min = VLN_MIN(IX, IY);

        return (f32)(IX + IY - (Min >> 1) - (Min >> 2) + (Min >> 4));
    }
    static f32 FastDist3D(f32 X, f32 Y, f32 Z) // 8% Error
    {
        // Absolute integer values
        i32 IX = (i32)(Abs(X)) << 10;
        i32 IY = (i32)(Abs(Y)) << 10;
        i32 IZ = (i32)(Abs(Z)) << 10;
        i32 T; // Temp

        // Sort
        if (IX > IY) VLN_SWAP(IX, IY, T);
        if (IY > IZ) VLN_SWAP(IY, IZ, T);
        if (IX > IY) VLN_SWAP(IX, IY, T);

        return (f32)(
            (IZ + 11*(IY >> 5) + (IX >> 2)) >> 10
        );
    }

    static f32 FastSin(f32 Deg)
    {
        Deg = std::fmodf(Deg, 360);
        if (Deg < 0)
        {
            Deg += 360;
        }

        const i32f I = (i32f)Deg;
        const f32 Remainder = Deg - (f32)I;
        return SinLook[I] + Remainder * (SinLook[I+1] - SinLook[I]);
    }
    static f32 FastCos(f32 Deg)
    {
        Deg = std::fmodf(Deg, 360);
        if (Deg < 0)
        {
            Deg += 360;
        }

        const i32f I = (i32f)Deg;
        const f32 Remainder = Deg - (f32)I;
        return CosLook[I] + Remainder * (CosLook[I+1] - CosLook[I]);
    }

    VLN_FINLINE static f32 Sqrt(f32 X)
    {
        return std::sqrtf(X);
    }
    VLN_FINLINE static f32 Sin(f32 Deg)
    {
        return std::sinf(Deg * DegToRadConversion);
    }
    VLN_FINLINE static f32 Cos(f32 Deg)
    {
        return std::cosf(Deg * DegToRadConversion);
    }
    VLN_FINLINE static f32 Tan(f32 Deg)
    {
        return std::tanf(Deg * DegToRadConversion);
    }

    VLN_FINLINE static f32 DegToRad(f32 Deg)
    {
        return Deg * DegToRadConversion;
    }
    VLN_FINLINE static f32 RadToDeg(f32 Rad)
    {
        return Rad * RadToDegConversion;
    }

    VLN_FINLINE static i32 Random(i32 Range) // From 0 to "Range"-1
    {
        return std::rand() % Range;
    }
    VLN_FINLINE static i32 Random(i32 From, i32 To) // From "From" to "To"
    {
        return From + (std::rand() % (To - From + 1));
    }

    VLN_FINLINE static f32 Mod(f32 Dividend, f32 Divisor)
    {
        return std::fmodf(Dividend, Divisor);
    }
};

inline VMath Math;

}
