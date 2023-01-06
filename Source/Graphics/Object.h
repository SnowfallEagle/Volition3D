/* TODO:
    - LoadCOB()
 */

#pragma once

#include "Graphics/Polygon.h"
#include "Graphics/Camera.h"
#include "Graphics/TransformType.h"
#include "Graphics/Vertex.h"

namespace EObjectState
{
    enum
    {
        Active  = BIT(1),
        Visible = BIT(2),
        Culled  = BIT(3)
    };
}

namespace EObjectAttr
{
    enum
    {
        MultiFrame  = BIT(1),
        HasTextures = BIT(2),
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

DEFINE_LOG_CHANNEL(hLogObject, "Object");

class VObject
{
public:
    static constexpr i32f NameSize = 64;
    static constexpr i32f MaxVtx = 1024;

public:
    char Name[NameSize];

    u32 State;
    u32 Attr;

    VPoint4 Position;
    VVector4 Direction;
    VVector4 UX, UY, UZ; // Local axes to track full orientation

    i32 NumFrames;
    i32 CurrentFrame;

    i32 NumVtx;
    i32 TotalNumVtx;

    VVertex* LocalVtxList;
    VVertex* TransVtxList;
    VVertex* HeadLocalVtxList;
    VVertex* HeadTransVtxList;

    i32 NumPoly;
    VPoly* PolyList;

    f32* AverageRadiusList;
    f32* MaxRadiusList;

    VSurface* Texture;
    VPoint2* TextureCoordsList;

    VMaterial* Material;

public:
    // Allocates verticies, polygons, radius lists and texture list
    void Allocate(i32 InNumVtx, i32 InNumPoly, i32 InNumFrames)
    {
        HeadLocalVtxList = LocalVtxList = new VVertex[InNumVtx * InNumFrames];
        HeadTransVtxList = TransVtxList = new VVertex[InNumVtx * InNumFrames];
        Memory.MemSetByte(HeadLocalVtxList, 0, sizeof(VVertex) * (InNumVtx * InNumFrames));
        Memory.MemSetByte(HeadTransVtxList, 0, sizeof(VVertex) * (InNumVtx * InNumFrames));

        PolyList          = new VPoly[InNumPoly];
        TextureCoordsList = new VPoint2[InNumPoly * 3];
        Memory.MemSetByte(PolyList, 0, sizeof(VPoly) * InNumPoly);
        Memory.MemSetByte(TextureCoordsList, 0, sizeof(VPoint2) * (InNumPoly * 3));

        AverageRadiusList = new f32[InNumFrames];
        MaxRadiusList     = new f32[InNumFrames];
        Memory.MemSetByte(AverageRadiusList, 0, sizeof(f32) * InNumFrames);
        Memory.MemSetByte(MaxRadiusList, 0, sizeof(f32) * InNumFrames);

        NumVtx      = InNumVtx;
        TotalNumVtx = InNumVtx * InNumFrames;
        NumPoly     = InNumPoly;
        NumFrames   = InNumFrames;
    }

    void Destroy()
    {
        SAFE_DELETE_ARRAY(HeadLocalVtxList);
        SAFE_DELETE_ARRAY(HeadTransVtxList);
        SAFE_DELETE_ARRAY(PolyList);
        SAFE_DELETE_ARRAY(AverageRadiusList);
        SAFE_DELETE_ARRAY(MaxRadiusList);
        SAFE_DELETE_ARRAY(TextureCoordsList);
    }

    void SetFrame(i32 Frame)
    {
        if (~Attr & EObjectAttr::MultiFrame)
        {
            return;
        }

        if (Frame < 0)
        {
            Frame = 0;
        }
        else if (Frame >= NumFrames)
        {
            Frame = NumFrames - 1;
        }

        CurrentFrame = Frame;

        LocalVtxList = &HeadLocalVtxList[Frame * NumVtx];
        TransVtxList = &HeadTransVtxList[Frame * NumVtx];
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

            Poly.LitColor[2] = Poly.LitColor[1] = Poly.LitColor[0] = Poly.OriginalColor;
        }
    }

