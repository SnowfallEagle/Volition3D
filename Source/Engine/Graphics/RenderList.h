/* TODO:
 */

#pragma once

#include <stdlib.h> // qsort()
#include "Engine/Math/Minimal.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/TransformType.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/Mesh.h"

namespace EClipFlags
{
    enum Type
    {
        X = VL_BIT(1),
        Y = VL_BIT(2),
        Z = VL_BIT(3),

        Full = X | Y | Z
    };
}

enum class ESortPolygonsMethod
{
    Average = 0,
    Near,
    Far
};

VL_DEFINE_LOG_CHANNEL(hLogRenderList, "RenderList");

class VRenderList
{
public:
    static constexpr i32f MaxPoly = 32768;

public:
    i32 NumPoly;
    VPolyFace* PolyPtrList[MaxPoly];
    VPolyFace PolyList[MaxPoly];

public:
    b32 InsertPoly(const VPoly& Poly, const VVertex* VtxList)
    {
        if (NumPoly >= MaxPoly)
        {
            return false;
        }

        PolyPtrList[NumPoly] = &PolyList[NumPoly];
        PolyList[NumPoly].State = Poly.State;
        PolyList[NumPoly].Attr = Poly.Attr;
        PolyList[NumPoly].OriginalColor = Poly.OriginalColor;
        PolyList[NumPoly].NormalLength = Poly.NormalLength;
        PolyList[NumPoly].Texture = Poly.Texture;
        PolyList[NumPoly].Material = Poly.Material;

        for (i32f I = 0; I < 3; ++I)
        {
            PolyList[NumPoly].TransVtx[I] = PolyList[NumPoly].LocalVtx[I] = VtxList[Poly.VtxIndices[I]];
            PolyList[NumPoly].TransVtx[I].TextureCoords =
                PolyList[NumPoly].LocalVtx[I].TextureCoords =
                    Poly.TextureCoordsList[Poly.TextureCoordsIndices[I]];

            PolyList[NumPoly].LitColor[I] = Poly.LitColor[I];
        }

        ++NumPoly;

        return true;
    }

    b32 InsertPolyFace(const VPolyFace& Poly)
    {
        if (NumPoly >= MaxPoly)
        {
            return false;
        }

        PolyPtrList[NumPoly] = &PolyList[NumPoly];
        PolyList[NumPoly] = Poly;

        ++NumPoly;

        return true;
    }

