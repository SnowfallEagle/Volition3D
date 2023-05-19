#pragma once

#include "Common/Types/Common.h"
#include "Common/Math/Minimal.h"

namespace Volition
{

enum class ERotateSeq
{
    YXZ = 0,
    ZXY
};

enum class EUVNMode
{
    Simple = 0,
    Spherical
};

namespace ECameraAttr
{
    enum Type
    {
        Euler = VLN_BIT(1),
        UVN = VLN_BIT(2)
    };
}

VLN_DECL_ALIGN_SSE() class VCamera
{
public:
    u32 Attr;

    VPoint4 Position;
    VVector4 Direction; /** Euler angles or look at direction */

    VVector4 U, V, N;
    VPoint4 Target;

    f32 FOV;
    f32 AspectRatio;
    f32 ViewDist;

    f32 ZNearClip;
    f32 ZFarClip;

    VVector2 ViewplaneSize;
    VVector2 ViewportSize;
    VVector2 ViewportCenter;

    VPlane3 LeftClipPlane;
    VPlane3 RightClipPlane;
    VPlane3 TopClipPlane;
    VPlane3 BottomClipPlane;

    VMatrix44 MatCamera;             /** World->Camera */
    VMatrix44 MatCameraRotationOnly; /** World->Camera but only for Normals or Directions */
    VMatrix44 MatPerspective;        /** Camera->Perspective */
    VMatrix44 MatScreen;             /** Perspective->Screen */

public:
    void Init(
        u32 InAttr,
        const VPoint4& InPos,
        const VVector4& InDir,
        const VPoint4& InTarget,
        f32 InFOV,
        f32 InZNearClip,
        f32 InZFarClip
    );

    void BuildWorldToCameraMat44();
    void BuildCameraToPerspectiveMat44();

    void BuildHomogeneousPerspectiveToScreenMat44();
    void BuildNonHomogeneousPerspectiveToScreenMat44();

    void Update(f32 DeltaTime);

private:
    void BuildWorldToCameraEulerMat44(ERotateSeq Seq = ERotateSeq::YXZ);
    void BuildWorldToCameraUVNMat44(EUVNMode Mode);

public:
    VLN_DEFINE_ALIGN_OPERATORS_SSE();
};

}
