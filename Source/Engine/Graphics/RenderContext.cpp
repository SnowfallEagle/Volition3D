#include "Engine/Core/World.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/RenderList.h"
#include "Engine/Graphics/RenderContext.h"

void VRenderContext::Init(const VRenderSpecification& RenderSpec)
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
        RenderList->RemoveBackFaces(Camera);
        RenderList->TransformWorldToCamera(Camera);
        RenderList->Clip(Camera);
        Renderer.TransformLights(Camera);
        RenderList->Light(Camera, Renderer.Lights, Renderer.MaxLights);
        RenderList->SortPolygons(ESortPolygonsMethod::Average);
        RenderList->TransformCameraToScreen(Camera);
    }

    // Render stuff
    // TODO(sean): RenderSolid() that set up InterpolationContext
    RenderList->RenderSolid(Buffer, Pitch);
}