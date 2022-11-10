#pragma once

/* TODO(sean)
    - Remove templates from vectors except 2D
 */

#include <cmath>
#include <ctime>
#include <string.h>
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Math/Rect.h"
#include "Math/Fixed16.h"

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

public:
    FINLINE void Zero()
    {
        X = Y = 0;
    }

    FINLINE void Print()
    {
        if (T == f32)
            VL_LOG("<%f, %f>", X, Y);
        else if (T == i32)
            VL_LOG("<%d, %d>", X, Y);
        else
            VL_LOG("<Unknown type>");
    }
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

public:
    FINLINE void Zero()
    {
        X = Y = Z = 0;
    }

    FINLINE void Print()
    {
        if (T == f32)
            VL_LOG("<%f, %f, %f>", X, Y, Z);
        else if (T == i32)
            VL_LOG("<%d, %d, %d>", X, Y, Z);
        else
            VL_LOG("<Unknown type>");
    }
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

public:
    FINLINE void Zero()
    {
        X = Y = Z = W = 0;
    }

    FINLINE void Print()
    {
        if (T == f32)
            VL_LOG("<%f, %f, %f, %f>", X, Y, Z, W);
        else if (T == i32)
            VL_LOG("<%d, %d, %d, %d>", X, Y, Z, W);
        else
            VL_LOG("<Unknown type>");
    }
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

public:
    FINLINE void operator=(const VMatrix44& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
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

public:
    FINLINE void operator=(const VMatrix43& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
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

public:
    FINLINE void operator=(const VMatrix14& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
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

public:
    FINLINE void operator=(const VMatrix33& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
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

public:
    FINLINE void operator=(const VMatrix32& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
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

public:
    FINLINE void operator=(const VMatrix13& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
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

public:
    FINLINE void operator=(const VMatrix22& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
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

public:
    FINLINE void operator=(const VMatrix12& Mat)
    {
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

// Indentity matrices

// NOTE(sean): Should we make it as extern stuff?
static const VMatrix44 IdentityMatrix44 = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

static const VMatrix43 IdentityMatrix43 = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
    0, 0, 0,
};

static const VMatrix33 IdentityMatrix33 = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
};

static const VMatrix32 IdentityMatrix32 = {
    1, 0,
    0, 1,
    0, 0,
};

static const VMatrix22 IdentityMatrix22 = {
    1, 0,
    0, 1,
};

// TODO(sean): Transpose, ColumnSwap, Print functions for matrices

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
            /** NOTE(sean):
                We can't cast quaternion to 4d
                vector because of W on first place
              */
            f32 W, X, Y, Z;
        };
    };

public:
    FINLINE void Zero()
    {
        W = X = Y = Z = 0;
    }

    FINLINE void InitVec3(const VVector3D* V)
    {
        W = 0;
        X = V->X;
        Y = V->Y;
        Z = V->Z;
    }

    FINLINE void Print()
    {
        VL_LOG("<%f, %f, %f, %f>", W, X, Y, Z);
    }
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
