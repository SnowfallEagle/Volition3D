#pragma once

#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/TransformType.h"
#include "Engine/Graphics/Vertex.h"

namespace Volition
{

namespace EMeshState
{
    enum
    {
        Active  = VLN_BIT(1),
        Visible = VLN_BIT(2),
        Culled  = VLN_BIT(3)
    };
}

namespace EMeshAttr
{
    enum
    {
        MultiFrame = VLN_BIT(1),
        HasTexture = VLN_BIT(2),
    };
}

namespace ECullType
{
    enum
    {
        X = VLN_BIT(1),
        Y = VLN_BIT(2),
        Z = VLN_BIT(3),
        XYZ = X | Y | Z,
    };
}

namespace ECOB
{
    enum
    {
        SwapYZ  = VLN_BIT(1),
        SwapUV  = VLN_BIT(2),
        InvertU = VLN_BIT(3),
        InvertV = VLN_BIT(4),
    };
}

VLN_DEFINE_LOG_CHANNEL(hLogObject, "Object");

class VMesh
{
public:
    static constexpr i32f NameSize = 64;
    static constexpr i32f MaxVtx = 4096;
    static constexpr i32f MaxPoly = 8192;

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

    VPoint2* TextureCoordsList;

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
        VLN_SAFE_DELETE_ARRAY(HeadLocalVtxList);
        VLN_SAFE_DELETE_ARRAY(HeadTransVtxList);
        VLN_SAFE_DELETE_ARRAY(PolyList);
        VLN_SAFE_DELETE_ARRAY(AverageRadiusList);
        VLN_SAFE_DELETE_ARRAY(MaxRadiusList);
        VLN_SAFE_DELETE_ARRAY(TextureCoordsList);
    }

    void SetFrame(i32 Frame)
    {
        if (~Attr & EMeshAttr::MultiFrame)
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

    // Call this function before do rendering
    void Reset()
    {
        // Reset object's state
        State &= ~EMeshState::Culled;

        // Restore polygons
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPoly& Poly = PolyList[I];
            if (~Poly.State & EPolyState::Active)
            {
                continue;
            }

            Poly.State &= ~(EPolyState::Clipped | EPolyState::BackFace | EPolyState::Lit);
            Poly.LitColor[2] = Poly.LitColor[1] = Poly.LitColor[0] = Poly.OriginalColor;
        }
    }

    void Init()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));

        State = EMeshState::Active | EMeshState::Visible;

        UX = { 1.0f, 0.0f, 0.0f };
        UY = { 0.0f, 1.0f, 0.0f };
        UZ = { 0.0f, 0.0f, 1.0f };

        NumFrames = 1;
        CurrentFrame = 0;
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

            VLN_NOTE(hLogObject, "\n\tFrame: %d\n\tAverage radius: %.3f\n\tMax radius: %.3f\n", FrameIndex, AverageRadius, MaxRadius);
        }
    }

    VLN_FINLINE f32 GetAverageRadius()
    {
        return AverageRadiusList[CurrentFrame];
    }
    VLN_FINLINE f32 GetMaxRadius()
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

            VLN_LOG("\tPolygon normal length [%d]: %f\n", I, PolyList[I].NormalLength);
        }
    }

    void ComputeVertexNormals()
    {
        i32* NumPolyTouchVtx = new i32[NumVtx];
        Memory.MemSetQuad(NumPolyTouchVtx, 0, NumVtx);

        for (i32f I = 0; I < NumPoly; ++I)
        {
            if (PolyList[I].Attr & EPolyAttr::ShadeModeGouraud)
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

                LocalVtxList[I].Attr |= EVertexAttr::HasNormal;
                TransVtxList[I].Attr = LocalVtxList[I].Attr;

                VLN_NOTE(hLogObject, "Vertex normal [%d]: ", I);
                LocalVtxList[I].Normal.Print();
                VLN_LOG("\n");
            }
        }

        delete[] NumPolyTouchVtx;
    }

    b32 LoadCOB(
        const char* Path,
        const VVector4& InPosition = { 0.0f, 0.0f, 0.0f, 0.0f },
        const VVector4& Scale      = { 1.0f, 1.0f, 1.0f, 1.0f },
        u32 Flags = 0
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

                if (LocalVtxList[I].Attr & EVertexAttr::HasNormal)
                {
                    VVector4::MulMat44(LocalVtxList[I].Normal, M, Res);
                    LocalVtxList[I].Normal = Res;
                }
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(TransVtxList[I].Position, M, Res);
                TransVtxList[I].Position = Res;

                if (TransVtxList[I].Attr & EVertexAttr::HasNormal)
                {
                    VVector4::MulMat44(TransVtxList[I].Normal, M, Res);
                    TransVtxList[I].Normal = Res;
                }
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4::MulMat44(LocalVtxList[I].Position, M, TransVtxList[I].Position);

                if (LocalVtxList[I].Attr & EVertexAttr::HasNormal)
                {
                    VVector4::MulMat44(LocalVtxList[I].Normal, M, TransVtxList[I].Normal);
                }
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
                TransVtxList[I] = LocalVtxList[I];
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
                State |= EMeshState::Culled;
                return true;
            }
        }

        if (CullType & ECullType::Y)
        {
            f32 ZTest = (0.5f * Cam.ViewPlaneSize.Y) * (SpherePos.Z / Cam.ViewDist);

            if (SpherePos.Y - MaxRadius > ZTest ||  // Check Sphere's Bottom with Top side
                SpherePos.Y + MaxRadius < -ZTest)   // Check Sphere's Top with Bottom side
            {
                State |= EMeshState::Culled;
                return true;
            }
        }

        if (CullType & ECullType::Z)
        {
            if (SpherePos.Z - MaxRadius > Cam.ZFarClip ||
                SpherePos.Z + MaxRadius < Cam.ZNearClip)
            {
                State |= EMeshState::Culled;
                return true;
            }
        }

        return false;
    }
};

}
