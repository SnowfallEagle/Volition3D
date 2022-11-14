#pragma once

#include <stdio.h>
#include <string.h>
#include "Core/Types.h"
#include "Core/Platform.h"
#include "Core/Assert.h"
#include "Math/Minimal.h"

// Polygon /////////////////////////////////
namespace EPolyStateV1
{
    enum
    {
        Active = BIT(1),
        Clipped = BIT(2),
        BackFace = BIT(3),
    };
}

namespace EPolyAttrV1
{
    enum
    {
        RGB32 = BIT(1),
        TwoSided = BIT(2),
        Transparent = BIT(3),

        ShadeModePure = BIT(4),
        ShadeModeFlat = BIT(5),
        ShadeModeGouraud = BIT(6),
        ShadeModePhong = BIT(7),
    };
}

class VPoly4DV1
{
public:
    u32 State;
    u32 Attr;
    u32 Color;

    VPoint4D* VtxList;
    i32 Vtx[3];
};

class VPolyFace4DV1 // Independent polygon face linked list
{
public:
    u32 State;
    u32 Attr;
    u32 Color;

    VPoint4D LocalVtx[3];
    VPoint4D TransVtx[3];

    VPolyFace4DV1* Prev; // ?
    VPolyFace4DV1* Next; // ?
};

// Renderer ////////////////////////////////////////////////

enum class ETransformType
{
    LocalOnly = 0,
    TransOnly,
    LocalToTrans,
};

class VRenderList4DV1
{
public:
    static constexpr i32f MaxPoly = 1024;

//private:
public:
    i32 NumPoly;
    VPolyFace4DV1* PolyPtrList[MaxPoly];
    VPolyFace4DV1 PolyList[MaxPoly];

public:
    void Reset()
    {
        NumPoly = 0;
    }

    void Transform(const VMatrix44& M, ETransformType Type)
    {
        VVector4D Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4D::MulMat44(Poly->LocalVtx[V], M, Res);
                    Poly->LocalVtx[V] = Res;
                }
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4D::MulMat44(Poly->TransVtx[V], M, Res);
                    Poly->TransVtx[V] = Res;
                }
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    VVector4D::MulMat44(Poly->LocalVtx[V], M, Res);
                    Poly->TransVtx[V] = Res;
                }
            }
        } break;
        }
    }

    // LocalToTrans or TransOnly
    void TransModelToWorld(const VPoint4D& WorldPos, ETransformType Type = ETransformType::LocalToTrans)
    {
        // TODO(sean): Test this function

        if (Type == ETransformType::LocalToTrans)
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    Poly->TransVtx[V] = Poly->LocalVtx[V] + WorldPos;
                }
            }
        }
        else // TransOnly
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPolyFace4DV1* Poly = PolyPtrList[I];
                if (!Poly ||
                    ~Poly->State & EPolyStateV1::Active ||
                    Poly->State & EPolyStateV1::Clipped ||
                    Poly->State & EPolyStateV1::BackFace)
                {
                    continue;
                }

                for (i32f V = 0; V < 3; ++V)
                {
                    Poly->TransVtx[V] += WorldPos;
                }
            }
        }
    }

    void RemoveBackFaces(VVector4D CamPos)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace4DV1* Poly = PolyPtrList[I];

            if (~Poly->State & EPolyStateV1::Active ||
                Poly->State & EPolyStateV1::Clipped ||
                Poly->Attr & EPolyAttrV1::TwoSided ||
                Poly->State & EPolyStateV1::BackFace)
            {
                continue;
            }

            VVector4D U, V, N;
            U = Poly->TransVtx[1] - Poly->TransVtx[0];
            V = Poly->TransVtx[2] - Poly->TransVtx[0];
            VVector4D::Cross(U, V, N);

            VVector4D View = CamPos - Poly->TransVtx[0];
            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4D::Dot(View, N) <= 0.0f)
                Poly->State |= EPolyStateV1::BackFace;
        }
    }

    void TransWorldToCamera(const VMatrix44& MatCamera)
    {
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPolyFace4DV1* Poly = PolyPtrList[I];
            if (!Poly ||
                ~Poly->State & EPolyStateV1::Active ||
                Poly->State & EPolyStateV1::Clipped ||
                Poly->State & EPolyStateV1::BackFace)
            {
                continue;
            }

            for (i32f V = 0; V < 3; ++V)
            {
                VVector4D Res;
                VVector4D::MulMat44(Poly->TransVtx[V], MatCamera, Res);
                Poly->TransVtx[V] = Res;
            }
        }
    }
};

