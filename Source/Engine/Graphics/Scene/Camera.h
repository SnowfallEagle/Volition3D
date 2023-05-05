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
    enum
    {
        Euler = VLN_BIT(1),
        UVN = VLN_BIT(2)
    };
}

VLN_DECL_ALIGN_SSE() class VCamera
{
public:
    u32 Attr;

    VPoint4 Pos;
    VVector4 Dir; /** Euler angles or look at direction */

    VVector4 U, V, N;
    VPoint4 Target;

    f32 FOV;
    f32 AspectRatio;
    f32 ViewDist;

    f32 ZNearClip;
    f32 ZFarClip;

    VVector2 ViewPlaneSize;
    VVector2 ViewPortSize;
    VVector2 ViewPortCenter;

    VPlane3 LeftClipPlane;
    VPlane3 RightClipPlane;
    VPlane3 TopClipPlane;
    VPlane3 BottomClipPlane;

    VMatrix44 MatCamera;      /** World->Camera */
    VMatrix44 MatCameraRotationOnly; /** World->Camera but only for Normals or Directions */
    VMatrix44 MatPerspective; /** Camera->Perspective */
    VMatrix44 MatScreen;      /** Perspective->Screen */

public:
    void Init(
        u32 InAttr,
        const VPoint4& InPos,
        const VVector4& InDir,
        const VPoint4& InTarget,
        f32 InFOV,
        f32 InZNearClip,
        f32 InZFarClip,
        const VVector2& InViewPortSize
    );

    // @TODO: Put in cpp
    void BuildWorldToCameraMat44()
    {
        Attr & ECameraAttr::Euler ? BuildWorldToCameraEulerMat44() : BuildWorldToCameraUVNMat44(EUVNMode::Spherical);
        MatCameraRotationOnly = MatCamera;
        MatCameraRotationOnly.C32 = MatCameraRotationOnly.C31 = MatCameraRotationOnly.C30 = 0.0f;
    }

    void BuildWorldToCameraEulerMat44(ERotateSeq Seq = ERotateSeq::YXZ);
    void BuildWorldToCameraUVNMat44(EUVNMode Mode);

    void BuildCameraToPerspectiveMat44();

    void BuildHomogeneousPerspectiveToScreenMat44();
    void BuildNonHomogeneousPerspectiveToScreenMat44();

public:
    VLN_DEFINE_ALIGN_OPERATORS(16);
};

}
