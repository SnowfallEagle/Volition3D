#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Math/Math.h"

class VMatrix44;

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

    FINLINE f32 GetLength()
    {
        return Math.Sqrt(X*X + Y*Y + Z*Z);
    }
    FINLINE f32 GetLengthFast()
    {
        return Math.FastDist3D(X, Y, Z);
    }

    void Normalize()
    {
        f32 Len = GetLength();

        // Don't do anything on zero vector
        if (Len < Math.Epsilon5)
            return;

        f32 Inv = 1.0f / Len;
        X *= Inv;
        Y *= Inv;
        Z *= Inv;
    }
    VVector3D GetNormalized()
    {
        f32 Len = GetLength();

        if (Len < Math.Epsilon5)
            return { 0.0f, 0.0f, 0.0f };

        f32 Inv = 1.0f / Len;
        return { X * Inv, Y * Inv, Z * Inv };
    }

    static VVector3D GetCross(const VVector3D& A, const VVector3D& B)
    {
        return {
            A.Y * B.Z - A.Z * B.Y,
            -(A.X * B.Z - A.Z * B.X),
            A.X * B.Y - A.Y * B.X
        };
    }
    static void Cross(const VVector3D& A, const VVector3D& B, VVector3D& R)
    {
        R = {
            A.Y * B.Z - A.Z * B.Y,
            -(A.X * B.Z - A.Z * B.X),
            A.X * B.Y - A.Y * B.X
        };
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
    // TODO(sean): Should they be force inlined?
    VVector4D operator+(const VVector4D& V)
    {
        return {
            X + V.X, Y + V.Y, Z + V.Z, 1.0f
        };
    }
    VVector4D& operator+=(const VVector4D& V)
    {
        X += V.X;
        Y += V.Y;
        Z += V.Z;
        W = 1.0f;
        return *this;
    }
    VVector4D operator-(const VVector4D& V)
    {
        return {
            X - V.X, Y - V.Y, Z - V.Z, 1.0f
        };
    }
    VVector4D& operator-=(const VVector4D& V)
    {
        X -= V.X;
        Y -= V.Y;
        Z -= V.Z;
        W = 1.0f;
        return *this;
    }

    FINLINE void Zero()
    {
        X = Y = Z = W = 0;
    }

    FINLINE void Print()
    {
        VL_LOG("<%f, %f, %f, %f>", X, Y, Z, W);
    }

    FINLINE void DivByW()
    {
        X /= W;
        Y /= W;
        Z /= W;
    }

    FINLINE f32 GetLength()
    {
        return Math.Sqrt(X*X + Y*Y + Z*Z);
    }
    FINLINE f32 GetLengthFast()
    {
        return Math.FastDist3D(X, Y, Z);
    }

    void Normalize()
    {
        f32 Len = GetLength();

        // Don't do anything on zero vector
        if (Len < Math.Epsilon5)
            return;

        f32 Inv = 1.0f / Len;
        X *= Inv;
        Y *= Inv;
        Z *= Inv;
        W = 1.0f;
    }
    VVector4D GetNormalized()
    {
        f32 Len = GetLength();

        if (Len < Math.Epsilon5)
            return { 0.0f, 0.0f, 0.0f, 1.0f };

        f32 Inv = 1.0f / Len;
        return { X * Inv, Y * Inv, Z * Inv, 1.0f };
    }

    static f32 Dot(const VVector4D& A, const VVector4D& B)
    {
        return A.X*B.X + A.Y*B.Y + A.Z+B.Z;
    }

    static VVector4D GetCross(const VVector4D& A, const VVector4D& B)
    {
        return {
            A.Y * B.Z - A.Z * B.Y,
            -(A.X * B.Z - A.Z * B.X),
            A.X * B.Y - A.Y * B.X,
            1.0f
        };
    }
    static void Cross(const VVector4D& A, const VVector4D& B, VVector4D& R)
    {
        R = {
            A.Y * B.Z - A.Z * B.Y,
            -(A.X * B.Z - A.Z * B.X),
            A.X * B.Y - A.Y * B.X,
            1.0f
        };
    }

    static void MulMat44(const VVector4D& A, const VMatrix44& M, VVector4D& R);
};

typedef VVector4D VPoint4D;
