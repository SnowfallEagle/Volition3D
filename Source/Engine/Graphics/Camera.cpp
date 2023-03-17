#include "Engine/Graphics/Camera.h"

namespace Volition
{

void VCamera::Init(u32 InAttr, const VPoint4& InPos, const VVector4& InDir, const VPoint4& InTarget, f32 InFOV, f32 InZNearClip, f32 InZFarClip, const VVector2& InViewPortSize)
{
    State = 0;
    Attr = InAttr;

    Pos = InPos;
    Dir = InDir;

    U = { 1.0f, 0.0f, 0.0f, 1.0f };
    V = { 0.0f, 1.0f, 0.0f, 1.0f };
    N = { 0.0f, 0.0f, 1.0f, 1.0f };
    Target = InTarget;

    FOV = InFOV;
    AspectRatio = InViewPortSize.X / InViewPortSize.Y;

    ZNearClip = InZNearClip;
    ZFarClip = InZFarClip;

    ViewPlaneSize = { 2.0f, 2.0f/AspectRatio };
    ViewDist = (ViewPlaneSize.X * 0.5f) / Math.Tan(FOV * 0.5f);
    ViewPortSize = InViewPortSize;
    ViewPortCenter = {
        (ViewPortSize.X - 1.0f) * 0.5f, (ViewPortSize.Y - 1.0f) * 0.5f
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

        const f32 MinusViewPlaneWidthDiv2 = -ViewPlaneSize.X * 0.5f;

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
        -Pos.X, -Pos.Y, -Pos.Z, 1.0f,
    };

    // Rotation X
    SinA = -Math.FastSin(Dir.X); // sin(-a) = -sin(a)
    CosA = Math.FastCos(Dir.X);  // cos(-a) = cos(a)

    InvX = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, CosA, SinA, 0.0f,
        0.0f, -SinA, CosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Rotation Y
    SinA = -Math.FastSin(Dir.Y); // sin(-a) = -sin(a)
    CosA = Math.FastCos(Dir.Y);  // cos(-a) = cos(a)

    InvY = {
        CosA, 0.0f, -SinA, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        SinA, 0.0f, CosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Rotation Z
    SinA = -Math.FastSin(Dir.Z); // sin(-a) = -sin(a)
    CosA = Math.FastCos(Dir.Z);  // cos(-a) = cos(a)

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
        -Pos.X, -Pos.Y, -Pos.Z, 1.0f
    };

    // Compute target for Spherical mode
    if (Mode == EUVNMode::Spherical)
    {
        // Elevation - Dir.X
        // Heading - Dir.Y

        const f32 SinPhi = Math.FastSin(Dir.X);

        Target.X = -1 * SinPhi * Math.FastSin(Dir.X);
        Target.Y = 1 * Math.FastCos(Dir.Y);
        Target.Z = 1 * SinPhi * Math.FastCos(Dir.Z);
    }

    // Compute UVN
    N = Target - Pos;
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

void VCamera::BuildHomogeneousPerspectiveToScreenMat44()
{
    /* NOTE:
        This function assumes that later in code
        we will perform conversion 4D->3D
    */

    const f32 Alpha = ViewPortSize.X * 0.5f - 0.5f;
    const f32 Beta = ViewPortSize.Y * 0.5f - 0.5f;

    MatScreen = {
        Alpha, 0.0f, 0.0f, 0.0f,
        0.0f, -Beta, 0.0f, 0.0f,
        Alpha, Beta, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };
}

void VCamera::BuildNonHomogeneousPerspectiveToScreenMat44()
{
    /* NOTE:
        This function assumes that we are already
        performed conversion 4D->3D
    */

    const f32 Alpha = ViewPortSize.X * 0.5f - 0.5f;
    const f32 Beta = ViewPortSize.Y * 0.5f - 0.5f;

    MatScreen = {
        Alpha, 0.0f, 0.0f, 0.0f,
        0.0f, -Beta, 0.0f, 0.0f,
        Alpha, Beta, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

}