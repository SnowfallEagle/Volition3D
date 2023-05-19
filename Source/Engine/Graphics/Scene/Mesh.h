#pragma once

#include "Engine/Graphics/Types/Vertex.h"
#include "Engine/Graphics/Types/Polygon.h"
#include "Engine/Graphics/Scene/Camera.h"
#include "Engine/Graphics/Types/TransformType.h"

namespace Volition
{

namespace EMeshState
{
    enum Type
    {
        Active   = VLN_BIT(1),
        Visible  = VLN_BIT(2),
        Culled   = VLN_BIT(3),
    };
}

namespace EMeshAttr
{
    enum Type
    {
        MultiFrame  = VLN_BIT(1),
        CanBeCulled = VLN_BIT(2),
        CastShadow  = VLN_BIT(3),
        TerrainMesh = VLN_BIT(4),
    };
}

namespace ECullType
{
    enum Type
    {
        X = VLN_BIT(1),
        Y = VLN_BIT(2),
        Z = VLN_BIT(3),

        XYZ = X | Y | Z,
    };
}

namespace ECOBFlags
{
    enum Type
    {
        SwapYZ  = VLN_BIT(1),
        SwapUV  = VLN_BIT(2),
        InvertU = VLN_BIT(3),
        InvertV = VLN_BIT(4),

        OverrideShadeMode = VLN_BIT(5),

        Default = SwapYZ
    };
}

enum class EShadeMode
{
    Emissive = EMaterialAttr::ShadeModeEmissive,
    Flat     = EMaterialAttr::ShadeModeFlat,
    Gouraud  = EMaterialAttr::ShadeModeGouraud,
};

enum class EMD2AnimationId : u8
{
    StandingIdle = 0,
    Run,
    Attack,
    Pain1,
    Pain2,
    Pain3,
    Jump,
    Flip,
    Salute,
    Taunt,
    Wave,
    Point,
    CrouchStand,
    CrouchWalk,
    CrouchAttack,
    CrouchPain,
    CrouchDeath,
    DeathBack,
    DeathForward,
    DeathSlow,

    MaxAnimations
};

enum class EAnimationInterpMode : u8
{
    Default = 0, // Get from animation table

    Linear,      // Smooth, but MD2 models can be jerky in transitions between looping animations
    Fixed,       // Not smooth, but good for looping MD2 animations
};

VLN_DEFINE_LOG_CHANNEL(hLogObject, "Object");

VLN_DECL_ALIGN_SSE() class VMesh
{
public:
    static constexpr i32f MaxMaterialsPerModel = 256;

public:
    char Name[64];

    u32 State;
    u32 Attr;

    VPoint4 Position;
    VVector4 UX, UY, UZ; /** Local axes to track full orientation */

    i32 NumFrames;
    f32 CurrentFrame;

    EMD2AnimationId CurrentAnimationId;
    EAnimationInterpMode AnimationInterpMode;
    f32 AnimationTimeAccum;

    b8 bLoopAnimation    : 1;
    b8 bAnimationPlayed  : 1;

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
        VVector3 InScale    = { 1.0f, 1.0f, 1.0f },
        EShadeMode ShadeMode = EShadeMode::Gouraud,
        const VVector3& ColorCorrection = { 1.0f, 1.0f, 1.0f }
    );

    b32 LoadCOB(
        const char* Path,
        const VVector4& InPosition = { 0.0f, 0.0f, 0.0f, 0.0f },
        const VVector4& Scale      = { 1.0f, 1.0f, 1.0f, 1.0f },
        u32 Flags = ECOBFlags::Default,
        EShadeMode OverrideShadeMode = EShadeMode::Gouraud // Only with ECobFlags::OverrideShadeMode
    );

    void PlayAnimation(EMD2AnimationId AnimationId, b32 bLoop = false, EAnimationInterpMode InterpMode = EAnimationInterpMode::Default);
    void UpdateAnimationAndTransformModelToWorld(f32 DeltaTime);

    /** LocalToTrans or TransOnly */
    void TransformModelToWorld(ETransformType Type = ETransformType::LocalToTrans);
    void Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis);
    b32 Cull(const VCamera& Cam, u32 CullType = ECullType::XYZ);

    void GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size, f32 Height, EShadeMode ShadeMode = EShadeMode::Gouraud);

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
