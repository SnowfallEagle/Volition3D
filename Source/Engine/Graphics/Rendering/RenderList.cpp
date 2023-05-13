#include "Engine/Graphics/Rendering/RenderList.h"

namespace Volition
{

b32 VRenderList::InsertPoly(const VPoly& Poly, const VVertex* VtxList, const VPoint2* TextureCoordsList)
{
    if (NumPoly >= MaxPoly)
    {
        return false;
    }

    PolyList[NumPoly].State = Poly.State;
    PolyList[NumPoly].Attr = Poly.Attr;
    PolyList[NumPoly].OriginalColor = Poly.OriginalColor;
    PolyList[NumPoly].NormalLength = Poly.NormalLength;
    PolyList[NumPoly].Material = Poly.Material;

    for (i32f i = 0; i < 3; ++i)
    {
        PolyList[NumPoly].TransVtx[i] = PolyList[NumPoly].LocalVtx[i] = VtxList[Poly.VtxIndices[i]];
        PolyList[NumPoly].TransVtx[i].TextureCoords = PolyList[NumPoly].LocalVtx[i].TextureCoords = TextureCoordsList[Poly.TextureCoordsIndices[i]];

        PolyList[NumPoly].LitColor[i] = Poly.LitColor[i];
    }

    ++NumPoly;
    return true;
}

b32 VRenderList::InsertPolyFace(const VPolyFace& Poly)
{
    if (NumPoly >= MaxPoly)
    {
        return false;
    }

    PolyList[NumPoly] = Poly;

    ++NumPoly;

    return true;
}

void VRenderList::InsertMesh(VMesh& Mesh, b32 bInsertLocal)
{
    if (~Mesh.State & EMeshState::Active  ||
        ~Mesh.State & EMeshState::Visible ||
        Mesh.State & EMeshState::Culled)
    {
        return;
    }

    for (i32f i = 0; i < Mesh.NumPoly; ++i)
    {
        const VPoly& Poly = Mesh.PolyList[i];

        if (~Poly.State & EPolyState::Active ||
            Poly.State & EPolyState::Clipped ||
            Poly.State & EPolyState::Backface)
        {
            continue;
        }

        if (!InsertPoly(Poly, bInsertLocal ? Mesh.LocalVtxList : Mesh.TransVtxList, Mesh.TextureCoordsList))
        {
            return;
        }
    }
}

void VRenderList::ResetStateAndSaveList()
{
    NumPoly -= NumAdditionalPoly;
    NumAdditionalPoly = 0;

    // Restore polygons
    for (i32f i = 0; i < NumPoly; ++i)
    {
        VPolyFace& Poly = PolyList[i];
        if (~Poly.State & EPolyState::Active)
        {
            continue;
        }

        Poly.State &= ~(EPolyState::Clipped | EPolyState::Backface | EPolyState::Lit);
        Poly.LitColor[2] = Poly.LitColor[1] = Poly.LitColor[0] = Poly.OriginalColor;
    }
}

void VRenderList::Transform(const VMatrix44& M, ETransformType Type)
{
    VVector4 Res;

    switch (Type)
    {
    case ETransformType::LocalOnly:
    {
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPolyFace* Poly = &PolyList[i];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::Backface)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                VMatrix44::MulVecMat(Poly->LocalVtx[V].Position, M, Res);
                Poly->LocalVtx[V].Position = Res;

                if (Poly->LocalVtx[V].Attr & EVertexAttr::HasNormal)
                {
                    VMatrix44::MulVecMat(Poly->LocalVtx[V].Normal, M, Res);
                    Poly->LocalVtx[V].Normal = Res;
                }
            }
        }
    } break;

    case ETransformType::TransOnly:
    {
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPolyFace* Poly = &PolyList[i];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::Backface)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                VMatrix44::MulVecMat(Poly->TransVtx[V].Position, M, Res);
                Poly->TransVtx[V].Position = Res;

                if (Poly->TransVtx[V].Attr & EVertexAttr::HasNormal)
                {
                    VMatrix44::MulVecMat(Poly->TransVtx[V].Normal, M, Res);
                    Poly->TransVtx[V].Normal = Res;
                }
            }
        }
    } break;

    case ETransformType::LocalToTrans:
    {
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPolyFace* Poly = &PolyList[i];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::Backface)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                VMatrix44::MulVecMat(Poly->LocalVtx[V].Position, M, Poly->TransVtx[V].Position);

                if (Poly->LocalVtx[V].Attr & EVertexAttr::HasNormal)
                {
                    VMatrix44::MulVecMat(Poly->LocalVtx[V].Normal, M, Poly->TransVtx[V].Normal);
                }
            }
        }
    } break;
    }
}

