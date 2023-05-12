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
        Active   = VLN_BIT(1),
        Visible  = VLN_BIT(2),
        Culled   = VLN_BIT(3),
    };
}

namespace EMeshAttr
{
    enum
    {
        MultiFrame  = VLN_BIT(1),
        HasTexture  = VLN_BIT(2),
        CanBeCulled = VLN_BIT(3),
        CastShadow  = VLN_BIT(4),
        TerrainMesh = VLN_BIT(5)
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

namespace ECOBFlags
{
    enum
    {
        SwapYZ  = VLN_BIT(1),
        SwapUV  = VLN_BIT(2),
        InvertU = VLN_BIT(3),
        InvertV = VLN_BIT(4),

        Default = SwapYZ
    };
}

namespace EMD2Flags
{
    enum
    {
        ShadeModeFlat = VLN_BIT(1),

        Default = 0
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
    f32 CurrentFrame;

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

    i32 NumTextureCoords;
    VPoint2* TextureCoordsList;

public:
    VMesh();

    /**
        Allocates verticies, polygons, radius lists and texture list.
        If not specified InNumTextureCoords = InNumPoly * 3
    */
    void Allocate(i32 InNumVtx, i32 InNumPoly, i32 InNumFrames, i32 InNumTextureCoords = -1);
    void Destroy();

    /** Called every time before rendering */
    void ResetRenderState();

    void ComputeRadius();
    void ComputePolygonNormalsLength();
    void ComputeVertexNormals();

    b32 LoadMD2(
        const char* Path,
        const char* InSkinPath = nullptr,
        i32 SkinIndex = 0, // Used if SpecificSkinPath != nullptr
        VVector4 InPosition = { 0.0f, 0.0f, 0.0f },
        VVector3 InScale = { 1.0f, 1.0f, 1.0f },
        u32 Flags = EMD2Flags::Default
    );

    b32 LoadCOB(
        const char* Path,
        const VVector4& InPosition = { 0.0f, 0.0f, 0.0f, 0.0f },
        const VVector4& Scale      = { 1.0f, 1.0f, 1.0f, 1.0f },
        u32 Flags = ECOBFlags::Default
    );

    void UpdateAnimation();

    /** LocalToTrans or TransOnly */
    void TransformModelToWorld(ETransformType Type = ETransformType::LocalToTrans);
    void Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis);
    b32 Cull(const VCamera& Cam, u32 CullType = ECullType::XYZ);

    void GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size, f32 Height);

    VLN_FINLINE i32f GetMaxPoly()
    {
        return Attr & EMeshAttr::TerrainMesh ? MaxTerrainPoly : MaxDefaultPoly;
    }

    VLN_FINLINE f32 GetAverageRadius()
    {
        return AverageRadiusList[(i32f)CurrentFrame];
    }

    VLN_FINLINE f32 GetMaxRadius()
    {
        return MaxRadiusList[(i32f)CurrentFrame];
    }

public:
    VLN_DEFINE_ALIGN_OPERATORS_SSE()
};

}
