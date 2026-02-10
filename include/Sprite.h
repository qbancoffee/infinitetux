/**
 * @file Sprite.h
 * @brief Base class for all game sprites.
 * @ingroup sprites
 * 
 * Sprite provides position, velocity, rendering, and
 * collision detection for all game objects.
 */
#pragma once
#include "Common.h"
#include <vector>

class SpriteTemplate;
class Shell;
class Fireball;
class Mario;
class LevelScene;

class Sprite {
public:
    float xOld = 0, yOld = 0, x = 0, y = 0, xa = 0, ya = 0;
    
    int xPic = 0, yPic = 0;
    int wPic = 32;
    int hPic = 32;
    int xPicO = 0, yPicO = 0;
    bool xFlipPic = false;
    bool yFlipPic = false;
    std::vector<std::vector<SDL_Texture*>>* sheet = nullptr;
    bool visible = true;
    
    int layer = 1;
    
    SpriteTemplate* spriteTemplate = nullptr;
    LevelScene* spriteContext = nullptr;
    
    Sprite();
    virtual ~Sprite() = default;
    
    virtual void move();
    virtual void render(SDL_Renderer* renderer, float alpha);
    
    virtual void tick();
    void tickNoMove();
    
    float getX(float alpha) const;
    float getY(float alpha) const;
    
    virtual void collideCheck();
    virtual void bumpCheck(int xTile, int yTile);
    virtual bool shellCollideCheck(Shell* shell);
    virtual void release(Mario* mario);
    virtual bool fireballCollideCheck(Fireball* fireball);
};
