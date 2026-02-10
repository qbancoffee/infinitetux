/**
 * @file FireFlower.cpp
 * @brief Fire flower implementation.
 */
// FireFlower.cpp
#include "FireFlower.h"
#include "LevelScene.h"
#include "Mario.h"
#include "Art.h"

FireFlower::FireFlower(LevelScene* world, float x, float y) : world(world) {
    this->x = x;
    this->y = y;
    sheet = &Art::items;
    xPic = 1;
    yPic = 0;
    wPic = 16;
    hPic = 16;
    xPicO = 8;
    yPicO = 15;
    life = 0;
}

void FireFlower::move() {
    if (life < 9) {
        layer = 0;
        y--;
        life++;
        return;
    }
    layer = 1;
}

void FireFlower::collideCheck() {
    float xMarioD = world->mario->x - x;
    float yMarioD = world->mario->y - y;
    
    if (xMarioD > -16 && xMarioD < 16) {
        if (yMarioD > -16 && yMarioD < world->mario->hPic) {
            world->mario->getFlower();
            world->removeSprite(this);
        }
    }
}
