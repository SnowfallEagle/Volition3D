#include <cstdio>
#include <cerrno>
#include "Core/Types.h"
#include "Core/Memory.h"
#include "Math/Vector.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"

/* ==============================================
                    PLG/X Loader
   ============================================== */
namespace EPLX
{
    enum
    {
        RGBFlag = BIT(16),
        TwoSidedFlag = BIT(13),

        ShadeModeMask        = BIT(15) | BIT(14),
        ShadeModePureFlag    = 0,
        ShadeModeFlatFlag    = BIT(14),
        ShadeModeGouraudFlag = BIT(15),
        ShadeModePhongFlag   = BIT(14) | BIT(15),

        RGB16Mask = 0x0FFF,
        RGB8Mask  = 0x00FF,
    };
}

DEFINE_LOG_CHANNEL(hLogPLX, "PLX Loader"); // TODO(sean)

char* GetLinePLG(std::FILE* File, char* Buffer, i32 Size)
{
    for (;;)
    {
        if (!std::fgets(Buffer, Size, File))
        {
            return nullptr;
        }

        i32f I, Len = std::strlen(Buffer);
        for (I = 0;
             I < Len && (Buffer[I] == ' ' || Buffer[I] == '\t' || Buffer[I] == '\r' || Buffer[I] == '\n');
             ++I)
            {}

        if (I < Len && Buffer[I] != '#')
        {
            return &Buffer[I];
        }
    }
}

b32 VObject::LoadPLG(
    const char* Path,
    const VVector4& InPosition,
    const VVector4& Scale,
    const VVector4& Rot
)
{
    static constexpr i32f BufferSize = 4096;

    // Initialize object
    {
        Clean();
        Position = InPosition;
    }

    // Load from file
    {
        std::FILE* File;
        char Buffer[BufferSize];

        // Open
        if (!(File = std::fopen(Path, "rb")))
        {
            VL_ERROR(hLogPLX, "Can't open %s file: %s\n", Path, strerror(errno));
            return false;
        }

        // Read header and allocate object
        {
            if (!GetLinePLG(File, Buffer, BufferSize))
            {
                VL_ERROR(hLogObject, "Can't read PLG header\n");
                std::fclose(File);
                return false;
            }
            if (sscanf(Buffer, "%s %d %d", Name, &NumVtx, &NumPoly) <= 0)
            {
                VL_ERROR(hLogObject, "Can't parse PLG header\n");
                std::fclose(File);
                return false;
            }

            Allocate(NumVtx, NumPoly, NumFrames);

            VL_NOTE(hLogObject, "Loading %s %d %d\n", Name, NumVtx, NumPoly);
        }

        // Read vertices and compute radius
        {
            VL_LOG("Vertices:\n");

            for (i32f I = 0; I < NumVtx; ++I)
            {
                if (!GetLinePLG(File, Buffer, BufferSize))
                {
                    VL_ERROR(hLogObject, "Can't parse PLG vertices\n");
                    std::fclose(File);
                    return false;
                }

                // Parse
                if (sscanf(Buffer, "%f %f %f",
                    &LocalVtxList[I].X, &LocalVtxList[I].Y, &LocalVtxList[I].Z) <= 0)
                {
                    VL_ERROR(hLogObject, "Can't parse vertex\n");
                    std::fclose(File);
                    return false;
                }
                LocalVtxList[I].W = 1.0f;

                // Scale
                LocalVtxList[I].X *= Scale.X;
                LocalVtxList[I].Y *= Scale.Y;
                LocalVtxList[I].Z *= Scale.Z;

                // Log
                VL_LOG("\t<%f, %f, %f>\n",
                    LocalVtxList[I].X, LocalVtxList[I].Y, LocalVtxList[I].Z
                );
            }
        }

        // Read polygons
        {
            i32f PolyDesc;
            char StrPolyDesc[BufferSize];

            VL_LOG("Polygons:\n");

            for (i32f I = 0; I < NumPoly; ++I)
            {
                if (!GetLinePLG(File, Buffer, BufferSize))
                {
                    VL_ERROR(hLogObject, "Can't parse PLG polygons\n");
                    std::fclose(File);
                    return false;
                }

                // Parse
                i32 NumPolyVtx;
                if (std::sscanf(Buffer, "%255s %d %d %d %d",
                    StrPolyDesc, &NumPolyVtx,
                    &PolyList[I].VtxIndices[0], &PolyList[I].VtxIndices[1], &PolyList[I].VtxIndices[2]) <= 0)
                {
                    VL_ERROR(hLogObject, "Can't parse polygon\n");
                    std::fclose(File);
                    return false;
                }

                // Get polygon description
                if (StrPolyDesc[1] == 'x' || StrPolyDesc[1] == 'X')
                {
                    if (std::sscanf(Buffer, "%x", &PolyDesc) <= 0)
                    {
                        VL_ERROR(hLogObject, "Can't parse polygon description\n");
                        fclose(File);
                        return false;
                    }
                }
                else
                {
                    if (sscanf(Buffer, "%d", &PolyDesc) <= 0)
                    {
                        VL_ERROR(hLogObject, "Can't parse polygon description\n");
                        std::fclose(File);
                        return false;
                    }
                }

                // Log
                VL_LOG("\t<0x%x, %d, <%d %d %d>>\n",
                    PolyDesc, NumVtx,
                    PolyList[I].VtxIndices[0], PolyList[I].VtxIndices[1], PolyList[I].VtxIndices[2]
                );

                // Set attributes and color
                if (PolyDesc & EPLX::RGBFlag)
                {
                    u32 Color444 = PolyDesc & EPLX::RGB16Mask;

                    /* NOTE(sean):
                        4 bit is 0xF but we need 8 since we use rgb32,
                        so 8 bit is 0xFF and we have to shift our colors
                     */
                    PolyList[I].Attr |= EPolyAttr::RGB32;
                    PolyList[I].OriginalColor = MAP_XRGB32(
                        (Color444 & 0xF00) >> 4,
                        (Color444 & 0xF0),
                        (Color444 & 0xF)   << 4
                    );
                }
                else
                {
                    // Just something like orange with index as blue
                    PolyList[I].OriginalColor = MAP_XRGB32(255, 255, PolyDesc & EPLX::RGB8Mask);
                    VL_WARNING(hLogObject, "There're no 8-bit support");
                }

                if (PolyDesc & EPLX::TwoSidedFlag)
                {
                    PolyList[I].Attr |= EPolyAttr::TwoSided;
                }

                switch (PolyDesc & EPLX::ShadeModeMask)
                {
                case EPLX::ShadeModePureFlag:
                {
                    PolyList[I].Attr |= EPolyAttr::ShadeModeEmissive;
                } break;

                case EPLX::ShadeModeFlatFlag:
                {
                    PolyList[I].Attr |= EPolyAttr::ShadeModeFlat;
                } break;

                case EPLX::ShadeModeGouraudFlag:
                {
                    PolyList[I].Attr |= EPolyAttr::ShadeModeGouraud;
                } break;

                case EPLX::ShadeModePhongFlag:
                {
                    PolyList[I].Attr |= EPolyAttr::ShadeModePhong;
                } break;

                default: {} break;
                }

                // Final
                PolyList[I].State = EPolyState::Active;
            }
        }

        std::fclose(File);
    }

    ComputeRadius();
    ComputePolygonNormalsLength();
    ComputeVertexNormals();

    return true;
}

