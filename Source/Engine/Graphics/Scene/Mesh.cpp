#include <cstdio>
#include <cerrno>
#include "Common/Types/Common.h"
#include "Common/Types/Array.h"
#include "Common/Platform/Memory.h"
#include "Common/Math/Vector.h"
#include "Engine/World/World.h"
#include "Engine/Graphics/Rendering/Renderer.h"
#include "Engine/Graphics/Scene/Mesh.h"

namespace Volition
{

VMesh::VMesh()
{
    // @FIXME: VString Name
    Memory.MemSetByte(this, 0, sizeof(*this));

    State = EMeshState::Active | EMeshState::Visible;

    UX = { 1.0f, 0.0f, 0.0f };
    UY = { 0.0f, 1.0f, 0.0f };
    UZ = { 0.0f, 0.0f, 1.0f };

    NumFrames = 1;
    CurrentFrame = 0.0f;

    bAnimationPlayed = true;
}

void VMesh::Allocate(i32 InNumVtx, i32 InNumPoly, i32 InNumFrames, i32 InNumTextureCoords)
{
    HeadLocalVtxList = LocalVtxList = new VVertex[InNumVtx * InNumFrames];
    HeadTransVtxList = TransVtxList = new VVertex[InNumVtx];
    Memory.MemSetByte(HeadLocalVtxList, 0, sizeof(VVertex) * (InNumVtx * InNumFrames));
    Memory.MemSetByte(HeadTransVtxList, 0, sizeof(VVertex) * (InNumVtx));

    PolyList          = new VPoly[InNumPoly];
    NumTextureCoords  = InNumTextureCoords != -1 ? InNumTextureCoords : InNumPoly * 3;
    TextureCoordsList = new VPoint2[NumTextureCoords];
    Memory.MemSetByte(PolyList, 0, sizeof(VPoly) * InNumPoly);
    Memory.MemSetByte(TextureCoordsList, 0, sizeof(VPoint2) * NumTextureCoords);

    AverageRadiusList = new f32[InNumFrames];
    MaxRadiusList     = new f32[InNumFrames];
    Memory.MemSetByte(AverageRadiusList, 0, sizeof(f32) * InNumFrames);
    Memory.MemSetByte(MaxRadiusList, 0, sizeof(f32) * InNumFrames);

    NumVtx      = InNumVtx;
    TotalNumVtx = InNumVtx * InNumFrames;
    NumPoly     = InNumPoly;
    NumFrames   = InNumFrames;
}

void VMesh::Destroy()
{
    VLN_SAFE_DELETE_ARRAY(HeadLocalVtxList);
    VLN_SAFE_DELETE_ARRAY(HeadTransVtxList);
    VLN_SAFE_DELETE_ARRAY(PolyList);
    VLN_SAFE_DELETE_ARRAY(TextureCoordsList);
    VLN_SAFE_DELETE_ARRAY(AverageRadiusList);
    VLN_SAFE_DELETE_ARRAY(MaxRadiusList);
}

void VMesh::ResetRenderState()
{
    // Reset object's state
    State &= ~EMeshState::Culled;

    // Restore polygons
    for (i32f i = 0; i < NumPoly; ++i)
    {
        VPoly& Poly = PolyList[i];
        if (~Poly.State & EPolyState::Active)
        {
            continue;
        }

        Poly.State &= ~(EPolyState::Clipped | EPolyState::Backface | EPolyState::Lit);
        Poly.LitColor[2] = Poly.LitColor[1] = Poly.LitColor[0] = Poly.OriginalColor;
    }
}

void VMesh::ComputeRadius()
{
    for (i32f FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
    {
        const VVertex* VtxList = &HeadLocalVtxList[FrameIndex * NumVtx];
        f32 AverageRadius = 0.0f;
        f32 MaxRadius = 0.0f;

        for (i32f VtxIndex = 0; VtxIndex < NumVtx; ++VtxIndex)
        {
            f32 Distance = VtxList[VtxIndex].Position.GetLength();

            AverageRadius += Distance;
            if (MaxRadius < Distance)
            {
                MaxRadius = Distance;
            }
        }
        AverageRadius /= NumVtx;

        AverageRadiusList[FrameIndex] = AverageRadius;
        MaxRadiusList[FrameIndex] = MaxRadius;

        VLN_LOG_VERBOSE("\n\tFrame: %d\n\tAverage radius: %.3f\n\tMax radius: %.3f\n", FrameIndex, AverageRadius, MaxRadius);
    }
}

void VMesh::ComputePolygonNormalsLength()
{
    for (i32f i = 0; i < NumPoly; ++i)
    {
        const i32f V0 = PolyList[i].VtxIndices[0];
        const i32f V1 = PolyList[i].VtxIndices[1];
        const i32f V2 = PolyList[i].VtxIndices[2];

        const VVector4 U = LocalVtxList[V1].Position - LocalVtxList[V0].Position;
        const VVector4 V = LocalVtxList[V2].Position - LocalVtxList[V0].Position;

        PolyList[i].NormalLength = VVector4::GetCross(U, V).GetLength();

        VLN_LOG_VERBOSE("\tPolygon normal length [%d]: %f\n", i, PolyList[i].NormalLength);
    }
}

void VMesh::ComputeVertexNormals()
{
    TArray<i32> NumPolyTouchVtx(NumVtx);
    Memory.MemSetQuad(NumPolyTouchVtx.GetData(), 0, NumVtx);

    for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
    {
        if (PolyList[PolyIndex].Attr & EPolyAttr::ShadeModeGouraud)
        {
            i32f V0 = PolyList[PolyIndex].VtxIndices[0];
            i32f V1 = PolyList[PolyIndex].VtxIndices[1];
            i32f V2 = PolyList[PolyIndex].VtxIndices[2];

            ++NumPolyTouchVtx[V0];
            ++NumPolyTouchVtx[V1];
            ++NumPolyTouchVtx[V2];

            for (i32f FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
            {
                const VVector4 U = HeadLocalVtxList[V1].Position - HeadLocalVtxList[V0].Position;
                const VVector4 V = HeadLocalVtxList[V2].Position - HeadLocalVtxList[V0].Position;

                VVector4 Normal;
                VVector4::Cross(U, V, Normal);

                HeadLocalVtxList[V0].Normal += Normal;
                HeadLocalVtxList[V1].Normal += Normal;
                HeadLocalVtxList[V2].Normal += Normal;

                V0 += NumVtx;
                V1 += NumVtx;
                V2 += NumVtx;
            }
        }
    }

    for (i32f FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
    {
        i32f VtxFrameStart = FrameIndex * NumVtx;
        i32f VtxFrameBound = (FrameIndex + 1) * NumVtx;

        for (i32f VtxIndex = VtxFrameStart, TouchIndex = 0; VtxIndex < VtxFrameBound; ++VtxIndex, ++TouchIndex)
        {
            if (NumPolyTouchVtx[TouchIndex] > 0)
            {
                HeadLocalVtxList[VtxIndex].Normal /= (f32)NumPolyTouchVtx[TouchIndex];
                HeadLocalVtxList[VtxIndex].Normal.Normalize();

                HeadLocalVtxList[VtxIndex].Attr |= EVertexAttr::HasNormal;

                VLN_LOG_VERBOSE("Vertex normal [%d]: <%.2f %.2f %.2f>\n", VtxIndex, HeadLocalVtxList[VtxIndex].Normal.X, HeadLocalVtxList[VtxIndex].Normal.Y, HeadLocalVtxList[VtxIndex].Normal.Z);
            }
        }
    }
}

void VMesh::TransformModelToWorld(ETransformType Type)
{
    if (Type == ETransformType::LocalToTrans)
    {
        for (i32f i = 0; i < NumVtx; ++i)
        {
            TransVtxList[i] = LocalVtxList[i];
            TransVtxList[i].Position += Position;
        }
    }
    else // TransOnly
    {
        for (i32f i = 0; i < NumVtx; ++i)
        {
            TransVtxList[i].Position += Position;
        }
    }
}

void VMesh::Transform(const VMatrix44& M, ETransformType Type, b32 bTransBasis)
{
    VVector4 Res;

    switch (Type)
    {
    case ETransformType::LocalOnly:
    {
        for (i32f i = 0; i < NumVtx; ++i)
        {
            VMatrix44::MulVecMat(LocalVtxList[i].Position, M, Res);
            LocalVtxList[i].Position = Res;

            if (LocalVtxList[i].Attr & EVertexAttr::HasNormal)
            {
                VMatrix44::MulVecMat(LocalVtxList[i].Normal, M, Res);
                LocalVtxList[i].Normal = Res;
            }
        }
    } break;

    case ETransformType::TransOnly:
    {
        for (i32f i = 0; i < NumVtx; ++i)
        {
            VMatrix44::MulVecMat(TransVtxList[i].Position, M, Res);
            TransVtxList[i].Position = Res;

            if (TransVtxList[i].Attr & EVertexAttr::HasNormal)
            {
                VMatrix44::MulVecMat(TransVtxList[i].Normal, M, Res);
                TransVtxList[i].Normal = Res;
            }
        }
    } break;

    case ETransformType::LocalToTrans:
    {
        for (i32f i = 0; i < NumVtx; ++i)
        {
            VMatrix44::MulVecMat(LocalVtxList[i].Position, M, TransVtxList[i].Position);

            if (LocalVtxList[i].Attr & EVertexAttr::HasNormal)
            {
                VMatrix44::MulVecMat(LocalVtxList[i].Normal, M, TransVtxList[i].Normal);
            }
        }
    } break;
    }

    if (bTransBasis)
    {
        VMatrix44::MulVecMat(UX, M, Res);
        UX = Res;

        VMatrix44::MulVecMat(UY, M, Res);
        UY = Res;

        VMatrix44::MulVecMat(UZ, M, Res);
        UZ = Res;
    }
}

b32 VMesh::Cull(const VCamera& Cam, u32 CullType)
{
    if (~Attr & EMeshAttr::CanBeCulled)
    {
        return false;
    }

    VVector4 SpherePos;
    VMatrix44::MulVecMat(Position, Cam.MatCamera, SpherePos);
    const f32 MaxRadius = GetMaxRadius();

    if (CullType & ECullType::X)
    {
        f32 ZTest = (0.5f * Cam.ViewplaneSize.X) * (SpherePos.Z / Cam.ViewDist);

        if (SpherePos.X - MaxRadius > ZTest ||  // Check Sphere's Left with Right side
            SpherePos.X + MaxRadius < -ZTest)   // Check Sphere's Right with Left side
        {
            State |= EMeshState::Culled;
            return true;
        }
    }

    if (CullType & ECullType::Y)
    {
        f32 ZTest = (0.5f * Cam.ViewplaneSize.Y) * (SpherePos.Z / Cam.ViewDist);

        if (SpherePos.Y - MaxRadius > ZTest ||  // Check Sphere's Bottom with Top side
            SpherePos.Y + MaxRadius < -ZTest)   // Check Sphere's Top with Bottom side
        {
            State |= EMeshState::Culled;
            return true;
        }
    }

    if (CullType & ECullType::Z)
    {
        if (SpherePos.Z - MaxRadius > Cam.ZFarClip ||
            SpherePos.Z + MaxRadius < Cam.ZNearClip)
        {
            State |= EMeshState::Culled;
            return true;
        }
    }

    return false;
}

/******************************************************
           Common Stuff For Loading Models            *
 ******************************************************/

static void GetTexturePathFromModelDirectory(char* TexturePath, i32f BufferSize, const char* TexturePathRaw, const char* ModelPath)
{
    i32f TextureSlashIndex;
    for (TextureSlashIndex = std::strlen(TexturePathRaw) - 1; TextureSlashIndex >= 0; --TextureSlashIndex)
    {
        if (TexturePathRaw[TextureSlashIndex] == '\\' || TexturePathRaw[TextureSlashIndex] == '/')
        {
            break;
        }
    }

    i32f PathSlashIndex;
    for (PathSlashIndex = std::strlen(ModelPath) - 1; PathSlashIndex >= 0; --PathSlashIndex)
    {
        if (ModelPath[PathSlashIndex] == '\\' || ModelPath[PathSlashIndex] == '/')
        {
            break;
        }
    }

    std::snprintf(TexturePath, BufferSize, "%.*s%s", PathSlashIndex + 1, ModelPath, TexturePathRaw + (TextureSlashIndex + 1));
}

/******************************************************
                    COB Loading                       *
 ******************************************************/

VLN_DEFINE_LOG_CHANNEL(hLogCOB, "COB Loader");

char* GetLineCOB(std::FILE* File, char* Buffer, i32 Size)
{
    for (;;)
    {
        if (!std::fgets(Buffer, Size, File))
        {
            return nullptr;
        }

        const i32f Len = std::strlen(Buffer);

        i32f i;
        for (i = 0; i < Len && (Buffer[i] == ' ' || Buffer[i] == '\t' || Buffer[i] == '\r' || Buffer[i] == '\n'); ++i)
            {}

        if (i < Len)
        {
            return &Buffer[i];
        }
    }
}

char* FindLineCOB(const char* Pattern, std::FILE* File, char* Buffer, i32 Size)
{
    const i32f PatternLength = std::strlen(Pattern);

    for (;;)
    {
        char* Line = GetLineCOB(File, Buffer, Size);
        if (!Line)
        {
            return nullptr;
        }

        b32 bFound = true;

        for (i32f i = 0; i < PatternLength; ++i)
        {
            if (Pattern[i] != Line[i])
            {
                bFound = false;
                break;
            }
        }

        if (bFound)
        {
            return Line;
        }
    }
}

b32 VMesh::LoadCOB(const char* Path, const VVector4& InPosition, const VVector4& Scale, u32 Flags, EShadeMode OverrideShadeMode)
{
    static constexpr i32f BufferSize = 4096;

    f32 TempFloat;

    VMatrix44 MatLocal = VMatrix44::Identity;
    VMatrix44 MatWorld = VMatrix44::Identity;

    // Initialization 
    {
        Position = InPosition;
        Attr |= EMeshAttr::CanBeCulled | EMeshAttr::CastShadow;
    }

    // Load from file
    {
        std::FILE* File;
        char Buffer[BufferSize];
        char* Line;

        VLN_NOTE(hLogCOB, "Starting parse object:\n");

        // Open file
        if (!(File = std::fopen(Path, "r")))
        {
            VLN_ERROR(hLogCOB, "Can't open %s file: %s\n", Path, strerror(errno));
            return false;
        }

        // Read name
        {
            Line = FindLineCOB("Name", File, Buffer, BufferSize);
            std::sscanf(Line, "Name %s", Name);
            VLN_LOG_VERBOSE("\tName: %s\n", Name);
        }

        // Read local transform matrix
        {
            Line = FindLineCOB("center", File, Buffer, BufferSize);
            std::sscanf(Line, "center %f %f %f", &MatLocal.C30, &MatLocal.C31, &MatLocal.C32);
            MatLocal.C30 = -MatLocal.C30;
            MatLocal.C31 = -MatLocal.C31;
            MatLocal.C32 = -MatLocal.C32;

            Line = FindLineCOB("x axis", File, Buffer, BufferSize);
            std::sscanf(Line, "x axis %f %f %f", &MatLocal.C00, &MatLocal.C10, &MatLocal.C20);

            Line = FindLineCOB("y axis", File, Buffer, BufferSize);
            std::sscanf(Line, "y axis %f %f %f", &MatLocal.C01, &MatLocal.C11, &MatLocal.C21);

            Line = FindLineCOB("z axis", File, Buffer, BufferSize);
            std::sscanf(Line, "z axis %f %f %f", &MatLocal.C02, &MatLocal.C12, &MatLocal.C22);

            VLN_LOG_VERBOSE("\tMatLocal: ");
            MatLocal.Print();
            VLN_LOG_VERBOSE("\n");
        }

        // Read world transform matrix
        {
            Line = FindLineCOB("Transform", File, Buffer, BufferSize);

            // X
            Line = GetLineCOB(File, Buffer, BufferSize);
            std::sscanf(Line, "%f %f %f", &MatWorld.C00, &MatWorld.C10, &MatWorld.C20);

            // Y
            Line = GetLineCOB(File, Buffer, BufferSize);
            std::sscanf(Line, "%f %f %f", &MatWorld.C01, &MatWorld.C11, &MatWorld.C21);

            // Z
            Line = GetLineCOB(File, Buffer, BufferSize);
            std::sscanf(Line, "%f %f %f", &MatWorld.C02, &MatWorld.C12, &MatWorld.C22);

            VLN_LOG_VERBOSE("\tMatWorld: ");
            MatWorld.Print();
            VLN_LOG_VERBOSE("\n");
        }

        // Read vertices and make object allocation
        {
            i32f NumWorldVertices;

            Line = FindLineCOB("World Vertices", File, Buffer, BufferSize);
            std::sscanf(Line, "World Vertices %d", &NumWorldVertices);

            Allocate(
                NumWorldVertices,
                NumWorldVertices * 3, // We don't know how much polygons we will get so it's <NumWorldVertices * 3>
                NumFrames
            );

            for (i32f i = 0; i < NumWorldVertices; ++i)
            {
                // Parse
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "%f %f %f", &LocalVtxList[i].X, &LocalVtxList[i].Y, &LocalVtxList[i].Z);
                LocalVtxList[i].W = 1.0f;

                // Apply matrix transformations
                VVector4 TempVector;
                VMatrix44::MulVecMat(LocalVtxList[i].Position, MatLocal, TempVector);
                VMatrix44::MulVecMat(TempVector, MatWorld, LocalVtxList[i].Position);

                // Swap YZ
                if (Flags & ECOBFlags::SwapYZ)
                {
                    VLN_SWAP(LocalVtxList[i].Y, LocalVtxList[i].Z, TempFloat);
                    LocalVtxList[i].Z = -LocalVtxList[i].Z;
                }

                // Scale
                LocalVtxList[i].X *= Scale.X;
                LocalVtxList[i].Y *= Scale.Y;
                LocalVtxList[i].Z *= Scale.Z;

                // Log
                VLN_LOG_VERBOSE("\tVertex [%d]: ", i);
                LocalVtxList[i].Position.Print();
                VLN_LOG_VERBOSE("\n");
            }
        }

        // Read texture coords
        {
            Line = FindLineCOB("Texture Vertices", File, Buffer, BufferSize);
            std::sscanf(Line, "Texture Vertices %d", &NumTextureCoords);
            VLN_LOG_VERBOSE("\tNum texture coords: %d\n", NumTextureCoords);

            for (i32f i = 0; i < NumTextureCoords; ++i)
            {
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "%f %f", &TextureCoordsList[i].X, &TextureCoordsList[i].Y);

                VLN_LOG_VERBOSE("\tTexture coord [%d]: ", i);
                TextureCoordsList[i].Print();
                VLN_LOG_VERBOSE("\n");
            }
        }

        // Read polygon faces
        struct VMaterialInfo
        {
            VMaterial* Material;
            b8 bAppearFirstTime;
        };

        i32 NumMaterialsInModel = 0;
        TArray<VMaterialInfo> MaterialInfoByIndex(MaxMaterialsPerModel, { nullptr, true });
        TArray<i32> MaterialIndexByPolyIndex(MaxPoly, 0);

        {
            Line = FindLineCOB("Faces", File, Buffer, BufferSize);
            std::sscanf(Line, "Faces %d", &NumPoly);

            for (i32f i = 0; i < NumPoly; ++i)
            {
                i32 DummyInt;
                i32 MaterialIndex;

                // Get material index
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "Face verts %d flags %d mat %d", &DummyInt, &DummyInt, &MaterialIndex);

                MaterialIndexByPolyIndex[i] = MaterialIndex;
                if (MaterialInfoByIndex[MaterialIndex].bAppearFirstTime)
                {
                    ++NumMaterialsInModel;
                    MaterialInfoByIndex[MaterialIndex].bAppearFirstTime = false;
                }

                VLN_LOG_VERBOSE("\tMaterial index of poly face [%d]: %d\n", i, MaterialIndexByPolyIndex[i]);

                // Get vertex and texture indices
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "<%d,%d> <%d,%d> <%d,%d>",
                    &PolyList[i].VtxIndices[2], &PolyList[i].TextureCoordsIndices[2],
                    &PolyList[i].VtxIndices[1], &PolyList[i].TextureCoordsIndices[1],
                    &PolyList[i].VtxIndices[0], &PolyList[i].TextureCoordsIndices[0]
                );

                VLN_LOG_VERBOSE("\tVertex and texture indices:\n");
                for (i32f j = 0; j < 3; ++j)
                {
                    VLN_LOG_VERBOSE("\t<%d, %d>\n", PolyList[i].VtxIndices[j], PolyList[i].TextureCoordsIndices[j]);
                }

                // Set default stuff
                PolyList[i].State = EPolyState::Active;
            }

            VLN_LOG_VERBOSE("\tNum materials in object: %d\n", NumMaterialsInObject);
        }

        // Read materials
        {
            for (i32f i = 0; i < NumMaterialsInModel; ++i)
            {
                VMaterial* CurrentMaterial = World.AddMaterial();

                static constexpr i32f FormatSize = 256;
                char Format[FormatSize];

                static constexpr i32f ShaderNameSize = 64;
                char ShaderName[ShaderNameSize];

                // Read color and material parameters
                {
                    f32 R, G, B, A;

                    Line = FindLineCOB("mat#", File, Buffer, BufferSize);
                    Line = FindLineCOB("rgb", File, Buffer, BufferSize);
                    std::sscanf(Line, "rgb %f,%f,%f", &R, &G, &B);

                    CurrentMaterial->Color = MAP_ARGB32(
                        255, // Opaque by default, may be overriden by transparency shader
                        (i32)(R * 255.0f + 0.5f),
                        (i32)(G * 255.0f + 0.5f),
                        (i32)(B * 255.0f + 0.5f)
                    );

                    // Material parameters
                    Line = FindLineCOB("alpha", File, Buffer, BufferSize);
                    std::sscanf(Line, "alpha %f ka %f ks %f exp %f",
                        &A, &CurrentMaterial->KAmbient, &CurrentMaterial->KSpecular, &CurrentMaterial->Power
                    );
                    // @NOTE: We try to find diffuse factor below, 1.0f by default in VMaterial::Init()

                    // Log color
                    VLN_LOG_VERBOSE("\tMaterial color [%d]: ", i);
                    VVector4 ColorVector = { R, G, B, A };
                    ColorVector.Print();
                    VLN_LOG_VERBOSE("\n");
                }

                // Check if we have texture
                {
                    Line = FindLineCOB("Shader class: color", File, Buffer, BufferSize);
                    Line = GetLineCOB(File, Buffer, BufferSize);

                    std::snprintf(Format, FormatSize, "Shader name: \"%%%d[a-z ]\"", ShaderNameSize - 1);
                    std::sscanf(Line, Format, ShaderName);
                    VLN_LOG_VERBOSE("\tShader name: %s\n", ShaderName);

                    if (0 == std::strncmp(ShaderName, "texture map", ShaderNameSize))
                    {
                        static constexpr i32f TexturePathSize = 256;
                        char TexturePathRaw[TexturePathSize];
                        char TexturePath[TexturePathSize];

                        // Scan raw texture path
                        Line = FindLineCOB("file name:", File, Buffer, BufferSize);

                        std::snprintf(Format, FormatSize, "file name: string \"%%%d[0-9a-zA-Z\\/:. ]\"", TexturePathSize - 1);
                        std::sscanf(Line, Format, TexturePathRaw);

                        // Get real texture path
                        GetTexturePathFromModelDirectory(TexturePath, TexturePathSize, TexturePathRaw, Path);

                        // Load texture in material
                        CurrentMaterial->Texture.Load(TexturePath);
                        CurrentMaterial->Attr |= EPolyAttr::ShadeModeTexture;

                        // Texture in object
                        Attr |= EMeshAttr::HasTexture;

                        VLN_LOG_VERBOSE("\tMaterial has texture, file path: %s\n", TexturePath);
                    }
                }

                // Read transparency shader
                {
                    Line = FindLineCOB("Shader class: transparency", File, Buffer, BufferSize);
                    Line = FindLineCOB("Shader name", File, Buffer, BufferSize);

                    std::snprintf(Format, FormatSize, "Shader name: \"%%%d[a-z ]\"", ShaderNameSize - 1);
                    std::sscanf(Line, Format, ShaderName);

                    VLN_LOG_VERBOSE("\tTransparency shader name: %s\n", ShaderName);
                    if (0 == std::strncmp(ShaderName, "filter", ShaderNameSize))
                    {
                        i32 AlphaRed, AlphaGreen, AlphaBlue;

                        Line = FindLineCOB("colour: color", File, Buffer, BufferSize);
                        std::sscanf(Buffer, "colour: color (%d, %d, %d)", &AlphaRed, &AlphaGreen, &AlphaBlue);

                        CurrentMaterial->Color.A = VLN_MAX(AlphaRed, VLN_MAX(AlphaGreen, AlphaBlue));
                        CurrentMaterial->Attr |= EPolyAttr::Transparent;

                        VLN_LOG_VERBOSE("\tAlpha channel: %d\n", CurrentMaterial.Color.A);
                    }
                }

                // Read reflectance shader
                {
                    Line = FindLineCOB("Shader class: reflectance", File, Buffer, BufferSize);
                    Line = GetLineCOB(File, Buffer, BufferSize);

                    std::snprintf(Format, FormatSize, "Shader name: \"%%%d[a-z ]\"", ShaderNameSize - 1);
                    std::sscanf(Line, Format, ShaderName);

                    if (0 == strncmp(ShaderName, "constant", ShaderNameSize))
                    {
                        CurrentMaterial->Attr |= EPolyAttr::ShadeModeEmissive;
                    }
                    else if (0 == strncmp(ShaderName, "matte", ShaderNameSize))
                    {
                        CurrentMaterial->Attr |= EPolyAttr::ShadeModeFlat;
                    }
                    else if (0 == strncmp(ShaderName, "plastic", ShaderNameSize) ||
                             0 == strncmp(ShaderName, "phong", ShaderNameSize))
                    {
                        // We have no phong support, so we use gouraud for phong too
                        CurrentMaterial->Attr |= EPolyAttr::ShadeModeGouraud;
                    }
                    else
                    {
                        CurrentMaterial->Attr |= EPolyAttr::ShadeModeEmissive;
                    }

                    VLN_LOG_VERBOSE("\tShader name: %s\n", ShaderName);

                    // Try to find diffuse factor
                    i32 NumParams;

                    Line = FindLineCOB("Number of parameters:", File, Buffer, BufferSize);
                    std::sscanf(Line, "Number of parameters: %d", &NumParams);

                    const char Pattern[] = "diffuse";
                    const VSizeType PatternLength = std::strlen(Pattern);

                    for (i32f ParamIndex = 0; ParamIndex < NumParams; ++ParamIndex)
                    {
                        Line = GetLineCOB(File, Buffer, BufferSize);
                        if (0 == std::strncmp(Line, Pattern, PatternLength))
                        {
                            std::sscanf(Line, "diffuse factor: float %f", &CurrentMaterial->KDiffuse);
                            VLN_LOG_VERBOSE("\tDiffuse factor found: %f\n", CurrentMaterial->KDiffuse);
                            break;
                        }
                    }
                }

                // Precompute reflectivities for engine
                for (i32f RGBIndex = 1; RGBIndex < 4; ++RGBIndex)
                {
                    // @TODO: Check overflow
                    CurrentMaterial->RAmbient.C[RGBIndex]  = (u8)(CurrentMaterial->KAmbient * CurrentMaterial->Color.C[RGBIndex]);
                    CurrentMaterial->RDiffuse.C[RGBIndex]  = (u8)(CurrentMaterial->KDiffuse * CurrentMaterial->Color.C[RGBIndex]);
                    CurrentMaterial->RSpecular.C[RGBIndex] = (u8)(CurrentMaterial->RSpecular * CurrentMaterial->Color.C[RGBIndex]);

                    // Log precomputed colors and factors
                    VLN_LOG_VERBOSE("\tRa [%d]: %d\n", RGBIndex, CurrentMaterial.RAmbient.C[RGBIndex]);
                    VLN_LOG_VERBOSE("\tRd [%d]: %d\n", RGBIndex, CurrentMaterial.RDiffuse.C[RGBIndex]);
                    VLN_LOG_VERBOSE("\tRs [%d]: %d\n", RGBIndex, CurrentMaterial.RSpecular.C[RGBIndex]);
                }

                // Log factors
                VLN_LOG_VERBOSE("\tKa %f Kd %f Ks %f Exp %f\n",
                    CurrentMaterial.KAmbient,
                    CurrentMaterial.KDiffuse,
                    CurrentMaterial.KSpecular,
                    CurrentMaterial.Power
                );
            }
        }

        // Apply materials for polygons
        {
            for (i32f i = 0; i < NumPoly; ++i)
            {
                VPoly& Poly = PolyList[i];
                const VMaterial* PolyMaterial = MaterialInfoByIndex[MaterialIndexByPolyIndex[i]].Material;

                // Set color
                if (PolyMaterial->Attr & EPolyAttr::ShadeModeTexture)
                {
                    Poly.OriginalColor = MAP_ARGB32(PolyMaterial->Color.A, 255, 255, 255);
                }
                else
                {
                    Poly.OriginalColor = PolyMaterial->Color;
                }

                // Set shade mode and params
                if (Flags & ECOBFlags::OverrideShadeMode)
                {
                    Poly.Attr |= (u32)OverrideShadeMode;
                }
                else
                {
                    if (PolyMaterial->Attr & EPolyAttr::ShadeModeEmissive)
                    {
                        Poly.Attr |= EPolyAttr::ShadeModeEmissive;
                    }
                    else if (PolyMaterial->Attr & EPolyAttr::ShadeModeFlat)
                    {
                        Poly.Attr |= EPolyAttr::ShadeModeFlat;
                    }
                    else if (PolyMaterial->Attr & EPolyAttr::ShadeModeGouraud ||
                             PolyMaterial->Attr & EPolyAttr::ShadeModePhong)
                    {
                        Poly.Attr |= EPolyAttr::ShadeModeGouraud;
                    }
                }

                if (PolyMaterial->Attr & EPolyAttr::ShadeModeTexture)
                {
                    Poly.Attr |= EPolyAttr::ShadeModeTexture;

                    TransVtxList[Poly.VtxIndices[0]].Attr =
                        LocalVtxList[Poly.VtxIndices[0]].Attr |= EVertexAttr::HasTextureCoords;
                    TransVtxList[Poly.VtxIndices[1]].Attr =
                        LocalVtxList[Poly.VtxIndices[1]].Attr |= EVertexAttr::HasTextureCoords;
                    TransVtxList[Poly.VtxIndices[2]].Attr =
                        LocalVtxList[Poly.VtxIndices[2]].Attr |= EVertexAttr::HasTextureCoords;
                }

                // Set transparent flag
                if (PolyMaterial->Attr & EPolyAttr::Transparent)
                {
                    Poly.Attr |= EPolyAttr::Transparent;
                }

                // Set poly material
                Poly.Attr |= EPolyAttr::UsesMaterial;
                Poly.Material = PolyMaterial;
            }
        }

        // Fix texture coords
        {
            if (Attr & EMeshAttr::HasTexture)
            {
                for (i32f i = 0; i < NumTextureCoords; ++i)
                {
                    if (Flags & ECOBFlags::InvertU)
                    {
                        TextureCoordsList[i].X = 1 - TextureCoordsList[i].X;
                    }
                    if (Flags & ECOBFlags::InvertV)
                    {
                        TextureCoordsList[i].Y = 1 - TextureCoordsList[i].Y;
                    }
                    if (Flags & ECOBFlags::SwapUV)
                    {
                        VLN_SWAP(TextureCoordsList[i].X, TextureCoordsList[i].Y, TempFloat);
                    }
                }
            }
        }

        // Close file
        std::fclose(File);
    }

