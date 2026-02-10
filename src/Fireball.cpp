/**
 * @file Fireball.cpp
 * @brief Fireball projectile implementation.
 */
#include "Fireball.h"
#include "LevelScene.h"
#include "Level.h"
#include "Art.h"
#include "Sparkle.h"

Fireball::Fireball(LevelScene* world, float x, float y, int facing)
    : world(world) {
    this->x = x;
    this->y = y;
    this->facing = facing;
    
    sheet = &Art::particles;
    xPicO = 4;
    yPicO = 4;
    wPic = 8;
    hPic = 8;
    xPic = 4;
    yPic = 3;
    layer = 1;
    height = 8;
    
    ya = 4;
    
    world->fireballsOnScreen++;
}

void Fireball::move() {
    if (deadTime > 0) {
        // Spawn death sparkles
        for (int i = 0; i < 8; i++) {
            world->addSprite(new Sparkle(
                (int)(x + (rand() % 8) - 4) + 4,
                (int)(y + (rand() % 8) - 4) + 2,
                (float)(rand() % 200) / 100.0f - 1 - facing,
                (float)(rand() % 200) / 100.0f - 1,
                0, 1, 5));
        }
        world->fireballsOnScreen--;
        world->removeSprite(this);
        return;
    }
    
    if (facing != 0) anim++;
    
    float sideWaysSpeed = 8.0f;
    
    if (xa > 2) facing = 1;
    if (xa < -2) facing = -1;
    
    xa = facing * sideWaysSpeed;
    
    world->checkFireballCollide(this);
    
    xFlipPic = (facing == -1);
    
    xPic = anim % 4;
    
    // Horizontal movement - die if blocked
    if (!moveImpl(xa, 0)) {
        die();
        return;
    }
    
    // Vertical movement - bounce if on ground
    onGround = false;
    moveImpl(0, ya);
    
    if (onGround) {
        ya = -10;
    }
    
    ya *= 0.95f;
    if (onGround) {
        xa *= 0.89f;
    } else {
        xa *= 0.89f;
    }
    
    if (!onGround) {
        ya += 1.5f;
    }
    
    // Remove if off screen
    if (x < world->xCam - 32 || x > world->xCam + SCREEN_WIDTH + 32 ||
        y < world->yCam - 32 || y > world->yCam + SCREEN_HEIGHT + 32) {
        world->fireballsOnScreen--;
        world->removeSprite(this);
    }
}

bool Fireball::moveImpl(float xa, float ya) {
    // Handle large movements in steps
    while (xa > 8) {
        if (!moveImpl(8, 0)) return false;
        xa -= 8;
    }
    while (xa < -8) {
        if (!moveImpl(-8, 0)) return false;
        xa += 8;
    }
    while (ya > 8) {
        if (!moveImpl(0, 8)) return false;
        ya -= 8;
    }
    while (ya < -8) {
        if (!moveImpl(0, -8)) return false;
        ya += 8;
    }
    
    bool collide = false;
    
    if (ya > 0) {
        if (isBlocking(x + xa - width, y + ya, xa, 0)) collide = true;
        else if (isBlocking(x + xa + width, y + ya, xa, 0)) collide = true;
        else if (isBlocking(x + xa - width, y + ya + 1, xa, ya)) collide = true;
        else if (isBlocking(x + xa + width, y + ya + 1, xa, ya)) collide = true;
    }
    if (ya < 0) {
        if (isBlocking(x + xa, y + ya - height, xa, ya)) collide = true;
        else if (isBlocking(x + xa - width, y + ya - height, xa, ya)) collide = true;
        else if (isBlocking(x + xa + width, y + ya - height, xa, ya)) collide = true;
    }
    if (xa > 0) {
        if (isBlocking(x + xa + width, y + ya - height, xa, ya)) collide = true;
        if (isBlocking(x + xa + width, y + ya - height / 2, xa, ya)) collide = true;
        if (isBlocking(x + xa + width, y + ya, xa, ya)) collide = true;
    }
    if (xa < 0) {
        if (isBlocking(x + xa - width, y + ya - height, xa, ya)) collide = true;
        if (isBlocking(x + xa - width, y + ya - height / 2, xa, ya)) collide = true;
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
        if (ya < 0) {
            y = (int)((y - height) / 16) * 16 + height;
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

bool Fireball::isBlocking(float _x, float _y, float xa, float ya) {
    int tx = (int)(_x / 16);
    int ty = (int)(_y / 16);
    
    // Don't collide with current tile
    if (tx == (int)(this->x / 16) && ty == (int)(this->y / 16)) return false;
    
    return world->level->isBlocking(tx, ty, xa, ya);
}

void Fireball::die() {
    dead = true;
    xa = -facing * 2;
    ya = -5;
    deadTime = 100;
}
