#include "Math/Math.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

const VMatrix44 VMatrix44::Identity = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

const VMatrix43 VMatrix43::Identity = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
    0, 0, 0,
};

const VMatrix33 VMatrix33::Identity = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
};

const VMatrix32 VMatrix32::Identity = {
    1, 0,
    0, 1,
    0, 0,
};

const VMatrix22 VMatrix22::Identity = {
    1, 0,
    0, 1,
};

void VMatrix44::BuildTranslate(const VVector4D& V)
{
    *this = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        V.X, V.Y, V.Z, 1
    };
}

void VMatrix44::BuildRotationXYZ(f32 X, f32 Y, f32 Z)
{
    // NOTE(sean):
    // We can optimize this function by checking
    // special conditions with switch case

    VMatrix44 MatX, MatY, MatZ, MatTemp;
    f32 SinAngle, CosAngle;

    MatX = Identity;
    MatY = Identity;
    MatZ = Identity;

    if (Math.Abs(X) > Math.Epsilon5)
    {
        SinAngle = Math.FastSin(X);
        CosAngle = Math.FastCos(X);

        MatX = {
            1, 0, 0, 0,
            0, CosAngle, SinAngle, 0,
            0, -SinAngle, CosAngle, 0,
            0, 0, 0, 1
        };
    }

    if (Math.Abs(Y) > Math.Epsilon5)
    {
        SinAngle = Math.FastSin(Y);
        CosAngle = Math.FastCos(Y);

        MatY = {
            CosAngle, 0, -SinAngle, 0,
            0,        1, 0,         0,
            SinAngle, 0, CosAngle,  0,
            0,        0, 0,         1
        };
    }

    if (Math.Abs(Z) > Math.Epsilon5)
    {
        SinAngle = Math.FastSin(Z);
        CosAngle = Math.FastCos(Z);

        MatZ = {
            CosAngle,  SinAngle, 0, 0,
            -SinAngle, CosAngle, 0, 0,
            0,         0,        1, 0,
            0,         0,        0, 1
        };
    }

    VMatrix44::Mul(MatX, MatY, MatTemp);
    VMatrix44::Mul(MatTemp, MatZ, *this);
}

void VMatrix44::Mul(const VMatrix44& A, const VMatrix44& B, VMatrix44& R)
{
    for (i32f Row = 0; Row < 4; ++Row)
    {
        for (i32f Col = 0; Col < 4; ++Col)
        {
            f32 C = 0.0f;
            for (i32f I = 0; I < 4; ++I)
            {
                C += A.C[Row][I] * B.C[I][Col];
            }
            R.C[Row][Col] = C;
        }
    }
}

b32 VMatrix44::Inverse(const VMatrix44& A, VMatrix44& R)
{
    /* Assumes that the last column is
        [ 0 ]
        [ 0 ]
        [ 0 ]
        [ 1 ]
     */

    // Find 3x3 det
    f32 Det =
        A.C00 * (A.C11 * A.C22 - A.C12 * A.C21) -
        A.C01 * (A.C10 * A.C22 - A.C12 * A.C20) +
        A.C02 * (A.C10 * A.C21 - A.C11 * A.C20);

    // Test if we can't inverse
    if (Math.Abs(Det) < Math.Epsilon5)
        return false;

    f32 InvDet = 1.0f / Det;

    R.C00 =  InvDet * (A.C11 * A.C22 - A.C12 * A.C21);
    R.C01 = -InvDet * (A.C01 * A.C22 - A.C02 * A.C21);
    R.C02 =  InvDet * (A.C01 * A.C12 - A.C02 * A.C11);
    R.C03 = 0.0f;

    R.C10 = -InvDet * (A.C10 * A.C22 - A.C12 * A.C20);
    R.C11 =  InvDet * (A.C00 * A.C22 - A.C02 * A.C20);
    R.C12 = -InvDet * (A.C00 * A.C12 - A.C02 * A.C10);
    R.C13 = 0.0f;

    R.C20 =  InvDet * (A.C10 * A.C21 - A.C11 * A.C20);
    R.C21 = -InvDet * (A.C00 * A.C21 - A.C01 * A.C20);
    R.C22 =  InvDet * (A.C00 * A.C11 - A.C01 * A.C10);
    R.C23 = 0.0f;

    R.C30 = -(A.C30 * R.C00 + A.C31 * R.C10 + A.C32 * R.C20);
    R.C31 = -(A.C30 * R.C01 + A.C31 * R.C11 + A.C32 * R.C21);
    R.C32 = -(A.C30 * R.C02 + A.C31 * R.C12 + A.C32 * R.C22);
    R.C33 = 1.0f;

    // Success
    return true;
}
