/**
 * Sparkle.cpp - Visual effect sprite for various game events
 * 
 * Sparkles are used for:
 * - Coin collection (when Mario touches a floating coin)
 * - CoinAnim disappearance (when coin block coin fades)
 * - Enemy death effects
 * - Various other visual feedback
 * 
 * Sparkles use the particles sprite sheet and animate through
 * frames based on their remaining lifetime.
 */

// Sparkle.cpp
#include "Sparkle.h"
#include "Art.h"
#include "LevelScene.h"
#include <cstdlib>

/**
 * Creates a sparkle effect at the specified position.
 * 
 * @param x X position in pixels
 * @param y Y position in pixels
 * @param xa X velocity (can be 0 for stationary sparkles)
 * @param ya Y velocity (can be 0 for stationary sparkles)
 * @param xPic Starting X frame in particle sheet
 * @param yPic Y row in particle sheet (different sparkle styles)
 * @param timeSpan Base duration modifier - actual life is 10 + random(0, timeSpan)
 */
Sparkle::Sparkle(int x, int y, float xa, float ya, int xPic, int yPic, int timeSpan)
    : xPicStart(xPic) {
    this->x = x;
    this->y = y;
    this->xa = xa;
    this->ya = ya;
    this->xPic = xPic;
    this->yPic = yPic;
    
    sheet = &Art::particles;
    wPic = 8;   // Particle sprites are 8x8
    hPic = 8;
    xPicO = 4;  // Center offset
    yPicO = 4;
    
    // Randomize lifetime for visual variety
    // Match Java: life = 10 + random(0 to timeSpan-1)
    life = 10 + rand() % timeSpan;
}

/**
 * Updates sparkle animation and position each tick.
 * 
 * Animation progresses through frames as life decreases:
 * - life > 10: xPic = 7 (bright/full sparkle)
 * - life <= 10: xPic fades from xPicStart to xPicStart+3
 * 
 * Sparkle is removed when life reaches 0.
 */
void Sparkle::move() {
    // Animate based on remaining life
    if (life > 10) {
        xPic = 7;  // Bright frame
    } else {
        // Fade animation: progress from xPicStart to xPicStart+3
        xPic = xPicStart + (10 - life) * 4 / 10;
    }
    
    // Remove when life expires
    if (--life < 0) {
        if (spriteContext) {
            spriteContext->removeSprite(this);
        }
        return;
    }
    
    // Update position
    x += xa;
    y += ya;
}
