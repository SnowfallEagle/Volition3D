/* TODO:
	- Lighting
 */

#pragma once

#include <stdlib.h> // qsort()
#include "Math/Minimal.h"
#include "Graphics/Polygon.h"
#include "Graphics/Camera.h"
#include "Graphics/TransformType.h"
#include "Graphics/Renderer.h"
#include "Graphics/Object.h"

enum class ESortPolygonsMethod
{
	Average = 0,
	Near,
	Far
};

class VRenderList
{
public:
	static constexpr i32f MaxPoly = 32768;

public:
	i32 NumPoly;
	VPolyFace* PolyPtrList[MaxPoly];
	VPolyFace PolyList[MaxPoly];

public:
	b32 InsertPoly(const VPoly& Poly, const VVertex* VtxList)
	{
		if (NumPoly >= MaxPoly)
		{
			return false;
		}

		PolyPtrList[NumPoly] = &PolyList[NumPoly];
		PolyList[NumPoly].State = Poly.State;
		PolyList[NumPoly].Attr = Poly.Attr;
		PolyList[NumPoly].OriginalColor = Poly.OriginalColor;

		for (i32f I = 0; I < 3; ++I)
		{
			PolyList[NumPoly].TransVtx[I] = PolyList[NumPoly].LocalVtx[I] = VtxList[Poly.VtxIndices[I]];
			PolyList[NumPoly].LitColor[I] = Poly.LitColor[I];
		}

		++NumPoly;

		return true;
	}

	b32 InsertPolyFace(const VPolyFace& PolyFace)
	{
		if (NumPoly >= MaxPoly)
		{
			return false;
		}

		PolyPtrList[NumPoly] = &PolyList[NumPoly];
		Memory.MemCopy(&PolyList[NumPoly], &PolyFace, sizeof(VPolyFace));

		++NumPoly;

		return true;
	}

	void InsertObject(VObject& Object, b32 bInsertLocal)
	{
		if (~Object.State & EObjectState::Active  ||
			~Object.State & EObjectState::Visible ||
			Object.State & EObjectState::Culled)
		{
			return;
		}

		for (i32f I = 0; I < Object.NumPoly; ++I)
		{
			VPoly& Poly = Object.PolyList[I];

			if (~Poly.State & EPolyState::Active ||
				Poly.State & EPolyState::Clipped ||
				Poly.State & EPolyState::BackFace)
			{
				continue;
			}

			if (!InsertPoly(Poly, bInsertLocal ? Object.LocalVtxList : Object.TransVtxList))
			{
				return;
			}
		}
	}

	void Reset()
	{
		NumPoly = 0;
	}

	void Transform(const VMatrix44& M, ETransformType Type)
	{
		VVector4 Res;

		switch (Type)
		{
		case ETransformType::LocalOnly:
		{
			for (i32f I = 0; I < NumPoly; ++I)
			{
				VPolyFace* Poly = PolyPtrList[I];
				if (!Poly ||
					~Poly->State & EPolyState::Active ||
					Poly->State & EPolyState::Clipped ||
					Poly->State & EPolyState::BackFace)
				{
					continue;
				}

				for (i32f V = 0; V < 3; ++V)
				{
					VVector4::MulMat44(Poly->LocalVtx[V].Position, M, Res);
					Poly->LocalVtx[V].Position = Res;
				}
			}
		} break;

		case ETransformType::TransOnly:
		{
			for (i32f I = 0; I < NumPoly; ++I)
			{
				VPolyFace* Poly = PolyPtrList[I];
				if (!Poly ||
					~Poly->State & EPolyState::Active ||
					Poly->State & EPolyState::Clipped ||
					Poly->State & EPolyState::BackFace)
				{
					continue;
				}

				for (i32f V = 0; V < 3; ++V)
				{
					VVector4::MulMat44(Poly->TransVtx[V].Position, M, Res);
					Poly->TransVtx[V].Position = Res;
				}
			}
		} break;

		case ETransformType::LocalToTrans:
		{
			for (i32f I = 0; I < NumPoly; ++I)
			{
				VPolyFace* Poly = PolyPtrList[I];
				if (!Poly ||
					~Poly->State & EPolyState::Active ||
					Poly->State & EPolyState::Clipped ||
					Poly->State & EPolyState::BackFace)
				{
					continue;
				}

				for (i32f V = 0; V < 3; ++V)
				{
					VVector4::MulMat44(Poly->LocalVtx[V].Position, M, Res);
					Poly->TransVtx[V].Position = Res;
				}
			}
		} break;
		}
	}

