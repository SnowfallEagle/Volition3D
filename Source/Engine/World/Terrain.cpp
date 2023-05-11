#include "Engine/Core/Time.h"
#include "Engine/World/Terrain.h"
#include "Engine/Graphics/Rendering/Renderer.h"

namespace Volition
{

void VTerrain::GenerateTerrain(const char* HeightMap, const char* Texture, f32 Size, f32 Height)
{
    Mesh->Attr |= EMeshAttr::TerrainMesh;
    Mesh->GenerateTerrain(HeightMap, Texture, Size, Height);
    Renderer.SetTerrain(*Mesh);
}

}
