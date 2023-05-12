#include <cstdio>
#include <cerrno>
#include "Common/Types/Common.h"
#include "Common/Types/Array.h"
#include "Common/Platform/Memory.h"
#include "Common/Math/Vector.h"
#include "Engine/Graphics/Rendering/Renderer.h"
#include "Engine/Graphics/Scene/Mesh.h"

namespace Volition
{

VMesh::VMesh()
{
    Memory.MemSetByte(this, 0, sizeof(*this));

    State = EMeshState::Active | EMeshState::Visible;

    UX = { 1.0f, 0.0f, 0.0f };
    UY = { 0.0f, 1.0f, 0.0f };
    UZ = { 0.0f, 0.0f, 1.0f };

    NumFrames = 1;
    CurrentFrame = 0;
}

void VMesh::Allocate(i32 InNumVtx, i32 InNumPoly, i32 InNumFrames, i32 InNumTextureCoords)
{
    HeadLocalVtxList = LocalVtxList = new VVertex[InNumVtx * InNumFrames];
    HeadTransVtxList = TransVtxList = new VVertex[InNumVtx * InNumFrames];
    Memory.MemSetByte(HeadLocalVtxList, 0, sizeof(VVertex) * (InNumVtx * InNumFrames));
    Memory.MemSetByte(HeadTransVtxList, 0, sizeof(VVertex) * (InNumVtx * InNumFrames));

    PolyList          = new VPoly[InNumPoly];
    NumTextureCoords = InNumTextureCoords != -1 ? InNumTextureCoords : InNumPoly * 3;
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
    VLN_SAFE_DELETE_ARRAY(AverageRadiusList);
    VLN_SAFE_DELETE_ARRAY(MaxRadiusList);
    VLN_SAFE_DELETE_ARRAY(TextureCoordsList);
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

void VMesh::SetFrame(i32 Frame)
{
    if (~Attr & EMeshAttr::MultiFrame)
    {
        return;
    }

    if (Frame < 0)
    {
        Frame = 0;
    }
    else if (Frame >= NumFrames)
    {
        Frame = NumFrames - 1;
    }

    CurrentFrame = Frame;

    LocalVtxList = &HeadLocalVtxList[Frame * NumVtx];
    TransVtxList = &HeadTransVtxList[Frame * NumVtx];
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
    i32* NumPolyTouchVtx = new i32[NumVtx];
    Memory.MemSetQuad(NumPolyTouchVtx, 0, NumVtx);

    for (i32f i = 0; i < NumPoly; ++i)
    {
        if (PolyList[i].Attr & EPolyAttr::ShadeModeGouraud)
        {
            const i32f V0 = PolyList[i].VtxIndices[0];
            const i32f V1 = PolyList[i].VtxIndices[1];
            const i32f V2 = PolyList[i].VtxIndices[2];

            const VVector4 U = LocalVtxList[V1].Position - LocalVtxList[V0].Position;
            const VVector4 V = LocalVtxList[V2].Position - LocalVtxList[V0].Position;

            VVector4 Normal;
            VVector4::Cross(U, V, Normal);

            LocalVtxList[V0].Normal += Normal;
            LocalVtxList[V1].Normal += Normal;
            LocalVtxList[V2].Normal += Normal;

            ++NumPolyTouchVtx[V0];
            ++NumPolyTouchVtx[V1];
            ++NumPolyTouchVtx[V2];
        }
    }

    for (i32f i = 0; i < NumVtx; ++i)
    {
        if (NumPolyTouchVtx[i] > 0)
        {
            LocalVtxList[i].Normal /= (f32)NumPolyTouchVtx[i];
            LocalVtxList[i].Normal.Normalize();

            TransVtxList[i].Attr = LocalVtxList[i].Attr |= EVertexAttr::HasNormal;

            VLN_LOG_VERBOSE("Vertex normal [%d]: <%.2f %.2f %.2f>\n", i, LocalVtxList[i].Normal.X, LocalVtxList[i].Normal.Y, LocalVtxList[i].Normal.Z);
        }
    }

    delete[] NumPolyTouchVtx;
}

void VMesh::TransformModelToWorld(ETransformType Type)
{
    if (Type == ETransformType::LocalToTrans)
    {
        for (i32f i = 0; i < NumVtx; ++i)
        {
            TransVtxList[i] = LocalVtxList[i];
            TransVtxList[i].Position = LocalVtxList[i].Position + Position;
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
        f32 ZTest = (0.5f * Cam.ViewPlaneSize.X) * (SpherePos.Z / Cam.ViewDist);

        if (SpherePos.X - MaxRadius > ZTest ||  // Check Sphere's Left with Right side
            SpherePos.X + MaxRadius < -ZTest)   // Check Sphere's Right with Left side
        {
            State |= EMeshState::Culled;
            return true;
        }
    }

    if (CullType & ECullType::Y)
    {
        f32 ZTest = (0.5f * Cam.ViewPlaneSize.Y) * (SpherePos.Z / Cam.ViewDist);

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

b32 VMesh::LoadCOB(const char* Path, const VVector4& InPosition, const VVector4& Scale, u32 Flags)
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
        i32 NumMaterialsInObject = 0;
        TArray<b8> DoesMaterialAppearFirstTime(Renderer.MaxMaterials, true);
        TArray<i32> MaterialIndexByPolyIndex(GetMaxPoly(), 0);

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
                if (DoesMaterialAppearFirstTime[MaterialIndex])
                {
                    ++NumMaterialsInObject;
                    DoesMaterialAppearFirstTime[MaterialIndex] = false;
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
            for (i32f i = 0; i < NumMaterialsInObject; ++i)
            {
                VMaterial& CurrentMaterial = Renderer.Materials[Renderer.NumMaterials + i];

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

                    CurrentMaterial.Color = MAP_ARGB32(
                        255, // Opaque by default, may be overriden by transparency shader
                        (i32)(R * 255.0f + 0.5f),
                        (i32)(G * 255.0f + 0.5f),
                        (i32)(B * 255.0f + 0.5f)
                    );

                    // Material parameters
                    Line = FindLineCOB("alpha", File, Buffer, BufferSize);
                    std::sscanf(Line, "alpha %f ka %f ks %f exp %f",
                        &A, &CurrentMaterial.KAmbient, &CurrentMaterial.KSpecular, &CurrentMaterial.Power
                    );

                    // We try to find diffuse factor below, but this is default
                    CurrentMaterial.KDiffuse = 1.0f;

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
                        CurrentMaterial.Texture.Load(TexturePath);
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeTexture;

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

                        CurrentMaterial.Color.A = VLN_MAX(AlphaRed, VLN_MAX(AlphaGreen, AlphaBlue));
                        CurrentMaterial.Attr |= EMaterialAttr::Transparent;

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
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeEmissive;
                    }
                    else if (0 == strncmp(ShaderName, "matte", ShaderNameSize))
                    {
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeFlat;
                    }
                    else if (0 == strncmp(ShaderName, "plastic", ShaderNameSize) ||
                             0 == strncmp(ShaderName, "phong", ShaderNameSize))
                    {
                        // We have no phong support, so we use gouraud for phong too
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeGouraud;
                    }
                    else
                    {
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeEmissive;
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
                            std::sscanf(Line, "diffuse factor: float %f", &CurrentMaterial.KDiffuse);
                            VLN_LOG_VERBOSE("\tDiffuse factor found: %f\n", CurrentMaterial.KDiffuse);
                            break;
                        }
                    }
                }

                // Precompute reflectivities for engine
                for (i32f RGBIndex = 1; RGBIndex < 4; ++RGBIndex)
                {
                    CurrentMaterial.RAmbient.C[RGBIndex]  = (u8)(CurrentMaterial.KAmbient * CurrentMaterial.Color.C[RGBIndex]);
                    CurrentMaterial.RDiffuse.C[RGBIndex]  = (u8)(CurrentMaterial.KDiffuse * CurrentMaterial.Color.C[RGBIndex]);
                    CurrentMaterial.RSpecular.C[RGBIndex] = (u8)(CurrentMaterial.RSpecular * CurrentMaterial.Color.C[RGBIndex]);

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
                const VMaterial& PolyMaterial = Renderer.Materials[Renderer.NumMaterials + MaterialIndexByPolyIndex[i]];

                // Set color
                if (PolyMaterial.Attr & EMaterialAttr::ShadeModeTexture)
                {
                    Poly.OriginalColor = MAP_ARGB32(PolyMaterial.Color.A, 255, 255, 255);
                }
                else
                {
                    Poly.OriginalColor = PolyMaterial.Color;
                }

                // Set shade mode and params
                if (PolyMaterial.Attr & EMaterialAttr::ShadeModeEmissive)
                {
                    Poly.Attr |= EPolyAttr::ShadeModeEmissive;
                }
                else if (PolyMaterial.Attr & EMaterialAttr::ShadeModeFlat)
                {
                    Poly.Attr |= EPolyAttr::ShadeModeFlat;
                }
                else if (PolyMaterial.Attr & EMaterialAttr::ShadeModeGouraud ||
                    PolyMaterial.Attr & EMaterialAttr::ShadeModePhong)
                {
                    Poly.Attr |= EPolyAttr::ShadeModeGouraud;
                }

                if (PolyMaterial.Attr & EMaterialAttr::ShadeModeTexture)
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
                if (PolyMaterial.Attr & EMaterialAttr::Transparent)
                {
                    Poly.Attr |= EPolyAttr::Transparent;
                }

                // Set poly material
                Poly.Attr |= EPolyAttr::UsesMaterial;
                Poly.Material = &PolyMaterial;
            }
        }

        // Update num materials in engine
        Renderer.NumMaterials += NumMaterialsInObject;

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

void VMesh::GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size, f32 Height)
{
    // Load texture in terrain material
    VMaterial& Material = Renderer.Materials[Renderer.NumMaterials];
    Material.Texture.Load(Texture, 1);

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
                X*TileSize,
                (f32)VColorARGB(Buffer[(i32f)YMap*Pitch + (i32f)XMap]).R * UnitsPerHeight,
                Y * TileSize,
             };

             TextureCoordsList[Index] = {
                (f32)X / (f32)VerticesInRow,
                (f32)Y / (f32)VerticesInRow
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
            Poly2.Attr          = Poly1.Attr         |= EPolyAttr::Terrain | EPolyAttr::ShadeModeGouraud | EPolyAttr::ShadeModeTexture;
            Poly2.OriginalColor = Poly1.OriginalColor = VColorARGB(0xFF, 0xFF, 0xFF, 0xFF);
            Poly2.Material      = Poly1.Material      = &Material;

            Poly1.TextureCoordsIndices[0] = Poly1.VtxIndices[0] = (Y + 1)*VerticesInRow + X;
            Poly1.TextureCoordsIndices[1] = Poly1.VtxIndices[1] = (Y + 1)*VerticesInRow + X + 1;
            Poly1.TextureCoordsIndices[2] = Poly1.VtxIndices[2] = Y*VerticesInRow + X;

            Poly2.TextureCoordsIndices[0] = Poly2.VtxIndices[0] = Y*VerticesInRow + X + 1;
            Poly2.TextureCoordsIndices[1] = Poly2.VtxIndices[1] = Y*VerticesInRow + X;
            Poly2.TextureCoordsIndices[2] = Poly2.VtxIndices[2] = (Y + 1)*VerticesInRow + X + 1;
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

static constexpr i32f MD2MaxAnimations = 20;
static constexpr i32f MD2SkinPathSize = 64;

enum class EMD2AnimState : u8
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
};

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

VLN_DEFINE_LOG_CHANNEL(hLogMD2, "MD2");

b32 VMesh::LoadMD2(const char* Path, const char* InSkinPath, i32 SkinIndex, VVector4 InPosition, VVector3 InScale, u32 Flags)
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
        // @TODO: Is it right?
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
                (f32)Frame->VtxList[VtxIndex].X * Scale.X + Translation.X,
                (f32)Frame->VtxList[VtxIndex].Z * Scale.Z + Translation.Z, // MD2 Z = Volition Y
                (f32)Frame->VtxList[VtxIndex].Y * Scale.Y + Translation.Y, // MD2 Y = Volition Z
            };

