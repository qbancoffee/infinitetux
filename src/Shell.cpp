/**
 * Shell.cpp - Koopa shell that can be kicked, carried, and used as a weapon
 * 
 * Shells are created when koopas are stomped. They can be:
 * - Kicked to slide across the ground and kill enemies
 * - Picked up and carried (hold run button while stomping)
 * - Thrown to attack enemies
 * - Hit by fireballs (all shells can be killed by fireballs)
 */

// Shell.cpp
#include "Shell.h"
#include "LevelScene.h"
#include "Level.h"
#include "Mario.h"
#include "Art.h"
#include "Fireball.h"
#include "SpriteTemplate.h"
#include "Common.h"

Shell::Shell(LevelScene* world, float x, float y, int type)
    : world(world), type(type) {
    this->x = x;
    this->y = y;
    sheet = &Art::enemies;
    xPicO = 8;
    yPicO = 31;
    yPic = type;  // 0 = red shell, 1 = green shell (same row as koopa)
    xPic = 4;     // Shell frame
    wPic = 16;
    hPic = 32;
    height = 12;
    ya = -5;      // Initial upward velocity when spawned
    
    // Push shell up if spawning inside ground (same as Enemy)
    int maxIterations = 8;
    while (maxIterations > 0) {
        int tileX = (int)this->x / 16;
        int tileY = (int)this->y / 16;
        
        if (tileX >= 0 && tileX < world->level->width && 
            tileY >= 0 && tileY < world->level->height) {
            uint8_t block = world->level->getBlock(tileX, tileY);
            if ((Level::TILE_BEHAVIORS[block] & Level::BIT_BLOCK_ALL) != 0) {
                this->y -= 16;
                DEBUG_PRINT("Shell pushed up from ground at tile (%d, %d)", tileX, tileY);
                maxIterations--;
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

/**
 * Override tick to handle carried state properly.
 * When carried, Mario has already set x, y, xOld, yOld - don't overwrite them.
 * When not carried, use standard tick behavior.
 */
void Shell::tick() {
    if (carried) {
        // When carried, Mario has set our position AND old position
        // Just call move() which will return early for carried shells
        move();
    } else {
        // Normal tick: save old position then move
        xOld = x;
        yOld = y;
        move();
    }
}

/**
 * Render the shell. When carried, the shell uses its own coordinates which
 * have been set by Mario to be relative to Mario's position. LevelScene
 * transforms these to camera space before calling render.
 */
void Shell::render(SDL_Renderer* renderer, float alpha) {
    if (!visible || !sheet || sheet->empty()) return;
    
    // Use standard interpolation - when carried, Mario has already set
    // our x/y to be at the correct position relative to Mario
    int xPixel = (int)(xOld + (x - xOld) * alpha) - xPicO;
    int yPixel = (int)(yOld + (y - yOld) * alpha) - yPicO;
    
    // When carried, flip so the larger hole faces AWAY from Mario
    bool flipH = xFlipPic;
    if (carried && world->mario) {
        // Flip when Mario faces right (1), don't flip when Mario faces left (-1)
        flipH = (world->mario->facing == 1);
    }
    
    if (xPic >= 0 && xPic < (int)sheet->size() && 
        yPic >= 0 && yPic < (int)(*sheet)[xPic].size()) {
        SDL_Texture* tex = (*sheet)[xPic][yPic];
        if (tex) {
            int renderHeight = hPic;
            bool flipVertical = yFlipPic;
            
            if (hPic < 0) {
                renderHeight = -hPic;
                flipVertical = !flipVertical;
            }
            
            SDL_Rect dst = {xPixel, yPixel, wPic, renderHeight};
            
            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (flipH) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
            if (flipVertical) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);
            
            SDL_RenderCopyEx(renderer, tex, nullptr, &dst, 0, nullptr, flip);
        }
    }
}

void Shell::move() {
    if (carried) {
        world->checkShellCollide(this);
        return;
    }
    
    if (deadTime > 0) {
        deadTime--;
        if (deadTime == 0) {
            world->removeSprite(this);
        }
        x += xa;
        y += ya;
        ya *= 0.95f;
        ya += 1;
        return;
    }
    
    if (facing != 0) anim++;
    
    float sideWaysSpeed = 11.0f;
    
    if (xa > 2) facing = 1;
    if (xa < -2) facing = -1;
    
    xa = facing * sideWaysSpeed;
    
    if (facing != 0) {
        world->checkShellCollide(this);
    }
    
    xFlipPic = facing == -1;
    
    // Shell animation: frames 3-6 when moving, frame 4 when still
    xPic = (anim / 2) % 4 + 3;
    
    if (!moveImpl(xa, 0)) {
        // Only play sound if shell is on screen
        if (x >= world->xCam - 16 && x <= world->xCam + SCREEN_WIDTH + 16 &&
            y >= world->yCam - 16 && y <= world->yCam + SCREEN_HEIGHT + 16) {
            Art::playSound(SAMPLE_SHELL_BUMP);
        }
        facing = -facing;
    }
    onGround = false;
    moveImpl(0, ya);
    
    ya *= 0.85f;
    if (onGround) {
        xa *= 0.89f;
    } else {
        xa *= 0.89f;
    }
    
    if (!onGround) {
        ya += 2;
    }
}

void Shell::collideCheck() {
    if (carried || dead || deadTime > 0) return;
    
    float xMarioD = world->mario->x - x;
    float yMarioD = world->mario->y - y;
    float w = 16;
    
    if (xMarioD > -w && xMarioD < w) {
        if (yMarioD > -height && yMarioD < world->mario->height) {
            // Check for stomp - Mario is above and moving down
            if (world->mario->ya > 0 && yMarioD <= 0 && 
                (!world->mario->onGround || !world->mario->wasOnGround)) {
                world->mario->stomp(this);
                if (facing != 0) {
                    xa = 0;
                    facing = 0;
                } else {
                    facing = world->mario->facing;
                }
            } else {
                // Not a stomp
                if (facing != 0) {
                    // Moving shell hurts Mario
                    world->mario->getHurt();
                } else {
                    // Stationary shell - Mario kicks it
                    world->mario->kick(this);
                    facing = world->mario->facing;
                }
            }
        }
    }
}

bool Shell::moveImpl(float xa, float ya) {
    bool collide = false;
    
    if (ya > 0) {
        // Pass ya (not 0) so BIT_BLOCK_UPPER tiles are detected
        if (isBlocking(x + xa - width, y + ya, xa, ya)) collide = true;
        else if (isBlocking(x + xa + width, y + ya, xa, ya)) collide = true;
    }
    if (xa > 0) {
        if (isBlocking(x + xa + width, y + ya, xa, ya)) collide = true;
    }
    if (xa < 0) {
        if (isBlocking(x + xa - width, y + ya, xa, ya)) collide = true;
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

bool Shell::isBlocking(float _x, float _y, float xa, float ya) {
    int tx = (int)(_x / 16);
    int ty = (int)(_y / 16);
    if (tx == (int)(x / 16) && ty == (int)(y / 16)) return false;
    return world->level->isBlocking(tx, ty, xa, ya);
}

void Shell::bumpCheck(int xTile, int yTile) {
    if (x + width > xTile * 16 && x - width < xTile * 16 + 16 && 
        yTile == (int)((y - 1) / 16)) {
        facing = -world->mario->facing;
        ya = -10;
    }
}

void Shell::release(Mario* mario) {
    carried = false;
    facing = mario->facing;
    x += facing * 8;
}

/**
 * Checks if a fireball collides with this shell.
 * 
 * Any shell hit by a fireball gets killed (flips upside down and flies away).
 * 
 * @param fireball The fireball to check collision with
 * @return true if collision occurred (fireball should die), false otherwise
 */
bool Shell::fireballCollideCheck(Fireball* fireball) {
    if (deadTime != 0) return false;
    
    float xD = fireball->x - x;
    float yD = fireball->y - y;
    
    if (xD > -16 && xD < 16) {
        if (yD > -height && yD < fireball->height) {
            // Shell gets killed by fireball
            Art::playSound(SAMPLE_MARIO_KICK);
            
            xa = fireball->facing * 2;
            ya = -5;
            if (spriteTemplate) spriteTemplate->isDead = true;
            deadTime = 100;
            yFlipPic = true;  // Flip upside down
            return true;
        }
    }
    return false;
}

bool Shell::shellCollideCheck(Shell* shell) {
    if (deadTime != 0) return false;
    
    float xD = shell->x - x;
    float yD = shell->y - y;
    
    if (xD > -16 && xD < 16) {
        if (yD > -height && yD < shell->height) {
            Art::playSound(SAMPLE_MARIO_KICK);
            
            if (world->mario->carried == shell || world->mario->carried == this) {
                world->mario->carried = nullptr;
            }
            
            die();
            shell->die();
            return true;
        }
    }
    return false;
}

void Shell::die() {
    dead = true;
    carried = false;
    xa = -facing * 2;
    ya = -5;
    deadTime = 100;
    yFlipPic = true;  // Flip upside down
}
