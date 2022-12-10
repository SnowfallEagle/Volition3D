#pragma once

#include "Math/Minimal.h"
#include "Graphics/Polygon.h"
#include "Graphics/Camera.h"
#include "Graphics/TransformType.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"

class VRenderList4DV1
{
public:
    static constexpr i32f MaxPoly = 1024;

public:
    i32 NumPoly;
    VPolyFace4DV1* PolyPtrList[MaxPoly];
    VPolyFace4DV1 PolyList[MaxPoly];

public:
    b32 InsertPoly(const VPoly4DV1& Poly)
    {
        if (NumPoly >= MaxPoly)
        {
            return false;
        }

        PolyPtrList[NumPoly] = &PolyList[NumPoly];
        PolyList[NumPoly].State = Poly.State;
        PolyList[NumPoly].Attr = Poly.Attr;
        PolyList[NumPoly].Color = Poly.Color;

        for (i32f I = 0; I < 3; ++I)
        {
            // Copy in PolyFace's trans and local vertices lists
            PolyList[NumPoly].TransVtx[I] = PolyList[NumPoly].LocalVtx[I] = Poly.VtxList[Poly.Vtx[I]];
        }

        PolyList[NumPoly].Next = nullptr;
        if (NumPoly == 0)
        {
            PolyList[NumPoly].Prev = nullptr;
        }
        else
        {
            PolyList[NumPoly].Prev = &PolyList[NumPoly - 1];
            PolyList[NumPoly - 1].Next = &PolyList[NumPoly];
        }

        ++NumPoly;

        return true;
    }

    b32 InsertPolyFace(const VPolyFace4DV1& PolyFace)
    {
        if (NumPoly >= MaxPoly)
        {
            return false;
        }

        PolyPtrList[NumPoly] = &PolyList[NumPoly];
        memcpy(&PolyList[NumPoly], &PolyFace, sizeof(VPolyFace4DV1));

        PolyList[NumPoly].Next = nullptr;
        if (NumPoly == 0)
        {
            PolyList[NumPoly].Prev = nullptr;
        }
        else
        {
            PolyList[NumPoly].Prev = &PolyList[NumPoly - 1];
            PolyList[NumPoly - 1].Next = &PolyList[NumPoly];
        }

        ++NumPoly;

        return true;
    }

    void InsertObject(VObject4DV1& Object, b32 bInsertLocal)
    {
        if (~Object.State & EObjectStateV1::Active  ||
            ~Object.State & EObjectStateV1::Visible ||
            Object.State & EObjectStateV1::Culled)
        {
            return;
        }

        for (i32f I = 0; I < Object.NumPoly; ++I)
        {
            VPoly4DV1& Poly = Object.PolyList[I];

            if (~Poly.State & EPolyStateV1::Active ||
                Poly.State & EPolyStateV1::Clipped ||
                Poly.State & EPolyStateV1::BackFace)
            {
                continue;
            }

            VPoint4D* OldVtxList = Poly.VtxList;
            Poly.VtxList = bInsertLocal ? Object.LocalVtxList : Object.TransVtxList;
            b32 bRes = InsertPoly(Poly);
            Poly.VtxList = OldVtxList;

            if (!bRes)
            {
                return;
            }
        }
    }

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

    void RemoveBackFaces(const VCam4DV1& Cam)
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

            VVector4D View = Cam.Pos - Poly->TransVtx[0];
            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4D::Dot(View, N) <= 0.0f)
            {
                Poly->State |= EPolyStateV1::BackFace;
            }
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
                f32 ViewDistDivZ = Cam.ViewDist / Poly->TransVtx[V].Z;

                Poly->TransVtx[V].X *= ViewDistDivZ;
                Poly->TransVtx[V].Y *= ViewDistDivZ;
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
                Poly->TransVtx[V].X = Poly->TransVtx[V].X + Alpha;
                Poly->TransVtx[V].Y = -Poly->TransVtx[V].Y + Beta;
            }
        }
    }

    void TransCameraToScreen(const VCam4DV1& Cam)
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
                f32 ViewDistDivZ = Cam.ViewDist / Poly->TransVtx[V].Z;

                Poly->TransVtx[V].X = Poly->TransVtx[V].X * ViewDistDivZ;
                Poly->TransVtx[V].Y = Poly->TransVtx[V].Y * ViewDistDivZ;

                Poly->TransVtx[V].X = Poly->TransVtx[V].X + Alpha;
                Poly->TransVtx[V].Y = -Poly->TransVtx[V].Y + Beta;
            }
        }
    }

    void RenderWire(u32* Buffer, i32 Pitch)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace4DV1* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyStateV1::Active ||
                Poly->State & EPolyStateV1::BackFace ||
                Poly->State & EPolyStateV1::Clipped)
            {
                continue;
            }

            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
                (i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
                Poly->Color
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
                (i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
                Poly->Color
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
                (i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
                Poly->Color
            );
        }
    }

    void RenderSolid(u32* Buffer, i32 Pitch)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace4DV1* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyStateV1::Active ||
                Poly->State & EPolyStateV1::BackFace ||
                Poly->State & EPolyStateV1::Clipped)
            {
                continue;
            }

            Renderer.DrawTriangle(
                Buffer, Pitch,
                (i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
                (i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
                (i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
                Poly->Color
            );
        }
    }
};
