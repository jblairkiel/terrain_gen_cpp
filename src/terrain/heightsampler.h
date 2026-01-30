#pragma once
#include "noise.h"

class HeightSampler
{
public:
    HeightSampler(float noiseScale, float heightScale, unsigned int seed = 1337);

    float sample(float x, float z) const;

private:
    float noiseScale;
    float heightScale;
    Noise noise;
};