// Camera /////////////////////////////////////////////////

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

namespace ECamV1State
{
    enum
    {

    };
}

namespace ECamV1Attr
{
    enum
    {
        Euler = BIT(1),
        UVN = BIT(2)
    };
}

class VCam4DV1
{
public:
    u32 State;
    u32 Attr;

    VPoint4D Pos;
    VVector4D Dir; // Euler angles or look at direction

    VVector4D U, V, N;
    VPoint4D Target;

    f32 FOV;
    f32 AspectRatio;
    f32 ViewDist;

    f32 ZNearClip;
    f32 ZFarClip;

    VVector2D ViewPlaneSize;
    VVector2DI ViewPortSize;
    VVector2DI ViewPortCenter;

    VPlane3D LeftClipPlane;
    VPlane3D RightClipPlane;
    VPlane3D TopClipPlane;
    VPlane3D BottomClipPlane;

    VMatrix44 MatCamera;      // World->Camera
    VMatrix44 MatPerspective; // Camera->Perspective
    VMatrix44 MatScreen;      // Perspective->Screen

public:
    void Init(
        u32 InAttr,
        const VPoint4D& InPos,
        const VVector4D& InDir,
        const VPoint4D& InTarget,
        f32 InFOV,
        f32 InZNearClip,
        f32 InZFarClip,
        const VVector2DI& InViewPortSize
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
        AspectRatio = (f32)InViewPortSize.X / (f32)InViewPortSize.Y;
        ViewDist = ((f32)InViewPortSize.X * 0.5f) / Math.Tan(FOV * 0.5f); // TODO(sean): Or maybe multiply by tan?

        ZNearClip = InZNearClip;
        ZFarClip = InZFarClip;

        ViewPlaneSize = { 2.0f, 2.0f/AspectRatio };
        ViewPortSize = InViewPortSize;
        ViewPortCenter = { (ViewPortSize.X-1)/2, (ViewPortSize.Y-1)/2 };

        MatCamera = VMatrix44::Identity;
        MatPerspective = VMatrix44::Identity;
        MatScreen = VMatrix44::Identity;

        if (FOV == 90.0f)
        {
            VPoint3D Origin;
            Origin.Zero();

            VVector3D N = { -1.0f, 0.0f, -1.0f };
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
            VPoint3D Origin;
            Origin.Zero();

            f32 MinusViewPlaneWidthDiv2 = -ViewPlaneSize.X * 0.5f;

            // TODO(sean): Check if it works
            VVector3D N = { -ViewDist, 0.0f, MinusViewPlaneWidthDiv2 };
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

    void BuildMatCameraEuler(ERotateSeq Seq = ERotateSeq::YXZ)
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
    void BuildMatCameraUVN(EUVNMode Mode)
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

            /* TODO(sean):
                   +Z
                    ^
                    |
                    -------->+Y
                    /
                   /
                  v
                 +X

                We convert this to:
                X = -Y
                Y = Z
                Z = X

                     +Y
                     ^
                     |
               +X<----
                     /
                    /
                   v
                  +Z

                But will just Z = -Z work???
             */
            Target.X = -1 * SinPhi * Math.FastSin(Dir.Y);
            Target.Y = 1 * Math.FastCos(Dir.X);
            Target.Z = 1 * SinPhi * Math.FastCos(Dir.Y);
        }

        // Compute UVN
        N = Target - Pos;
        V = { 0.0f, 1.0f, 0.0f, 1.0f };
        VVector4D::Cross(V, N, U);
        VVector4D::Cross(N, U, V);

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

    void BuildCameraToPerspectiveMat44(VMatrix44& M)
    {
        M = {
            ViewDist, 0.0f, 0.0f, 0.0f,
            0.0f, ViewDist*AspectRatio, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        };
    }
};

// Object ////////////////////////////////////////////
namespace EObjectStateV1
{
    enum
    {
        Active = BIT(1),
        Visible = BIT(2),
        Culled = BIT(3)
    };
}

namespace EObjectAttrV1
{
    enum
    {

    };
}

namespace EPLX
{
    enum
    {
        RGBFlag = BIT(16),

