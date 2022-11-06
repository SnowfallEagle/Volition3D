#ifndef MATH_MATH_H_

#include <cmath>
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Math/Rect.h"
#include "Math/Fixed16.h"

/** NOTE(sean):
    It's small C-styled math library, so 
     we don't use methods here, only functions.

    Only constants are in math namespace
 */
namespace math
{
    static constexpr f32 Pi = 3.141592654f;
    static constexpr f32 Pi2 = 6.283185307f;
    static constexpr f32 PiDiv2 = 1.570796327f;
    static constexpr f32 PiDiv4 = 0.785398163f;
    static constexpr f32 PiInv = 0.318309886f;

    static constexpr f32 DegToRadConversion = math::Pi/180.0f;
    static constexpr f32 RadToDegConversion = 180.0f/math::Pi;

    static constexpr f32 Epsilon4 = (f32)(1E-4);
    static constexpr f32 Epsilon5 = (f32)(1E-5);
    static constexpr f32 Epsilon6 = (f32)(1E-6);
}

#define Min(A, B) ((A) < (B) ? (A) : (B))
#define Max(A, B) ((A) > (B) ? (A) : (B))
#define Swap(A, B, T) { T = A; A = B; B = T; }

#define DegToRad(A) ((A) * DegToRadConversion)
#define RadToDeg(A) ((A) * RadToDegConversion)

FINLINE i32 Random(i32 Range) // From 0 to Range-1
{
    return std::rand() % Range;
}

FINLINE i32 Random(i32 From, i32 To)
{
    return From + (std::rand() % (To - From + 1));
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
            /** NOTE(sean):
                We can't cast quaternion to 4d
                vector because of W on first place
              */
            f32 W, X, Y, Z;
        };
    };
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

#define MATH_MATH_H_
#endif