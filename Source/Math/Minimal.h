#pragma once

#include "Math/Math.h"
#include "Math/Fixed16.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

/** NOTE(sean):
    Here some stuff that we don't
    put in header, but later we may will
 */

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

// *** Parametric Line ***
class VParamLine3D
{
public:
    VPoint3D P0, P1;
    VVector3D V; // P1 - P0 direction vector
};

// *** Plane ***
class VPlane3D
{
public:
    VPoint3D P0; // Point to the plane
    VVector3D N; // It's normal vector
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
    f32 P; // Distance to the point from the origin
    f32 Theta; // X-Y
    f32 Phi; // Z-Y
};
