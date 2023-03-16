#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/Memory.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Math/Vector.h"

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

    void BuildRotationXYZ(f32 X, f32 Y, f32 Z);

    VLN_FINLINE static void MulVecMat(const VVector4& A, const VMatrix44& M, VVector4& R)
    {
#if VLN_SSE
        __m128 AX = _mm_shuffle_ps(A.MC, A.MC, 0x00);
        __m128 AY = _mm_shuffle_ps(A.MC, A.MC, 0x55);
        __m128 AZ = _mm_shuffle_ps(A.MC, A.MC, 0xAA);
        __m128 AW = _mm_shuffle_ps(A.MC, A.MC, 0xFF);

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

    static b32 Inverse(const VMatrix44& A, VMatrix44& R);

    void Print()
    {
        VLN_LOG("<\n");
        for (i32f Y = 0; Y < 4; ++Y)
        {
            VLN_LOG("\t");
            for (i32f X = 0; X < 4; ++X)
            {
                VLN_LOG("%f, ", C[Y][X]);
            }
            VLN_LOG("\n");
        }
        VLN_LOG(">\n");
    }
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

}