    // Compute stuff
    {
        ComputeRadius();
        ComputePolygonNormalsLength();
        ComputeVertexNormals();
    }

    VLN_NOTE(hLogCOB, "Object parsing ended\n");
    return true;
}

/******************************************************
                    Terrain Generation                *
 ******************************************************/

void VMesh::GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size, f32 Height, EShadeMode ShadeMode)
{
    Destroy();

    // Load texture in terrain material
    VMaterial* Material = World.AddMaterial();
    Material->Texture.Load(Texture, { 1.0f, 1.0f, 1.0f }, 1);

    // Load height map
    VSurface MapSurface;
    MapSurface.Load(HeightMap);
    i32 MapRowSize = MapSurface.GetWidth();

    // Lock height map surface
    u32* Buffer;
    i32 Pitch;
    MapSurface.Lock(Buffer, Pitch);

    // Allocate mesh
    i32f VerticesInRow = MapRowSize + 1;
    Allocate(VerticesInRow * VerticesInRow, (MapRowSize * MapRowSize) * 2, 1);

    // Set vertex positions
    f32 MapStep = (f32)MapRowSize / (f32)VerticesInRow;
    f32 UnitsPerHeight = (f32)Height / 255.0f;
    f32 TileSize = Size / MapRowSize;

    f32 YMap = 0.5f;
    for (i32f Y = 0; Y < VerticesInRow; ++Y, YMap += MapStep)
    {
        f32 XMap = 0.5f;
        for (i32f X = 0; X < VerticesInRow; ++X, XMap += MapStep)
        {
            i32f Index = Y*VerticesInRow + X;

             LocalVtxList[Index].Position = {
                X * TileSize,
                (f32)VColorARGB(Buffer[(i32f)YMap*Pitch + (i32f)XMap]).R * UnitsPerHeight,
                Y * TileSize,
             };

             TextureCoordsList[Index] = {
                (f32)X / (f32)VerticesInRow,
                1.0f - ((f32)Y / (f32)VerticesInRow) // Invert Y because we start from bottom
             };
        }
    }

    // Destroy height map surface
    MapSurface.Unlock();
    MapSurface.Destroy();

    // Set polygon info
    for (i32f Y = 0; Y < MapRowSize; ++Y)
    {
        for (i32f X = 0; X < MapRowSize; ++X)
        {
            i32f PolyIndex = (Y * 2)*MapRowSize + X*2;

            VPoly& Poly1 = PolyList[PolyIndex];
            VPoly& Poly2 = PolyList[PolyIndex + 1];

            Poly2.State         = Poly1.State        |= EPolyState::Active;
            Poly2.Attr          = Poly1.Attr         |= EPolyAttr::Terrain | (u32)ShadeMode | EPolyAttr::ShadeModeTexture;
            Poly2.OriginalColor = Poly1.OriginalColor = VColorARGB(0xFF, 0xFF, 0xFF, 0xFF);
            Poly2.Material      = Poly1.Material      = Material;

            Poly1.TextureCoordsIndices[0] = Poly1.VtxIndices[0] = Y*VerticesInRow + X;
            Poly1.TextureCoordsIndices[1] = Poly1.VtxIndices[1] = (Y + 1)*VerticesInRow + X;
            Poly1.TextureCoordsIndices[2] = Poly1.VtxIndices[2] = (Y + 1)*VerticesInRow + X + 1;

            Poly2.TextureCoordsIndices[0] = Poly2.VtxIndices[0] = Y*VerticesInRow + X;
            Poly2.TextureCoordsIndices[1] = Poly2.VtxIndices[1] = (Y + 1)*VerticesInRow + X + 1;
            Poly2.TextureCoordsIndices[2] = Poly2.VtxIndices[2] = Y*VerticesInRow + X + 1;
        }
    }

    // Set vertex info
    for (i32f i = 0; i < NumVtx; ++i)
    {
        TransVtxList[i].Attr = LocalVtxList[i].Attr |= EVertexAttr::HasTextureCoords;
    }

    // Compute stuff
    ComputeRadius();
    ComputePolygonNormalsLength();
    ComputeVertexNormals();

    Position = { -Size / 2.0f, -Height / 2.0f, -Size / 2.0f };
}

