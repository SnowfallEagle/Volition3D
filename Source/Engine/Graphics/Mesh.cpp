#include <cstdio>
#include <cerrno>
#include "Engine/Core/Types.h"
#include "Engine/Core/Memory.h"
#include "Engine/Math/Vector.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/Mesh.h"

namespace Volition
{

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

        i32f I;
        for (I = 0;
             I < Len && (Buffer[I] == ' ' || Buffer[I] == '\t' || Buffer[I] == '\r' || Buffer[I] == '\n');
             ++I)
            {}

        if (I < Len)
        {
            return &Buffer[I];
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

        for (i32f I = 0; I < PatternLength; ++I)
        {
            if (Pattern[I] != Line[I])
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

    VMatrix44 MatLocal = VMatrix44::Identity;
    VMatrix44 MatWorld = VMatrix44::Identity;

    // Initialization 
    {
        Position = InPosition;
        Attr |= EMeshAttr::CanBeCulled;
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
            VLN_LOG("\tName: %s\n", Name);
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

            VLN_LOG("\tMatLocal: ");
            MatLocal.Print();
            VLN_LOG("\n");
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

            VLN_LOG("\tMatWorld: ");
            MatWorld.Print();
            VLN_LOG("\n");
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

            for (i32f I = 0; I < NumWorldVertices; ++I)
            {
                // Parse
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "%f %f %f", &LocalVtxList[I].X, &LocalVtxList[I].Y, &LocalVtxList[I].Z);
                LocalVtxList[I].W = 1.0f;

                // Apply matrix transformations
                VVector4 TempVector;
                VMatrix44::MulVecMat(LocalVtxList[I].Position, MatLocal, TempVector);
                VMatrix44::MulVecMat(TempVector, MatWorld, LocalVtxList[I].Position);

                // Swap YZ
                if (Flags & ECOB::SwapYZ)
                {
                    f32 TempFloat;
                    VLN_SWAP(LocalVtxList[I].Y, LocalVtxList[I].Z, TempFloat);
                    LocalVtxList[I].Z = -LocalVtxList[I].Z;
                }

                // Scale
                LocalVtxList[I].X *= Scale.X;
                LocalVtxList[I].Y *= Scale.Y;
                LocalVtxList[I].Z *= Scale.Z;

                // Log
                VLN_LOG("\tVertex [%d]: ", I);
                LocalVtxList[I].Position.Print();
                VLN_LOG("\n");
            }
        }

        // Read texture coords
        i32f NumTextureVtx;
        {
            Line = FindLineCOB("Texture Vertices", File, Buffer, BufferSize);
            std::sscanf(Line, "Texture Vertices %d", &NumTextureVtx);
            VLN_LOG("\tNum texture coords: %d\n", NumTextureVtx);

            for (i32f I = 0; I < NumTextureVtx; ++I)
            {
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "%f %f", &TextureCoordsList[I].X, &TextureCoordsList[I].Y);

                VLN_LOG("\tTexture coord [%d]: ", I);
                TextureCoordsList[I].Print();
                VLN_LOG("\n");
            }
        }

        // Read polygon faces
        i32 NumMaterialsInObject = 0;
        b32 DoesMaterialAppearFirstTime[Renderer.MaxMaterials];
        i32 MaterialIndexByPolyIndex[MaxPoly];

