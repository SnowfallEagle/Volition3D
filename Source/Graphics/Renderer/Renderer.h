#pragma once

#include "Core/Types.h"
#include "Math/Math.h"

namespace EPolyState
{
    enum
    {

    };
}

namespace EPolyAttr
{
    enum
    {

    };
}

class VPoly4DV1
{
public:
    i32 State;
    i32 Attr;
    i32 Color;

    VPoint4D* VtxList;
    i32 Vtx[3];
};

class VPolyFace4DV1 // Independent polygon face linked list
{
public:
    i32 State;
    i32 Attr;
    i32 Color;

    i32 Vtx[3];
    i32 TransVtx[3];

    VPolyFace4DV1* Prev;
    VPolyFace4DV1* Next;
};

class VObject4DV1
{
public:
    static constexpr i32f NameSize = 64;
    static constexpr i32f NumVtxInList = 64;
    static constexpr i32f NumPolyInList = 128;

public:
    int ID;
    char Name[NameSize];

    int State;
    int Attr;

    int AvgRadius;
    int MaxRadius;

    VPoint4D WorldPos;
    VVector4D Dir;
    VVector4D UX, UY, UZ; // Local axes to track full orientation

    i32 NumVtx;
    VPoint4D LocalVtxList[NumVtxInList];
    VPoint4D TransVtxList[NumVtxInList];

    i32 NumPoly;
    VPoly4DV1 PolyList[NumPolyInList];
};