    void InsertMesh(VMesh& Mesh, b32 bInsertLocal)
    {
        if (~Mesh.State & EMeshState::Active  ||
            ~Mesh.State & EMeshState::Visible ||
            Mesh.State & EMeshState::Culled)
        {
            return;
        }

        for (i32f I = 0; I < Mesh.NumPoly; ++I)
        {
            VPoly& Poly = Mesh.PolyList[I];

            if (~Poly.State & EPolyState::Active ||
                Poly.State & EPolyState::Clipped ||
                Poly.State & EPolyState::BackFace)
            {
                continue;
            }

            if (!InsertPoly(Poly, bInsertLocal ? Mesh.LocalVtxList : Mesh.TransVtxList))
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
                    VVector4::MulMat44(Poly->LocalVtx[V].Position, M, Res);
                    Poly->LocalVtx[V].Position = Res;

                    if (Poly->LocalVtx[V].Attr & EVertexAttr::HasNormal)
                    {
                        VVector4::MulMat44(Poly->LocalVtx[V].Normal, M, Res);
                        Poly->LocalVtx[V].Normal = Res;
                    }
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
                    VVector4::MulMat44(Poly->TransVtx[V].Position, M, Res);
                    Poly->TransVtx[V].Position = Res;

                    if (Poly->TransVtx[V].Attr & EVertexAttr::HasNormal)
                    {
                        VVector4::MulMat44(Poly->TransVtx[V].Normal, M, Res);
                        Poly->TransVtx[V].Normal = Res;
                    }
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
                    VVector4::MulMat44(Poly->LocalVtx[V].Position, M, Poly->TransVtx[V].Position);

                    if (Poly->LocalVtx[V].Attr & EVertexAttr::HasNormal)
                    {
                        VVector4::MulMat44(Poly->LocalVtx[V].Normal, M, Poly->TransVtx[V].Normal);
                    }
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
                    // TODO(sean): Maybe we should copy Intensity and Attr too
                    Poly->TransVtx[V].Position = Poly->LocalVtx[V].Position + WorldPos;
                    Poly->TransVtx[V].Normal = Poly->LocalVtx[V].Normal;
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
                    Poly->TransVtx[V].Position += WorldPos;
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
            U = Poly->TransVtx[1].Position - Poly->TransVtx[0].Position;
            V = Poly->TransVtx[2].Position - Poly->TransVtx[0].Position;

            VVector4::Cross(U, V, N);
            VVector4 View = Cam.Pos - Poly->TransVtx[0].Position;

            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4::Dot(View, N) <= 0.0f)
            {
                Poly->State |= EPolyState::BackFace;
            }
        }
    }

    void Light(const VCamera& Cam, const VLight* Lights, i32 NumLights)
    {
        for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
        {
            // Check if we need to draw this poly
            VPolyFace* Poly = PolyPtrList[PolyIndex];

            if (~Poly->State & EPolyState::Active  ||
                Poly->State & EPolyState::Clipped  ||
                Poly->State & EPolyState::BackFace ||
                Poly->State & EPolyState::Lit)
            {
                continue;
            }

            // Set lit flag
            Poly->State |= EPolyState::Lit;

            // Do lighting
            if (Poly->Attr & EPolyAttr::ShadeModeFlat)
            {
                u32 RSum = 0;
                u32 GSum = 0;
                u32 BSum = 0;

                VVector4 SurfaceNormal = VVector4::GetCross(
                    Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                    Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
                );
                f32 SurfaceNormalLength = Poly->NormalLength;

                for (i32f LightIndex = 0; LightIndex < NumLights; ++LightIndex)
                {
                    if (~Lights[LightIndex].State & ELightState::Active)
                    {
                        continue;
                    }

                    if (Lights[LightIndex].Attr & ELightAttr::Ambient)
                    {
                        RSum += (Poly->OriginalColor.R * Lights[LightIndex].CAmbient.R) / 256;
                        GSum += (Poly->OriginalColor.G * Lights[LightIndex].CAmbient.G) / 256;
                        BSum += (Poly->OriginalColor.B * Lights[LightIndex].CAmbient.B) / 256;
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::Infinite)
                    {
                        f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].TransDir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)( 128.0f * (Math.Abs(Dot) / SurfaceNormalLength) );
                            RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::Point)
                    {
                        VVector4 Direction = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;

                        f32 Dot = VVector4::Dot(SurfaceNormal, Direction);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            f32 Distance = Direction.GetLengthFast();
                            f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / (SurfaceNormalLength * Distance * Atten)
                            );

                            RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::SimpleSpotlight)
                    {
                        f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].TransDir);

                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            f32 Distance = (Poly->TransVtx[0].Position - Lights[LightIndex].TransPos).GetLengthFast();
                            f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / (SurfaceNormalLength * Atten)
                            );

                            RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::ComplexSpotlight)
                    {
                        f32 DotNormalDirection = VVector4::Dot(SurfaceNormal, Lights[LightIndex].TransDir);

                        if (DotNormalDirection < 0)
                        {
                            VVector4 DistanceVector = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;
                            f32 Distance = DistanceVector.GetLengthFast();
                            f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                            if (DotDistanceDirection > 0)
                            {
                                f32 DotDistanceDirectionExp = DotDistanceDirection;
                                // For optimization use integer power
                                i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                                for (i32f I = 1; I < IntegerExp; ++I)
                                {
                                    DotDistanceDirectionExp *= DotDistanceDirection;
                                }

                                // 128 used for fixed point to don't lose accuracy with integers
                                f32 Atten =
                                    Lights[LightIndex].KConst +
                                    Lights[LightIndex].KLinear * Distance +
                                    Lights[LightIndex].KQuad * Distance * Distance;
                                i32 Intensity = (i32)(
                                    (128.0f * Math.Abs(DotNormalDirection) * DotDistanceDirectionExp) /
                                    (SurfaceNormalLength * Atten)
                                );

                                RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                                GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                                BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                            }
                        }
                    }
                }

                // Check that we are in range
                if (RSum > 255) RSum = 255;
                if (GSum > 255) GSum = 255;
                if (BSum > 255) BSum = 255;

                // Put final color
                Poly->LitColor[0] = MAP_ARGB32(Poly->OriginalColor.A, RSum, GSum, BSum);
            }
            else if (Poly->Attr & EPolyAttr::ShadeModeGouraud)
            {
                u32 RSum0 = 0;
                u32 RSum1 = 0;
                u32 RSum2 = 0;

                u32 GSum0 = 0;
                u32 GSum1 = 0;
                u32 GSum2 = 0;

                u32 BSum0 = 0;
                u32 BSum1 = 0;
                u32 BSum2 = 0;

                VVector4 SurfaceNormal = VVector4::GetCross(
                    Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                    Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
                );
                f32 SurfaceNormalLength = Poly->NormalLength;

                for (i32f LightIndex = 0; LightIndex < NumLights; ++LightIndex)
                {
                    if (~Lights[LightIndex].State & ELightState::Active)
                    {
                        continue;
                    }

                    if (Lights[LightIndex].Attr & ELightAttr::Ambient)
                    {
                        i32 RIntensity = (Poly->OriginalColor.R * Lights[LightIndex].CAmbient.R) / 256;
                        i32 GIntensity = (Poly->OriginalColor.G * Lights[LightIndex].CAmbient.G) / 256;
                        i32 BIntensity = (Poly->OriginalColor.B * Lights[LightIndex].CAmbient.B) / 256;

                        RSum0 += RIntensity;
                        RSum1 += RIntensity;
                        RSum2 += RIntensity;

                        GSum0 += GIntensity;
                        GSum1 += GIntensity;
                        GSum2 += GIntensity;

                        BSum0 += BIntensity;
                        BSum1 += BIntensity;
                        BSum2 += BIntensity;
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::Infinite)
                    {
                        f32 Dot = VVector4::Dot(Poly->TransVtx[0].Normal, Lights[LightIndex].TransDir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)( 128.0f * Math.Abs(Dot));
                            RSum0 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum0 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum0 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }

                        Dot = VVector4::Dot(Poly->TransVtx[1].Normal, Lights[LightIndex].TransDir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)( 128.0f * Math.Abs(Dot));
                            RSum1 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum1 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum1 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }

                        Dot = VVector4::Dot(Poly->TransVtx[2].Normal, Lights[LightIndex].TransDir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)( 128.0f * Math.Abs(Dot));
                            RSum2 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum2 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum2 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::Point)
                    {
                        VVector4 Direction = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;
                        f32 Distance = Direction.GetLengthFast();
                        f32 Atten =
                            Lights[LightIndex].KConst +
                            Lights[LightIndex].KLinear * Distance +
                            Lights[LightIndex].KQuad * Distance * Distance;

                        f32 Dot = VVector4::Dot(Poly->TransVtx[0].Normal, Direction);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / (Distance * Atten)
                            );

                            RSum0 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum0 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum0 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }

                        Dot = VVector4::Dot(Poly->TransVtx[1].Normal, Direction);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / (Distance * Atten)
                            );

                            RSum1 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum1 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum1 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }

                        Dot = VVector4::Dot(Poly->TransVtx[2].Normal, Direction);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / (Distance * Atten)
                            );

                            RSum2 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum2 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum2 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::SimpleSpotlight)
                    {
                        f32 Distance = (Poly->TransVtx[0].Position - Lights[LightIndex].TransPos).GetLengthFast();
                        f32 Atten =
                            Lights[LightIndex].KConst +
                            Lights[LightIndex].KLinear * Distance +
                            Lights[LightIndex].KQuad * Distance * Distance;

                        f32 Dot = VVector4::Dot(Poly->TransVtx[0].Normal, Lights[LightIndex].TransDir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / Atten
                            );

                            RSum0 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum0 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum0 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }

                        Dot = VVector4::Dot(Poly->TransVtx[1].Normal, Lights[LightIndex].TransDir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / Atten
                            );

                            RSum1 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum1 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum1 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }

                        Dot = VVector4::Dot(Poly->TransVtx[2].Normal, Lights[LightIndex].TransDir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / Atten
                            );

                            RSum2 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum2 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum2 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::ComplexSpotlight)
                    {
                        f32 DotNormalDirection = VVector4::Dot(Poly->TransVtx[0].Normal, Lights[LightIndex].TransDir);
                        if (DotNormalDirection < 0)
                        {
                            VVector4 DistanceVector = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;
                            f32 Distance = DistanceVector.GetLengthFast();
                            f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                            if (DotDistanceDirection > 0)
                            {
                                f32 Atten =
                                    Lights[LightIndex].KConst +
                                    Lights[LightIndex].KLinear * Distance +
                                    Lights[LightIndex].KQuad * Distance * Distance;

                                f32 DotDistanceDirectionExp = DotDistanceDirection;
                                // For optimization use integer power
                                i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                                for (i32f I = 1; I < IntegerExp; ++I)
                                {
                                    DotDistanceDirectionExp *= DotDistanceDirection;
                                }

                                // 128 used for fixed point to don't lose accuracy with integers
                                i32 Intensity = (i32)(
                                    (128.0f * Math.Abs(DotNormalDirection) * DotDistanceDirectionExp) / Atten
                                );

                                RSum0 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                                GSum0 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                                BSum0 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                            }
                        }

                        DotNormalDirection = VVector4::Dot(Poly->TransVtx[1].Normal, Lights[LightIndex].TransDir);
                        if (DotNormalDirection < 0)
                        {
                            VVector4 DistanceVector = Poly->TransVtx[1].Position - Lights[LightIndex].TransPos;
                            f32 Distance = DistanceVector.GetLengthFast();
                            f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                            if (DotDistanceDirection > 0)
                            {
                                f32 Atten =
                                    Lights[LightIndex].KConst +
                                    Lights[LightIndex].KLinear * Distance +
                                    Lights[LightIndex].KQuad * Distance * Distance;

                                f32 DotDistanceDirectionExp = DotDistanceDirection;
                                // For optimization use integer power
                                i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                                for (i32f I = 1; I < IntegerExp; ++I)
                                {
                                    DotDistanceDirectionExp *= DotDistanceDirection;
                                }

                                // 128 used for fixed point to don't lose accuracy with integers
                                i32 Intensity = (i32)(
                                    (128.0f * Math.Abs(DotNormalDirection) * DotDistanceDirectionExp) / Atten
                                );

                                RSum1 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                                GSum1 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                                BSum1 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                            }
                        }

                        DotNormalDirection = VVector4::Dot(Poly->TransVtx[2].Normal, Lights[LightIndex].TransDir);
                        if (DotNormalDirection < 0)
                        {
                            VVector4 DistanceVector = Poly->TransVtx[2].Position - Lights[LightIndex].TransPos;
                            f32 Distance = DistanceVector.GetLengthFast();
                            f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                            if (DotDistanceDirection > 0)
                            {
                                f32 Atten =
                                    Lights[LightIndex].KConst +
                                    Lights[LightIndex].KLinear * Distance +
                                    Lights[LightIndex].KQuad * Distance * Distance;

                                f32 DotDistanceDirectionExp = DotDistanceDirection;
                                // For optimization use integer power
                                i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                                for (i32f I = 1; I < IntegerExp; ++I)
                                {
                                    DotDistanceDirectionExp *= DotDistanceDirection;
                                }

                                // 128 used for fixed point to don't lose accuracy with integers
                                i32 Intensity = (i32)(
                                    (128.0f * Math.Abs(DotNormalDirection) * DotDistanceDirectionExp) / Atten
                                );

                                RSum2 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                                GSum2 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                                BSum2 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                            }
                        }
                    }
                }

                // Check that we are in range
                if (RSum0 > 255) RSum0 = 255;
                if (RSum1 > 255) RSum1 = 255;
                if (RSum2 > 255) RSum2 = 255;

                if (GSum0 > 255) GSum0 = 255;
                if (GSum1 > 255) GSum1 = 255;
                if (GSum2 > 255) GSum2 = 255;

                if (BSum0 > 255) BSum0 = 255;
                if (BSum1 > 255) BSum1 = 255;
                if (BSum2 > 255) BSum2 = 255;

                // Put final color
                Poly->LitColor[0] = MAP_ARGB32(Poly->OriginalColor.A, RSum0, GSum0, BSum0);
                Poly->LitColor[1] = MAP_ARGB32(Poly->OriginalColor.A, RSum1, GSum1, BSum1);
                Poly->LitColor[2] = MAP_ARGB32(Poly->OriginalColor.A, RSum2, GSum2, BSum2);
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
                VVector4::MulMat44(Poly->TransVtx[V].Position, Camera.MatCamera, Res);
                Poly->TransVtx[V].Position = Res;
            }
        }
    }

    void Clip(const VCamera& Camera, EClipFlags::Type Flags = EClipFlags::Full)
    {
        enum EClipCode
        {
            XGreater = VL_BIT(1),
            XLess    = VL_BIT(2),

            YGreater = VL_BIT(3),
            YLess    = VL_BIT(4),

            ZGreater = VL_BIT(5),
            ZLess    = VL_BIT(6),
            ZIn      = VL_BIT(7),
        };

        i32f SavedNumPoly = NumPoly;

        for (i32f PolyIndex = 0; PolyIndex < SavedNumPoly; ++PolyIndex)
        {
            VPolyFace& Poly = *PolyPtrList[PolyIndex];

            if (~Poly.State & EPolyState::Active  ||
                Poly.State & EPolyState::BackFace ||
                Poly.State & EPolyState::Clipped)
            {
                continue;
            }

            u32 ClipCodes[3] = { 0, 0, 0 };
            f32 ZFactor;
            f32 ZTest;

            if (Flags & EClipFlags::X)
            {
                ZFactor = (0.5f * Camera.ViewPlaneSize.X) / Camera.ViewDist;
                ZTest = ZFactor * Poly.TransVtx[0].Z;

                if (Poly.TransVtx[0].X > ZTest)
                {
                    ClipCodes[0] |= EClipCode::XGreater;
                }
                else if (Poly.TransVtx[0].X < -ZTest)
                {
                    ClipCodes[0] |= EClipCode::XLess;
                }

                ZTest = ZFactor * Poly.TransVtx[1].Z;

                if (Poly.TransVtx[1].X > ZTest)
                {
                    ClipCodes[1] |= EClipCode::XGreater;
                }
                else if (Poly.TransVtx[1].X < -ZTest)
                {
                    ClipCodes[1] |= EClipCode::XLess;
                }

                ZTest = ZFactor * Poly.TransVtx[2].Z;

                if (Poly.TransVtx[2].X > ZTest)
                {
                    ClipCodes[2] |= EClipCode::XGreater;
                }
                else if (Poly.TransVtx[2].X < -ZTest)
                {
                    ClipCodes[2] |= EClipCode::XLess;
                }

                if ((ClipCodes[0] & EClipCode::XLess &&
                     ClipCodes[1] & EClipCode::XLess &&
                     ClipCodes[2] & EClipCode::XLess) ||

                    (ClipCodes[0] & EClipCode::XGreater &&
                     ClipCodes[1] & EClipCode::XGreater &&
                     ClipCodes[2] & EClipCode::XGreater))
                {
                    Poly.State |= EPolyState::Clipped;
                    continue;
                }
            }

            if (Flags & EClipFlags::Y)
            {
                ZFactor = (0.5f * Camera.ViewPlaneSize.Y) / Camera.ViewDist;
                ZTest = ZFactor * Poly.TransVtx[0].Z;

                if (Poly.TransVtx[0].Y > ZTest)
                {
                    ClipCodes[0] |= EClipCode::YGreater;
                }
                else if (Poly.TransVtx[0].Y < -ZTest)
                {
                    ClipCodes[0] |= EClipCode::YLess;
                }

                ZTest = ZFactor * Poly.TransVtx[1].Z;

                if (Poly.TransVtx[1].Y > ZTest)
                {
                    ClipCodes[1] |= EClipCode::YGreater;
                }
                else if (Poly.TransVtx[1].Y < -ZTest)
                {
                    ClipCodes[1] |= EClipCode::YLess;
                }

                ZTest = ZFactor * Poly.TransVtx[2].Z;

                if (Poly.TransVtx[2].Y > ZTest)
                {
                    ClipCodes[2] |= EClipCode::YGreater;
                }
                else if (Poly.TransVtx[2].Y < -ZTest)
                {
                    ClipCodes[2] |= EClipCode::YLess;
                }

                if ((ClipCodes[0] & EClipCode::YLess &&
                     ClipCodes[1] & EClipCode::YLess &&
                     ClipCodes[2] & EClipCode::YLess) ||

                    (ClipCodes[0] & EClipCode::YGreater &&
                     ClipCodes[1] & EClipCode::YGreater &&
                     ClipCodes[2] & EClipCode::YGreater))
                {
                    Poly.State |= EPolyState::Clipped;
                    continue;
                }
            }

            if (Flags & EClipFlags::Z)
            {
                i32f NumVertsIn = 0;

                if (Poly.TransVtx[0].Z < Camera.ZNearClip)
                {
                    ClipCodes[0] |= EClipCode::ZLess;
                }
                else if (Poly.TransVtx[0].Z > Camera.ZFarClip)
                {
                    ClipCodes[0] |= EClipCode::ZGreater;
                }
                else
                {
                    ClipCodes[0] |= EClipCode::ZIn;
                    ++NumVertsIn;
                }

                if (Poly.TransVtx[1].Z < Camera.ZNearClip)
                {
                    ClipCodes[1] |= EClipCode::ZLess;
                }
                else if (Poly.TransVtx[1].Z > Camera.ZFarClip)
                {
                    ClipCodes[1] |= EClipCode::ZGreater;
                }
                else
                {
                    ClipCodes[1] |= EClipCode::ZIn;
                    ++NumVertsIn;
                }

                if (Poly.TransVtx[2].Z < Camera.ZNearClip)
                {
                    ClipCodes[2] |= EClipCode::ZLess;
                }
                else if (Poly.TransVtx[2].Z > Camera.ZFarClip)
                {
                    ClipCodes[2] |= EClipCode::ZGreater;
                }
                else
                {
                    ClipCodes[2] |= EClipCode::ZIn;
                    ++NumVertsIn;
                }

                if ((ClipCodes[0] & EClipCode::ZLess &&
                     ClipCodes[1] & EClipCode::ZLess &&
                     ClipCodes[2] & EClipCode::ZLess) ||

                    (ClipCodes[0] & EClipCode::ZGreater &&
                     ClipCodes[1] & EClipCode::ZGreater &&
                     ClipCodes[2] & EClipCode::ZGreater))
                {
                    Poly.State |= EPolyState::Clipped;
                    continue;
                }

                if ((ClipCodes[0] | ClipCodes[1] | ClipCodes[2]) & EClipCode::ZLess)
                {
                    i32f V0, V1, V2;

                    if (NumVertsIn == 1)
                    {
                        // TODO(sean): Try to optimize it through V0 - V1

                        /*
                            ZNearClip = Z0 + (Z1 - Z0) * t?

                            t! = (ZNearClip - Z0) / (Z1 - Z0)
                            NewX = X0 + (X1 - X0) * t!
                            NewY = Y0 + (Y1 - Y0) * t!
                        */

                        // Get vertex indices
                        if (ClipCodes[0] & EClipCode::ZIn)
                        {
                            V0 = 0;
                            V1 = 1;
                            V2 = 2;
                        }
                        else if (ClipCodes[1] & EClipCode::ZIn)
                        {
                            V0 = 1;
                            V1 = 2;
                            V2 = 0;
                        }
                        else
                        {
                            V0 = 2;
                            V1 = 0;
                            V2 = 1;
                        }

                        // Recompute X and Y for ZNearClip
                        VVector4 Direction = Poly.TransVtx[V1].Position - Poly.TransVtx[V0].Position;
                        f32 T1 = (Camera.ZNearClip - Poly.TransVtx[V0].Z) / Direction.Z;

                        Poly.TransVtx[V1].X = Poly.TransVtx[V0].X + Direction.X * T1;
                        Poly.TransVtx[V1].Y = Poly.TransVtx[V0].Y + Direction.Y * T1;
                        Poly.TransVtx[V1].Z = Camera.ZNearClip;

                        Direction = Poly.TransVtx[V2].Position - Poly.TransVtx[V0].Position;
                        f32 T2 = (Camera.ZNearClip - Poly.TransVtx[V0].Z) / Direction.Z;

                        Poly.TransVtx[V2].X = Poly.TransVtx[V0].X + Direction.X * T2;
                        Poly.TransVtx[V2].Y = Poly.TransVtx[V0].Y + Direction.Y * T2;
                        Poly.TransVtx[V2].Z = Camera.ZNearClip;

                        // Recompute texture coords
                        if (Poly.Attr & EPolyAttr::ShadeModeTexture)
                        {
                            VPoint2 TextureDirection = Poly.TransVtx[V1].TextureCoords - Poly.TransVtx[V0].TextureCoords;

                            Poly.TransVtx[V1].U = Poly.TransVtx[V0].U + TextureDirection.X * T1;
                            Poly.TransVtx[V1].V = Poly.TransVtx[V0].V + TextureDirection.Y * T1;

                            TextureDirection = Poly.TransVtx[V2].TextureCoords - Poly.TransVtx[V0].TextureCoords;

                            Poly.TransVtx[V2].U = Poly.TransVtx[V0].U + TextureDirection.X * T2;
                            Poly.TransVtx[V2].V = Poly.TransVtx[V0].V + TextureDirection.Y * T2;
                        }

                        // Recompute poly normal length
                        VVector4 Vec1 = Poly.TransVtx[V1].Position - Poly.TransVtx[V0].Position;
                        VVector4 Vec2 = Poly.TransVtx[V2].Position - Poly.TransVtx[V0].Position;
                        VVector4 VecNormal;

                        VVector4::Cross(Vec1, Vec2, VecNormal);
                        Poly.NormalLength = VecNormal.GetLengthFast();
                    }
                    else
                    {
                        // Copy current poly
                        VPolyFace NewPoly = Poly;

                        // Get vertex indices
                        if (ClipCodes[0] & EClipCode::ZLess)
                        {
                            V0 = 0;
                            V1 = 1;
                            V2 = 2;
                        }
                        else if (ClipCodes[1] & EClipCode::ZLess)
                        {
                            V0 = 1;
                            V1 = 2;
                            V2 = 0;
                        }
                        else
                        {
                            V0 = 2;
                            V1 = 0;
                            V2 = 1;
                        }

                        // Recompute X and Y for ZNearClip
                        VVector4 Direction = Poly.TransVtx[V1].Position - Poly.TransVtx[V0].Position;
                        f32 T1 = (Camera.ZNearClip - Poly.TransVtx[V0].Z) / Direction.Z;

                        f32 X01 = Poly.TransVtx[V0].X + Direction.X * T1;
                        f32 Y01 = Poly.TransVtx[V0].Y + Direction.Y * T1;

                        Direction = Poly.TransVtx[V2].Position - Poly.TransVtx[V0].Position;
                        f32 T2 = (Camera.ZNearClip - Poly.TransVtx[V0].Z) / Direction.Z;

                        f32 X02 = Poly.TransVtx[V0].X + Direction.X * T2;
                        f32 Y02 = Poly.TransVtx[V0].Y + Direction.Y * T2;

                        // Put values in polygons
                        Poly.TransVtx[V0].X = X01;
                        Poly.TransVtx[V0].Y = Y01;
                        Poly.TransVtx[V0].Z = Camera.ZNearClip;

                        NewPoly.TransVtx[V0].X = X02;
                        NewPoly.TransVtx[V0].Y = Y02;
                        NewPoly.TransVtx[V0].Z = Camera.ZNearClip;

                        NewPoly.TransVtx[V1].X = X01;
                        NewPoly.TransVtx[V1].Y = Y01;
                        NewPoly.TransVtx[V1].Z = Camera.ZNearClip;

                        // Recompute texture coords
                        if (Poly.Attr & EPolyAttr::ShadeModeTexture)
                        {
                            VPoint2 TextureDirection = Poly.TransVtx[V1].TextureCoords - Poly.TransVtx[V0].TextureCoords;

                            f32 U01 = Poly.TransVtx[V0].U + TextureDirection.X * T1;
                            f32 V01 = Poly.TransVtx[V0].V + TextureDirection.Y * T1;

                            TextureDirection = Poly.TransVtx[V2].TextureCoords - Poly.TransVtx[V0].TextureCoords;

                            f32 U02 = Poly.TransVtx[V0].U + TextureDirection.X * T2;
                            f32 V02 = Poly.TransVtx[V0].V + TextureDirection.Y * T2;

                            Poly.TransVtx[V0].U = U01;
                            Poly.TransVtx[V0].V = V01;

                            NewPoly.TransVtx[V0].U = U02;
                            NewPoly.TransVtx[V0].V = V02;
                            NewPoly.TransVtx[V1].U = U01;
                            NewPoly.TransVtx[V1].V = V01;
                        }

                        // Recompute poly normal length
                        VVector4 Vec1 = Poly.TransVtx[V1].Position - Poly.TransVtx[V0].Position;
                        VVector4 Vec2 = Poly.TransVtx[V2].Position - Poly.TransVtx[V0].Position;
                        VVector4 VecNormal;

                        VVector4::Cross(Vec1, Vec2, VecNormal);
                        Poly.NormalLength = VecNormal.GetLengthFast();

                        Vec1 = NewPoly.TransVtx[V1].Position - NewPoly.TransVtx[V0].Position;
                        Vec2 = NewPoly.TransVtx[V2].Position - NewPoly.TransVtx[V0].Position;
                        VecNormal;

                        VVector4::Cross(Vec1, Vec2, VecNormal);
                        NewPoly.NormalLength = VecNormal.GetLengthFast();

                        // Finally
                        InsertPolyFace(NewPoly);
                    }
                }
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

        f32 ZMin1 = VL_MIN(VL_MIN(Poly1->TransVtx[0].Z, Poly1->TransVtx[1].Z), Poly1->TransVtx[2].Z);
        f32 ZMin2 = VL_MIN(VL_MIN(Poly2->TransVtx[0].Z, Poly2->TransVtx[1].Z), Poly2->TransVtx[2].Z);

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

        f32 ZMax1 = VL_MAX(VL_MAX(Poly1->TransVtx[0].Z, Poly1->TransVtx[1].Z), Poly1->TransVtx[2].Z);
        f32 ZMax2 = VL_MAX(VL_MAX(Poly2->TransVtx[0].Z, Poly2->TransVtx[1].Z), Poly2->TransVtx[2].Z);

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
                Poly->TransVtx[V].Position.DivByW();
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
                Poly->LitColor[0]
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
                (i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
                Poly->LitColor[0]
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
                (i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
                Poly->LitColor[0]
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

            Renderer.DrawTriangle(Buffer, Pitch, *Poly);
        }
    }
};