// LocalToTrans or TransOnly
void VRenderList::TransformModelToWorld(const VPoint4& WorldPos, ETransformType Type)
{
    if (Type == ETransformType::LocalToTrans)
    {
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPolyFace* Poly = &PolyList[i];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::Backface)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                Poly->TransVtx[V].Position = Poly->LocalVtx[V].Position + WorldPos;
            }
        }
    }
    else // TransOnly
    {
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPolyFace* Poly = &PolyList[i];
            if (!Poly ||
                ~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::Backface)
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

void VRenderList::RemoveBackfaces(const VCamera& Cam)
{
    if (bTerrain)
    {
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPolyFace* Poly = &PolyList[i];

            if (~Poly->State & EPolyState::Active ||
                Poly->State & EPolyState::Clipped ||
                Poly->State & EPolyState::Backface ||
                Poly->Attr & EPolyAttr::TwoSided)
            {
                continue;
            }

            // @NOTE: Use local vtx because we didn't transformed vertices at this stage yet
            const VVector4 U = Poly->LocalVtx[1].Position - Poly->LocalVtx[0].Position;
            const VVector4 V = Poly->LocalVtx[2].Position - Poly->LocalVtx[0].Position;

            VVector4 N;
            VVector4::Cross(U, V, N);

            const VVector4 View = Cam.Pos - Poly->LocalVtx[0].Position;

            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4::Dot(View, N) / (N.GetLength() * View.GetLength()) < -0.45f)
            {
                Poly->State |= EPolyState::Backface;
            }
        }
    }
    else
    {
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPolyFace* Poly = &PolyList[i];

            if (~Poly->State & EPolyState::Active  ||
                Poly->State & EPolyState::Clipped  ||
                Poly->State & EPolyState::Backface ||
                Poly->Attr & EPolyAttr::TwoSided)
            {
                continue;
            }

            const VVector4 U = Poly->LocalVtx[1].Position - Poly->LocalVtx[0].Position;
            const VVector4 V = Poly->LocalVtx[2].Position - Poly->LocalVtx[0].Position;

            VVector4 N;
            VVector4::Cross(U, V, N);

            const VVector4 View = Cam.Pos - Poly->LocalVtx[0].Position;

            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4::Dot(View, N) < 0.0f)
            {
                Poly->State |= EPolyState::Backface;
            }
        }
    }
}