        ShadeModeMask = BIT(15) | BIT(14),
        ShadeModePureFlag = 0,
        ShadeModeFlatFlag = BIT(14),
        ShadeModeGouraudFlag = BIT(15),
        ShadeModePhongFlag = BIT(14) | BIT(15),

        TwoSidedFlag = BIT(13),
        RGB16Mask = 0x0FFF,
        RGB8Mask = 0x00FF,
    };
}

namespace ECullType
{
    enum
    {
        X = BIT(1),
        Y = BIT(2),
        Z = BIT(3),
        XYZ = X | Y | Z,
    };
}

DEFINE_LOG_CHANNEL(hObjectV1Log, "ObjectV1");

class VObject4DV1
{
public:
    static constexpr i32f NameSize = 64;
    static constexpr i32f MaxVtx = 64;
    static constexpr i32f MaxPoly = 128;

public:
    i32 ID;
    char Name[NameSize];

    u32 State;
    u32 Attr;

    f32 AvgRadius;
    f32 MaxRadius;

    VPoint4D WorldPos;
    VVector4D Dir;
    VVector4D UX, UY, UZ; // Local axes to track full orientation

    i32 NumVtx;
    VPoint4D LocalVtxList[MaxVtx];
    VPoint4D TransVtxList[MaxVtx];

    i32 NumPoly;
    VPoly4DV1 PolyList[MaxPoly];

public:
    b32 LoadPLG(
        const char* Path,
        const VVector4D& Pos,
        const VVector4D& Scale,
        const VVector4D& Rot
    );

    static char* GetLinePLG(FILE* File, char* Buffer, i32 Size)
    {
        for (;;)
        {
            if (!fgets(Buffer, Size, File))
                return nullptr;

            i32f I, Len = strlen(Buffer);
            for (I = 0; I < Len && Buffer[I] == ' ' && Buffer[I] == '\t'; ++I)
                {}

            if (I < Len && Buffer[I] != '#' && Buffer[I] != '\r' && Buffer[I] != '\n')
                return &Buffer[I];
        }
    }

    void ComputeRadius()
    {
        AvgRadius = 0.0f;
        MaxRadius = 0.0f;

        for (i32f I = 0; I < NumVtx; ++I)
        {
            f32 Dist = LocalVtxList[I].GetLength();
            AvgRadius += Dist;
            if (MaxRadius < Dist)
                MaxRadius = Dist;
        }

        AvgRadius /= NumVtx;
    }

    void Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis)
    {
        VVector4D Res;

        switch (Type)
        {
        case ETransformType::LocalOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4D::MulMat44(LocalVtxList[I], M, Res);
                LocalVtxList[I] = Res;
            }
        } break;

