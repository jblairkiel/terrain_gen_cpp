#include "heightsampler.h"

HeightSampler::HeightSampler(float ns, float hs, unsigned int seed)
    : noiseScale(ns), heightScale(hs), noise(seed) {}

float HeightSampler::sample(float x, float z) const
{
    float n = noise.perlin(x * noiseScale, z * noiseScale);
    return n * heightScale;
}