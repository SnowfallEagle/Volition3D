#ifndef MATH_MATH_H_

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Math/Rect.h"

/** NOTE(sean):
    Only constants are in math namespace
 */
namespace math
{
    static constexpr f32 Pi = 3.141592654f;
    static constexpr f32 Pi2 = 6.283185307f;
    static constexpr f32 PiDiv2 = 1.570796327f;
    static constexpr f32 PiDiv4 = 0.785398163f;
    static constexpr f32 PiInv = 0.318309886f;

    static constexpr f32 Epsilon4 = (f32)(1E-4);
    static constexpr f32 Epsilon5 = (f32)(1E-5);
    static constexpr f32 Epsilon6 = (f32)(1E-6);
}

// *** Vector ***

// 2D
template<class T>
class TVector2D
{
public:
    union
    {
        T C[2];
        struct
        {
            T X, Y;
        };
    };
};

typedef TVector2D<f32> VVector2D, VPoint2D;
typedef TVector2D<i32> VVector2DI, VPoint2DI;

// 3D
template<class T>
class TVector3D
{
public:
    union
    {
        T C[3];
        struct
        {
            T X, Y, Z;
        };
    };
};

typedef TVector3D<f32> VVector3D, VPoint3D;
typedef TVector3D<i32> VVector3DI, VPoint3DI;

// 4D
template<class T>
class TVector4D
{
public:
    union
    {
        T C[4];
        struct
        {
            T X, Y, Z, W;
        };
    };
};

typedef TVector4D<f32> VVector4D, VPoint4D;
typedef TVector4D<i32> VVector4DI, VPoint4DI;

// *** Parametric Line ***
template<class POINT, class VECTOR>
class TParamLine
{
public:
    POINT P0, P1;
    VECTOR V; // P1 - P0 direction vector
};

// NOTE(sean): Idk if we need param line with floats or fixed...
typedef TParamLine<VPoint2D, VVector2D> VParamLine2D;
typedef TParamLine<VPoint3D, VVector3D> VParamLine3D;

// *** Plane ***
class VPlane3D
{
public:
    VPoint3D P0; // Point to the plane
    VVector3D N; // It's normal vector
};

// *** Matrix ***

// 4x4
class VMatrix44
{
public:
    union
    {
        f32 C[4][4];
        struct
        {
            f32 C00, C01, C02, C03;
            f32 C10, C11, C12, C13;
            f32 C20, C21, C22, C23;
            f32 C30, C31, C32, C33;
        };
    };
};

// 4x3
class VMatrix43
{
public:
    union
    {
        f32 C[4][3];
        struct
        {
            f32 C00, C01, C02;
            f32 C10, C11, C12;
            f32 C20, C21, C22;
            f32 C30, C31, C32;
        };
    };
};

// 1x4
class VMatrix14
{
public:
    union
    {
        f32 C[4];
        struct
        {
            f32 C00, C01, C02, C03;
        };
    };
};

// 3x3
class VMatrix33
{
public:
    union
    {
        f32 C[3][3];
        struct
        {
            f32 C00, C01, C02;
            f32 C10, C11, C12;
            f32 C20, C21, C22;
        };
    };
};

// 3x2
class VMatrix32
{
public:
    union
    {
        f32 C[3][2];
        struct
        {
            f32 C00, C01;
            f32 C10, C11;
            f32 C20, C21;
        };
    };
};

// 1x3
class VMatrix13
{
public:
    union
    {
        f32 C[3];
        struct
        {
            f32 C00, C01, C02;
        };
    };
};

// 2x2
class VMatrix22
{
public:
    union
    {
        f32 C[2][2];
        struct
        {
            f32 C00, C01;
            f32 C10, C11;
        };
    };
};

// 1x2
class VMatrix12
{
public:
    union
    {
        f32 C[2];
        struct
        {
            f32 C00, C01;
        };
    };
};

// *** Quaternion ***
class VQuaternion
{
public:
    union
    {
        f32 C[4];
        struct
        {
            f32 Q0;
            VVector3D QV;
        };
        struct
        {
            f32 W, X, Y, Z;
        };
    };
};

// *** Coordinates ***
class VPolar2D
{
public:
    f32 R; // Radius
    f32 Theta; // Angle in radians
};

class VCylindrical3D
{
public:
    f32 R; // Radius
    f32 Theta; // Angle about z-axis
    f32 Z; // z-height
};

class VSpherical3D
{
public:
    f32 P; // Rho, the distance to the point from the origin
    f32 Theta; // Angle that projection O->P on x-y plane. Just like Theta in VPolar2D
    f32 Phi; // Angle from the z-axis and the line segment O->P 
};

// *** Fixed point 16.16 ***
typedef i32 fx16;

namespace
{
    static constexpr i32f Shift = 16;
    static constexpr f32 Magnitude = 65535.0f;

    static constexpr i32f WholePartMask = 0xFFFF0000;
    static constexpr i32f DecimalPartMask = 0x0000FFFF;
    static constexpr i32f RoundUpMask = 0x00008000;
}

FINLINE fx16 ToFixed(i32 I)
{
    return I << Shift;
}

FINLINE fx16 ToFixed(f32 F)
{
    return (fx16)(F * Magnitude + 0.5f);
}

FINLINE i32 ToInt(fx16 Fx)
{
    return Fx >> Shift;
}

FINLINE f32 ToFloat(fx16 Fx)
{
    return (f32)Fx / Magnitude;
}

FINLINE i32 GetWholePart(fx16 Fx)
{
    return Fx >> Shift;
}

FINLINE i32 GetDecimalPart(fx16 Fx)
{
    return Fx & DecimalPartMask;
}

FINLINE void Print(fx16 Fx, char EndChar = 0)
{
    VL_LOG("%f%c", ToFloat(Fx), EndChar);
}

FINLINE fx16 MulFx(fx16 Fx1, fx16 Fx2)
{
    /* NOTE(sean):
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
}

FINLINE fx16 DivFx(fx16 Fx1, fx16 Fx2)
{
    /* NOTE(sean):
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
}

#define MATH_MATH_H_
#endif