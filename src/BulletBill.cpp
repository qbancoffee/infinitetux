/**
 * @file BulletBill.cpp
 * @brief Bullet Bill implementation.
 */
// BulletBill.cpp
#include "BulletBill.h"
#include "LevelScene.h"
#include "Mario.h"
#include "Art.h"
#include "Shell.h"
#include "Fireball.h"

BulletBill::BulletBill(LevelScene* world, float x, float y, int dir)
    : world(world) {
    this->x = x;
    this->y = y;
    facing = dir;
    
    sheet = &Art::enemies;
    xPic = 0;
    yPic = 5;
    wPic = 16;
    hPic = 32;
    xPicO = 8;
    yPicO = 31;
    
    xFlipPic = facing == -1;
}

void BulletBill::move() {
    if (deadTime > 0) {
        deadTime--;
        if (deadTime == 0) {
            deadTime = 1;
            world->removeSprite(this);
        }
        
        x += xa;
        y += ya;
        ya += 1;
        return;
    }
    
    xa = facing * 4;
    x += xa;
}

void BulletBill::collideCheck() {
    if (deadTime > 0) return;
    
    float xMarioD = world->mario->x - x;
    float yMarioD = world->mario->y - y;
    float w = 16;
    
    if (xMarioD > -w && xMarioD < w) {
        // Use Mario's collision height, not sprite height
        if (yMarioD > -height && yMarioD < world->mario->height) {
            // Stomp check: Mario falling, above bullet, and airborne
            if (world->mario->ya > 0 && yMarioD <= 0 && 
                (!world->mario->onGround || !world->mario->wasOnGround)) {
                world->mario->stomp(this);
                deadTime = 100;
                xa = 0;
                ya = 1;
            } else {
                world->mario->getHurt();
            }
        }
    }
}

bool BulletBill::shellCollideCheck(Shell* shell) {
    if (deadTime > 0) return false;
    
    float xD = shell->x - x;
    float yD = shell->y - y;
    
    if (xD > -16 && xD < 16) {
        if (yD > -height && yD < shell->height) {
            Art::playSound(SAMPLE_MARIO_KICK);
            deadTime = 100;
            ya = -5;
            return true;
        }
    }
    return false;
}

/**
 * Bullet Bills are invincible to fireballs.
 * They can only be killed by stomping on them.
 * 
 * @param fireball The fireball to check collision with
 * @return false - fireballs pass through without effect
 */
bool BulletBill::fireballCollideCheck(Fireball* fireball) {
    // Bullet Bills are immune to fireballs - can only be stomped
    return false;
}
