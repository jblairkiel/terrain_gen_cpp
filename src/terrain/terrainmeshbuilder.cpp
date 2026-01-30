#include "terrainmeshbuilder.h"

TerrainMeshBuilder::TerrainMeshBuilder(const HeightSampler &s)
    : sampler(s) {}

TerrainMesh TerrainMeshBuilder::build(int width, int height) const
{
    TerrainMesh mesh;
    mesh.vertices.resize(width * height);

    // Build vertices
    for (int z = 0; z < height; z++)
    {
        for (int x = 0; x < width; x++)
        {
            float y = sampler.sample((float)x, (float)z);

            mesh.vertices[z * width + x].position = glm::vec3(x, y, z);
        }
    }

    // Build normals
    for (int z = 0; z < height; z++)
    {
        for (int x = 0; x < width; x++)
        {
            mesh.vertices[z * width + x].normal =
                computeNormal(x, z, width, height);
        }
    }

    // Build indices
    for (int z = 0; z < height - 1; z++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            int i = z * width + x;

            mesh.indices.push_back(i);
            mesh.indices.push_back(i + width);
            mesh.indices.push_back(i + 1);

            mesh.indices.push_back(i + 1);
            mesh.indices.push_back(i + width);
            mesh.indices.push_back(i + width + 1);
        }
    }

    return mesh;
}

glm::vec3 TerrainMeshBuilder::computeNormal(int x, int z, int width, int height) const
{
    float hL = sampler.sample(x - 1, z);
    float hR = sampler.sample(x + 1, z);
    float hD = sampler.sample(x, z - 1);
    float hU = sampler.sample(x, z + 1);

    glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f, hD - hU));
    return normal;
}