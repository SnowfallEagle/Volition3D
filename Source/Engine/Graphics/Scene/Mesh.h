#pragma once

#include "Engine/Graphics/Types/Vertex.h"
#include "Engine/Graphics/Types/Polygon.h"
#include "Engine/Graphics/Scene/Camera.h"
#include "Engine/Graphics/Scene/TransformType.h"

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
        MultiFrame  = VLN_BIT(1),
        HasTexture  = VLN_BIT(2),
        CanBeCulled = VLN_BIT(3),
        TerrainMesh = VLN_BIT(4)
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

VLN_DECL_ALIGN_SSE() class VMesh
{
public:
    static constexpr i32f NameSize = 64;
    static constexpr i32f MaxDefaultPoly = 32768;
    static constexpr i32f MaxTerrainPoly = 262'144;

public:
    char Name[NameSize];

    u32 State;
    u32 Attr;

    VPoint4 Position;
    VVector4 UX, UY, UZ; /** Local axes to track full orientation */

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
    VMesh()
    {
        Memory.MemSetByte(this, 0, sizeof(*this));

        State = EMeshState::Active | EMeshState::Visible;

        UX = { 1.0f, 0.0f, 0.0f };
        UY = { 0.0f, 1.0f, 0.0f };
        UZ = { 0.0f, 0.0f, 1.0f };

        NumFrames = 1;
        CurrentFrame = 0;
    }

    /** Allocates verticies, polygons, radius lists and texture list */
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

    /** Called every time before rendering */
    void ResetRenderState()
    {
        // Reset object's state
        State &= ~EMeshState::Culled;

        // Restore polygons
        for (i32f i = 0; i < NumPoly; ++i)
        {
            VPoly& Poly = PolyList[i];
            if (~Poly.State & EPolyState::Active)
            {
                continue;
            }

            Poly.State &= ~(EPolyState::Clipped | EPolyState::BackFace | EPolyState::Lit);
            Poly.LitColor[2] = Poly.LitColor[1] = Poly.LitColor[0] = Poly.OriginalColor;
        }
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

    void ComputeRadius()
    {
        for (i32f FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
        {
            const VVertex* VtxList = &HeadLocalVtxList[FrameIndex * NumVtx];
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

            VLN_LOG_VERBOSE("\n\tFrame: %d\n\tAverage radius: %.3f\n\tMax radius: %.3f\n", FrameIndex, AverageRadius, MaxRadius);
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
        for (i32f i = 0; i < NumPoly; ++i)
        {
            const i32f V0 = PolyList[i].VtxIndices[0];
            const i32f V1 = PolyList[i].VtxIndices[1];
            const i32f V2 = PolyList[i].VtxIndices[2];

            const VVector4 U = LocalVtxList[V1].Position - LocalVtxList[V0].Position;
            const VVector4 V = LocalVtxList[V2].Position - LocalVtxList[V0].Position;

            PolyList[i].NormalLength = VVector4::GetCross(U, V).GetLength();

            VLN_LOG_VERBOSE("\tPolygon normal length [%d]: %f\n", I, PolyList[i].NormalLength);
        }
    }

    void ComputeVertexNormals()
    {
        i32* NumPolyTouchVtx = new i32[NumVtx];
        Memory.MemSetQuad(NumPolyTouchVtx, 0, NumVtx);

        for (i32f i = 0; i < NumPoly; ++i)
        {
            if (PolyList[i].Attr & EPolyAttr::ShadeModeGouraud)
            {
                const i32f V0 = PolyList[i].VtxIndices[0];
                const i32f V1 = PolyList[i].VtxIndices[1];
                const i32f V2 = PolyList[i].VtxIndices[2];

                const VVector4 U = LocalVtxList[V1].Position - LocalVtxList[V0].Position;
                const VVector4 V = LocalVtxList[V2].Position - LocalVtxList[V0].Position;

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

        for (i32f i = 0; i < NumVtx; ++i)
        {
            if (NumPolyTouchVtx[i] > 0)
            {
                LocalVtxList[i].Normal /= (f32)NumPolyTouchVtx[i];
                LocalVtxList[i].Normal.Normalize();

                LocalVtxList[i].Attr |= EVertexAttr::HasNormal;
                TransVtxList[i].Attr = LocalVtxList[i].Attr;

                VLN_LOG_VERBOSE("Vertex normal [%d]: <%.2f %.2f %.2f>\n", I, LocalVtxList[i].Normal.X, LocalVtxList[i].Normal.Y, LocalVtxList[i].Normal.Z);
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

    void GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size, f32 Height);

    void Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis)
    {
        VVector4 Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f i = 0; i < NumVtx; ++i)
            {
                VMatrix44::MulVecMat(LocalVtxList[i].Position, M, Res);
                LocalVtxList[i].Position = Res;

                if (LocalVtxList[i].Attr & EVertexAttr::HasNormal)
                {
                    VMatrix44::MulVecMat(LocalVtxList[i].Normal, M, Res);
                    LocalVtxList[i].Normal = Res;
                }
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f i = 0; i < NumVtx; ++i)
            {
                VMatrix44::MulVecMat(TransVtxList[i].Position, M, Res);
                TransVtxList[i].Position = Res;

                if (TransVtxList[i].Attr & EVertexAttr::HasNormal)
                {
                    VMatrix44::MulVecMat(TransVtxList[i].Normal, M, Res);
                    TransVtxList[i].Normal = Res;
                }
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f i = 0; i < NumVtx; ++i)
            {
                VMatrix44::MulVecMat(LocalVtxList[i].Position, M, TransVtxList[i].Position);

                if (LocalVtxList[i].Attr & EVertexAttr::HasNormal)
                {
                    VMatrix44::MulVecMat(LocalVtxList[i].Normal, M, TransVtxList[i].Normal);
                }
            }
        } break;
        }

        if (bTransBasis)
        {
            VMatrix44::MulVecMat(UX, M, Res);
            UX = Res;

            VMatrix44::MulVecMat(UY, M, Res);
            UY = Res;

            VMatrix44::MulVecMat(UZ, M, Res);
            UZ = Res;
        }
    }

    /** LocalToTrans or TransOnly */
    void TransformModelToWorld(ETransformType Type = ETransformType::LocalToTrans)
    {
        if (Type == ETransformType::LocalToTrans)
        {
            for (i32f i = 0; i < NumVtx; ++i)
            {
                TransVtxList[i] = LocalVtxList[i];
                TransVtxList[i].Position = LocalVtxList[i].Position + Position;
            }
        }
        else // TransOnly
        {
            for (i32f i = 0; i < NumVtx; ++i)
            {
                TransVtxList[i].Position += Position;
            }
        }
    }

    b32 Cull(const VCamera& Cam, u32 CullType = ECullType::XYZ)
    {
        if (~Attr & EMeshAttr::CanBeCulled)
        {
            return false;
        }

        VVector4 SpherePos;
        VMatrix44::MulVecMat(Position, Cam.MatCamera, SpherePos);
        const f32 MaxRadius = GetMaxRadius();

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

    VLN_FINLINE i32f GetMaxPoly()
    {
        return Attr & EMeshAttr::TerrainMesh ? MaxTerrainPoly : MaxDefaultPoly;
    }

public:
    VLN_DEFINE_ALIGN_OPERATORS_SSE()
};

}
