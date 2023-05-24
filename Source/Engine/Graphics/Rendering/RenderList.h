#pragma once

#include <cstdlib>
#include "Common/Math/Minimal.h"
#include "Engine/Graphics/Types/Polygon.h"
#include "Engine/Graphics/Scene/Camera.h"
#include "Engine/Graphics/Types/TransformType.h"
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

VLN_DEFINE_LOG_CHANNEL(hLogRenderList, "RenderList");

VLN_DECL_ALIGN_SSE() class VRenderList
{
public:
    i32 MaxPoly = 0;

    i32 NumPoly = 0;
    i32 NumAdditionalPoly = 0;

    b8 bTerrain = false;

    VPolyFace* PolyList = nullptr;

public:
    VRenderList(i32 InMaxPoly)
    {
        MaxPoly = InMaxPoly;
        PolyList = new VPolyFace[InMaxPoly];
    }

    ~VRenderList()
    {
        delete[] PolyList;
    }

    b32 InsertPoly(const VPoly& Poly, const VVertex* VtxList, const VPoint2* TextureCoordsList, const VMaterial* Material);
    b32 InsertPolyFace(const VPolyFace& Poly);
    void InsertMesh(VMesh& Mesh, const VVertex* VtxList, const VMaterial* OverrideMaterial = nullptr);

    void Transform(const VMatrix44& M, ETransformType Type);

    /** LocalToTrans or TransOnly */
    void TransformModelToWorld(const VPoint4& WorldPos, ETransformType Type = ETransformType::LocalToTrans);
    void TransformWorldToCamera(const VCamera& Camera);
    void TransformCameraToPerspective(const VCamera& Cam);
    void TransformPerspectiveToScreen(const VCamera& Cam);
    void TransformCameraToScreen(const VCamera& Cam);

    /* Returns num backfaced polygons **/
    i32 RemoveBackfaces(const VCamera& Cam);

    void Light(const VCamera& Cam, const TArray<VLight>& Lights);

    /* Returns num clipped polygons **/
    i32 Clip(const VCamera& Camera, EClipFlags::Type Flags = EClipFlags::Full);

    void ConvertFromHomogeneous();

    VLN_FINLINE void ResetList()
    {
        NumPoly = 0;
        NumAdditionalPoly = 0;
    }

    void ResetStateAndSaveList();

public:
    VLN_DEFINE_ALIGN_OPERATORS_SSE()
};

}
