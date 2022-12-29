#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Memory.h"
#include "Core/DebugLog.h"

class VVector4;

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
    };

public:
    FINLINE void operator=(const VMatrix44& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }
    FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }

    void BuildTranslate(const VVector4& V);
    void BuildRotationXYZ(f32 X, f32 Y, f32 Z);
    static void Mul(const VMatrix44& A, const VMatrix44& B, VMatrix44& R);
    static b32 Inverse(const VMatrix44& A, VMatrix44& R);

    void Print()
    {
        VL_LOG("<\n");
        for (i32f Y = 0; Y < 4; ++Y)
        {
            VL_LOG("\t");
            for (i32f X = 0; X < 4; ++X)
            {
                VL_LOG("%f, ", C[Y][X]);
            }
            VL_LOG("\n");
        }
        VL_LOG(">\n");
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
    FINLINE void operator=(const VMatrix43& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
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
    FINLINE void operator=(const VMatrix14& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
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
    FINLINE void operator=(const VMatrix33& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
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
    FINLINE void operator=(const VMatrix32& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
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
    FINLINE void operator=(const VMatrix13& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
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
    FINLINE void operator=(const VMatrix22& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
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
    FINLINE void operator=(const VMatrix12& Mat)
    {
        Memory.MemCopy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));
    }
};

