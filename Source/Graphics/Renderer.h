#pragma once

#include <stdio.h>
#include <string.h>
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Math/Minimal.h"

// Polygon /////////////////////////////////
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

    VPoint4D LocalVtx[3];
    VPoint4D TransVtx[3];

    VPolyFace4DV1* Prev; // ?
    VPolyFace4DV1* Next; // ?
};

// Renderer ////////////////////////////////////////////////

enum class ETransformType
{
    LocalOnly = 0,
    TransOnly,
    LocalToTrans,
};

class VRenderList4DV1
{
public:
    static constexpr i32f MaxPoly = 1024;

//private:
public:
    i32 NumPoly;
    VPolyFace4DV1* PolyPtrList[MaxPoly];
    VPolyFace4DV1 PolyList[MaxPoly];

public:
    void Reset()
    {
        NumPoly = 0;
    }

    void Transform(const VMatrix44& M, ETransformType Type)
    {
        VVector4D Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4D::MulMat44(Poly->LocalVtx[V], M, Res);
                    Poly->LocalVtx[V] = Res;
                }
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4D::MulMat44(Poly->TransVtx[V], M, Res);
                    Poly->TransVtx[V] = Res;
                }
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4D::MulMat44(Poly->LocalVtx[V], M, Res);
                    Poly->TransVtx[V] = Res;
                }
            }
        } break;
        }
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


    void Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis)
    {
        VVector4D Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4D::MulMat44(LocalVtxList[I], M, Res);
                LocalVtxList[I] = Res;
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4D::MulMat44(TransVtxList[I], M, Res);
                TransVtxList[I] = Res;
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4D::MulMat44(LocalVtxList[I], M, Res);
                TransVtxList[I] = Res;
            }
        } break;
        }

        if (bTransBasis)
        {
            // TODO(sean)
        }
    }
};