void VRenderList::Light(const VCamera& Cam, const VLight* Lights, i32 NumLights)
{
    for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
    {
        // Check if we need to draw this poly
        VPolyFace* Poly = &PolyList[PolyIndex];

        if (~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::NotLightTest)
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

            const VVector4 SurfaceNormal = VVector4::GetCross(
                Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
            );
            const f32 SurfaceNormalLength = Poly->NormalLength;

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
                    const f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].TransDir);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        const i32 Intensity = (i32)( 128.0f * (Math.Abs(Dot) / SurfaceNormalLength) );
                        RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::Point)
                {
                    const VVector4 Direction = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;

                    const f32 Dot = VVector4::Dot(SurfaceNormal, Direction);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        const f32 Distance = Direction.GetLengthFast();
                        const f32 Atten =
                            Lights[LightIndex].KConst +
                            Lights[LightIndex].KLinear * Distance +
                            Lights[LightIndex].KQuad * Distance * Distance;
                        const i32 Intensity = (i32)(
                            (128.0f * Math.Abs(Dot)) / (SurfaceNormalLength * Distance * Atten)
                        );

                        RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::SimpleSpotlight)
                {
                    const f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].TransDir);

                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        const f32 Distance = (Poly->TransVtx[0].Position - Lights[LightIndex].TransPos).GetLengthFast();
                        const f32 Atten =
                            Lights[LightIndex].KConst +
                            Lights[LightIndex].KLinear * Distance +
                            Lights[LightIndex].KQuad * Distance * Distance;
                        const i32 Intensity = (i32)(
                            (128.0f * Math.Abs(Dot)) / (SurfaceNormalLength * Atten)
                        );

                        RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::ComplexSpotlight)
                {
                    const f32 DotNormalDirection = VVector4::Dot(SurfaceNormal, Lights[LightIndex].TransDir);

                    if (DotNormalDirection < 0)
                    {
                        const VVector4 DistanceVector = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;
                        const f32 Distance = DistanceVector.GetLengthFast();
                        const f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                        if (DotDistanceDirection > 0)
                        {
                            f32 DotDistanceDirectionExp = DotDistanceDirection;
                            // For optimization use integer power
                            const i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                            for (i32f i = 1; i < IntegerExp; ++i)
                            {
                                DotDistanceDirectionExp *= DotDistanceDirection;
                            }

                            // 128 used for fixed point to don't lose accuracy with integers
                            const f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;
                            const i32 Intensity = (i32)(
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

            const VVector4 SurfaceNormal = VVector4::GetCross(
                Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
            );
            const f32 SurfaceNormalLength = Poly->NormalLength;

            for (i32f LightIndex = 0; LightIndex < NumLights; ++LightIndex)
            {
                if (~Lights[LightIndex].State & ELightState::Active)
                {
                    continue;
                }

                if (Lights[LightIndex].Attr & ELightAttr::Ambient)
                {
                    const i32 RIntensity = (Poly->OriginalColor.R * Lights[LightIndex].CAmbient.R) / 256;
                    const i32 GIntensity = (Poly->OriginalColor.G * Lights[LightIndex].CAmbient.G) / 256;
                    const i32 BIntensity = (Poly->OriginalColor.B * Lights[LightIndex].CAmbient.B) / 256;

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
                        const i32 Intensity = (i32)(128.0f * Math.Abs(Dot));
                        RSum0 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum0 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum0 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }

                    Dot = VVector4::Dot(Poly->TransVtx[1].Normal, Lights[LightIndex].TransDir);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        const i32 Intensity = (i32)(128.0f * Math.Abs(Dot));
                        RSum1 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum1 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum1 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }

                    Dot = VVector4::Dot(Poly->TransVtx[2].Normal, Lights[LightIndex].TransDir);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        const i32 Intensity = (i32)(128.0f * Math.Abs(Dot));
                        RSum2 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum2 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum2 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::Point)
                {
                    const VVector4 Direction = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;
                    const f32 Distance = Direction.GetLengthFast();
                    const f32 Atten =
                        Lights[LightIndex].KConst +
                        Lights[LightIndex].KLinear * Distance +
                        Lights[LightIndex].KQuad * Distance * Distance;

                    f32 Dot = VVector4::Dot(Poly->TransVtx[0].Normal, Direction);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        const i32 Intensity = (i32)(
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
                        const i32 Intensity = (i32)(
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
                        const i32 Intensity = (i32)(
                            (128.0f * Math.Abs(Dot)) / (Distance * Atten)
                        );

                        RSum2 += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum2 += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum2 += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::SimpleSpotlight)
                {
                    const f32 Distance = (Poly->TransVtx[0].Position - Lights[LightIndex].TransPos).GetLengthFast();
                    const f32 Atten =
                        Lights[LightIndex].KConst +
                        Lights[LightIndex].KLinear * Distance +
                        Lights[LightIndex].KQuad * Distance * Distance;

                    f32 Dot = VVector4::Dot(Poly->TransVtx[0].Normal, Lights[LightIndex].TransDir);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        const i32 Intensity = (i32)(
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
                        const i32 Intensity = (i32)(
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
                        const i32 Intensity = (i32)(
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
                        const VVector4 DistanceVector = Poly->TransVtx[0].Position - Lights[LightIndex].TransPos;
                        const f32 Distance = DistanceVector.GetLengthFast();
                        const f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                        if (DotDistanceDirection > 0)
                        {
                            const f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;

                            f32 DotDistanceDirectionExp = DotDistanceDirection;
                            // For optimization use integer power
                            const i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                            for (i32f i = 1; i < IntegerExp; ++i)
                            {
                                DotDistanceDirectionExp *= DotDistanceDirection;
                            }

                            // 128 used for fixed point to don't lose accuracy with integers
                            const i32 Intensity = (i32)(
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
                        const VVector4 DistanceVector = Poly->TransVtx[1].Position - Lights[LightIndex].TransPos;
                        const f32 Distance = DistanceVector.GetLengthFast();
                        const f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                        if (DotDistanceDirection > 0)
                        {
                            const f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;

                            f32 DotDistanceDirectionExp = DotDistanceDirection;
                            // For optimization use integer power
                            const i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                            for (i32f i = 1; i < IntegerExp; ++i)
                            {
                                DotDistanceDirectionExp *= DotDistanceDirection;
                            }

                            // 128 used for fixed point to don't lose accuracy with integers
                            const i32 Intensity = (i32)(
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
                        const VVector4 DistanceVector = Poly->TransVtx[2].Position - Lights[LightIndex].TransPos;
                        const f32 Distance = DistanceVector.GetLengthFast();
                        const f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].TransDir) / Distance;

                        if (DotDistanceDirection > 0)
                        {
                            const f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;

                            f32 DotDistanceDirectionExp = DotDistanceDirection;
                            // For optimization use integer power
                            const i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                            for (i32f i = 1; i < IntegerExp; ++i)
                            {
                                DotDistanceDirectionExp *= DotDistanceDirection;
                            }

                            // 128 used for fixed point to don't lose accuracy with integers
                            const i32 Intensity = (i32)(
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

void VRenderList::TransformWorldToCamera(const VCamera& Camera)
{
    for (i32f i = 0; i < NumPoly; ++i)
    {
        VPolyFace* Poly = &PolyList[i];
        if (!Poly ||
            ~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::Clipped ||
            Poly->State & EPolyState::Backface)
        {
            continue;
        }

        for (i32f V = 0; V < 3; ++V)
        {
            VMatrix44::MulVecMat(Poly->LocalVtx[V].Position, Camera.MatCamera, Poly->TransVtx[V].Position);

            if (Poly->TransVtx[V].Attr & EVertexAttr::HasNormal)
            {
                VMatrix44::MulVecMat(Poly->LocalVtx[V].Normal, Camera.MatCameraRotationOnly, Poly->TransVtx[V].Normal);
            }
        }
    }
}

void VRenderList::Clip(const VCamera& Camera, EClipFlags::Type Flags)
{
    enum EClipCode
    {
        XGreater = VLN_BIT(1),
        XLess    = VLN_BIT(2),

        YGreater = VLN_BIT(3),
        YLess    = VLN_BIT(4),

        ZGreater = VLN_BIT(5),
        ZLess    = VLN_BIT(6),
        ZIn      = VLN_BIT(7),
    };

    const i32f SavedNumPoly = NumPoly;

    for (i32f PolyIndex = 0; PolyIndex < SavedNumPoly; ++PolyIndex)
    {
        VPolyFace& Poly = PolyList[PolyIndex];

        if (~Poly.State & EPolyState::Active  ||
            Poly.State & EPolyState::Backface ||
            Poly.State & EPolyState::Clipped)
        {
            continue;
        }

        u32 ClipCodes[3] = { 0, 0, 0 };
        f32 ZFactor;
        f32 ZTest;

        if (Flags & EClipFlags::X)
        {
            ZFactor = (0.5f * Camera.ViewplaneSize.X) / Camera.ViewDist;
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
            ZFactor = (0.5f * Camera.ViewplaneSize.Y) / Camera.ViewDist;
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
                    /*
                        ZNearClip = Z0 + (Z1 - Z0) * t

                        t = (ZNearClip - Z0) / (Z1 - Z0)
                        NewX = X0 + (X1 - X0) * t
                        NewY = Y0 + (Y1 - Y0) * !
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

                    // Copy current poly and mark it "clipped"
                    VPolyFace NewPoly = Poly;
                    Poly.State |= EPolyState::Clipped;

                    // Recompute X and Y for ZNearClip
                    VVector4 Direction = NewPoly.TransVtx[V1].Position - NewPoly.TransVtx[V0].Position;
                    const f32 T1 = (Camera.ZNearClip - NewPoly.TransVtx[V0].Z) / Direction.Z;

                    NewPoly.TransVtx[V1].X = 0.5f + NewPoly.TransVtx[V0].X + Direction.X * T1;
                    NewPoly.TransVtx[V1].Y = 0.5f + NewPoly.TransVtx[V0].Y + Direction.Y * T1;
                    NewPoly.TransVtx[V1].Z = Camera.ZNearClip;

                    Direction = NewPoly.TransVtx[V2].Position - NewPoly.TransVtx[V0].Position;
                    const f32 T2 = (Camera.ZNearClip - NewPoly.TransVtx[V0].Z) / Direction.Z;

                    NewPoly.TransVtx[V2].X = NewPoly.TransVtx[V0].X + Direction.X * T2;
                    NewPoly.TransVtx[V2].Y = NewPoly.TransVtx[V0].Y + Direction.Y * T2;
                    NewPoly.TransVtx[V2].Z = Camera.ZNearClip;

                    // Recompute texture coords
                    if (NewPoly.Attr & EPolyAttr::ShadeModeTexture)
                    {
                        VPoint2 TextureDirection = NewPoly.TransVtx[V1].TextureCoords - NewPoly.TransVtx[V0].TextureCoords;

                        NewPoly.TransVtx[V1].U = NewPoly.TransVtx[V0].U + TextureDirection.X * T1;
                        NewPoly.TransVtx[V1].V = NewPoly.TransVtx[V0].V + TextureDirection.Y * T1;

                        TextureDirection = NewPoly.TransVtx[V2].TextureCoords - NewPoly.TransVtx[V0].TextureCoords;

                        NewPoly.TransVtx[V2].U = NewPoly.TransVtx[V0].U + TextureDirection.X * T2;
                        NewPoly.TransVtx[V2].V = NewPoly.TransVtx[V0].V + TextureDirection.Y * T2;
                    }

                    // Recompute poly normal length
                    const VVector4 Vec1 = NewPoly.TransVtx[V1].Position - NewPoly.TransVtx[V0].Position;
                    const VVector4 Vec2 = NewPoly.TransVtx[V2].Position - NewPoly.TransVtx[V0].Position;
                    VVector4 VecNormal;

                    VVector4::Cross(Vec1, Vec2, VecNormal);
                    NewPoly.NormalLength = VecNormal.GetLengthFast();

                    // Insert
                    InsertPolyFace(NewPoly);
                    ++NumAdditionalPoly;
                }
                else
                {
                    // Copy current poly and mark it "clipped"
                    VPolyFace NewPoly1, NewPoly2;
                    NewPoly2 = NewPoly1 = Poly;
                    Poly.State |= EPolyState::Clipped;

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
                    VVector4 Direction = NewPoly1.TransVtx[V1].Position - NewPoly1.TransVtx[V0].Position;
                    const f32 T1 = (Camera.ZNearClip - NewPoly1.TransVtx[V0].Z) / Direction.Z;

                    const f32 X01 = NewPoly1.TransVtx[V0].X + Direction.X * T1;
                    const f32 Y01 = NewPoly1.TransVtx[V0].Y + Direction.Y * T1;

                    Direction = NewPoly1.TransVtx[V2].Position - NewPoly1.TransVtx[V0].Position;
                    const f32 T2 = (Camera.ZNearClip - NewPoly1.TransVtx[V0].Z) / Direction.Z;

                    const f32 X02 = NewPoly1.TransVtx[V0].X + Direction.X * T2;
                    const f32 Y02 = NewPoly1.TransVtx[V0].Y + Direction.Y * T2;

                    // Put values in polygons
                    NewPoly1.TransVtx[V0].X = X01;
                    NewPoly1.TransVtx[V0].Y = Y01;
                    NewPoly1.TransVtx[V0].Z = Camera.ZNearClip;

                    NewPoly2.TransVtx[V0].X = X02;
                    NewPoly2.TransVtx[V0].Y = Y02;
                    NewPoly2.TransVtx[V0].Z = Camera.ZNearClip;

                    NewPoly2.TransVtx[V1].X = X01;
                    NewPoly2.TransVtx[V1].Y = Y01;
                    NewPoly2.TransVtx[V1].Z = Camera.ZNearClip;

                    // Recompute texture coords
                    if (NewPoly1.Attr & EPolyAttr::ShadeModeTexture)
                    {
                        VPoint2 TextureDirection = NewPoly1.TransVtx[V1].TextureCoords - NewPoly1.TransVtx[V0].TextureCoords;

                        const f32 U01 = NewPoly1.TransVtx[V0].U + TextureDirection.X * T1;
                        const f32 V01 = NewPoly1.TransVtx[V0].V + TextureDirection.Y * T1;

                        TextureDirection = NewPoly1.TransVtx[V2].TextureCoords - NewPoly1.TransVtx[V0].TextureCoords;

                        const f32 U02 = NewPoly1.TransVtx[V0].U + TextureDirection.X * T2;
                        const f32 V02 = NewPoly1.TransVtx[V0].V + TextureDirection.Y * T2;

                        NewPoly1.TransVtx[V0].U = U01;
                        NewPoly1.TransVtx[V0].V = V01;

                        NewPoly2.TransVtx[V0].U = U02;
                        NewPoly2.TransVtx[V0].V = V02;
                        NewPoly2.TransVtx[V1].U = U01;
                        NewPoly2.TransVtx[V1].V = V01;
                    }

                    // Recompute poly normal length
                    VVector4 Vec1 = NewPoly1.TransVtx[V1].Position - NewPoly1.TransVtx[V0].Position;
                    VVector4 Vec2 = NewPoly1.TransVtx[V2].Position - NewPoly1.TransVtx[V0].Position;
                    VVector4 VecNormal;

                    VVector4::Cross(Vec1, Vec2, VecNormal);
                    NewPoly1.NormalLength = VecNormal.GetLengthFast();

                    Vec1 = NewPoly2.TransVtx[V1].Position - NewPoly2.TransVtx[V0].Position;
                    Vec2 = NewPoly2.TransVtx[V2].Position - NewPoly2.TransVtx[V0].Position;
                    VecNormal;

                    VVector4::Cross(Vec1, Vec2, VecNormal);
                    NewPoly2.NormalLength = VecNormal.GetLengthFast();

                    // Finally
                    InsertPolyFace(NewPoly1);
                    InsertPolyFace(NewPoly2);
                    NumAdditionalPoly += 2;
                }
            }
        }
    }
}

void VRenderList::TransformCameraToPerspective(const VCamera& Cam)
{
    for (i32f i = 0; i < NumPoly; ++i)
    {
        VPolyFace* Poly = &PolyList[i];
        if (!Poly ||
            ~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::Clipped ||
            Poly->State & EPolyState::Backface)
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

void VRenderList::ConvertFromHomogeneous()
{
    for (i32f i = 0; i < NumPoly; ++i)
    {
        VPolyFace* Poly = &PolyList[i];
        if (!Poly ||
            ~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::Clipped ||
            Poly->State & EPolyState::Backface)
        {
            continue;
        }

        for (i32f V = 0; V < 3; ++V)
        {
            Poly->TransVtx[V].Position.DivByW();
        }
    }
}

void VRenderList::TransformPerspectiveToScreen(const VCamera& Cam)
{
    const f32 Alpha = Cam.ViewportSize.X * 0.5f - 0.5f;
    const f32 Beta = Cam.ViewportSize.Y * 0.5f - 0.5f;

    for (i32f i = 0; i < NumPoly; ++i)
    {
        VPolyFace* Poly = &PolyList[i];
        if (!Poly ||
            ~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::Clipped ||
            Poly->State & EPolyState::Backface)
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

void VRenderList::TransformCameraToScreen(const VCamera& Cam)
{
    const f32 Alpha = Cam.ViewportSize.X * 0.5f - 0.5f;
    const f32 Beta = Cam.ViewportSize.Y * 0.5f - 0.5f;

    for (i32f i = 0; i < NumPoly; ++i)
    {
        VPolyFace* Poly = &PolyList[i];
        if (!Poly ||
            ~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::Clipped ||
            Poly->State & EPolyState::Backface)
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

}
