#pragma once

#include "Graphics/Polygon.h"
#include "Graphics/Camera.h"
#include "Graphics/TransformType.h"

namespace EObjectState
{
    enum
    {
        Active = BIT(1),
        Visible = BIT(2),
        Culled = BIT(3)
    };
}

namespace EObjectAttr
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

namespace ECullType
{
    enum
    {
        X = BIT(1),
        Y = BIT(2),
        Z = BIT(3),
        XYZ = X | Y | Z,
    };
}

DEFINE_LOG_CHANNEL(hObjectV1Log, "ObjectV1");

class VObject
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

    f32 AvgRadius;
    f32 MaxRadius;

    VPoint4 WorldPos;
    VVector4 Dir;
    VVector4 UX, UY, UZ; // Local axes to track full orientation

    i32 NumVtx;
    VPoint4 LocalVtxList[MaxVtx];
    VPoint4 TransVtxList[MaxVtx];

    i32 NumPoly;
    VPoly PolyList[MaxPoly];

public:
    b32 LoadPLG(
        const char* Path,
        const VVector4& Pos,
        const VVector4& Scale,
        const VVector4& Rot
    );

    static char* GetLinePLG(FILE* File, char* Buffer, i32 Size)
    {
        for (;;)
        {
            if (!fgets(Buffer, Size, File))
            {
                return nullptr;
            }

            i32f I, Len = strlen(Buffer);
            for (I = 0; I < Len && (Buffer[I] == ' ' || Buffer[I] == '\t' || Buffer[I] == '\r' || Buffer[I] == '\n'); ++I)
                {}

            if (I < Len && Buffer[I] != '#')
            {
                return &Buffer[I];
            }
        }
    }

    void ComputeRadius()
    {
        AvgRadius = 0.0f;
        MaxRadius = 0.0f;

        for (i32f I = 0; I < NumVtx; ++I)
        {
            f32 Dist = LocalVtxList[I].GetLength();
            AvgRadius += Dist;
            if (MaxRadius < Dist)
            {
                MaxRadius = Dist;
            }
        }

        AvgRadius /= NumVtx;
    }

    void Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis)
    {
        VVector4 Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(LocalVtxList[I], M, Res);
                LocalVtxList[I] = Res;
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(TransVtxList[I], M, Res);
                TransVtxList[I] = Res;
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(LocalVtxList[I], M, Res);
                TransVtxList[I] = Res;
            }
        } break;
        }

        if (bTransBasis)
        {
            VVector4::MulMat44(UX, M, Res);
            UX = Res;

            VVector4::MulMat44(UY, M, Res);
            UY = Res;

            VVector4::MulMat44(UZ, M, Res);
            UZ = Res;
        }
    }

    // LocalToTrans or TransOnly
    void TransformModelToWorld(ETransformType Type = ETransformType::LocalToTrans)
    {
        if (Type == ETransformType::LocalToTrans)
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                TransVtxList[I] = LocalVtxList[I] + WorldPos;
            }
        }
        else // TransOnly
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                TransVtxList[I] += WorldPos;
            }
        }
    }

    b32 Cull(const VCamera& Cam, u32 CullType = ECullType::XYZ)
    {
        VVector4 SpherePos;
        VVector4::MulMat44(WorldPos, Cam.MatCamera, SpherePos);

        if (CullType & ECullType::X)
        {
            f32 ZTest = (0.5f * Cam.ViewPlaneSize.X) * (SpherePos.Z / Cam.ViewDist);

            if (SpherePos.X - MaxRadius > ZTest ||  // Check Sphere's Left with Right side
                SpherePos.X + MaxRadius < -ZTest)   // Check Sphere's Right with Left side
            {
                VL_LOG("Culled X\n");
                State |= EObjectState::Culled;
                return true;
            }
        }

        if (CullType & ECullType::Y)
        {
            f32 ZTest = (0.5f * Cam.ViewPlaneSize.Y) * (SpherePos.Z / Cam.ViewDist);

            if (SpherePos.Y - MaxRadius > ZTest ||  // Check Sphere's Bottom with Top side
                SpherePos.Y + MaxRadius < -ZTest)   // Check Sphere's Top with Bottom side
            {
                VL_LOG("Culled Y\n");
                State |= EObjectState::Culled;
                return true;
            }
        }

        if (CullType & ECullType::Z)
        {
            if (SpherePos.Z - MaxRadius > Cam.ZFarClip ||
                SpherePos.Z + MaxRadius < Cam.ZNearClip)
            {
                VL_LOG("Culled Z\n");
                State |= EObjectState::Culled;
                return true;
            }
        }

        return false;
    }

    void Light(const VCamera& Cam, const VLight* Lights, i32 NumLights)
    {
        // NOTE(sean): We can simplify this stuff by converting calculations to floating point

        if (~State & EObjectState::Active ||
            State & EObjectState::Culled  ||
            ~State & EObjectState::Visible)
        {
            return;
        }

        for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
        {
            VPoly& Poly = PolyList[PolyIndex];
            if (~Poly.State & EPolyState::Active ||
                Poly.State & EPolyState::Clipped ||
                Poly.State & EPolyState::BackFace)
            {
                continue;
            }

            if (~Poly.Attr & EPolyAttr::ShadeModeFlat &&
                ~Poly.Attr & EPolyAttr::ShadeModeGouraud)
            {
                continue; // As emissive, no changes
            }

            i32f V0 = Poly.Vtx[0];
            i32f V1 = Poly.Vtx[1];
            i32f V2 = Poly.Vtx[2];

            u32 RSum = 0;
            u32 GSum = 0;
            u32 BSum = 0;

            for (i32f LightIndex = 0; LightIndex < NumLights; ++LightIndex)
            {
                if (~Lights[LightIndex].State & ELightState::Active)
                {
                    continue;
                }

                if (Lights[LightIndex].Attr & ELightAttr::Ambient)
                {
                    // NOTE(sean): Maybe 255? or even >> 8
                    RSum += (Poly.OriginalColor.R * Lights[LightIndex].CAmbient.R) / 256;
                    GSum += (Poly.OriginalColor.G * Lights[LightIndex].CAmbient.G) / 256;
                    BSum += (Poly.OriginalColor.B * Lights[LightIndex].CAmbient.B) / 256;
                }
                else if (Lights[LightIndex].Attr & ELightAttr::Infinite)
                {
                    VVector4 SurfaceNormal = VVector4::GetCross(
                        Poly.VtxList[V1] - Poly.VtxList[V0],
                        Poly.VtxList[V2] - Poly.VtxList[V0]
                    );

                    f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        i32 Intensity = (i32)( 128.0f * (Math.Abs(Dot) / SurfaceNormal.GetLengthFast()) );
                        RSum += (Poly.OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum += (Poly.OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum += (Poly.OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::Point)
                {
                    VVector4 SurfaceNormal = VVector4::GetCross(
                        Poly.VtxList[V1] - Poly.VtxList[V0],
                        Poly.VtxList[V2] - Poly.VtxList[V0]
                    );
                    VVector4 Direction = Poly.VtxList[V0] - Lights[LightIndex].Pos;

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
                            (128.0f * Math.Abs(Dot)) / (SurfaceNormal.GetLengthFast() * Distance * Atten)
                        );

                        RSum += (Poly.OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum += (Poly.OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum += (Poly.OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::SimpleSpotlight)
                {
                    // FIXME(sean): I think we should check if dot of normal and vector between surface and position is negative

                    VVector4 SurfaceNormal = VVector4::GetCross(
                        Poly.VtxList[V1] - Poly.VtxList[V0],
                        Poly.VtxList[V2] - Poly.VtxList[V0]
                    );
                    f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);

                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        f32 Distance = (TransVtxList[V0] - Lights[LightIndex].Pos).GetLengthFast();
                        f32 Atten =
                            Lights[LightIndex].KConst +
                            Lights[LightIndex].KLinear * Distance +
                            Lights[LightIndex].KQuad * Distance * Distance;
                        i32 Intensity = (i32)(
                            (128.0f * Math.Abs(Dot)) / (SurfaceNormal.GetLengthFast() * Atten)
                        );

                        RSum += (Poly.OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum += (Poly.OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum += (Poly.OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
                else if (Lights[LightIndex].Attr & ELightAttr::ComplexSpotlight)
                {
                    VVector4 SurfaceNormal = VVector4::GetCross(
                        Poly.VtxList[V1] - Poly.VtxList[V0],
                        Poly.VtxList[V2] - Poly.VtxList[V0]
                    );
                    f32 DotNormalDirection = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);

                    if (DotNormalDirection < 0)
                    {
                        VVector4 DistanceVector = TransVtxList[V0] - Lights[LightIndex].Pos;
                        f32 Distance = DistanceVector.GetLengthFast();
                        f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].Dir) / Distance;

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
                                (SurfaceNormal.GetLengthFast() * Atten)
                            );

                            RSum += (Poly.OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum += (Poly.OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum += (Poly.OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                }
            }

            if (RSum > 255) RSum = 255;
            if (GSum > 255) GSum = 255;
            if (BSum > 255) BSum = 255;

            Poly.FinalColor = MAP_XRGB32(RSum, GSum, BSum);
        }
    }

    void RemoveBackFaces(VCamera Cam)
    {
        if (State & EObjectState::Culled)
        {
            return;
        }

        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPoly& Poly = PolyList[I];

            if (~Poly.State & EPolyState::Active ||
                Poly.State & EPolyState::Clipped ||
                Poly.Attr & EPolyAttr::TwoSided ||
                Poly.State & EPolyState::BackFace)
            {
                continue;
            }

            VVector4 U, V, N;
            U = TransVtxList[Poly.Vtx[1]] - TransVtxList[Poly.Vtx[0]];
            V = TransVtxList[Poly.Vtx[2]] - TransVtxList[Poly.Vtx[0]];

            VVector4::Cross(U, V, N);
            VVector4 View = Cam.Pos - TransVtxList[Poly.Vtx[0]];

            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4::Dot(View, N) <= 0.0f)
            {
                Poly.State |= EPolyState::BackFace;
            }
        }
    }

    void TransformWorldToCamera(const VCamera& Camera)
    {
        for (i32f I = 0; I < NumVtx; ++I)
        {
            VVector4 Res;
            VVector4::MulMat44(TransVtxList[I], Camera.MatCamera, Res);
            TransVtxList[I] = Res;
        }
    }

    void TransformCameraToPerspective(const VCamera& Cam)
    {
        for (i32f I = 0; I < NumVtx; ++I)
        {
            f32 Z = TransVtxList[I].Z;

            TransVtxList[I].X = TransVtxList[I].X * (Cam.ViewDist / Z);
            TransVtxList[I].Y = TransVtxList[I].Y * Cam.AspectRatio * (Cam.ViewDist / Z);
            // Z = Z
        }
    }

    void ConvertFromHomogeneous()
    {
        for (i32f I = 0; I < NumVtx; ++I)
        {
            TransVtxList[I].DivByW();
        }
    }

    void TransformPerspectiveToScreen(const VCamera& Cam)
    {
        f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

        for (i32f I = 0; I < NumVtx; ++I)
        {
            TransVtxList[I].X = Alpha + Alpha * TransVtxList[I].X;
            TransVtxList[I].Y = Beta - Beta * TransVtxList[I].Y;
        }
    }

    void TransformCameraToScreen(const VCamera& Cam)
    {
        f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

        for (i32f I = 0; I < NumVtx; ++I)
        {
            f32 ViewDistDivZ = Cam.ViewDist / TransVtxList[I].Z;

            TransVtxList[I].X *= ViewDistDivZ;
            TransVtxList[I].Y *= Cam.AspectRatio * ViewDistDivZ;

            TransVtxList[I].X = Alpha + Alpha * TransVtxList[I].X;
            TransVtxList[I].Y = Beta - Beta * TransVtxList[I].Y;
        }
    }

    void Reset()
    {
        // Reset object's state
        State &= ~EObjectState::Culled;

        // Restore polygons
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPoly& Poly = PolyList[I];
            if (~Poly.State & EPolyState::Active)
            {
                continue;
            }

            Poly.State &= ~EPolyState::Clipped;
            Poly.State &= ~EPolyState::BackFace;

            Poly.FinalColor = Poly.OriginalColor;
        }
    }

    void RenderWire(u32* Buffer, i32 Pitch)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            if (~PolyList[I].State & EPolyState::Active ||
                PolyList[I].State & EPolyState::Clipped ||
                PolyList[I].State & EPolyState::BackFace)
            {
                continue;
            }

            i32 V0 = PolyList[I].Vtx[0];
            i32 V1 = PolyList[I].Vtx[1];
            i32 V2 = PolyList[I].Vtx[2];

            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)TransVtxList[V0].X, (i32)TransVtxList[V0].Y,
                (i32)TransVtxList[V1].X, (i32)TransVtxList[V1].Y,
                PolyList[I].FinalColor
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)TransVtxList[V1].X, (i32)TransVtxList[V1].Y,
                (i32)TransVtxList[V2].X, (i32)TransVtxList[V2].Y,
                PolyList[I].FinalColor
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)TransVtxList[V2].X, (i32)TransVtxList[V2].Y,
                (i32)TransVtxList[V0].X, (i32)TransVtxList[V0].Y,
                PolyList[I].FinalColor
            );
        }
    }

    void RenderSolid(u32* Buffer, i32 Pitch)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            if (~PolyList[I].State & EPolyState::Active ||
                PolyList[I].State & EPolyState::Clipped ||
                PolyList[I].State & EPolyState::BackFace)
            {
                continue;
            }

            i32f V0 = PolyList[I].Vtx[0];
            i32f V1 = PolyList[I].Vtx[1];
            i32f V2 = PolyList[I].Vtx[2];

            Renderer.DrawTriangle(
                Buffer, Pitch,
                (i32)TransVtxList[V0].X, (i32)TransVtxList[V0].Y,
                (i32)TransVtxList[V1].X, (i32)TransVtxList[V1].Y,
                (i32)TransVtxList[V2].X, (i32)TransVtxList[V2].Y,
                PolyList[I].FinalColor
            );
        }
    }
};
