#include <errno.h>
#include "Graphics/Graphics.h"
#include "Graphics/Renderer.h"

// Object ////////////////////////////////////////////
b32 VObject4DV1::LoadPLG(
    const char* Path,
    const VVector4D& Pos,
    const VVector4D& Scale,
    const VVector4D& Rot
)
{
    static constexpr i32f BufferSize = 256;

    // Clean object
    memset(this, 0, sizeof(*this));
    State = EObjectStateV1::Active | EObjectStateV1::Visible;
    WorldPos = Pos;
    UX = { 1.0f, 0.0f, 0.0f, 0.0f };
    UY = { 0.0f, 1.0f, 0.0f, 0.0f };
    UZ = { 0.0f, 0.0f, 1.0f, 0.0f };

    // Load from file
    FILE* File;
    char Buffer[BufferSize];

    // Open
    if (!(File = fopen(Path, "rb")))
    {
        VL_ERROR(hObjectV1Log, "Can't open %s file: %s\n", Path, strerror(errno));
        return false;
    }

    // Read header
    if (!GetLinePLG(File, Buffer, BufferSize))
    {
        VL_ERROR(hObjectV1Log, "Can't read PLG header\n");
        fclose(File);
        return false;
    }
    if (sscanf(Buffer, "%s %d %d", Name, &NumVtx, &NumPoly) <= 0)
    {
        VL_ERROR(hObjectV1Log, "Can't parse PLG header\n");
        fclose(File);
        return false;
    }

    VL_NOTE(hObjectV1Log, "Loading %s %d %d\n", Name, NumVtx, NumPoly);
    VL_LOG("Vertices:\n");

    // Read vertices
    for (i32f I = 0; I < NumVtx; ++I)
    {
        if (!GetLinePLG(File, Buffer, BufferSize))
        {
            VL_ERROR(hObjectV1Log, "Can't parse PLG vertices\n");
            fclose(File);
            return false;
        }

        // Parse
        if (sscanf(Buffer, "%f %f %f",
                   &LocalVtxList[I].X, &LocalVtxList[I].Y, &LocalVtxList[I].Z) <= 0)
        {
            VL_ERROR(hObjectV1Log, "Can't parse vertex\n");
            fclose(File);
            return false;
        }
        LocalVtxList[I].Z = 1.0f;

        // Scale
        LocalVtxList[I].X *= Scale.X;
        LocalVtxList[I].Y *= Scale.Y;
        LocalVtxList[I].Z *= Scale.Z;

        // Log
        VL_LOG("\t<%f, %f, %f>\n",
            LocalVtxList[I].X, LocalVtxList[I].Y, LocalVtxList[I].Z
        );
    }

    ComputeRadius();

    // Read polygons
    i32f PolyDesc;
    char StrPolyDesc[BufferSize];

    VL_LOG("Polygons:\n");

    for (i32f I = 0; I < NumPoly; ++I)
    {
        if (!GetLinePLG(File, Buffer, BufferSize))
        {
            VL_ERROR(hObjectV1Log, "Can't parse PLG polygons\n");
            fclose(File);
            return false;
        }

        // Parse
        i32 NumPolyVtx;
        if (sscanf(Buffer, "%255s %d %d %d %d",
                   StrPolyDesc, &NumPolyVtx,
                   &PolyList[I].Vtx[0], &PolyList[I].Vtx[1], &PolyList[I].Vtx[2]) <= 0)
        {
            VL_ERROR(hObjectV1Log, "Can't parse polygon\n");
            fclose(File);
            return false;
        }

        // Get polygon description
        if (StrPolyDesc[1] == 'x' || StrPolyDesc[1] == 'X')
        {
            if (sscanf(Buffer, "%x", &PolyDesc) <= 0)
            {
                VL_ERROR(hObjectV1Log, "Can't parse polygon description\n");
                fclose(File);
                return false;
            }
        }
        else
        {
            if (sscanf(Buffer, "%d", &PolyDesc) <= 0)
            {
                VL_ERROR(hObjectV1Log, "Can't parse polygon description\n");
                fclose(File);
                return false;
            }
        }

        // Set vertex list
        PolyList[I].VtxList = LocalVtxList;

        // Log
        VL_LOG("\t<0x%x, %d, <%d %d %d>>\n",
            PolyDesc, NumVtx,
            PolyList[I].Vtx[0], PolyList[I].Vtx[1], PolyList[I].Vtx[2]
        );

        // Set attributes and color
        if (PolyDesc & EPLX::RGBFlag)
        {
            u32 Color444 = PolyDesc & EPLX::RGB16Mask;

            PolyList[I].Attr |= EPolyAttrV1::RGB32;
            PolyList[I].Color = _RGB32((Color444 & 0xF00) * 2, (Color444 & 0xF0) * 2, (Color444 & 0xF) * 2);
        }
        else
        {
            // Just something like orange with index as blue
            PolyList[I].Color = _RGB32(255, 255, PolyDesc & EPLX::RGB8Mask);
            VL_WARNING(hObjectV1Log, "There're no 8-bit support");
        }

        if (PolyDesc & EPLX::TwoSidedFlag)
            PolyList[I].Attr |= EPolyAttrV1::TwoSided;

        switch (PolyDesc & EPLX::ShadeModeMask)
        {
        case EPLX::ShadeModePhongFlag:   PolyList[I].Attr |= EPolyAttrV1::ShadeModePhong; break;
        case EPLX::ShadeModeGouraudFlag: PolyList[I].Attr |= EPolyAttrV1::ShadeModeGouraud; break;
        case EPLX::ShadeModeFlatFlag:    PolyList[I].Attr |= EPolyAttrV1::ShadeModeFlat; break;
        case EPLX::ShadeModePureFlag:    PolyList[I].Attr |= EPolyAttrV1::ShadeModePure; break;
        default: break;
        }

        // Final
        PolyList[I].State = EPolyStateV1::Active;
    }

    fclose(File);
    return true;
}

