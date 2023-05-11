#pragma once

#include <cstdlib>
#include "Common/Math/Minimal.h"
#include "Engine/Graphics/Types/Polygon.h"
#include "Engine/Graphics/Scene/Camera.h"
#include "Engine/Graphics/Scene/TransformType.h"
#include "Engine/Graphics/Scene/Mesh.h"
#include "Engine/Graphics/Scene/Light.h"

namespace Volition
{

namespace EClipFlags
{
    enum Type
    {
        X = VLN_BIT(1),
        Y = VLN_BIT(2),
        Z = VLN_BIT(3),

        Full = X | Y | Z
    };
}

enum class ESortPolygonsMethod
{
    Average = 0,
    Near,
    Far
};

VLN_DEFINE_LOG_CHANNEL(hLogRenderList, "RenderList");

VLN_DECL_ALIGN_SSE() class VRenderList
{
public:
    static constexpr i32f MaxPoly = 524'288;

public:
    i32 NumPoly;
    i32 NumAdditionalPoly;
    b8 bTerrain = false;

    VPolyFace PolyList[MaxPoly];

public:
    b32 InsertPoly(const VPoly& Poly, const VVertex* VtxList, const VPoint2* TextureCoordsList);
    b32 InsertPolyFace(const VPolyFace& Poly);
    void InsertMesh(VMesh& Mesh, b32 bInsertLocal);

    void ResetStateAndSaveList();

    void Transform(const VMatrix44& M, ETransformType Type);

    /** LocalToTrans or TransOnly */
    void TransformModelToWorld(const VPoint4& WorldPos, ETransformType Type = ETransformType::LocalToTrans);
    void TransformWorldToCamera(const VCamera& Camera);
    void TransformCameraToPerspective(const VCamera& Cam);
    void TransformPerspectiveToScreen(const VCamera& Cam);
    void TransformCameraToScreen(const VCamera& Cam);

    void RemoveBackfaces(const VCamera& Cam);
    void Light(const VCamera& Cam, const VLight* Lights, i32 NumLights);
    void Clip(const VCamera& Camera, EClipFlags::Type Flags = EClipFlags::Full);

    void ConvertFromHomogeneous();

    VLN_FINLINE void ResetList()
    {
        NumPoly = 0;
        NumAdditionalPoly = 0;
    }

public:
    VLN_DEFINE_ALIGN_OPERATORS_SSE()
};

}