        case ETransformType::TransOnly:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4D::MulMat44(TransVtxList[I], M, Res);
                TransVtxList[I] = Res;
            }
        } break;

        case ETransformType::LocalToTrans:
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                VVector4D::MulMat44(LocalVtxList[I], M, Res);
                TransVtxList[I] = Res;
            }
        } break;
        }

        if (bTransBasis)
        {
            VVector4D::MulMat44(UX, M, Res);
            UX = Res;

            VVector4D::MulMat44(UY, M, Res);
            UY = Res;

            VVector4D::MulMat44(UZ, M, Res);
            UZ = Res;
        }
    }

    // LocalToTrans or TransOnly
    void TransModelToWorld(ETransformType Type = ETransformType::LocalToTrans)
    {
        if (Type == ETransformType::LocalToTrans)
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                TransVtxList[I] = LocalVtxList[I] + WorldPos;
            }
        }
        else // TransOnly
        {
            for (i32f I = 0; I < NumVtx; ++I)
            {
                TransVtxList[I] += WorldPos;
            }
        }
    }

    b32 Cull(const VCam4DV1& Cam, u32 CullType = ECullType::XYZ)
    {
        VVector4D SpherePos;
        VVector4D::MulMat44(WorldPos, Cam.MatCamera, SpherePos);

        if (CullType & ECullType::X)
        {
            f32 ZTest = (0.5f * Cam.ViewPlaneSize.X) * (SpherePos.Z / Cam.ViewDist);

            if (SpherePos.X - MaxRadius > ZTest ||  // Check Sphere's Left with Right side
                SpherePos.X + MaxRadius < -ZTest)   // Check Sphere's Right with Left side
            {
                State |= EObjectStateV1::Culled;
                return true;
            }
        }

        if (CullType & ECullType::Y)
        {
            f32 ZTest = (0.5f * Cam.ViewPlaneSize.Y) * (SpherePos.Z / Cam.ViewDist);

            // TODO(sean): SpherePos.Y + MaxRadius for first check???
            if (SpherePos.Y - MaxRadius > ZTest ||  // Check Sphere's Bottom with Top side
                SpherePos.Y + MaxRadius < -ZTest)   // Check Sphere's Top with Bottom side
            {
                State |= EObjectStateV1::Culled;
                return true;
            }
        }

        if (CullType & ECullType::Z)
        {
            if (SpherePos.Z - MaxRadius > Cam.ZFarClip ||
                SpherePos.Z + MaxRadius < Cam.ZNearClip)
            {
                State |= EObjectStateV1::Culled;
                return true;
            }
        }

        return false;
    }

    void RemoveBackFaces(VVector4D CamPos)
    {
        if (State & EObjectStateV1::Culled)
            return;

        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPoly4DV1& Poly = PolyList[I];

            if (~Poly.State & EPolyStateV1::Active ||
                Poly.State & EPolyStateV1::Clipped ||
                Poly.Attr & EPolyAttrV1::TwoSided ||
                Poly.State & EPolyStateV1::BackFace)
            {
                continue;
            }

            VVector4D U, V, N;
            U = TransVtxList[Poly.Vtx[1]] - TransVtxList[Poly.Vtx[0]];
            V = TransVtxList[Poly.Vtx[2]] - TransVtxList[Poly.Vtx[0]];
            VVector4D::Cross(U, V, N);

            VVector4D View = CamPos - TransVtxList[Poly.Vtx[0]];
            // If > 0 then N watch in the same direction as View vector and visible
            if (VVector4D::Dot(View, N) <= 0.0f)
                Poly.State |= EPolyStateV1::BackFace;
        }
    }

    void TransWorldToCamera(const VMatrix44& MatCamera)
    {
        for (i32f I = 0; I < NumVtx; ++I)
        {
            VVector4D Res;
            VVector4D::MulMat44(TransVtxList[I], MatCamera, Res);
            TransVtxList[I] = Res;
        }
    }

    void TransCameraToPerspective(const VCam4DV1& Cam)
    {
        for (i32f I = 0; I < NumVtx; ++I)
        {
            f32 Z = TransVtxList[I].Z;

            // TODO(sean): Understand role of multiplication by AspectRatio...
            TransVtxList[I].X *= Cam.ViewDist / Z;
            TransVtxList[I].Y *= Cam.AspectRatio * (Cam.ViewDist / Z);
            // Z = Z
        }
    }

    void ConvertFromHomogeneous()
    {
        for (i32f I = 0; I < NumVtx; ++I)
        {
            TransVtxList[I].DivByW();
        }
    }

    void Reset()
    {
        // Reset object's state
        State = State & ~EObjectStateV1::Culled;

        // Restore polygons
        for (i32f I = 0; I < NumPoly; ++I)
        {
            VPoly4DV1& Poly = PolyList[I];
            if (~Poly.State & EPolyStateV1::Active)
                continue;

            Poly.State &= ~EPolyStateV1::Clipped;
            Poly.State &= ~EPolyStateV1::BackFace;
        }
    }
};