        Memory.MemSetQuad(&MaterialIndexByPolyIndex, 0, MaxPoly);
        Memory.MemSetQuad(&DoesMaterialAppearFirstTime, (b32)true, Renderer.MaxMaterials);
        {
            Line = FindLineCOB("Faces", File, Buffer, BufferSize);
            std::sscanf(Line, "Faces %d", &NumPoly);

            for (i32f I = 0; I < NumPoly; ++I)
            {
                i32 DummyInt;
                i32 MaterialIndex;

                // Get material index
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "Face verts %d flags %d mat %d", &DummyInt, &DummyInt, &MaterialIndex);

                MaterialIndexByPolyIndex[I] = MaterialIndex;
                if (DoesMaterialAppearFirstTime[MaterialIndex])
                {
                    ++NumMaterialsInObject;
                    DoesMaterialAppearFirstTime[MaterialIndex] = false;
                }

                VLN_LOG("\tMaterial index of poly face [%d]: %d\n", I, MaterialIndexByPolyIndex[I]);

                // Get vertex and texture indices
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "<%d,%d> <%d,%d> <%d,%d>",
                    &PolyList[I].VtxIndices[2], &PolyList[I].TextureCoordsIndices[2],
                    &PolyList[I].VtxIndices[1], &PolyList[I].TextureCoordsIndices[1],
                    &PolyList[I].VtxIndices[0], &PolyList[I].TextureCoordsIndices[0]
                );

                VLN_LOG("\tVertex and texture indices:\n");
                for (i32f J = 0; J < 3; ++J)
                {
                    VLN_LOG("\t<%d, %d>\n", PolyList[I].VtxIndices[J], PolyList[I].TextureCoordsIndices[J]);
                }