    void ComputeRadius()
    {
        for (i32f FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
        {
            VVertex* VtxList = &HeadLocalVtxList[FrameIndex * NumVtx];
            f32 AverageRadius = 0.0f;
            f32 MaxRadius = 0.0f;

            for (i32f VtxIndex = 0; VtxIndex < NumVtx; ++VtxIndex)
            {
                f32 Distance = VtxList[VtxIndex].Position.GetLength();

                AverageRadius += Distance;
                if (MaxRadius < Distance)
                {
                    MaxRadius = Distance;
                }
            }
            AverageRadius /= NumVtx;

            AverageRadiusList[FrameIndex] = AverageRadius;
            MaxRadiusList[FrameIndex] = MaxRadius;

            VL_NOTE(hLogObject, "\n\tFrame: %d\n\tAverage radius: %.3f\n\tMax radius: %.3f\n", FrameIndex, AverageRadius, MaxRadius);
        }
    }

    FINLINE f32 GetAverageRadius()
    {
        return AverageRadiusList[CurrentFrame];
    }
    FINLINE f32 GetMaxRadius()
    {
        return MaxRadiusList[CurrentFrame];
    }

    void ComputePolygonNormalsLength()
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            i32f V0 = PolyList[I].VtxIndices[0];
            i32f V1 = PolyList[I].VtxIndices[1];
            i32f V2 = PolyList[I].VtxIndices[2];

            VVector4 U = LocalVtxList[V1].Position - LocalVtxList[V0].Position;
            VVector4 V = LocalVtxList[V2].Position - LocalVtxList[V0].Position;

            PolyList[I].NormalLength = VVector4::GetCross(U, V).GetLength();

            VL_NOTE(hLogObject, "Polygon normal length [%d]: %f\n", I, PolyList[I].NormalLength);
        }
    }

    void ComputeVertexNormals()
    {
        i32* NumPolyTouchVtx = new i32[NumVtx];
        Memory.MemSetByte(NumPolyTouchVtx, 0, sizeof(i32) * NumVtx); // We also could use MemSetQuad for this case, but whatever...

        for (i32f I = 0; I < NumPoly; ++I)
        {
            if (PolyList[I].Attr & EPolyAttr::ShadeModeGouraud ||
                PolyList[I].Attr & EPolyAttr::ShadeModePhong)
            {
                i32f V0 = PolyList[I].VtxIndices[0];
                i32f V1 = PolyList[I].VtxIndices[1];
                i32f V2 = PolyList[I].VtxIndices[2];

                VVector4 U = LocalVtxList[V1].Position - LocalVtxList[V0].Position;
                VVector4 V = LocalVtxList[V2].Position - LocalVtxList[V0].Position;

                VVector4 Normal;
                VVector4::Cross(U, V, Normal);

                LocalVtxList[V0].Normal += Normal;
                LocalVtxList[V1].Normal += Normal;
                LocalVtxList[V2].Normal += Normal;

                ++NumPolyTouchVtx[V0];
                ++NumPolyTouchVtx[V1];
                ++NumPolyTouchVtx[V2];
            }
        }

        for (i32f I = 0; I < NumVtx; ++I)
        {
            if (NumPolyTouchVtx[I] > 0)
            {
                LocalVtxList[I].Normal /= (f32)NumPolyTouchVtx[I];
                LocalVtxList[I].Normal.Normalize();

                LocalVtxList[I].Attr &= EVertexAttr::HasNormal;

                VL_NOTE(hLogObject, "Vertex normal [%d]: ", I);
                LocalVtxList[I].Normal.Print();
                VL_LOG("\n");
            }
        }

        delete[] NumPolyTouchVtx;
    }

    b32 LoadPLG(
        const char* Path,
        const VVector4& Pos,
        const VVector4& Scale,
        const VVector4& Rot
    );