	// LocalToTrans or TransOnly
	void TransformModelToWorld(const VPoint4& WorldPos, ETransformType Type = ETransformType::LocalToTrans)
	{
		if (Type == ETransformType::LocalToTrans)
		{
			for (i32f I = 0; I < NumPoly; ++I)
			{
				VPolyFace* Poly = PolyPtrList[I];
				if (!Poly ||
					~Poly->State & EPolyState::Active ||
					Poly->State & EPolyState::Clipped ||
					Poly->State & EPolyState::BackFace)
				{
					continue;
				}

				for (i32f V = 0; V < 3; ++V)
				{
					Poly->TransVtx[V].Position = Poly->LocalVtx[V].Position + WorldPos;
				}
			}
		}
		else // TransOnly
		{
			for (i32f I = 0; I < NumPoly; ++I)
			{
				VPolyFace* Poly = PolyPtrList[I];
				if (!Poly ||
					~Poly->State & EPolyState::Active ||
					Poly->State & EPolyState::Clipped ||
					Poly->State & EPolyState::BackFace)
				{
					continue;
				}

				for (i32f V = 0; V < 3; ++V)
				{
					Poly->TransVtx[V].Position += WorldPos;
				}
			}
		}
	}

	void RemoveBackFaces(const VCamera& Cam)
	{
		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];

			if (~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::Clipped ||
				Poly->Attr & EPolyAttr::TwoSided ||
				Poly->State & EPolyState::BackFace)
			{
				continue;
			}

			VVector4 U, V, N;
			U = Poly->TransVtx[1].Position - Poly->TransVtx[0].Position;
			V = Poly->TransVtx[2].Position - Poly->TransVtx[0].Position;

			VVector4::Cross(U, V, N);
			VVector4 View = Cam.Pos - Poly->TransVtx[0].Position;

