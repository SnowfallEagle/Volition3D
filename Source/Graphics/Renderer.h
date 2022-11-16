#pragma once

#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Math/Minimal.h"
#include "Graphics/Camera.h"

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

    // LocalToTrans or TransOnly
    void TransModelToWorld(const VPoint4D& WorldPos, ETransformType Type = ETransformType::LocalToTrans)
    {
        if (Type == ETransformType::LocalToTrans)
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
                    Poly->TransVtx[V] = Poly->LocalVtx[V] + WorldPos;
                }
            }
        }
        else // TransOnly
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
                    Poly->TransVtx[V] += WorldPos;
                }
            }
        }
    }

    void RemoveBackFaces(VVector4D CamPos)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace4DV1* Poly = PolyPtrList[I];

            if (~Poly->State & EPolyStateV1::Active ||
                Poly->State & EPolyStateV1::Clipped ||
                Poly->Attr & EPolyAttrV1::TwoSided ||
                Poly->State & EPolyStateV1::BackFace)
            {
                continue;
            }

            VVector4D U, V, N;
            U = Poly->TransVtx[1] - Poly->TransVtx[0];
            V = Poly->TransVtx[2] - Poly->TransVtx[0];
            VVector4D::Cross(U, V, N);

            VVector4D View = CamPos - Poly->TransVtx[0];
            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4D::Dot(View, N) <= 0.0f)
                Poly->State |= EPolyStateV1::BackFace;
        }
    }

    void TransWorldToCamera(const VMatrix44& MatCamera)
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
                VVector4D Res;
                VVector4D::MulMat44(Poly->TransVtx[V], MatCamera, Res);
                Poly->TransVtx[V] = Res;
            }
        }
    }

    void TransCameraToPerspective(const VCam4DV1& Cam)
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
                f32 Z = Poly->TransVtx[V].Z;

                Poly->TransVtx[V].X *= Cam.ViewDist / Z;
                Poly->TransVtx[V].Y *= Cam.AspectRatio * (Cam.ViewDist / Z);
                // Z = Z
            }
        }
    }

    void ConvertFromHomogeneous()
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
                Poly->TransVtx[V].DivByW();
            }
        }
    }

    void TransPerspectiveToScreen(const VCam4DV1& Cam)
    {
        f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

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
                Poly->TransVtx[V].X = Alpha + Poly->TransVtx[V].X * Alpha;
                Poly->TransVtx[V].Y = Beta - Poly->TransVtx[V].Y * Beta;
            }
        }
    }
};

