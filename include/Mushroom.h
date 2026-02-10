/**
 * @file Mushroom.h
 * @brief Power-up mushroom item.
 * @ingroup sprites
 * 
 * Mushroom walks along ground. Grants large state
 * (or 1-up variant grants extra life).
 */
#pragma once
#include "Sprite.h"

class LevelScene;

class Mushroom : public Sprite {
public:
    Mushroom(LevelScene* world, float x, float y);
    void move() override;
    void collideCheck() override;
    void bumpCheck(int xTile, int yTile) override;

private:
    LevelScene* world;
    int facing = 1;
    int life = 0;
    bool onGround = false;
    int width = 4;
    int height = 12;
    
    bool moveImpl(float xa, float ya);
    bool isBlocking(float _x, float _y, float xa, float ya);
};