            HeadLocalVtxList[(FrameIndex * NumVtx) + VtxIndex].Position = Position;
        }
    }

    // Set up material
    VMaterial* Material = &Renderer.Materials[Renderer.NumMaterials];
    ++Renderer.NumMaterials;

    if (InSkinPath)
    {
        VLN_LOG_VERBOSE("Skin Path: %s\n", InSkinPath);
        Material->Texture.Load(InSkinPath);
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
        Material->Texture.Load(SkinPath);
    }

    // Read polygons
    VMD2Poly* MD2Polygons = (VMD2Poly*)(FileBuffer.GetData() + Header->OffsetPoly);
    for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
    {
        VPoly& Poly = PolyList[PolyIndex];

        Poly.VtxIndices[0] = MD2Polygons[PolyIndex].VtxIndices[0];
        Poly.VtxIndices[1] = MD2Polygons[PolyIndex].VtxIndices[1];
        Poly.VtxIndices[2] = MD2Polygons[PolyIndex].VtxIndices[2];

        Poly.TextureCoordsIndices[0] = MD2Polygons[PolyIndex].TextureIndices[0];
        Poly.TextureCoordsIndices[1] = MD2Polygons[PolyIndex].TextureIndices[1];
        Poly.TextureCoordsIndices[2] = MD2Polygons[PolyIndex].TextureIndices[2];

        HeadTransVtxList[Poly.VtxIndices[0]].Attr = HeadLocalVtxList[Poly.VtxIndices[0]].Attr |= EVertexAttr::HasTextureCoords;
        HeadTransVtxList[Poly.VtxIndices[1]].Attr = HeadLocalVtxList[Poly.VtxIndices[1]].Attr |= EVertexAttr::HasTextureCoords;
        HeadTransVtxList[Poly.VtxIndices[2]].Attr = HeadLocalVtxList[Poly.VtxIndices[2]].Attr |= EVertexAttr::HasTextureCoords;

        Poly.State = EPolyState::Active;
        Poly.Attr = (Flags & EMD2Flags::ShadeModeFlat ? EPolyAttr::ShadeModeFlat : EPolyAttr::ShadeModeGouraud) | EPolyAttr::RGB32 | EPolyAttr::ShadeModeTexture | EPolyAttr::UsesMaterial;
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
