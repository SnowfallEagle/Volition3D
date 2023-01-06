#include <stdio.h>
#include <errno.h>
#include "Core/Types.h"
#include "Core/Memory.h"
#include "Math/Vector.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"

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

char* GetLinePLG(FILE* File, char* Buffer, i32 Size)
{
	for (;;)
	{
		if (!fgets(Buffer, Size, File))
		{
			return nullptr;
		}

		i32f I, Len = strlen(Buffer);
		for (
			I = 0;
			I < Len && (Buffer[I] == ' ' || Buffer[I] == '\t' || Buffer[I] == '\r' || Buffer[I] == '\n');
			++I
		)
			{}

		if (I < Len && Buffer[I] != '#')
		{
			return &Buffer[I];
		}
	}
}

b32 VObject::LoadPLG(
	const char* Path,
	const VVector4& Pos,
	const VVector4& Scale,
	const VVector4& Rot
)
{
	static constexpr i32f BufferSize = 256;

	// Clean object
	{
		Memory.MemSetByte(this, 0, sizeof(*this));

		State = EObjectState::Active | EObjectState::Visible;

		Position = Pos;
		UX = { 1.0f, 0.0f, 0.0f };
		UY = { 0.0f, 1.0f, 0.0f };
		UZ = { 0.0f, 0.0f, 1.0f };

		NumFrames = 1;
		CurrentFrame = 0;
	}

	// Load from file
	{
		FILE* File;
		char Buffer[BufferSize];

		// Open
		if (!(File = fopen(Path, "rb")))
		{
			VL_ERROR(hLogObject, "Can't open %s file: %s\n", Path, strerror(errno));
			return false;
		}

		// Read header and allocate object
		{
			if (!GetLinePLG(File, Buffer, BufferSize))
			{
				VL_ERROR(hLogObject, "Can't read PLG header\n");
				fclose(File);
				return false;
			}
			if (sscanf(Buffer, "%s %d %d", Name, &NumVtx, &NumPoly) <= 0)
			{
				VL_ERROR(hLogObject, "Can't parse PLG header\n");
				fclose(File);
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
					fclose(File);
					return false;
				}

				// Parse
				if (sscanf(Buffer, "%f %f %f",
					&LocalVtxList[I].X, &LocalVtxList[I].Y, &LocalVtxList[I].Z) <= 0)
				{
					VL_ERROR(hLogObject, "Can't parse vertex\n");
					fclose(File);
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
					fclose(File);
					return false;
				}

				// Parse
				i32 NumPolyVtx;
				if (sscanf(Buffer, "%255s %d %d %d %d",
					StrPolyDesc, &NumPolyVtx,
					&PolyList[I].VtxIndices[0], &PolyList[I].VtxIndices[1], &PolyList[I].VtxIndices[2]) <= 0)
				{
					VL_ERROR(hLogObject, "Can't parse polygon\n");
					fclose(File);
					return false;
				}

				// Get polygon description
				if (StrPolyDesc[1] == 'x' || StrPolyDesc[1] == 'X')
				{
					if (sscanf(Buffer, "%x", &PolyDesc) <= 0)
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
						fclose(File);
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

		fclose(File);
	}

	ComputeRadius();
	ComputePolygonNormalsLength();
	ComputeVertexNormals();

	return true;
}

