/**
 * Particle.cpp - Debris particles for brick block destruction
 * 
 * When Mario (as big Mario) breaks a brick block from below,
 * four Particle sprites are created to show the block breaking
 * into pieces. Unlike Sparkles, Particles have gravity and fall.
 * 
 * Key differences from Sparkle:
 * - Has gravity (ya += 3 each tick)
 * - Used specifically for brick debris
 * - Shorter lifetime (10 ticks)
 */

// Particle.cpp
#include "Particle.h"
#include "Art.h"
#include "LevelScene.h"
#include <cstdlib>

/**
 * Creates a particle with random sprite frame.
 * Convenience constructor that randomly selects between frame 0 and 1.
 */
Particle::Particle(int x, int y, float xa, float ya)
    : Particle(x, y, xa, ya, rand() % 2, 0) {
}

/**
 * Creates a particle with specified sprite frame.
 * 
 * @param x X position in pixels
 * @param y Y position in pixels
 * @param xa X velocity (typically ±4 to scatter debris)
 * @param ya Y velocity (typically negative for initial upward burst)
 * @param xPic X frame in particle sheet
 * @param yPic Y row in particle sheet
 */
Particle::Particle(int x, int y, float xa, float ya, int xPic, int yPic) {
    sheet = &Art::particles;
    this->x = x;
    this->y = y;
    this->xa = xa;
    this->ya = ya;
    this->xPic = xPic;
    this->yPic = yPic;
    this->xPicO = 4;  // Center offset for 8x8 sprite
    this->yPicO = 4;
    wPic = 8;
    hPic = 8;
    life = 10;  // Short lifetime for debris
}

/**
 * Updates particle position with gravity each tick.
 * 
 * Particles fall faster than other sprites due to high gravity (ya += 3).
 * This creates a realistic debris arc when blocks break.
 */
void Particle::move() {
    if (--life < 0 && spriteContext) {
        spriteContext->removeSprite(this);
        return;
    }
    
    x += xa;
    y += ya;
    ya *= 0.95f;  // Air resistance (slight)
    ya += 3;      // Strong gravity - debris falls fast
}
