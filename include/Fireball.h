/**
 * @file Fireball.h
 * @brief Mario's fire projectile.
 * @ingroup sprites
 * 
 * Fireball bounces along the ground and kills
 * most enemies on contact.
 */
#pragma once
#include "Sprite.h"

class LevelScene;

class Fireball : public Sprite {
public:
    int facing = 1;
    int height = 8;
    bool dead = false;
    
    Fireball(LevelScene* world, float x, float y, int facing);
    void move() override;
    void die();

private:
    LevelScene* world;
    int width = 4;
    int anim = 0;
    bool onGround = false;
    int deadTime = 0;
    
    bool moveImpl(float xa, float ya);
    bool isBlocking(float _x, float _y, float xa, float ya);
};
