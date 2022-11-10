#pragma once

#include <stdio.h>
#include <string.h>
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Math/Math.h"

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

class VPolyFace4DV1 // Independent polygon face linked list
{
public:
    u32 State;
    u32 Attr;
    u32 Color;

    VPoint4D Vtx[3];
    VPoint4D TransVtx[3];

    VPolyFace4DV1* Prev; // ?
    VPolyFace4DV1* Next; // ?
};

class VRenderList4DV1
{
public:
    static constexpr i32f MaxPoly = 1024;

private:
    i32 NumPoly;
    VPolyFace4DV1* PolyPtrList[MaxPoly];
    VPolyFace4DV1 PolyList[MaxPoly];

public:
    void Reset()
    {
        NumPoly = 0;
    }
};

// Camera /////////////////////////////////////////////////
namespace ECamV1State
{
    enum
    {

    };
}

namespace ECamV1Attr
{
    enum
    {
        Euler = BIT(1),
        UVN = BIT(2)
    };
}

class VCam4DV1
{
public:
    u32 State;
    u32 Attr;

    VPoint4D Pos;
    VVector4D Dir; // Euler angles or look at direction

    VVector4D U, V, N;
    VPoint4D Target;

    VVector2D ViewDist; // X-Horizontal, Y-Vertical
    f32 FOV;

    f32 ZNearClip;
    f32 ZFarClip;

    VPlane3D LeftClipPlane;
    VPlane3D RightClipPlane;
    VPlane3D TopClipPlane;
    VPlane3D BottomClipPlane;

    VVector2D ViewPlaneSize;
    VVector2D ViewPortSize;
    VVector2D ViewPortCenter;

    f32 AspectRatio;

    VMatrix44 MatCamera;      // World->Camera
    VMatrix44 MatPerspective; // Camera->Perspective
    VMatrix44 MatScreen;      // Perspective->Screen
};

// Object ////////////////////////////////////////////
namespace EObjectStateV1
{
    enum
    {
        Active = BIT(1),
        Visible = BIT(2),
    };
}

namespace EObjectAttrV1
{
    enum
    {
    };
}

namespace EPLX
{
    enum
    {
        RGBFlag = BIT(16),

        ShadeModeMask = BIT(15) | BIT(14),
        ShadeModePureFlag = 0,
        ShadeModeFlatFlag = BIT(14),
        ShadeModeGouraudFlag = BIT(15),
        ShadeModePhongFlag = BIT(14) | BIT(15),

        TwoSidedFlag = BIT(13),
        RGB16Mask = 0x0FFF,
        RGB8Mask = 0x00FF,
    };
}

DEFINE_LOG_CHANNEL(hObjectV1Log, "ObjectV1");

class VObject4DV1
{
public:
    static constexpr i32f NameSize = 64;
    static constexpr i32f MaxVtx = 64;
    static constexpr i32f MaxPoly = 128;

public:
    i32 ID;
    char Name[NameSize];

    u32 State;
    u32 Attr;

    i32 AvgRadius;
    i32 MaxRadius;

    VPoint4D WorldPos;
    VVector4D Dir;
    VVector4D UX, UY, UZ; // Local axes to track full orientation

    i32 NumVtx;
    VPoint4D LocalVtxList[MaxVtx];
    VPoint4D TransVtxList[MaxVtx];

    i32 NumPoly;
    VPoly4DV1 PolyList[MaxPoly];

public:
    b32 LoadPLG(
        const char* Path,
        const VVector4D& Pos,
        const VVector4D& Scale,
        const VVector4D& Rot
    );

    static char* GetLinePLG(FILE* File, char* Buffer, i32 Size)
    {
        for (;;)
        {
            if (!fgets(Buffer, Size, File))
                return nullptr;

            i32f I, Len = strlen(Buffer);
            for (I = 0; I < Len && Buffer[I] == ' ' && Buffer[I] == '\t'; ++I)
                {}

            if (I < Len && Buffer[I] != '#' && Buffer[I] != '\r' && Buffer[I] != '\n')
                return &Buffer[I];
        }
    }

    void ComputeRadius()
    {
        // TODO(sean)
    }
};
