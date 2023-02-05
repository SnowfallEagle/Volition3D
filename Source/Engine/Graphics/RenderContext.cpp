#include "Engine/Core/World.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/RenderList.h"
#include "Engine/Graphics/RenderContext.h"

void VRenderContext::Init()
{
    RenderList = new VRenderList();
    ZBuffer.Create(RenderSpec.TargetSize.X, RenderSpec.TargetSize.Y);
}

void VRenderContext::Destroy()
{
    ZBuffer.Destroy();
    delete RenderList;
}

void VRenderContext::PrepareToRender()
{
    ZBuffer.Clear();
    RenderList->Reset();
}

void VRenderContext::RenderWorld(u32* Buffer, i32 Pitch)
{
    // Set up camera
    VCamera& Camera = *World.Camera;
    Camera.BuildWorldToCameraEulerMat44();

    // Proccess and insert meshes
    {
        for (auto Entity : World.Entities)
        {
            if (Entity && Entity->Mesh)
            {
                VMesh* Mesh = Entity->Mesh;

                Mesh->Reset();
                Mesh->TransformModelToWorld();
                Mesh->Cull(Camera);

                RenderList->InsertMesh(*Mesh, false);
            }
        }
    }

    // Proccess render list
    {
        if (RenderSpec.bBackfaceRemoval)
        {
            RenderList->RemoveBackFaces(Camera);
        }
        RenderList->TransformWorldToCamera(Camera);
        RenderList->Clip(Camera);
        Renderer.TransformLights(Camera);
        RenderList->Light(Camera, Renderer.Lights, Renderer.MaxLights);
        RenderList->SortPolygons(ESortPolygonsMethod::Average);
        RenderList->TransformCameraToScreen(Camera);
    }

    // Render stuff
    InterpolationContext.Buffer = Buffer;
    InterpolationContext.BufferPitch = Pitch;

    RenderSpec.bRenderSolid ? RenderSolid() : RenderWire();
}

void VRenderContext::RenderSolid()
{
    for (i32f I = 0; I < RenderList->NumPoly; ++I)
    {
        VPolyFace* Poly = RenderList->PolyPtrList[I];
        if (!Poly ||
            ~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::BackFace ||
            Poly->State & EPolyState::Clipped)
        {
            continue;
        }

        InterpolationContext.Poly = Poly;

        Renderer.DrawTriangle(InterpolationContext);
    }
}

void VRenderContext::RenderWire()
{
    for (i32f I = 0; I < RenderList->NumPoly; ++I)
    {
        VPolyFace* Poly = RenderList->PolyPtrList[I];
        if (!Poly ||
            ~Poly->State & EPolyState::Active ||
            Poly->State & EPolyState::BackFace ||
            Poly->State & EPolyState::Clipped)
        {
            continue;
        }

        Renderer.DrawClippedLine(
            InterpolationContext.Buffer, InterpolationContext.BufferPitch,
            (i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
            (i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
            Poly->LitColor[0]
        );
        Renderer.DrawClippedLine(
            InterpolationContext.Buffer, InterpolationContext.BufferPitch,
            (i32)Poly->TransVtx[1].X, (i32)Poly->TransVtx[1].Y,
            (i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
            Poly->LitColor[1]
        );
        Renderer.DrawClippedLine(
            InterpolationContext.Buffer, InterpolationContext.BufferPitch,
            (i32)Poly->TransVtx[2].X, (i32)Poly->TransVtx[2].Y,
            (i32)Poly->TransVtx[0].X, (i32)Poly->TransVtx[0].Y,
            Poly->LitColor[2]
        );
    }
}