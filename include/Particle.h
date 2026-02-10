/**
 * @file Particle.h
 * @brief Visual particle effects.
 * @ingroup sprites
 * 
 * Particle is physics-driven debris with no collision,
 * used for brick breaking effects.
 */
#pragma once
#include "Sprite.h"

class Particle : public Sprite {
public:
    Particle(int x, int y, float xa, float ya);
    Particle(int x, int y, float xa, float ya, int xPic, int yPic);
    void move() override;

private:
    int life = 10;
};
