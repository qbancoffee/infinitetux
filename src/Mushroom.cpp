/**
 * @file Mushroom.cpp
 * @brief Power-up mushroom implementation.
 */
// Mushroom.cpp
#include "Mushroom.h"
#include "LevelScene.h"
#include "Level.h"
#include "Mario.h"
#include "Art.h"

Mushroom::Mushroom(LevelScene* world, float x, float y) : world(world) {
    this->x = x;
    this->y = y;
    sheet = &Art::items;
    xPic = 0;
    yPic = 0;
    wPic = 16;
    hPic = 16;
    xPicO = 8;
    yPicO = 15;
    facing = 1;
    life = 0;
}

void Mushroom::move() {
    if (life < 9) {
        layer = 0;
        y--;
        life++;
        return;
    }
    layer = 1;
    
    float sideWaysSpeed = 1.75f;
    if (xa > 2) facing = 1;
    if (xa < -2) facing = -1;
    
    xa = facing * sideWaysSpeed;
    xFlipPic = facing == -1;
    
    if (!moveImpl(xa, 0)) facing = -facing;
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

void Mushroom::collideCheck() {
    float xMarioD = world->mario->x - x;
    float yMarioD = world->mario->y - y;
    
    if (xMarioD > -16 && xMarioD < 16) {
        if (yMarioD > -height && yMarioD < world->mario->hPic) {
            world->mario->getMushroom();
            world->removeSprite(this);
        }
    }
}

bool Mushroom::moveImpl(float xa, float ya) {
    bool collide = false;
    
    if (ya > 0) {
        if (isBlocking(x + xa - width, y + ya, xa, 0)) collide = true;
        else if (isBlocking(x + xa + width, y + ya, xa, 0)) collide = true;
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
    }
    
    x += xa;
    y += ya;
    return true;
}

bool Mushroom::isBlocking(float _x, float _y, float xa, float ya) {
    int tx = (int)(_x / 16);
    int ty = (int)(_y / 16);
    if (tx == (int)(x / 16) && ty == (int)(y / 16)) return false;
    return world->level->isBlocking(tx, ty, xa, ya);
}

void Mushroom::bumpCheck(int xTile, int yTile) {
    // Check if mushroom is on top of the bumped block
    // yTile is the block that was bumped, mushroom should be on yTile-1 (one tile above)
    // or standing on yTile
    if (x + width > xTile * 16 && x - width < xTile * 16 + 16 && 
        yTile == (int)(y / 16)) {
        // Mushroom is on this block - make it jump and change direction
        ya = -6;  // Jump up
        facing = -facing;  // Change direction
        onGround = false;
    }
}
