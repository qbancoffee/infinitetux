/**
 * FlowerEnemy.cpp - Piranha Plant enemy implementation
 * 
 * Piranha plants emerge from pipes and move up/down. They are:
 * - Protected from fireballs AND shells when inside the pipe (y >= yStart - 8)
 * - Vulnerable when extended above the pipe
 * - Cannot be stomped (inherits from ENEMY_SPIKY type)
 * 
 * Movement pattern:
 * - Starts inside pipe, rises up (ya = -8)
 * - When fully emerged, waits if Mario is too close (within 24 pixels)
 * - After waiting (jumpTime > 40), descends back into pipe
 * - Cycle repeats
 */

#include "FlowerEnemy.h"
#include "LevelScene.h"
#include "Mario.h"
#include "Art.h"
#include "Sparkle.h"
#include "Fireball.h"
#include "Shell.h"
#include "Common.h"
#include <cstdlib>
#include <cmath>

/**
 * Creates a piranha plant at the specified pipe location.
 * 
 * @param world The level scene this enemy belongs to
 * @param x X position (pixel coordinates, typically pipe center)
 * @param y Y position (pixel coordinates, typically pipe top)
 */
FlowerEnemy::FlowerEnemy(LevelScene* world, int x, int y)
    : Enemy(world, x, y, 1, ENEMY_SPIKY, false) {
    
    noFireballDeath = false;  // Can be killed by fireballs (when exposed)
    this->xPic = 0;
    this->yPic = 6;    // Row 6 in enemy sprite sheet for piranha plant
    this->yPicO = 24;  // Y offset for sprite positioning
    this->height = 12;
    this->width = 2;
    
    yStart = y;  // Remember starting Y position (pipe level)
    ya = -8;     // Initial upward velocity to emerge from pipe
    
    this->y -= 1;
    this->layer = 0;  // Render behind tiles (so plant appears inside pipe)
    
    DEBUG_PRINT("FlowerEnemy spawned at (%d, %d)", x, y);
    
    // Run a few movement ticks to start the emergence animation
    for (int i = 0; i < 4; i++) {
        move();
    }
}

/**
 * Updates piranha plant state each game tick.
 * 
 * Handles:
 * - Death animation (inherited from Enemy, uses flyDeath)
 * - Up/down movement pattern based on yStart (pipe level)
 * - Waiting behavior when Mario is close
 * - Chomping animation (xPic cycles)
 */
void FlowerEnemy::move() {
    // Death animation - fly upward, then fall with gravity
    if (deadTime > 0) {
        deadTime--;
        
        if (deadTime == 0) {
            deadTime = 1;
            // Spawn sparkles on death
            for (int i = 0; i < 8; i++) {
                world->addSprite(new Sparkle(
                    (int)(x + rand() % 16 - 8) + 4,
                    (int)(y - rand() % 8) + 4,
                    (float)(rand() % 200) / 100.0f - 1,
                    (float)(rand() % 100) / 100.0f * -1,
                    0, 1, 5));
            }
            spriteContext->removeSprite(this);
        }
        
        // Continue death movement
        x += xa;
        y += ya;
        ya *= 0.95f;
        ya += 1;  // Gravity
        
        return;
    }
    
    tick++;
    
    // Movement logic - oscillate up and down from pipe
    if (y >= yStart) {
        // At or below pipe level - decide whether to emerge
        y = yStart;
        
        int xd = (int)(std::abs(world->mario->x - x));
        jumpTime++;
        
        // Only emerge if Mario is far enough away (> 24 pixels)
        // and we've waited long enough (> 40 ticks)
        if (jumpTime > 40 && xd > 24) {
            ya = -8;  // Rise up
        } else {
            ya = 0;   // Stay in pipe
        }
    } else {
        // Above pipe level - reset wait timer
        jumpTime = 0;
    }
    
    y += ya;
    ya *= 0.9f;   // Deceleration
    ya += 0.1f;   // Gravity (slow descent)
    
    // Chomping animation - cycles through 4 frames
    xPic = ((tick / 2) & 1) * 2 + ((tick / 6) & 1);
}

/**
 * Checks if a fireball hits this piranha plant.
 * 
 * The plant is protected when inside the pipe (y >= yStart - 8).
 * This prevents players from killing plants by shooting into pipes.
 * When the plant is extended above the pipe, it can be killed normally.
 * 
 * @param fireball The fireball to check collision with
 * @return true if fireball hit (and should be destroyed), false otherwise
 */
bool FlowerEnemy::fireballCollideCheck(Fireball* fireball) {
    // Plant is protected when mostly inside the pipe
    // yStart is the pipe top; when y >= yStart - 8, plant is inside
    if (y >= yStart - 8) {
        return false;  // Inside pipe, protected from fireballs
    }
    // Plant is exposed - use parent's collision check for death handling
    return Enemy::fireballCollideCheck(fireball);
}

/**
 * Checks if a shell hits this piranha plant.
 * 
 * Like fireballs, the plant is protected when inside the pipe.
 * This prevents Mario from killing plants by carrying shells into pipes.
 * 
 * @param shell The shell to check collision with
 * @return true if shell hit (and should bounce), false otherwise
 */
bool FlowerEnemy::shellCollideCheck(Shell* shell) {
    // Plant is protected when mostly inside the pipe
    if (y >= yStart - 8) {
        return false;  // Inside pipe, protected from shells
    }
    // Plant is exposed - use parent's collision check for death handling
    return Enemy::shellCollideCheck(shell);
}
