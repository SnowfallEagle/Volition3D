#pragma once

#include "Engine/Core/DebugLog.h"
#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"
#include "Common/Platform/Memory.h"
#include "Common/Math/Vector.h"

namespace Volition
{

class VMatrix44
{
public:
    static const VMatrix44 Identity;

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

        VVector4 RowV[4];

        __m128 RowM[4];
    };

public:
    VLN_FINLINE void operator=(const VMatrix44& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }

    VLN_FINLINE void BuildTranslate(const VVector4& V)
    {
        *this = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            V.X, V.Y, V.Z, 1
        };
    }

    void BuildRotationXYZ(f32 X, f32 Y, f32 Z)
    {
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

    VLN_FINLINE static void MulVecMat(const VVector4& A, const VMatrix44& M, VVector4& R)
    {
#if VLN_SSE
        const __m128 AX = _mm_shuffle_ps(A.MC, A.MC, 0x00);
        const __m128 AY = _mm_shuffle_ps(A.MC, A.MC, 0x55);
        const __m128 AZ = _mm_shuffle_ps(A.MC, A.MC, 0xAA);
        const __m128 AW = _mm_shuffle_ps(A.MC, A.MC, 0xFF);

        __m128 RM = _mm_mul_ps(AX, M.RowM[0]);
        RM = _mm_add_ps(RM, _mm_mul_ps(AY, M.RowM[1]));
        RM = _mm_add_ps(RM, _mm_mul_ps(AZ, M.RowM[2]));
        RM = _mm_add_ps(RM, _mm_mul_ps(AW, M.RowM[3]));

        R.MC = RM;
#else
        for (i32f X = 0; X < 4; ++X)
        {
            f32 C = 0.0f;

            for (i32f Y = 0; Y < 4; ++Y)
            {
                C += A.C[Y] * M.C[Y][X];
            }

            R.C[X] = C;
        }
#endif // VLN_SSE
    }

    VLN_FINLINE static void Mul(const VMatrix44& A, const VMatrix44& B, VMatrix44& R)
    {
#ifdef VLN_SSE
        MulVecMat(A.RowV[0], B, R.RowV[0]);
        MulVecMat(A.RowV[1], B, R.RowV[1]);
        MulVecMat(A.RowV[2], B, R.RowV[2]);
        MulVecMat(A.RowV[3], B, R.RowV[3]);
#else
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
#endif // VLN_SSE
    }

    static b32 Inverse(const VMatrix44& A, VMatrix44& R)
    {
        /* Assumes that the last column is
            [ 0 ]
            [ 0 ]
            [ 0 ]
            [ 1 ]
         */

        // Find 3x3 det
        const f32 Det =
            A.C00 * (A.C11 * A.C22 - A.C12 * A.C21) -
            A.C01 * (A.C10 * A.C22 - A.C12 * A.C20) +
            A.C02 * (A.C10 * A.C21 - A.C11 * A.C20);

        // Test if we can't inverse
        if (Math.Abs(Det) < Math.Epsilon5)
        {
            return false;
        }

        const f32 InvDet = 1.0f / Det;

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

    void Print()
    {
        VLN_LOG_VERBOSE("<\n");
        for (i32f Y = 0; Y < 4; ++Y)
        {
            VLN_LOG_VERBOSE("\t");
            for (i32f X = 0; X < 4; ++X)
            {
                VLN_LOG_VERBOSE("%f, ", C[Y][X]);
            }
            VLN_LOG_VERBOSE("\n");
        }
        VLN_LOG_VERBOSE(">\n");
    }
};

inline const VMatrix44 VMatrix44::Identity = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

class VMatrix43
{
public:
    static const VMatrix43 Identity;

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
    VLN_FINLINE void operator=(const VMatrix43& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

inline const VMatrix43 VMatrix43::Identity = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
    0, 0, 0,
};

class VMatrix14
{
public:
    static const VMatrix14 Identity;

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
    VLN_FINLINE void operator=(const VMatrix14& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

inline const VMatrix14 VMatrix14::Identity = {
    1, 0, 0, 0
};

// 3x3
class VMatrix33
{
public:
    static const VMatrix33 Identity;

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
    VLN_FINLINE void operator=(const VMatrix33& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

inline const VMatrix33 VMatrix33::Identity = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
};

class VMatrix32
{
public:
    static const VMatrix32 Identity;

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
    VLN_FINLINE void operator=(const VMatrix32& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

inline const VMatrix32 VMatrix32::Identity = {
    1, 0,
    0, 1,
    0, 0,
};

class VMatrix13
{
public:
    static const VMatrix13 Identity;

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
    VLN_FINLINE void operator=(const VMatrix13& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

inline const VMatrix13 VMatrix13::Identity = {
    1, 0, 0
};

class VMatrix22
{
public:
    static const VMatrix22 Identity;

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
    VLN_FINLINE void operator=(const VMatrix22& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

inline const VMatrix22 VMatrix22::Identity = {
    1, 0,
    0, 1,
};

class VMatrix12
{
public:
    static const VMatrix12 Identity;

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
    VLN_FINLINE void operator=(const VMatrix12& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    VLN_FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

inline const VMatrix12 VMatrix12::Identity = {
    1, 0,
};


}
