#pragma once
#include "heightsampler.h"
#include "terrainmeshbuilder.h"

class Terrain
{
public:
    Terrain(int width, int height, float noiseScale, float heightScale);

    const TerrainMesh &getMesh() const { return mesh; }

private:
    HeightSampler sampler;
    TerrainMeshBuilder builder;
    TerrainMesh mesh;
};