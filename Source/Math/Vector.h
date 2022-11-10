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

