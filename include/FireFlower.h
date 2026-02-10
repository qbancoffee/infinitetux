/**
 * @file FireFlower.h
 * @brief Fire power-up item.
 * @ingroup sprites
 * 
 * FireFlower emerges from blocks and grants fire
 * power when collected by large Mario.
 */
#pragma once
#include "Sprite.h"

class LevelScene;

class FireFlower : public Sprite {
public:
    FireFlower(LevelScene* world, float x, float y);
    void move() override;
    void collideCheck() override;

private:
    LevelScene* world;
    int life = 0;
};