/******************************************************
                    MD2 Loading                       *
 ******************************************************/

static constexpr i32f MD2Magic = ('I') + ('D' << 8) + ('P' << 16) + ('2' << 24);
static constexpr i32f MD2Version = 8;
static constexpr i32f MD2SkinPathSize = 64;

class VMD2Header
{
public:
    i32 Magic;
    i32 Version;

    i32 SkinWidth;
    i32 SkinHeight;
    i32 FrameSize;

    i32 NumSkins;
    i32 NumVtx;
    i32 NumTextureCoords;
    i32 NumPoly;
    i32 NumOpenGLCmds;
    i32 NumFrames;

    i32 OffsetSkins;
    i32 OffsetTextureCoords;
    i32 OffsetPoly;
    i32 OffsetFrames;
    i32 OffsetOpenGLCmds;
    i32 OffsetEnd;
};

class VMD2Point
{
public:
    u8 X, Y, Z;
    u8 NormalIndex;
};

class VMD2TextureCoord
{
public:
    i16 U, V;
};

class VMD2Frame
{
public:
    f32 Scale[3];
    f32 Translation[3];
    char Name[16];
    VMD2Point VtxList[1]; // @FIXME: Maybe 0?
};

class VMD2Poly
{
public:
    u16 VtxIndices[3];
    u16 TextureIndices[3];
};

