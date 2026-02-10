/**
 * @file Shell.h
 * @brief Kicked/carried koopa shell.
 * @ingroup sprites
 * 
 * Shell can be stationary, moving (kills enemies),
 * or carried by Mario.
 */
#pragma once
#include "Sprite.h"

class LevelScene;

class Shell : public Sprite {
public:
    int facing = 0;
    bool carried = false;
    int height = 12;
    int anim = 0;
    bool dead = false;
    
    Shell(LevelScene* world, float x, float y, int type);
    void tick() override;  // Override to handle carried state
    void move() override;
    void render(SDL_Renderer* renderer, float alpha) override;
    void collideCheck() override;
    void bumpCheck(int xTile, int yTile) override;
    void release(Mario* mario) override;
    bool fireballCollideCheck(Fireball* fireball) override;
    bool shellCollideCheck(Shell* shell) override;
    void die();

private:
    LevelScene* world;
    int type;
    bool onGround = false;
    int width = 4;
    int deadTime = 0;
    
    bool moveImpl(float xa, float ya);
    bool isBlocking(float _x, float _y, float xa, float ya);
};
