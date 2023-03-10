#pragma once

#include "Engine/Math/Math.h"
#include "Engine/Math/Fixed16.h"
#include "Engine/Math/Rect.h"
#include "Engine/Math/Vector.h"
#include "Engine/Math/Matrix.h"

namespace Volition
{

// *** Quaternion ***
class VQuat
{
public:
    union
    {
        f32 C[4];
        struct
        {
            f32 Q0;
            VVector3 QV;
        };
        struct
        {
            /* NOTE(sean):
                We can't cast quaternion to 4d
                vector because of W on first place
            */
            f32 W, X, Y, Z;
        };
    };

public:
    VLN_FINLINE void Zero()
    {
        W = X = Y = Z = 0;
    }

    VLN_FINLINE void InitVec3(const VVector3* V)
    {
        W = 0;
        X = V->X;
        Y = V->Y;
        Z = V->Z;
    }

    VLN_FINLINE void Print()
    {
        VLN_LOG("<%f, %f, %f, %f>", W, X, Y, Z);
    }
};

// *** Parametric Line ***
class VParamLine3
{
public:
    VPoint3 P0, P1;
    VVector3 V; // P1 - P0 direction vector
};

// *** Plane ***
class VPlane3
{
public:
    VPoint3 P0; // Point to the plane
    VVector3 N; // It's normal vector
};

// *** Coordinates ***
class VPolar2
{
public:
    f32 R; // Radius
    f32 Theta; // Angle in radians
};

class VCylindrical3
{
public:
    f32 R; // Radius
    f32 Theta; // Angle about z-axis
    f32 Z; // z-height
};

class VSpherical3
{
public:
    f32 P; // Distance to the point from the origin
    f32 Theta; // X-Y
    f32 Phi; // Z-Y
};

}