    void Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis)
    {
        VVector4 Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(LocalVtxList[I].Position, M, Res);
                LocalVtxList[I].Position = Res;
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(TransVtxList[I].Position, M, Res);
                TransVtxList[I].Position = Res;
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(LocalVtxList[I].Position, M, Res);
                TransVtxList[I].Position = Res;
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
                TransVtxList[I].Position = LocalVtxList[I].Position + Position;
            }
        }
        else // TransOnly
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                TransVtxList[I].Position += Position;
            }
        }
    }

    b32 Cull(const VCamera& Cam, u32 CullType = ECullType::XYZ)
    {
        VVector4 SpherePos;
        VVector4::MulMat44(Position, Cam.MatCamera, SpherePos);
        f32 MaxRadius = GetMaxRadius();

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

#if 0 // Now we are doing this kind of stuff in RenderList only
    void Light(const VCamera& Cam, const VLight* Lights, i32 NumLights)
    {
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

            i32f V0 = Poly.VtxIndices[0];
            i32f V1 = Poly.VtxIndices[1];
            i32f V2 = Poly.VtxIndices[2];

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
                        TransVtxList[V1].Position - TransVtxList[V0].Position,
                        TransVtxList[V2].Position - TransVtxList[V0].Position
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
                        TransVtxList[V1].Position - TransVtxList[V0].Position,
                        TransVtxList[V2].Position - TransVtxList[V0].Position
                    );
                    VVector4 Direction = TransVtxList[V0].Position - Lights[LightIndex].Pos;

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
                        TransVtxList[V1].Position - TransVtxList[V0].Position,
                        TransVtxList[V2].Position - TransVtxList[V0].Position
                    );
                    f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);

                    if (Dot < 0)
                    {
                        // 128 used for fixed point to don't lose accuracy with integers
                        f32 Distance = (TransVtxList[V0].Position - Lights[LightIndex].Pos).GetLengthFast();
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
                        TransVtxList[V1].Position - TransVtxList[V0].Position,
                        TransVtxList[V2].Position - TransVtxList[V0].Position
                    );
                    f32 DotNormalDirection = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);

                    if (DotNormalDirection < 0)
                    {
                        VVector4 DistanceVector = TransVtxList[V0].Position - Lights[LightIndex].Pos;
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

            Poly.LitColor[0] = MAP_XRGB32(RSum, GSum, BSum);
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
                Poly.Attr & EPolyAttr::TwoSided  ||
                Poly.State & EPolyState::BackFace)
            {
                continue;
            }

            VVector4 U, V, N;
            U = TransVtxList[Poly.VtxIndices[1]].Position - TransVtxList[Poly.VtxIndices[0]].Position;
            V = TransVtxList[Poly.VtxIndices[2]].Position - TransVtxList[Poly.VtxIndices[0]].Position;

            VVector4::Cross(U, V, N);
            VVector4 View = Cam.Pos - TransVtxList[Poly.VtxIndices[0]].Position;

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
            VVector4::MulMat44(TransVtxList[I].Position, Camera.MatCamera, Res);
            TransVtxList[I].Position = Res;
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
            TransVtxList[I].Position.DivByW();
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

            i32 V0 = PolyList[I].VtxIndices[0];
            i32 V1 = PolyList[I].VtxIndices[1];
            i32 V2 = PolyList[I].VtxIndices[2];

            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)TransVtxList[V0].X, (i32)TransVtxList[V0].Y,
                (i32)TransVtxList[V1].X, (i32)TransVtxList[V1].Y,
                PolyList[I].LitColor[0]
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)TransVtxList[V1].X, (i32)TransVtxList[V1].Y,
                (i32)TransVtxList[V2].X, (i32)TransVtxList[V2].Y,
                PolyList[I].LitColor[0]
            );
            Renderer.DrawClippedLine(
                Buffer, Pitch,
                (i32)TransVtxList[V2].X, (i32)TransVtxList[V2].Y,
                (i32)TransVtxList[V0].X, (i32)TransVtxList[V0].Y,
                PolyList[I].LitColor[0]
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

            i32f V0 = PolyList[I].VtxIndices[0];
            i32f V1 = PolyList[I].VtxIndices[1];
            i32f V2 = PolyList[I].VtxIndices[2];

            Renderer.DrawTriangle(
                Buffer, Pitch,
                TransVtxList[V0].X, TransVtxList[V0].Y,
                TransVtxList[V1].X, TransVtxList[V1].Y,
                TransVtxList[V2].X, TransVtxList[V2].Y,
                PolyList[I].LitColor[0]
            );
        }
    }
#endif
};
