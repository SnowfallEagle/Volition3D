#pragma once

#include <string.h>
#include "Core/Types.h"
#include "Core/Platform.h"

class VMatrix44
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

class VMatrix43
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

class VMatrix14
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

// 3x3
class VMatrix33
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

class VMatrix32
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

class VMatrix13
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

class VMatrix22
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

class VMatrix12
{
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
        memcpy(this, &Mat, sizeof(*this));
    }

    FINLINE void Zero()
    {
        memset(this, 0, sizeof(*this));
    }
};

// Indentity matrices
extern const VMatrix44 IdentityMatrix44;
extern const VMatrix43 IdentityMatrix43;
extern const VMatrix33 IdentityMatrix33;
extern const VMatrix32 IdentityMatrix32;
extern const VMatrix22 IdentityMatrix22;

// TODO(sean): Transpose, ColumnSwap, Print functions for matrices

