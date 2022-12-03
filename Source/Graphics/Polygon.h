#pragma once

#include "Core/Types.h"
#include "Math/Vector.h"

namespace EPolyStateV1
{
    enum
    {
        Active = BIT(1),
        Clipped = BIT(2),
        BackFace = BIT(3),
    };
}

namespace EPolyAttrV1
{
    enum
    {
        RGB32 = BIT(1),
        TwoSided = BIT(2),
        Transparent = BIT(3),

        ShadeModePure = BIT(4),
        ShadeModeFlat = BIT(5),
        ShadeModeGouraud = BIT(6),
        ShadeModePhong = BIT(7),
    };
}

class VPoly4DV1
{
public:
    u32 State;
    u32 Attr;
    u32 Color;

    VPoint4D* VtxList;
    i32 Vtx[3];
};

class VPolyFace4DV1
{
public:
    u32 State;
    u32 Attr;
    u32 Color;

    VPoint4D LocalVtx[3];
    VPoint4D TransVtx[3];

    VPolyFace4DV1* Prev;
    VPolyFace4DV1* Next;
};