class VMD2Animation
{
public:
    i32 FrameStart;
    i32 FrameEnd;

    f32 InterpRate;
    f32 InterpOnceInSeconds;

    EAnimationInterpMode InterpMode = EAnimationInterpMode::Linear;
};

VLN_DEFINE_LOG_CHANNEL(hLogMD2, "MD2");

static VMD2Animation MD2AnimationTable[(i32f)EMD2AnimationId::MaxAnimations] = {
    { 0,   39,  0.5f,  0.05f, EAnimationInterpMode::Fixed },      // StandingIdle
    { 40,  45,  0.5f,  0.05f, EAnimationInterpMode::Fixed },      // Run
    { 46,  53,  0.5f,  0.05f },                                   // Attack
    { 54,  57,  0.5f,  0.05f },                                   // Pain1 
    { 58,  61,  0.5f,  0.05f },                                   // Pain2
    { 62,  65,  0.5f,  0.05f },                                   // Pain3
    { 66,  71,  0.5f,  0.05f },                                   // Jump
    { 72,  83,  0.5f,  0.05f },                                   // Flip
    { 84,  94,  0.5f,  0.05f },                                   // Salute
    { 95,  111, 0.5f,  0.05f },                                   // Taun
    { 112, 122, 0.5f,  0.05f },                                   // Wave
    { 123, 134, 0.5f,  0.05f },                                   // Point
    { 135, 153, 0.5f,  0.05f },                                   // CrouchStand
    { 154, 159, 0.5f,  0.05f, EAnimationInterpMode::Fixed },      // CrouchWalk
    { 160, 168, 0.5f,  0.05f },                                   // CrouchAttack
    { 169, 172, 0.5f,  0.05f },                                   // CrouchPain
    { 173, 177, 0.25f, 0.025f },                                  // CrouchDeath
    { 178, 183, 0.25f, 0.025f },                                  // DeathBack
    { 184, 189, 0.25f, 0.025f },                                  // DeathForward
    { 190, 197, 0.25f, 0.025f }                                   // DeathSlow
};

