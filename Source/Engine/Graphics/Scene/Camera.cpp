#include "Engine/Core/Config/Config.h"
#include "Engine/Graphics/Scene/Camera.h"

namespace Volition
{

void VCamera::Init(u32 InAttr, const VPoint4& InPos, const VVector4& InDir, const VPoint4& InTarget, f32 InFOV, f32 InZNearClip, f32 InZFarClip)
{
    Attr = InAttr;

    Position = InPos;
    Direction = InDir;

    U = { 1.0f, 0.0f, 0.0f, 1.0f };
    V = { 0.0f, 1.0f, 0.0f, 1.0f };
    N = { 0.0f, 0.0f, 1.0f, 1.0f };
    Target = InTarget;

    FOV = InFOV;
    ViewportSize = { (f32)Config.RenderSpec.TargetSize.X, (f32)Config.RenderSpec.TargetSize.Y };
    AspectRatio = ViewportSize.X / ViewportSize.Y;

    ZNearClip = InZNearClip;
    ZFarClip = InZFarClip;

    ViewplaneSize = { 2.0f, 2.0f/AspectRatio };
    ViewDist = (ViewplaneSize.X * 0.5f) / Math.Tan(FOV * 0.5f);
    ViewportCenter = {
        (ViewportSize.X - 1.0f) * 0.5f, (ViewportSize.Y - 1.0f) * 0.5f
    };

    MatCamera = VMatrix44::Identity;
    MatPerspective = VMatrix44::Identity;
    MatScreen = VMatrix44::Identity;

    if (FOV == 90.0f)
    {
        VPoint3 Origin;
        Origin.Zero();

        VVector3 N = { -1.0f, 0.0f, -1.0f };
        N.Normalize();
        LeftClipPlane = { Origin, N };

        N = { 1.0f, 0.0f, -1.0f };
        N.Normalize();
        RightClipPlane = { Origin, N };

        N = { 0.0f, 1.0f, -1.0f };
        N.Normalize();
        TopClipPlane = { Origin, N };

        N = { 0.0f, -1.0f, -1.0f };
        N.Normalize();
        BottomClipPlane = { Origin, N };
    }
    else
    {
        VPoint3 Origin;
        Origin.Zero();

        const f32 MinusViewPlaneWidthDiv2 = -ViewplaneSize.X * 0.5f;

        VVector3 N = { -ViewDist, 0.0f, MinusViewPlaneWidthDiv2 };
        N.Normalize();
        LeftClipPlane = { Origin, N };

        N = { ViewDist, 0.0f, MinusViewPlaneWidthDiv2 };
        N.Normalize();
        RightClipPlane = { Origin, N };

        N = { 0.0f, ViewDist, MinusViewPlaneWidthDiv2 };
        N.Normalize();
        TopClipPlane = { Origin, N };

        N = { 0.0f, -ViewDist, MinusViewPlaneWidthDiv2 };
        N.Normalize();
        BottomClipPlane = { Origin, N };
    }
}

void VCamera::BuildWorldToCameraEulerMat44(ERotateSeq Seq)
{
    VMatrix44 InvTranslate, InvX, InvY, InvZ, Rot, Temp;
    f32 SinA, CosA;

    // Translate
    InvTranslate = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -Position.X, -Position.Y, -Position.Z, 1.0f,
    };

    // Rotation X
    SinA = -Math.FastSin(Direction.X); // sin(-a) = -sin(a)
    CosA = Math.FastCos(Direction.X);  // cos(-a) = cos(a)

    InvX = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, CosA, SinA, 0.0f,
        0.0f, -SinA, CosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Rotation Y
    SinA = -Math.FastSin(Direction.Y); // sin(-a) = -sin(a)
    CosA = Math.FastCos(Direction.Y);  // cos(-a) = cos(a)

    InvY = {
        CosA, 0.0f, -SinA, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        SinA, 0.0f, CosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Rotation Z
    SinA = -Math.FastSin(Direction.Z); // sin(-a) = -sin(a)
    CosA = Math.FastCos(Direction.Z);  // cos(-a) = cos(a)

    InvZ = {
        CosA, SinA, 0.0f, 0.0f,
        -SinA, CosA, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Rotations
    switch (Seq)
    {
    case ERotateSeq::YXZ:
    {
        VMatrix44::Mul(InvY, InvX, Temp);
        VMatrix44::Mul(Temp, InvZ, Rot);
    } break;

    case ERotateSeq::ZXY:
    {
        VMatrix44::Mul(InvZ, InvX, Temp);
        VMatrix44::Mul(Temp, InvY, Rot);
    } break;
    }

    // Result
    VMatrix44::Mul(InvTranslate, Rot, MatCamera);
}

void VCamera::BuildWorldToCameraUVNMat44(EUVNMode Mode)
{
    // Translate matrix
    VMatrix44 InvTranslate = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -Position.X, -Position.Y, -Position.Z, 1.0f
    };

    // Compute target for Spherical mode
    if (Mode == EUVNMode::Spherical)
    {
        // Elevation - Direction.X
        // Heading - Direction.Y

        const f32 SinPhi = Math.FastSin(Direction.X);

        Target.X = -1 * SinPhi * Math.FastSin(Direction.X);
        Target.Y = 1 * Math.FastCos(Direction.Y);
        Target.Z = 1 * SinPhi * Math.FastCos(Direction.Z);
    }

    // Compute UVN
    N = Target - Position;
    V = { 0.0f, 1.0f, 0.0f, 1.0f };
    VVector4::Cross(V, N, U);
    VVector4::Cross(N, U, V);

    // Normalize UVN vectors
    U.Normalize();
    V.Normalize();
    N.Normalize();

    // Compute UVN matrix
    VMatrix44 UVN = {
        U.X, V.X, N.X, 0.0f,
        U.Y, V.Y, N.Y, 0.0f,
        U.Z, V.Z, N.Z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Compute camera matrix
    VMatrix44::Mul(InvTranslate, UVN, MatCamera);
}

void VCamera::BuildCameraToPerspectiveMat44()
{
    MatPerspective = {
        ViewDist, 0.0f, 0.0f, 0.0f,
        0.0f, ViewDist*AspectRatio, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

void VCamera::BuildWorldToCameraMat44()
{
    Attr & ECameraAttr::Euler ? BuildWorldToCameraEulerMat44() : BuildWorldToCameraUVNMat44(EUVNMode::Spherical);
    MatCameraRotationOnly = MatCamera;
    MatCameraRotationOnly.C32 = MatCameraRotationOnly.C31 = MatCameraRotationOnly.C30 = 0.0f;
}

void VCamera::BuildHomogeneousPerspectiveToScreenMat44()
{
    /*
        This function assumes that later in code
        we will perform conversion 4D->3D
    */

    const f32 Alpha = ViewportSize.X * 0.5f - 0.5f;
    const f32 Beta = ViewportSize.Y * 0.5f - 0.5f;

    MatScreen = {
        Alpha, 0.0f, 0.0f, 0.0f,
        0.0f, -Beta, 0.0f, 0.0f,
        Alpha, Beta, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };
}

void VCamera::BuildNonHomogeneousPerspectiveToScreenMat44()
{
    /*
        This function assumes that we are already
        performed conversion 4D->3D
    */

    const f32 Alpha = ViewportSize.X * 0.5f - 0.5f;
    const f32 Beta = ViewportSize.Y * 0.5f - 0.5f;

    MatScreen = {
        Alpha, 0.0f, 0.0f, 0.0f,
        0.0f, -Beta, 0.0f, 0.0f,
        Alpha, Beta, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

}