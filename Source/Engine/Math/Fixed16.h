#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/DebugLog.h"

namespace Volition
{

// *** Fixed point 16.16 ***
using fx16 = i32;

namespace
{
    static constexpr i32 Shift = 16;
    static constexpr f32 Magnitude = 65535.0f;

    static constexpr i32 WholePartMask = 0xFFFF0000;
    static constexpr i32 DecimalPartMask = 0x0000FFFF;
}

static constexpr i32 Fx16RoundUp = 0x00008000;

VLN_FINLINE fx16 IntToFx16(i32 I)
{
    return I << Shift;
}

VLN_FINLINE fx16 FloatToFx16(f32 F)
{
    return (fx16)(F * Magnitude + 0.5f);
}

VLN_FINLINE i32 Fx16ToInt(fx16 Fx)
{
    return Fx >> Shift;
}

VLN_FINLINE i32 Fx16ToIntRounded(fx16 Fx)
{
    return (Fx + Fx16RoundUp) >> Shift;
}

VLN_FINLINE f32 Fx16ToFloat(fx16 Fx)
{
    return (f32)Fx / Magnitude;
}

VLN_FINLINE fx16 MulFx16(fx16 Fx1, fx16 Fx2)
{
#if VLN_COMPILER_MSVC
    /* NOTE:
        Let X, Y are integers and P, Q are fixed point numbers.
        P = (fx16)X, Q = (fx16)Y.
        Now P is actually (X * 2^16) and Q is (Y * 2^16).

        When we want to multiply P by Q we got:
        Res = (X * 2^16) * (Y * 2^16) = XY * 2^32
        But we do want to see this: Res is equal XY * 2^16

        So all we need to do is:
            1. Use 64 math to get result in EDX:EAX
            2. Shift our whole result right by 16
            3. Get result in one EAX register
    */

    __asm
    {
        mov     eax, Fx1            // Fx1->eax
        imul    Fx2                 // eax *= Fx2, result in edx:eax
        shrd    eax, edx, 16        // shift eax right by 2^16,
                                    // move low 16 bytes from edx to
                                    //  eax high 16 bytes
        // Result in eax
    }
#else
# error "There are no implementation"
#endif // VLN_COMPILER_MSVC
}

VLN_FINLINE fx16 DivFx16(fx16 Fx1, fx16 Fx2)
{
#if VLN_COMPILER_MSVC
    /* NOTE:
        Let X, Y are integers and P, Q are fixed point numbers.
        P = (fx16)X, Q = (fx16)Y.
        Now P is actually (X * 2^16) and Q is (Y * 2^16).

        When we want to divide P by Q we got:
        Res = (X * 2^16) / (Y * 2^16) = XY
        But we do want to see this: Res is equal XY * 2^16

        So all we need to do is:
            1. Extend fx16 to 64 bit to prevent accuracy losing
            2. Shift this left by 16
            3. Divide our extended fixed number
            4. Get result in one EAX register
    */

    __asm
    {
        mov     eax, Fx1            // Fx1->eax
        cdq                         // Extend eax to edx:eax
        shld    edx, eax, 16        // Now 16 high bits of Fx1 in dl and
                                    //  16 low bits of Fx1 in ah
        sal     eax, 16             // Previous operand didn't shifted our eax, so
                                    //  eax *= 2^16 and keep sign
        idiv    Fx2                 // Divide by Fx2
        // Result in eax
    }
#else
# error "There are no implementation"
#endif // VLN_COMPILER_MSVC
}

VLN_FINLINE i32 GetFx16WholePart(fx16 Fx)
{
    return Fx >> Shift;
}

VLN_FINLINE i32 GetFx16DecimalPart(fx16 Fx)
{
    return Fx & DecimalPartMask;
}

VLN_FINLINE void PrintFx16(fx16 Fx, char EndChar = 0)
{
    VLN_LOG("%f%c", Fx16ToFloat(Fx), EndChar);
}

}