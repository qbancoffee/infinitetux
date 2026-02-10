/**
 * @file ImprovedNoise.h
 * @brief Perlin noise generator.
 * @ingroup level
 * 
 * ImprovedNoise generates smooth pseudo-random values
 * for terrain variation in level generation.
 */
#pragma once
#include <cmath>
#include <cstdint>

class ImprovedNoise {
public:
    ImprovedNoise(int64_t seed = 0);
    
    double noise(double x, double y, double z) const;
    double perlinNoise(double x, double y) const;

private:
    double fade(double t) const { return t * t * t * (t * (t * 6 - 15) + 10); }
    double lerp(double t, double a, double b) const { return a + t * (b - a); }
    double grad(int hash, double x, double y, double z) const;
    
    int p[512];
};