			// If > 0 then N watch in the same direction as View vector and visible
			if (VVector4::Dot(View, N) <= 0.0f)
			{
				Poly->State |= EPolyState::BackFace;
			}
		}
	}

    void Light(const VCamera& Cam, const VLight* Lights, i32 NumLights)
    {
        for (i32f PolyIndex = 0; PolyIndex < NumPoly; ++PolyIndex)
        {
            // Check if we need to draw this poly
            VPolyFace* Poly = PolyPtrList[PolyIndex];

            if (~Poly->State & EPolyState::Active  ||
                Poly->State & EPolyState::Clipped  ||
                Poly->State & EPolyState::BackFace ||
                Poly->State & EPolyState::Lit)
            {
                continue;
            }

            // Set lit flag
            Poly->State |= EPolyState::Lit;

            if (Poly->Attr & EPolyAttr::ShadeModeFlat)
            {
                u32 RSum = 0;
                u32 GSum = 0;
                u32 BSum = 0;

                for (i32f LightIndex = 0; LightIndex < NumLights; ++LightIndex)
                {
                    if (~Lights[LightIndex].State & ELightState::Active)
                    {
                        continue;
                    }

                    if (Lights[LightIndex].Attr & ELightAttr::Ambient)
                    {
                        // NOTE(sean): Maybe 255? or even >> 8
                        RSum += (Poly->OriginalColor.R * Lights[LightIndex].CAmbient.R) / 256;
                        GSum += (Poly->OriginalColor.G * Lights[LightIndex].CAmbient.G) / 256;
                        BSum += (Poly->OriginalColor.B * Lights[LightIndex].CAmbient.B) / 256;
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::Infinite)
                    {
                        VVector4 SurfaceNormal = VVector4::GetCross(
                            Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                            Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
                        );

                        f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            i32 Intensity = (i32)( 128.0f * (Math.Abs(Dot) / SurfaceNormal.GetLengthFast()) );
                            RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::Point)
                    {
                        VVector4 SurfaceNormal = VVector4::GetCross(
                            Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                            Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
                        );
                        VVector4 Direction = Poly->TransVtx[0].Position - Lights[LightIndex].Pos;

                        f32 Dot = VVector4::Dot(SurfaceNormal, Direction);
                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            f32 Distance = Direction.GetLengthFast();
                            f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / (SurfaceNormal.GetLengthFast() * Distance * Atten)
                            );

                            RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::SimpleSpotlight)
                    {
                        VVector4 SurfaceNormal = VVector4::GetCross(
                            Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                            Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
                        );
                        f32 Dot = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);

                        if (Dot < 0)
                        {
                            // 128 used for fixed point to don't lose accuracy with integers
                            f32 Distance = (Poly->TransVtx[0].Position - Lights[LightIndex].Pos).GetLengthFast();
                            f32 Atten =
                                Lights[LightIndex].KConst +
                                Lights[LightIndex].KLinear * Distance +
                                Lights[LightIndex].KQuad * Distance * Distance;
                            i32 Intensity = (i32)(
                                (128.0f * Math.Abs(Dot)) / (SurfaceNormal.GetLengthFast() * Atten)
                            );

                            RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                            GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                            BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                        }
                    }
                    else if (Lights[LightIndex].Attr & ELightAttr::ComplexSpotlight)
                    {
                        VVector4 SurfaceNormal = VVector4::GetCross(
                            Poly->TransVtx[1].Position - Poly->TransVtx[0].Position,
                            Poly->TransVtx[2].Position - Poly->TransVtx[0].Position
                        );
                        f32 DotNormalDirection = VVector4::Dot(SurfaceNormal, Lights[LightIndex].Dir);

                        if (DotNormalDirection < 0)
                        {
                            VVector4 DistanceVector = Poly->TransVtx[0].Position - Lights[LightIndex].Pos;
                            f32 Distance = DistanceVector.GetLengthFast();
                            f32 DotDistanceDirection = VVector4::Dot(DistanceVector, Lights[LightIndex].Dir) / Distance;

                            if (DotDistanceDirection > 0)
                            {
                                f32 DotDistanceDirectionExp = DotDistanceDirection;
                                // For optimization use integer power
                                i32f IntegerExp = (i32f)Lights[LightIndex].Power;
                                for (i32f I = 1; I < IntegerExp; ++I)
                                {
                                    DotDistanceDirectionExp *= DotDistanceDirection;
                                }

                                // 128 used for fixed point to don't lose accuracy with integers
                                f32 Atten =
                                    Lights[LightIndex].KConst +
                                    Lights[LightIndex].KLinear * Distance +
                                    Lights[LightIndex].KQuad * Distance * Distance;
                                i32 Intensity = (i32)(
                                    (128.0f * Math.Abs(DotNormalDirection) * DotDistanceDirectionExp) /
                                    (SurfaceNormal.GetLengthFast() * Atten)
                                );

                                RSum += (Poly->OriginalColor.R * Lights[LightIndex].CDiffuse.R * Intensity) / (256 * 128);
                                GSum += (Poly->OriginalColor.G * Lights[LightIndex].CDiffuse.G * Intensity) / (256 * 128);
                                BSum += (Poly->OriginalColor.B * Lights[LightIndex].CDiffuse.B * Intensity) / (256 * 128);
                            }
                        }
                    }
                }

                if (RSum > 255) RSum = 255;
                if (GSum > 255) GSum = 255;
                if (BSum > 255) BSum = 255;

                Poly->LitColor[0] = MAP_XRGB32(RSum, GSum, BSum);
            }
            else if (Poly->Attr & EPolyAttr::ShadeModeGouraud)
            {
                VL_LOG("Light gouraud: PolyIndex [%d]\n", PolyIndex);
            }
        }
    }

	void TransformWorldToCamera(const VCamera& Camera)
	{
		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];
			if (!Poly ||
				~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::Clipped ||
				Poly->State & EPolyState::BackFace)
			{
				continue;
			}

			for (i32f V = 0; V < 3; ++V)
			{
				VVector4 Res;
				VVector4::MulMat44(Poly->TransVtx[V].Position, Camera.MatCamera, Res);
				Poly->TransVtx[V].Position = Res;
			}
		}
	}

	static i32 SortPolygonsCompareAverage(const void* Arg1, const void* Arg2)
	{
		const VPolyFace* Poly1 = *(const VPolyFace**)Arg1;
		const VPolyFace* Poly2 = *(const VPolyFace**)Arg2;

		f32 Z1 = 0.33333f * (Poly1->TransVtx[0].Z + Poly1->TransVtx[1].Z + Poly1->TransVtx[2].Z);
		f32 Z2 = 0.33333f * (Poly2->TransVtx[0].Z + Poly2->TransVtx[1].Z + Poly2->TransVtx[2].Z);

		if (Z1 < Z2)
		{
			return 1;
		}
		else if (Z1 > Z2)
		{
			return -1;
		}
		else
		{
			return 0;
		}

		return 1;
	}

	static i32 SortPolygonsCompareNear(const void* Arg1, const void* Arg2)
	{
		const VPolyFace* Poly1 = *(const VPolyFace**)Arg1;
		const VPolyFace* Poly2 = *(const VPolyFace**)Arg2;

		f32 ZMin1 = MIN(MIN(Poly1->TransVtx[0].Z, Poly1->TransVtx[1].Z), Poly1->TransVtx[2].Z);
		f32 ZMin2 = MIN(MIN(Poly2->TransVtx[0].Z, Poly2->TransVtx[1].Z), Poly2->TransVtx[2].Z);

		if (ZMin1 < ZMin2)
		{
			return 1;
		}
		else if (ZMin1 > ZMin2)
		{
			return -1;
		}
		else
		{
			return 0;
		}

		return 1;
	}

	static i32 SortPolygonsCompareFar(const void* Arg1, const void* Arg2)
	{
		const VPolyFace* Poly1 = *(const VPolyFace**)Arg1;
		const VPolyFace* Poly2 = *(const VPolyFace**)Arg2;

		f32 ZMax1 = MAX(MAX(Poly1->TransVtx[0].Z, Poly1->TransVtx[1].Z), Poly1->TransVtx[2].Z);
		f32 ZMax2 = MAX(MAX(Poly2->TransVtx[0].Z, Poly2->TransVtx[1].Z), Poly2->TransVtx[2].Z);

		if (ZMax1 < ZMax2)
		{
			return 1;
		}
		else if (ZMax1 > ZMax2)
		{
			return -1;
		}
		else
		{
			return 0;
		}

		return 1;
	}

	void SortPolygons(ESortPolygonsMethod Method = ESortPolygonsMethod::Average)
	{
		switch (Method)
		{
		case ESortPolygonsMethod::Average: qsort(PolyPtrList, NumPoly, sizeof(*PolyPtrList), SortPolygonsCompareAverage); break;
		case ESortPolygonsMethod::Near:    qsort(PolyPtrList, NumPoly, sizeof(*PolyPtrList), SortPolygonsCompareNear); break;
		case ESortPolygonsMethod::Far:     qsort(PolyPtrList, NumPoly, sizeof(*PolyPtrList), SortPolygonsCompareFar); break;
		}
	}

	void TransformCameraToPerspective(const VCamera& Cam)
	{
		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];
			if (!Poly ||
				~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::Clipped ||
				Poly->State & EPolyState::BackFace)
			{
				continue;
			}

			for (i32f V = 0; V < 3; ++V)
			{
				Poly->TransVtx[V].X = Cam.ViewDist * Poly->TransVtx[V].X / Poly->TransVtx[V].Z;
				Poly->TransVtx[V].Y = Cam.ViewDist * Poly->TransVtx[V].Y * Cam.AspectRatio / Poly->TransVtx[V].Z;
			}
		}
	}

	void ConvertFromHomogeneous()
	{
		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];
			if (!Poly ||
				~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::Clipped ||
				Poly->State & EPolyState::BackFace)
			{
				continue;
			}

			for (i32f V = 0; V < 3; ++V)
			{
				Poly->TransVtx[V].Position.DivByW();
			}
		}
	}

	void TransformPerspectiveToScreen(const VCamera& Cam)
	{
		f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
		f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];
			if (!Poly ||
				~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::Clipped ||
				Poly->State & EPolyState::BackFace)
			{
				continue;
			}

			for (i32f V = 0; V < 3; ++V)
			{
				Poly->TransVtx[V].X = Alpha + Alpha * Poly->TransVtx[V].X;
				Poly->TransVtx[V].Y = Beta - Beta * Poly->TransVtx[V].Y;
			}
		}
	}

	void TransformCameraToScreen(const VCamera& Cam)
	{
		f32 Alpha = Cam.ViewPortSize.X * 0.5f - 0.5f;
		f32 Beta = Cam.ViewPortSize.Y * 0.5f - 0.5f;

		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];
			if (!Poly ||
				~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::Clipped ||
				Poly->State & EPolyState::BackFace)
			{
				continue;
			}

			for (i32f V = 0; V < 3; ++V)
			{
				f32 ViewDistDivZ = Cam.ViewDist / Poly->TransVtx[V].Z;

				Poly->TransVtx[V].X = Poly->TransVtx[V].X * ViewDistDivZ;
				Poly->TransVtx[V].Y = Poly->TransVtx[V].Y * Cam.AspectRatio * ViewDistDivZ;

				Poly->TransVtx[V].X = Alpha + Alpha * Poly->TransVtx[V].X;
				Poly->TransVtx[V].Y = Beta - Beta * Poly->TransVtx[V].Y;
			}
		}
	}

	void RenderWire(u32* Buffer, i32 Pitch)
	{
		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];
			if (!Poly ||
				~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::BackFace ||
				Poly->State & EPolyState::Clipped)
			{
				continue;
			}

			Renderer.DrawClippedLine(
				Buffer, Pitch,
				(i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
				(i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
				Poly->LitColor[0]
			);
			Renderer.DrawClippedLine(
				Buffer, Pitch,
				(i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
				(i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
				Poly->LitColor[0]
			);
			Renderer.DrawClippedLine(
				Buffer, Pitch,
				(i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
				(i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
				Poly->LitColor[0]
			);
		}
	}

	void RenderSolid(u32* Buffer, i32 Pitch)
	{
		for (i32f I = 0; I < NumPoly; ++I)
		{
			VPolyFace* Poly = PolyPtrList[I];
			if (!Poly ||
				~Poly->State & EPolyState::Active ||
				Poly->State & EPolyState::BackFace ||
				Poly->State & EPolyState::Clipped)
			{
				continue;
			}

            if (Poly->Attr & EPolyAttr::ShadeModeGouraud)
            {
                Renderer.DrawGouraudTriangle(Buffer, Pitch, *Poly);
            }
            else
            {
                Renderer.DrawTriangle(
                    Buffer, Pitch,
                    Poly->TransVtx[0].X, Poly->TransVtx[0].Y,
                    Poly->TransVtx[1].X, Poly->TransVtx[1].Y,
                    Poly->TransVtx[2].X, Poly->TransVtx[2].Y,
                    Poly->LitColor[0]
                );
            }
		}
	}
};
