/**
 * @file ImprovedNoise.cpp
 * @brief Perlin noise implementation.
 */
// ImprovedNoise.cpp - Perlin noise for terrain generation
#include "ImprovedNoise.h"
#include "Common.h"  // For Random class
#include <algorithm>

ImprovedNoise::ImprovedNoise(int64_t seed) {
    // Use Java-compatible Random for shuffling
    Random random(seed);
    
    // Initialize permutation array with 0-255
    int permutation[256];
    for (int i = 0; i < 256; i++) {
        permutation[i] = i;
    }
    
    // Shuffle exactly like Java does
    for (int i = 0; i < 256; i++) {
        int j = random.nextInt(256 - i) + i;
        int tmp = permutation[i];
        permutation[i] = permutation[j];
        permutation[j] = tmp;
        p[i + 256] = p[i] = permutation[i];
    }
}

double ImprovedNoise::grad(int hash, double x, double y, double z) const {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double ImprovedNoise::noise(double x, double y, double z) const {
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;
    
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);
    
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);
    
    int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
    int B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;
    
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
                                   grad(p[BA], x - 1, y, z)),
                          lerp(u, grad(p[AB], x, y - 1, z),
                                   grad(p[BB], x - 1, y - 1, z))),
                  lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                                   grad(p[BA + 1], x - 1, y, z - 1)),
                          lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                                   grad(p[BB + 1], x - 1, y - 1, z - 1))));
}

double ImprovedNoise::perlinNoise(double x, double y) const {
    double n = 0;
    
    for (int i = 0; i < 8; i++) {
        double stepSize = 64.0 / (1 << i);
        n += noise(x / stepSize, y / stepSize, 128) * 1.0 / (1 << i);
    }
    
    return n;
}
