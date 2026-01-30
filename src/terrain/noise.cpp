#include "noise.h"
#include <numeric>
#include <algorithm>
#include <random>
#include <cmath>

static float fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

static float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static float grad(int hash, float x, float y)
{
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

Noise::Noise(unsigned int seed)
{
    perm.resize(256);
    std::iota(perm.begin(), perm.end(), 0);

    std::default_random_engine rng(seed);
    std::shuffle(perm.begin(), perm.end(), rng);

    // Duplicate the permutation table
    perm.insert(perm.end(), perm.begin(), perm.end());
}

float Noise::perlin(float x, float y) const
{
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;

    float xf = x - std::floor(x);
    float yf = y - std::floor(y);

    float u = fade(xf);
    float v = fade(yf);

    int aa = perm[perm[xi] + yi];
    int ab = perm[perm[xi] + yi + 1];
    int ba = perm[perm[xi + 1] + yi];
    int bb = perm[perm[xi + 1] + yi + 1];

    float x1 = lerp(grad(aa, xf, yf),
                    grad(ba, xf - 1, yf), u);

    float x2 = lerp(grad(ab, xf, yf - 1),
                    grad(bb, xf - 1, yf - 1), u);

    float result = lerp(x1, x2, v);

    return result;
}