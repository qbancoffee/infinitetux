/**
 * @file Enemy.h
 * @brief Base class for ground enemies.
 * @ingroup sprites
 * 
 * Enemy provides walking patrol behavior, gravity,
 * and death handling for Goomba, Koopa, and Spiky enemies.
 */
#pragma once
#include "Sprite.h"

class LevelScene;
class Shell;
class Fireball;

class Enemy : public Sprite {
public:
    static constexpr int ENEMY_RED_KOOPA = 0;
    static constexpr int ENEMY_GREEN_KOOPA = 1;
    static constexpr int ENEMY_GOOMBA = 2;
    static constexpr int ENEMY_SPIKY = 3;
    static constexpr int ENEMY_FLOWER = 4;
    
    int facing = 1;
    int deadTime = 0;
    bool flyDeath = false;
    bool avoidCliffs = true;
    bool winged = true;
    bool noFireballDeath = false;
    
    Enemy(LevelScene* world, int x, int y, int dir, int type, bool winged);
    
    void collideCheck() override;
    void move() override;
    void render(SDL_Renderer* renderer, float alpha) override;
    bool shellCollideCheck(Shell* shell) override;
    bool fireballCollideCheck(Fireball* fireball) override;
    void bumpCheck(int xTile, int yTile) override;

protected:
    static constexpr float GROUND_INERTIA = 0.89f;
    static constexpr float AIR_INERTIA = 0.89f;
    
    float runTime = 0;
    bool onGround = false;
    bool mayJump = false;
    int jumpTime = 0;
    float xJumpSpeed = 0;
    float yJumpSpeed = 0;
    
    int width = 4;
    int height = 24;
    
    LevelScene* world;
    int type;
    int wingTime = 0;
    
    bool moveImpl(float xa, float ya);
    bool isBlocking(float _x, float _y, float xa, float ya);
};