void VMesh::PlayAnimation(EMD2AnimationId AnimationId, b32 bLoop, EAnimationInterpMode InterpMode)
{
    if (~Attr & EMeshAttr::MultiFrame)
    {
        return;
    }

    CurrentAnimationId = AnimationId;
    AnimationInterpMode = InterpMode == EAnimationInterpMode::Default && !bLoop ? EAnimationInterpMode::Linear : InterpMode;
    AnimationTimeAccum = 0.0f;

    bLoopAnimation    = bLoop;
    bAnimationPlayed  = false;

    CurrentFrame = (f32)MD2AnimationTable[(i32f)AnimationId].FrameStart;
}

void VMesh::UpdateAnimationAndTransformModelToWorld(f32 DeltaTime)
{
    i32f Frame1 = (i32f)CurrentFrame;
    i32f Frame2;

    // Check frame overflow
    if (Frame1 < NumFrames)
    {
        Frame2 = Frame1 + 1;
    }
    else
    {
        Frame1 = NumFrames - 1;
        Frame2 = Frame1;
    }


    if (Frame2 < NumFrames) // Interpolate if we didn't overflow
    {
        f32 FrameInterp = CurrentFrame - Math.Floor(CurrentFrame);

        for (i32f VtxIndex = 0; VtxIndex < NumVtx; ++VtxIndex)
        {
            i32f Frame1Index = (Frame1 * NumVtx) + VtxIndex;
            TransVtxList[VtxIndex] = HeadLocalVtxList[Frame1Index]; // Copy other from position stuff

            // Interpolate
            i32f Frame2Index = Frame1Index + NumVtx;
            TransVtxList[VtxIndex].Position = Position +
                (1.0f - FrameInterp) * HeadLocalVtxList[Frame1Index].Position +
                FrameInterp          * HeadLocalVtxList[Frame2Index].Position;
        }
    }
    else // Get position from one frame on overflow
    {
        for (i32f VtxIndex = 0; VtxIndex < NumVtx; ++VtxIndex)
        {
            TransVtxList[VtxIndex] = HeadLocalVtxList[(Frame1 * NumVtx) + VtxIndex]; // Copy other from position stuff
            TransVtxList[VtxIndex].Position += Position;                             // Add world position
        }
    }

    // Check if we already played animation
    if (bAnimationPlayed)
    {
        return;
    }

    // Get animation and interpolation mode
    const VMD2Animation& Animation = MD2AnimationTable[(i32f)CurrentAnimationId];
    EAnimationInterpMode InterpMode = AnimationInterpMode != EAnimationInterpMode:: Default ? AnimationInterpMode : Animation.InterpMode;

    if (InterpMode == EAnimationInterpMode::Linear)
    {
        CurrentFrame += ((DeltaTime / 1000.0f) / Animation.InterpOnceInSeconds) * Animation.InterpRate;
        if (CurrentFrame >= Animation.FrameEnd)
        {
            if (bLoopAnimation)
            {
                // Interpolate a bit to make it more smooth
                CurrentFrame = (f32)Animation.FrameStart + Animation.InterpRate * (Math.Floor(CurrentFrame) / Animation.InterpOnceInSeconds);
                
                // Check possible case
                if (CurrentFrame > (f32)Animation.FrameEnd)
                {
                    CurrentFrame = (f32)Animation.FrameStart;
                }
            }
            else
            {
                CurrentFrame = (f32)Animation.FrameEnd;
                bAnimationPlayed = true;
            }
        }
    }
    else
    {
        AnimationTimeAccum += DeltaTime / 1000.0f;
        i32f InterpCount = (i32f)(AnimationTimeAccum / Animation.InterpOnceInSeconds);
        AnimationTimeAccum -= InterpCount * Animation.InterpOnceInSeconds;

        CurrentFrame += InterpCount * Animation.InterpRate;
        if (CurrentFrame >= Animation.FrameEnd)
        {
            if (bLoopAnimation)
            {
                // Interpolate a bit to make it more smooth
                CurrentFrame = (f32)Animation.FrameStart + Animation.InterpRate * (AnimationTimeAccum / Animation.InterpOnceInSeconds);
                AnimationTimeAccum = 0.0f;
                
                // Check possible case
                if (CurrentFrame > (f32)Animation.FrameEnd)
                {
                    CurrentFrame = (f32)Animation.FrameStart;
                }
            }
            else
            {
                CurrentFrame = (f32)Animation.FrameEnd;
                bAnimationPlayed = true;
            }
        }
    }
}

