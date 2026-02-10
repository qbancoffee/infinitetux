/**
 * CoinAnim.cpp - Animated coin that pops out of coin blocks
 * 
 * When Mario hits a coin block from below, a CoinAnim sprite is created
 * that shows a coin popping up, spinning, and disappearing with sparkles.
 * 
 * Animation sequence:
 * 1. Coin shoots upward (ya = -6 initial velocity)
 * 2. Coin spins (xPic cycles through 4 frames)
 * 3. Gravity pulls coin back down (ya += 1 each tick)
 * 4. After 10 ticks, coin disappears with sparkle effect
 * 
 * Note: Uses Art::level sprite sheet (not items), row 2 for coin frames.
 */

// CoinAnim.cpp
#include "CoinAnim.h"
#include "LevelScene.h"
#include "Art.h"
#include "Sparkle.h"
#include <cstdlib>

/**
 * Creates a coin animation at the specified tile position.
 * 
 * The coin starts slightly above the block (y - 16 pixels) and
 * shoots upward with initial velocity ya = -6.
 * 
 * @param xTile X tile coordinate (will be converted to pixels)
 * @param yTile Y tile coordinate (will be converted to pixels)
 */
CoinAnim::CoinAnim(int xTile, int yTile) {
    sheet = &Art::level;  // Use level sheet (has coin animation frames)
    wPic = 16;
    hPic = 16;
    
    // Convert tile coordinates to pixel coordinates
    // Start above the block that was hit
    x = xTile * 16;
    y = yTile * 16 - 16;
    
    xa = 0;
    ya = -6.0f;  // Initial upward velocity
    xPic = 0;
    yPic = 2;    // Row 2 in level sheet has coin frames
}

/**
 * Updates coin animation each tick.
 * 
 * Handles:
 * - Spinning animation (xPic cycles 0-3)
 * - Physics (gravity pulls coin down)
 * - Death with sparkle effect after life expires
 */
void CoinAnim::move() {
    if (--life < 0) {
        if (spriteContext) {
            // Spawn sparkles when coin disappears
            for (int xx = 0; xx < 2; xx++) {
                for (int yy = 0; yy < 2; yy++) {
                    spriteContext->addSprite(new Sparkle(
                        (int)x + xx * 8 + rand() % 8,
                        (int)y + yy * 8 + rand() % 8,
                        0, 0, 0, 2, 5));
                }
            }
            spriteContext->removeSprite(this);
        }
        return;
    }
    
    // Spinning animation - cycle through 4 frames
    xPic = life & 3;
    
    // Apply velocity and gravity
    x += xa;
    y += ya;
    ya += 1;  // Gravity
}