                // Set default stuff
                PolyList[I].State = EPolyState::Active;
            }

            VLN_LOG("\tNum materials in object: %d\n", NumMaterialsInObject);
        }

        // Read materials
        {
            for (i32f I = 0; I < NumMaterialsInObject; ++I)
            {
                VMaterial& CurrentMaterial = Renderer.Materials[Renderer.NumMaterials + I];

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
                    VLN_LOG("\tMaterial color [%d]: ", I);
                    VVector4 ColorVector = { R, G, B, A };
                    ColorVector.Print();
                    VLN_LOG("\n");
                }

                // Check if we have texture
                {
                    Line = FindLineCOB("Shader class: color", File, Buffer, BufferSize);
                    Line = GetLineCOB(File, Buffer, BufferSize);

                    std::snprintf(Format, FormatSize, "Shader name: \"%%%d[a-z ]\"", ShaderNameSize - 1);
                    std::sscanf(Line, Format, ShaderName);
                    VLN_LOG("\tShader name: %s\n", ShaderName);

                    if (0 == std::strncmp(ShaderName, "texture map", ShaderNameSize))
                    {
                        static constexpr i32f TexturePathSize = 256;
                        char TexturePathRaw[TexturePathSize];
                        char TexturePath[TexturePathSize];

                        // Scan raw texture path
                        Line = FindLineCOB("file name:", File, Buffer, BufferSize);

                        std::snprintf(Format, FormatSize, "file name: string \"%%%d[0-9a-zA-Z\\/:. ]\"", TexturePathSize - 1);
                        std::sscanf(Line, Format, TexturePathRaw);

                        // Cut stuff
                        i32f CharIndex;
                        for (CharIndex = std::strlen(TexturePathRaw) - 1; CharIndex >= 0; --CharIndex)
                        {
                            if (TexturePathRaw[CharIndex] == '\\' || TexturePathRaw[CharIndex] == '/')
                            {
                                break;
                            }
                        }
                        std::strncpy(TexturePath, TexturePathRaw + (CharIndex + 1), TexturePathSize);

                        // Load texture in material
                        std::strncpy(CurrentMaterial.Name, TexturePath, CurrentMaterial.NameSize);
                        CurrentMaterial.Texture.LoadBMP(TexturePath);
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeTexture;

                        // Texture in object
                        Attr |= EMeshAttr::HasTexture;

                        VLN_LOG("\tMaterial has texture, file path: %s\n", TexturePath);
                    }
                }

                // Read transparency shader
                {
                    Line = FindLineCOB("Shader class: transparency", File, Buffer, BufferSize);
                    Line = FindLineCOB("Shader name", File, Buffer, BufferSize);

                    std::snprintf(Format, FormatSize, "Shader name: \"%%%d[a-z ]\"", ShaderNameSize - 1);
                    std::sscanf(Line, Format, ShaderName);

                    VLN_LOG("\tTransparency shader name: %s\n", ShaderName);
                    if (0 == std::strncmp(ShaderName, "filter", ShaderNameSize))
                    {
                        i32 AlphaRed, AlphaGreen, AlphaBlue;

                        Line = FindLineCOB("colour: color", File, Buffer, BufferSize);
                        std::sscanf(Buffer, "colour: color (%d, %d, %d)", &AlphaRed, &AlphaGreen, &AlphaBlue);

                        CurrentMaterial.Color.A = VLN_MAX(AlphaRed, VLN_MAX(AlphaGreen, AlphaBlue));
                        CurrentMaterial.Attr |= EMaterialAttr::Transparent;

                        VLN_LOG("\tAlpha channel: %d\n", CurrentMaterial.Color.A);
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

                    VLN_LOG("\tShader name: %s\n", ShaderName);

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
                            VLN_LOG("\tDiffuse factor found: %f\n", CurrentMaterial.KDiffuse);
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
                    VLN_LOG("\tRa [%d]: %d\n", RGBIndex, CurrentMaterial.RAmbient.C[RGBIndex]);
                    VLN_LOG("\tRd [%d]: %d\n", RGBIndex, CurrentMaterial.RDiffuse.C[RGBIndex]);
                    VLN_LOG("\tRs [%d]: %d\n", RGBIndex, CurrentMaterial.RSpecular.C[RGBIndex]);
                }

                // Log factors
                VLN_LOG("\tKa %f Kd %f Ks %f Exp %f\n",
                    CurrentMaterial.KAmbient,
                    CurrentMaterial.KDiffuse,
                    CurrentMaterial.KSpecular,
                    CurrentMaterial.Power
                );
            }
        }

        // Apply materials for polygons
        {
            for (i32f I = 0; I < NumPoly; ++I)
            {
                VPoly& Poly = PolyList[I];
                const VMaterial& PolyMaterial = Renderer.Materials[Renderer.NumMaterials + MaterialIndexByPolyIndex[I]];

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
                for (i32f I = 0; I < NumTextureVtx; ++I)
                {
                    if (Flags & ECOB::InvertU)
                    {
                        TextureCoordsList[I].X = 1 - TextureCoordsList[I].X;
                    }
                    if (Flags & ECOB::InvertV)
                    {
                        TextureCoordsList[I].Y = 1 - TextureCoordsList[I].Y;
                    }
                    if (Flags & ECOB::SwapUV)
                    {
                        f32 TempFloat;
                        VLN_SWAP(TextureCoordsList[I].X, TextureCoordsList[I].Y, TempFloat);
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

void VMesh::GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size, f32 Height)
{
    // Load texture in terrain material
    VMaterial& Material = Renderer.Materials[Renderer.NumMaterials];
    Material.Texture.LoadBMP(Texture, 1);

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
             LocalVtxList[Y*VerticesInRow + X].Position = {
                X * TileSize,
                (f32)VColorARGB(Buffer[(i32f)YMap*Pitch + (i32f)XMap]).R * UnitsPerHeight,
                -Y * TileSize,
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
            Poly2.Attr          = Poly1.Attr         |= EPolyAttr::ShadeModeGouraud | EPolyAttr::TwoSided /* @TODO: | EPolyAttr::ShadeModeTexture */;
            Poly2.OriginalColor = Poly1.OriginalColor = VColorARGB(0xFF, 0xFF, 0xFF, 0xFF);

            Poly1.VtxIndices[0] = Y*VerticesInRow + X;
            Poly1.VtxIndices[1] = (Y + 1)*VerticesInRow + X + 1;
            Poly1.VtxIndices[2] = (Y + 1)*VerticesInRow + X;

            Poly2.VtxIndices[0] = Poly1.VtxIndices[0];
            Poly2.VtxIndices[1] = Y*VerticesInRow + X + 1;
            Poly2.VtxIndices[2] = Poly1.VtxIndices[1];

            /* @TODO
                Poly.TextureCoordsIndices;
            */
        }
    }

    // Compute stuff
    ComputeRadius();
    ComputePolygonNormalsLength();
    ComputeVertexNormals();
}

}
