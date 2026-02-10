/**
 * @file Enemy.cpp
 * @brief Ground enemy implementation.
 */
#include "Enemy.h"
#include "Art.h"
#include "LevelScene.h"
#include "Level.h"
#include "Mario.h"
#include "Sparkle.h"
#include "Shell.h"
#include "Fireball.h"
#include "SpriteTemplate.h"
#include <cmath>

static const char* getEnemyTypeName(int type) {
    switch (type) {
        case Enemy::ENEMY_RED_KOOPA: return "Red Koopa";
        case Enemy::ENEMY_GREEN_KOOPA: return "Green Koopa";
        case Enemy::ENEMY_GOOMBA: return "Goomba";
        case Enemy::ENEMY_SPIKY: return "Spiky";
        case Enemy::ENEMY_FLOWER: return "Flower";
        default: return "Unknown";
    }
}

Enemy::Enemy(LevelScene* world, int x, int y, int dir, int type, bool winged)
    : world(world), type(type) {
    sheet = &Art::enemies;
    this->winged = winged;
    this->x = x;
    this->y = y;
    xPicO = 8;
    yPicO = 31;
    layer = 1;  // Render in game layer
    
    avoidCliffs = type == ENEMY_RED_KOOPA;
    noFireballDeath = false;  // All enemies can be killed by fireballs (spinies too)
    yPic = type;
    if (yPic > 1) height = 12;
    facing = dir;
    if (facing == 0) facing = 1;
    wPic = 16;
    
    // Push enemy up if spawning inside ground
    // This handles cases where level generation places enemy sprite inside a tile
    int maxIterations = 8;
    while (maxIterations > 0) {
        int tileX = (int)this->x / 16;
        int tileY = (int)this->y / 16;
        
        if (tileX >= 0 && tileX < world->level->width && 
            tileY >= 0 && tileY < world->level->height) {
            uint8_t block = world->level->getBlock(tileX, tileY);
            if ((Level::TILE_BEHAVIORS[block] & Level::BIT_BLOCK_ALL) != 0) {
                this->y -= 16;
                DEBUG_PRINT("Enemy pushed up from ground at tile (%d, %d)", tileX, tileY);
                maxIterations--;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    
    // Initialize old position to current to prevent interpolation glitches
    xOld = this->x;
    yOld = this->y;
    
    DEBUG_PRINT("Enemy spawned: %s at (%d, %d) dir=%d winged=%s", 
                getEnemyTypeName(type), (int)this->x, (int)this->y, dir, winged ? "yes" : "no");
}

void Enemy::collideCheck() {
    if (deadTime != 0) return;
    
    float xMarioD = world->mario->x - x;
    float yMarioD = world->mario->y - y;
    
    // Check if Mario and enemy are overlapping horizontally
    if (xMarioD > -width * 2 - 4 && xMarioD < width * 2 + 4) {
        // Check vertical overlap
        if (yMarioD > -height && yMarioD < world->mario->height) {
            // Mario can stomp if:
            // 1. Not a spiky enemy
            // 2. Mario is moving downward (ya > 0)
            // 3. Mario's feet are above or at enemy's head (yMarioD <= 0)
            // 4. Mario is not standing firmly on ground
            if (type != ENEMY_SPIKY && world->mario->ya > 0 && yMarioD <= 0 && 
                (!world->mario->onGround || !world->mario->wasOnGround)) {
                world->mario->stomp(this);
                if (winged) {
                    winged = false;
                    ya = 0;
                    DEBUG_PRINT("Enemy %s at (%.0f, %.0f) lost wings from stomp", getEnemyTypeName(type), x, y);
                } else {
                    yPicO = 31 - (32 - 8);
                    hPic = 8;
                    if (spriteTemplate) spriteTemplate->isDead = true;
                    deadTime = 10;
                    winged = false;
                    DEBUG_PRINT("Enemy %s stomped at (%.0f, %.0f)", getEnemyTypeName(type), x, y);
                    
                    if (type == ENEMY_RED_KOOPA) {
                        spriteContext->addSprite(new Shell(world, x, y, 0));
                        DEBUG_PRINT("  -> Spawned red shell");
                    } else if (type == ENEMY_GREEN_KOOPA) {
                        spriteContext->addSprite(new Shell(world, x, y, 1));
                        DEBUG_PRINT("  -> Spawned green shell");
                    }
                }
            } else {
                // Enemy hurts Mario
                world->mario->getHurt();
            }
        }
    }
}

/**
 * Updates enemy state each game tick.
 * 
 * This method handles:
 * - Death animation: If deadTime > 0, the enemy is dying. During flyDeath,
 *   the enemy continues moving with gravity (ya += 1) until deadTime reaches 0,
 *   at which point sparkles are spawned and the enemy is removed.
 * 
 * - Normal movement: Enemies walk left/right based on facing direction.
 *   Red koopas (avoidCliffs=true) turn around at ledges to avoid falling.
 *   
 * - Winged enemies: Have additional jumping behavior and wing animation.
 * 
 * - Animation: xPic cycles through walk frames based on runTime.
 */
void Enemy::move() {
    wingTime++;
    
    // === DEATH ANIMATION ===
    // When deadTime > 0, enemy is in death animation (killed by fireball, shell, etc.)
    if (deadTime > 0) {
        deadTime--;
        if (deadTime == 0) {
            // Death animation complete - spawn sparkles and remove enemy
            deadTime = 1;  // Prevent re-triggering
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
        
        // flyDeath: enemy flies through air when killed (e.g., by fireball)
        // The enemy continues moving with gravity until deadTime expires
        if (flyDeath) {
            x += xa;
            y += ya;
            ya *= 0.95f;  // Air resistance
            ya += 1;      // Gravity
        }
        return;  // Skip normal movement when dying
    }
    
    // === NORMAL MOVEMENT ===
    float sideWaysSpeed = 1.75f;  // Base walking speed for all enemies
    
    // Update facing direction based on velocity
    if (xa > 2) facing = 1;   // Moving right
    if (xa < -2) facing = -1; // Moving left
    
    xa = facing * sideWaysSpeed;
    mayJump = onGround;
    xFlipPic = facing == -1;  // Flip sprite horizontally when facing left
    runTime += std::abs(xa) + 5;
    
    // Animation frame - cycles between 0 and 1 for walk animation
    int runFrame = ((int)(runTime / 20)) % 2;
    if (!onGround) runFrame = 1;
    
    if (!moveImpl(xa, 0)) facing = -facing;
    onGround = false;
    moveImpl(0, ya);
    
    ya *= winged ? 0.95f : 0.85f;
    if (onGround) {
        xa *= GROUND_INERTIA;
    } else {
        xa *= AIR_INERTIA;
    }
    
    if (!onGround) {
        ya += winged ? 0.6f : 2;
    } else if (winged) {
        ya = -10;
    }
    
    if (winged) runFrame = wingTime / 4 % 2;
    xPic = runFrame;
}

bool Enemy::moveImpl(float xa, float ya) {
    bool collide = false;
    
    if (ya > 0) {
        // Pass ya (not 0) so BIT_BLOCK_UPPER tiles are detected
        if (isBlocking(x + xa - width, y + ya, xa, ya)) collide = true;
        else if (isBlocking(x + xa + width, y + ya, xa, ya)) collide = true;
    }
    if (ya < 0) {
        if (isBlocking(x + xa, y + ya - height, xa, ya)) collide = true;
    }
    if (xa > 0) {
        if (isBlocking(x + xa + width, y + ya - height, xa, ya)) collide = true;
        if (isBlocking(x + xa + width, y + ya, xa, ya)) collide = true;
        if (avoidCliffs && onGround && !world->level->isBlocking(
            (int)((x + xa + width) / 16), (int)(y / 16 + 1), xa, 1)) collide = true;
    }
    if (xa < 0) {
        if (isBlocking(x + xa - width, y + ya - height, xa, ya)) collide = true;
        if (isBlocking(x + xa - width, y + ya, xa, ya)) collide = true;
        if (avoidCliffs && onGround && !world->level->isBlocking(
            (int)((x + xa - width) / 16), (int)(y / 16 + 1), xa, 1)) collide = true;
    }
    
    if (collide) {
        if (xa < 0) {
            x = (int)((x - width) / 16) * 16 + width;
            this->xa = 0;
        }
        if (xa > 0) {
            x = (int)((x + width) / 16 + 1) * 16 - width - 1;
            this->xa = 0;
        }
        if (ya < 0) {
            y = (int)((y - height) / 16) * 16 + height;
            jumpTime = 0;
            this->ya = 0;
        }
        if (ya > 0) {
            y = (int)(y / 16 + 1) * 16 - 1;
            onGround = true;
        }
        return false;
    } else {
        x += xa;
        y += ya;
        return true;
    }
}

bool Enemy::isBlocking(float _x, float _y, float xa, float ya) {
    int tx = (int)(_x / 16);
    int ty = (int)(_y / 16);
    if (tx == (int)(x / 16) && ty == (int)(y / 16)) return false;
    return world->level->isBlocking(tx, ty, xa, ya);
}

bool Enemy::shellCollideCheck(Shell* shell) {
    if (deadTime != 0) return false;
    
    float xD = shell->x - x;
    float yD = shell->y - y;
    
    if (xD > -16 && xD < 16) {
        if (yD > -height && yD < shell->height) {
            Art::playSound(SAMPLE_MARIO_KICK);
            xa = shell->facing * 2;
            ya = -5;
            flyDeath = true;
            if (spriteTemplate) spriteTemplate->isDead = true;
            deadTime = 100;
            winged = false;
            yFlipPic = true;  // Flip upside down
            return true;
        }
    }
    return false;
}

/**
 * Checks if a fireball has collided with this enemy and handles the death.
 * 
 * When an enemy is killed by a fireball:
 * - yFlipPic is set to true to flip the sprite upside down
 * - It flies in the direction the fireball was traveling (xa = fireball->facing * 2)
 * - It has upward initial velocity (ya = -5) then falls with gravity
 * - flyDeath flag is set so move() continues updating position during death
 * - After deadTime ticks, sparkles spawn and enemy is removed
 * 
 * Note: noFireballDeath can make enemies immune to fireballs (not currently used).
 * 
 * @param fireball The fireball to check collision with
 * @return true if collision occurred (fireball should die), false otherwise
 */
bool Enemy::fireballCollideCheck(Fireball* fireball) {
    if (deadTime != 0) return false;
    
    float xD = fireball->x - x;
    float yD = fireball->y - y;
    
    // Check bounding box collision
    if (xD > -16 && xD < 16) {
        if (yD > -height && yD < fireball->height) {
            if (noFireballDeath) {
                DEBUG_PRINT("Enemy %s at (%.0f, %.0f) immune to fireball", getEnemyTypeName(type), x, y);
                return true;  // Absorb fireball but don't die
            }
            
            Art::playSound(SAMPLE_MARIO_KICK);
            DEBUG_PRINT("Enemy %s killed by fireball at (%.0f, %.0f), fireball facing=%d", 
                        getEnemyTypeName(type), x, y, fireball->facing);
            Mario::addScore(200);  // More points for fireball kills
            
            // Set death animation velocity - fly in direction fireball was traveling
            xa = fireball->facing * 2;
            ya = -5;  // Initial upward velocity
            flyDeath = true;  // Enable movement during death animation
            
            if (spriteTemplate) spriteTemplate->isDead = true;
            deadTime = 100;  // Ticks until removal
            winged = false;  // Remove wings
            
            // Flip sprite upside down for death animation
            // Use yFlipPic instead of negative hPic for cleaner flip
            yFlipPic = true;
            
            return true;
        }
    }
    return false;
}

void Enemy::bumpCheck(int xTile, int yTile) {
    if (deadTime != 0) return;
    
    if (x + width > xTile * 16 && x - width < xTile * 16 + 16 && 
        yTile == (int)((y - 1) / 16)) {
        Art::playSound(SAMPLE_MARIO_KICK);
        xa = -world->mario->facing * 2;
        ya = -5;
        flyDeath = true;
        if (spriteTemplate) spriteTemplate->isDead = true;
        deadTime = 100;
        winged = false;
        yFlipPic = true;  // Flip upside down
    }
}

void Enemy::render(SDL_Renderer* renderer, float alpha) {
    if (!visible || !sheet || sheet->empty()) return;
    
    int xPixel = (int)(xOld + (x - xOld) * alpha) - xPicO;
    int yPixel = (int)(yOld + (y - yOld) * alpha) - yPicO;
    
    // For winged non-koopa enemies, render back wing first (behind body)
    if (winged && type != ENEMY_GREEN_KOOPA && type != ENEMY_RED_KOOPA) {
        bool oldFlip = xFlipPic;
        xFlipPic = !xFlipPic;
        
        int wingXPic = wingTime / 4 % 2;
        int wingYPic = 4;  // Wings row
        
        if (wingXPic < (int)sheet->size() && wingYPic < (int)(*sheet)[wingXPic].size()) {
            SDL_Texture* wingTex = (*sheet)[wingXPic][wingYPic];
            if (wingTex) {
                // Java: xPixel + (xFlipPic ? wPic : 0) + (xFlipPic ? 10 : -10)
                // When xFlipPic=false: xPixel - 10 (wing to left)
                // When xFlipPic=true: xPixel + wPic + 10, but draws right-to-left
                int wingX;
                if (xFlipPic) {
                    // Flipped - wing on right side, but need to account for SDL flip behavior
                    wingX = xPixel + 10;
                } else {
                    // Not flipped - wing on left side
                    wingX = xPixel - 10;
                }
                int wingY = yPixel - 8;
                SDL_Rect dst = {wingX, wingY, wPic, hPic};
                
                SDL_RendererFlip flip = xFlipPic ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                SDL_RenderCopyEx(renderer, wingTex, nullptr, &dst, 0, nullptr, flip);
            }
        }
        xFlipPic = oldFlip;
    }
    
    // Render the main enemy body using base class render
    Sprite::render(renderer, alpha);
    
    // Render wings in front for all winged enemies
    if (winged) {
        int wingXPic = wingTime / 4 % 2;
        int wingYPic = 4;  // Wings row
        
        if (wingXPic < (int)sheet->size() && wingYPic < (int)(*sheet)[wingXPic].size()) {
            SDL_Texture* wingTex = (*sheet)[wingXPic][wingYPic];
            if (wingTex) {
                // Java: xPixel + (xFlipPic ? wPic : 0) + (xFlipPic ? 10 : -10)
                // When xFlipPic=false: xPixel - 10, wing drawn normally
                // When xFlipPic=true: xPixel + wPic + 10, wing flipped (draws from that point leftward)
                int wingX;
                if (xFlipPic) {
                    // Flipped (facing left) - wing on right side of body
                    wingX = xPixel + 10;
                } else {
                    // Not flipped (facing right) - wing on left side of body
                    wingX = xPixel - 10;
                }
                int wingY = yPixel;
                if (type == ENEMY_GREEN_KOOPA || type == ENEMY_RED_KOOPA) {
                    wingY -= 10;
                } else {
                    wingY -= 8;
                }
                
                SDL_Rect dst = {wingX, wingY, wPic, hPic};
                
                SDL_RendererFlip flip = xFlipPic ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                SDL_RenderCopyEx(renderer, wingTex, nullptr, &dst, 0, nullptr, flip);
            }
        }
    }
}
