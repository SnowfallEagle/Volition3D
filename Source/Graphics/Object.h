#pragma once

#include "Graphics/Polygon.h"
#include "Graphics/Camera.h"
#include "Graphics/TransformType.h"

namespace EObjectStateV1
{
    enum
    {
        Active = BIT(1),
        Visible = BIT(2),
        Culled = BIT(3)
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

    f32 AvgRadius;
    f32 MaxRadius;

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
            VVector4D::MulMat44(UX, M, Res);
            UX = Res;

            VVector4D::MulMat44(UY, M, Res);
            UY = Res;

            VVector4D::MulMat44(UZ, M, Res);
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

    b32 Cull(const VCam4DV1& Cam, u32 CullType = ECullType::XYZ)
    {
        VVector4D SpherePos;
        VVector4D::MulMat44(WorldPos, Cam.MatCamera, SpherePos);

        if (CullType & ECullType::X)
        {
            f32 ZTest = (0.5f * Cam.ViewPlaneSize.X) * (SpherePos.Z / Cam.ViewDist);

            if (SpherePos.X - MaxRadius > ZTest ||  // Check Sphere's Left with Right side
                SpherePos.X + MaxRadius < -ZTest)   // Check Sphere's Right with Left side
            {
                VL_LOG("Culled X\n");
                State |= EObjectStateV1::Culled;
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
                State |= EObjectStateV1::Culled;
                return true;
            }
        }

        if (CullType & ECullType::Z)
        {
            if (SpherePos.Z - MaxRadius > Cam.ZFarClip ||
                SpherePos.Z + MaxRadius < Cam.ZNearClip)
            {
                VL_LOG("Culled Z\n");
                State |= EObjectStateV1::Culled;
                return true;
            }
        }

        return false;
    }

    void Light(const VCam4DV1& Cam, const VLightV1* Lights, i32 NumLights)
    {
        // NOTE(sean): We can simplify this stuff by converting calculations to floating point

        if (~State & EObjectStateV1::Active ||
            State & EObjectStateV1::Culled  ||
            ~State & EObjectStateV1::Visible)
        {
            return;
        }

        for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
        {
            VPoly4DV1& Poly = PolyList[PolyIndex];
            if (~Poly.State & EPolyStateV1::Active ||
                Poly.State & EPolyStateV1::Clipped ||
                Poly.State & EPolyStateV1::BackFace)
            {
                continue;
            }

            if (~Poly.Attr & EPolyAttrV1::ShadeModeFlat &&
                ~Poly.Attr & EPolyAttrV1::ShadeModeGouraud)
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
                if (~Lights[LightIndex].State & ELightStateV1::Active)
                {
                    continue;
                }

                if (Lights[LightIndex].Attr & ELightAttrV1::Ambient)
                {
                    // NOTE(sean): Maybe 255? or even >> 8
                    RSum += (Poly.OriginalColor.R * Lights[LightIndex].CAmbient.R) / 256;
                    GSum += (Poly.OriginalColor.G * Lights[LightIndex].CAmbient.G) / 256;
                    BSum += (Poly.OriginalColor.B * Lights[LightIndex].CAmbient.B) / 256;
                }
                else if (Lights[LightIndex].Attr & ELightAttrV1::Infinite)
                {
                    VVector4D SurfaceNormal = VVector4D::GetCross(
                        Poly.VtxList[V1] - Poly.VtxList[V0],
                        Poly.VtxList[V2] - Poly.VtxList[V0]
                    );

                    f32 Dot = VVector4D::Dot(SurfaceNormal, Lights[LightIndex].Dir);
                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        i32 Intensity = (i32)( 128.0f * (Math.Abs(Dot) / SurfaceNormal.GetLengthFast()) );
                        RSum += (Poly.OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                        GSum += (Poly.OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                        BSum += (Poly.OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                    }
                }
            }

            if (RSum > 255) RSum = 255;
            if (GSum > 255) GSum = 255;
            if (BSum > 255) BSum = 255;

            Poly.FinalColor = MAP_XRGB32(RSum, GSum, BSum);
        }
    }

    void RemoveBackFaces(VCam4DV1 Cam)
    {
        if (State & EObjectStateV1::Culled)
        {
            return;
        }

        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPoly4DV1& Poly = PolyList[I];

            if (~Poly.State & EPolyStateV1::Active ||
                Poly.State & EPolyStateV1::Clipped ||
                Poly.Attr & EPolyAttrV1::TwoSided ||
                Poly.State & EPolyStateV1::BackFace)
            {
                continue;
            }

            VVector4D U, V, N;
            U = TransVtxList[Poly.Vtx[1]] - TransVtxList[Poly.Vtx[0]];
            V = TransVtxList[Poly.Vtx[2]] - TransVtxList[Poly.Vtx[0]];

            VVector4D::Cross(U, V, N);
            VVector4D View = Cam.Pos - TransVtxList[Poly.Vtx[0]];

            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4D::Dot(View, N) <= 0.0f)
            {
                Poly.State |= EPolyStateV1::BackFace;
            }
        }
    }

    void TransformWorldToCamera(const VCam4DV1& Camera)
    {
        for (i32f I = 0; I < NumVtx; ++I)
        {
            VVector4D Res;
            VVector4D::MulMat44(TransVtxList[I], Camera.MatCamera, Res);
            TransVtxList[I] = Res;
        }
    }

    void TransformCameraToPerspective(const VCam4DV1& Cam)
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

    void TransformPerspectiveToScreen(const VCam4DV1& Cam)
    {
        f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

        for (i32f I = 0; I < NumVtx; ++I)
        {
            TransVtxList[I].X = Alpha + Alpha * TransVtxList[I].X;
            TransVtxList[I].Y = Beta - Beta * TransVtxList[I].Y;
        }
    }

    void TransformCameraToScreen(const VCam4DV1& Cam)
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
        State &= ~EObjectStateV1::Culled;

        // Restore polygons
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPoly4DV1& Poly = PolyList[I];
            if (~Poly.State & EPolyStateV1::Active)
            {
                continue;
            }

            Poly.State &= ~EPolyStateV1::Clipped;
            Poly.State &= ~EPolyStateV1::BackFace;

            Poly.FinalColor = Poly.OriginalColor;
        }
    }

    void RenderWire(u32* Buffer, i32 Pitch)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            if (~PolyList[I].State & EPolyStateV1::Active ||
                PolyList[I].State & EPolyStateV1::Clipped ||
                PolyList[I].State & EPolyStateV1::BackFace)
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
            if (~PolyList[I].State & EPolyStateV1::Active ||
                PolyList[I].State & EPolyStateV1::Clipped ||
                PolyList[I].State & EPolyStateV1::BackFace)
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
