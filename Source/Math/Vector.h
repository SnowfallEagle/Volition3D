#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"

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

class VVector3D
{
public:
    union
    {
        f32 C[3];
        struct
        {
            f32 X, Y, Z;
        };
    };

public:
    FINLINE void Zero()
    {
        X = Y = Z = 0;
    }

    FINLINE void Print()
    {
        VL_LOG("<%f, %f, %f>", X, Y, Z);
    }
};

typedef VVector3D VPoint3D;

class VVector4D
{
public:
    union
    {
        f32 C[4];
        struct
        {
            f32 X, Y, Z, W;
        };
    };

public:
    FINLINE void Zero()
    {
        X = Y = Z = W = 0;
    }

    FINLINE void Print()
    {
        VL_LOG("<%f, %f, %f, %f>", X, Y, Z, W);
    }
};

typedef VVector4D VPoint4D;

