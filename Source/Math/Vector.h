#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/DebugLog.h"
#include "Math/Math.h"

class VMatrix44;

template<class T>
class TVector2
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
        {
            VL_LOG("<%f, %f>", X, Y);
        }
        else if (T == i32)
        {
            VL_LOG("<%d, %d>", X, Y);
        }
        else
        {
            VL_LOG("<Unknown type>");
        }
    }
};

typedef TVector2<f32> VVector2, VPoint2;
typedef TVector2<i32> VVector2I, VPoint2I;

class VVector3
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
    VVector3 GetNormalized()
    {
        f32 Len = GetLength();

        if (Len < Math.Epsilon5)
            return { 0.0f, 0.0f, 0.0f };

        f32 Inv = 1.0f / Len;
        return { X * Inv, Y * Inv, Z * Inv };
    }

    FINLINE static VVector3 GetCross(const VVector3& A, const VVector3& B)
    {
        return {
            (A.Y * B.Z)   - (A.Z * B.Y),
            -((A.X * B.Z) - (A.Z * B.X)),
            (A.X * B.Y)   - (A.Y * B.X)
        };
    }
    FINLINE static void Cross(const VVector3& A, const VVector3& B, VVector3& R)
    {
        R = {
            (A.Y * B.Z)   - (A.Z * B.Y),
            -((A.X * B.Z) - (A.Z * B.X)),
            (A.X * B.Y)   - (A.Y * B.X)
        };
    }

    FINLINE void Print()
    {
        VL_LOG("<%f, %f, %f>", X, Y, Z);
    }
};

typedef VVector3 VPoint3;

class VVector4
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
    VVector4() = default;
    VVector4(f32 InX, f32 InY, f32 InZ, f32 InW = 1.0f)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }

    FINLINE VVector4 operator+(const VVector4& V) const
    {
        return {
            X + V.X, Y + V.Y, Z + V.Z, 1.0f
        };
    }
    FINLINE VVector4& operator+=(const VVector4& V)
    {
        X += V.X;
        Y += V.Y;
        Z += V.Z;
        W = 1.0f;
        return *this;
    }
    FINLINE VVector4 operator-(const VVector4& V) const
    {
        return {
            X - V.X, Y - V.Y, Z - V.Z, 1.0f
        };
    }
    FINLINE VVector4& operator-=(const VVector4& V)
    {
        X -= V.X;
        Y -= V.Y;
        Z -= V.Z;
        W = 1.0f;
        return *this;
    }
    FINLINE VVector4 operator*(f32 Scalar) const
    {
        return {
            X * Scalar,
            Y * Scalar,
            Z * Scalar,
            1.0f
        };
    }

    FINLINE void Zero()
    {
        X = Y = Z = 0.0f;
        W = 1.0f;
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

    FINLINE f32 GetLength() const
    {
        return Math.Sqrt(X*X + Y*Y + Z*Z);
    }
    FINLINE f32 GetLengthFast() const
    {
        return Math.FastDist3D(X, Y, Z);
    }

    void Normalize()
    {
        f32 Len = GetLength();

        // Don't do anything on zero vector
        if (Len < Math.Epsilon5)
        {
            return;
        }

        f32 Inv = 1.0f / Len;
        X *= Inv;
        Y *= Inv;
        Z *= Inv;
        W = 1.0f;
    }
    VVector4 GetNormalized() const
    {
        f32 Len = GetLength();

        if (Len < Math.Epsilon5)
        {
            return { 0.0f, 0.0f, 0.0f, 1.0f };
        }

        f32 Inv = 1.0f / Len;
        return { X * Inv, Y * Inv, Z * Inv, 1.0f };
    }

    FINLINE static f32 Dot(const VVector4& A, const VVector4& B)
    {
        return A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    }

    FINLINE static VVector4 GetCross(const VVector4& A, const VVector4& B)
    {
        return {
            A.Y * B.Z - A.Z * B.Y,
            -(A.X * B.Z - A.Z * B.X),
            A.X * B.Y - A.Y * B.X,
            1.0f
        };
    }
    FINLINE static void Cross(const VVector4& A, const VVector4& B, VVector4& R)
    {
        R = {
            A.Y * B.Z - A.Z * B.Y,
            -(A.X * B.Z - A.Z * B.X),
            A.X * B.Y - A.Y * B.X,
            1.0f
        };
    }

    static void MulMat44(const VVector4& A, const VMatrix44& M, VVector4& R);
};

typedef VVector4 VPoint4;

