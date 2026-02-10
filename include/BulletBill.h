/**
 * @file BulletBill.h
 * @brief Flying bullet enemy.
 * @ingroup sprites
 * 
 * BulletBill flies horizontally and is immune to
 * fireballs. Can only be defeated by stomping.
 */
#pragma once
#include "Sprite.h"

class LevelScene;

class BulletBill : public Sprite {
public:
    int facing = 1;
    int height = 12;
    int deadTime = 0;
    
    BulletBill(LevelScene* world, float x, float y, int dir);
    void move() override;
    void collideCheck() override;
    bool shellCollideCheck(Shell* shell) override;
    bool fireballCollideCheck(Fireball* fireball) override;

private:
    LevelScene* world;
};
