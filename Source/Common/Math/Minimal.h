#pragma once

#include "Common/Math/Math.h"
#include "Common/Math/Fixed16.h"
#include "Common/Math/Rect.h"
#include "Common/Math/Vector.h"
#include "Common/Math/Matrix.h"

namespace Volition
{

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
            /**
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

class VParamLine3
{
public:
    VPoint3 P0, P1;
    VVector3 V;
};

class VPlane3
{
public:
    VPoint3 P0;
    VVector3 N;
};

class VPolar2
{
public:
    f32 R;     /** Radius */
    f32 Theta; /** Angle in radians */
};

class VCylindrical3
{
public:
    f32 R;     /** Radius */
    f32 Theta; /** Angle about z-axis */
    f32 Z;     /** Z height */
};

class VSpherical3
{
public:
    f32 P;     /** Distance to the point from the origin */
    f32 Theta; /** X-Y */
    f32 Phi;   /** Z-Y */
};

}
