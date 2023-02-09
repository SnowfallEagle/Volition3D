#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Math/Minimal.h"

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

namespace ECameraState
{
    enum
    {

    };
}

namespace ECameraAttr
{
    enum
    {
        Euler = VL_BIT(1),
        UVN = VL_BIT(2)
    };
}

class VCamera
{
public:
    u32 State;
    u32 Attr;

    VPoint4 Pos;
    VVector4 Dir; // Euler angles or look at direction

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

    VMatrix44 MatCamera;      // World->Camera
    VMatrix44 MatPerspective; // Camera->Perspective
    VMatrix44 MatScreen;      // Perspective->Screen

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
    )
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

            f32 MinusViewPlaneWidthDiv2 = -ViewPlaneSize.X * 0.5f;

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

    void BuildWorldToCameraMat44()
    {
        Attr & ECameraAttr::Euler ? BuildWorldToCameraEulerMat44() : BuildWorldToCameraUVNMat44(EUVNMode::Spherical);
    }

    void BuildWorldToCameraEulerMat44(ERotateSeq Seq = ERotateSeq::YXZ)
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

    // On Spherical mode Dir.X - elevation, Dir.Y - heading
    void BuildWorldToCameraUVNMat44(EUVNMode Mode)
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

            f32 SinPhi = Math.FastSin(Dir.X);

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

    void BuildCameraToPerspectiveMat44()
    {
        MatPerspective = {
            ViewDist, 0.0f, 0.0f, 0.0f,
            0.0f, ViewDist*AspectRatio, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }

    void BuildHomogeneousPerspectiveToScreenMat44()
    {
        /* NOTE(sean):
            This function assumes that later in code
            we will perform conversion 4D->3D
        */

        f32 Alpha = ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = ViewPortSize.Y * 0.5f - 0.5f;

        MatScreen = {
            Alpha, 0.0f, 0.0f, 0.0f,
            0.0f, -Beta, 0.0f, 0.0f,
            Alpha, Beta, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        };
    }

    void BuildNonHomogeneousPerspectiveToScreenMat44()
    {
        /* NOTE(sean):
            This function assumes that we are already
            performed conversion 4D->3D
        */

        f32 Alpha = ViewPortSize.X * 0.5f - 0.5f;
        f32 Beta = ViewPortSize.Y * 0.5f - 0.5f;

        MatScreen = {
            Alpha, 0.0f, 0.0f, 0.0f,
            0.0f, -Beta, 0.0f, 0.0f,
            Alpha, Beta, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
    }
};

