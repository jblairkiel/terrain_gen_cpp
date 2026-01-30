#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "heightsampler.h"

struct TerrainVertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct TerrainMesh
{
    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;
};

class TerrainMeshBuilder
{
public:
    TerrainMeshBuilder(const HeightSampler &sampler);

    TerrainMesh build(int width, int height) const;

private:
    const HeightSampler &sampler;

    glm::vec3 computeNormal(int x, int z, int width, int height) const;
};