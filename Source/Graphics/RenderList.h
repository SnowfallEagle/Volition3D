#pragma once

#include <stdlib.h> // qsort()
#include "Math/Minimal.h"
#include "Graphics/Polygon.h"
#include "Graphics/Camera.h"
#include "Graphics/TransformType.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"

enum class ESortPolygonsMethod
{
    Average = 0,
    Near,
    Far
};

class VRenderList
{
public:
    static constexpr i32f MaxPoly = 1024;

public:
    i32 NumPoly;
    VPolyFace* PolyPtrList[MaxPoly];
    VPolyFace PolyList[MaxPoly];

public:
    b32 InsertPoly(const VPoly& Poly)
    {
        if (NumPoly >= MaxPoly)
        {
            return false;
        }

        PolyPtrList[NumPoly] = &PolyList[NumPoly];
        PolyList[NumPoly].State = Poly.State;
        PolyList[NumPoly].Attr = Poly.Attr;
        PolyList[NumPoly].Color = Poly.FinalColor;

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

    b32 InsertPolyFace(const VPolyFace& PolyFace)
    {
        if (NumPoly >= MaxPoly)
        {
            return false;
        }

        PolyPtrList[NumPoly] = &PolyList[NumPoly];
        memcpy(&PolyList[NumPoly], &PolyFace, sizeof(VPolyFace));

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

    void InsertObject(VObject& Object, b32 bInsertLocal)
    {
        if (~Object.State & EObjectState::Active  ||
            ~Object.State & EObjectState::Visible ||
            Object.State & EObjectState::Culled)
        {
            return;
        }

        for (i32f I = 0; I < Object.NumPoly; ++I)
        {
            VPoly& Poly = Object.PolyList[I];

            if (~Poly.State & EPolyState::Active ||
                Poly.State & EPolyState::Clipped ||
                Poly.State & EPolyState::BackFace)
            {
                continue;
            }

            VPoint4* OldVtxList = Poly.VtxList;
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
        VVector4 Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyState::Active ||
                    Poly->State & EPolyState::Clipped ||
                    Poly->State & EPolyState::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4::MulMat44(Poly->LocalVtx[V], M, Res);
                    Poly->LocalVtx[V] = Res;
                }
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyState::Active ||
                    Poly->State & EPolyState::Clipped ||
                    Poly->State & EPolyState::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4::MulMat44(Poly->TransVtx[V], M, Res);
                    Poly->TransVtx[V] = Res;
                }
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyState::Active ||
                    Poly->State & EPolyState::Clipped ||
                    Poly->State & EPolyState::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4::MulMat44(Poly->LocalVtx[V], M, Res);
                    Poly->TransVtx[V] = Res;
                }
            }
        } break;
        }
    }

    // LocalToTrans or TransOnly
    void TransformModelToWorld(const VPoint4& WorldPos, ETransformType Type = ETransformType::LocalToTrans)
    {
        if (Type == ETransformType::LocalToTrans)
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyState::Active ||
                    Poly->State & EPolyState::Clipped ||
                    Poly->State & EPolyState::BackFace)
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
                VPolyFace* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyState::Active ||
                    Poly->State & EPolyState::Clipped ||
                    Poly->State & EPolyState::BackFace)
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

    void RemoveBackFaces(const VCamera& Cam)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace* Poly = PolyPtrList[I];

            if (~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->Attr & EPolyAttr::TwoSided ||
                Poly->State & EPolyState::BackFace)
            {
                continue;
            }

            VVector4 U, V, N;
            U = Poly->TransVtx[1] - Poly->TransVtx[0];
            V = Poly->TransVtx[2] - Poly->TransVtx[0];

            VVector4::Cross(U, V, N);
            VVector4 View = Cam.Pos - Poly->TransVtx[0];

            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4::Dot(View, N) <= 0.0f)
            {
                Poly->State |= EPolyState::BackFace;
            }
        }
    }

    void TransformWorldToCamera(const VCamera& Camera)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::BackFace)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                VVector4 Res;
                VVector4::MulMat44(Poly->TransVtx[V], Camera.MatCamera, Res);
                Poly->TransVtx[V] = Res;
            }
        }
    }

    static i32 SortPolygonsCompareAverage(const void* Arg1, const void* Arg2)
    {
        const VPolyFace* Poly1 = *(const VPolyFace**)Arg1;
        const VPolyFace* Poly2 = *(const VPolyFace**)Arg2;

        f32 Z1 = 0.33333f * (Poly1->TransVtx[0].Z + Poly1->TransVtx[1].Z + Poly1->TransVtx[2].Z);
        f32 Z2 = 0.33333f * (Poly2->TransVtx[0].Z + Poly2->TransVtx[1].Z + Poly2->TransVtx[2].Z);

        if (Z1 < Z2)
        {
            return 1;
        }
        else if (Z1 > Z2)
        {
            return -1;
        }
        else
        {
            return 0;
        }

        return 1;
    }

    static i32 SortPolygonsCompareNear(const void* Arg1, const void* Arg2)
    {
        const VPolyFace* Poly1 = *(const VPolyFace**)Arg1;
        const VPolyFace* Poly2 = *(const VPolyFace**)Arg2;

        f32 ZMin1 = MIN(MIN(Poly1->TransVtx[0].Z, Poly1->TransVtx[1].Z), Poly1->TransVtx[2].Z);
        f32 ZMin2 = MIN(MIN(Poly2->TransVtx[0].Z, Poly2->TransVtx[1].Z), Poly2->TransVtx[2].Z);

        if (ZMin1 < ZMin2)
        {
            return 1;
        }
        else if (ZMin1 > ZMin2)
        {
            return -1;
        }
        else
        {
            return 0;
        }

        return 1;
    }

    static i32 SortPolygonsCompareFar(const void* Arg1, const void* Arg2)
    {
        const VPolyFace* Poly1 = *(const VPolyFace**)Arg1;
        const VPolyFace* Poly2 = *(const VPolyFace**)Arg2;

        f32 ZMax1 = MAX(MAX(Poly1->TransVtx[0].Z, Poly1->TransVtx[1].Z), Poly1->TransVtx[2].Z);
        f32 ZMax2 = MAX(MAX(Poly2->TransVtx[0].Z, Poly2->TransVtx[1].Z), Poly2->TransVtx[2].Z);

        if (ZMax1 < ZMax2)
        {
            return 1;
        }
        else if (ZMax1 > ZMax2)
        {
            return -1;
        }
        else
        {
            return 0;
        }

        return 1;
    }

    void SortPolygons(ESortPolygonsMethod Method = ESortPolygonsMethod::Average)
    {
        switch (Method)
        {
        case ESortPolygonsMethod::Average: qsort(PolyPtrList, NumPoly, sizeof(*PolyPtrList), SortPolygonsCompareAverage); break;
        case ESortPolygonsMethod::Near:    qsort(PolyPtrList, NumPoly, sizeof(*PolyPtrList), SortPolygonsCompareNear); break;
        case ESortPolygonsMethod::Far:     qsort(PolyPtrList, NumPoly, sizeof(*PolyPtrList), SortPolygonsCompareFar); break;
        }
    }

    void TransformCameraToPerspective(const VCamera& Cam)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::BackFace)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                Poly->TransVtx[V].X = Cam.ViewDist * Poly->TransVtx[V].X / Poly->TransVtx[V].Z;
                Poly->TransVtx[V].Y = Cam.ViewDist * Poly->TransVtx[V].Y * Cam.AspectRatio / Poly->TransVtx[V].Z;
            }
        }
    }

    void ConvertFromHomogeneous()
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::BackFace)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                Poly->TransVtx[V].DivByW();
            }
        }
    }

    void TransformPerspectiveToScreen(const VCamera& Cam)
    {
        f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::BackFace)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                Poly->TransVtx[V].X = Alpha + Alpha * Poly->TransVtx[V].X;
                Poly->TransVtx[V].Y = Beta - Beta * Poly->TransVtx[V].Y;
            }
        }
    }

    void TransformCameraToScreen(const VCamera& Cam)
    {
        f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::BackFace)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                f32 ViewDistDivZ = Cam.ViewDist / Poly->TransVtx[V].Z;

                Poly->TransVtx[V].X = Poly->TransVtx[V].X * ViewDistDivZ;
                Poly->TransVtx[V].Y = Poly->TransVtx[V].Y * Cam.AspectRatio * ViewDistDivZ;

                Poly->TransVtx[V].X = Alpha + Alpha * Poly->TransVtx[V].X;
                Poly->TransVtx[V].Y = Beta - Beta * Poly->TransVtx[V].Y;
            }
        }
    }

    void RenderWire(u32* Buffer, i32 Pitch)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::BackFace ||
                Poly->State & EPolyState::Clipped)
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
            VPolyFace* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::BackFace ||
                Poly->State & EPolyState::Clipped)
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