/* ==============================================
                    COB Loader
   ============================================== */

DEFINE_LOG_CHANNEL(hLogCOB, "COB Loader");

char* GetLineCOB(std::FILE* File, char* Buffer, i32 Size)
{
    for (;;)
    {
        if (!std::fgets(Buffer, Size, File))
        {
            return nullptr;
        }

        i32f I, Len = std::strlen(Buffer);
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
    i32f PatternLength = std::strlen(Pattern);

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

b32 VObject::LoadCOB(const char* Path, const VVector4& InPosition, const VVector4& Scale, const VVector4& Rot, u32 Flags)
{
    static constexpr i32f BufferSize = 4096;

    VMatrix44 MatLocal = VMatrix44::Identity;
    VMatrix44 MatWorld = VMatrix44::Identity;

    // Initialize object
    {
        Clean();
        Position = InPosition;
    }

    // Load from file
    {
        std::FILE* File;
        char Buffer[BufferSize];
        char* Line;

        VL_NOTE(hLogCOB, "Starting parse object:\n");

        // Open file
        if (!(File = std::fopen(Path, "r")))
        {
            VL_ERROR(hLogCOB, "Can't open %s file: %s\n", Path, strerror(errno));
            return false;
        }

        // Read name
        {
            Line = FindLineCOB("Name", File, Buffer, BufferSize);
            std::sscanf(Line, "Name %s", Name);
            VL_LOG("\tName: %s\n", Name);
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

            VL_LOG("\tMatLocal: ");
            MatLocal.Print();
            VL_LOG("\n");
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

            VL_LOG("\tMatWorld: ");
            MatWorld.Print();
            VL_LOG("\n");
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
                VVector4::MulMat44(LocalVtxList[I].Position, MatLocal, TempVector);
                VVector4::MulMat44(TempVector, MatWorld, LocalVtxList[I].Position);

                // Swap YZ
                if (Flags & ECOB::SwapYZ)
                {
                    f32 TempFloat;
                    SWAP(LocalVtxList[I].Y, LocalVtxList[I].Z, TempFloat);
                    LocalVtxList[I].Z = -LocalVtxList[I].Z;
                }

                // Scale
                LocalVtxList[I].X *= Scale.X;
                LocalVtxList[I].Y *= Scale.Y;
                LocalVtxList[I].Z *= Scale.Z;

                // Log
                VL_LOG("\tVertex [%d]: ", I);
                LocalVtxList[I].Position.Print();
                VL_LOG("\n");
            }
        }

        // Read texture coords
        i32f NumTextureVtx;
        {
            Line = FindLineCOB("Texture Vertices", File, Buffer, BufferSize);
            std::sscanf(Line, "Texture Vertices %d", &NumTextureVtx);
            VL_LOG("\tNum texture coords: %d\n", NumTextureVtx);

            for (i32f I = 0; I < NumTextureVtx; ++I)
            {
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "%f %f", &TextureCoordsList[I].X, &TextureCoordsList[I].Y);

                VL_LOG("\tTexture coord [%d]: ", I);
                TextureCoordsList[I].Print();
                VL_LOG("\n");
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

                VL_LOG("\tMaterial index of poly face [%d]: %d\n", I, MaterialIndexByPolyIndex[I]);

                // Get vertex and texture indices
                Line = GetLineCOB(File, Buffer, BufferSize);
                std::sscanf(Line, "<%d,%d> <%d,%d> <%d,%d>",
                    &PolyList[I].VtxIndices[2], &PolyList[I].TextureCoordsIndices[2],
                    &PolyList[I].VtxIndices[1], &PolyList[I].TextureCoordsIndices[1],
                    &PolyList[I].VtxIndices[0], &PolyList[I].TextureCoordsIndices[0]
                );

                VL_LOG("\tVertex and texture indices:\n");
                for (i32f J = 0; J < 3; ++J)
                {
                    VL_LOG("\t<%d, %d>\n", PolyList[I].VtxIndices[J], PolyList[I].TextureCoordsIndices[J]);
                }

                // Set default stuff
                PolyList[I].State = EPolyState::Active;
                PolyList[I].TextureCoordsList = TextureCoordsList;
            }

            VL_LOG("\tNum materials in object: %d\n", NumMaterialsInObject);
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

                    CurrentMaterial.Color = MAP_RGBA32(
                        (i32)(R * 255.0f + 0.5f),
                        (i32)(G * 255.0f + 0.5f),
                        (i32)(B * 255.0f + 0.5f),
                        255 // Opaque by default, may be overriden by transparency shader
                    );

                    // Material parameters
                    Line = FindLineCOB("alpha", File, Buffer, BufferSize);
                    std::sscanf(Line, "alpha %f ka %f ks %f exp %f",
                        &A, &CurrentMaterial.KAmbient, &CurrentMaterial.KSpecular, &CurrentMaterial.Power
                    );

                    /* NOTE(sean):
                        We try to find diffuse factor below, but this is default
                     */
                    CurrentMaterial.KDiffuse = 1.0f;

                    // Log color
                    VL_LOG("\tMaterial color [%d]: ", I);
                    VVector4 ColorVector = { R, G, B, A };
                    ColorVector.Print();
                    VL_LOG("\n");
                }

                // Check if we have texture
                {
                    Line = FindLineCOB("Shader class: color", File, Buffer, BufferSize);
                    Line = GetLineCOB(File, Buffer, BufferSize);

                    std::snprintf(Format, FormatSize, "Shader name: \"%%%d[a-z ]\"", ShaderNameSize - 1);
                    std::sscanf(Line, Format, ShaderName);
                    VL_LOG("\tShader name: %s\n", ShaderName);

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
                        CurrentMaterial.Texture.Load(TexturePath);
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeTexture;

                        // Texture in object
                        Texture = &CurrentMaterial.Texture;
                        Attr |= EObjectAttr::HasTexture;

                        VL_LOG("\tMaterial has texture, file path: %s\n", TexturePath);
                    }
                }

                // Read transparency shader
                {
                    Line = FindLineCOB("Shader class: transparency", File, Buffer, BufferSize);
                    Line = FindLineCOB("Shader name", File, Buffer, BufferSize);

                    std::snprintf(Format, FormatSize, "Shader name: \"%%%d[a-z ]\"", ShaderNameSize - 1);
                    std::sscanf(Line, Format, ShaderName);

                    VL_LOG("\tTransparency shader name: %s\n", ShaderName);
                    if (0 == std::strncmp(ShaderName, "filter", ShaderNameSize))
                    {
                        i32 AlphaRed, AlphaGreen, AlphaBlue;

                        Line = FindLineCOB("colour: color", File, Buffer, BufferSize);
                        std::sscanf(Buffer, "colour: color (%d, %d, %d)", &AlphaRed, &AlphaGreen, &AlphaBlue);

                        CurrentMaterial.Color.A = MAX(AlphaRed, MAX(AlphaGreen, AlphaBlue));
                        CurrentMaterial.Attr |= EMaterialAttr::Transparent;

                        VL_LOG("\tAlpha channel: %d\n", CurrentMaterial.Color.A);
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
                        /* NOTE(sean):
                            We have no phong support, so we use gouraud for phong too
                         */
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeGouraud;
                    }
                    else
                    {
                        CurrentMaterial.Attr |= EMaterialAttr::ShadeModeEmissive;
                    }

                    VL_LOG("\tShader name: %s\n", ShaderName);

                    // Try to find diffuse factor
                    i32 NumParams;

                    Line = FindLineCOB("Number of parameters:", File, Buffer, BufferSize);
                    std::sscanf(Line, "Number of parameters: %d", &NumParams);

                    const char Pattern[] = "diffuse";
                    VSizeType PatternLength = std::strlen(Pattern);

                    for (i32f ParamIndex = 0; ParamIndex < NumParams; ++ParamIndex)
                    {
                        Line = GetLineCOB(File, Buffer, BufferSize);
                        if (0 == std::strncmp(Line, Pattern, PatternLength))
                        {
                            std::sscanf(Line, "diffuse factor: float %f", &CurrentMaterial.KDiffuse);
                            VL_LOG("\tDiffuse factor found: %f\n", CurrentMaterial.KDiffuse);
                            break;
                        }
                    }
                }

                // Precompute reflectivities for engine
                for (i32f RGBIndex = 0; RGBIndex < 3; ++RGBIndex)
                {
                    CurrentMaterial.RAmbient.C[RGBIndex]  = (u8)(CurrentMaterial.KAmbient * CurrentMaterial.Color.C[RGBIndex]);
                    CurrentMaterial.RDiffuse.C[RGBIndex]  = (u8)(CurrentMaterial.KDiffuse * CurrentMaterial.Color.C[RGBIndex]);
                    CurrentMaterial.RSpecular.C[RGBIndex] = (u8)(CurrentMaterial.RSpecular * CurrentMaterial.Color.C[RGBIndex]);

                    // Log precomputed colors and factors
                    VL_LOG("\tRa [%d]: %d\n", RGBIndex, CurrentMaterial.RAmbient.C[RGBIndex]);
                    VL_LOG("\tRd [%d]: %d\n", RGBIndex, CurrentMaterial.RDiffuse.C[RGBIndex]);
                    VL_LOG("\tRs [%d]: %d\n", RGBIndex, CurrentMaterial.RSpecular.C[RGBIndex]);
                }

                // Log factors
                VL_LOG("\tKa %f Kd %f Ks %f Exp %f\n",
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
                VMaterial& PolyMaterial = Renderer.Materials[Renderer.NumMaterials + MaterialIndexByPolyIndex[I]];

                // Set color
                if (PolyMaterial.Attr & EMaterialAttr::ShadeModeTexture)
                {
                    Poly.OriginalColor = MAP_ARGB32(PolyMaterial.Color.A, 255, 255, 255);
                }
                else
                {
                    Poly.OriginalColor = MAP_ARGB32(
                        PolyMaterial.Color.A,
                        PolyMaterial.Color.R,
                        PolyMaterial.Color.G,
                        PolyMaterial.Color.B
                    );
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
                    Poly.Texture = &PolyMaterial.Texture;

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
            if (Texture)
            {
                for (i32f I = 0; I < NumTextureVtx; ++I)
                {
                    i32 TextureScaleSize = Texture->GetWidth() - 1;

                    TextureCoordsList[I].X *= TextureScaleSize;
                    TextureCoordsList[I].Y *= TextureScaleSize;

                    if (Flags & ECOB::InvertU)
                    {
                        TextureCoordsList[I].X = TextureScaleSize - TextureCoordsList[I].X;
                    }
                    if (Flags & ECOB::InvertV)
                    {
                        TextureCoordsList[I].Y = TextureScaleSize - TextureCoordsList[I].Y;
                    }
                    if (Flags & ECOB::SwapUV)
                    {
                        f32 TempFloat;
                        SWAP(TextureCoordsList[I].X, TextureCoordsList[I].Y, TempFloat);
                    }
                }
            }
        }

        // Close file
        std::fclose(File);
    }

    // Compute stuff
    ComputeRadius();
    ComputePolygonNormalsLength();
    ComputeVertexNormals();

    // Log our success
    VL_NOTE(hLogCOB, "Ended parse object\n");

    return true;
}
