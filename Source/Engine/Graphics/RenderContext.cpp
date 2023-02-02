#include "Engine/Core/World.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/RenderList.h"
#include "Engine/Graphics/RenderContext.h"

void VRenderContext::Init(i32 ScreenWidth, i32 ScreenHeight)
{
    RenderList = new VRenderList();
    ZBuffer.Create(ScreenWidth, ScreenHeight);
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

    // Proccess and insert objects
    {
        for (auto Object : World.Objects)
        {
            Object->Reset();
            Object->TransformModelToWorld();
            Object->Cull(Camera);

            RenderList->InsertObject(*Object, false);
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
    RenderList->RenderSolid(Buffer, Pitch);
}