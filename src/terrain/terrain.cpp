#include "terrain.h"

Terrain::Terrain(int width, int height, float ns, float hs)
    : sampler(ns, hs), builder(sampler)
{
    mesh = builder.build(width, height);
}