b32 VMesh::LoadMD2(const char* Path, const char* InSkinPath, i32 SkinIndex, VVector4 InPosition, VVector3 InScale, EShadeMode ShadeMode, const VVector3& ColorCorrection)
{
    VLN_NOTE(hLogMD2, "Parsing started\n");

    // Open file
    std::FILE* File;
    if (!(File = std::fopen(Path, "rb")))
    {
        VLN_ERROR(hLogMD2, "Can't open file %s: %s", Path, std::strerror(errno));
        return false;
    }

    // Get file length
    std::fseek(File, 0, SEEK_END);
    const i32f FileLength = std::ftell(File);

    VLN_LOG_VERBOSE("FileLength: %d\n", FileLength);

    // Allocate buffer for file
    std::fseek(File, 0, SEEK_SET);
    TArray<u8> FileBuffer(FileLength);

    // Read file in buffer and close file
    std::fread(FileBuffer.GetData(), sizeof(u8), FileLength, File);
    std::fclose(File);

    // Read header
    const VMD2Header* Header = (VMD2Header*)FileBuffer.GetData();
    if (Header->Magic != MD2Magic || Header->Version != 8)
    {
        VLN_ERROR(hLogMD2, "Magic number or version in header is not correct, Magic: %d, Version: %d", Header->Magic, Header->Version);
        return false;
    }

    VLN_LOG_VERBOSE("Header:\n"
        "\tSkinWidth: %d\n"
        "\tSkinHeight: %d\n"
        "\tFrameSize: %d\n"

        "\tNumSkins: %d\n"
        "\tNumVtx: %d\n"
        "\tNumTextureCoords: %d\n"
        "\tNumPoly: %d\n"
        "\tNumOpenGLCmds: %d\n"
        "\tNumFrames: %d\n"

        "\tOffsetSkins: %d\n"
        "\tOffsetTextureCoords: %d\n"
        "\tOffsetPoly: %d\n"
        "\tOffsetFrames: %d\n"
        "\tOffsetOpenGLCmds: %d\n"
        "\tOffsetEnd: %d\n",

        Header->SkinWidth,
        Header->SkinHeight,
        Header->FrameSize,

        Header->NumSkins,
        Header->NumVtx,
        Header->NumTextureCoords,
        Header->NumPoly,
        Header->NumOpenGLCmds,
        Header->NumFrames,

        Header->OffsetSkins,
        Header->OffsetTextureCoords,
        Header->OffsetPoly,
        Header->OffsetFrames,
        Header->OffsetOpenGLCmds,
        Header->OffsetEnd
    );

    // Initialize model 
    Attr = EMeshAttr::CanBeCulled | EMeshAttr::CastShadow | EMeshAttr::HasTexture | EMeshAttr::MultiFrame;
    Position = InPosition;
    Allocate(Header->NumVtx, Header->NumPoly, Header->NumFrames, Header->NumTextureCoords);

    // Read texture coords
    VMD2TextureCoord* MD2TextureCoords = (VMD2TextureCoord*)(FileBuffer.GetData() + Header->OffsetTextureCoords);
    for (i32f TextureCoord = 0; TextureCoord < NumTextureCoords; ++TextureCoord)
    {
        TextureCoordsList[TextureCoord].X = (f32)MD2TextureCoords[TextureCoord].U / (f32)Header->SkinWidth;
        TextureCoordsList[TextureCoord].Y = (f32)MD2TextureCoords[TextureCoord].V / (f32)Header->SkinHeight;
    }

    // Read frames with vertices
    for (i32f FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
    {
        VMD2Frame* Frame = (VMD2Frame*)(FileBuffer.GetData() + Header->OffsetFrames + (FrameIndex * Header->FrameSize));

        for (i32f VtxIndex = 0; VtxIndex < NumVtx; ++VtxIndex)
        {
            VVector3 Scale = { Frame->Scale[0] * InScale.X, Frame->Scale[1] * InScale.Y, Frame->Scale[2] * InScale.Z };
            VVector3 Translation = { Frame->Translation[0], Frame->Translation[1], Frame->Translation[2] };

            VVector4 Position = {
                (f32)Frame->VtxList[VtxIndex].Y * Scale.Y + Translation.Y, // MD2 Y = Volition X
                (f32)Frame->VtxList[VtxIndex].Z * Scale.Z + Translation.Z, // MD2 Z = Volition Y
                (f32)Frame->VtxList[VtxIndex].X * Scale.X + Translation.X, // MD2 X = Volition Z
            };

            HeadLocalVtxList[(FrameIndex * NumVtx) + VtxIndex].Position = Position;
        }
    }

    // Set up material
    VMaterial* Material = World.AddMaterial();

    if (InSkinPath)
    {
        VLN_LOG_VERBOSE("Skin Path: %s\n", InSkinPath);
        Material->Texture.Load(InSkinPath, ColorCorrection);
    }
    else
    {
        if (Header->NumSkins > 0)
        {
            SkinIndex %= Header->NumSkins;
        }

        const char* SkinPathRaw = (const char*)(FileBuffer.GetData() + Header->OffsetSkins + (SkinIndex * MD2SkinPathSize));
        char SkinPath[MD2SkinPathSize];
        GetTexturePathFromModelDirectory(SkinPath, MD2SkinPathSize, SkinPathRaw, Path);

        VLN_LOG_VERBOSE("Skin Path: %s\n", SkinPath);
        Material->Texture.Load(SkinPath, ColorCorrection);
    }

    // Read polygons
    VMD2Poly* MD2Polygons = (VMD2Poly*)(FileBuffer.GetData() + Header->OffsetPoly);
    for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
    {
        VPoly& Poly = PolyList[PolyIndex];

        // @NOTE: MD2 format use CCW order, so paste (0->2->1) -> (0->1->2)
        Poly.VtxIndices[0] = MD2Polygons[PolyIndex].VtxIndices[0];
        Poly.VtxIndices[1] = MD2Polygons[PolyIndex].VtxIndices[2];
        Poly.VtxIndices[2] = MD2Polygons[PolyIndex].VtxIndices[1];

        Poly.TextureCoordsIndices[0] = MD2Polygons[PolyIndex].TextureIndices[0];
        Poly.TextureCoordsIndices[1] = MD2Polygons[PolyIndex].TextureIndices[2];
        Poly.TextureCoordsIndices[2] = MD2Polygons[PolyIndex].TextureIndices[1];

        HeadTransVtxList[Poly.VtxIndices[0]].Attr = HeadLocalVtxList[Poly.VtxIndices[0]].Attr |= EVertexAttr::HasTextureCoords;
        HeadTransVtxList[Poly.VtxIndices[1]].Attr = HeadLocalVtxList[Poly.VtxIndices[1]].Attr |= EVertexAttr::HasTextureCoords;
        HeadTransVtxList[Poly.VtxIndices[2]].Attr = HeadLocalVtxList[Poly.VtxIndices[2]].Attr |= EVertexAttr::HasTextureCoords;

        Poly.State = EPolyState::Active;
        Poly.Attr = (u32)ShadeMode | EPolyAttr::RGB32 | EPolyAttr::ShadeModeTexture | EPolyAttr::UsesMaterial;
        Poly.OriginalColor = MAP_XRGB32(0xFF, 0xFF, 0xFF);

        Poly.Material = Material;
    }

    ComputeRadius();
    ComputePolygonNormalsLength();
    ComputeVertexNormals();

    VLN_NOTE(hLogMD2, "Parsing ended\n");
    return true;
}

}
