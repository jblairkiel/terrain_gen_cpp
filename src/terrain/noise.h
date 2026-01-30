#pragma once
#include <vector>

class Noise
{
public:
    Noise(unsigned int seed = 1337);

    float perlin(float x, float y) const;

private:
    std::vector<int> perm;
};